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
static char rcs_id[]="$Id: max_xtrn.c,v 1.8 2003/12/16 12:31:35 paltas Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <process.h>
#include <errno.h>
#include "prog.h"
#include "mm.h"
#include "max_msg.h"
#include "max_file.h"
#include "dr.h"

#ifdef __WATCOMC__
#include <malloc.h> /* for _heapshrink() and _heapgrow() */
#endif

#ifdef SWAP
#include "../swap2/exec.h"
#endif

static void near Close_Files(void);
static void near Reopen_Files(void);
static void near Out_Save_Directories(int stay);
static void near Out_Disable_Ctrlc(void);
static void near Out_Video_Down(void);
static void near Out_Video_Up(void);
static void near Out_Reinstall_Fossil(void);
static void near Outside_Reread(int tr, int tonline);
static char * near Add_Task_Number(char *fname,char *new);
static void near Write_External_Ctlfile(int ctltype,char *parm,int method);
static int near Form_Outside_Filename(char *in,char *out,int method);


#ifndef ORACLE

#ifdef SWAP

char *max_swap_filename;

int _fast fexist_maybehidden(char *filename)
{
  FFIND *ff;

  ff=FindOpen(filename, ATTR_HIDDEN | ATTR_SYSTEM);

  if (ff)
  {
    FindClose(ff);
    return TRUE;
  }
  else return FALSE;
}

static char * near MakeFullPath(char *cmd)
{
  char this[PATHLEN];
  char progname[PATHLEN];
  char *path=getenv("PATH");
  char *try;
  char *newpath;
  char *rc;
  char *s;
  word last=TRUE;
  
  
  /* The name of the program to run is the first word of the command */
  
  getword(cmd, progname, " ;", 1);
  

  /* If no path set, use the current directory */

  newpath=strdup(path ? path : ".");

  if (!newpath)
    return NULL;


  /* Now scan all directories on the path */

#ifdef UNIX
  for (s=strtok(newpath, " ;"); s || last; s=strtok(NULL, " ;"))
#else
  for (s=strtok(newpath, ":"); s || last; s=strtok(NULL, ":"))
#endif
  {
    if (s)
    {
      (void)strcpy(this, s);
      Add_Trailing(this, PATH_DELIM);
    }
    else
    {
      (void)strcpy(this, "");
      last=FALSE;
    }
    
    (void)strcat(this, progname);

    try=strdup(this);

    if (!try)
    {
      free(newpath);
      return NULL;
    }
    
#ifndef UNIX
    if (!fexist_maybehidden(this))
    {
      (void)strcpy(this, try);
      (void)strcat(this, ".com");
      
      if (!fexist_maybehidden(this))
      {
        (void)strcpy(this, try);
        (void)strcat(this, ".exe");

        if (!fexist_maybehidden(this))
        {
          /* doesn't exist, so search next dir on path */
          free(try);
          *this='\0';
          continue;
        }
      }
    }
#endif

    /* Now tack on the rest of the command */
        
    s=firstchar(cmd, " ;", 2);
    
    (void)strcat(this, " ");
    (void)strcat(this, s ? s : "");
    
    free(try);
    break;
  }
  
  free(newpath);
  
  rc = malloc(strlen(this) + 2);
  strcpy(rc, this);
  return rc;
}


/* Call the swapper with a space-delimited ASCIIZ argument list */

static int near swapsz(char *szCmd)
{
  int rc;
  static char szSwapName[15]; /* must be static for swap to work properly */
  char *szRun;
  char *s;

  /* Put the full path to the .exe at the beginning of the string */

  szRun=MakeFullPath(szCmd);

  if (!szRun)
  {
    errno=ENOMEM;
    return -1;
  }

  /* Ensure there is an extra nul at the end */

  szRun[strlen(szRun)+1] = 0;

  /* Chop off the first argument and leave the rest as an argument string */

  s=strtok(szRun, " ");

  if (!s)
    s=szRun;



  /* Create a unique swap file for this task */

  sprintf(szSwapName, "~~~max%02x.~~~", task_num);
  max_swap_filename = szSwapName;

  rc = do_exec(s, s+strlen(s)+1, USE_EMS | USE_XMS | USE_FILE, 0xffff, NULL);

  if (rc >= 0x100)
  {
    if (rc >= 0x100 && rc <= 0x1ff)
      errno = ENOMEM;
    else if (rc >= 0x200 && rc <= 0x2ff)
      errno = ENOENT;
    else if (rc >= 0x300 && rc <= 0x3ff)
    {
      /* Translation of DOS error codes */

      switch (rc)
      {
        case 0x302:
        case 0x303:
        case 0x305:
          errno = ENOENT;
          break;

        case 0x308:
          errno = ENOMEM;
          break;

        default:
          errno = rc;
      }
    }
    else if (rc==0x502)
      errno = ENOSPC;
    else
      errno = 9999;

    rc=-1;
  }

  free(szRun);
  return rc;
}



