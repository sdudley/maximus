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
static char rcs_id[]="$Id: f_down.c,v 1.1.1.1 2002/10/01 17:51:03 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: D)ownload command and associated functions
*/

#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include "prog.h"
#include "dr.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"
#include "f_down.h"
#include "f_idx.h"
#include "xmodem.h"
#include "zsjd.h"
#include "pdata.h"


static word fsent=0;
static word near expd;

void File_Download(char *mname)
{
  struct _css css;
  sword protocol;         /* Protocol to use for download */
  int hangup;
  word fn;

  FENTRY fent;

  if (local)
    baud=38400u;

  if (baud < ClassGetInfo(cls,CIT_MIN_XFER_BAUD))
  {
    Display_File(0, NULL, PRM(xferbaud));
    Printf(baudtoolowforxfer, ClassGetInfo(cls,CIT_MIN_XFER_BAUD));
    Press_ENTER();
    return;
  }

  if (File_Get_Protocol(&protocol, FALSE, FALSE)==-1)
    return;

  ChatSaveStatus(&css);
  ChatSetStatus(FALSE, cs_file_xfer);

  /* Check list of names to download, and see if any of them are tagged.    *
   * If not, then clear the 'fnames' variable to zero, and get a new        *
   * list.                                                                  */
     
  for (fn=0; GetFileEntry(fn, &fent); fn++)
    if (fent.fFlags & FFLAG_TAG)
      break;

  if (fn==FileEntries())
    Free_Filenames_Buffer(0);

  hangup=File_Get_Download_Names(TAG_NONE, protocol);

  if (hangup==-1 || !FileEntries())         /* At least one file to send */
    Puts(xferaborted);
  else
  {
    if (File_Send_Files(protocol, mname, FAS(fah, uppath), 0))
    {
      /* If user wants to hang up after download, give 'em time to abort */

      if (hangup && Shall_We_Continue(10, file_hangup_text))
        mdm_hangup();
    }
  }

  ChatRestoreStatus(&css);
}



