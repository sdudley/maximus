/*
 * Maximus Version 3.02
 * Copyright 1989, 2002 by Lanius Corporation.  All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#pragma off(unreferenced)
static char rcs_id[]="$Id: m_attach.c,v 1.2 2003/06/04 23:25:09 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: File attach related functions
*/

#define MAX_LANG_m_area
#define MAX_LANG_m_browse
#define MAX_LANG_max_chat
#define MAX_LANG_f_area

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "dr.h"
#include "prog.h"
#include "mm.h"
#include "max_msg.h"
#include "max_file.h"
#include "l_attach.h"
#include "display.h"
#include "m_attach.h"
#include "arcmatch.h"
#include "userapi.h"

static char *attach_path=NULL;

static const char lfa_kludge[] = "\x01LFA: ";

static void near LFAFindClose(LFAFIND * pLfaF);

void Msg_AttachUpload(PMAH pmah, HAREA marea, XMSG *msg, UMSGID uid)
{
  int ok;                   /* TRUE if attach was successfully uploaded */
  int fGotFiles;            /* TRUE if we received a file to attach */

  FENTRY fent;

  logit(log_attach_upload);

  if ((attach_path=malloc(PATHLEN))==NULL)
  {
    logit(mem_none);
    return;
  }
  
  if (Make_Attach_Directory()==-1)
  {
    free(attach_path);
    attach_path=NULL;
    return;
  }

  save_tag_list(NULL);
  
  /* Loop until we get a file or until user wants out. */

  fGotFiles = TRUE;
  ok = FALSE;

  while (Receive_Attach()==-1)
  {
    Printf(err_receive_attach);

    if (GetyNAnswer(really_abort_attach, 0)==YES)
    {
      fGotFiles = FALSE;
      break;
    }

    WhiteN();
  }

  if (fGotFiles)
  {
    dword iLfa_flags;
    char fname[PATHLEN*3];

     /* Local attaches stay where they are, but marked "do not delete" */

    WhiteN();
    iLfa_flags=0L;
    ok=TRUE;
    *fname='\0';

    if (local)
    {
      int fn;
      char *p=fname;

      for (fn=0; GetFileEntry(fn, &fent); ++fn)
      {
        strcpy(p, fent.szName);
        p += strlen(p);
        *p++=' ';
      }
      if (p > fname)
        --p;
      *p='\0';
    }

    if (local && GetYnAnswer(attach_ask_compr, 0)==NO)
      iLfa_flags=LFA_NODELETE|LFA_FILE_PATH;

     /* Otherwise, we need to compress the uploaded file(s) */

    else if (Compress_Attach_Files(pmah, fname, &iLfa_flags)==-1)
    {
      Printf(err_compr_attach);
      ok=FALSE;
    }

    if (ok)
      Update_Attach_Db(pmah, marea, msg, uid, fname, iLfa_flags);
  }

  if (ok)
  {

    /* If everything went OK, write the MSGFILE bit to the base */

    HMSG mh = MsgOpenMsg(marea, MOPEN_RW, MsgUidToMsgn(marea,uid,UID_EXACT));
    if (!mh)
      logit("!Can't reopen msg - msgapierr=%d", msgapierr);
    else
    {
      if (MsgReadMsg(mh,msg,0L,0L,NULL,0L,NULL)==-1)
        logit("!Error reading msg - msgapierr=%d", msgapierr);
      else
      {
        msg->attr |= MSGFILE;

        if (MsgWriteMsg(mh,FALSE,msg,NULL,0L,0L,0L,NULL)==-1)
          logit("!Error writing msg - msgapierr=%d", msgapierr);
      }
      MsgCloseMsg(mh);
    }
  }

  Clean_Directory(attach_path,TRUE);
  free(attach_path);
  attach_path=NULL;

  Free_Filenames_Buffer(0);
  restore_tag_list(NULL, FALSE);
}



