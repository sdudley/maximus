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
static char rcs_id[]="$Id: sem_scop.c,v 1.3 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Scope opening/closing routines for compiler
*/

#include <stdlib.h>
#include <string.h>
#include "alc.h"
#include "prog.h"
#include "mex.h"







void scope_open(void)
{
  scope++;

  #ifdef SCOPEDEBUG
  debug("Opening scope %d (offset=%d)",scope,offset);
  #endif
}





void scope_close(void)
{
  VMADDR freed;
  
  patch_gotos();

  freed=st_killscope(symtab,scope--);

  #ifdef SCOPEDEBUG
  debug("Closing scope %d (freed %d in AR, now offset=%d)",
        scope+1, freed, offset);
  #endif
}

/* Allocate a new type descriptor record from the heap.  This routine       *
 * should also probably record the current scope number, so that the        *
 * type descriptor can be freed when the scope closes, but that's not       *
 * too important right now.                                                 */

TYPEDESC * NewTypeDescriptor(void)
{
  TYPEDESC *nt;

  if ((nt=malloc(sizeof(TYPEDESC)))==NULL)
    NoMem();

  memset(nt, '\0', sizeof(TYPEDESC));
  return nt;
}


DATAOBJ * NewDataObj(void)
{
  DATAOBJ *nd = NULL;

  if ((nd=malloc(sizeof(DATAOBJ)))==NULL)
    NoMem();

  memset(nd, '\0', sizeof(DATAOBJ));

  return nd;
}





