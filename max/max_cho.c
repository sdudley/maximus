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
static char rcs_id[]="$Id: max_cho.c,v 1.2 2003/08/16 23:45:33 paltas Exp $";
#pragma on(unreferenced)

/*# name=Chat Mode routines (overlayed).  Includes the chat itself, routines
    name=for reading IPCxx.BBS, and for processing inbound messages.
*/

#define MAX_LANG_max_chat

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mem.h>
#include <string.h>
#include "prog.h"
#include "ffind.h"
#include "mm.h"
#include "alc.h"
#include "max_chat.h"

#ifdef MCP
  #define INCL_DOS
  #include "pos2.h"
  #include "mcp.h"
  extern HPIPE hpMCP;
#endif

extern struct _cstat cstat;

extern word cur_ch;
extern word num_task;
extern word *tasks;

extern long scan_time;

static void near Chat_Toggle(void);
static void near Chat(int use_cb, int doing_answer);
static void near Page(void);
static void near ChatNotAvail(int tid);
static void near ChatCantPageSelf(void);
static int near Remove_From_Chat_List(byte tid);
static void near Add_To_Chat_List(byte tid);


int Exec_Chat(int type, char **result)
{
  *result=NULL;
  
  switch(type)
  {
    case who_is_on:   Who_Is_On();      break;
    case o_page:      Page();           break;
    case o_chat_cb:   Chat(TRUE, FALSE);break;
    case o_chat_pvt:  Chat(FALSE, TRUE);break;
    case chat_toggle: Chat_Toggle();    break;
    default:          logit(bad_menu_opt, type); return 0;
  }
  
  return 0;
}

      


static struct _cgs * near ChatFindOpen(void)
{
  static struct _cgs *cgs;

#ifndef MCP
  if (! *PRM(ipc_path))
    return NULL;
#endif

  if ((cgs=malloc(sizeof(struct _cgs)))==NULL)
    return NULL;

#ifdef MCP
    if (!hpMCP)
      return NULL;

  {
    USHORT usQuery=PMSG_QUERY_TASKLIST;
    OS2UINT cbRead, rc;
    byte tids[255];
    int i;

    /* Query the active task numbers */

    rc=DosTransactNmPipe(hpMCP, (PBYTE)&usQuery, sizeof usQuery,
                         tids, sizeof tids, &cbRead);

    if (rc)
    {
      logit("!SYS%04d: DosTransactNmPipe (ChatFindOpen)", rc);
      free(cgs);
      return NULL;
    }

    /* tids[0] is the number of ids */

    for (i=1; i < cbRead; i++)
      cgs->tids[i-1]=tids[i];

    cgs->num_tid=cbRead-1;
  }

#else
  {
    char temp[PATHLEN];
    FFIND *ff;

    /* Scan all of the current IPC files, and read them into an array */

    sprintf(temp, ipc_star, PRM(ipc_path));

    cgs->num_tid=0;

    if ((ff=FindOpen(temp, 0))==NULL)
    {
      free(cgs);
      return NULL;
    }

    for (;;)
    {
      int tid;

#ifndef UNIX
      strupr(ff->szName);
#endif

      /* Grab the number out of this one */

      if (sscanf(ff->szName, ipc_x, &tid)==1)
        cgs->tids[cgs->num_tid++]=tid;

      if (FindNext(ff) != 0)
        break;
    }

    FindClose(ff);
  }

#endif

  /* Now sort them, so they come out in the correct order */

  qksort(cgs->tids, cgs->num_tid);

  /* And 'ptr' tells the FindNext() routine where to start at. */

  cgs->ptr=0;

  return cgs;
}




/* Same as the FindNext() call above, except it checks the stats for a     *
 * particular task number.                                                 */

int ChatFindIndividual(byte tid, char *username, char *status, word *avail)
{
  struct _cstat cstat;

#ifdef MCP
    byte req[3];
    OS2UINT cbRead, rc;

    if (!hpMCP)
      return FALSE;

    *(USHORT *)req=PMSG_QUERY_TASKINFO;
    req[2]=tid;


    /* Query the specified task */

    rc=DosTransactNmPipe(hpMCP, req, sizeof req,
                         (PBYTE)&cstat, sizeof cstat, &cbRead);

    if (rc)
    {
      logit("!SYS%04d: DosTransactNmPipe (ChatFindIndividual)", rc);
      return FALSE;
    }

#else
  int fd;

  if (! *PRM(ipc_path))
    return FALSE;

  if ((fd=ChatOpenIPC(tid))==-1)
    return FALSE;

  read(fd, (char *)&cstat, sizeof(struct _cstat));

  ChatCloseIPC(fd);
#endif

  if (username)
    strcpy(username, cstat.username);

  if (status)
    strcpy(status, cstat.status);

  if (avail)
    *avail=cstat.avail;

  return TRUE;
}



