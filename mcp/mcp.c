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

/* $Id: mcp.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#define INCL_ERRORS
#define INCL_DOS
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <errno.h>
#include <stdlib.h>
#include "pos2.h"
#include "max.h"
#include "mcp_int.h"

struct _ses_info *si;
int max_ses;
static char szPipeName[PATHLEN];
static char szClientPipeName[PATHLEN];

/* Main handler thread for each Maximus/MCP pipe */

void far PipeMain(void far *arg)
{
  USHORT rc;
  PINFO *ppi;

  static DISPATCH dt[]=
  {
    {PMSG_SET_STATUS,       MSetStatus},
    {PMSG_MAX_SEND_MSG,     MMaxSendMsg},
    {PMSG_QUERY_TASKINFO,   MQueryTaskInfo},
    {PMSG_QUERY_TASKLIST,   MQueryTaskList},
    {PMSG_HELLO,            MHello},
    {PMSG_EOT,              MEOT},
    {PMSG_VIO,              MVio},
    {PMSG_VIO_DUMP,         MVioDump},
    {PMSG_PING,             MPing},
    {PMSG_LOG,              MLog},
    {0,                     0}
  };


  if ((ppi=malloc(sizeof(*ppi)))==NULL)
  {
    MCPLog("!Out of memory in PipeMain 1");
    DosExit(EXIT_PROCESS, 1);
  }

  if ((ppi->pbBuf=malloc(PIPE_MSG_SIZE))==NULL)
  {
    MCPLog("!Out of memory in PipeMain 2");
    DosExit(EXIT_PROCESS, 1);
  }

  ppi->cbBufLen=PIPE_MSG_SIZE;
  ppi->session=(int)arg;

  /* Store a copy of our pipe info structure */

  memset(si+ppi->session, 0, sizeof *si);
  si[ppi->session].ppi=ppi;

#ifdef __FLAT__
  rc=DosCreateNPipe(szPipeName,
                    &ppi->hp,
                    NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_WRITEBEHIND,
                    NP_WAIT | NP_UNLIMITED_INSTANCES | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE,
                    4096,    /* output buffer */
                    8192,    /* input buffer */
                    250);
#else
  rc=DosMakeNmPipe(szPipeName,
                   &ppi->hp,
                   NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_WRITEBEHIND,
                   NP_WAIT | NP_UNLIMITED_INSTANCES | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE,
                   4096,    /* output buffer */
                   8192,    /* input buffer */
                   250);
#endif

  if (rc)
  {
    MCPLog("!%02d PipeMain: SYS%04d: DosMakeNmPipe(\"%s\")",
           ppi->session, rc, szPipeName);

    DosExit(EXIT_PROCESS, 1);
  }

  /* Now wait for an infinite number of clients to connect */

  for (;;)
  {
    MCPLog("+%02d Waiting for client", ppi->session);

#ifdef __FLAT__
    if ((rc=DosConnectNPipe(ppi->hp)) != 0)
#else
    if ((rc=DosConnectNmPipe(ppi->hp)) != 0)
#endif
      MCPLog("!%02d PipeMain: SYS%04d: DosConnectNmPipe",
             ppi->session, rc);

    MCPLog("=%02d Connected to client", ppi->session);

    while (McpGetMsg(ppi->hp, (PVOID)ppi->pbBuf, &ppi->cbMsg, ppi->cbBufLen)==0)
    {
      int rc;

      if ((rc=McpDispatchMsg(ppi->pbBuf, ppi->cbMsg, dt, ppi))==-1)
        MCPLog("!%02d Received unknown message #%d", ppi->session, *(USHORT *)ppi->pbBuf);
      else if (rc > 0)
      {
        MCPLog("!%02d DispatchMsg returned %d", ppi->session, rc);
        break;
      }
    }

    /* Make sure that the node is recognized as being off-line, jsut in
     * case the session was closed without Max's knowledge.
     */

    MEOT(ppi);

#ifdef __FLAT__
    DosDisConnectNPipe(ppi->hp);
#else
    DosDisConnectNmPipe(ppi->hp);
#endif

    DosSleep(1);
    MCPLog("+%02d Recycling");
  }

  /* free(ppi->pbBuf); */
  /* free(ppi); */
  /* DosClose(ppi->hp); */
}



/* Start a pipe-handling thread */

static void near StartThread(void (far *f)(void far *arg), int session,
                             void *arg)
{
  PVOID pv;
  USHORT rc;


#ifdef __FLAT__
  if ((rc=DosAllocMem(&pv, THREAD_STK_SIZE, PAG_COMMIT | PAG_READ | PAG_WRITE)) != 0)
  {
    MCPLog("!%02d StartThread: SYS%04d: DosAllocMem(%u)", session, rc,
           THREAD_STK_SIZE);
    DosExit(EXIT_PROCESS, 1);
  }
#else
  SEL sel;

  if ((rc=DosAllocSeg(THREAD_STK_SIZE, &sel, SEG_NONSHARED)) != 0)
  {
    MCPLog("!%02d StartThread: SYS%04d: DosAllocSeg(%u)", session, rc,
           THREAD_STK_SIZE);
    DosExit(EXIT_PROCESS, 1);
  }

  pv = MAKEP(sel, 0);
#endif

  if ((rc=_beginthread(f, pv, THREAD_STK_SIZE, (void far *)arg))==-1)
  {
    MCPLog("!%02d StartThread: _beginthread failed (%d)", session, errno);
    DosExit(EXIT_PROCESS, 1);
  }
}