/* Call the swapper with an array of ASCIIZ arguments */

static int near swapvp(char *szName, char **szArgs)
{
  char *szProg=malloc(PATHLEN*2);
  int rc;

  NW(szName);

  if (!szProg)
  {
    errno=ENOMEM;
    return -1;
  }

  *szProg=0;

  while (*szArgs)
  {
    strcat(szProg, *szArgs++);

    if (*szArgs)
      strcat(szProg, " ");
  }

  rc=swapsz(szProg);

  free(szProg);

  return rc;
}

#endif /* SWAP */



/* Return the name of the current command processor */

static char * near GetComspec(void)
{
  char *comspec;

  comspec=getenv("COMSPEC");

#ifdef UNIX
  if (!comspec)
    comspec=getenv("SHELL");
#endif

  if (!comspec)
#if defined(__MSDOS__)
    comspec="command.com";
#elif defined(OS_2) || defined(NT)
    comspec="cmd.exe";
#elif defined(UNIX)
    comspec="/bin/sh";
#else
    #error Unknown OS
#endif

  return comspec;
}

int Outside(char *leaving,char *returning,int method,char *parm,
            int slogan,int ctltype,char restart_type,
            char *restart_name)
{

  struct _css *css;
  int tonline;

  char **args=NULL, *temp, *temp2=NULL, *p, *s;

  sword erl, stay, reread, nofix;
  sword timeremain, x;

  byte save_inchat;
  long starttime;

#ifdef __MSDOS__
  int save_stdout, save_stdin;
  FILE *bat;
#else
  (void)restart_type;
  (void)restart_name;
#endif

  NW(slogan);

  /* Allocate some memory to work with. */

  if ((temp=malloc(PATHLEN*2))==NULL ||
      (temp2=malloc(PATHLEN*2))==NULL ||
      (args=malloc(MAX_EXECARGS*sizeof(char *)))==NULL ||
      (css=malloc(sizeof(struct _css)))==NULL)
  {
    if (temp)
    {
      free(temp);

      if (temp2)
      {
        free(temp2);

        if (args)
          free(args);
      }
    }

    return -1;
  }

  erl=-10;
  save_inchat=inchat;
  inchat=FALSE;
  stay=reread=nofix=FALSE;

  starttime=time(NULL);
  timeremain=timeleft();

#ifndef __MSDOS__
  /* Errorlevel exits not supported under OS/2 */

  if (method==OUTSIDE_ERRORLEVEL)
    method = OUTSIDE_RUN;
#endif

  ChatSaveStatus(css);

  if (!in_wfc)
    ChatSetStatus(FALSE, cs_outside);


  /* Copy bits into scalar, and then screen them off */

  if (method & OUTSIDE_STAY)
    stay=TRUE;

  if (method & OUTSIDE_REREAD)
    reread=TRUE;
  
  if (method & OUTSIDE_NOFIX)
    nofix=TRUE;

  method &= ~(OUTSIDE_STAY | OUTSIDE_REREAD | OUTSIDE_NOFIX);

  tonline=timeonline();

  if (!in_wfc)
  {
    /* Display LEAVING.BBS */

    display_line=display_col=1;

    if (leaving)
      Display_File(0,NULL,leaving);

    /* Fix up lastread pointers */

    if (method != OUTSIDE_ERRORLEVEL && !nofix)
      FixLastread(sq, mah.ma.type, last_msg, MAS(mah, path));

    /* Write LASTUSER.BBS */

    tonline=timeonline();

    Write_LastUser();

    /* Write RESTARxx.BBS */

#ifdef __MSDOS__
    if (method==OUTSIDE_ERRORLEVEL)
      Write_Restart(restart_type, restart_name, rst_offset, usr.time,
                    ctltype, parm, starttime, css, returning);
#endif
  }


  /* Disable the FOSSIL */

/*  if (!in_wfc)
    Mdm_flush_ck_tic(4000, FALSE, TRUE);

  mdm_deinit();*/


  Out_Save_Directories(stay);


  /* Convert "%" command characters */

  if (strchr(parm,'%'))
    Parse_Outside_Cmd(parm, temp2);
  else
    strcpy(temp2, parm);

  parm=temp2;


  /* Write .CTL file for Opus-compatible junk */

  if (ctltype==CTL_NORMAL && !in_wfc)
    Write_External_Ctlfile(ctltype, parm, method);


  /* OUTSIDE_chain will disable the ^c handler elsewhere */

  if (method != OUTSIDE_CHAIN)
    Out_Disable_Ctrlc();

  Out_Video_Down();


  #if defined(OS_2)
    {
      /* Hide our file handles so that children do not inherit them - saves *
       * all kinds of problems with archivers and other things.             */

      extern HCOMM hcModem;
      HFILE hf;
      int i;

      hf=ComGetFH(hcModem);

      for (i=5; i < 50; i++)
        if (i != hf)
          DosSetFHandState(i, OPEN_FLAGS_NOINHERIT);
    }
  #elif defined(__MSDOS__)
    /* Save the current stdin/stdout handles */

    save_stdout=dup(fileno(stdout));
    save_stdin=dup(fileno(stdin));
  #endif /* nothing required for NT or others */


#ifdef __MSDOS__
  if (method==OUTSIDE_ERRORLEVEL)
  {
    /***********************************************************************/
    /*                            ERRORLEVEL EXIT                          */
    /***********************************************************************/

    erl=atoi(parm);

    if (erl==255 || erl <= 5)
      logit(log_bad_erl);
    else
    {
      sprintf(temp, erl_xx, parm);

      if (!in_wfc)
        logit(external_prog, temp);

      if ((p=firstchar(parm, ctl_delim, 2)) != NULL && *p)
      {
#ifndef UNIX
        if (task_num)
          sprintf(temp, "%sERRORL%02x.BAT", original_path, task_num);
        else sprintf(temp, "%sERRORLVL.BAT", original_path);
#else
        if (task_num)
          sprintf(temp, "%serrorl%02x.sh", original_path, task_num);
        else sprintf(temp, "%serrorl.sh", original_path);
#endif

        if ((bat=shfopen(temp, fopen_write, O_WRONLY | O_CREAT | O_TRUNC))==NULL)
          cant_open(temp);
        else
        {
          fprintf(bat,"%s\n",p);
          fclose(bat);
        }
      }

/*      if ((prm.flags & FLAG_watchdog) && !local && !in_wfc)
        mdm_watchdog(1);*/

      nowrite_lastuser=TRUE;
      quit(erl);
    }
  }
  else
#endif
  if (method==OUTSIDE_RUN || method==OUTSIDE_CHAIN ||
      method==OUTSIDE_CONCUR)
  {

    /***********************************************************************/
    /*                          RUN/CHAIN EXIT                             */
    /***********************************************************************/

    strcpy(temp, parm);

    if (!in_wfc)
      logit(external_prog, parm);

    for (x=0, p=strtok(temp, ctl_delim);
         p && x < MAX_EXECARGS;
         x++, p=strtok(NULL, ctl_delim))
    {
#ifndef OS_2
      if (*p=='<')
      {
        freopen(p+1, fopen_read, stdin);
        x--;
      }
      else if (*p=='>')
      {
        if (*(p+1)=='>')  /* Is it for append? */
          freopen(p+2, fopen_append, stdout);
        else freopen(p+1, fopen_write, stdout);

        x--;
      }
      else  /* Just a normal argument */
#endif
      {
        if ((args[x]=(char *)malloc(strlen(p)+1))==NULL)
        {
          logit("!OA-MEMOVFL");
          erl=-1;
          goto RetProc;
        }

        strcpy(args[x],p);

        s=strrchr(args[x], '.');

        if (x==0 && s &&
            (strnicmp(s,".bat",4)==0 ||
            strnicmp(s,".btm",4)==0 ||
            strnicmp(s,".cmd",4)==0))
        {
          logit(log_badex1);
          logit(log_badex2);

          args[++x]=NULL;
          erl=-1;
          goto Skip;
        }
      }
    }

    args[x]=NULL;

    Close_Files();

    if (method==OUTSIDE_CHAIN)
    {
      /* Close files, terminate keyboard handler, etc. */

      nowrite_lastuser=TRUE;
      FinishUp2(FALSE);
      nowrite_lastuser=FALSE;

      Out_Disable_Ctrlc();
    }

/*    if ((prm.flags & FLAG_watchdog) && !local && !in_wfc)
      mdm_watchdog(1);*/

    IoPause();

    #if defined(__WATCOMC__) && !defined(__386__)
      _heapshrink();
    #endif

  #ifdef SWAP
    if (prm.flags2 & FLAG2_SWAPOUT)
      erl=swapvp(args[0], args);
    else
  #endif
  #ifndef UNIX
      erl=spawnvp(P_WAIT, args[0], args);
  #else
      erl=xxspawnvp(P_WAIT, args[0], args);
  #endif
    IoResume();

/*    if ((prm.flags & FLAG_watchdog) && !local && !in_wfc)
      mdm_watchdog(0);*/

    Reopen_Files();

    if (erl==-1)
    {
      switch(errno)
      {
        case ENOENT:
          logit(log_badnf);
          break;

        case ENOMEM:
          logit(log_badnm);
          break;

        default:
          logit(log_badee, errno);
          break;
      }
    }

Skip:

    for (x=0; args[x]; x++)
      if (args[x])
        free(args[x]);

    strcpy(temp, parm);

    if (!in_wfc)
      logit(return_prog, parm, erl);
  }
  else if (method==OUTSIDE_DOS)
  {
    /***********************************************************************/
    /*                                DOS EXIT                             */
    /***********************************************************************/

    strcpy(temp, parm);

    if (!in_wfc)
      logit(external_prog, temp);

    Close_Files();

/*    if ((prm.flags & FLAG_watchdog) && !local && !in_wfc)
      mdm_watchdog(1);*/

    IoPause();

    #if defined(__WATCOMC__) && !defined(__FLAT__)
      _heapshrink();
    #endif

  #ifdef SWAP
    if (prm.flags2 & FLAG2_SWAPOUT)
    {
      strcpy(temp, GetComspec());
  #ifdef UNIX
      strcat(temp, " -c ");
  #else
      strcat(temp, " /c ");
  #endif
      strcat(temp, parm);

      erl=swapsz(temp);

      strcpy(temp, parm);
    }
    else
      erl=system(temp);
  #endif

    IoResume();

/*    if ((prm.flags & FLAG_watchdog) && !local && !in_wfc)
      mdm_watchdog(0);*/

    Reopen_Files();

    if (!in_wfc)
      logit(return_prog, temp, erl);
  }
  else logit(log_badom);

RetProc:

  /* Restore the saved stdin/stdout files */

  #ifdef __MSDOS__
    dup2(save_stdin, fileno(stdin));
    dup2(save_stdout, fileno(stdout));

    close(save_stdin);
    close(save_stdout);
  #endif



  #if defined(__WATCOMC__) && !defined(__386__)
    _heapgrow();
  #endif

  /* Grab the 'timeremaining' value from LASTUSxx.BBS */

  if (reread && !in_wfc)
  {
    word old_priv = usr.priv;
    word old_max2priv = usr.max2priv;

    Outside_Reread(timeremain, tonline);

    /* If the max2 priv level was changed in the user record, convert it
     * so that the new priv level reflects this change.
     */

    if (old_max2priv != usr.max2priv && old_priv==usr.priv)
      usr.priv=max3priv(usr.max2priv);

    Find_Class_Number();
    SetUserName(&usr, usrname);
  }

  /* Reinstall the critical error handler */

  if ((prm.flags2 & FLAG2_NOCRIT)==0)
    install_24();

  /* Restart ^c handler */

#if !(defined(OS_2) && defined(__FLAT__))
  brktrap();
#endif

#ifdef __MSDOS__
  Restore_Directories3();
#endif

  Out_Reinstall_Fossil();
  Out_Video_Up();

  display_line=display_col=1;
  inchat=save_inchat;

//  mdm_attr=curattr=-1;
  
  Puts(GRAY);
  
  if (returning && !in_wfc)
  {
    Display_File(0, NULL, returning);
    vbuf_flush();
  }

  ChatRestoreStatus(css);

  free(css);
  free(args);
  free(temp2);
  free(temp);
 
  return erl;
}


