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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: atexit.c,v 1.4 2004/01/27 21:00:26 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
#include <errno.h>
#endif
#include "alc.h"
#include "prog.h"
#include "mm.h"


typedef struct _atexitstruct
{
  void (*exitFunc)(void);
  struct _atexitstruct *next;
}
AtExitStruct;

AtExitStruct *paeExitList = 0;


int maximus_atexit( register void ( *func )( void ) )
{
  AtExitStruct *pae;

  if ((pae = malloc(sizeof(*pae)))==NULL)
    exit(2);

  pae->exitFunc = func;
  pae->next = paeExitList;
  paeExitList = pae;
  return 0;
}


void maximus_exit(int status)
{
  AtExitStruct *pae, *paeNext;
#ifdef UNIX
  const char *afterMax;
#endif

  for (pae = paeExitList; pae; pae = paeNext)
  {
    (*pae->exitFunc)();
    paeNext = pae->next;
    free(pae);
  }

#ifdef UNIX
  if ((afterMax = getenv("AFTER_MAX")))
  {
    char 		buf[32];
    char 		*argv[] = { afterMax, "AFTER_MAX", buf, NULL };
    extern char 	**environ;

    snprintf(buf, sizeof(buf), "ERRORLEVEL=%i", status);
    putenv(buf);
    snprintf(buf, sizeof(buf), "%i", status);

    logit(":Becoming AFTER_MAX program: %s %s %s", argv[0], argv[1], argv[2]);

    errno=0;
    execve(argv[0], argv, environ);

    logit("!Error: Could not execute AFTER_MAX program: %s %s %s (%s)", argv[0], argv[1], 
	  argv[2], strerror(errno));
  }
#endif

  exit(status);
}


