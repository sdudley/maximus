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
static char rcs_id[]="$Id: max_out.c,v 1.2 2003/06/06 01:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Modem/local output routines
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>
#include "prog.h"
#include "mm.h"

#if defined(NT) || defined(UNIX)
# include "ntcomm.h"
#else
# define COMMAPI_VER 0
#endif

int last_cc=-1;
char strng[20];
static char *szOutString = NULL;

void OutputAllocStr(void)
{
  if (szOutString==NULL && (szOutString=malloc(MAX_PRINTF))==NULL)
  {
    printf(printfstringtoolong,"P");
    quit(ERROR_CRITICAL);
  }
}

void OutputFreeStr(void)
{
  free(szOutString);
  szOutString = NULL;
}

int _stdc Printf(char *format,...)   /* Sends AVATAR string to console/modem */
{
  va_list var_args;
  int x;

  va_start(var_args,format);
  x=vsprintf(szOutString, format, var_args);
  va_end(var_args);

  Puts(szOutString);
  return x;
}


/* Sends AVATAR string to local console */

int _stdc Lprintf(char *format,...)   
{
  va_list var_args;
  int x;

  char string[MAX_PRINTF];

  if (strlen(format) >= MAX_PRINTF)
  {
    printf(printfstringtoolong,"Lp");
    return -1;
  }

  va_start(var_args,format);
  x=vsprintf(string,format,var_args);
  va_end(var_args);

  Lputs(string);
  return x;
}


/* Displays AVATAR string to modem */

int _stdc Mdm_printf(char *format,...)   
{
  va_list var_args;
  int x;

  char string[MAX_PRINTF];

  if (strlen(format) >= MAX_PRINTF)
  {
    printf(printfstringtoolong,"Mdm_p");
    return -1;
  }

  va_start(var_args,format);
  x=vsprintf(string,format,var_args);
  va_end(var_args);

  Mdm_puts(string);
  return x;
}


void Putc(int ch)
{
  if (!no_remote_output)
    Mdm_putc(ch);

  if ((snoop || local) && !no_local_output)
    Lputc(ch);
}

void Puts(char *s)
{
  if (!no_remote_output)
    Mdm_puts(s);

  if ((snoop || local) && !no_local_output)
    Lputs(s);
}

void Mdm_puts(char *s)
{
#if (COMMAPI_VER > 1)
  extern HCOMM hcModem;
  BOOL lastState = ComBurstMode(hcModem, TRUE);
#endif

  while (*s)
    Mdm_putc(*s++);

#if (COMMAPI_VER > 1)
  ComBurstMode(hcModem, lastState);
#endif
}


void _stdc DoWinPutc(int ch)
{
  WinPutc(win, (byte)ch);
}

void _stdc DoWinPuts(char *s)
{
  while (*s)
    WinPutc(win,*s++);
}


void vbuf_flush(void)
{
  if (no_video)
    return;

#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
#endif
    WinSync(win, in_wfc ? FALSE : TRUE);
#ifdef TTYVIDEO
  else fflush(stdout);
#endif
}



void Lputs(char *s)
{

  while (*s)
    Lputc(*s++);

  /* Otherwise, only flush video buffer if in file transfer protocol */

  if (in_file_xfer)
    vbuf_flush();

}