static int near Make_Attach_Directory(void)
{
  /* Make sure that the appropriate control file sections are filled out */

  if (*PRM(attach_path)=='\0')
  {
    char temp[PATHLEN];
#ifndef UNIX
    sprintf(temp, "%sNOUPLOAD.BBS", FAS(fah, downpath));
#else
    sprintf(temp, "%snoupload.bbs", FAS(fah, downpath));
#endif

    if (fexist(temp))
      Display_File(DISPLAY_NONE,NULL,temp);
    else Puts(noupl);      
    return -1;
  }

  sprintf(attach_path,"%snode%02x", PRM(attach_path), task_num);

  return Make_Clean_Directory(attach_path);
}


static int near Receive_Attach(void)
{
  struct _css css;
  sword protocol;
  char temp[PATHLEN*3];
  long ul_start_time, b_free;

  /* Perform the actual upload */

  Free_Filenames_Buffer(0);

  if (local)
  {
    char *s;

    WhiteN();
    InputGetsL(temp,0,make_attach_local);

    for (s=strtok(temp,cmd_delim); s; s=strtok(NULL,cmd_delim))
    {
      if (AddFileEntry(s, 0, 0)==0)
        break;
    }
  }
  else
  {
    word sf2;

    if (File_Get_Protocol(&protocol, FALSE, TRUE)==-1)
      return -1;

    ChatSaveStatus(&css);
    ChatSetStatus(FALSE, cs_file_xfer);

    if (baud < ClassGetInfo(cls,CIT_MIN_XFER_BAUD))
    {
      Display_File(0, NULL, PRM(xferbaud));
      Printf(baudtoolowforxfer, ClassGetInfo(cls,CIT_MIN_XFER_BAUD));
      Press_ENTER();
      return -1;
    }

    b_free=zfree(attach_path);

    if (prm.k_free && b_free < (long)prm.k_free*1000L)
    {
      Display_File(0, NULL, PRM(no_space));
      return -1;
    }

    Printf(bytes_for_ul, commaize(b_free-(long)prm.k_free*1000L, temp));

    if (! IsBatch(protocol))
      File_Get_Upload_Names();

    /* Disable the dupe upload checking */

    sf2=prm.flags2;
    prm.flags2 &= ~(FLAG2_CHECKDUPE|FLAG2_CHECKEXT);
    ul_start_time=time(NULL);

    File_Get_Files(protocol, NULL, attach_path);

    prm.flags2=sf2;
    /* Compensate the user for the time spent uploading */
    Add_To_Time(time(NULL)-ul_start_time);
    ChatRestoreStatus(&css);

  }

  if (FileEntries())
  {
    word fn;
    sword gotfiles;

    FENTRY fent;

    for (gotfiles=0, fn=0; GetFileEntry(fn, &fent); ++fn)
    {
      if (local)
        strcpy(temp, fent.szName);
      else
      {
        strcpy(temp, attach_path);
        strcat(temp, fent.szName);
      }
      if (fexist(temp))
        ++gotfiles;
      else
        logit(cantfind, temp);
    }
    if (gotfiles)   /* Got at least one file successfully */
      return 0;
  }
  return -1;
}

static struct _arcinfo *near attach_archiver()
{
  struct _arcinfo *ar;

  Load_Archivers();

  if ((ar=ari)!=NULL)
  {
    for (;ar;ar=ar->next)
      if (eqstri(PRM(attach_archiver),ar->arcname))
        break;
    if (!ar)
      ar=ari;
  }
  return ar;
}

