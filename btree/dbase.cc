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
static char rcs_id[]="$Id: dbase.cc,v 1.1 2002/10/01 17:49:28 sdudley Exp $";
#pragma on(unreferenced)

#ifdef OS_2
  extern "C"
  {
      #define INCL_DOS
      #include <os2.h>
  };
#endif

#include <stdio.h>
#include <string.h>
#include <new.h>
#include "dbase.h"

#ifndef max
#define max(a,b)              (((a) > (b)) ? (a) : (b))
#define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif


// Constructor for the database object

CPPEXPORT DBASE::DBASE()
{
  fOpen=FALSE;
}

// Default destructor.  If the database is still open, close it.

CPPEXPORT DBASE::~DBASE()
{
  if (fOpen)
    close();
}


// Lock the database file for exclusive access.  This is used to
// ensure that two tasks cannot write to the database file at
// the same time.

int CPPEXPORT DBASE::obtain_lock(void)
{
  int iTries=10;

  // If we already have a previous lock, just increment the counter

  if (uiLockCnt++)
    return TRUE;

  while (iTries--)
  {
    // If we successfully lock the file, read in the database
    // header to make sure that it is up-to-date.

    if (lock(0))
    {
      // Flush the buffer

      enable_buffer(TRUE);
      return read_dbhdr();
    }

#ifdef OS_2
    DosSleep(500);
#endif
  }

  uiLockCnt--;
  return FALSE;
}



// Releasre a database that was locked with obtain_lock().

int CPPEXPORT DBASE::release_lock(void)
{
  // If we have more locks to go, do nothing

  if (--uiLockCnt)
    return TRUE;

  // Write the file header out to disk

  int rc=write_dbhdr();

  // Flush the file buffer

  if (!enable_buffer(TRUE))
    rc=FALSE;

  // Release the database lock

  if (!unlock(0))
    rc=FALSE;

  return rc;
}



// Read the file header for the .db file

int DBASE::read_dbhdr(void)
{
  DISKDBASEHDR ddh;

  if (!get_header((char *)&ddh, sizeof ddh))
    return FALSE;

  nnFree=ddh.nnFree;
  return TRUE;
}


// Write the file header for the .db file

int DBASE::write_dbhdr(void)
{
  DISKDBASEHDR ddh;

  ddh.nnFree=nnFree;

  if (!put_header((char *)&ddh, sizeof ddh))
    return FALSE;

  return TRUE;
}



// Function to open the current database

