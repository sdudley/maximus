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

#ifndef __GNUC__
#pragma off(unreferenced)
#endif

static char __attribute__((unused)) rcs_id[]="$Id: max_fini.c,v 1.3 2004/01/11 19:43:21 wmcbrine Exp $";

#ifndef __GNUC__
#pragma on(unreferenced)
#endif

/*# tname=Termination code
*/

#define MAX_LANG_max_init
#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include "prog.h"
#include "ffind.h"
#include "max_msg.h"
#include "max_file.h"
#include "dr.h"
#include "userapi.h"
#include "trackm.h"

#ifndef ORACLE

static void near FreeAccess(void);
static void near Local_Cleanup(void);
static void near Write_User(void);
static void near ChatOff(void);

/* This routine is called by FinishUp(), which just acts as a gateway for  *
 * this overlaid function.                                                 *
 *                                                                         *
 * This cleans up everything, closes files, updates the user's             *
 * entry in USER.BBS and LASTUSER.BBS, puts the modem on-hook, restores    *
 * the current directory, and writes the echo tosslog.                     */

void FinishUp2(int hangup)
{
  long wasonfor;
  char temp[PATHLEN];

  /* Make sure that we don't recursively call ourself */

  if (finished)
    return;

  finished=TRUE;

  

  wasonfor=timeonline();
  caller_online=FALSE;

  /* Turn off flow control so we don't hang! */
  Mdm_Flow(FLOW_OFF);

  if (hangup && rst_offset==-1L)
    ChatOff();    /* Finish off any multi-line chat jazz */


#if defined(OS_2) || defined(NT)
  if (!local)
  {
    ComWatchDog(hcModem, FALSE, 0);
    ComPurge(hcModem, COMM_PURGE_ALL);
  }
#endif

  if (!local && carrier())
    mdm_dump(DUMP_INPUT);

  Restore_Directories3();
  Restore_Directories2();

  /* Exit the current message and file areas */

  while (PopMsgArea())
    ;

  while (PopFileArea())
    ;

  if (MsgCloseApi()==-1)
    logit(log_err_msgapi);
  
  if (this_logon_bad)
    usr.bits2 |= BITS2_BADLOGON;

  Lputs(GRAY);

  if (hangup && !in_wfc)
  {
    Lputc('\n');


    if (fLoggedOn && rst_offset==-1L)
      logit(log_user_off_line,
            *usrname ? usrname : user_text,
            usr.times,
            (long)wasonfor,
            (long)usr.time+wasonfor);
  }

  ExitMsgAreaBarricade();
  ExitFileAreaBarricade();

  if (locked)
  {
    locked=FALSE;
    usr.priv=lockpriv;
  }


  if (fLoggedOn && !in_wfc)
  {
    usr.ludate=next_ludate;

    if (!nowrite_lastuser)
      Write_LastUser();

    TagWriteTagFile(&mtm);
  }

#ifdef MAX_TRACKER
  DeinitTracker();
#endif

  usr.time += (int)wasonfor;
  usr.call++;

  if (usr.xp_flag & XFLAG_EXPMINS)
  {
    if ((signed long)usr.xp_mins-wasonfor <= 0)
      Xpired(REASON_TIME);
    else usr.xp_mins -= wasonfor;
  }

  if (fLoggedOn && !acsflag(CFLAGA_HIDDEN))
    ci_save();

  if ((fLoggedOn || this_logon_bad) && !nowrite_lastuser && !in_wfc)
  {
    Write_User();
    if (fLoggedOn && !acsflag(CFLAGA_HIDDEN))
    {
      char *name=usr.name;

      if ((prm.flags & FLAG_alias) && *usr.alias)
        name=usr.alias;

      strncpy(bstats.lastuser, name, sizeof bstats.lastuser);
    }
  }

  usr.video=GRAPH_ANSI;  /* to make the "lputs(gray)" work */
  Lputs(GRAY);

  Write_Stats(&bstats);

  if (hangup && rst_offset==-1L && in_node_chat)
    ChatCleanUp();


  if (rst_offset==-1L && !in_wfc)
  {
    sprintf(temp,activexx_bbs,original_path,task_num);
    unlink(temp);
  }


  if (chatlog)
    Close_Chatlog();


  if (hangup && rst_offset==-1L)
  {
    logit(log_max_end, version, (word)(byte)erl);
/*    LogWrite("\n");*/
  }


  LogClose();

  AreaFileClose(ham);
  AreaFileClose(haf);

  
  if (fossil_initd && rst_offset==-1L)
  {
    if (!local)
    {
      Mdm_Flow(FLOW_OFF);
      mdm_dump(DUMP_OUTPUT);

      mdm_baud((char)Decimal_Baud_To_Mask(prm.max_baud));
      mdm_cmd(PRM(m_busy));
    }

    mdm_deinit();

    fossil_initd=FALSE;
  }

  
  if (offsets)
    (free)(offsets);
  
  if (original_prompt)
  {
    /* extra parens for dmalloc() kludge - see max.c */
    (free)(original_prompt);
  }

#ifdef MCP
  #ifdef MCP_VIDEO
    mcp_out_flush();
  #endif

  ChatCloseMCP();
#endif

  ShutDownVideo();
  LanguageCleanup();

  ClassDispose();
  FreeAccess();
  EventListFree();
  OutputFreeStr();
  InputFreeStr();
  Free_File_Buffer();


#ifdef DBG
  printf("ô\n"
         "³ Maximus down.\n"
         "õ\n\n");
#endif
}


