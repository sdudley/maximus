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
static char rcs_id[]="$Id: f_up.c,v 1.1.1.1 2002/10/01 17:51:12 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: U)pload command and associated functions
*/

#define MAX_LANG_f_area
#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>

#include "zsjd.h"

#ifdef BINK_PROTOCOLS
  #include "zmodem.h"
#endif

#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"
#ifdef BINK_PROTOCOLS
  #include "prototyp.h"
#endif
#include "f_idx.h"
#include "f_up.h"
#include "xmodem.h"

#ifndef BINK_PROTOCOLS
  #include "zsjd.h"
  #include "pdata.h"
#endif


static word near FileIsBad(char *name);
static int near Check_Filename(char *ulpath,char *fname);
static void near Add_Upload_Bytes(long fsize);
static void near ParseXferInfo(void);
static void near FBLog(void);
/*static char * near comma_number(dword n);*/



void File_Upload(char *mname)
{
  struct _css css;
  byte temp[BUFLEN];
  sword protocol;
  long ul_start_time;
  unsigned long b_free;

  if (! *FAS(fah, uppath) || !direxist(FAS(fah, uppath)))
  {
    sprintf(temp, "%sNOUPLOAD.BBS", FAS(fah, downpath));

    if (fexist(temp))
      Display_File(0,NULL,temp);
    else Puts(noupl);      

    return;
  }
  
  if (!local && baud < ClassGetInfo(cls,CIT_MIN_XFER_BAUD))
  {
    Display_File(0, NULL, PRM(xferbaud));
    Printf(baudtoolowforxfer, ClassGetInfo(cls,CIT_MIN_XFER_BAUD));
    Press_ENTER();
    return;
  }

  b_free=zfree(FAS(fah, uppath));

  if (prm.k_free && b_free < (unsigned long)((unsigned long)prm.k_free*1000Lu))
  {
    Display_File(0, NULL, PRM(no_space));
    return;
  }

  Printf(bytes_for_ul, commaize(b_free-(long)prm.k_free*1000L, temp));

  if (local)
  {
    Puts(err999_1);
    InputGets(temp,err999_2);
    return;
  }

  if (File_Get_Protocol(&protocol, FALSE, FALSE)==-1)
    return;

  ChatSaveStatus(&css);
  ChatSetStatus(FALSE, cs_file_xfer);

  save_tag_list(NULL);
  Free_Filenames_Buffer(0);

  if (IsBatch(protocol) || (File_Get_Upload_Names(), FileEntries()))
  {
    ul_start_time=time(NULL);
    File_Get_Files(protocol, mname, FAS(fah, uppath));
    File_Process_Uploads(ul_start_time, protocol, FAS(fah, uppath));
  }
  else Puts(xferaborted);

  ChatRestoreStatus(&css);
  Free_Filenames_Buffer(0);
  restore_tag_list(NULL,FALSE);
}




void File_Get_Upload_Names(void)
{
  byte inp[PATHLEN];
  byte testbuf[PATHLEN];

  Free_Filenames_Buffer(0);

  InputGetsLL(inp, PATHLEN-1, file_ul);
  Strip_Path(inp);

  /* If we're uploading, so check for a valid filename */

  getword(inp, testbuf, dot, 2);

  if (strlen(inp) > MAX_FN_LEN || strlen(testbuf) > 3 ||
      strpbrk(inp," |<>")!=NULL)
  {
    Display_File(0,NULL,PRM(fname_format));
    return;
  }
  
  if ((prm.flags2 & FLAG2_CHECKDUPE) && FileIsDupe(inp))
  {
    Printf(dupe_file, upper_fn(inp));
    Press_ENTER();
    return;
  }

  if (!acsflag(CFLAGA_ULBBSOK) && FileIsBad(inp))
    return;
  
  if (*inp)
    AddFileEntry(inp, 0, 0);
}




/* Checks the compiled file database to determine whether or not a certain  *
 * file is a duplicate.                                                     */