int File_Get_Download_Names(int do_tag,sword protocol)
{
  FFIND *ff;
  
  byte *fpath=NULL;

  byte *namebuf, *newfn=NULL, *no_path=NULL;
  byte *p;

  int fBreakLoop, ret;
  word notinlist, flag, fn;
  sword gotret;

  FENTRY fent;


  if (local)
    baud=38400L;


  #define NAMEBUFLEN 255

  if ((namebuf=malloc(NAMEBUFLEN))==NULL ||
      (newfn=malloc(NAMEBUFLEN))==NULL ||
      (no_path=malloc(NAMEBUFLEN))==NULL ||
      (fpath=malloc(PATHLEN))==NULL)
  {
    if (namebuf)
    {
      if (newfn)
      {
        if (no_path)
          free(no_path);

        free(newfn);
      }

      free(namebuf);
    }

    logit(mem_none);
    return -1;
  }

  ret=FALSE;

  if (do_tag != TAG_ONELINE)
    Putc('\n');

  /* Tell the user how to perform various downloading commands */

  if (usr.help==NOVICE && do_tag==TAG_NONE && *linebuf=='\0')
    Puts(how_dl);


  /* Loop while we pick up file names */
  
  do
  {
    word breakout=FALSE;

    /* If we're doing a tag, ask the question specially */
    
    Puts(WHITE);

    if (do_tag)
    {
      Inputf(namebuf,(((do_tag==TAG_ONELINE && usr.video) ? INPUT_NOLF : 0) |
                  INPUT_LB_LINE), 0, NAMEBUFLEN-1,file_enter_tag,FileEntries()+1);

      if (do_tag==TAG_ONELINE)
      {
        Putc((usr.video & GRAPH_TTY) ? '\n' : '\r');
        Puts(file_clear_tag); /* DLN: RIP hook */
      }
    }
    else
    {

      InputGetsLL(namebuf,NAMEBUFLEN-1, IsBatch(protocol) ? file_dl : file1_dl, FileEntries()+1);
    }


    /* If the user just hit <enter>, then start the DL */

    if (*namebuf=='\0')
    {
      long realbytes, realtime;

      /* Only let the user proceed with the download if limits are OK */

      if (FileLimitsOkay(0L, 0, usr.def_proto, &realbytes, &realtime))
        break;
      else
      {
        Putc('\n');
        Puts(how_dl);
      }
    }

    /* Parse the filename string, and extract any file names the user       *
     * entered.  We'll place all of them in the download list, whether      *
     * or not they exist, since we need to verify all of them later in      *
     * FILES.BBS anyway...                                                  */

    fBreakLoop=FALSE;

    for (p=strtok(namebuf, dl_delim);
         p && !fBreakLoop;
         p=strtok(NULL, dl_delim))
    {
      char *cpnp;

      if (*p=='/')
      {
        p[1]=(byte)toupper(p[1]);

        if (p[1]==file_dl_keys[0])        /* edit list */
          File_Tag(TRUE);
        else if (p[1]==file_dl_keys[1])   /* goodbye after dl */
        {
          ret=TRUE;
          breakout=TRUE;
          break;
        }
        else if (p[1]==file_dl_keys[2])   /* quit */
        {
          ret=-1;
          breakout=TRUE;
          break;
        }
        
        continue;
      }

      if (*p=='|')
      {
        breakout=TRUE;
        break;
      }
      
      /* Create a copy of the specified filename */
      
      strcpy(no_path, p);
      Strip_Path(no_path);

      /* Remove the path entry, if any, that the user may have added */

      if (!CanAddFileEntry())
        break;

      /* So far, this entry has NOT been expanded */

      cpnp=strdup(no_path);

      expd=EXP_NONE;
      flag=0;
      notinlist=FALSE;


      /* If we don't get a match, and the filename doesn't include a '.',   *
       * then try to add a '.*' to the end of the filename.                 */

      /* Create a full pathspec for this puppy */

      strcpy(newfn, FAS(fah, downpath));
      strcat(newfn, no_path);
      
      *fpath='\0';


      /* Check for files in the CURRENT area */

      if ((ff=FindOpen(newfn, 0))==NULL && !strchr(no_path, '.'))
      {
        strcat(newfn, dot_star);
        strcat(no_path, dot_star);

        ff=FindOpen(newfn, 0);
      }


      if (ff==NULL)
      {
        if (InFileList(&fah, no_path, &flag, fpath)==DL_NOTFOUND)
          notinlist=TRUE;
        else
        {
          strcpy(newfn, fpath);
          strcat(newfn, no_path);
  
          ff=FindOpen(newfn, 0);
        }
      }
      
      if (ff)
      {
        do
        {
          /* Check for ^C */

          if (halt())
          {
            fBreakLoop=TRUE;
            break;
          }

          flag=0;

          if (!CanAddFileEntry())
            break;

          strupr(ff->szName);

          if (CanDownload(&fah, ff->szName, &flag, fpath))
          {
            word flags=FFLAG_OK | FFLAG_THIS1 | flag;
            long size=ff->ulSize;
            char temp[PATHLEN];

            sprintf(temp,
                    fn_format,
                    *fpath ? fpath : FAS(fah, downpath),
                    MAX_FN_LEN, 
                    ff->szName);

            if (do_tag)
              flags |= FFLAG_TAG;

            if (fah.fa.attribs & FA_STAGED)
              flags |= FFLAG_STAGE;

            if (fah.fa.attribs & FA_SLOW)
              flags |= FFLAG_SLOW;

            gotret=GotFile(temp, size, do_tag, protocol, flags);

            if (gotret==-1 || (gotret==0 && AddFileEntry(temp, flags, size)==-1))
              break;
          }

          if (flag & FFLAG_NOLIST)
            notinlist=TRUE;
        }
        while (FindNext(ff)==0);

        FindClose(ff);
      }


      if (!fBreakLoop)
      {
        /* Now check the global index to see if it's there, starting with   *
         * the basic filename again.                                        */

        if (cpnp==NULL)
        {
          /* If we couldn't allocate it, start again from the beginning */

          strcpy(no_path, p);
          Strip_Path(no_path);
        }
        else
        {
          /* Else just make a copy of the stripped path */

          strcpy(no_path, cpnp);
          free(cpnp);
          cpnp=NULL;
        }

        IndexSearch(no_path, do_tag, protocol);

        /* Not found in the global index either.  If there's no period in   *
         * the filename, try adding a ".*" to the requested file, and doing *
         * another index search.                                            */

        if (expd==EXP_NONE && !strchr(no_path, '.'))
        {
          strcat(no_path, ".*");

          IndexSearch(no_path, do_tag, protocol);
        }

        /* If we didn't get anything, report an error message */

        if (expd==EXP_NONE)
        {
          if (notinlist)
            File_NotExist(no_path);
          else
            File_IsOffline(no_path);
        }
      }
    }

    /* Make sure that only 1 file can be downloaded with a non-batch proto */

    if (!IsBatch(protocol) && FileEntries() > 1)
    {
      Puts(file_only1);
      Free_Filenames_Buffer(1);
    }

    for (fn=0; GetFileEntry(fn, &fent); fn++)
    {
      if (fent.fFlags & FFLAG_THIS1)
      {
        fent.fFlags &= ~FFLAG_THIS1;
        UpdFileEntry(fn, &fent);
      }
    }

    if (breakout)
      break;

    /* If the user entered too many file names, mention it to user */

    if (!CanAddFileEntry())
      Printf(file_max_expand, FileEntries());
  }
  while (!do_tag && IsBatch(protocol)); /* loop if we're NOT tagging files */

  free(fpath);
  free(no_path);
  free(newfn);
  free(namebuf);

  return ret;
}










