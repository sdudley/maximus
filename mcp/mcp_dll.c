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
static char rcs_id[]="$Id: mcp_dll.c,v 1.1.1.1 2002/10/01 17:53:27 sdudley Exp $";
#pragma on(unreferenced)

#include <string.h>
#define INCL_DOS
#define INCL_SUB
#include "pos2.h"
#include "max.h"
#include "mcp.h"


/* Peek a message from the pipe */

int EXPENTRY McpPeekMsg(HPIPE hp, PVOID *pv, USHORT *pusSize, USHORT usMaxSize)
{
  OS2UINT fsState;
  OS2UINT usSize, rc;
  AVAILDATA ad;

  if (!hp)
    return 230;  /* ERROR_BAD_PIPE */

  DosSetNmPHandState(hp, NP_READMODE_MESSAGE | NP_WAIT);

  rc=DosPeekNmPipe(hp, (PBYTE)pv, usMaxSize, &usSize, &ad, &fsState);

  if (pusSize)
    *pusSize=(USHORT)usSize;

  if (rc)
    return rc;
  else if (fsState==NP_CLOSING)
    return 109;

  return 0;
}


/* Read a message from the pipe */

int EXPENTRY McpGetMsg(HPIPE hp, PVOID *pv, USHORT *pusSize, USHORT usMaxSize)
{
  OS2UINT usSize;
  OS2UINT rc;

  if (!hp)
    return 230;  /* ERROR_BAD_PIPE */

  DosSetNmPHandState(hp, NP_READMODE_MESSAGE | NP_WAIT);

  rc=DosRead(hp, pv, usMaxSize, &usSize);

  if (pusSize)
    *pusSize=(USHORT)usSize;

  if (rc)
    return rc;


  if (*pusSize==0)
  {
    OS2UINT fsState;
    BYTE pb[1];
    OS2UINT cbRead;
    AVAILDATA ad;

    if ((rc=DosPeekNmPipe(hp, pb, 1, &cbRead, &ad, &fsState)) != 0)
      return rc;
    else if (fsState==NP_CLOSING)
      return 109; /* error_broken_pipe */
  }

  return 0;
}



/* Dispatch a message read from the pipe */

int EXPENTRY McpDispatchMsg(PVOID pv, USHORT usSize, DISPATCH *dt, PVOID pvParm)
{
  OS2UINT code, rc;
  DISPATCH *pdt;

  NW(usSize);

  code=*(USHORT *)pv;

  for (pdt=dt; pdt->f; pdt++)
    if (pdt->dispatch_code==code)
    {
      if ((rc=(*pdt->f)(pvParm)) != 0)
        return rc;

      break;
    }

  if (! pdt->f)
    return -1; /* unknown function */

  return 0;
}



/* Send a message of type usType to MCP */

int EXPENTRY McpSendMsg(HPIPE hp, USHORT usType, BYTE *pbMsg, USHORT cbMsg)
{
  BYTE *buf;
  OS2UINT cbWritten, rc;
#ifdef __FLAT__
  PVOID pv;
#else
  SEL sel;
#endif

  if (!hp)
    return 230; /* bad pipe */

#ifdef __FLAT__
  if ((rc=DosAllocMem(&pv, cbMsg+sizeof(USHORT), PAG_READ | PAG_WRITE | PAG_COMMIT)) != 0)
    return rc;

  buf=pv;
#else
  if ((rc=DosAllocSeg(cbMsg+sizeof(USHORT), &sel, SEG_NONSHARED)) != 0)
    return rc;

  buf=MAKEP(sel, 0);
#endif

  *(USHORT *)buf=usType;

  if (pbMsg)
    memmove(buf+sizeof(USHORT), pbMsg, cbMsg);

  rc=DosWrite(hp, buf, cbMsg+sizeof(USHORT), &cbWritten);

#ifdef __FLAT__
  DosFreeMem(pv);
#else
  DosFreeSeg(sel);
#endif
  return rc;
}


/* Begin an IPC session with MCP */

int EXPENTRY McpOpenPipe(char *pszPipeName, HPIPE *php)
{
  OS2UINT usAction;

#ifdef __FLAT__
  return DosOpen(pszPipeName, php, &usAction, 0L, FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                 OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE |
                 OPEN_FLAGS_NOINHERIT, NULL);
#else
  return DosOpen(pszPipeName, php, &usAction, 0, FILE_NORMAL, FILE_OPEN,
                 OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE |
                   OPEN_FLAGS_NOINHERIT, 0);
#endif
}


/* Close the specified named pipe */

void EXPENTRY McpClosePipe(HPIPE hp)
{
  if (hp)
  {
    DosClose(hp);
    hp=0;
  }
}

void APIENTRY __DLLend(OS2UINT usTermCode)
{
  (void)usTermCode;

  /*
  char temp[100];
  sprintf(temp, "MCP 1.0 Termination, usTermCode=%d\r\n",
          usTermCode);

  VioWrtTTY(temp, 35, 0);
  */
  DosExitList(EXLST_EXIT, 0);
}

int __dll_initialize(void)
{
  /*
  VioWrtTTY("MCP 1.0 Startup\r\n", 17, 0);
  */

  grow_handles(0);
  /*DosBeep(550, 20);*/

  DosExitList(EXLST_ADD, __DLLend);
  return 1;
}

