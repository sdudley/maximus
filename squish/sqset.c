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

/* $Id: sqset.c,v 1.4 2004/01/22 08:04:28 wmcbrine Exp $ */

#define NOVARS
#define NOVER
#define MSGAPI_HANDLERS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "api_sq.h"
#include "sqver.h"


static void near ErrOpen(char *name)
{
  printf("Fatal error opening `%s'!\n",name);
  exit(1);
}

int _stdc main(int argc,char *argv[])
{
  struct _sqbase sqb;
  char fname[PATHLEN];
  char *p, *s;
  int sfd;

  Hello("SQSET", "Squish High-Message Adjustment Utility",
        SQVERSION, "1991, " THIS_YEAR);

  if (argc < 2)
  {
    printf("Error!  Command-line format:\n\n");

    printf("SQSET <areaname> <max_msgs> <msgs_to_skip> <days_to_keep>\n");
    exit(1);
  }

  p=strrchr(argv[1],'.');
  s=strrstr(argv[1], "/\\");

  if (p && (!s || p > s))
    if (p && strnicmp(p+1, "sqd", 3)==0)
      *p='\0';

#ifndef UNIX
  sprintf(fname, "%s.Sqd", argv[1]);
#else
  sprintf(fname, "%s.sqd", argv[1]);
#endif

  if ((sfd=open(fname,O_RDWR | O_BINARY))==-1)
    ErrOpen(fname);

  if (read(sfd,(char *)&sqb,sizeof(struct _sqbase)) != sizeof(struct _sqbase))
  {
    printf("Error reading squishfile!\n");
    exit(1);
  }

  lseek(sfd,0L,SEEK_SET);

  if (argc > 2)
  {
    sqb.max_msg=atoi(argv[2]);

    if (argc > 3)
      sqb.skip_msg=atoi(argv[3]);

    if (argc > 4)
      sqb.keep_days=atoi(argv[4]);

    if (write(sfd,(char *)&sqb,sizeof(struct _sqbase)) !=
                                       sizeof(struct _sqbase))
    {
      printf("Error writing squishfile!\n");
      exit(1);
    }
  }

  printf("Settings:  Max Messages   = %lu\n", (unsigned long) sqb.max_msg);
  printf("           Protected Msgs = %lu\n", (unsigned long) sqb.skip_msg);
  printf("           Days to keep   = %u\n", sqb.keep_days);

  close(sfd);

  if (argc > 2)
    printf("Done.\n");

  return 0;
}