static void near Out_Save_Directories(int stay)
{
#ifndef __MSDOS__
  NW(stay);

  /* Real operating systems don't let the child change the parent's         *
   * current directory.                                                     */

#else
  char *temp;
  int disk, max_d, x;

  /* Save the current active drive */
  disk=getdisk();

  /* Find out how many drives we have available by calling setdisk().    *
   * This call doesn't actually CHANGE anything, but we need the return  *
   * value to figure out which drives to save.                           */

  max_d=setdisk(disk);
  max_d=min(max_d,MAX_DRIVES);

  /* Scan the "Save Directories" statement to see if we were supposed to *
   * save any drives...                                                  */

  for (x=0;x < max_d;x++)
    if (IsBit(prm.drives_to_save,x))
      break;

  /* If we are saving some drives, go into this next loop... */

  if (x != max_d)
  {
    /* If the current disk is less than 2 (ie. A: or B:), then we should   *
     * assume that we're running a floppy-based system, and should save    *
     * all of the drives we can get.  Otherwise, we're probably running    *
     * on a HARD DISK, and should only save the drives above A: and B:.    *
     * We also save as many drives as we can get our hands on...           */

    if ((temp=malloc(PATHLEN)) != NULL)
    {
      for (x=0;x < max_d;x++)
      {
        if (IsBit(prm.drives_to_save,x))
        {
          sprintf(temp,"%c:.",'A'+x);
          Save_Directory3(temp);

          if (!stay && orig_path2[x])
            chdir(orig_path2[x]);
        }
      }
      
      free(temp);
    }

    /* Change us back to the right disk drive */

    setdisk(disk);
  }
#endif
}