int CPPEXPORT DBASE::open(char *szNam, FIELD *pf, unsigned uiNumFields,
                          unsigned new_file, unsigned uiOrder, int fVarLength)
{
  int i;

  if (fOpen)
    return -100;

  // Copy information into the local database header

  uiFields=uiNumFields;

  // Allocate memory for the name

  if ((szName=new char[strlen(szNam)+1])==0)
    return -50;

  strcpy(szName, szNam);

  // Allocate memory for the array of field descriptors

  if ((pfFields=new FIELD[uiFields])==0)
  {
    delete [] szName;
    return -50;
  }

  memmove(pfFields, pf, sizeof(FIELD) * uiFields);

  // Determine the record size for this file

  for (uiRecSize=0, i=0; i < uiFields; i++)
    uiRecSize += pfFields[i].uiSize;

  // Include a block count and 'next' link for variable-length databases.

  if (fVarLength)
    uiRecSize += sizeof(NNUM)*2;

  this->fVarLen = fVarLength;

  // Allocate memory for a filename buffer

  char *fname=new char[strlen(szName)+10];

  // Try to open the database files

  try
  {
    if (!fname)
      throw -50;

    // Initially, we have no B-tree indices

    pbIndex=0;
    pbIndexBuf=0;

    // Count the number of indices in this database

    for (uiIndex=0;
         uiIndex < uiFields && pfFields[uiIndex].kf;
         uiIndex++)
      ;

    // We need at least one index field!

    if (!uiIndex)
      throw -1;

    // Try to open the .DB data file

    strcpy(fname, szName);
    strcat(fname, ".db");

    unsigned fNewFile=FALSE;

    if (!BLKIOBUF::open(fname, FALSE))
    {
      if (!new_file || !BLKIOBUF::open(fname, TRUE))
        throw -2;
      else
        fNewFile=TRUE;
    }

    // Initialize a new .DB file header, if necessary

    if (fNewFile)
    {
      nnFree=0;

      // Zero out the initial header, write to disk, and free memory

      char *pc=new char[uiRecSize];

      if (!pc)
        throw -50;

      memset(pc, 0, uiRecSize);

      int rc=put_header(pc, uiRecSize);

      delete [] pc;

      // Now write the real database header

      if (!rc || !write_dbhdr())
        throw -2;
    }
    else
    {
      // Read the header from an existing .DB file

      if (!read_dbhdr())
        throw -2;
    }


    // KLUDGE ALERT:
    //
    // Allocate memory for the B-tree indices.
    //
    // We can't just do "pbIndex=new BTREE[uiIndex];" here, since
    // WC 9.5a calls the new() function from the MAXDB.DLL module,
    // but it calls the delete() function from MAXBT.DLL, thereby
    // trashing the heap.  Consequently, we do all allocation
    // explicitly using the placement operators.

                                                // for WC's array count
    pbIndexBuf=new char[uiIndex * sizeof(BTREE) + sizeof(long)];

    if (!pbIndexBuf)
      throw -50;

    pbIndex=new (pbIndexBuf) BTREE[uiIndex];

    if (!pbIndex)
      throw -50;

    // Now loop around and open each B-tree index

    for (i=0; i < uiIndex; i++)
    {
      // Create the index file dbasenam.iXX for each index

      strcpy(fname, szName);
      sprintf(fname+strlen(fname), ".i%02x", i);

      // Return -3 .. -(n+3) if we cannot open an index file

      if (!pbIndex[i].open(fname, pfFields[i].kf, pfFields[i].kf_base,
                           pfFields[i].uiSize + sizeof(NNUM),
                           new_file, uiOrder))
        throw (-3 - i);
    }

    // We don't know how big the .db file is as of yet

    nnLastFree=0L;
  }
  catch(int rc)
  {
    // Exception handler for file-opening problems

    if (pbIndex)
      for (i=0; i < uiIndex; i++)
      {
        pbIndex[i].close();
        pbIndex[i].~BTREE();
      }

    delete [] pbIndexBuf;

    // Close the main database file

    BLKIOBUF::close();

    delete [] fname;
    return rc;
  }

  delete [] fname;


  // Notify the I/O library of this change

  set_block_size(uiRecSize);
  enable_buffer(TRUE);

  fOpen=TRUE;
  uiLockCnt=0;
  return 0;
}



// Function to close the current database

int CPPEXPORT DBASE::close()
{
  if (!fOpen)
    return FALSE;

  // Close all of the B-tree indices

  for (int i=0; i < uiIndex; i++)
  {
    pbIndex[i].close();
    pbIndex[i].~BTREE();
  }

  delete [] pbIndexBuf;

  // Dump our current status into the .db file header

  if (!write_dbhdr())
  {
    //printf("Error writing .db file header!\n");
    return FALSE;
  }

  // Close the .db file itself

  BLKIOBUF::close();

  // Deallocate memory used by the database handler

  delete [] pfFields;
  delete [] szName;

  fOpen=FALSE;
  return TRUE;
}


// Get a free block from the .db file

NNUM DBASE::get_free_block(void)
{
  NNUM rc;

  // If we have a list of free blocks, use those first.

  if (nnFree)
  {
    char *pcRecord=new char[uiRecSize];

    if (!pcRecord)
      return 0;

    // Use the first block in the free list

    rc=nnFree;

    // Try to read the free block to update the 'free' list

    if (! BLKIOBUF::get(nnFree, pcRecord))
    {
      rc=0;
    }
    else
    {
      // Use the next free block in the linked list.

      nnFree=*(NNUM *)pcRecord;
    }

    delete [] pcRecord;
    return rc;
  }

  if (nnLastFree)
    return nnLastFree++;


  // Nothing left in the free list, so allocate a block at EOF.

  nnLastFree=high_node();

  return nnLastFree++;
}



// Insert the record 'pvRecord' into the database

