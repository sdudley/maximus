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

/* Presentation manager debugging code.  This is from another               *
 * project of mine that I scooped to help debug PMBal.  This window is only *
 * used in the debugging process (#define DEBUG in client.c), so it does not*
 * appear in the "real" version of the program.                             */

#define INCL_WIN
#define INCL_DOS

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/*#include "pmbal.h"*/

#include <pos2.h>

#include "prog.h"
#include "pmdebug.h"

static HWND hwndDbgFrame=0;
static HWND hwndDbgClient=0;
static HWND hwndDbgList=(HWND)0;
static USHORT cItem=0;
static USHORT cMaxItem=0;
static BOOL fBegan=0;

static MRESULT EXPENTRY DebugProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2);

void DbgBegin(HWND hwndOwner, HAB hab, USHORT usMaxItem, USHORT fMinimize)
{
  static char szDebugClass[]="DebugClass";
  static ULONG flFrameFlags=FCF_TITLEBAR | FCF_SIZEBORDER |
                            FCF_MINMAX | FCF_SYSMENU;
  static BOOL fInit=0;
  USHORT cx, cy;


  if (fBegan)
    return;

  fBegan=TRUE;

  if (!fInit)
  {
    WinRegisterClass(hab, szDebugClass, (PFNWP)DebugProc,
                     (ULONG)CS_SIZEREDRAW, 0);
    fInit=TRUE;
  }

  cItem=0;
  cMaxItem=usMaxItem;
  hwndDbgFrame=WinCreateStdWindow(hwndOwner, 0,
                                  &flFrameFlags, szDebugClass,
                                  "Debug Window", 0L, (HMODULE)0,
                                  0, &hwndDbgClient);



  WinSetParent(hwndDbgFrame, HWND_DESKTOP, TRUE);
  WinSetOwner(hwndDbgFrame, HWND_DESKTOP);

  cx=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
  cy=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

  /* Now move the window to an appropriate spot */

  WinShowWindow(hwndDbgFrame, TRUE);

  WinSetWindowPos(hwndDbgFrame,
                  (HWND)0,
                  10,/*swp.x+swp.cx/10,*/
                  10,/*swp.y+swp.cy/10,*/
                  cx/2,
                  cy/4,
                  SWP_ACTIVATE | SWP_SIZE | SWP_MOVE |
                    (fMinimize ? SWP_MINIMIZE : 0));
}



void DbgEnd(void)
{
  if (!fBegan)
    return;

  fBegan=FALSE;
  WinDestroyWindow(hwndDbgFrame);
  hwndDbgList=(HWND)0;
}




static MRESULT EXPENTRY DebugProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2)
{
  RECTL rcl;
  HPS hps;

  switch (msg)
  {
    case WM_CREATE:
      /* Query the size of the client window */

      WinQueryWindowRect(hwnd, &rcl);

      /* Now create a giant listbox that fills the whole thing */

      hwndDbgList=WinCreateWindow(hwnd, WC_LISTBOX, "lb",
                                  WS_VISIBLE | LS_NOADJUSTPOS | LS_HORZSCROLL,
                                  0, 0, (SHORT)rcl.xRight, (SHORT)rcl.yTop,
                                  hwnd, HWND_TOP, LB_ID,
                                  NULL, NULL);

      WinShowWindow(hwndDbgList, TRUE);
      break;

    case WM_SIZE:
      /* Resize the listbox too */

      WinSetWindowPos(hwndDbgList, (HWND)0,
                      0, 0,
                      SHORT1FROMMP(mp2), SHORT2FROMMP(mp2),
                      SWP_SIZE | SWP_MOVE);
      break;

    case WM_PAINT:
      hps=WinBeginPaint(hwnd, (HPS)0, NULL);
      GpiErase(hps);
      WinEndPaint(hps);
      break;

    case WM_CLOSE:
      /* Tell PM to close only this window */
      WinDestroyWindow(hwndDbgFrame);
      return 0;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}

void DbgPrint(char *txt)
{
  if (!hwndDbgList)
    return;

  WinSendMsg(hwndDbgList, LM_INSERTITEM, MPFROMSHORT(LIT_END), MPFROMP(txt));

  if (++cItem >= cMaxItem)
  {
    WinSendMsg(hwndDbgList, LM_DELETEITEM, MPFROMSHORT(0), 0);
    cItem--;
  }

  /* Now select the item that we just added */

  WinSendMsg(hwndDbgList, LM_SELECTITEM, MPFROMSHORT(cItem-1),
             MPFROMSHORT(TRUE));
}

void _stdc DbgPrintf(char *format, ...)
{
  va_list var_args;
  static char string[512];

  va_start(var_args, format);
  vsprintf(string, format, var_args);
  va_end(var_args);

  DbgPrint(string);
}


void DbgPrintp(char *txt)
{
  if (!hwndDbgList)
    return;

  WinPostMsg(hwndDbgList, LM_INSERTITEM, MPFROMSHORT(LIT_END),
             MPFROMP(strdup(txt)));

  if (++cItem >= cMaxItem)
  {
    WinPostMsg(hwndDbgList, LM_DELETEITEM, MPFROMSHORT(0), 0);
    cItem--;
  }

  /* Now select the item that we just added */

  WinPostMsg(hwndDbgList, LM_SELECTITEM, MPFROMSHORT(cItem-1),
             MPFROMSHORT(TRUE));
}

void _stdc DbgPrintfp(char *format, ...)
{
  va_list var_args;
  static char string[512];

  va_start(var_args, format);
  vsprintf(string, format, var_args);
  va_end(var_args);

  DbgPrintp(string);
}


