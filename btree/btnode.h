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

/* $Id: btnode.h,v 1.1.1.1 2002/10/01 17:49:21 sdudley Exp $ */

#ifndef __BTNODE_H_DEFINED
#define __BTNODE_H_DEFINED

class BTREE;                          // Forward decl for B-tree class

// In-memory node record

class BTNODE
{
  BTREE *pbt;                         // Related B-tree
  char *pcDiskNode;                   // On-disk version of node

public:
  NNUM nn;                            // Number of this node
  unsigned uiKeys;                    // Number of keys in this node

  unsigned fFree;                     // Is this node free or in use?
  NNUM nnNextFree;                    // The next free node in the free list

  NNUM *child;                        // Children of this node
  void **key;                         // Keys for this node


  BTNODE(BTREE *pbt, NNUM n=0L);
  ~BTNODE();
  int get();
  int put();
};


#endif // __BTNODE_H_DEFINED

