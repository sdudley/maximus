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
static char rcs_id[]="$Id: s_log.c,v 1.1.1.1 2002/10/01 17:56:25 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <share.h>
#include "prog.h"
#include "max.h"
#include "squish.h"

static FILE *logfile=NULL;
static char *slogfmt="%c %02d %s %02d:%02d:%02d %s %s\n";

#define MAX_LOG_LEN   256     /* max len of line in log file */


void S_LogOpen(char *name)
{
  if (*name)
    if ((logfile=sfopen(name, "a", O_CREAT | O_WRONLY | O_APPEND | O_NOINHERIT,
                        SH_DENYWR))==NULL)
    {
      ErrOpening("log",name);
    }

  if (logfile)
    (void)fputc('\n',logfile);
}

void S_LogClose(void)
{
  if (logfile)
  {
    (void)fclose(logfile);
    logfile=NULL;
  }
}

static struct _ll
{
  byte ch;
  byte lev;
} loglevels[]=
{{'!', 0},
 {'@', 0},
 {'+', 1},
 {'*', 2},
 {'-', 3},
 {'#', 4},
 {':', 5},
 {' ', 6},
 {0,   0}};


/* Add a line to the Squish log file */

void cdecl OS2FAR S_LogLine(char OS2FAR *string)
{
  time_t gmt;
  struct tm *localt;
  static char log_line[MAX_LOG_LEN];

  gmt=time(NULL);
  localt=localtime(&gmt);

  (void)sprintf(log_line, slogfmt, *string, localt->tm_mday,
                months_ab[localt->tm_mon], localt->tm_hour,
                localt->tm_min, localt->tm_sec, "SQSH", string+1);

  if (logfile)
  {
    struct _ll *ll;
    
    for (ll=loglevels; ll->ch; ll++)
      if (ll->ch==(byte)*log_line)
        break;
      
    if ((!ll->ch) || (ll->ch && config.loglevel >= ll->lev))
      (void)fputs(log_line, logfile);
  }

  if (*string=='!' || *string=='@')
  {
    if (logfile)
    {
      (void)fflush(logfile);
      flush_handle(logfile);
    }
    
    (void)printf("\r                                                              "
                 "             \r%s", log_line);
  }
}


/* Normal entrypoint to the log message routine */

void _stdc S_LogMsg(char *format,...)
{
  char string[MAX_LOG_LEN];
  va_list var_args;

  va_start(var_args, format);
  (void)vsprintf(string, format, var_args);
  va_end(var_args);

  S_LogLine(string);
}

