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

/* $Id: sem_goto.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=GOTO processing code
*/

#include <stdlib.h>
#include "prog.h"
#include "mex.h"


void ProcessGoto(IDTYPE *id)
{
  GOTO *g;

  /* Make sure that the found variable is actually a label */

  g=smalloc(sizeof(GOTO));
  
  g->name=sstrdup(id);
  g->quad=this_quad;
  g->scope=scope;

  /* Append to linked list of gotos */

  g->next=gstack;
  gstack=g;
  
  /* Generate the code for the jump itself.  The actual quad number to     *
   * jump to will be backpatched after this scope is closed, to handle      *
   * forward references properly.                                           */
  
  Generate(QOP_JMP, NULL, NULL, NULL);
}





/* Declare a label for use with a 'goto' statement */

void DeclareLabel(IDTYPE *id)
{
  ATTRIBUTES *attr;
  byte present;
  
  attr=st_enter(symtab, id, &present);
  
  #ifdef DECLDEBUG
  debug("Label '%s' at quad %u", id, this_quad);
  #endif
    

  if (present)
  {
    error(MEXERR_REDECLOFLABEL, id);
    return;
  }
  

  attr->typedesc=NULL;
  attr->class=ClassLabel;
  attr->c.label_quad_no=this_quad;
}


void patch_gotos(void)
{
  ATTRIBUTES *attr;
  GOTO *g, *last, *next;
  PATCH pat;
  
  for (g=gstack, last=NULL; g; g=next)
  {
    /* If the label was used in a scope equal to (or deeper than) the       *
     * current one, and if it has been defined, patch the quads here.      */

    if (g->scope >= scope && ((attr=st_find(symtab, g->name, FALSE)) != NULL))
    {
      if (attr->class != ClassLabel)
        error(MEXERR_NOTAGOTOLABEL);
      else
      {
        /* Otherwise, it's okay to backpatch properly */
        
        pat.quad=g->quad;
        pat.next=NULL;

        BackPatch(&pat, attr->c.label_quad_no);
      }
      
      /* Now free this goto node, and skip the linked list over it */
        
      if (last)
        last->next=g->next;
      else gstack=g->next;
      
      /* Free memory used by this one */
      
      free(g->name);
      
      next=g->next;
      free(g);
    }
    else
    {
      /* If this node wasn't removed from the chain, set the last ptr to it */
      last=g;
      next=g->next;
    }
  }
  
  /* If we're closing a function, spit out errors for any unresolved        *
   * labels.                                                                */
     
  if (scope==1)
  {
    for (g=gstack; g; g=next)
    {
      error(MEXERR_UNDEFLABEL, g->name);
      
      next=g->next;

      free(g->name);
      free(g);
    }
    
    gstack=NULL;
  }
}