static int near Compress_Attach_Files(PMAH pmah, char *szAttachName, dword *pulFlags)
{
  sword ret;

  struct _arcinfo *pai =attach_archiver();
  if (!pai)
  {
    Puts(unknown_compr);
    ret=-1;
  }
  else
  {
    char pth[PATHLEN];
    char files[PATHLEN];
    char tmp[PATHLEN * 2];
    char cmd[PATHLEN * 2];

    if (!pmah->ma.attachpath)
      strcpy(pth,PRM(attach_path));
    else
    {
      *pulFlags |= LFA_AREA_OVERRIDE;
      strcpy(pth,PMAS(pmah,attachpath));
    }
    Add_Trailing(pth, PATH_DELIM);

    if (*szAttachName)
      strcpy(files, szAttachName);  /* Passed name(s) of all files here */
    else
      sprintf(files, "%s" WILDCARD_ALL, attach_path);

    for (ret=strlen(pth) ; ; Delay(100))
    {
      sprintf(szAttachName, "%08lX.%s", (time(NULL)<<4L)|(long)task_num, pai->extension);
      strcpy(pth+ret,szAttachName);
      if (!fexist(pth))
        break;
    }
    
    Form_Archiver_Cmd(pth, files, cmd, pai->add);

    /* Add MaxPipe to the call */

    sprintf(tmp, maxpipe_cmd, cmd);
    strcpy(cmd, tmp);

    Puts(wait_compr_attach);

    ret=Outside(NULL, NULL, OUTSIDE_RUN, cmd, FALSE, CTL_NONE, 0, NULL);

    if (ret != 0 || !fexist(pth))
    {
      logit(log_err_compr, ret);
      unlink(pth);
      ret=-1;
    }
  }
  return ret;
}


/* Add record to the file attach database
   and add the ^aLFA: aid FILENAME kludge in the message */

static void near Update_Attach_Db(PMAH pmah,
                                  HAREA marea,
                                  XMSG *msg,
                                  UMSGID uid,
                                  char *szFileName,
                                  dword ulFlags)
{

  /* In netmail areas, make it a real file attach */

  if ((pmah->ma.attribs & MA_NET)!=0)
  {
    HMSG mh = MsgOpenMsg(marea, MOPEN_RW, MsgUidToMsgn(marea,uid,UID_EXACT));
    if (!mh)
      logit("!Can't reopen msg - msgapierr=%d", msgapierr);
    else
    {
      if (MsgReadMsg(mh,msg,0L,0L,NULL,0L,NULL)==-1)
        logit("!Error reading msg - msgapierr=%d", msgapierr);
      else
      {
        if (ulFlags & LFA_FILE_PATH)
          *msg->subj = '\0';
        else if (!(ulFlags & LFA_AREA_OVERRIDE))
          strcpy(msg->subj, PRM(attach_path));
        else
          strcpy(msg->subj, PMAS(pmah,attachpath));
        strcat(msg->subj, szFileName);

        if (MsgWriteMsg(mh,FALSE,msg,NULL,0L,0L,0L,NULL)==-1)
          logit("!Error writing msg - msgapierr=%d", msgapierr);
      }
      MsgCloseMsg(mh);
    }
  }
  else
  {

    /* Otherwise, enter it into the database */

    DBASE OS2FAR * pDb = LFAdbOpen(PRM(attach_base));

    if (!pDb)
      logit(log_err_open_attach_db, PRM(attach_base));
    else
    {
      int ok;
      HUF huf;
      char recipient[36];
      LFA_REC lfa;

      /* If receipient is a user, use realname to make searching easier */

      strcpy(recipient,msg->to);
      if ((huf=UserFileOpen(PRM(user_file), 0))!=NULL)
      {
        struct _usr u;

        if (!UserFileFind(huf, recipient, NULL, &u) &&
             UserFileFind(huf, NULL, recipient, &u))
          strcpy(recipient, u.name);

        UserFileClose(huf);
      }

      ok=TRUE;
      LFARecInit(&lfa, recipient, PMAS(pmah,name), uid, msg->from, szFileName);
      lfa.ulAttributes |= ulFlags;
      if (! LFAdbInsert(pDb, &lfa))
        ok=FALSE;
      else
      {
        HMSG mh = MsgOpenMsg(marea, MOPEN_RW, MsgUidToMsgn(marea,uid,UID_EXACT));
        if (!mh)
          logit("!Can't reopen msg - msgapierr=%d", msgapierr);
        else
        {
          unsigned klen;
          char * kludge;

          klen=(unsigned)MsgGetCtrlLen(mh);
          if ((kludge=malloc(klen+strlen(szFileName)+20))!=NULL)
          {
            if (MsgReadMsg(mh,msg,0L,0L,NULL,klen,kludge)==-1)
              logit("!Error reading msg - msgapierr=%d", msgapierr);
            else
            {
              klen=strlen(kludge);
              while (klen && kludge[klen-1]=='\x01')
                --klen;
              sprintf(kludge+klen, "%s%lx %s", lfa_kludge, lfa.ulAttachID, szFileName);
              if (MsgWriteMsg(mh,FALSE,msg,NULL,0L,0L,strlen(kludge)+1,kludge)==-1)
                logit("!Error writing msg - msgapierr=%d", msgapierr);
            }
            free(kludge);
          }
          MsgCloseMsg(mh);
        }
      }
      LFAdbClose(pDb);
      if (! ok)
        logit(log_err_upd_attach_db);
    }
  }
}