static void near FreeAccess(void)
{
#ifndef ORACLE
  extern PLIST *pl_privs;
  free(pl_privs);
#endif
}


/* Cleans up any of the sticky IPC*.BBS files that may be lying around */

static void near ChatOff(void)
{
  char temp[PATHLEN];

  if (! *PRM(ipc_path))
    return;

  sprintf(temp, ipcxx_bbs, PRM(ipc_path), task_num);
  unlink(temp);
}








void Got_A_Null_Pointer(char *type, char *where)
{
  logit(">!Null ptr/%s%s. PLEASE REPORT BUG TO AUTHOR!", type, where);

  #ifdef __NEARDATA__
  logit(">!DataSeg=%08lx%08lx%08lx%08lx", *(long *)0x0000,
        *(long *)0x0004, *(long *)0x0008, *(long *)0x000c);
  #endif
  
/*  Restore_Directories();*/

  logit(log_got_null_ptr);

  Puts(found_nptr);
  
  Delay(300);

  LogClose();

  if (fossil_initd && rst_offset==-1L)
  {
    if (!local && !in_wfc)
    {
      mdm_baud(Decimal_Baud_To_Mask(prm.max_baud));
      mdm_cmd(PRM(m_busy));
    }

    mdm_deinit();

    fossil_initd=FALSE;
  }

  brkuntrap();

  _exit(16);
}







void ShutDownVideo(void)
{
  vbuf_flush();

#ifdef TTYVIDEO
  if (displaymode != VIDEO_IBM)
    return;
#endif

  if (prm.flags & FLAG_statusline)
    Draw_StatusLine(STATUS_REMOVE);
  
  if (dspwin)
  {
    WinClose(dspwin);
    dspwin_time=0L;
  }

  Local_Cleanup();

  /* Reset the standard output pointers, in case we try to output something *
   * later.                                                                 */

  local_putc=(void (_stdc *)(int))fputchar;
  local_puts=(void (_stdc *)(char *))putss;

  if (!no_video)
    WinSync(win, TRUE);

#ifdef __MSDOS__
  if (multitasker==MULTITASKER_desqview)
    End_Shadow();
#endif

  WinClose(win);
  WinApiClose();
  VidClose();
}



/* Close the pop-up priv window, if it exists */

static void near Local_Cleanup(void)
{
  extern VWIN *privwin;
  
  if (privwin)
  {
    WinClose(privwin);
    privwin=NULL;
  }
}



void Lost_Carrier(void)  /* This is what we run if a user drops carrier */
{
  if (finished || in_wfc)
    return;

  ci_carrier();

  if (caller_online || !fLoggedOn)
  {
    Lputs(CYAN "\n");
    
    if (do_caller_vanished)
      logit(log_byebye);
  }
  
  caller_online=FALSE;

  Mdm_Flow(FLOW_OFF);
  mdm_dump(DUMP_ALL);

  quit(0);
}





/* Used when caller falls asleep */

void PleaseRespond(void)
{
  if (finished)
    return;

  if (!shut_up)
    Puts(pls_rsp);

  vbuf_flush();
}



void Time5Left(void) /* Only 5 min. left */
{
  if (finished)
    return;

  if (!sent_time_5left && !shut_up)
  {
    sent_time_5left=TRUE;
    do_timecheck=FALSE;
    Puts(min5_left);
    do_timecheck=TRUE;
    vbuf_flush();
  }
}



void TimeAlmostUp(void)  /* Time limit almost used up (2 mins) */
{
  if (finished)
    return;

  if (!sent_time_almostup && !shut_up)
  {
    sent_time_almostup=TRUE;
    do_timecheck=FALSE;
    Puts(almost_up);
    do_timecheck=TRUE;
    vbuf_flush();
  }
}


void TimeLimit(void)   /* User overrran time limit! */
{
  if (finished)
    return;

  ci_timelimit();

  logit(log_tlimit);

  do_timecheck=FALSE;

  if (!shut_up)
  {
    Puts(time_up);
    Display_File(0, NULL, time_file, PRM(misc_path));
  }

  vbuf_flush();
  mdm_hangup();
}