int CPPEXPORT DBASE::insert(void *pvRecord, int iLength)
{
  NNUM nnData;    // Block for storing the entire record
  char *pcKey;    // Temp storage for inserting each key
  char *pcRecord=(char *)pvRecord;

  if (!fOpen)
    return FALSE;

  // First check to see if this record is already in the database

  char *pcBuf=new char[uiRecSize];

  if (!pcBuf)
    return FALSE;

  PALIST pl;  // List for storing our parents

  while ((pcKey=(char *)pbIndex[0].lookup(pcRecord + pfFields[0].uiOfs, &pl)) != 0)
  {
    // Get the record number for this record

    NNUM nnRec=*(NNUM *)(pcKey + pfFields[0].uiSize);

    if (! BLKIOBUF::get(nnRec, pcBuf))
    {
      //printf("Error validating record %ld for insert!\n", nnRec);
      delete pcBuf;
      return FALSE;
    }

    // Loop through all of the index fields to make sure that
    // at least one is different

    for (int j=0; j < uiIndex; j++)
      if ((*pfFields[j].kf_base)((char *)pvRecord + pfFields[j].uiOfs,
                                 pcBuf + pfFields[j].uiOfs) != 0)
        break;

    if (j==uiIndex)
    {
      //printf("Error!  Attempt to insert duplicate record!\n");
      delete pcBuf;
      return FALSE;
    }
  }

  // Deallocate memory for temporary record buffer

  delete [] pcBuf;


  // Now get a lock on the database file

  if (!obtain_lock())
    return FALSE;

  // Do stuff while we have the lock:

  try
  {
    // Allocate a record for this item in the database file

    nnData=get_free_block();

    // Now that we are sure that our key fields make this a distinct
    // record, it's okay to insert the keys into the database.

    for (int i=0; i < uiIndex; i++)
    {
      // Allocate mem for length of key plus offset into data file

      pcKey=new char[pfFields[i].uiSize + sizeof(NNUM)];

      if (!pcKey)
        throw NoMem(0);

      // Move the key itself into the buffer

      memmove(pcKey, pcRecord + pfFields[i].uiOfs, pfFields[i].uiSize);
      memmove(pcKey + pfFields[i].uiSize, &nnData, sizeof(NNUM));


      if (!pbIndex[i].insert(pcKey, IF_DUPE))
      {
        //printf("Error inserting key in tree!\n");
        delete pcKey;
        throw 0;
      }

      delete [] pcKey;
    }
  }
  catch (NoMem)
  {
    release_lock();
    return FALSE;
  }
  catch (int)
  {
    release_lock();
    return FALSE;
  }
  catch (...) // catch all other exceptions
  {
    release_lock();
    throw;
  }

  int rc = FALSE;

  // Write this record to the .db file.  But special handling is needed
  // for variable-length records.

  if (!fVarLen)
    rc = BLKIOBUF::put(nnData, (char *)pvRecord);
  else
  {
    char *pcDupBlock = new char[uiRecSize];

    if (!pcDupBlock)
      rc = FALSE;
    else
    {
      int iDataSize = uiRecSize - VARLENGTH_DATA;
      char *pcEnd = (char *)pvRecord + iDataSize;
      NNUM nnNext;

      // If no length specified, assume the standard record size.

      if (iLength==0)
        iLength = iDataSize;

      // Now write one block to the data file for each iDataSize
      // increment of data in the source record.

      for (char *pcSrc = (char *)pvRecord;
           pcSrc < pcEnd;
           pcSrc += iDataSize)
      {
        memcpy(pcDupBlock + VARLENGTH_DATA, pcSrc, iDataSize);

        // If this is the last block in the record, set the
        // next field to 0.  Otherwise, allocate a block for the
        // next write.

        if (pcSrc + iDataSize >= pcEnd)
          nnNext = 0;
        else
          nnNext = get_free_block();

        // Store the next field in the record header.

        *(NNUM *)(pcDupBlock + iDataSize)                = nnNext;
        *(NNUM *)(pcDupBlock + iDataSize + sizeof(NNUM)) = iLength;

        // Write this block to disk

        rc = BLKIOBUF::put(nnData, (char *)pvRecord);

        if (!rc)
            break;

        if (nnNext)
          nnData = nnNext;
      }

      delete [] pcDupBlock;
    }
  }

  release_lock();
  return rc;
}


// Update the record 'pvRecOld' with the information in 'pvRecNew'

