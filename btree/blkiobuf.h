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

/* $Id: blkiobuf.h,v 1.1 2002/10/01 17:49:21 sdudley Exp $ */

#include "blkio.h"
#include "bbuf.h"

// BLKIOBUF:  Buffered block I/O class

class BLKIOBUF : public BLKIO, private BLOCKBUF
{
  NNUM nnHighPut;                       // Highest node # put to file
  unsigned uiBlkSize;                   // Block size for file
  unsigned uiMaxBlocks;                 // Max # of blocks to buffer

  int _insert_list(NNUM nn, char *pcDiskNode, unsigned fSetDelta);

public:
  CPPEXPORT BLKIOBUF(unsigned uiMaxBlocks = 32);
  virtual CPPEXPORT ~BLKIOBUF();

  int CPPEXPORT enable_buffer(unsigned fEnable);
  int CPPEXPORT flush_buffer(void);

  // Functions overriden from BLKIO

  virtual int CPPEXPORT close();
  virtual int CPPEXPORT set_block_size(unsigned int uiBlkSize);

  virtual int CPPEXPORT get(NNUM nn, char *pcDiskNode);
  virtual int CPPEXPORT put(NNUM nn, char *pcDiskNode);
  virtual NNUM CPPEXPORT high_node(void);

  // Functions inherited from BLKIO but not overriden:
  //
  //int open(char *szPath, int fNewFile);
  //int get_header(char *pcDiskNode, unsigned uiSize);
  //int put_header(char *pcDiskNode, unsigned uiSize);
  //int lock(NNUM nn);
  //int unlock(NNUM nn);
};