word FileIsDupe(char *name)
{
  IDXF *ix;

  char newname[PATHLEN];
  char *p;

  FDAT fdat;
  word ret=FALSE;
  

  strcpy(newname, name);


  /* Strip off the extension, if necessary */

  if ((prm.flags2 & FLAG2_CHECKEXT)==0)
  {
    /* Strip off any dots in the filename */

   if ((p=strrchr(newname, '.')) != NULL)
      *p='\0';
  
    strcat(newname, ".*");
  }


  if ((ix=IndexOpen(newname))==NULL)
    return FALSE;
  
  if (FidxIsOkay(&ix->fidx, &fdat, NULL, NULL, TRUE, FALSE, NULL))
    ret=TRUE;

  IndexClose(ix);
  return ret;
}


/* Checks the 'bad uploads' list to see if the file is listed as unwanted  */

static word near FileIsBad(char *name)
{
  word ret;
  FILE *fp;
  char badname[PATHLEN];

  ret=FALSE;
  sprintf(badname, "%sbadfiles.bbs", PRM(sys_path));

  if ((fp=shfopen(badname, fopen_read, O_RDONLY))!=NULL)
  {
    while (ret==FALSE && fgets(badname, PATHLEN-1, fp)!=NULL)
    {
      char *p=badname+strspn(badname, " \t");

      if (*p && *p != ';')
      {
        cstrlwr(p);

        for (p=strtok(badname, cmd_delim);
             ret==FALSE && p;
             p=strtok(NULL, cmd_delim))
        {
          ret=MatchWC(p, name);
        }
      }
    }

    fclose(fp);

    if (ret && carrier())
    {
      sprintf(badname, ss, PRM(misc_path), "bad_upld.bbs");

      if (fexist(badname))
        Display_File(0, NULL, badname);
      else
        Printf(badupload, upper_fn(name));

      Press_ENTER();
    }
  }

  return ret;
}




