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

/* $Id: maxstart.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "prog.h"
#define INCL_DOS
#include "pos2.h"
#include "mcp.h"



int main(int argc, char *argv[])
{
  char szPipe[PATHLEN];
  HPIPE hp;
  byte tid;
  USHORT rc;

  if (argc < 2)
  {
    printf("usage:  maxstart <pipe_name> <node_num>\n");
    exit(1);
  }

  strcpy(szPipe, argv[1]);
  strcat(szPipe, "\\client");

  if ((rc=McpOpenPipe(szPipe, &hp)) != 0)
  {
    printf("SYS%04d: McpOpenPipe\n", rc);
    return 1;
  }

  /* Tell MCP to start monitoring the node in question */

  tid=atoi(argv[2]);

  if ((rc=McpSendMsg(hp, CLMSG_START, &tid, 1)) != 0)
  {
    printf("SYS%04d: McpSendMsg\n", rc);
    return 1;
  }

  McpClosePipe(hp);

  return 0;
}