int CPPEXPORT DBASE::update(void *pvRecOld, void *pvRecNew, int iLength)
{
  if (!fOpen)
    return FALSE;

  void **ppv=new void *[uiIndex];

  if (!ppv)
    return FALSE;

  // Convert the pvRecOld (flat memory) into an array of void pointers
  // that point to the start of each of the key fields

  for (int i=0; i < uiIndex; i++)
    ppv[i]=(char *)pvRecOld + pfFields[i].uiOfs;

  int rc=TRUE;

  if (!remove(ppv) || !insert(pvRecNew, iLength))
    rc=FALSE;

  delete ppv;
  return rc;
}

// This function returns TRUE if block 'nn' is not in the 'free list'

int DBASE::_block_in_nnflist(NNFLIST *pnnfHead, NNUM nn)
{
  NNFLIST *pnnf=pnnfHead;

  while (pnnf)
  {
    if (pnnf->nnFree==nn)
      return TRUE;
  }

  return FALSE;
}

// Get a linked list of free records within the database

NNFLIST * DBASE::_get_free_records(void)
{
  NNUM nn = nnFree;
  NNFLIST *pnnfHead=NULL;
  NNFLIST *pnnf;
  char *pcRecord=new char[uiRecSize];

  if (!pcRecord)
    return NULL;

  while (nn)
  {
    if (! BLKIOBUF::get(nn, pcRecord))
      break;

    if ((pnnf=new NNFLIST) != NULL)
    {
      pnnf->nnFree=nn;
      pnnf->next=pnnfHead;
      pnnfHead=pnnf;
    }

    // Use the next free block in the linked list.

    nn=*(NNUM *)pcRecord;
  }

  delete [] pcRecord;
  return pnnfHead;
}



#if 0 /* notused */

// findseq_open
//
// Perform a "sequential" look-up, returning a record from the database
// file in no specified order.  This is much faster than an ordered
// look-up because the b-tree indices are not used.

SEQFIND CPPEXPORT DBASE::findseq_open(void *pvRec)
{
  SEQFIND sf;
  NNUM nn=(NNUM)1;

  // Sequential find not currently supported for variable-length records

  if (fVarLen)
    return NULL;

  if ((sf=new(struct _seqfind))==0)
    return NULL;

  sf->pnnf=_get_free_records();
  sf->pvRec=pvRec;

  for (; nn <= size(); nn++)
    if (!_block_in_nnflist(sf->pnnf, nn))
      break;

  if (nn > size())
  {
    delete sf;
    return NULL;
  }

  /* Now try to read the record we found */

  if (! BLKIOBUF::get(nn, (char *)pvRec))
  {
    delete sf;
    return NULL;
  }

  sf->nnLast=nn;
  return sf;
}

// findseq_next
//
// Find the next record in a sequential database search.  Returns
// a pointer to the found record; FALSE otherwise.  (The record
// is stored in the data area in the void*pvRec pointer passed
// to findseq_open.)

void * CPPEXPORT DBASE::findseq_next(SEQFIND sf)
{
  if (!sf)
    return (void *)0;

  NNUM nn=sf->nnLast+1;

  while (nn <= size())
    if (!_block_in_nnflist(sf->pnnf, nn))
      break;

  sf->nnLast=nn;

  if (nn > size() || !BLKIOBUF::get(nn, (char *)sf->pvRec))
    return NULL;

  return sf->pvRec;
}

int CPPEXPORT DBASE::findseq_close(SEQFIND sf)
{
  if (!sf)
    return FALSE;

  NNFLIST *pnnf, *pnnfNext;

  // Free the linked list of free node numbers

  for (pnnf=sf->pnnf; pnnf; pnnfNext=pnnf->next, delete pnnf, pnnf=pnnfNext)
    ;

  delete sf;
  return TRUE;
}
#endif


// Remove the specified record from the database