static void near Out_Disable_Ctrlc(void)
{
  /* Temporarily get rid of our ^C handler, in case that we get SWAPPED to *
   * disk by some other program.  If we DIDN'T do this, then the computer  *
   * would branch to V'ger if the Sysop hit ^C or ^break while we were     *
   * away...  Chain will uninstall the ^C handler in a different place,    *
   * which is why we don't try to do it here.                              */

  /* Print something to stdio before removing the keyboard handler --    *
   * DOS doesn't like to execute the 0x23 interrupt until something      *
   * is printed, and we want to make sure that if it is going to         *
   * happen, that it does so while we have our handlers in place.        */

  fprintf(stdout,"\x08");

#if !(defined(OS_2) && defined(__FLAT__))
  brkuntrap();
#endif

  if ((prm.flags2 & FLAG2_NOCRIT)==0)
    uninstall_24();

  /* Put cursor back to real position, and flush output */
  vbuf_flush();
}




static void near Out_Video_Down(void)
{
#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
#endif
  {
    #ifdef SHUTVIDEO
      ShutDownVideo();  /* shut down completely */
    #else
      if (dspwin)
      {
        WinClose(dspwin);
        dspwin_time=0L;
        dspwin=NULL;
      }
  
      Draw_StatusLine(STATUS_REMOVE);
  
      #ifdef __MSDOS__
      if (multitasker==MULTITASKER_desqview)
        End_Shadow();
      #endif
      
      if (!no_video && !in_wfc)
        WinSyncAll();
    #endif
  }
}