static int near ChatFindNext(struct _cgs *cgs, byte *tid, char *username, char *status, word *avail)
{
  if (cgs->ptr==cgs->num_tid)
    return FALSE;

  *tid=(byte)cgs->tids[cgs->ptr++];

  return ChatFindIndividual(*tid, username, status, avail);
}



/* Must be called after ChatFindOpen() */

static void near ChatFindClose(struct _cgs *cgs)
{
  if (cgs)
    free(cgs);
}




static void near Page(void)
{
  char temp[PATHLEN];

  word avail;
  byte tid;

#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif

  for (strcpy(temp, qmark); eqstri(temp, qmark);)
  {
    WhiteN();

    InputGets(temp, ch_node_to_page);

    if (eqstri(temp, qmark))
      Who_Is_On();
  }

  if ((tid=(byte)atoi(temp)) != 0)
  {
    if (tid==task_num)
      ChatCantPageSelf();
    else if (! ChatFindIndividual(tid, NULL, NULL, &avail) || !avail)
      ChatNotAvail(tid);
    else
    {
      sprintf(temp, ch_being_paged, usrname, task_num);

      if (ChatSendMsg(tid, CMSG_PAGE, strlen(temp)+1, temp)==-1)
      #ifdef DEBUG
        Printf("*MsgErr*");
      #else
        ;
      #endif

      Printf(ch_waiting, tid);

      sprintf(linebuf, "%d", tid);
      Chat(FALSE, FALSE);
    }
  }
}



/* Figure out who is on the other nodes of a multi-line system */

void Who_Is_On(void)
{
  struct _cgs *cgs;

  byte tid;

  char nonstop;

  char username[36];
  char status[80];

#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif

  nonstop=FALSE;
  display_line=display_col=1;

  Puts(hu_is_on_1);
  Puts(hu_is_on_2);

  if ((cgs=ChatFindOpen())==NULL)
    return;

  while (ChatFindNext(cgs, &tid, username, status, NULL))
  {
    if (! tid)
      continue;

    Printf(hu_is_on_3, username, tid, status,
           eqstri(username, usrname) ? ch_you : blank_str);

    if (MoreYnBreak(&nonstop, CYAN))
      break;

    if (halt())
      break;
  }
  
  ChatFindClose(cgs);
}

static void near Chat_Toggle(void)
{
  word avail;

  if (! ChatFindIndividual(task_num, NULL, NULL, &avail))
    return;

  /* If user WAS available, make him unavailable */

  if (avail)
  {
    usr.bits |= BITS_NOTAVAIL;
    ChatSetStatus(FALSE, cs_notavail);
  }
  else
  {
    /* User was NOT available, so we should make him available */

    ChatSetStatus(TRUE, cs_avail);
    usr.bits &= ~BITS_NOTAVAIL;
  }
}


