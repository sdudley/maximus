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
static char rcs_id[]="$Id: max_chat.c,v 1.1.1.1 2002/10/01 17:51:29 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Chat module.  IPCxx.BBS writing and checking routines, and
    name=resident portion of inbound message handler.
*/

#define MAX_LANG_max_chat

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <mem.h>
#include <string.h>
#include "prog.h"
#include "mm.h"
#include "alc.h"
#include "max_chat.h"

#ifdef OS_2
#include "modem.h"
#include "pos2.h"
#endif

static struct _cstat cstat;

word cur_ch=-1;
word num_task=0;

word *tasks=NULL;

long scan_time=NORM_SCAN_TIME;

#ifdef MCP

  #define INCL_DOS
  #include "pos2.h"

  #include "mcp.h"

  HPIPE hpMCP;

  /* Send a ping to the server every two minutes to show that we're awake */

  void mcp_sleep(void)
  {
    static long time_ping=0L;

    if (hpMCP && timeup(time_ping))
    {
      McpSendMsg(hpMCP, PMSG_PING, NULL, 0);

      /* Five minute timeout */

      time_ping=timerset(2 * 60 * 100);
    }
  }

  void SendVideoDump(void)
  {
    VIO_DUMP_HDR *pdh;
    int i, size;
    byte *buf, *bufp;

    if ((buf=bufp=malloc(size=win->s_width * win->s_height * 2 + sizeof *pdh))==NULL)
    {
      logit(mem_none);
      return;
    }

    pdh=(VIO_DUMP_HDR *)bufp;

    pdh->bHeight=(BYTE)win->s_height;
    pdh->bWidth=(BYTE)win->s_width;
    pdh->bCurRow=(BYTE)win->row;
    pdh->bCurCol=(BYTE)win->col;
    pdh->bCurAttr=(BYTE)curattr;

    bufp += sizeof *pdh;

    for (i=0; i < win->s_height; i++)
    {
      memmove(bufp, win->rowtable[i], win->s_width*2);
      bufp += win->s_width*2;
    }

    /* Send the screen dump to the client */

    McpSendMsg(hpMCP, PMSG_VIO_DUMP, buf, size);

    free(buf);
  }


  /* Set our current activity status.  This sends a message to MCP and      *
   * sets our status in one of its internal fields.                         */

  void ChatSetStatus(int avail, char *status)
  {
    if (!hpMCP)
      return;

    cstat.avail=avail;
    strcpy(cstat.username, usrname);
    strcpy(cstat.status, status);

    McpSendMsg(hpMCP, PMSG_SET_STATUS, (BYTE *)&cstat, sizeof cstat);
  }


  /* Send an interprocess message directly to another Maximus node.  This   *
   * does not use the PipeSendMsg function for speed reasons; by directly   *
   * filling in the message type, we only need to shift the buffer          *
   * around in memory once, not twice.                                      */

  int ChatSendMsg(byte dest_tid, int type, int len, char *msg)
  {
    struct _cdat *pcd;
    BYTE *buf;
    OS2UINT rc, cbWritten;

    if (!hpMCP)
      return -1;

    if ((buf=malloc(len+sizeof(USHORT)+sizeof *pcd))==NULL)
    {
      logit(mem_none);
      return -1;
    }

    *(USHORT *)buf=PMSG_MAX_SEND_MSG;

    pcd=(struct _cdat *)(buf+sizeof(USHORT));
    pcd->tid=task_num;
    pcd->dest_tid=dest_tid;
    pcd->type=type;
    pcd->len=len;

    memmove(buf+sizeof(USHORT)+sizeof *pcd, msg, len);

    if ((rc=DosWrite(hpMCP, buf, len + sizeof(USHORT) + sizeof *pcd,
                     &cbWritten)) != 0)
    {
      logit("!SYS%04d: DosWrite MCP 2", rc);
      return -1;
    }

    free(buf);

    return 0;
  }


  /* Begin an IPC session with MCP */

  int ChatOpenMCP(void)
  {
    char szPipe[PATHLEN];
    OS2UINT rc, usAction;
    int fFirst;
    int fOpened;
    byte tid=task_num;
    long lTryTime;

    /* IPC already open */

    if (hpMCP)
      return -1;


    /* The place for us to connect is the "\maximus" side off the root
     * MCP "path".
     */

    if (! *szMcpPipe)
      strcpy(szMcpPipe, "\\pipe\\maximus\\mcp");

    strcpy(szPipe, szMcpPipe);
    strcat(szPipe, "\\maximus");

    fFirst = TRUE;

    /* Try to start MCP for up to five seconds */

    lTryTime = timerset(500);

    do
    {
      fOpened = FALSE;

#ifdef __FLAT__
      rc = DosOpen(szPipe, &hpMCP, &usAction, 0L, FILE_NORMAL,
                   OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                   OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE |
                   OPEN_FLAGS_NOINHERIT, NULL);
#else
      rc=DosOpen(szPipe, &hpMCP, &usAction, 0, FILE_NORMAL, FILE_OPEN,
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
                   OPEN_FLAGS_NOINHERIT, 0);
#endif

      if (rc==0)
        fOpened = TRUE;
      else
      {
        if (!fFirst)
          DosSleep(100L);
        else
        {
          extern HCOMM hcModem;
          RESULTCODES rcd;
          ULONG ulState;
          HFILE hf;
          char szFailObj[PATHLEN];
          char szMcpString[PATHLEN];
          char *psz;

          logit(log_starting_mcp);

          fFirst = FALSE;

          strcpy(szMcpString, "mcp.exe");
          psz = szMcpString + strlen(szMcpString) + 1;

          sprintf(psz,
                  ". %s %d server\0",
                  szMcpPipe,
                  prm.mcp_sessions);

          /* Add an extra nul at the end as per convention */

          psz[strlen(psz)] = 0;


          /* Now ensure that MCP doesn't inherit our modem handle */

          if (!local)
          {
            hf=ComGetFH(hcModem);

            DosQueryFHState(hf, &ulState);
            ulState &= 0x7f88;
            DosSetFHState(hf, ulState | OPEN_FLAGS_NOINHERIT);
          }

          /* Try to start MCP as a detached process */

          rc = DosExecPgm(szFailObj,
                          PATHLEN,
                          EXEC_BACKGROUND,
                          szMcpString,
                          NULL,
                          &rcd,
                          szMcpString);

          /* Restore the modem handle back to its original settings */

          if (!local)
            DosSetFHandState(hf, ulState);


          /* If we can't start MCP server, put a msg in the log */

          if (rc)
          {
            logit(log_mcp_err_2, rc);
            break;
          }
        }
      }
    }
    while (!fOpened && !timeup(lTryTime));

    if (fOpened)
      McpSendMsg(hpMCP, PMSG_HELLO, &tid, 1);
    else
    {
      hpMCP = 0;
      logit(log_no_mcp, szMcpPipe);
    }

    return 0;
  }


  /* Terminate the IPC session */

  void ChatCloseMCP(void)
  {
    if (hpMCP)
    {
      McpSendMsg(hpMCP, PMSG_EOT, NULL, 0);
      DosClose(hpMCP);
      hpMCP=0;
    }
  }

  /* This function is called to dispatch messages of types other            *
   * than PMSG_MAX_SEND_MSG.                                                */

  int usStrokes;
  byte cbStrokeBuf[MAX_BUF_STROKE];

  static void near ChatDispatchMsg(byte *pbMsg, USHORT cbMsg)
  {
    USHORT usType=*(USHORT *)pbMsg;
    USHORT usAdd;

    if (!hpMCP)
      return;

    switch (usType)
    {
      case RPMSG_HAPPY_DAGGER:
        logit("!MCP-initiated shutdown");
        ChatSetStatus(FALSE, "Terminating...");
        quit(ERROR_KEXIT);
        break;

      case RPMSG_MONITOR:
        mcp_video=!!pbMsg[2];

        /* If we just enabled video monitoring, send a dump of our          *
         * video buffer.                                                    */

        if (mcp_video)
          SendVideoDump();
        break;

      /* If the user hit ^c... */

      case RPMSG_CTRLC:
      case RPMSG_BREAK:
        brk_trapped++;
        break;

      /* Add characters to our local keyboard buffer */

      case RPMSG_KEY:
        cbMsg -= 2;
        usAdd=min(MAX_BUF_STROKE-usStrokes, cbMsg);

        memmove(cbStrokeBuf + usStrokes, pbMsg+sizeof(USHORT),
                usAdd);

        usStrokes += usAdd;
        break;
    }
  }


  /* Retrieve a message from the MCP server */

  static int near ChatGetMsg(byte *tid,int *type,int *len,char *msg,int maxlen)
  {
    AVAILDATA ad;
    OS2UINT fsState;
    OS2UINT rc, usGot;
    BYTE buf;

    if (!hpMCP)
      return -1;

    if ((rc=DosPeekNmPipe(hpMCP, &buf, 1, &usGot, &ad, &fsState)) != 0)
      return -1;

    /* No messages available */

    if (!usGot)
      return 1;

    if (ad.cbmessage > maxlen)
    {
      logit("!MCP message truncated (%d)", ad.cbmessage);
      ad.cbmessage=maxlen;
    }

    if ((rc=DosRead(hpMCP, msg, ad.cbmessage, &usGot)) != 0)
      logit("!SYS%04d: MCP DosRead", rc);
    else if (usGot != ad.cbmessage)
      logit("!Could not read all MCP data from pipe %d/%d", usGot,
            ad.cbmessage);
    else
    {
      struct _cdat *pcd;

      if (*(USHORT *)msg==RPMSG_GOT_MSG)
      {
        /* Extract the header data */

        pcd=(struct _cdat *)(msg+sizeof(USHORT));
        *tid=pcd->tid;
        *type=pcd->type;
        *len=pcd->len;

        /* Now shift the message back to hide the header data */

        memmove(msg, msg+sizeof(USHORT)+sizeof *pcd, min(ad.cbmessage, *len));

        return 0;
      }
      else ChatDispatchMsg(msg, ad.cbmessage);
    }

    return 1;
  }