static void near Out_Video_Up(void)
{
#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
  {
#endif
    #ifdef SHUTVIDEO
      StartUpVideo(); /* start-up from scratch */
    #else

      #ifdef __MSDOS__
      if (multitasker==MULTITASKER_desqview)
        Start_Shadow();
      #endif

      if (!no_video)
      {
        WinDirtyAll(win);
        VidGotoXY(current_col,current_line,TRUE);
        WinSync(win,TRUE);

        if (in_wfc)
          WinSyncAll();
      }
    #endif
      
    vbuf_flush();

    if (!in_wfc)
      Draw_StatusLine(STATUS_FORCE);
#ifdef TTYVIDEO
  }
  else
  {
    /* We just ran an external program, so we have no idea what the        *
     * other program was doing with the user's screen location, so our     *
     * best guess is where the local cursor is on our screen...            */

    Lputs(CLS);
    fossil_getxy(&current_line,&current_col);
  }
#endif
}



static void near Out_Reinstall_Fossil(void)
{
  /* The next reinit is just so we can make sure the FOSSIL is still *
   * there, and that an external program didn't borrow our interrupt *
   * vector, and forget to give it back.  (How rude!)                */

/*  mdm_deinit();
  Fossil_Install(FALSE);
  Mdm_Flow_On();*/
}