static int near CanDownload(PFAH pfah, char *name, word *flag, char *path)
{
  int fex;
/*int flist;*/

  fex=FileExist(*path ? path : FAS(*pfah, downpath), name);

  /*flist=*/ InFileList(pfah, name, flag, NULL);

  /* If the file exists, but it's not in the list, we can still download    *
   * it if we're above the dl-all priv level                                */

  if (fex && (*flag & FFLAG_NOLIST) && acsflag(CFLAGA_FLIST))
    *flag &= ~FFLAG_NOLIST;

  
  return (fex && (*flag & FFLAG_NOLIST)==0);
}



static int near FileExist(char *path, char *name)
{
  char *t=malloc(NAMEBUFLEN);
  int ret;
  
  if (t==NULL)
  {
    logit(mem_none);
    return FALSE;
  }
  
  sprintf(t, ss, path, name);
  
  ret=fexist(t);
  
  free(t);
  return ret;
}


static int near InFileList(PFAH pfah, char *name, word *flag, char *pt)
{
  char *p=strrstr(name, pdel_only);
  
  /* If p != NULL, it points to the last filespec delimiter, which we       *
   * should skip over.  Otherwise, there was no delimiter, so we should     *
   * use the base filename.                                                 */

  if (p)
    p++;
  else p=name;
  
  return (/*IsInFilesDat(ar,name,flag)||*/ IsInFilesBbs(pfah, name, flag, pt));
}


static int near IndexSearch(char *name, int do_tag, sword protocol)
{
  IDXF *ix;
  char *p=strrstr(name, pdel_only);
  
  /* Strip off the path specification */

  if (p)
    p++;
  else p=name;

  if ((ix=IndexOpen(name))==NULL)
    return -1;
  
  do
  {
    /* Check for ^C */

    if (halt())
      break;

    if (FindIndexFile(&ix->fidx, do_tag, protocol)==-1)
    {
      IndexClose(ix);
      return -1;
    }
  }
  while (IndexNext(ix)==0);
  
  IndexClose(ix);
  
  return 0;
}

  




/* This routine is called after finding a possible match in MAXFILES.IDX.   *
 * Returns 0 for not found, 1 for found, and -1 for error.                  */

static int near FindIndexFile(struct _fidx *fidx, int do_tag, sword protocol)
{
  FAH fa={0};
  struct _fdat fdat;
  char areaname[MAX_ALEN];
  char apath[PATHLEN];
  char fname[PATHLEN];
  int rc=0;
  int x;

  /* This file is okay as long as the index is there, if it is NOT the      *
   * current area (since that'd be picked up by a FILES.BBS/DAT serach),    *
   * as long as the entry is a file, and as long as the user's priv is      *
   * high enough to download it.                                            */

  if (FidxIsOkay(fidx, &fdat, areaname, apath, FALSE, TRUE, &fa) &&
      !eqstri(areaname, FAS(fah, name)) &&
      ((fdat.flag & (FF_FILE|FF_OFFLINE|FF_DELETED|FF_COMMENT))==FF_FILE))
  {
    strcpy(fname, apath);
    strcat(fname, fdat.name);
    
    if ((fa.fa.attribs & FA_SLOW) || fexist(fname))
    {
      dword size=-1L;
      word flags=FFLAG_OK | FFLAG_THIS1;

      /* Get the file size from the .dat if it's a slow file area */

      if (fa.fa.attribs & FA_SLOW)
        size=fdat.fsize;
      else
        size=fsize(fname);

      if (do_tag)
        flags |= FFLAG_TAG;

      if ((fdat.flag & FF_NOTIME) || (fa.fa.attribs & FA_FREETIME))
        flags |= FFLAG_NOTIME;

      if ((fdat.flag & FF_NOBYTES) || (fa.fa.attribs & FA_FREESIZE))
        flags |= FFLAG_NOBYTES;

      if (fa.fa.attribs & FA_STAGED)
        flags |= FFLAG_STAGE;

      if (fa.fa.attribs & FA_SLOW)
        flags |= FFLAG_SLOW;

        /* If we got the file okay, add it to the files list */

      x=GotFile(fname, size, do_tag, protocol, flags);

      if (x==-1 || (x==0 && AddFileEntry(fname, flags, size)==-1))
        rc=-1;
    }
  }

  DisposeFah(&fa);
  return rc;
}






