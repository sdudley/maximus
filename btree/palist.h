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

/* $Id: palist.h,v 1.1.1.1 2002/10/01 17:49:30 sdudley Exp $ */

#ifndef __PALIST_H_DEFINED
#define __PALIST_H_DEFINED

#include "btype.h"

// Node used in list of 'our node's parents'

struct PNODE
{
  NNUM nn;                                // Node number of parent
  int current_idx;                        // Current offset within keys
  PNODE *next;
};

// Linked list used for storing chain-of-command (list of all
// our parents, grandparents, etc.) from the head of the
// B-tree down to this node

class PALIST
{
  PNODE *ppn;                         // Linked list of nodes
public:
  int fSearched;                      // Been used for a search already?
  int current_idx;                    // Index in tree.  Only used for browse.
  NNUM current_node;                  // Node num in tree. "   "    "    "

  CPPEXPORT PALIST();
  CPPEXPORT ~PALIST();
  int CPPEXPORT add(NNUM nn, int current_idx);
  NNUM CPPEXPORT pop(int *pcurrent_idx);
  PNODE * CPPEXPORT nodes();
};


#endif // __PALIST_H_DEFINED