#else
  void ChatSetStatus(int avail, char *status)
  {
    char temp[PATHLEN];

    int flag,
        fd;

    if (! *PRM(ipc_path))
      return;

    sprintf(temp, ipcxx_bbs, PRM(ipc_path), task_num);

    if (fexist(temp))
    {
      if ((fd=ChatOpenIPC(task_num))==-1)
        return;

      flag=TRUE;
    }
    else
    {
      if ((fd=sopen(temp,O_WRONLY | O_CREAT | O_TRUNC | O_BINARY | O_NOINHERIT,
                    SH_DENYNONE,S_IREAD | S_IWRITE))==-1)
        return;

      flag=FALSE;
    }

    if (read(fd, (char *)&cstat, sizeof(struct _cstat)) < sizeof(struct _cstat))
      memset(&cstat, '\0', sizeof(struct _cstat));

    cstat.avail=avail;
    strcpy(cstat.username, usrname);
    strcpy(cstat.status, status);

    if (cstat.msgs_waiting==0)
      cstat.next_msgofs=(long)sizeof(struct _cstat);

    lseek(fd, 0L, SEEK_SET);
    write(fd, (char *)&cstat, sizeof(struct _cstat));

    if (flag)
      ChatCloseIPC(fd);
    else close(fd);
  }



  int ChatSendMsg(byte tid,int type,int len,char *msg)
  {
    struct _cdat cdat;
    struct _cstat cs;

    int fd;

    if ((fd=ChatOpenIPC(tid))==-1)
      return -1;

    read(fd, (char *)&cs, sizeof(struct _cstat));

    cs.msgs_waiting++;

    cdat.tid=task_num;
    cdat.type=type;
    cdat.len=len;

    /* Only update the stats in the _cstat structure if this is the 1st msg */

    if (cs.msgs_waiting != 1)
      lseek(fd, cs.new_msgofs, SEEK_SET);  /* Seek to the next message */
    else
    {
      /* Otherwise write over the new stuff */
      cs.next_msgofs=sizeof(struct _cstat);
      lseek(fd,sizeof(struct _cstat),SEEK_SET);
    }


    write(fd, (char *)&cdat, sizeof(struct _cdat));
    write(fd,msg,len);

    cs.new_msgofs=tell(fd);

    lseek(fd, 0L, SEEK_SET);
    write(fd, (char *)&cs, sizeof(struct _cstat));

    ChatCloseIPC(fd);

    return 0;
  }




  static int near ChatGetMsg(byte *tid, int *type, int *len, char *msg, int maxlen)
  {
    struct _cstat *cs;
    struct _cdat cdat;

    word err=1;
    int fd;

    if ((fd=ChatOpenIPC(task_num))==-1)
      return -1;

    if ((cs=malloc(sizeof(struct _cstat))) != NULL &&
        read(fd, (char *)cs, sizeof(struct _cstat))==sizeof(struct _cstat) &&
        cs->msgs_waiting)
    {
      err=0;

      cs->msgs_waiting--;

      if (lseek(fd, cs->next_msgofs, SEEK_SET)==(long)cs->next_msgofs &&
          read(fd, (char *)&cdat, sizeof(struct _cdat))==sizeof(struct _cdat))
      {
        *tid=(byte)cdat.tid;
        *type=cdat.type;
        *len=cdat.len;

        read(fd, msg, min((word)maxlen, cdat.len));
      }
      else
      {
        *tid=(byte)-1;
        *type=-1;
        *len=0;
      }


      /* To ensure that the cstat file hasn't been corrupted */

      if (cs->msgs_waiting > 5000)
        cs->msgs_waiting=0;

      /* Now update the offset */

      if (cs->msgs_waiting)
        cs->next_msgofs=tell(fd);
      else
      {
        cs->next_msgofs=(long)sizeof(struct _cstat);
        cs->new_msgofs=(long)sizeof(struct _cstat);
      }

      lseek(fd, 0L, SEEK_SET);
      write(fd, (char *)cs, sizeof(struct _cstat));
    }

    if (cs)
      free(cs);

    ChatCloseIPC(fd);

    return err;
  }



  int ChatOpenIPC(byte tid)
  {
    char *fname;

    int tries, fd;

    if (! *PRM(ipc_path))
    {
      #ifdef DEBUG
      Printf("IPC:nopath ");
      #endif

      return -1;
    }

    if ((fname=malloc(PATHLEN))==NULL)
      return -1;

    sprintf(fname, ipcxx_bbs, PRM(ipc_path), tid);

    if ((fd=shopen(fname, O_RDWR | O_BINARY | O_NOINHERIT))==-1)
    {
      #ifdef DEBUG
      Printf("IPC:openerr ");
      #endif
    }
    else if ((prm.flags2 & FLAG2_noshare)==0)
    {
      for (tries=MAX_TRIES; tries-- && lock(fd,0L,1L)==-1; )
        Giveaway_Slice();

      if (tries <= 0)   /* Blocked out */
      {
        #ifdef DEBUG
        Printf("IPC:max ");
        #endif

        close(fd);
        fd=-1;
      }
    }

    if (fd != -1)
      lseek(fd, 0L, SEEK_SET);

    free(fname);

    return fd;
  }



  void ChatCloseIPC(int fd)
  {
    if (! *PRM(ipc_path))
      return;

    if (! (prm.flags2 & FLAG2_noshare))
      unlock(fd, 0L, 1L);

    close(fd);
  }
