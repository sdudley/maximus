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
static char rcs_id[]="$Id: pdclient.c,v 1.1.1.1 2002/10/01 17:49:35 sdudley Exp $";
#pragma on(unreferenced)

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "pdclient.h"

static HANDLE hp;
static HANDLE hmx;

BOOL PDInit(char *pipe)
{
  if (!WaitNamedPipe(pipe, NMPWAIT_WAIT_FOREVER))
  {
    printf("WaitNamedPipe returned %d\n", GetLastError());
    return FALSE;
  }

  if ((hmx=CreateMutex(NULL, FALSE, NULL))==INVALID_HANDLE_VALUE)
  {
    printf("Couldn't create mutex semaphore!\n");
    exit(1);
  }

  if ((hp=CreateFile(pipe, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL))==INVALID_HANDLE_VALUE)
  {
    printf("CreateFile returned %d\n", GetLastError());
    return FALSE;
  }

  return TRUE;
}


int PDWrite(char *s)
{
  DWORD sent;
  int rc;

  if ((rc=WaitForSingleObject(hmx, INFINITE)) != 0)
    printf("WaitForSingleObject rc=%d, err=%d\n", rc, GetLastError());


  if (!WriteFile(hp, s, strlen(s), &sent, NULL) /*|| sent != strlen(s)*/)
  {
    printf("Writefile returned %d - sent=%d, strlen=%d\n",
           GetLastError(), sent, strlen(s));

    return FALSE;
  }

  if (!ReleaseMutex(hmx))
    printf("ReleaseMutex rc=%d\n", GetLastError());

  return TRUE;
}



int PDPrintf(char *fmt, ...)
{
  va_list va;
  static char temp[500];

  va_start(va, fmt);
  vsprintf(temp, fmt, va);
  va_end(va);

  PDWrite(temp);
  return 0;
}


void PDDeinit(void)
{
  CloseHandle(hmx);
  CloseHandle(hp);
}