word File_Get_Files(sword protocol, char *mname, char *path)
{
  extern word *crctab;
  extern dword *cr3tab;
  byte save_tc;

  FILE *xferinfo;

  char temp[PATHLEN];
  char *filename;

  word fn, ok;

  FENTRY fent;


  Printf(mode_bg, Protocol_Name(protocol,temp));
  AlwaysWhiteN();

  last_bps=0;
  save_tc=do_timecheck;
  do_timecheck=FALSE;

  /* Force logit() calls to be displayed on-screen */
  in_file_xfer=TRUE;

  filename=NULL;

  if (IsBatch(protocol))
    Free_Filenames_Buffer(0);

  logit(log_getting_from, usr.name, baud);

  /* Turn off ^S/^Q flow control and ^C/^K checking, so it doesn't screw    *
   * up the transfers!                                                      */

  Open_OpusXfer(&xferinfo);

  Mdm_flush();
  Mdm_Flow_Off();

  ul_no_space=FALSE;

  if (protocol >= 0)
  {
    struct _proto *pro;

    /* Get the letter of the protocol to use */

    protocol_letter=*Protocol_Name(protocol,temp);


    /* Allocate space for the protocol structure */

    if ((pro=malloc(sizeof(struct _proto)))==NULL)
      logit(mem_none);
    else
    {
      char cmd[PATHLEN];
      zstr old_uppath;

      /* Find the protocol in protocols.max */

      if (FindProtocol(protocol, pro))
      {

        if (GetFileEntry(0, &fent))
        {
          char fname[PATHLEN];

          /* Shift the filename to make room for the path */

          sprintf(fname, ss, path, No_Path(fent.szName));
          fent.szName=fname;
          UpdFileEntry(0, &fent);
        }
        else
        {
          /* If no specific filename, just send the path */

          AddFileEntry(path, 0, 0);
        }
          
        /* Write the control file */
        
        MaxWriteCtl(pro, TRUE);

        /* Execute the command */
        
        last_protocol=protocol;
        
        if (pro->flag & P_ERL)
          sprintf(cmd, "%d %s", prm.protoexit, pro->ulcmd);
        else strcpy(cmd, pro->ulcmd);

        /* Save a copy of this area's upload path, and insert the current   *
         * QWK upload path instead.  The code for                           */

        old_uppath=fah.fa.uppath;
        strcpy(fah.heap + fah.fa.cbHeap, path);
        fah.fa.uppath=fah.fa.cbHeap;

        Outside(NULL, NULL, 
                (pro->flag & P_ERL) ? OUTSIDE_ERRORLEVEL : OUTSIDE_RUN,
                cmd, FALSE, CTL_UPLOAD,
                RESTART_MENU, mname);

        Free_Filenames_Buffer(0);

        /* Read the log file */

        MaxReadLog(pro, TRUE);

        fah.fa.uppath=old_uppath;
      }


      /* Handle any files downloaded during the same session too */

      for (fn=0; GetFileEntry(fn, &fent); fn++)
        if (fent.fFlags & FFLAG_SENT)
          MaxSentFile(fn, TRUE, -1L);

      free(pro);
    }
  }
#ifdef BINK_PROTOCOLS
  else if ((Txbuf=Secbuf=zalloc())==NULL ||
           (cr3tab=mkcrc32tab())==NULL ||
           (crctab=mkcrc16tab())==NULL)
  {
    if (Txbuf)
    {
      if (cr3tab)
        (free)(cr3tab);
      
      free(Txbuf);
    }
  }
#else /* !BINK_PROTOCOLS */
  else if (!ZmInitStatics() ||
           (cr3tab=mkcrc32tab())==NULL ||
           (crctab=mkcrc16tab())==NULL)
  {
    if (crctab)
      (free)(crctab);

    if (cr3tab)
      (free)(cr3tab);
  }
#endif
  else
  {
    XferWinOpen(protocol, FALSE);

    if (protocol==PROTOCOL_ZMODEM)
    {
#if 1
      int ch;
      int fFirst=TRUE;

      while ((ch=ZmodemRecvFile(path, temp, fFirst))==ZEOF || ch==ZSKIP)
      {
        /* We ran out of space to receive if we ZSKIPped but no dupe */

        int fNoSpace=(ch==ZSKIP && *temp != '*');

        fFirst=FALSE;

        if (!fNoSpace && *temp)
        {
          if (*temp=='*')
            AddFileEntry(temp+1, FFLAG_GOT | FFLAG_DUPE, -1);
          else
          {
            AddFileEntry(temp, FFLAG_GOT, -1);
            ThruLog(fsize(temp));
            logit(log_ul, 'Z', blank_str, temp);
          }
        }

        XferWinClear();
      }

/*      logit("@Zmodem loop done");*/

#elif defined(BINK_PROTOCOLS)
      get_Zmodem(path, xferinfo);
#else
      int ch;

      ZMsg(M_RESET);

      ZmodemRecv(path, NULL);

      while ((ch=ZmodemRecv(path, temp))==ZEOF || ch==ZSKIP)
      {
        if (*temp)
        {
          if (*temp=='*')
            AddFileEntry(temp+1, FFLAG_GOT | FFLAG_DUPE, -1);
          else
          {
            AddFileEntry(temp, FFLAG_GOT, -1);
            ThruLog(size);
            logit(log_ul, 'Z', blank_str, temp);
          }
        }

        XferWinClear();
      }

      if (ch >= 0)
        ZmodemRecv(NULL, NULL);
#endif
    }
    else
    {
      unsigned eob;

      do
      {
        /* Filename to receive */

        *temp='\0';
        if (protocol==PROTOCOL_XMODEM || protocol==PROTOCOL_XMODEM1K)
        {
          if (GetFileEntry(0, &fent))
            strcpy(temp, fent.szName);
        }

        XferWinClear();
        ok=XmRx(path, temp, protocol, &eob);

        if (ok && *temp)
        {
          int iFlag = FFLAG_GOT;
          char *psz = temp;

          if (*temp=='*')
          {
            psz++;
            iFlag |= FFLAG_DUPE;
          }

          if (IsBatch(protocol))
            AddFileEntry(psz, iFlag, -1);
          else
          {
            if (GetFileEntry(0, &fent))
            {
              fent.fFlags |= iFlag;
              UpdFileEntry(0, &fent);
            }
          }

          if (*temp=='*')
            break;
        }
      }
      while (ok && !eob); /* while received successfully and not end of batch */
    }

    XferWinClose();

#ifdef BINK_PROTOCOLS
    free(Txbuf);
    Txbuf=Secbuf=NULL;
#endif
    ZmDeinitStatics();
    (free)(cr3tab);
    (free)(crctab);
  }
  
  /* Now strip off all of the path specifications from the upload filespecs */
  
  for (fn=0; GetFileEntry(fn, &fent); fn++)
  {
    if (!(fent.fFlags & FFLAG_DUPE))
    {
      strcpy(temp, No_Path(fent.szName));
      fent.szName=temp;
      UpdFileEntry(fn, &fent);
    }
  }

  /* In case anything is left over from a file transfer... */

  mdm_dump(DUMP_INPUT);
  Clear_KBuffer();

  /* And turn flow control back on! */

  Mdm_Flow_On();

  in_file_xfer=FALSE;

  Close_OpusXfer(&xferinfo);

  /* Wait for user to finish up */

  Delay(200);

#ifdef BINK_PROTOCOLS
  if (protocol==PROTOCOL_ZMODEM)  
    ParseXferInfo();
#endif

  Delete_OpusXfer(&xferinfo);
  do_timecheck=save_tc;

  return ((word)FileEntries());
}



