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

/* $Id: btreep.h,v 1.1.1.1 2002/10/01 17:49:22 sdudley Exp $ */

// Header for a B-tree node, as it is stored on disk

struct DISKNODEHDR
{
  unsigned short fFree;               // Is this node free or in use?
  NNUM nnNextFree;                    // The next free node in the free list
  unsigned short usKeys;              // Number of keys in this node
};


// Header for a B-tree file, as it is stored on disk.

struct DISKFILEHDR
{
  NNUM nnHead;                        // Head of the B-tree
  NNUM nnFree;                        // Next free block in file
  unsigned short usOrder;             // Order of the B-tree
  unsigned short usKeySize;           // Size of each key element
  unsigned long cKeys;                // Total number of keys in file
};