static void near Close_Files(void)
{
  if ((prm.flags & FLAG_close_sf)==0)
  {
    LogFlush();
    return;
  }
  
  /* we can't do this if there are any messages open!!! */
/*  Switch_Msg_Area(NULL, 0);*/ /* close the current message area */

  LogClose();
}







static void near Reopen_Files(void)
{
  if (prm.flags & FLAG_close_sf)
  {
    if (! LogOpen())
      quit(ERROR_CRITICAL);
  }
}





static void near Outside_Reread(int tr, int tonline)
{
  char temp[PATHLEN];
  int ffile;

  if (task_num)
    sprintf(temp, lastusxx_bbs, original_path, task_num);
  else sprintf(temp, lastuser_bbs, original_path);

  if ((ffile=shopen(temp, O_RDONLY | O_BINARY))==-1)
  {
    cant_open(temp);
    return;
  }

  read(ffile, (char *)&usr, sizeof(struct _usr));
  close(ffile);

  /* The 'timeremain' field is calculated as a relative value...  In      *
   * other words, the door program can only add or subtract minutes       *
   * from the time-remaining value, and Maximus will figure out the       *
   * relative inc/decrement by comparing the amount of time, as it was    *
   * written into the file by Max itself, to the time limit that was      *
   * read back in this function.  This works well, since the time limit   *
   * will continue normally if the program doesn't touch the              *
   * time-remaining field.  If the program wants to guarantee that a      *
   * specific number of minutes are left when the program exits, then     *
   * it must calculate the current time when the external program is      *
   * started, and add the value in usr.timeremain to it.  It should       *
   * then calculate the desired value for usr.timeremain, to get          *
   * the time-off to equal the required value.  (The usr.timeremain       *
   * field can be either decremented or incremented, and Max will         *
   * adjust appropriately.)                                               */

  timeoff += (usr.timeremaining-tr)*60L;
  
  /* We did a usr.time += timeonline() earlier, so we now have to undo    *
   * it if we need the user record for our own use.                       */

  usr.time -= tonline;

  /* Set the user's delflag back to normal */

  usr.delflag=usr.df_save;

}



void Shell_To_Dos(void)
{
  int sheap;

  if (no_shell)
    return;

  sheap=Language_Save_Heap();

  display_line=display_col=1;

  if (*PRM(shelltodos) && !in_wfc)
    Display_File(0, NULL, PRM(shelltodos));

  if (!in_wfc)
    Puts(WHITE);

  Outside(NULL, NULL, OUTSIDE_RUN, GetComspec(), FALSE, CTL_NONE,
          RESTART_MENU, NULL);

  if (!in_wfc)
    Lputs(CLS);

  if (*PRM(backfromdos) && !in_wfc)
    Display_File(0, NULL, PRM(backfromdos));

  Printf(attr_string,mdm_attr);  /* Set us back to the right colour */

  if (inmagnet)
    Fix_MagnEt();

  Language_Restore_Heap(sheap);
  vbuf_flush();
}


#endif /* !ORACLE */