int CPPEXPORT DBASE::remove(void **ppvFields)
{
  if (!fOpen)
    return FALSE;

  NNUM nn;
  int rc;

  // Allocate memory for a dummy record, then read the block into it
  // to make sure that it exists.  By doing this, we get the node
  // number placed in nn.

  void *pvFoundRec=(void *)new char[uiRecSize];

  if (!pvFoundRec)
    return FALSE;

  rc=ppvFields && _lookup(ppvFields, 0, pvFoundRec, &nn, 0, TRUE, 0, 0);

  delete [] pvFoundRec;

  if (!rc)
    return FALSE;

  if (!obtain_lock())
    return FALSE;

  // Do stuff while we have the database locked:

  try
  {
    for (int i=0; i < uiIndex; i++)
    {
      PALIST pl;
      char *pc;
      int gotit=FALSE;

      while ((pc=(char *)pbIndex[i].lookup(ppvFields[i], &pl)) != 0)
      {
        if (*(NNUM *)(pc+pfFields[i].uiSize)==nn)
        {
          if (!pbIndex[i].removep(&pl))
          {
            //printf("DB: Error removing key from index %d!\n", i);
            return FALSE;
          }

          gotit=TRUE;
          break;
        }
      }

      if (!gotit)
      {
        //printf("DB: Couldn't find key for removal from index %d!\n", i);
        return FALSE;
      }
    }

    // Add this record to the free list

    char *pcRecord=new char[uiRecSize];

    if (pcRecord)
    {
      NNUM nnHead = nn;

      // If doing variable-length records, make sure that we free
      // all of the records in the chain.

      if (fVarLen)
      {
        for (;;)
        {
          if (!BLKIOBUF::get(nn, pcRecord))
            return FALSE;

          // If we found the end of the chain, we can now stop freeing records.

          if (*(NNUM *)pcRecord==0)
            break;

          nn = *(NNUM *)pcRecord;
        }

        // At this point, nnHead points to the first file record associated
        // with the item that we removed.  nn points to the last file
        // record associated with the node.  The links at the beginning
        // of the record are the same as those used in the free list,
        // so to add all of these records to the list, we only need to
        // point the head of the free list at nnHead, and update the
        // pointer in nn to point to the old head of the free list.
        //
        // This can be done using the same code for non-variable
        // records, below.
      }

      // Link the next free list into this record

      *(NNUM *)pcRecord=nnFree;

      if (! BLKIOBUF::put(nn, pcRecord))
      {
        //printf("Error updating free-next list!\n");
        return FALSE;
      }

      delete [] pcRecord;
      nnFree=nnHead;
    }
  }
  catch (int)
  {
    release_lock();
    return FALSE;
  }
  catch (NoMem)
  {
    release_lock();
    return FALSE;
  }

  // Add this to the head of the free list

  release_lock();
  return TRUE;
}


// See if the record that we found is a match

int DBASE::lookup_match(void **ppvFields, void *pvFoundRec)
{
  // Loop through all of the keys and check for a match.  If it's
  // not a match (and the template in ppvFields is not a wildcard)
  // we must exit.

  for (int i=0; i < uiIndex; i++)
    if (ppvFields[i] &&
        (*pfFields[i].kf_base)(ppvFields[i],
                               (char *)pvFoundRec + pfFields[i].uiOfs) != 0)
      break;

  // return true if all fields matched

  return i==uiIndex;
}


// Perform a search on the records in the database