static int near GotFile(char *fname, long size, int do_tag, sword protocol,
                        int flags)
{
  long this_time=XferTime(protocol, size);

  long realbytes, realtime;
  word thisfile;

  FENTRY fent;

  
  /* First check the list of file names, to make sure that the user         *
   * hasn't already selected this one for download.                         */

  for (thisfile=0; GetFileEntry(thisfile, &fent); thisfile++)
    if (fent.szName && eqstri(No_Path(fent.szName), No_Path(fname)))
    {
      /* Only display error message if not expanded during this wc pass */
      
      if ((fent.fFlags & FFLAG_THIS1)==0)
        Printf(dup_filename, No_Path(fent.szName));

      expd |= EXP_ERR;
      return 1;
    }

  if (do_tag==TAG_ONELINE)
    Putc('\r');
  
  Printf(file_stats,
         thisfile+1,
         No_Path(fname),
         (int)(this_time / 60L),
         (int)(this_time % 60L),
         (long)size);

  if (do_tag != TAG_ONELINE || usr.video==GRAPH_TTY)
    Putc('\n');

  vbuf_flush();
  
  /* Return a value which indicates whether or not we've run out of time,   *
   * bytes, ratio, etc.                                                     */
 
  if (FileLimitsOkay(size, flags, usr.def_proto, &realbytes, &realtime))
  {
    expd |= EXP_YES;
    return 0;
  }
  
  /* Else file limits NOT okay */
  expd |= EXP_ERR;
  return -1;
}

















/* Take care of the stuff we need to do after successfully sending a file */

void MaxSentFile(word fn, word log_it, long max_time)
{
  long fsz;
  long to_add;

  FENTRY fent;

  GetFileEntry(fn, &fent);

  fsent++;

  if (fent.fFlags & FFLAG_NOTIME)
  {
    to_add=XferTime(PROTOCOL_ZMODEM, fent.ulSize);

    /* Make sure that we don't give back more time than the user            *
     * actually spent, and make sure that we don't give back                *
     * more than it would have ideally taken to download the file.          */

    if (max_time != -1L)
      to_add=min(to_add, max_time);

    logit(log_free_time_dl, Add_To_Time(to_add));
  }

  /* Use array for slow file areas */

  if (fent.fFlags & FFLAG_SLOW)
    fsz=fent.ulSize / 1024L;
  else
    fsz=fsize(fent.szName) / 1024L;

  if (fent.fFlags & FFLAG_NOBYTES)
    logit(log_free_bytes_dl, fsz);
  else
  {
    usr.down += fsz;
    usr.downtoday += fsz;
    usr.ndown++;
    usr.ndowntoday++;
    ci_dnload(fsz);
  }

  bstats.total_dl += fsz;

  /* If we were supposed to log it, make a statement in the log. */

  if (log_it)
    logit(log_dl, protocol_letter, fent.szName);
}



/* Get the name of the directory used for staging file transfes */

static void near StageFileGetDir(char *szPath)
{
  sprintf(szPath, "%snode%02x" PATH_DELIMS, PRM(stagepath), task_num);
}



/* Get the staged name of a file to be downloaded */

static void near StageFileGetName(char *szOriginal, char *szNew)
{
  StageFileGetDir(szNew);
  strcat(szNew, No_Path(szOriginal));
}



/* Create a directory whose name ends with a backslash */

static int near mkdirslash(char *path)
{
  char last_ch=0;
  int len;
  int rc;

  if (path[len=strlen(path)-1]=='\\' || path[len]=='/')
  {
    last_ch=path[len];
    path[len]=0;
  }

  rc=mkdir(path);

  if (last_ch)
    path[len]=last_ch;

  return rc;
}



/* Try to stage a file transfer by copying a file to a temporary
 * directory, placing the new filename in szNew.
 */