#ifdef BINK_PROTOCOLS

static void near ParseXferInfo(void)
{
  FILE *xferinfo;
  char temp[PATHLEN];
  char *p;
  
  sprintf(temp, opusxfer_name, original_path, task_num);

  Free_Filenames_Buffer(0);

  if ((xferinfo=shfopen(temp, fopen_read, O_RDONLY | O_NOINHERIT))==NULL)
  {
    logit(cantfind,temp);
    return;
  }

  while (fgets(temp, PATHLEN, xferinfo) && CanAddFileEntry())
  {
    Trim_Line(temp);

    /* Bitch if a duplicate file was sent */

    if (*temp == '*')
    {
      Printf(dupe_file, upper_fn(temp+1));
      Press_ENTER();
      continue;
    }

    /* Chop off path */
    if ((p=strrchr(temp,'\\')) != NULL || (p=strrchr(temp,'/')) != NULL)
      p++;
    else p=temp;

    if (p && *p)
      AddFileEntry(p, FFLAG_GOT, -1L);
  }

  fclose(xferinfo);
}

#endif


void File_Process_Uploads(long ul_start_time, sword protocol, char *path)
{
  FFIND *ff;
  
  long temp_long;

  byte temp[PATHLEN];
  byte save_tc;
  word uploads=FALSE;

  FENTRY fent;
  
  word fn, fn2, dupes;
  
  NW(protocol);
  

  save_tc=do_timecheck;
  do_timecheck=FALSE;

  if (ul_no_space)
    Display_File(0,NULL,PRM(no_space));

  for (dupes=fn=fn2=0; GetFileEntry(fn, &fent); fn++)
  {
    if (fent.fFlags & FFLAG_GOT)
    {
      ++fn2;

      if (fent.fFlags & FFLAG_DUPE)
        ++dupes;
    }
  }
      
  /* If we got at least one file... */

  if (fn2 || dupes)
  {
    /* Even if user *did* drop carrier, we want to add these to the       *
     * FILES.BBS anyway, so they don't get lost and hog the Sysop's disk  *
     * space!                                                             */

    if (carrier())
    {
      Delay(50);

      Puts(xfercomplete);

      if (last_bps)
        Printf(cps_rating,(long)(last_bps/10L),(long)(last_bps*100L/(long)baud));
      else Putc('\n');

      if (fn2 && (fn2 != dupes))
        Printf(tnx4ul,firstname);
    }

    /* Now find out what/how much user sent!  Screen out files with a    *
     * .BBS or .GBS extension, to prevent trojan .BBS file uploads!      */

    for (fn=0; GetFileEntry(fn, &fent); fn++)
    {
      if (fent.fFlags & FFLAG_GOT)
      {
        if (fent.fFlags & FFLAG_DUPE)
        {
          Printf(dupe_file, upper_fn(fent.szName));
          Press_ENTER();
        }
        else
        {
          if (!acsflag(CFLAGA_ULBBSOK) && FileIsBad(fent.szName))
          {

            /* We can safely remove it as an unwanted upload */

            sprintf(temp, ss, path, fent.szName);
            unlink(temp);

            /* Lie and say we didn't really get it */

            fent.fFlags &= ~FFLAG_GOT;
            UpdFileEntry(fn, &fent);
          }
          else if (Check_Filename(path, fent.szName))
            UpdFileEntry(fn, &fent);
        }
      }
    }

    /* Only do this if a user is on-line -- Otherwise, skip over it.     *
     * Even if user did drop carrier, we want to make sure to write      *
     * the file names into FILES.BBS...                                  */

    if (carrier() && ul_start_time)
    {
      /* Figure out how many seconds the user was uploading for... */
      temp_long=time(NULL)-ul_start_time;

      /* Catch any divide-by-zero errors... */

      if (temp_long != 0)
      {
        long lReward = ClassGetInfo(cls, CIT_UPLOAD_REWARD);

        /* Now factor in the upload "reward" */

        if (lReward)
        {
          temp_long=Add_To_Time((lReward * temp_long) / 100L);

          if (temp_long)
            Printf(time_added_for_ul,
                   (long)(temp_long / 60L),
                   (long)(temp_long % 60L));
        }
      }
    }

    /* Now verify all of the files ULed, and prompt for descriptions */

    for (fn=0; GetFileEntry(fn, &fent); fn++)
    {
      if ((fent.fFlags & FFLAG_GOT) && !(fent.fFlags & FFLAG_DUPE))
      {
        uploads=TRUE;

        sprintf(temp, ss, path, fent.szName);

        /* If we found it */

        if ((ff=FindOpen(temp, 0))==NULL)
          logit(cantfind, temp);
        else
        {
          fent.ulSize=ff->ulSize;

          Add_To_Upload_Log(path, ff->szName, ff->ulSize);

          if (! LookForVirus(path, ff->szName))
          {
            Add_Upload_Bytes(ff->ulSize);
            Get_File_Description(ff->szName, ff->ulSize,
                                 fent.szDesc ? fent.szDesc : blank_str);
          }
        }
        
        fent.szDesc=NULL;
        FindClose(ff);
        UpdFileEntry(fn, &fent);
      }
    }

    if (uploads)
      FBLog();
  }
  else
  {
    Delay(50);
    logit(lxferaborted);
    Puts(xferaborted);
  }

  do_timecheck=save_tc;
}