static void near Chat(int use_cb, int doing_answer)
{
  struct _css css;
  struct _cgs *cgs;

  char temp[PATHLEN];
  char prompt[PATHLEN];
  char nextw[21];
  char stat[PATHLEN];
  byte colour;

  byte tid;
  word avail, cc;
  word x, y, z;

  *nextw='\0';

#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif

  if (use_cb)
  {
    WhiteN();

    InputGets(temp, ch_enter_cb);
    cc=atoi(temp);

    if (cc < 1 || cc > 255)
      return;

    sprintf(stat, ch_chat_cb, cc);
    logit(log_cb_start, cc);
  }
  else
  {
    for (strcpy(temp, qmark); eqstri(temp, qmark); )
    {
      WhiteN();
      
      InputGets(temp, ch_enter_node);

      if (eqstri(temp, qmark))
        Who_Is_On();
    }

    x=atoi(temp);

    if (x < 1 || x > 255)
      return;

    /* Make sure that user can only chat with available nodes */
    
    if (x==task_num)
    {
      ChatCantPageSelf();
      return;
    }
    else if (! ChatFindIndividual((byte)x, temp, NULL, &avail) ||
             (!doing_answer && !avail))
    {
      ChatNotAvail(x);
      return;
    }

    /* Now calculate a unique channel number.  This number is unique, for  *
     * any two nodes talking to each other.                                */

    y=min(x, task_num);
    z=max(x, task_num);

    cc=(y*256)+z;

    sprintf(stat, ch_chat_pvt, x);
    logit(log_pvt_start, temp, x);
  }

  if (*temp && cc)
  {
    scan_time=0L; /* Force a check to be made */
    Check_For_Message(NULL, NULL);

    cur_ch=cc;

    /* Allocate mem for list of tasks listening to this channel. */

    if ((tasks=malloc(MAX_TASK*sizeof(int)))==NULL)
      return;

    /* Prepare to send a message to all tasks, asking if they're listening *
     * to this channel.                                                    */

    *(int *)temp=cur_ch;
    strcpy(temp+2, usrname);

    if ((cgs=ChatFindOpen())==NULL)
      return;

    while (ChatFindNext(cgs, &tid, NULL, NULL, &avail))
      if (tid && tid != task_num)
        if (ChatSendMsg(tid, CMSG_ENQ, strlen(temp+2)+3, temp)==-1)
          #ifdef DEBUG
            Printf("%MsgErr %d%",tid);
          #else
            ;
          #endif

    ChatFindClose(cgs);

    scan_time=CHAT_SCAN_TIME;
    in_node_chat=TRUE;

    ChatSaveStatus(&css);
    ChatSetStatus(FALSE, stat);

    Puts(ch_enter_chat);
    Puts(ch_help_str);

    for (;;)
    {
      colour=(byte)((task_num % 6)+9);

      if (colour==10)
        colour=15;

      sprintf(prompt, ch_byline, usrname, colour);

      if (*nextw)
        strcpy(temp, nextw);
      else *temp='\0';

      if (Input(temp, INPUT_NLB_LINE | INPUT_WORDWRAP,
                0, min(TermWidth()-2, PATHLEN-2)-19, prompt)==1)
      {
        strcpy(nextw, temp+strlen(temp)+1);
      }
      else *nextw='\0';

      if (eqstri(temp, "/w"))
      {
        Puts(ch_ulist_hdr);

        if (! num_task)
          Puts(ch_alone);

        for (x=0; x < num_task; x++)
          if (ChatFindIndividual((byte)tasks[x], temp, NULL, NULL))
          {
            Puts(temp);
            Putc('\n');
          }

        Putc('\n');
      }
      else if (eqstri(temp, "/s"))
      {
        Who_Is_On();
        Putc('\n');
      }
      else if (eqstri(temp, "/?") || eqstri(temp, qmark))
        Display_File(0, NULL, ss, PRM(misc_path), "CHATHELP");
      else if (eqstri(temp, "/q"))
        break;
      else    /* Must have been a text string */
      {
        /* Add the text entered to the message */
        strcat(prompt,temp);


        /* Shift the message over one byte, and insert the channel number */

        strocpy(prompt + sizeof(word), prompt);

        *(word *)prompt=cur_ch;

        for (x=0; x < num_task; x++)
          if (ChatSendMsg((byte)tasks[x], CMSG_CDATA, strlen(prompt+2)+3,
                          prompt)==-1)
          #ifdef DEBUG
            Printf("~MsgErr %d~", tasks[x]);
          #else
            ;
          #endif
      }
    }

    in_node_chat=FALSE;

    strcpy(temp, usrname);

    for (x=0; x < num_task; x++)
      if (ChatSendMsg((byte)tasks[x], CMSG_EOT, strlen(temp)+1, temp)==-1)
      #ifdef DEBUG
        Printf("@MsgErr %d@",tasks[x]);
      #else
        ;
      #endif

    num_task=0;

    free(tasks);
    tasks=NULL;

    ChatRestoreStatus(&css);

    scan_time=0L; /* Force a check to be made */
    Check_For_Message(NULL,NULL);

    scan_time=NORM_SCAN_TIME;
    logit(log_exit_chat);
  }

  cur_ch=-1;
}




static void near Add_To_Chat_List(byte tid)
{
  word x;

  #ifdef DEBUG
  Printf("@adding %d to chat list.\n",tid);
  #endif

  if (tasks && num_task < MAX_TASK)
  {
    /* Scan table for task number to add */

    for (x=0; x < num_task; x++)
      if (tasks[x]==tid)
        break;

    /* Don't add task, if it's already there! */

    if (x==num_task)
      tasks[num_task++]=tid;
  }
}




static int near Remove_From_Chat_List(byte tid)
{
  word x, y;

  if (!tasks)
    return FALSE;

  for (x=0; x < num_task; x++)
  {
    if (tasks[x]==(word)tid)
    {
      for (y=x; y < num_task-1; y++)
        tasks[y]=tasks[y+1];

      break;
    }
  }

  if (x==num_task)
    return FALSE;

  num_task--;
  return TRUE;
}