static int near StageFile(char *szOriginal, char *szNew, int fClean)
{
  char szPath[PATHLEN];
  char *szStagePath=PRM(stagepath);
  FFIND *ff;

  /* Make sure that the basic staging directory exists */

  if (*szStagePath==0)
    return FALSE;

  if (!direxist(szStagePath))
    if (mkdirslash(szStagePath) != 0)
      return FALSE;

  /* Get the name for this node's staging directory */

  StageFileGetDir(szPath);

  if (!direxist(szPath))
    if (mkdirslash(szPath) != 0)
      return FALSE;

  /* Get the name of the staged file */

  StageFileGetName(szOriginal, szNew);


  /* Delete all files from the directory before we do this */

  strcat(szPath, WILDCARD_ALL);

  if (fClean && (ff=FindOpen(szPath, 0)) != NULL)
  {
    do
    {
      StageFileGetDir(szPath);
      strcat(szPath, ff->szName);
      unlink(szPath);
    }
    while (FindNext(ff)==0);

    FindClose(ff);
  }

  if (lcopy(szOriginal, szNew) != 0)
  {
    unlink(szNew);
    return FALSE;
  }

  return TRUE;
}




/* Delete a staged file and try to remove the staging directory */

static void near UnstageFile(char *szOld)
{
  char szPath[PATHLEN];

  unlink(szOld);

  StageFileGetDir(szPath);
  Strip_Trailing(szPath, PATH_DELIM);
  rmdir(szPath);
}



/* Send multiple files to the remote user */

/* 'Sub' procedure does no display work or dload checking before download */
/* Note: 'flag' here is different from File_Send_Files() - it determines */
/* whether or not to send any output to the remote system */