static void near Add_Upload_Bytes(long fsize)
{
  long kbs=fsize/1024L;

  usr.nup++;
  usr.up += kbs;
  ci_upload(kbs);
  ultoday += kbs;
  bstats.total_ul += kbs;
}


static int near Check_Filename(char *ulpath,char *fname)
{
  char *p;

  char from[PATHLEN];
  char to[PATHLEN];

   
  if (!(acsflag(CFLAGA_ULBBSOK)) &&
      ((p=stristr(fname, dotbbs)) != NULL ||
       (p=stristr(fname, dotgbs)) != NULL ||
       (p=stristr(fname, dotrbs)) != NULL) ||
       (p=stristr(fname, filesdat_name)) != NULL ||
       (p=stristr(fname, filesdmp_name)) != NULL ||
       (p=stristr(fname, filesidx_name)) != NULL)
  {
    /* Save the original filename... */

    sprintf(from, ss, ulpath, fname);


    /* Change *(p+3) (the 's' of ".bbs") to an 'x' */

    p[3]='x';


    /* Copy this name into to, with the required path */

    sprintf(to, ss, ulpath, No_Path(fname));


    /* Increment the extension until no more dupes */

    unique_name(to);


    /* Actually do the rename call... */

    move_file(from, to);


    /* And we're done! */
    
    logit(ul_renamed, from, to);
    return TRUE;
  }
  return FALSE;
}


/* Add an upload to the FB log */

static void near FBLog(void)
{
  char temp[PATHLEN];

  #if defined(__MSDOS__) || defined(NT)
    sprintf(temp, "%srunfb.bat", original_path);
  #elif defined(OS_2)
    sprintf(temp, "%srunfb.cmd", original_path);
  #else
    #error Unknown batch file extension!
  #endif

  if (fexist(temp))
  {
    Puts(please_wait);
    
    sprintf(temp, "runfb %s %s -u", PRM(farea_name), FAS(fah, name));
    Outside(NULL, NULL, OUTSIDE_DOS, temp, FALSE, CTL_NONE, 0, NULL);
    
    Puts(YELLOW);
    Putc(' ');
    Puts(done_ex);
  }
}

#if 0
static char * near comma_number(dword n)
{
  static char cbuf[20];
  char *s;
  char ch;

  sprintf(cbuf, "%lu", n);

  if (strlen(cbuf) <= 3)
    return cbuf;

  for (s=cbuf+strlen(cbuf)-3; s > cbuf; s -= 3)
  {
    strocpy(s+1, s);

    ch=*number_comma;
    *s=ch ? ch : ',';
  }

  return cbuf;
}
#endif

