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

/* $Id: mcp_cli.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#define INCL_DOS
#include <string.h>
#include "pos2.h"
#include "max.h"
#include "mcp_int.h"

extern struct _ses_info *si;
extern int max_ses;

static HPIPE ahpOverview[MAX_MCP_MONITORS];  /* Monitors for overview info */
static USHORT uiOverviews;                    /* Number of active monitors */

/* A Max task sent a ping -- propagate this to all overview monitors */

void OverviewSendPing(byte tid)
{
  int i;

  for (i=0; i < uiOverviews; i++)
  {
    MCPLog("!CL Echoing ping from node %d to ovr[%d]", tid, i);
    McpSendMsg(ahpOverview[i], RCMSG_PING, &tid, sizeof(tid));
  }
}

/* Retransmit the status of this node to everyone */

void OverviewSendStatus(byte tid, struct _cstat *pcs)
{
  char acBuf[sizeof(*pcs) + 1];
  int i;

  /* Send the chat status to all overview monitors */

  acBuf[0]=tid;
  memmove(acBuf+1, pcs, sizeof(*pcs));

  for (i=0; i < uiOverviews; i++)
  {
    MCPLog("!CL Sending overview for task %d to ovr[%d]", tid, i);
    McpSendMsg(ahpOverview[i], RCMSG_OVERVIEW, acBuf, sizeof(acBuf));
  }
}



/* Indicate to an overview monitor that a Max node is off-line */

void OverviewEnd(byte tid)
{
  struct _cstat cs;

  memset(&cs, 0, sizeof cs);

  OverviewSendStatus(tid, &cs);
}



/* Send the status of all currently-active nodes */

void OverviewSendAllStatus(void)
{
  int i;

  for (i=0; i < max_ses; i++)
    if (si[i].tid)
      OverviewSendStatus(si[i].tid, &si[i].cs);
}


/* Set a flag in the session parameters telling MCP to start sending        *
 * monitoring information to our client thread.                             */

int CALLBACK ClBeginMonitor(CINFO *pci)
{
  int ses, any_others, i;
  byte tid=pci->pbBuf[2];

  /* Scan for the indicated task number */

  for (ses=0; ses < max_ses; ses++)
    if (si[ses].tid==tid && tid)
      break;

  MCPLog("=CL Begin monitoring of task %d", tid);

  if (ses==max_ses)
  {
    MCPLog("!CL Node %d not active!", tid);
    McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
    return 0;
  }

  /* Try to add our pipe to monitor list */

  for (i=any_others=0; i < MAX_MCP_MONITORS; i++)
    if (!si[ses].ahpMonitors[i])
    {
      /* Store our pipe in the "list of monitoring pipes */

      si[ses].ahpMonitors[i]=pci->hp;

      MCPLog("=CL Monitoring of task %d established", tid);
      McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
      break;
    }

  if (i==MAX_MCP_MONITORS)
  {
    MCPLog("!CL Too many monitors on node %d!", tid);
    McpSendMsg(pci->hp, RCMSG_NORESOURCES, NULL, 0);
  }
  else
  {
    byte msg=1;

    /* Tell the Max task to send initial screen dump and commence monitoring */

    McpSendMsg(si[ses].ppi->hp, RPMSG_MONITOR, &msg, 1);
  }

  return 0;
}


/* Send a message directly to a Maximus task */

int CALLBACK ClMaxSendMsg(CINFO *pci)
{
  struct _cdat *pcd;
  int ses;

  /* Pull the header out of the message */

  pcd=(struct _cdat *)(pci->pbBuf+2);

  /* Scan for the indicated task number */

  for (ses=0; ses < max_ses; ses++)
    if (si[ses].tid==pcd->dest_tid)
      break;

  if (ses==max_ses)
  {
    MCPLog("!CL Node %d not active!", pcd->dest_tid);
    McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
    return 0;
  }

  /* Now forward the message to that Max task */

  MCPLog("!CL Send msg to node %d!", pcd->dest_tid);
  McpSendMsg(si[ses].ppi->hp, RPMSG_GOT_MSG, (void *)pcd, pci->cbMsg-2);

  return 0;
}