word File_Send_Files_Sub(sword protocol, char *mname, char *newuppath, long realbytes, int flag)
{
  extern word *crctab;
  extern dword *cr3tab;

  char temp[PATHLEN];
  char xferstatus;

  word fn;                           /* The file number we're working on */
  int result;

  FENTRY fent;

  fsent=0;

  /* Force log messages to appear on-screen */

  in_file_xfer=TRUE;
  last_bps=0;

  if (flag)
    logit(log_sending_to, usr.name, baud);

  /* Turn off ^S/^Q flow control and ^C/^K checking, so it doesn't screw  *
   * up the transfers!                                                    */

  Mdm_flush();
  Mdm_Flow_Off();


  vbuf_flush();


  /* If it's an external protocol, let the Outside() command handle it */

  if (protocol >= 0)
  {
    struct _proto *pro;
    word gotfile=FALSE;


    /* Get the letter of the protocol to use */

    protocol_letter=*Protocol_Name(protocol,temp);


    /* Allocate space for the protocol structure */

    if ((pro=malloc(sizeof(struct _proto)))==NULL)
      logit(mem_none);
    else
    {
      char cmd[PATHLEN];

      /* Find the protocol in protocols.max */

      if (FindProtocol(protocol, pro))
      {
        zstr old_uppath;
        char **ppszDupeNames;

        /* Allocate an array to store the real file names being
         * transferred.  This is only used if we are staging files
         * and sending them from somewhere other than their regular
         * path.
         */

        if ((ppszDupeNames = malloc(FileEntries() * sizeof(char *)))==NULL)
          logit(mem_none);
        else
        {
          int nostage=FALSE;
          FENTRY fent;
          int iToSend = FileEntries();

          memset(ppszDupeNames, 0, iToSend * sizeof(char *));

          /* Stage all of the files before the transfer starts */

          for (fn=0; fn < iToSend && GetFileEntry(fn, &fent); fn++)
          {
            if (fent.fFlags & FFLAG_STAGE)
            {
              if (nostage)
                fent.fFlags &= ~FFLAG_STAGE;
              else
              {
                if (!StageFile(fent.szName, temp, fn==0))
                {
                  nostage=TRUE;
                  fent.fFlags &= ~FFLAG_STAGE;
                }
                else
                {

                  /* Create a copy of the old (non-staged) filename,
                   * and then copy the staged filename into the temp
                   * array.*/

                  ppszDupeNames[fn] = strdup(fent.szName);
                  fent.szName=temp;
                }
              }

              UpdFileEntry(fn, &fent);
            }
          }

          /* Write the control file */

          MaxWriteCtl(pro, FALSE);

          /* Execute the command */

          last_protocol=protocol;

          if (pro->flag & P_ERL)
            sprintf(cmd, "%d %s", prm.protoexit, pro->dlcmd);
          else
            strcpy(cmd, pro->dlcmd);

          /* Save the original upload path */

          old_uppath=fah.fa.uppath;

          if (!newuppath)
            newuppath="q:\\asdfghjk.zxc\\q";

          /* Temporarily use the new upload path */

          if (fah.heap)
            strcpy(fah.heap + (fah.fa.uppath=fah.fa.cbHeap), newuppath);

          Outside(NULL, NULL,
                  (pro->flag & P_ERL) ? OUTSIDE_ERRORLEVEL : OUTSIDE_RUN,
                  cmd, FALSE, CTL_DOWNLOAD,
                  RESTART_MENU, mname);


          /* Restore the old upload path */

          fah.fa.uppath=old_uppath;


          /* Read the log file */

          gotfile=MaxReadLog(pro, FALSE);

          /* Delete any staged files */

          /* Now that the log file has been read and the staged files have
           * been deleted, we can restore the real file names back to the
           * download list for logging purposes.
           */

          for (fn=0; GetFileEntry(fn, &fent); fn++)
          {
            if (fent.fFlags & FFLAG_STAGE)
              UnstageFile(fent.szName);

            /* Check to see if we are to unstage this file.  However,
             * ensure that we only check as many dupe file entries as
             * we had originally allocated. (In the case where the user
             * does a bidirectional file transfer, the value of
             * FileEntries() here could exceed what it was when we
             * originally allocated ppszDupeNames.)
             */

            if (fn < iToSend && ppszDupeNames[fn])
            {
              fent.szName=ppszDupeNames[fn];
              UpdFileEntry(fn, &fent);
              free(ppszDupeNames[fn]);
              ppszDupeNames[fn] = 0;
            }
          }

          free(ppszDupeNames);
        }
      }
      
      free(pro);
    }

    for (fn=0; GetFileEntry(fn, &fent); fn++)
      if (fent.fFlags & FFLAG_SENT)
        MaxSentFile(fn, TRUE, -1L);

    for (fn=0; GetFileEntry(fn, &fent); fn++)
      if (fent.fFlags & FFLAG_GOT)
      {
        File_Process_Uploads(0L, protocol, FAS(fah, uppath));
        break;
      }

    /* If there were no uploads, log the status of the transfer ourselves */

    if (flag && fn==FileEntries())
    {
      if (gotfile)
      {
        Puts(xfercomplete);
        Putc('\n');
      }
      else Puts(xferaborted);
    }

  }
  else
  {
    if (!ZmInitStatics() ||
        (cr3tab=mkcrc32tab())==NULL ||
        (crctab=mkcrc16tab())==NULL)
    {
      if (crctab)
        (free)(crctab);

      if (cr3tab)
        (free)(cr3tab);
    }
    else
    {

      int iFilesLeft=(int)FileEntries();
      long lBytesLeft=(long)realbytes;

      protocol_letter=Get_Protocol_Letter(protocol);
      xferstatus=TRUE;

      Lputc('\n');

      fn=0;   /* Start with the first filename */

      XferWinOpen(protocol, TRUE);


      if (protocol==PROTOCOL_ZMODEM)
      {
        if (ZmodemSendFile(NULL, TRUE, (long)iFilesLeft, lBytesLeft) != OK)
        {
          fn=FileEntries();
          xferstatus=FALSE;
        }
      }


      /* Now loop through each filename given and send it */

      for (; GetFileEntry(fn, &fent); fn++)
      {
        int fStaged=FALSE;
        long start;


        /* Check the time to make sure the user is pushed off within the
         * time limit if the current file does not have 'free time'. This
         * might happen if throughput is particularly bad on previous files.
         */

        if (!(fent.fFlags & FFLAG_NOTIME) &&
            do_timecheck && !acsflag(CFLAGA_NOTIME) &&
            (timeleft()-(int)(XferTime(protocol,fent.ulSize)/60L)) <= 0)
          break;

        /* Copy the file to a staging area, if necessary */

        if (fent.fFlags & FFLAG_STAGE)
          fStaged=StageFile(fent.szName, temp, TRUE);

        if (!fStaged)
          strcpy(temp, fent.szName);

        /* If we're doing a batch transfer but not using a batch protocol,  *
         * print out the name of each file before we send it.               */

        if (flag && FileEntries() && !IsBatch(protocol))
        {
          Puts(down_fnam);
          Puts(No_Path(fent.szName));
          Puts(GRAY "\n");
        }

        start=time(NULL);

        if (protocol==PROTOCOL_ZMODEM)
        {
          #define SPEC_COND       0x4242u

          int ch;

          /* Transmit a single file to the user */

          ch=ZmodemSendFile(temp,
                            FALSE,
                            (long)iFilesLeft,
                            lBytesLeft);

          /* Leech Zmodem check */

          if (ch==-100)
            result=-100;
          else if (ch==ERROR)
            result=FALSE;
          else if (ch==ZSKIP)
            result=SPEC_COND;
          else
            result=TRUE;

          if (flag && result && result != SPEC_COND)
          {
            ThruLog(fent.ulSize);
            logit(log_dl, 'Z', fent.szName);
          }

          XferWinClear();
        }
        else
        {
          result=XmTx(temp, protocol, FileEntries()-fn, realbytes, fent.szName);
          XferWinClear();
        }


        /* Delete the staged file */

        if (fStaged)
          UnstageFile(temp);

        if (result)
        {
          fent.fFlags |= FFLAG_SENT;
          UpdFileEntry(fn, &fent);
        }
        else
        {
          /* If the transfer was bad */

          xferstatus=FALSE;
          if (flag)
          {
            Delay(50);
            logit(lxferaborted);
            Puts(xferaborted);
          }
          break;
        }

        /* Subtract the number of bytes that we just sent */

        realbytes -= fent.ulSize;

        /* Otherwise, it must have been okay, so debit download ctrs.
         * (If we're using Zmodem, don't debit downloads if we got
         * a ZSKIP header!)
         */

        if (flag && (protocol==PROTOCOL_ZMODEM && result != SPEC_COND) ||
            protocol != PROTOCOL_ZMODEM)
        {
          start=time(NULL)-start;

          MaxSentFile(fn, FALSE, start);
        }

        /* Leech Zmodem fix */

        if (result==-100)
        {
          logit("!Attempted Leech Zmodem download!");
          break;
        }

        /* Decrement the file and byte count, but make sure that they
         * do not go below zero!
         */

        if (--iFilesLeft < 0)
          iFilesLeft=0;

        if ((lBytesLeft -= fent.ulSize) < 0)
          lBytesLeft=0;

      }

      if (xferstatus) /* End batch transfer! */
      {
        if (protocol==PROTOCOL_ZMODEM)
        {
          /* Zmodem protocol cleanup */

          if (fn==FileEntries() && protocol==PROTOCOL_ZMODEM)
            ZmodemSendFile(NULL, FALSE, 0L, 0L);
        }
        else
        {
          result=XmTx(NULL, protocol, 0, 0, NULL);
        }

        Delay(protocol==PROTOCOL_ZMODEM ? 150 : 50);

        if (flag)
        {
          Puts(xfercomplete);

          if (!last_bps)
            Putc('\n');
          else
          {
            Printf(cps_rating,
                   (long)(last_bps/10L),
                   last_bps*100L/(long)baud);
          }
        }
      }

      XferWinClose();

      (free)(cr3tab);
      (free)(crctab);
      ZmDeinitStatics();
    }
  }

  /* Let's try to salvage any unsent tagged files */

  for (fn=0; GetFileEntry(fn, &fent); fn++)
  {
    if ((fent.fFlags & (FFLAG_SENT|FFLAG_GOT)))
    {
      RemoveFileEntry(fn);
      fn--;
    }
  }

  if (FileEntries()==0)
    Free_Filenames_Buffer(0);
  
  mdm_dump(DUMP_INPUT);
  Clear_KBuffer();

  /* And turn flow control back on! */
  
  Mdm_Flow_On();

  in_file_xfer=FALSE;
  return fsent;
}