#endif


/* These two functions save and restore the current CHAT availability and   *
 * status.                                                                  */

void ChatSaveStatus(struct _css *css)
{
#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif

  css->avail=cstat.avail;
  strcpy(css->status, cstat.status);
}



void ChatRestoreStatus(struct _css *css)
{
#ifndef MCP
  if (! *PRM(ipc_path))
    return;
#endif

  ChatSetStatus(css->avail, css->status);
}


void Check_For_Message(char *s1,char *s2)
{
  int redo, type, len;
  time_t now;
  byte tid;

  char *msg;
  static long last_check=0L;

#ifdef MCP
  static long last_received=0x7fffffffL;
#endif

  now=time(NULL);

  if (
#ifdef MCP
  hpMCP &&
#else
  *PRM(ipc_path) &&
#endif
      fLoggedOn && task_num &&
      (now >= last_check+scan_time
#ifdef MCP  // If we received a msg within last 20 seconds, poll continuously
      || now <= last_received+20
#endif
      ))
  {
    last_check=time(NULL);

    redo=FALSE;

    if ((msg=malloc(PATHLEN)) != NULL)
    {
      while (ChatGetMsg(&tid, &type, &len, msg, PATHLEN)==0)
      {
#ifdef MCP
        last_received=now;
#endif
        ChatHandleMessage(tid, type, len, msg, &redo);
      }

      free(msg);
    }

    if (redo)
    {
      if (s1 && s2)
        Printf(ss, s1, s2);

      vbuf_flush();
    }
  }
}