int DBASE::_lookup(void **ppvFields, PALIST *ppl, void *pvFoundRec,
                   NNUM *pnn, unsigned uiIdx, int fForward, int iLength,
                   unsigned long *pulLength)
{
  void * (CPPEXPORT BTREE::*pfLook)(void *key, PALIST *pl);
  PALIST plParent;
  int rc = FALSE;
  int i;

  // Handle either forward or backwards searches

  if (fForward)
    pfLook=BTREE::lookup;
//  else
//    pfLook=BTREE::lookupr;


  // If no traversal object is specific, provide our own for the user
  // to use.

  if (!ppl)
  {
    // Allocate enough memory to hold parent stacks for each key

    ppl=&plParent;
  }

  // Now loop through all of the fields and try to find a match

  for (i=0; i < uiIndex && ppvFields && !ppvFields[i]; i++)
    ;


  /* If all fields are null... */

  if (!ppvFields || i==uiIndex)
    i=uiIdx;


  // Keep looping while we have matches on this key

  char *pc;
                      
  while (rc==FALSE &&
         (pc=(char *)(pbIndex[i].*pfLook)(ppvFields ? ppvFields[i] : 0,
                                          ppl)) != 0)
  {
    // Extract the record number from this key field

    NNUM nnRec=*(NNUM *)(pc + pfFields[i].uiSize);
    char *pcDupBlock;

    // Try to retrieve the record for this key

    if (fVarLen)
    {
      pcDupBlock = new char [uiRecSize];

      if (!pcDupBlock)
        return FALSE;

      if (!BLKIOBUF::get(nnRec, (char *)pcDupBlock))
      {
        delete [] pcDupBlock;
        return FALSE;
      }
    }
    else
    {
      if (! BLKIOBUF::get(nnRec, (char *)pvFoundRec))
        return FALSE;

      pcDupBlock = (char *)pvFoundRec;
    }

    // Does this match all of the other search criteria?  If so,
    // return to caller.

    if (!ppvFields || lookup_match(ppvFields, pcDupBlock))
    {
      // If the caller asked for a node number of the found
      // record, give it to them here.

      if (pnn)
        *pnn=nnRec;

      rc = TRUE;
    }

    // Handle special stuff for the variable-length records here.

    if (fVarLen)
    {
      // If we found a match, copy the requested amount of information
      // to the caller's buffer.

      if (rc)
      {
        int iDataSize = uiRecSize - VARLENGTH_DATA;
        char *pcDest;

        // If no explicit length specified, copy only one record.

        if (iLength==0)
          iLength = iDataSize;

        // If the caller wants the record length, give it to 'em.

        if (pulLength)
          *pulLength = *(NNUM *)(pcDupBlock + sizeof(NNUM));

        // Now loop while we still have a 'next' link in the record
        // and we still have data to copy.

        for (pcDest = (char *)pvFoundRec;
             iLength && *(NNUM *)pcDupBlock != 0;
             )
        {
          int iCopySize = min(iLength, iDataSize);

          // Copy it to the caller's buffer and adjust pointers

          memcpy(pcDest, pcDupBlock + VARLENGTH_DATA, iCopySize);
          pcDest += iCopySize;
          iLength -= iCopySize;

          // Now get the next block in the list

          if (! BLKIOBUF::get(*(NNUM *)pcDupBlock, (char *)pcDupBlock))
            return FALSE;
        }
      }

      delete [] pcDupBlock;
    }
  }

  // Couldn't find a match for this record

  return rc;
}


// Perform a database lookup.
//
// ppl is the 'parent list' for this lookup.  For doing a single
// lookup, ppl can be null.  For performing a browse on multiple
// keys, the caller should create a PALIST object and pass it to lookup
// for successive iterations until lookup returns FALSE.
//
// ppvFields is the array of pointers to keys to use for this search.
// A null pointer for any element of that array indicates a wildcard.
//
// pvFoundRec is where the found database record will be placed.
//
// uiIdx indicates the index number to use for performing the lookup.
// This doesn't change the records which are returned, but it does
// ensure that the records are returned in the order of a particular
// key.
//
// lookup returns TRUE if a record was found; FALSE otherwise.

int CPPEXPORT DBASE::lookup(void **ppvFields, PALIST *ppl, void *pvFoundRec,
                            unsigned uiIdx, int iLength,
                            unsigned long *pulLength)
{
  if (!fOpen)
    return FALSE;

  return _lookup(ppvFields, ppl, pvFoundRec, 0,
                 uiIdx, TRUE, iLength, pulLength);
}


// Reverse search

//int CPPEXPORT DBASE::lookupr(void **ppvFields, PALIST *ppl, void *pvFoundRec,
//                             unsigned uiIdx, int iLength,
//                             unsigned long *pulLength)
//{
//  if (!fOpen)
//    return FALSE;
//
//  return _lookup(ppvFields, ppl, pvFoundRec, 0,
//                 uiIdx, FALSE, iLength, pulLength);
//}


// Return a pointer to our internal array of B-tree index objects

BTREE * CPPEXPORT DBASE::get_btrees(void)
{
  return pbIndex;
}


// Return the number of records stored in the database

unsigned long CPPEXPORT DBASE::size(void)
{
  return pbIndex[0].size();
}

#ifdef TEST

/////////////////////////////////////////////////////////////////////////////
// Test harness for the database manager
/////////////////////////////////////////////////////////////////////////////

#define NAME_SIZE       36
#define LOCATION_SIZE   40

typedef struct
{
  char name[NAME_SIZE];
  char location[LOCATION_SIZE];
  int age;
} DBREC;


