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

#define INCL_PM
#define INCL_GPI
#define INCL_DOS
#define INCL_VIO
#define INCL_AVIO

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include "pos2.h"
#include "max.h"
#include "mcp.h"
#include "smrc.h"
#include "sm.h"
#include "pmdebug.h"

extern HWND hwndClient;
extern HPIPE hp;


#define DbgPrintf !!Error!!

/* Signal a fatal error */

static void near Fatal(int n)
{
  while (n--)
  {
    DosSleep(1000L);

    DosBeep(600L, 25);
    DosBeep(2600L, 50);
    DosBeep(600L, 25);
  }

  for (;;)
    ;
}

int CALLBACK RcIoMonitor(SMDISPATCH *psd)
{
  char *newstream;

  DbgPrintfp("Got RcIoMonitor");


  if ((newstream=malloc(psd->cbGot-sizeof(USHORT))) != NULL)
  {
    memmove(newstream, psd->pb, psd->cbGot-sizeof(USHORT));

    DbgPrintfp("Got video i/o for node %d (%d bytes)", *psd->pb,
               psd->cbGot-sizeof(USHORT));

    WinPostMsg(hwndClient, SM_VIO,
               MPFROM2SHORT(*psd->pb, psd->cbGot-sizeof(USHORT)),
               MPFROMP(newstream));
  }

  return 0;
}


/* Spool the video dump information to the caller */

int CALLBACK RcIoMonitorDump(SMDISPATCH *psd)
{
  USHORT usHeight, usWidth, usSize;
  char *newdump;

  DbgPrintfp("Got RcIoMonitorDump");

  /* ignore the task number up front */

  usHeight=psd->pb[1];
  usWidth=psd->pb[2];

  if ((newdump=malloc(usSize=usHeight * usWidth * sizeof(USHORT) + 5)) != NULL)
  {
    memmove(newdump, psd->pb, usSize);

    DbgPrintfp("Got video dump for node %d (%dx%d)", *psd->pb,
               usWidth, usHeight);

    WinPostMsg(hwndClient, SM_VIODUMP, MPFROMSHORT(*psd->pb),
               MPFROMP(newdump));
  }

  return 0;
}

int CALLBACK RcAck(SMDISPATCH *psd)
{
  NW(psd);
  DbgPrintfp("Got RcAck");
  return 0;
}

int CALLBACK RcBadNode(SMDISPATCH *psd)
{
  NW(psd);
  DbgPrintfp("Got RcBadNode");
  return 0;
}

int CALLBACK RcNoResources(SMDISPATCH *psd)
{
  NW(psd);
  DbgPrintfp("Got RcNoResources");
  return 0;
}

int CALLBACK RcCiaoBaby(SMDISPATCH *psd)
{
  NW(psd);
  DbgPrintfp("Got RcCiaoBaby");
  return 0;
}

int CALLBACK RcPing(SMDISPATCH *psd)
{
  /* We got a ping from a node */

  DbgPrintfp("Got ping for %d", *psd->pb);

  WinPostMsg(hwndClient, SM_PING, MPFROMSHORT(*psd->pb),
             MPFROMLONG(time(NULL)));

  return 0;
}

int CALLBACK RcOverview(SMDISPATCH *psd)
{
  struct _cstat *pcs, *newpcs;

  DbgPrintfp("Got RcOverview");

  pcs=(struct _cstat *)(psd->pb+1);

  if ((newpcs=malloc(sizeof *newpcs)) != NULL)
  {
    memmove(newpcs, pcs, sizeof *pcs);

    DbgPrintfp("Setting status for node %d", *psd->pb);

    WinPostMsg(hwndClient, SM_SETSTATUS, MPFROMSHORT(*psd->pb),
               MPFROMP(newpcs));
  }

  return 0;
}

/* Thread to monitor an MCP server */

void ThreadMon(char far *szPipe)
{
  SMDISPATCH sd;

  DISPATCH dispatch[]=
  {
    {RCMSG_ACK,             RcAck},
    {RCMSG_BADNODE,         RcBadNode},
    {RCMSG_NORESOURCES,     RcNoResources},
    {RCMSG_CIAO_BABY,       RcCiaoBaby},
    {RCMSG_IO_MONITOR,      RcIoMonitor},
    {RCMSG_IO_MONITOR_DUMP, RcIoMonitorDump},
    {RCMSG_PING,            RcPing},
    {RCMSG_OVERVIEW,        RcOverview},
    {0,                     0}
  };

  char *pcBuf;      /* Buffer for incoming bytes */
  USHORT rc;

  DbgPrintfp("Connecting to pipe '%s'", szPipe);

  /* Try to connect to the pipe */

  do
  {
    if ((rc=McpOpenPipe(szPipe, &hp)) != 0)
      DbgPrintfp("Pipe rc=%d (%s)", rc, szPipe);

    /* Wait for one minute if we could not open the pipe, since MCP
     * probably has not been started.
     */

    if (rc)
    {
      WinPostMsg(hwndClient, SM_PIPEWAIT, MPFROMLONG(1L), 0L);
      DosSleep(30000L);
    }
  }
  while (rc);

  /* Tell the PM thread that we are done waiting */

  WinPostMsg(hwndClient, SM_PIPEWAIT, MPFROMLONG(0L), 0L);

  if ((pcBuf=malloc(PIPE_MSG_SIZE))==NULL)
  {
    DbgPrintfp("pipemsg malloc failed", rc);
    Fatal(2);
  }

  if (McpSendMsg(hp, CLMSG_BEGINOVR, NULL, 0) != 0)
  {
    DbgPrintfp("can't McpSendMsg");
    Fatal(3);
  }

  /* Save a pointer to the buffer */

  sd.pb=pcBuf+sizeof(USHORT);

  /* Loop around, collecting messages from the MCP server */

  while (McpGetMsg(hp, pcBuf, &sd.cbGot, PIPE_MSG_SIZE)==0)
    if (McpDispatchMsg(pcBuf, sd.cbGot, dispatch, &sd) > 0)
      break;

  if (McpSendMsg(hp, CLMSG_ENDOVR, NULL, 0) != 0)
    Fatal(4);

  free(pcBuf);
  McpClosePipe(hp);

  DbgPrintfp("done thread");
}


/* Start a thread to monitor an MCP server */

TID StartMonitor(char *szPipeRoot)
{
  char *szPipe;
  char *pcStack;

  /* Connect to the client end of the pipe */

  if ((szPipe=malloc(strlen(szPipeRoot)+10))==NULL)
    return -1;

  strcpy(szPipe, szPipeRoot);
  strcat(szPipe, "\\client");

  if ((pcStack=malloc(THREAD_STK_SIZE))==NULL)
    return -1;

  return _beginthread(ThreadMon, pcStack, THREAD_STK_SIZE, (void *)szPipe);
}


