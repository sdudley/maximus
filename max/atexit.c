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
static char rcs_id[]="$Id: atexit.c,v 1.1.1.1 2002/10/01 17:50:45 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

  for (pae = paeExitList; pae; pae = paeNext)
  {
    (*pae->exitFunc)();
    paeNext = pae->next;
    free(pae);
  }

  exit(status);
}