int CALLBACK ClKey(CINFO *pci)
{
  byte dest_tid=pci->pbBuf[2];
  int i;

  for (i=0; i < max_ses; i++)
    if (si[i].tid==dest_tid)
      break;

  if (i==max_ses)
  {
    MCPLog("!CL Unknown destination for ClKey");
    McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
    return 0;
  }

  /* Send the keystrokes to the session in question */

  McpSendMsg(si[i].ppi->hp, RPMSG_KEY, pci->pbBuf+sizeof(USHORT)+1,
             pci->cbMsg-3);

  McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);

  /*MCPLog("!CL Sent %d strokes to session %d", pci->cbMsg-3, i);*/
  return 0;
}


/* Tell the Max task to terminate */

int CALLBACK ClHappyDagger(CINFO *pci)
{
  byte dest_tid=pci->pbBuf[2];
  int i;

  for (i=0; i < max_ses; i++)
    if (si[i].tid==dest_tid)
      break;

  if (i==max_ses)
  {
    MCPLog("!CL Unknown destination for ClHappyDagger");
    McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
    return 0;
  }

  /* Send the keystrokes to the session in question */

  McpSendMsg(si[i].ppi->hp, RPMSG_HAPPY_DAGGER, NULL, 0);
  McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);

  MCPLog("!CL Sent RPMSG_HAPPY_DAGGER to session %d", i);
  return 0;
}


/* Stop monitoring a specific node */

int CALLBACK ClEndMonitor(CINFO *pci)
{
  int ses, i;
  byte tid=pci->pbBuf[2];

  for (ses=0; ses < max_ses; ses++)
    if (si[ses].tid==tid)
      break;

  if (ses==max_ses)
  {
    MCPLog("!CL Unknown destination for ClEndMonitor");
    McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
    return 0;
  }

  for (i=0; i < MAX_MCP_MONITORS; i++)
    if (si[ses].ahpMonitors[i]==pci->hp)
    {
      byte msg=0; /* tell Max to stop monitoring */

      McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
      McpSendMsg(si[ses].ppi->hp, RPMSG_MONITOR, &msg, 1);
      si[ses].ahpMonitors[i]=0;
      return 0;
    }

  McpSendMsg(pci->hp, RCMSG_NORESOURCES, NULL, 0);
  return 0;
}


/* Begin overview.  This tells MCP to send any status information           *
 * regarding all of the Max nodes to the named client.                      */

int CALLBACK ClBeginOvr(CINFO *pci)
{
  MCPLog("!CL Sending overview information");

  /* Send a NAK back if we can't handle it */

  if (uiOverviews >= MAX_MCP_MONITORS)
  {
    MCPLog("!CL Too many overview monitors!");
    McpSendMsg(pci->hp, RCMSG_NORESOURCES, NULL, 0);
    return 0;
  }

  /* Add our pipe to the overview counter */

  ahpOverview[uiOverviews++]=pci->hp;

  McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
  OverviewSendAllStatus();
  return 0;
}


/* End overview - stop sending overview information */

int CALLBACK ClEndOvr(CINFO *pci)
{
  int i;

  for (i=0; i < uiOverviews; i++)
    if (ahpOverview[i]==pci->hp)
    {
      memmove(ahpOverview+i, ahpOverview+i+1, (uiOverviews-i-1) * sizeof(HPIPE));
      McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
      return 0;
    }

  McpSendMsg(pci->hp, RCMSG_BADNODE, NULL, 0);
  return 0;
}


/* This is a cleanup routine which is called whenever a client              *
 * disconnects.  ByeBye makes sure that we have been disconnected           *
 * from all MCP resources.                                                  */

int CALLBACK ClByeBye(CINFO *pci)
{
  int ses;
  int i;

  for (ses=0; ses < max_ses; ses++)
    if (si[ses].tid)
      for (i=0; i < MAX_MCP_MONITORS; i++)
        if (si[ses].ahpMonitors[i]==pci->hp)
          si[ses].ahpMonitors[i]=0;

  for (i=0; i < uiOverviews; i++)
    if (ahpOverview[i]==pci->hp)
      memmove(ahpOverview+i, ahpOverview+i+1, --uiOverviews-i);

  McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
  return 0;
}


/* Start the specified Max session number */