#if 0 /* obsolete */
int Header_Chat(int entry,int silent)
{
  word avail;

  NW(entry);
  
  if (! *linebuf && !silent)
  {
    Puts(ch_chat_sect);

    Who_Is_On();

    if (ChatFindIndividual(task_num, NULL, NULL, &avail))
      Printf(ch_us_avail, avail ? blank_str : ch_us_noavail);
  }

  restart_system=FALSE;
  return TRUE;
}
#endif

static void near R_Cleol(void);

static void near R_Cleol(void)
{
  if (usr.video==GRAPH_TTY)
    Printf("\r\x19 %c\r", current_col);
  else Printf("\r" CLEOL);
}


void ChatHandleMessage(byte tid, int type, int len, char *msg, int *redo)
{
  char *temp;

  NW(len);

  switch (type)
  {
    /* Generic Californian message-type (just display it).                  *
     *                                                                      *
     * Same as 'CMSG_PAGE', except that it doesn't beep automatically, and  *
     * it doesn't display the CHATPAGE.BBS file.                            */

    case CMSG_HEY_DUDE:   
      /* Space over, instead of CLEOL, for TTY callers */

      R_Cleol();
      Printf(szHeyDude, msg);

      *redo=TRUE;
      break;

    case CMSG_DISPLAY:
      /* Simply display the specified filespec */

      R_Cleol();
      Display_File(0, NULL, msg);
      vbuf_flush();
      break;

    case CMSG_PAGE:
      /* Space over, instead of CLEOL, for TTY callers */

      R_Cleol();
      Printf(szPageMsg, msg);

      *redo=TRUE;

      if ((temp=malloc(PATHLEN)) != NULL)
      {
        sprintf(temp, "%schatpage", PRM(misc_path));
        Display_File(0, NULL, temp);
        free(temp);
      }
      break;

    case CMSG_ENQ:          /* Respond to any incoming ENQs */
      #ifdef DEBUG
      Printf("@enq from tid %d --",tid);
      #endif

      /* If we're hooked up to the requested conference, then ACK it.  *
       * otherwise, ignore.                                            */

      if (*(word *)msg==cur_ch)
      {
        #ifdef DEBUG
        Printf(" sending ack.",tid);
        #endif


        if (ChatSendMsg(tid, CMSG_ACK, 0, msg)==-1)
        #ifdef DEBUG
          Printf("!MsgErr!");
        #else
          ;
        #endif

        Add_To_Chat_List(tid);

        R_Cleol();

        Printf(ch_xx_join, msg+2);

        *redo=TRUE;
      }

      #ifdef DEBUG
      Putc('\n');
      #endif
      break;

    case CMSG_ACK:  /* Remote ack'd our ENQ, so add to our chat list. */
      #ifdef DEBUG
      Printf("@ack from tid %d\n",tid);
      #endif

      Add_To_Chat_List(tid);
      break;

    case CMSG_EOT:  /* Someone is leaving conversation */
      #ifdef DEBUG
      Printf("@eot from tid %d.\n",tid);
      #endif

      if (Remove_From_Chat_List(tid))
      {
        R_Cleol();
        Printf(ch_xx_leave,msg);
        WhiteN();
        Puts(ch_help_str);

        *redo=TRUE;
      }
      break;

    case CMSG_CDATA:
      /* If we're not hooked up to this channel, then warn the bimbo   *
       * who sent the message!                                         */

      if (*(word *)msg != cur_ch)
      {
        #ifdef DEBUG
        Printf("#MsgUnSol %d# ",(int)*msg);
        #endif

        ChatSendMsg(tid, CMSG_EOT, strlen(usrname)+1, usrname);
      }
      else /* just display it! */
      {
        R_Cleol();
        Printf(ch_msg, msg+sizeof(word));
        *redo=TRUE;
      }
      break;
  }
}


    
static void near ChatNotAvail(int tid)
{
  Printf(ch_nodenavail, tid);
  Press_ENTER();
}

    
static void near ChatCantPageSelf(void)
{
  Puts(ch_you_dummy);
  Press_ENTER();
}



/* Send an all-points-bulletin to all nodes currently on-line */

void ChatAPB(char *msg)
{
  static struct _cgs *cgs;
  word avail;
  byte tid;

#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif
  
  if ((cgs=ChatFindOpen())==NULL)
    return;
  
  while (ChatFindNext(cgs, &tid, NULL, NULL, &avail))
    if (avail && tid != task_num)
      if (ChatSendMsg(tid, CMSG_HEY_DUDE, strlen(msg)+1, msg)==-1)
      #ifdef DEBUG
        Printf("(MsgErr %d)", tid);
      #else
        ;
      #endif
        
  ChatFindClose(cgs);

}