// Compare two ASCIIZ strings

int asciizcomp(void *a1, void *a2)
{
  return stricmp((char *)a1, (char *)a2);
}


int main(void)
{
  static DBASE db;

  static FIELD f[]=
  {
    {"Name",      NAME_SIZE,  asciizcomp, asciizcomp},
    {"Location",  LOCATION_SIZE, asciizcomp, asciizcomp},
    {"Age",       sizeof(int), 0}
  };

  int rc;

  // Open the database

  if ((rc=db.open("employee", f, 3, TRUE)) != 0)
  {
    printf("Error opening database!\n");
    return 1;
  }

  static DBREC dbra[]=
  {
    {"Scott Dudley", "Kingston, Ontario", 19},
    {"Scott Dudley", "Basingstoke, Hants", 19},
    {"Scott Dudley", "Waterloo, Ontario", 19},
    {"Bill Cassidy", "Kingston, Ontario", 41},
    {"Peter Fitzsimmons", "Mississauga, Ontario", 27},
    {"Tom Hall", "Calgary, Alberta", 35},
    {"Peter Zion", "Kingston, Ontario", 19},
    {"George Wu", "Montreal, Quebec", 21},
    {"Cameron Lacy", "Waterloo, Ontario", 19},
    {"Grant Whitehead", "Waterloo, Ontario", 20},
    {"George Wu", "Kingston, Ontario", 21},
    {"Scott Irving", "Waterloo, Ontario", 19},
    {"Kevin Hare", "Waterloo, Ontario", 19},
    {"Dan Fischer", "Guelph, Ontario", 18},
    {"John Souvestre", "New Orleans, LA", 35},
    {"Mike Phillips", "Waterloo, Ontario", 20},
    {"Brian Stecher", "Waterloo, Ontario", 29},
    {"Jason Cheeong-Kee-You", "Waterloo, Ontario", 20},
    {"Mike Gostzynski", "Waterloo, Ontario", 21},
    {"Kevin O'Keefe", "Waterloo, Ontario", 19},
    {"Yoon Hwan Lee", "Waterloo, Ontario", 21},
    {"Scott Irving", "Deep River, Ontario", 19},
    {"Kevin Hare", "Mississauga, Ontario", 19},
    {"George Wu", "Kingston, Ontario", 15},
    {"Bruce Norman", "Kingston, Ontario", 20},
    {"Dan Fischer", "Waterloo, Ontario", 18},
    {"Bruce Norman", "Waterloo, Ontario", 20},
    {"Dilbagh Singh", "Basingstoke, Hants", 29}
  };

  for (int i=0; i < sizeof(dbra)/sizeof(*dbra); i++)
  {
    printf("Inserting record #%d\n", i);

    if (!db.insert(dbra+i))
      printf("Error inserting database record %d!\n", i);
  }

  void *pvLookScott[]={"Scott Dudley", 0};
  void *pvLookKingston[]={0, "Kingston, Ontario"};
  void *pvLookWaterloo[]={0, "Waterloo, Ontario"};
  void *pvLookScottWat[]={"Scott Dudley", "Waterloo, Ontario"};
  static PALIST plScott, plKingston, plWaterloo, plScottWat;
  static DBREC dbFound;

  printf("\nLooking up Scott...\n");

  while (db.lookup(pvLookScott, &plScott, &dbFound))
    printf("Name: %s, City: %s, Age: %d\n",
           dbFound.name, dbFound.location, dbFound.age);

  printf("\nLooking up Kingston...\n");

  while (db.lookup(pvLookKingston, &plKingston, &dbFound))
    printf("Name: %s, City: %s, Age: %d\n",
           dbFound.name, dbFound.location, dbFound.age);

  printf("\nLooking up Waterloo...\n");

  while (db.lookup(pvLookWaterloo, &plWaterloo, &dbFound))
    printf("Name: %s, City: %s, Age: %d\n",
           dbFound.name, dbFound.location, dbFound.age);

  printf("\nLooking up Scott in Waterloo...\n");

  while (db.lookup(pvLookScottWat, &plScottWat, &dbFound))
    printf("Name: %s, City: %s, Age: %d\n",
           dbFound.name, dbFound.location, dbFound.age);

  db.close();
  return 0;
}

#endif