int Exec_Xtern(int type, struct _opt *thisopt, char *arg, char **result, char *menuname)
{
  int method;
  int ctl, ret;


  *result=NULL;

  switch(type)
  {
    case xtern_erlvl:   method=OUTSIDE_ERRORLEVEL;  break;
    case xtern_run:     method=OUTSIDE_RUN;         break;
    case xtern_concur:  method=OUTSIDE_CONCUR;      break;
    case xtern_chain:   method=OUTSIDE_CHAIN;       break;
    case xtern_dos:     method=OUTSIDE_DOS;         break;
    default:            logit(bad_menu_opt, type);  return 0;
  }

  if (thisopt->flag & OFLAG_REREAD)
    method |= OUTSIDE_REREAD;

  ctl = (thisopt->flag & OFLAG_CTL) ? CTL_NORMAL : CTL_NONE;
  
  ret=Outside(PRM(out_leaving), PRM(out_return), method,
              Strip_Underscore(arg), TRUE, ctl, RESTART_MENU,
              menuname);

  Clear_KBuffer();
  
  return ret;
}


#ifndef ORACLE

static int near Form_Outside_Filename(char *in, char *out, int method)
{
  if (method != OUTSIDE_ERRORLEVEL)
  {
    getword(in, out, ".", 1);
    return 0;
  }
  
  getword(in, out, " .", 2);

  /* Return TRUE if out is empty */
  return (*out=='\0');
}



/* write a .ctl file for opus-compatible junk */

static void near Write_External_Ctlfile(int ctltype, char *parm, int method)
{
  FILE *ctl;

  char temp[PATHLEN];
  char temp2[PATHLEN];
  long bd=steady_baud_l ? steady_baud_l : baud;

  NW(ctltype);

  if (Form_Outside_Filename(parm, temp, method))
    return;

  strcat(temp, ".ctl");

  if ((ctl=shfopen(Add_Task_Number(temp, temp2),
                   fopen_write,
                   O_WRONLY | O_CREAT | O_TRUNC))==NULL)
  {
    cant_open(temp2);
    return;
  }

  if (local)
    fprintf(ctl, xctl_keyboard);
  else fprintf(ctl, xctl_port_baud, port+1, baud);

  if (!local)
    fprintf(ctl, xctl_modem,
            port+1, bd,
            prm.handshake_mask /* 3 */, prm.carrier_mask, baud);

  fprintf(ctl, xctl_time, timeleft());

  if (log_name && *log_name)
    fprintf(ctl, xctl_log, log_name);

  fprintf(ctl, xctl_msgs, MAS(mah, path));
  fprintf(ctl, xctl_uploads, FAS(fah, uppath));
  fprintf(ctl, xctl_downloads, FAS(fah, downpath));
  fprintf(ctl, xctl_help, original_path);

  if (*FAS(fah, filesbbs))
    fprintf(ctl, xctl_filesbbs, FAS(fah, filesbbs));

#ifdef NEVER
  if (FileEntries())
  {
    FENTRY fent;

    char *m=(ctltype & CTL_DOWNLOAD)?"Send %s\n"
                                    :(ctltype & CTL_UPLOAD)?"Got %s\n":blank_str;
    for (x=0; GetFileEntry(x, &fent); x++)
      fprintf(ctl, m, fent.szName);
  }
#endif

  fclose(ctl);
}


static char * near Add_Task_Number(char *fname, char *new)
{
  char *p,*s;
  char tn[10];

  if (new != fname)
    strocpy(new, fname);

  if (task_num)
  {
    /*         s       p    */
    /*         v       v    */
    /* D:\Path\Filename.Ext */

    sprintf(tn, "%02x", task_num);

    p=strrchr(new, '.');
    s=strrchr(new, PATH_DELIM);

    if (s)
      s++;
    else s=new;

    if (!p)
      p=s+strlen(s);

    if (p <= s+6)
      strocpy(p+2, p);
    else if (p==s+7)
    {
      strocpy(p+1, p);
      p--;
    }
    else p -= 2;

    *p=*tn;
    *(p+1)=*(tn+1);
  }

  return new;
}


#endif /* !ORACLE */