/* ClientMain; A copy of this thread is started every time a new client     *
 * attaches to MCP, and also once at MCP's IPL.                             */

void far ClientMain(void far *arg)
{
  HPIPE hp;
  USHORT rc;

  (void)arg;

  MCPLog("!ClientMain starting");

#ifdef __FLAT__
  rc=DosCreateNPipe(szClientPipeName,
                    &hp,
                    NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_WRITEBEHIND,
                    NP_WAIT | NP_UNLIMITED_INSTANCES | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE,
                    4096,    /* output buffer */
                    8192,    /* input buffer */
                    250);

#else
  rc=DosMakeNmPipe(szClientPipeName,
                   &hp,
                   NP_ACCESS_DUPLEX | NP_NOINHERIT | NP_WRITEBEHIND,
                   NP_WAIT | NP_UNLIMITED_INSTANCES | NP_TYPE_MESSAGE | NP_READMODE_MESSAGE,
                   4096,    /* output buffer */
                   8192,    /* input buffer */
                   250);
#endif

  if (rc)
  {
    MCPLog("!CL SYS%04d: DosMakeNmPipe(\"%s\")",
           rc, szPipeName);

    DosExit(EXIT_PROCESS, 1);
  }

  /* Now wait for an infinite number of clients to connect */

  MCPLog("+CL Waiting for client to connect");

#ifdef __FLAT__
  if ((rc=DosConnectNPipe(hp)) != 0)
#else
  if ((rc=DosConnectNmPipe(hp)) != 0)
#endif
    MCPLog("!CL SYS%04d: DosConnectNmPipe",
           rc);

  MCPLog("=CL Connected to client; spawning new thread");

  StartThread(ClientMain, 999, NULL);

  ClientIO(hp);

  MCPLog("=CL Disconnecting from client");

#ifdef __FLAT__
  DosDisConnectNPipe(hp);
#else
  DosDisConnectNmPipe(hp);
#endif

  DosClose(hp);

  MCPLog("=CL Ending thread");
  DosExit(EXIT_THREAD, 1);
}




/* Exit list handler for MCP */

#ifdef __FLAT__
static void APIENTRY MCPExitList(ULONG usTermCode)
#else
static void pascal far MCPExitList(USHORT usTermCode)
#endif
{
  MCPLog("+end, MCP " VERSION " (usTermCode=%d)", usTermCode);
  MCPLogClose();
  DosExitList(EXLST_EXIT, (PFNEXITLIST)NULL);
}


/* Ensure that we are the only running copy of MCP */

static void Synchronize(void)
{
  HMTX hmtx;
  ULONG rc;

  rc = DosCreateMutexSem("\\sem32\\maximus\\mcp",
                         &hmtx,
                         DC_SEM_SHARED,
                         FALSE);

  if (rc)
    DosExit(EXIT_PROCESS, 2);

  rc = DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);

  if (rc)
    DosExit(EXIT_PROCESS, 3);
}


int main(int argc, char *argv[])
{
  int i;

  /* Make sure that the user does not start us */

  if (argc != 5 || stricmp(argv[4], "server") != 0)
  {
    printf("MCP is started automatically by Maximus.\n");
    DosExit(EXIT_PROCESS, 1);
  }

  printf("\nMCP server process\n"
         "Copyright 1993, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  Synchronize();

  DosSetMaxFH(100);
  MCPLogOpen(argv[1]);

  MCPLog("+begin, Master Control Program " VERSION);

  strcpy(szPipeName, argv[2]);
  strcpy(szClientPipeName, argv[2]);
  strcat(szPipeName, "\\maximus");
  strcat(szClientPipeName, "\\client");

  DosExitList(EXLST_ADD, MCPExitList);

  max_ses=atoi(argv[3]);

  if ((si=malloc(sizeof(*si) * max_ses))==NULL)
  {
    MCPLog("!No memory!");
    DosExit(EXIT_PROCESS, 1);
  }

  memset(si, 0, sizeof(*si) * max_ses);

  /* Start handler threads for all of the Max sessions */

  for (i=0; i < max_ses; i++)
    StartThread(PipeMain, i, (void *)i);

  /* Start threads to handle the client programs too */

  StartThread(ClientMain, 999, NULL);

  for (;;)
    DosSleep(1000000L);

  /*DosSemWait(hsemDie, SEM_INDEFINITE_WAIT);*/
  /*free(si);*/
  return 0;
}


