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
static char rcs_id[]="$Id: maxshow.c,v 1.1.1.1 2002/10/01 17:53:25 sdudley Exp $";
#pragma on(unreferenced)

#include <stdlib.h>
#include <string.h>
#define INCL_DOS
#include "pos2.h"
#include "max.h"
#include "mcp.h"

static char buf[512];

int main(int argc, char *argv[])
{
  struct _cdat *pcd;
  HPIPE hp;
  USHORT rc;

  if (argc < 3)
  {
    printf("usage:  maxshow <pipe_name> <node_num> <.bbs_filespec>\n");
    exit(1);
  }

  if ((rc=McpOpenPipe(argv[1], &hp)) != 0)
  {
    printf("SYS%04d: McpOpenPipe\n", rc);
    return 1;
  }

  pcd=(struct _cdat *)buf;

  /* Fill out the Max-to-Max messag header */

  pcd->tid=0;               /* as a standalone program, we have no task num */
  pcd->type=CMSG_DISPLAY;
  pcd->len=strlen(argv[3])+1;
  pcd->dest_tid=atoi(argv[2]);

  /* Add in the message data */

  strcpy((char *)(pcd+1), argv[3]);

  if ((rc=McpSendMsg(hp, PMSG_MAX_SEND_MSG, buf,
                        sizeof(*pcd) + pcd->len)) != 0)
  {
    printf("SYS%04d: McpSendMsg\n", rc);
    return 1;
  }

  McpClosePipe(hp);

  printf("Told node %d to display filename '%s'.\n",
         pcd->dest_tid, (char *)(pcd+1));

  return 0;
}

