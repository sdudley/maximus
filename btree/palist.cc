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
static char rcs_id[]="$Id: palist.cc,v 1.1 2002/10/01 17:49:29 sdudley Exp $";
#pragma on(unreferenced)

#include "palist.h"

//////////////////////////////////////////////////////////////////////////////
// Parent list object
//////////////////////////////////////////////////////////////////////////////

// Constructor for the list of parent nodes

CPPEXPORT PALIST::PALIST()
{
  ppn=new PNODE;

  if (ppn)
  {
    ppn->next=0;
    ppn->nn=0L;
  }

  fSearched=FALSE;
  current_idx=0;
}

// Destructor for the list of parent nodes

CPPEXPORT PALIST::~PALIST()
{
  PNODE *p, *pnext;

  // Free the linked list of nodes

  for (p=ppn; p; pnext=p->next, delete p, p=pnext)
    ;
}

// Add another parent to the list of parents

int CPPEXPORT PALIST::add(NNUM nn, int current_idx)
{
  PNODE *p;

  p=new PNODE;

  if (!p)
    return FALSE;

  p->current_idx=current_idx;
  p->nn=nn;
  p->next=ppn;
  ppn=p;

  return TRUE;
}

// Pop one node off the list and return its node number

NNUM CPPEXPORT PALIST::pop(int *pcurrent_idx)
{
  PNODE *p;
  NNUM rc;

  if (!ppn)
    return 0L;

  p=ppn;
  rc=p->nn;
  *pcurrent_idx=p->current_idx;

  // Unlink the first node

  ppn=ppn->next;

  delete p;

  return rc;
}

// Return a linked list of parents of this node

PNODE * CPPEXPORT PALIST::nodes()
{
  return ppn;
}