word File_Send_Files(sword protocol, char *mname, char *newuppath, int flag)
{
  char pname[PATHLEN];
  char *s;
  long realbytes;
  long realtime;                  /* Protocol time (real) */
  word fn;                      /* The file number we're working on */

  FENTRY fent;

  fsent=0;

  /* "File: " */

  Puts(down_fnam);

  /* Now display all of the file names to send */

  for (fn=0; GetFileEntry(fn, &fent); fn++)
  {
    /* Don't show the path specification, if it exists */

    s=No_Path(fent.szName);
    
    /* Word-wrap the file names */
    
    if (current_col+strlen(s) >= TermWidth())
      Puts("\n\x19 \x06");
    
    Printf("%s ", cstrupr(s));
  }

  Putc('\n');

  /* Make sure that user has enough time/bytes/ratio to DL these files */

  if (! FileLimitsOkay(0L, 0, protocol, &realbytes, &realtime))
    return 0;
  

  /* "Size: xxx bytes (xxx Xmodem blocks)" */
  
  Printf(down_fsiz, realbytes, (realbytes/128L)+1);


  /* "Time: 12:34 minutes (estimated)" */
  
  Printf(down_ftim, (word)(realtime / 60L), (word)(realtime % 60L));


  /* Mode: Zmodem */
  
  Printf(down_fmode, Protocol_Name(protocol, pname));


  /* If we need to, add a "wait 10 seconds to download" prompt */

  if (flag && !Shall_We_Continue(10, method_download))
    return fsent;

  /* Start receiving now, or ... */
  
  Puts(start_receive);

  if (!local)
    fsent=File_Send_Files_Sub(protocol, mname, newuppath, realbytes, 1);
  else Free_Filenames_Buffer(0);

  return fsent;
}



