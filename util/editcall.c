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
static char rcs_id[]="$Id: editcall.c,v 1.2 2003/06/05 23:27:31 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Number-of-callers fudging utility
*/

#define MAX_INCL_VER

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "max.h"

int _stdc main(int argc,char *argv[])
{
  char temp[120];
  struct _bbs_stats bbstat;
  int bfile;

  Hello("EDITCALL", "Number-of-calls fudging utility", VERSION,
        "1990, " THIS_YEAR);

  if (argc < 2)
  {
    printf("Error!  Format is:\n\n");

    printf("EDITCALL <task_num> <num_calls>\n\n");

    printf("Use `0' for <task_num> if no task number.\n\n");
    printf("Omit <num_calls> to display the current value\n");

    return 1;
  }

  sprintf(temp,"etc/bbstat%02x.bbs",atoi(argv[1]));

  if ((bfile=open(temp,O_RDWR | O_BINARY))==-1)
  {
    printf("Error!  `%s' does not exist.  Run Maximus once to generate\n",temp);
    printf("preliminary %s file (while using the appropriate task number),\n",temp);
    printf("and then re-run EDITCALL.\n");
    return 1;
  }

  read(bfile,(char *)&bbstat,sizeof(struct _bbs_stats));

  printf("Current value = %lu calls.\n",bbstat.num_callers);

  if(argc==3)
  {
    bbstat.num_callers=atol(argv[2]);
    printf("New value     = %lu calls.\n",bbstat.num_callers);

    lseek(bfile,0L,SEEK_SET);

    write(bfile,(char *)&bbstat,sizeof(struct _bbs_stats));
  }

  close(bfile);

  printf("Done!\n");

  return 0;
}

