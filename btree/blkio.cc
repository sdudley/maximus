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
static char rcs_id[]="$Id: blkio.cc,v 1.2 2003/06/04 22:33:43 wesgarland Exp $";
#pragma on(unreferenced)

// Block I/O functions for B-tree and database routines

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "blkio.h"

#ifdef __MSDOS__
  extern "C" { sword far pascal shareloaded(void); };
#else
  #define shareloaded() TRUE
#endif

// Constructor for the block I/O class

CPPEXPORT BLKIO::BLKIO()
{
  fShareLoaded=shareloaded();
  fOpen=FALSE;
}

// Destructor for the block I/O class

CPPEXPORT BLKIO::~BLKIO()
{
  if (fOpen)
    close();

  fOpen=FALSE;
}


// Open a file for block I/O

int CPPEXPORT BLKIO::open(char *szPath, int fNewFile)
{
  if ((fd=::open(szPath, O_RDWR | O_BINARY |
                   (fNewFile ? (O_CREAT | O_TRUNC) : 0),
                 S_IREAD | S_IWRITE))==-1)
  {
    return FALSE;
  }

  uiBlkSize=0;
  fOpen=TRUE;
  return TRUE;
}


// Close a block I/O file

int CPPEXPORT BLKIO::close()
{
  if (!fOpen)
    return FALSE;

  ::close(fd);
  fOpen=FALSE;

  return TRUE;
}


// Set the block size for this file

int CPPEXPORT BLKIO::set_block_size(unsigned int uiBlk)
{
  if (!fOpen)
    return FALSE;

  uiBlkSize=uiBlk;
  return TRUE;
}


// Position the internal file pointer to a specified block number

int BLKIO::position(NNUM nn)
{
  long ofs=nn * (long)uiBlkSize;

  // Try to seek to the appropriate position

  return lseek(fd, ofs, SEEK_SET)==ofs;
}


// Read a block from this file

int CPPEXPORT BLKIO::get(NNUM nn, char *pcDiskNode)
{
  if (!uiBlkSize || !fOpen || !position(nn))
    return FALSE;

  // Make sure that we can read the right number of bytes from disk

  return (read(fd, pcDiskNode, uiBlkSize) == (signed)uiBlkSize);
}


// Write a block to this file

int CPPEXPORT BLKIO::put(NNUM nn, char *pcDiskNode)
{
  if (!uiBlkSize || !fOpen || !position(nn))
    return FALSE;

  return (write(fd, pcDiskNode, uiBlkSize) == (signed)uiBlkSize);
}


// Get a variable-length header from the beginning of the file

int CPPEXPORT BLKIO::get_header(char *pcDiskNode, unsigned uiSize)
{
  lseek(fd, 0L, SEEK_SET);

  return read(fd, pcDiskNode, uiSize)==(signed)uiSize;
}

// Put a variable-length header to the beginning of the file

int CPPEXPORT BLKIO::put_header(char *pcDiskNode, unsigned uiSize)
{
  lseek(fd, 0L, SEEK_SET);

  return write(fd, pcDiskNode, uiSize)==(signed)uiSize;
}


// Returns the node number for EOF

NNUM CPPEXPORT BLKIO::high_node(void)
{
  return (NNUM)lseek(fd, 0L, SEEK_END) / (NNUM)uiBlkSize;
}


// Lock a certain record of the file.

int CPPEXPORT BLKIO::lock(NNUM nn)
{
#if defined(UNIX)
  struct flock	lck;

  lck.l_type	= F_WRLCK;			/* setting a write lock */
  lck.l_whence	= SEEK_SET;			/* offset l_start from beginning of file */
  lck.l_start	= (off_t)nn * (long)uiBlkSize;	
  lck.l_len	= nn ? uiBlkSize : (off_t)0;	/* one record, or until the end of the file */
  
  return (fcntl(fd, F_SETLK, &lck) < 0) ? FALSE : TRUE;
#else
  if (fShareLoaded)
    return ::lock(fd, nn * (long)uiBlkSize, 1)==0;
  else
    return TRUE;
#endif
}


// Unlock a certain record of the file.

int CPPEXPORT BLKIO::unlock(NNUM nn)
{
#if defined(UNIX)
  struct flock	lck;

  lck.l_type	= F_UNLCK;			/* setting not locked */
  lck.l_whence	= SEEK_SET;			/* offset l_start from beginning of file */
  lck.l_start	= (off_t)nn * (long)uiBlkSize;	
  lck.l_len	= nn ? uiBlkSize : (off_t)0;	/* one record, or until the end of the file */
  
  return (fcntl(fd, F_SETLK, &lck) < 0) ? FALSE : TRUE;
#else
  if (fShareLoaded)
    return ::unlock(fd, nn * (long)uiBlkSize, 1)==0;
  else
    return TRUE;
#endif
}

