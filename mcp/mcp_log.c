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
static char rcs_id[]="$Id: mcp_log.c,v 1.1.1.1 2002/10/01 17:53:27 sdudley Exp $";
#pragma on(unreferenced)

#define INCL_DOS
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "pos2.h"
#include "max.h"
#include "mcp_int.h"

static FILE *fplog=NULL;
static char months_ab[][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
                            "Aug","Sep","Oct","Nov","Dec"};


void MCPLogOpen(char *fname)
{
  if (eqstri(fname, "."))
    fplog=NULL;
  else if ((fplog=fopen(fname, "a"))==NULL)
  {
    printf("Error opening log file %s for write!\n", fname);
    DosExit(EXIT_PROCESS, 1);
  }
}

void MCPLog(char *fmt, ...)
{
  va_list var_arg;
  char out_date[256];
  char out[256];
  time_t t;
  struct tm *ptm;

  va_start(var_arg, fmt);
  vsprintf(out, fmt, var_arg);
  va_end(var_arg);

  t=time(NULL);
  ptm=localtime(&t);

  sprintf(out_date, "%c %02d %s %02d:%02d:%02d %s %s\n",
          *out, ptm->tm_mday, months_ab[ptm->tm_mon],
          ptm->tm_hour, ptm->tm_min, ptm->tm_sec, "MCP ", out+1);

  if (fplog)
    fputs(out_date, fplog);

  printf("%s", out_date);
}


void MCPLogClose(void)
{
  if (fplog)
  {
    fputs("\n", fplog);
    fclose(fplog);
    fplog=NULL;
  }
}