/* FileLimitsOkay
 *
 * Routine to determine if the file to be added, with size 'ulSize' and flags
 * 'flags', would break our download limit check.
 */

static int near FileLimitsOkay(unsigned long ulSize, int flags, sword protocol,
                               long *realbytes, long *prtime)
{
  long fsz;
  long virtbytes;
  long virttbytes;
  long virtkbs;
  long pvtime;
  int fn;

  FENTRY fent;
  
  for (fn=0, *realbytes=virtbytes=virttbytes=0L;
       fn < FileEntries() && GetFileEntry(fn, &fent);
       fn++)
  {
    /* Count the number of bytes in this file, using appropriate method
     * for fast/slow areas.
     */
    
    if (fent.fFlags & FFLAG_SLOW)
      fsz=fent.ulSize;
    else
      fsz=fsize(fent.szName);

    /* "Virtual" bytes is the count we use for checking time limits et al */

    if ((fent.fFlags & FFLAG_NOBYTES)==0)
      virtbytes += fsz;
    
    if ((fent.fFlags & FFLAG_NOTIME)==0)
      virttbytes += fsz;

    /* Realbytes is the actual number of bytes */

    *realbytes += fsz;
  }

  /* Now perform that processing for the file to be added too */

  if ((flags & FFLAG_NOBYTES)==0)
    virtbytes += ulSize;

  if ((flags & FFLAG_NOTIME)==0)
    virttbytes += ulSize;

  *realbytes += ulSize;


  /* Finally, calculate transfer times */

  virtkbs=virtbytes/1024L;

  *prtime=XferTime(protocol, *realbytes);
  pvtime=XferTime(protocol, virttbytes);
  

  if (!acsflag(CFLAGA_NOLIMIT))
  {

    /* If this request would exceed the user's daily download limit */

    if (virtkbs && (long)usr.downtoday+virtkbs > ClassGetInfo(cls,CIT_DL_LIMIT))
    {
      ci_dlexceed();
      logit(log_dllim);
      Puts(exc_daily_lim);
      Display_File(0, NULL, "%sexcbytes", PRM(misc_path));
      return FALSE;
    }

    /* If this would exceed the user's ratio... */
  
    if (virtkbs && (dword)(usr.down+virtkbs) > (dword)ClassGetInfo(cls,CIT_FREE_RATIO) &&
        ClassGetInfo(cls,CIT_RATIO) &&
        (usr.down+virtkbs) > (dword)ClassGetInfo(cls,CIT_RATIO) * usr.up)
    {
      ci_dlexceed();
      logit(log_exc_ratio);
      Printf(exc_ratio, ClassGetInfo(cls,CIT_RATIO));
      Display_File(0, NULL, "%sexcratio", PRM(misc_path));
      return FALSE;
    }
  }

  if (!acsflag(CFLAGA_NOTIME))
  {
    /* Now make sure that it won't exceed the user's time limit */

    if ((dword)(time(NULL)+pvtime) > timeoff ||
        (dword)(time(NULL)+*prtime) > getoff)
    {

      /* If the time to transfer all of the non-free files (virtual           *
       * number of bytes transferred) would exceed the session time limit,    *
       * or if we'd run over the '-t' parameter by transferring all of        *
       * these files (virtual or not), give an error message.                 */

      Puts(exc_time_limit);
      Display_File(0, NULL, "%sexctime", PRM(misc_path));
      return FALSE;
    }
  }
  
  return TRUE;
}




