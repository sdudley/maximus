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

/* $Id: blkio.h,v 1.1 2002/10/01 17:49:20 sdudley Exp $ */

// Block I/O class for B-tree and database routines

#ifndef __BLKIO_H_DEFINED
#define __BLKIO_H_DEFINED

#include "btype.h"                      // B-tree type definitions

class BLKIO
{
  int fd;                               // File handle used for I/O
  unsigned fOpen;                       // Has the file been opened?
  unsigned uiBlkSize;                   // Block size for this file
  unsigned fShareLoaded;                // Is SHARE.EXE loaded?

  int BLKIO::position(NNUM nn);

public:

  CPPEXPORT BLKIO();
  virtual CPPEXPORT ~BLKIO();

  int CPPEXPORT open(char *szPath, int fNewFile);
  int CPPEXPORT get_header(char *pcDiskNode, unsigned uiSize);
  int CPPEXPORT put_header(char *pcDiskNode, unsigned uiSize);

  virtual int CPPEXPORT set_block_size(unsigned int uiBlkSize);
  virtual int CPPEXPORT close();

  virtual int CPPEXPORT get(NNUM nn, char *pcDiskNode);
  virtual int CPPEXPORT put(NNUM nn, char *pcDiskNode);
  virtual int CPPEXPORT lock(NNUM nn);
  virtual int CPPEXPORT unlock(NNUM nn);

  virtual NNUM CPPEXPORT high_node(void);
};

#endif // __BLKIO_H_DEFINED

