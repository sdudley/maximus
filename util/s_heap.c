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
static char rcs_id[]="$Id: s_heap.c,v 1.1.1.1 2002/10/01 17:57:45 sdudley Exp $";
#pragma on(unreferenced)

#define SILT
#define NOVARS
#define NOINIT
#define NO_MSGH_DEF

#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "s_heap.h"

void HeapAdd(PHEAP ph, zstr *pzstr, char *txt)
{
  if (ph->end-ph->heap + strlen(txt) >= ph->size)
  {
    printf("\n\aError!  Internal SILT heap is full!  (No more than %d bytes allowed.)\n",
           ph->size);
    exit(1);
  }

  /* Add string to heap and update pointers */

  strcpy(ph->end, txt);
  *pzstr=ph->end-ph->heap;
  ph->end += strlen(ph->end)+1;
}

/* Allocate memory for a new heap */

void HeapNew(PHEAP ph, int size)
{
  ph->size=size;

  if ((ph->heap=malloc(size))==NULL)
    NoMem();

  *ph->heap=0;
  ph->end=ph->heap+1;
}


void HeapDelete(PHEAP ph)
{
  free(ph->heap);
  ph->heap=ph->end=NULL;
}


