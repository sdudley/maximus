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

/* $Id: bbuf.h,v 1.2 2003/06/05 23:26:47 wesgarland Exp $ */

#include "btype.h"

// Internal structure used for maintaining the list of buffered
// blocks.

struct _bblist
{
  NNUM nn;                            // This node number
  unsigned uiHits;                    // Number of hits for this block
  void *pvBlock;                      // Data for this block
  unsigned fDelta;                    // Has data been changed?
  struct _bblist *next;               // Next block in list
};

// Block memory buffer object

class BLOCKBUF
{
  unsigned fOpen;                     // Have we been inited?
  unsigned uiBlkSize;                 // Block size
  struct _bblist *bbList;             // Head of list of blocks
  struct _bblist *bbDelta;            // Current delta serach pos'n
  unsigned uiNumBlocks;               // Number of blocks in buffer
  unsigned uiLRUCtr;                  // LRU counter for this buffer

public:

  // Functions to initialize and deinitialize the buffer

  int CPPEXPORT init(unsigned uiBlkSize);
  int CPPEXPORT deinit(void);

  // Function to determine if we are open

  inline int CPPEXPORT buffer_enabled() { return fOpen; }

  // Functions for reading and writing from the buffer

  void * CPPEXPORT get(NNUM nn);
  int CPPEXPORT put(NNUM nn, void *pvBlock, unsigned fDelta);

  // Retrieving blocks with the delta bit set

  void * CPPEXPORT delta_first(NNUM *pnn);
  void * CPPEXPORT delta_next(NNUM *pnn);

  // Return the number of blocks in the buffer

  inline int CPPEXPORT num_blocks() { return uiNumBlocks; }
  int CPPEXPORT get_lru(NNUM *pnn, void *pvBlock, unsigned *pfDelta);

  CPPEXPORT BLOCKBUF();
  virtual CPPEXPORT ~BLOCKBUF();
};


