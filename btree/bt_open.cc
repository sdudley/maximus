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

/* $Id: bt_open.cc,v 1.2 2004/01/22 09:02:29 wmcbrine Exp $ */

#include <string.h>
#include "btree.h"
#include "btreep.h"

// Default constructor just indicates that the file is closed.  Allow
// for up to 100 nodes in the B-tree cache.

CPPEXPORT BTREE::BTREE() : BLKIOBUF(512)
{
  fOpen=FALSE;
}


// Default destructor.  If this B-tree is currently open, close it.

CPPEXPORT BTREE::~BTREE()
{
  if (fOpen)
    close();
}


// File-open routine for a Btree.  If no order is specified, assume
// a default order of 8.

int CPPEXPORT BTREE::open(char *pszFile, keycomp_t kf_ins, keycomp_t kf_srch, unsigned uiKeySz, unsigned new_file, unsigned uiOrd, unsigned fBuffer)
{
  // Don't let the user open more than one base for this instance

  if (fOpen)
    return FALSE;

  // Make sure that uiOrder is acceptable

  if (uiOrd < 4)
    return FALSE;

  this->bteLastError = BTE_NONE;

  // Store the function to be used for comparing keys

  this->kf_ins=kf_ins;
  this->kf_srch=kf_srch;

  // Try to open the named B-tree file

  unsigned fNewFile=FALSE;

  if (! BLKIOBUF::open(pszFile, FALSE))
  {
    if (!new_file || ! BLKIOBUF::open(pszFile, TRUE))
      return FALSE;
    else fNewFile=TRUE;
  }

  DISKFILEHDR dfh;                      // Current B-tree information

  if (fNewFile)
  {
    dfh.nnFree=0L;                      // No free nodes to start with
    dfh.nnHead=0L;                      // Default to no root node
    dfh.usOrder=(unsigned short)uiOrd;  // Use the specified values
    dfh.usKeySize=(unsigned short)uiKeySz; // for order and key size
    dfh.cKeys=0L;                       // No keys in an empty tree
  }
  else
  {
    // Read the disk file header to determine the real block size

    if (! get_header((char *)&dfh, sizeof dfh))
    {
      this->bteLastError = BTE_CANTREAD;
      BLKIOBUF::close();
      return FALSE;
    }
  }

  // Copy the information from the on-disk header into our own
  // class information

  cKeys=dfh.cKeys;
  nnFree=dfh.nnFree;
  nnHead=dfh.nnHead;
  uiOrder=dfh.usOrder;
  uiKeySize=dfh.usKeySize;

  // Now calculate the size of each node, based on the information
  // that we have retrieved from the file header.

  uiNodeSize = sizeof(DISKNODEHDR) +
               (sizeof(NNUM) * (uiOrder+1)) +
               (uiKeySize * uiOrder);

  // Configure the BLKIOBUF library for this size.

  set_block_size(uiNodeSize);

  // Turn on the BLKIOBUF cache

  if (fBuffer)
    enable_buffer(TRUE);

  // If we're creating a new file, write the initial header file

  if (fNewFile)
  {
    // Create a blank header to blank out the first block

    char *p=new char[uiNodeSize];
    int rc=FALSE;

    if (p)
    {
      memset(p, 0, uiNodeSize);

      // Try to put it to disk, and then free its memory

      rc=put_header(p, uiNodeSize);
      delete [] p;

      // If that worked, try to write the real file header

      if (rc)
        rc=put_header((char *)&dfh, sizeof dfh);
    }

    // If it didn't work, get out.

    if (!rc)
    {
      BLKIOBUF::close();
      return FALSE;
    }
  }

  // Figure out which number to use for assigning new nodes at EOF

  nnHighNode=high_node();

  // Indicate that the file is now open

  fOpen=TRUE;
  return TRUE;
}

// Routine to write the file header to disk

int BTREE::write_file_header()
{
  DISKFILEHDR dfh;

  // Copy the information from this class instance back into
  // the disk header, then write to disk

  dfh.cKeys=cKeys;
  dfh.nnFree=nnFree;
  dfh.nnHead=nnHead;
  dfh.usOrder=(unsigned short)uiOrder;
  dfh.usKeySize=(unsigned short)uiKeySize;

  // Write the file header

  return put_header((char *)&dfh, sizeof dfh);
}

// Destroy the B-tree

int CPPEXPORT BTREE::close()
{
  if (!fOpen)
    return FALSE;

  write_file_header();

  BLKIOBUF::close();

  // Set the 'file open' flag to false

  fOpen=FALSE;
  return TRUE;
}