DBASE OS2FAR * Read_Attach(LFA_REC *plfa, XMSG *pxmsg, char * szCtrl, int isnetmsg)
{
  DBASE OS2FAR * pdb;
  char * p;
  dword attachId;
  char temp[PATHLEN];

  pdb=NULL;
  if (pxmsg && (pxmsg->attr & MSGFILE)!=0)
  {

    /* See if it's a local attach */

    if (szCtrl!=NULL &&
        (p=strstr(szCtrl, lfa_kludge))!=NULL &&
        sscanf(p+6,"%lx %s\x01", &attachId, temp)==2)
    {

      /* If it is, then see if it has been received */

      if ((pdb=LFAdbOpen(PRM(attach_base)))==NULL)
        logit(log_err_open_attach_db,PRM(attach_base));
      else
      {
        int rc;
        void *pvLookId[] = { NULL, NULL, NULL };
        PALIST * pplLook;

        pvLookId[0] = &attachId;
        pplLook = PalistNew();
        rc=LFAdbLookup(pdb, pvLookId, pplLook, plfa);
        PalistDelete(pplLook);
        if (!rc)
        {
          LFAdbClose(pdb);
          pdb=NULL;
        }
      }
    }
    else   /* Assume that it is a FTS-1 file attach */
    {
      char *s, *p=plfa->szFile;
      int gotpath;

      memset(plfa, '\0', sizeof(LFA_REC));
      plfa->ulAttributes = LFA_NODELETE|LFA_FILE_PATH;
      for (s=strtok(pxmsg->subj,cmd_delim); s; s=strtok(NULL, cmd_delim))
      {
          /* Must have a path, otherwise assume inbound directory */

        gotpath=(strrstr(s, path_delim)!=NULL);

        if (isnetmsg || gotpath)
        {
          if (!gotpath)
          { /* Inbound files are deletable */
            strcpy(p, PRM(inbound_files));
            plfa->ulAttributes &= ~LFA_NODELETE;
          }
          else  /* Files in the attach directory are deletable */
            if (eqstrni(s, PRM(attach_path), strlen(PRM(attach_path))))
              plfa->ulAttributes &= ~LFA_NODELETE;
          strcat(p, s);
          if (fexist(p))
          {
            p += strlen(p);
            *p++ = ' ';
          }
        }
      }
      if (p > plfa->szFile)   /* Got at least one attached file */
      {
        --p;
        pdb=FTSATTACHDBH;     /* 'Magic' handle which LFAdbClose() knows */
      }
      *p = '\0';
    }
  }
  return pdb;
}