int CALLBACK ClStart(CINFO *pci)
{
  OS2UINT rc;
  OS2UINT uiSID, uiPID;
  STARTDATA sd;
  char szTitle[PATHLEN];
  char szInputs[PATHLEN];
  char szPgmName[PATHLEN];
  char szCurDir[PATHLEN];
  OS2UINT usDisk, usLen;
  ULONG ulMap;

  sprintf(szTitle, "%d - Maximus", (int)pci->pbBuf[2]);

#ifdef __FLAT__
  rc=DosSearchPath(SEARCH_ENVIRONMENT, "PATH", "cmd.exe", szPgmName, PATHLEN);
#else
  rc=DosSearchPath(DSP_ENVIRONMENT, "PATH", "cmd.exe", szPgmName, PATHLEN);
#endif

  if (rc != 0)
  {
    MCPLog("!CL DosSearchPath rc=%d", rc);
    McpSendMsg(pci->hp, RCMSG_NORESOURCES, NULL, 0);
  }

  DosQCurDisk(&usDisk, &ulMap);

  szCurDir[0]='A'+usDisk-1;
  szCurDir[1]=':';
  szCurDir[2]='\\';
  usLen=PATHLEN - 4;
  DosQCurDir(usDisk, szCurDir+3, &usLen);
  Add_Trailing(szCurDir, '\\');

  sprintf(szInputs, "/c %smcpstart.cmd %d", szCurDir, (int)pci->pbBuf[2]);

  memset(&sd, 0, sizeof sd);

  sd.Length=50;   /* we don't use the full structure */
  sd.Related=FALSE;
  sd.FgBg=TRUE;         /* background */
  sd.TraceOpt=0;        /* no tracing */
  sd.PgmTitle=szTitle;
  sd.PgmName=szPgmName;
  sd.PgmInputs=szInputs;
  sd.TermQ=0;           /* no termination queue */
  sd.Environment=0;     /* inherit default environment */
  sd.InheritOpt=0;      /* don't inherit our file handles */
  sd.SessionType=0;     /* default session type */
  sd.IconFile=0;        /* no icon */
  sd.PgmHandle=0;       /* no program handle */
  sd.PgmControl=4;      /* start up minimized */
  sd.InitXPos=0;
  sd.InitYPos=0;
  sd.InitXSize=0;

  rc=DosStartSession(&sd, &uiSID, &uiPID);

  if (rc)
  {
    MCPLog("!CL Can't start session %d - rc=%d!", (int)pci->pbBuf[2], rc);
    McpSendMsg(pci->hp, RCMSG_NORESOURCES, NULL, 0);
  }
  else
  {
    MCPLog("!CL Started session %d!", (int)pci->pbBuf[2]);
    McpSendMsg(pci->hp, RCMSG_ACK, NULL, 0);
  }

  return 0;
}



/* Process I/O requests from the client until it disconnects */

void ClientIO(HPIPE hp)
{
  CINFO ci;

  DISPATCH dt[]=
  {
    {CLMSG_BEGINMONITOR,    ClBeginMonitor},
    {CLMSG_ENDMONITOR,      ClEndMonitor},
    {CLMSG_BEGINOVR,        ClBeginOvr},
    {CLMSG_ENDOVR,          ClEndOvr},
    {CLMSG_BYEBYE,          ClByeBye},
    {CLMSG_KEY,             ClKey},
    {CLMSG_HAPPY_DAGGER,    ClHappyDagger},
    {CLMSG_START,           ClStart},
    {CLMSG_MAX_SEND_MSG,    ClMaxSendMsg},
    {0,                     0}
  };

  ci.hp=hp;

  if ((ci.pbBuf=malloc(PIPE_MSG_SIZE))==NULL)
  {
    MCPLog("!CL Error allocating client pipe buffer!");
    DosExit(EXIT_THREAD, 0);
  }

  ci.cbBufLen=PIPE_MSG_SIZE;

  while (McpGetMsg(ci.hp, (PVOID)ci.pbBuf, &ci.cbMsg, ci.cbBufLen)==0)
    if (McpDispatchMsg(ci.pbBuf, ci.cbMsg, dt, &ci))
      /*break*/;

  /* Make sure that we have disconnected from all of the pipes */

  ClByeBye(&ci);

  free(ci.pbBuf);
}


