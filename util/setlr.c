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
static char rcs_id[]="$Id: setlr.c,v 1.1.1.1 2002/10/01 17:57:36 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "msgapi.h"


static void near SetLR(char *name, UMSGID uid, int num_users)
{
  char fname[120];
  word wordid;
  int fd;

  printf("Creating lastread file...\n");

  /* Open the lastread file */

  sprintf(fname, *name=='$' ? "%s.sql" : "%slastread.bbs",
          name + (*name=='$'));

  if ((fd=open(fname,  O_WRONLY | O_BINARY | O_CREAT | O_TRUNC,
               S_IREAD | S_IWRITE))==-1)
  {
    printf("Error opening %s for write!\n");
    exit(1);
  }

  if (*name != '$')
  {
    wordid=(word)uid;

    while (num_users--)
      if (write(fd, (char *)&wordid, sizeof wordid) != sizeof wordid)
      {
        printf("Error writing to lastread file!\n");
        exit(1);
      }
  }
  else
  {
    while (num_users--)
      if (write(fd, (char *)&uid, sizeof uid) != sizeof uid)
      {
        printf("Error writing to lastread file!\n");
        exit(1);
      }
  }

  close(fd);
}


int main(int argc, char *argv[])
{
  struct _minf mi;
  dword high;
  UMSGID uhigh;
  HAREA ha;
  int num_users;

  /* setlr $d:\path\areaname 3000 [2900] */

  if (argc < 3)
  {
    printf("Usage:\n\n"

           "SETLR <areaname> <num_users> [message_num]\n\n"

           "If <areaname> is a Squish-format area, add a '$' to the beginning of the path.\n");

    return 1;
  }

  mi.req_version=0;
  MsgOpenApi(&mi);

  if ((ha=MsgOpenArea(argv[1]+(*argv[1]=='$'), MSGAREA_NORMAL,
                      *argv[1]=='$' ? MSGTYPE_SQUISH : MSGTYPE_SDM))==NULL)
  {
    printf("Error!  Can't open message area %s!\n"
           "(use \"$d:\path\areaname\" for Squish-format areas!)\n");
    return 1;
  }

  high=MsgGetHighMsg(ha);
  printf("Highest message number is %ld.\n", high);

  num_users=atoi(argv[2]);

  if (argc >= 4)
    high=atoi(argv[3]);

  printf("Setting message number for all %d users to %ld.\n",
         num_users, high);

  uhigh=MsgMsgnToUid(ha, high);

  if (!uhigh)
  {
    printf("Error!  Message not found!\n");
    return 1;
  }

  MsgCloseArea(ha);
  MsgCloseApi();

  SetLR(argv[1], uhigh, num_users);

  printf("Done!\n");

  return 0;
}

