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

/* $Id: sqreidx.c,v 1.4 2004/01/22 08:04:28 wmcbrine Exp $ */

#define NO_MSGH_DEF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "msgapi.h"
#include "api_sq.h"
#include "sqver.h"


#define VERSION SQVERSION

#ifndef UNIX
# define SQI_EXT ".SQI"
#else
# define SQI_EXT ".sqi"
#endif

char *idxname="$$TEMP$$" SQI_EXT;

int _stdc main(int argc, char *argv[])
{
  char temp[PATHLEN];
  SQIDX idx;
  XMSG msg;
  MSG *in_area;
  MSGH *in_msg;
  dword umsgid;
  dword msgn;
  int idxfile;

  umsgid=1L;

  printf("\nSQREIDX  SquishMail Database Reindexing Utility, Version " VERSION ".\n");
  printf("Copyright 1991, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  if (argc < 2)
  {
    printf("Command-line format:\n\n");

    printf("  SQREIDX <area_name>\n\n");

    printf("SQREIDX will reconstruct the .SQI file for the named area.  (If the linked\n");
    printf("lists are grunged, or you wish to rebuild the entire file, try using\n");
    printf("SQFIX instead.)\n");

    exit(1);
  }

  printf("Rebuilding index for area %s\n",argv[1]);



  /* Open the index file, write some junk to it, and close it.              *
   * This ensures that the "MsgOpenArea" call will succeed.                 */


  sprintf(temp,"%s" SQI_EXT,argv[1]);

  if ((idxfile=open(temp,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    S_IREAD | S_IWRITE))==-1)
  {
    printf("Error opening index file `%s' for write!\n",temp);
    exit(1);
  }

  memset(&idx,'\0',sizeof(SQIDX));
  write(idxfile,(char *)&idx,sizeof(SQIDX));
  close(idxfile);


  /* Now open the temporary index file */

  if ((idxfile=open(idxname,O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    S_IREAD | S_IWRITE))==-1)
  {
    printf("Error opening index file `%s' for write!\n",idxname);
    exit(1);
  }



  if ((in_area=MsgOpenArea(argv[1],MSGAREA_NORMAL,MSGTYPE_SQUISH))==NULL)
  {
    printf("Error opening area `%s' for read!\n",argv[1]);
    exit(1);
  }

  for (msgn=1L; ; msgn++)
  {
    if ((msgn % 5)==0)
      printf("Msg: %" INT32_FORMAT "\r",msgn);

    if ((in_msg=MsgOpenMsg(in_area,MOPEN_READ,msgn))==NULL)
    {
      if (msgn <= MsgHighMsg(in_area))
        continue;
      else break;
    }

    MsgReadMsg(in_msg,&msg,0L,0L,NULL,0L,NULL);

    idx.ofs=in_msg->foRead;
    idx.umsgid=umsgid++;

    idx.hash=SquishHash(msg.to);

    if (msg.attr & MSGREAD)
      idx.hash |= 0x80000000Lu;

    write(idxfile,(char *)&idx,sizeof(SQIDX));

    MsgCloseMsg(in_msg);
  }

  close(idxfile);
  MsgCloseArea(in_area);

  sprintf(temp,"%s" SQI_EXT,argv[1]);

  lcopy(idxname,temp);
  unlink(idxname);

  printf("\nDone!\n");
  return 0;
}



