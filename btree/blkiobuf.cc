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

/* $Id: blkiobuf.cc,v 1.3 2004/01/22 09:02:29 wmcbrine Exp $ */

#include <stdio.h>
#include <string.h>
#include "blkiobuf.h"


// Default constructpor for buffered block i/o.  Just set the buffered
// state to 'false'.

CPPEXPORT BLKIOBUF::BLKIOBUF(unsigned uiMaxBlocks)
{
  this->uiMaxBlocks=uiMaxBlocks;
}


// Destroy this i/o buffer by deleting the cache

CPPEXPORT BLKIOBUF::~BLKIOBUF()
{
  if (buffer_enabled())
    enable_buffer(FALSE);
}


// Set the block size for this file

int CPPEXPORT BLKIOBUF::set_block_size(unsigned int uiBlkSize)
{
  // Turn off the cache, since this will invalidate everything
  // stored within.

  enable_buffer(FALSE);

  BLKIO::set_block_size(uiBlkSize);
  this->uiBlkSize=uiBlkSize;
  return TRUE;
}


// Close the file, dumping the cache first

int CPPEXPORT BLKIOBUF::close()
{
  enable_buffer(FALSE);
  return BLKIO::close();
}



// Return the highest used node number

NNUM CPPEXPORT BLKIOBUF::high_node(void)
{
  NNUM nnHigh=BLKIO::high_node();

  return buffer_enabled() && nnHighPut != (NNUM)-1L && nnHighPut+1 > nnHigh
            ? nnHighPut+1 : nnHigh;
}



// Selectively enable and disable the block buffer

int CPPEXPORT BLKIOBUF::enable_buffer(unsigned fEnable)
{
  // Close the existing buffer, if any

  if (buffer_enabled())
  {
    if (!flush_buffer())
      return FALSE;

    if (!BLOCKBUF::deinit())
      return FALSE;
  }

  // If we want to enable the buffer, do so now.

  if (fEnable)
  {
    if (!BLOCKBUF::init(uiBlkSize))
      return FALSE;

    nnHighPut=(NNUM)-1L;
  }

  return TRUE;
}

// Get a node from either the cache or the disk, as necessary.

int CPPEXPORT BLKIOBUF::get(NNUM nn, char *pcDiskNode)
{
  void *pv;

  // Try to get it from the cache

  if (buffer_enabled())
    if ((pv=BLOCKBUF::get(nn)) != 0)
    {
      memmove(pcDiskNode, pv, uiBlkSize);
      return TRUE;
    }

  // Get it from disk

  int rc=BLKIO::get(nn, pcDiskNode);

  // If we got it, insert it in the cache for later

  if (rc && buffer_enabled())
    if (!_insert_list(nn, pcDiskNode, FALSE))
      rc=FALSE;

  return rc;
}



// Write a node to the cache, flushing if necessary

int CPPEXPORT BLKIOBUF::put(NNUM nn, char *pcDiskNode)
{
  if (nn > nnHighPut || nnHighPut==(NNUM)-1L)
    nnHighPut=nn;

  if (buffer_enabled())
    return _insert_list(nn, pcDiskNode, TRUE);
  else
    return BLKIO::put(nn, pcDiskNode);
}



// Flush the cache buffer to disk

int CPPEXPORT BLKIOBUF::flush_buffer(void)
{
  NNUM nn;
  void *pv;
  int rc=TRUE;

  if (!buffer_enabled())
    return TRUE;

  // Look for the first delta bit

  if ((pv=delta_first(&nn)) != 0)
  {

    do
    {
      // Write this block to disk

      if (!BLKIO::put(nn, (char *)pv))
        rc=FALSE;
    }
    while ((pv=delta_next(&nn)) != 0);
  }

  return rc;
}


// Insert a block into the list of buffered blocks, discarding old
// blocks as necessary.

int BLKIOBUF::_insert_list(NNUM nn, char *pcDiskNode, unsigned fSetDelta)
{
  // If we have too many blocks to handle...

  if (num_blocks() >= uiMaxBlocks)
  {
    void *pvBlock = (void *)new char[uiBlkSize];

    if (!pvBlock)
      return FALSE;

    NNUM nnLRU;                    // Node number of LRU block
    unsigned fDelta;              // Delta setting of LRU block

    // Get the least recently-used block

    if (get_lru(&nnLRU, pvBlock, &fDelta))
    {
//      printf("\a*** removed %lx from cache (lru) - delta=%d\n", nnLRU, fDelta);

      // If the block has been changed, write it back to disk

      if (fDelta)
        BLKIO::put(nnLRU, (char *)pvBlock);
    }
    else
    {
      printf("\a***couldn't get lru block\n");
    }

    delete [] (char *)pvBlock;
  }

  return BLOCKBUF::put(nn, pcDiskNode, fSetDelta);
}