int Attach_File(LFA_REC *plfa, char * szFile, char * szMsg)
{
  char *s;
  char *p = szFile;

  for (s=strtok(plfa->szFile,cmd_delim); s; s=strtok(NULL, cmd_delim))
  {
    if (plfa->ulAttributes & LFA_FILE_PATH)
      *p='\0';
    else if (!(plfa->ulAttributes & LFA_AREA_OVERRIDE))
      strcpy(p, PRM(attach_path));
    else
    {
      MAH ma={0};        /* We need to read the area */

      if (!ReadMsgArea(ham, plfa->szArea, &ma))
      {
        *p='\0';
        logit(log_err_noarea, plfa->szArea);
        return FALSE;
      }
      strcpy(p, MAS(ma, attachpath));

      /* If the sysop has removed the attach path for this area,
       * default to the main attach path.
       */

      if (!*p)
        strcpy(p, PRM(attach_path));

      DisposeMah(&ma);
    }
    strcat(p, s);
    if (! fexist(p))
    {
      logit(cantfind, p);
      if (szMsg)
      {
        Puts(szMsg);
        Press_ENTER();
      }
      return FALSE;
    }
    p += strlen(p);
    *p++=' ';
  }

  if (p > szFile)
    --p;
  *p = '\0';
  return TRUE;
}


int Msg_UnreceivedAttach(XMSG *pxmsg, char * szCtrl, int isnetmsg)
{
  int rc;
  DBASE OS2FAR * pdb;
  LFA_REC lfa;

  if ((pdb=Read_Attach(&lfa, pxmsg, szCtrl, isnetmsg))==NULL)
    rc=FALSE;
  else
  {
    LFAdbClose(pdb);
    rc=TRUE;
  }
  return rc;
}

static int near Send_Attach(int fDelok)
{
  int good = FALSE;

  FENTRY fent;


  if (local)
  {

    /* With local logins, ask for a directory where to move the files */

    char temp[PATHLEN];

    do
    {
      InputGets(temp, recv_attach_local);
      if (*temp)
        Add_Trailing(temp, PATH_DELIM);
      Puts("\n");
    }
    while (*temp && (!direxist(temp) || eqstri(temp, attach_path)));

    if (*temp)
    {
      int fn;
      int fDoMove;    /* TRUE if we are to move the file */


      /* Got one, so assume ok until we get an error */

      good=TRUE;
      for (fn=0; GetFileEntry(fn, &fent); ++fn)
      {
        char *p;
        char temp2[PATHLEN];
        char prompt[PATHLEN];

        /* Get the filename part */

        p = No_Path(fent.szName);

        /* Make the destination filename */

        sprintf(temp2, ss, temp, p);

        fDoMove = fDelok;

        /* If the file exists, make sure that the user really
         * wants to overwrite it.
         */

        if (fexist(temp2))
        {
          sprintf(prompt, ok_to_overwrite, temp2);

          if (GetyNAnswer(prompt, 0)==NO)
            continue;

          fDoMove = FALSE;
        }

        /* And move it */

        if ((fDoMove && move_file(fent.szName,temp2)==-1) ||
            (!fDoMove && lcopy(fent.szName,temp2)!=0))
        {
          logit(cantmove,fent.szName,temp2);
          good=FALSE;
        }

        /* Delete the file if necessary */

        if (fDelok && !fDoMove)
          unlink(fent.szName);
      }
    }
  }
  else
  {
    struct _css css;
    sword protocol;

    if (File_Get_Protocol(&protocol, FALSE, TRUE)!=-1)
    {

      ChatSaveStatus(&css);
      ChatSetStatus(FALSE, cs_file_xfer);

      if (File_Send_Files(protocol, NULL, NULL, 1))
        good=TRUE;

      ChatRestoreStatus(&css);

    }
  }

  return good;
}

