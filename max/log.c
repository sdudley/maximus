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

/* $Id: log.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=Log-file manipulation functions
*/

#define MAX_LANG_max_log

#include <stdio.h>
#include <stdlib.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "mm.h"

#ifdef MCP
  #define INCL_DOS
  #include "pos2.h"
  #include "mcp.h"
#endif

extern char log_name[];

static FILE *logfile=NULL;


#ifdef MAXSNOOP

  #include "snserver.h"

  static HSNOOP hsnoop;
  static char *pipe = NULL;

  void SnSetPipeName(char *str)
  {
    if (pipe)
      free(pipe);

    pipe = strdup(str);
  }

#endif



static int log_written=FALSE;

#ifdef MAXSNOOP
  static int pascal far NullNotify(int sError, char far *pszMsg)
  {
    NW(sError); NW(pszMsg);
    return 0;
  }
#endif

int LogOpen(void)
{
#ifdef MAXSNOOP
  if(!pipe)
    SnSetPipeName("\\pipe\\maxsnoop");

  SnoopOpen(pipe, &hsnoop, xfer_id, NullNotify);
#endif

  if (!logfile && log_name && log_name[0])
  {
    log_written=TRUE;

    Convert_Star_To_Task(log_name);

    if ((logfile=sfopen(log_name, fopen_append, O_APPEND | O_TEXT | O_WRONLY |
                        O_CREAT | O_NOINHERIT, SH_DENYWR))==NULL)
    {
      Lprintf(copen_log, log_name);
      vbuf_flush();
    }

    return (logfile != NULL);
  }
  else return TRUE;
}


void LogClose(void)
{
  if (logfile)
  {
    log_written=TRUE;

    LogFlush();
    fclose(logfile);
    logfile=NULL;
  }

#ifdef MAXSNOOP
  SnoopClose(hsnoop);
  hsnoop=0;
#endif
}


int LogWrite(char *str)
{
  int c=0;

  if (logfile)
  {
    c=fwrite(str, 1, strlen(str), logfile);
    log_written=TRUE;
  }

  return c;
}

void LogFlush(void)
{
  if (logfile && log_written)
  {
    log_written=FALSE;

    fflush(logfile);
    flush_handle(logfile);
  }
}

#ifdef MAXSNOOP
  void SnWrite(char *str)
  {
    if (hsnoop)
      SnoopWrite(hsnoop, str);
  }
#endif


/* Add a line to the system log...  First character in string is the
   character to add at the beginning of the line -- The rest is the line
   to log.  Can include printf()-style formatting info.                   */

#ifdef ORACLE

void cdecl logit(char *format,...)
{
  NW(format);
}

#else

#define LOGIT_SIZE    240

void cdecl logit(char *format,...)
{
  time_t gmtime;
  struct tm *localt;

  int size;
  char *string;
  char *string2;
  char screen_log;
  char *p;

  va_list var_args;

  int doit;

  /* We may be doing lots of debug logging during file transfers,
   * but don't waste any time if no logging is required.
   */

  if (*format=='@' && !debuglog)
    return;

  if ((string=malloc(LOGIT_SIZE+20))==NULL ||
      (string2=malloc(LOGIT_SIZE))==NULL)
  {
    if (string)
      free(string);
    
    Lputs(GRAY "\r! NoMem: ");
    Lputs(format);
    return;
  }
  
  va_start(var_args,format);
  size=vsprintf(string, format, var_args);
  va_end(var_args);

  /* Did we booboo? */

  if (size >= LOGIT_SIZE-20)
  {
    LogWrite("!****** FATAL: logit() too long.  Please report to author!\n");
    LogWrite("!****** Log was `");
    LogWrite(string);
    LogWrite("'!\n");

    LogFlush();
    LogClose();
    brkuntrap();
    uninstall_24();
    _exit(ERROR_CRITICAL);
  }

  p=string;

  if (*p=='>')
  {
    screen_log=TRUE;
    p++;
  }
  else screen_log=FALSE;

  gmtime=time(NULL);
  localt=localtime(&gmtime);

  doit=FALSE;

  switch(*p)
  {
    case '!':
      doit=TRUE;
      break;

    case '+':
      if (prm.log_mode >= 1)
        doit=TRUE;
      break;

    case '=':
      if (prm.log_mode >= 2)
        doit=TRUE;
      break;

    case ':':
      if (prm.log_mode >= 3)
        doit=TRUE;
      break;

    case '~':
      if (prm.log_mode >= 4)
        doit=TRUE;
      break;

    case '#':
      if (prm.log_mode >= 5)
        doit=TRUE;
      break;

    case '$':
      if (prm.log_mode >= 6)
        doit=TRUE;
      break;

    case ' ':     /* Never log these lines */
      if (prm.log_mode >= 7)
        doit=TRUE;
      break;

    case '@':
      doit=(int)debuglog;
      break;

    default:
      doit=TRUE;
      break;
  }

  /* Create abbreviated version of log string */

  sprintf(string2, "%c %02d:%02d:%02d %s\n",
          *p, localt->tm_hour, localt->tm_min, localt->tm_sec, p+1);


#ifdef MCP
  {
    extern HPIPE hpMCP;

    if (*string2 != '@')
      McpSendMsg(hpMCP, PMSG_LOG, string2, strlen(string2)+1);
  }
#endif


#ifdef MAXSNOOP
  if (*string2 != '@' || debuglog)
    SnWrite(string2);
#endif

  /* Now create real log entry */

  sprintf(string2, logformat, *p, localt->tm_mday, months_ab[localt->tm_mon],
          localt->tm_hour, localt->tm_min, localt->tm_sec, nameabbr, p+1);


  if (doit)
  {
    LogWrite(string2);

    /* Something fishy, make sure it stays on record! */

    if (*string=='!' /*|| *string=='@'*/)
      LogFlush();
    else
    {
      /* Flush each line regardless, if we're running a test version, since *
       * we may need this output if it crashes beforehand.                  */

      #if defined(FLUSH_LOG) && !defined(OS_2)
      LogFlush();
      #endif
    }
  }


  if (log_wfc && displaymode==VIDEO_IBM)
    WFC_LogMsg(string2);
  else if ((!snoop && !local) || !caller_online || in_file_xfer ||
            screen_log || *string2=='!')
  {
/*    if (*string2 != '@' || debuglog)*/
    if (*string2 != '@')
    {
      Lputs(GRAY "\r" CLEOL);
      Lputs(string2);
      vbuf_flush();
    }
  }

  free(string2);
  free(string);
}

#endif /* !ORACLE */