void Xpired(int reason)
{
  int xpflag;
  
  Display_File(0, 
               NULL,
               "%sXP%s",
               PRM(misc_path),
               (reason==REASON_TIME) ? "TIME" : "DATE");
  
  xpflag=usr.xp_flag;
  usr.xp_flag &= ~(XFLAG_EXPMINS | XFLAG_EXPDATE | XFLAG_AXE | XFLAG_DEMOTE);
  usr.xp_mins=0;
  usr.xp_date.ldate=0L;

  ci_expired();  
  
  if (xpflag & XFLAG_AXE)
  {
    usr.delflag |= UFLAG_DEL;
    
    /* If reason==REASON_TIME, then we're already hanging up anyway */
    
    if (reason==REASON_DATE)
      mdm_hangup();
  }
  
  if (xpflag & XFLAG_DEMOTE)
  {
    usr.priv=usr.xp_priv;
    Find_Class_Number();
  }
}



/* Clean up after a multi-line chat session */

void ChatCleanUp(void)
{
  extern word *tasks;
  extern word num_task;

  char temp[PATHLEN];
  word x;

  if (! (tasks && in_node_chat))
    return;
  
  for (x=0; x < num_task; x++)
  {
    sprintf(temp, ch_off_abnormally, usrname);

    if (ChatSendMsg((byte)tasks[x], CMSG_HEY_DUDE, strlen(temp)+1, temp)==-1)
    #ifdef DEBUG
      Printf("@MsgErr %d@",tasks[x]);
    #else
      ;
    #endif


    strcpy(temp, usrname);

    if (ChatSendMsg((byte)tasks[x], CMSG_EOT, strlen(temp)+1, temp)==-1)
    #ifdef DEBUG
      Printf("@MsgErr %d@",tasks[x]);
    #else
      ;
    #endif
  }

  num_task=0;
  free(tasks);
}



#endif /* !ORACLE */

void mdm_hangup(void)  /* Do the raise DTR/drop DTR thingy */
{
  long flush_tout;

  if (!local 
#if (COMMAPI_VER > 1)
	&& ComIsAModem(hcModem)
#endif
     )
  {
    /* Turn off flow control so we don't get stuck with a ^s! */
    Mdm_Flow_Off();

#ifdef OS_2
    if (ComIsOnline(hcModem))
        ComTxWait(hcModem, 10000L);

    ComWatchDog(hcModem, FALSE, 0);
    ComPurge(hcModem, COMM_PURGE_ALL);
#endif

    /* Set a time limit anyway, just in case FOSSIL is brain-dead and      *
     * ignores instructions.                                               */

    flush_tout=timerset(1500);

    while (! out_empty() && !timeup(flush_tout))
      Giveaway_Slice();

    mdm_dump(DUMP_OUTPUT);
    
    if (!no_dtr_drop)
    {
      mdm_dtr(DTR_DOWN);
      Delay(250);
      mdm_dtr(DTR_UP);
    }
  }

#if (COMMAPI_VER > 1)
  if (!local && !ComIsAModem(hcModem))
  {
    ComClose(hcModem);
  }
#endif
  
  quit(0);
}

void medfini(void);

int quit(int el) /* A substitute for exit, it exits with error code <erl> */
{             /* only if erl > 0. (Typically used for an ERROR exit
                 condition.)  If the errorlevel IS zero, then we check
                 all of the flags to find out if the user wrote a matrix
                 or echomail message, and then exit with the appropriate
                 errorlevel.                                               */

  if (el)
    erl=(char)el;
  else if (!fLoggedOn)
    erl=ERROR_RECYCLE;
  else if ((written_echomail || written_conf) && prm.echo_exit)
    erl=prm.echo_exit;
  else if (written_matrix && prm.edit_exit)
    erl=prm.edit_exit;
  else if (written_local && prm.local_exit)
    erl=prm.local_exit;
  else if (prm.exit_val)  /* If we have a specific el for after each caller */
    erl=prm.exit_val;
  else erl=ERROR_RECYCLE;

#ifdef OS_2
  medfini();
#endif

  maximus_exit(erl);
  return 0;
}





/* Write user number from the structure pointed to by `user' */

#ifndef ORACLE
static void near Write_User(void)
{
  HUF huf;
  struct _usr user;
  
  if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
  {
    cant_open(PRM(user_file));
    Local_Beep(3);
    return;
  }

  /* Compatibility with Max 2.00 */

  user=usr;

  user.max2priv=max2priv(user.priv);
  Adjust_User_Record(&user);
  
  if (!UserFileUpdate(huf, origusr.name, origusr.alias, &user))
    logit(cantwrite, PRM(user_file));

  UserFileClose(huf);
}
#endif /* !ORACLE */