static int near Decompress_Attach(char *szName)
{
  struct _arcinfo * pai;
  int fd, ret;

  Load_Archivers();

  if ((fd=shopen(szName, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return -1;

  for (pai=ari; pai; pai=pai->next)
    if (MatchArcType(fd, pai))
      break;

  close(fd);

  if (!pai)
    pai=ari;

  if (!pai)
  {
    Puts(unknown_compr);
    ret=-1;
  }
  else
  {
    FFIND *ff;
    char temp[PATHLEN];
    char szFullName[PATHLEN];

    strcpy(szFullName, szName);

    if (szFullName[1] != ':' && szFullName[0] != '\\' && szFullName[0] != '/')
    {
      strcpy(szFullName, PRM(sys_path));
      strcat(szFullName, szName);
    }


    /* Decompress the attach */

    Form_Archiver_Cmd(szFullName, "*.*", temp, pai->extract);


    /* Save our lastread pointers now, while we are in the current
     * directory and know how to reference the message bases.
     */

    FixLastread(sq, mah.ma.type, last_msg, MAS(mah, path));


    Save_Directory2(attach_path);
    ret=Outside(NULL, NULL, OUTSIDE_RUN | OUTSIDE_STAY | OUTSIDE_NOFIX,
                temp, FALSE, CTL_NONE, 0, NULL);
    Restore_Directories2();

    if (ret==0)
    {

      sprintf(temp, "%s" WILDCARD_ALL, attach_path);
      if ((ff=FindOpen(temp,0)) != NULL)
      {
        do
        {
          strcpy(temp, attach_path);
          strcat(temp, ff->szName);

          if (AddFileEntry(temp, FFLAG_NOBYTES, -1L)==-1)
          {
            Free_Filenames_Buffer(0);
            break;
          }
        }
        while (FindNext(ff)==0 && CanAddFileEntry());
        FindClose(ff);
      }
    }

    if (!FileEntries())
      Puts(no_attach_files);
  }

  return ret;
}


void Msg_AttachDownload(XMSG *pxmsg, char *szCtrl, int isnetmsg)
{
  DBASE OS2FAR * pdb;
  LFA_REC lfa;

  /* Get the file attach record */

  if ((pdb=Read_Attach(&lfa,pxmsg,szCtrl,isnetmsg))!=NULL)
  {
    char temp[PATHLEN*3];

    save_tag_list(NULL);
    Free_Filenames_Buffer(0);

    /* Get the path to the file and verify that it exists */

    if (Attach_File(&lfa,temp,attach_notavail))
    {

      logit(log_attach_download);

      if ((attach_path=malloc(PATHLEN))==NULL)
        logit(mem_none);
      else
      {

        /* Make our node-specific directory */

        if (Make_Attach_Directory()!=-1)
        {
          /* If it's a local attach, just add it directly
             to the transmission list */

          if (lfa.ulAttributes & LFA_FILE_PATH)
          {
            char *s;

            for (s=strtok(temp,cmd_delim); s ; s=strtok(NULL,cmd_delim))
            {
              if (AddFileEntry(s, FFLAG_NOBYTES, -1L)==-1)
                goto Done;
            }
          }

          /* Otherwise, decompress the attach into the node directory */

          else
          {

            Puts(wait_decompr_attach);

            if (Decompress_Attach(temp)==-1)
              Printf(err_decompr_attach);
          }

          /* If we've got any work to do, send it.
             If sending went ok, and we have auto-kill on, then kill'em */

          if (FileEntries() &&
              Send_Attach(!(lfa.ulAttributes & LFA_NODELETE)) &&
              prm.kill_attach==2 &&
              !(lfa.ulAttributes & LFA_NODELETEATTACH))

          {
            if (!(lfa.ulAttributes & LFA_NODELETE))
              unlink(temp);
            if (pdb!=FTSATTACHDBH)
            {
              void *pvLookId[] = { NULL, NULL, NULL };

              pvLookId[0] = &lfa.ulAttachID;
              pvLookId[1] = lfa.szTo;
              pvLookId[2] = lfa.szArea;
              LFAdbRemove(pdb, pvLookId);
            }
          }

          Clean_Directory(attach_path,TRUE);
        }

      Done:

        free(attach_path);
        attach_path=NULL;

      }
    }

    LFAdbClose(pdb);

    Free_Filenames_Buffer(0);
    restore_tag_list(NULL, FALSE);

  }
}

void Msg_AttachKill(XMSG *pxmsg, char *szCtrl, int isnetmsg)
{
  DBASE OS2FAR * pdb;
  LFA_REC lfa;

  /* Look up the file attach */

  if ((pdb=Read_Attach(&lfa, pxmsg, szCtrl, isnetmsg))!=NULL)
  {
    char temp[PATHLEN*3];

    if (!(lfa.ulAttributes & (LFA_NODELETE|LFA_NODELETEATTACH)) &&
        Attach_File(&lfa, temp, NULL))
    {
      char *s;
      for (s=strtok(temp,cmd_delim); s; s=strtok(NULL, cmd_delim))
        unlink(s);
    }

    /* If it's deletable, do it */

    if (pdb!=FTSATTACHDBH && (lfa.ulAttributes & LFA_NODELETEATTACH)==0)
    {
      void *pvLookId[] = { NULL, NULL, NULL };

      pvLookId[0] = &lfa.ulAttachID;
      pvLookId[1] = lfa.szTo;
      pvLookId[2] = lfa.szArea;
      LFAdbRemove(pdb, pvLookId);
    }
    LFAdbClose(pdb);
  }  
}

/* Determine whether there are any file attaches for the current user */

int Msg_AttachedFiles(void)
{
  int rc;

  rc=FALSE;
  if (prm.attach_base)
  {
    DBASE OS2FAR * pdb;

    if ((pdb=LFAdbOpen(PRM(attach_base)))==NULL)
      logit(log_err_open_attach_db,PRM(attach_base));
    else
    {
      LFA_REC lfa;
      void *pvLookId[] = { NULL, NULL, NULL };
      PALIST * pplLook;
  
      pvLookId[1] = &usr.name;  /* Only need to look up realname */
      pplLook = PalistNew();
      rc=LFAdbLookup(pdb, pvLookId, pplLook, &lfa);
      PalistDelete(pplLook);

      LFAdbClose(pdb);
    }
  }

  return rc;
}


static LFAFIND * near LFAFindOpen(char * name)
{
  DBASE OS2FAR * pdb;
  LFAFIND * pLfaF;

  pLfaF = NULL;
  if (prm.attach_base)
  {
    if ((pdb = LFAdbOpen(PRM(attach_base)))==NULL)
      logit(log_err_open_attach_db, PRM(attach_base));
    else if ((pLfaF=malloc(sizeof(LFAFIND)))==NULL)
      logit(mem_none);
    else
    {
      pLfaF->pdb = pdb;
      pLfaF->pvLookId[0] = NULL;
      pLfaF->pvLookId[1] = name;
      pLfaF->pvLookId[2] = NULL;
      pLfaF->pplLook = PalistNew();
      if (!LFAdbLookup(pLfaF->pdb, pLfaF->pvLookId, pLfaF->pplLook, &pLfaF->lfa))
      {
        LFAFindClose(pLfaF);
        pLfaF=NULL;
      }
    }
  }
  return pLfaF;
}


static int near LFAFindNext(LFAFIND * pLfaF)
{
  if (pLfaF!=NULL)
    return LFAdbLookup(pLfaF->pdb, pLfaF->pvLookId, pLfaF->pplLook, &pLfaF->lfa);
  return 0;
}

static void near LFAFindClose(LFAFIND * pLfaF)
{
  if (pLfaF!=NULL)
  {
    PalistDelete(pLfaF->pplLook);
    LFAdbClose(pLfaF->pdb);
    free(pLfaF);
  }
}


static int near Attach_List(void);
static void near Download_Attaches(void);

void Msg_Attach_Download(void)
{
  int n;

  n=Attach_List();

  WhiteN();

  if (n == 0)
  {
    Puts(no_files_attached);
    Press_ENTER();
  }
  else
  {
    char * p = malloc(strlen(n_files_attached)+12);
    sprintf(p, n_files_attached, n);

    if (n==1 || GetYnAnswer(p, 0)!=NO)
      Download_Attaches();

    free(p);
  }
}

static int near Attach_List(void)
{
  int n;
  LFAFIND * pLfaF;

  n = 0;
  if ((pLfaF=LFAFindOpen(usr.name))!=NULL)
  {
    byte nonstop;

    nonstop=FALSE;
    display_line=display_col=1;

    Printf(attach_hdr0,usr.name);
    WhiteN();
    Puts(attach_hdr0_5);
    Puts(attach_hdr1);
    Puts(attach_hdr2);

    do
    {
      char temp[PATHLEN];
      char szSize[20];
      long lSize = 0;
      char *s;

      if (halt() || MoreYnBreak(&nonstop, NULL))
        break;
      ++n;

      /* Add up the sizes of all of the files in this attach */

      if (Attach_File(&pLfaF->lfa, temp, NULL))
      {
        for (s=strtok(temp,cmd_delim); s; s=strtok(NULL,cmd_delim))
        {
          long l;

          l = fsize(s);

          if (l > 0)
            lSize += l;
        }
      }

      /* Turn size into kbytes */

      lSize /= 1000;

      if (lSize >= 1000)
        sprintf(szSize, "%d.%dM", lSize / 1000, (lSize / 100) % 10);
      else
        sprintf(szSize, "%dK", lSize);

      Printf(attach_fmt, FileDateFormat(&pLfaF->lfa.scDateAttached,temp),
                         pLfaF->lfa.scDateAttached.msg_st.time.hh,
                         pLfaF->lfa.scDateAttached.msg_st.time.mm,
                         pLfaF->lfa.szFrom,
                         szSize,
                         pLfaF->lfa.szArea);
    }
    while (LFAFindNext(pLfaF));
    LFAFindClose(pLfaF);

    Puts(attach_hdr2);
  }
  return n;

}

static void near Download_Attaches(void)
{
  LFAFIND * pLfaF;

  if ((pLfaF=LFAFindOpen(usr.name))!=NULL)
  {
    char temp[PATHLEN];

    save_tag_list(NULL);

    do
    {

      Printf(attach_fmt2, FileDateFormat(&pLfaF->lfa.scDateAttached, temp),
                          pLfaF->lfa.scDateAttached.msg_st.time.hh,
                          pLfaF->lfa.scDateAttached.msg_st.time.mm,
                          pLfaF->lfa.szFrom,
                          pLfaF->lfa.szArea);


      if (Attach_File(&pLfaF->lfa,temp,attach_notavail) &&
          GetYnAnswer(attach_dload,0)!=NO)
      {

        logit(log_attach_download);

        if ((attach_path=malloc(PATHLEN))==NULL)
          logit(mem_none);
        else
        {

          /* Make our node-specific directory */

          if (Make_Attach_Directory()!=-1)
          {

            /* If it's a local attach, just add it directly
               to the transmission list */

            if (pLfaF->lfa.ulAttributes & LFA_FILE_PATH)
            {
              if (AddFileEntry(temp, FFLAG_NOBYTES, -1L)==-1)
                goto Done;
            }

            /* Otherwise, decompress the attach into the node directory */

            else if (Decompress_Attach(temp)==-1)
              Printf(err_decompr_attach);

            /* If we've got any work to do, send it.
               If sending went ok, and we have auto-kill on, then kill'em */

            if (FileEntries() &&
                Send_Attach(!(pLfaF->lfa.ulAttributes & LFA_NODELETE)))
            {
              if ((prm.kill_attach==2 && !(pLfaF->lfa.ulAttributes & LFA_NODELETEATTACH)) ||
                  Ask_KillAttach())
              {
                void *pvLookId[] = { NULL, NULL, NULL };

                pvLookId[0] = &pLfaF->lfa.ulAttachID;
                pvLookId[1] = pLfaF->lfa.szTo;
                pvLookId[2] = pLfaF->lfa.szArea;
                if (!(pLfaF->lfa.ulAttributes & LFA_NODELETE))
                  unlink(temp);
                LFAdbRemove(pLfaF->pdb, pvLookId);
              }
            }
            else
              break;

            Clean_Directory(attach_path,TRUE);
          }

        Done:

          free(attach_path);
          attach_path=NULL;
        }
      }
    }
    while (LFAFindNext(pLfaF));
    LFAFindClose(pLfaF);

    restore_tag_list(NULL,FALSE);
  }
}

