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

/* $Id: mcp_max.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#define INCL_DOS
#include <stdio.h>
#include <string.h>
#include "pos2.h"
#include "max.h"
#include "mcp_int.h"

extrn struct _ses_info *si;
extern int max_ses;

/* Store the user's status in the status holder */

int CALLBACK MSetStatus(PINFO *ppi)
{
  struct _cstat *pcs;

  pcs=(struct _cstat *)(ppi->pbBuf+sizeof(USHORT));
  si[ppi->session].cs=*pcs;

  /* Send status to overview monitors */

  OverviewSendStatus(si[ppi->session].tid, pcs);

  MCPLog("=%02d Max Node %d; User %s; Status %s",
         ppi->session,
         si[ppi->session].tid,
         si[ppi->session].cs.username,
         si[ppi->session].cs.status);

  return 0;
}


/* A new task has connected */

int CALLBACK MHello(PINFO *ppi)
{
  si[ppi->session].tid=ppi->pbBuf[2];

  /* CLear out the list of monitors for this task */

  memset(si[ppi->session].ahpMonitors, 0, sizeof(HPIPE)*MAX_MCP_MONITORS);

  MCPLog("=%02d Max Node %d on-line", ppi->session, si[ppi->session].tid);
  return 0;
}


/* A task has terminated.  This is sometimes called without a valid
 * message in ppi->pbBuf, since we may want to terminate the session
 * without getting advice from Max itself, such as when a Max session
 * is forced close and the pipe is broken.
 */

int CALLBACK MEOT(PINFO *ppi)
{
  if (si[ppi->session].tid)
  {
    MCPLog("=%02d Max Node %d off-line", ppi->session, si[ppi->session].tid);
    OverviewEnd(si[ppi->session].tid);
    si[ppi->session].tid=0;

    /* CLear out the list of monitors for this task */

    memset(si[ppi->session].ahpMonitors, 0, sizeof(HPIPE)*MAX_MCP_MONITORS);
  }

  return 0;
}


/* Ping so that we know that node is still active */

int CALLBACK MPing(PINFO *ppi)
{
#if 0
  MCPLog("!%02d Received Ping", ppi->session);
#else
  MCPLog(" %02d Max Ping", ppi->session);
#endif
  OverviewSendPing(si[ppi->session].tid);
  return 0;
}


/* Send a message from one Max node to another */

int CALLBACK MMaxSendMsg(PINFO *ppi)
{
  USHORT usGot;
  struct _cdat *pcd;
  int i;

  pcd=(struct _cdat *)(ppi->pbBuf+sizeof(USHORT));

  MCPLog("#%02d Max Dispatch msg %d -> %d", ppi->session, pcd->tid,
         pcd->dest_tid);

  /* Find the session number of the destination task */

  for (i=0; i < max_ses; i++)
    if (si[i].tid==pcd->dest_tid)
      break;

  if (i==max_ses)
  {
    MCPLog("!%02d Max Msg from node %d destined to unknown node %d!",
           ppi->session, pcd->tid, pcd->dest_tid);
    return 0;
  }

  usGot=ppi->cbMsg-sizeof(USHORT);

  McpSendMsg(si[i].ppi->hp, RPMSG_GOT_MSG, (PBYTE)pcd, usGot);
  /*rc=DosWrite(si[i].ppi->hp, (PBYTE)pcd, usGot, &usSent);*/

  MCPLog("#%02d Max Sent msg %d -> %d, len=%d", ppi->session, pcd->tid,
         pcd->dest_tid, usGot);

  return 0;
}

int CALLBACK MQueryTaskInfo(PINFO *ppi)
{
  OS2UINT rc, usGot;
  struct _cstat cs;
  int i;

  for (i=0; i < max_ses; i++)
    if (ppi->pbBuf[2]==si[i].tid)
      break;

  if (i==max_ses)
    memset(&cs, 0, sizeof cs);
  else memmove(&cs, &si[i].cs, sizeof cs);

  MCPLog(":%02d Max Query status for node %d (user %s)",
         ppi->session, ppi->pbBuf[2], cs.username);

  /* Write the chat status back to the pipe */

  rc=DosWrite(ppi->hp, &cs, sizeof cs, &usGot);

  if (rc)
    MCPLog("!%02d Max SYS%04d: DosWrite MMQueryActive",
           ppi->session, rc);

  return 0;
}



/* The caller wants a list of all currently-active Max nodes */

int CALLBACK MQueryTaskList(PINFO *ppi)
{
  OS2UINT usSent, rc;
  byte tids[255+2];
  int i, num_tid;

  /* Copy stuff into the task id array for the caller */

  for (i=0, num_tid=1; i < max_ses; i++)
    if (si[i].tid)
      tids[num_tid++]=si[i].tid;

  tids[0]=num_tid-1;

  MCPLog(":%02d Max Query task numbers (%d active)", ppi->session, num_tid-1);

  /* Send it back down the pipe */

  rc=DosWrite(ppi->hp, tids, num_tid, &usSent);

  if (rc)
    MCPLog("!%02d Max SYS%04d: DosWrite QA", ppi->session, rc);

  return 0;
}


/* Caller wants to perform video i/o */

int CALLBACK MVio(PINFO *ppi)
{
  int i;

  /* Stuff the task ID in the buffer, to send to the monitoring task */

  ppi->pbBuf[1]=si[ppi->session].tid;

  /* Send this to any installed monitor tasks */

  for (i=0; i < MAX_MCP_MONITORS; i++)
    if (si[ppi->session].ahpMonitors[i])
      McpSendMsg(si[ppi->session].ahpMonitors[i], RCMSG_IO_MONITOR,
                 ppi->pbBuf+1, ppi->cbMsg-1);

  return 0;
}

int CALLBACK MVioDump(PINFO *ppi)
{
  int i;

  /* Stuff the task ID in the buffer, to send to the monitoring task */

  ppi->pbBuf[1]=si[ppi->session].tid;


  /* Send this to any installed monitor tasks */

  for (i=0; i < MAX_MCP_MONITORS; i++)
    if (si[ppi->session].ahpMonitors[i])
    {
      McpSendMsg(si[ppi->session].ahpMonitors[i], RCMSG_IO_MONITOR_DUMP,
                 ppi->pbBuf+1, ppi->cbMsg-1);
    }

  return 0;
}


/* Caller wants to echo a log message */

int CALLBACK MLog(PINFO *ppi)
{
  int i;

#if 0
  MCPLog("!%02d Got log line \"%s\"", ppi->session, ppi->pbBuf+2);
#else
  char *p=strchr(ppi->pbBuf+2,'\n');
  if (p)
    *p='\0';
  MCPLog("%c%02d Max %s", *(ppi->pbBuf+2), ppi->session, ppi->pbBuf+13);
#endif

  /* Send this to any installed monitor tasks */

  for (i=0; i < MAX_MCP_MONITORS; i++)
    if (si[ppi->session].ahpMonitors[i])
      McpSendMsg(si[ppi->session].ahpMonitors[i], RCMSG_LOG,
                 ppi->pbBuf+2, ppi->cbMsg-2);

  return 0;
}

