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

/* $Id: dbase.h,v 1.1 2002/10/01 17:49:28 sdudley Exp $ */

#ifndef __DBASE_H_DEFINED
#define __DBASE_H_DEFINED

#include "btype.h"

#ifndef __BTREE_H_DEFINED
  #include "btree.h"
#endif


#define CACHE_RECORDS   32                // Store 32 records in .db cache
#define INDEX_ORDER     8                 // Use 16-way index files
#define VARLENGTH_DATA  (sizeof(NNUM)*2)  // Private data used at end of
                                          // record for variable-length
                                          // databases.


// An array of these structs is used to describe the entire database


// !!!!!!!! WARNING !!!!!!!!!!
// This structure must also be updated
// in dbasec.h as well as this file!
// !!!!!!!! WARNING !!!!!!!!!!


// Field structure.  An array of these is used to define the fields
// in the database.

typedef struct
{
  char *szKey;                            // Name of this key

  unsigned uiOfs;                         // Offset within dbase record
  unsigned uiSize;                        // Size of this field

  keycomp_t kf;                           // Comparison function that
                                          // does check the record number.

                                          // If kf==NULL, this field is
                                          // not an index.  All index fields
                                          // must be at front of record!

  keycomp_t kf_base;                      // Comparison function that
                                          // does NOT check the record
                                          // number following the
                                          // field data.
} FIELD;


// Structure used as the header of a .db file

typedef struct
{
  NNUM nnFree;                            // Pointer to the 1st free node
} DISKDBASEHDR;

typedef struct _nnfreelist
{
  NNUM nnFree;
  struct _nnfreelist *next;
} NNFLIST;

typedef struct _seqfind
{
  NNFLIST *pnnf;                      // List of free nodes to exclude
  NNUM nnLast;                        // Last node number to
  void *pvRec;
} *SEQFIND;

// Class object for a database, derived from the block I/O class

class DBASE : private BLKIOBUF
{
  char *szName;                           // Name of the database
  unsigned uiFields;                      // Number of fields
  unsigned fOpen;                         // Is this database open?
  FIELD *pfFields;                        // Fields in this database
  BTREE *pbIndex;                         // Btrees to hold indices
  char *pbIndexBuf;                       // WC kludge: buffer to hold pbIndex
  int fVarLen;                            // TRUE if variable-length records

  // Calculated fields

  unsigned uiIndex;                       // Number of indices
  unsigned uiRecSize;                     // Size of each data record
  unsigned *puiFieldOfs;                  // Offset of each field in record
  NNUM nnFree;                            // First free block in .db file
  NNUM nnLastFree;                        // Last block # in .db file
  unsigned uiLockCnt;                     // # of locks currently active

  int get_record(NNUM nn, void *pvBuf);   // Get record from .db file
  int put_record(NNUM nn, void *pvBuf);   // Put record in .db file
  NNUM get_free_block(void);              // Get offset of free block
  int read_dbhdr(void);                   // Read header of .db file
  int write_dbhdr(void);                  // Write header of .db file
  int lookup_match(void **ppvFields, void *pvFoundRec); // Matches all fields?
  int _lookup(void **ppvFields, PALIST *ppl, void *pvFoundRec, NNUM *pnn,
              unsigned uiIdx, int fForward, int iLength,
              unsigned long *pulLength);
  int _block_in_nnflist(NNFLIST *pnnfHead, NNUM nn);
  NNFLIST * _get_free_records(void);

  // Inherited from BLKIOBUF:
  //
  //int enable_buffer(unsigned fEnable);
  //int flush_buffer(void);
  //virtual int close();
  //virtual int set_block_size(unsigned int uiBlkSize);
  //virtual int get(NNUM nn, char *pcDiskNode);
  //virtual int put(NNUM nn, char *pcDiskNode);
  //virtual NNUM high_node(void);
  //int open(char *szPath, int fNewFile);
  //int get_header(char *pcDiskNode, unsigned uiSize);
  //int put_header(char *pcDiskNode, unsigned uiSize);
  //int lock(NNUM nn);
  //int unlock(NNUM nn);


public:
  // Standard constructors and destructors:

  CPPEXPORT DBASE();
  virtual CPPEXPORT ~DBASE();

  // Standard interface functions:

  int CPPEXPORT open(char *szNam, FIELD *pf, unsigned uiNumFields,
                     unsigned new_file, unsigned uiOrder = INDEX_ORDER,
                     int fVarLength = FALSE);

  int CPPEXPORT insert(void *pvRecord, int iLength = 0);

  int CPPEXPORT lookup(void **ppvFields, PALIST *ppl, void *pvFoundRec,
                       unsigned uiIdx = 0, int iLength = 0,
                       unsigned long *pulLength = 0);

/*  int CPPEXPORT lookupr(void **ppvFields, PALIST *ppl, void *pvFoundRec, */
/*                        unsigned uiIdx = 0, int iLength = 0,             */
/*                        unsigned long *pulLength = 0);                   */

  int CPPEXPORT update(void *pvRecOld, void *pvRecNew, int iLength = 0);
  int CPPEXPORT remove(void **ppvFields);
  int CPPEXPORT close();
  int CPPEXPORT obtain_lock(void);
  int CPPEXPORT release_lock(void);
  unsigned long CPPEXPORT size(void);

// notused
//  SEQFIND CPPEXPORT DBASE::findseq_open(void *pvRec);
//  void * CPPEXPORT DBASE::findseq_next(SEQFIND sf);
//  int CPPEXPORT DBASE::findseq_close(SEQFIND sf);

  // Functions to obtain internal access to the database B-trees.  Not
  // for use by standard funtions:

  BTREE * CPPEXPORT get_btrees();
};

#endif // __DBASE_H_DEFINED

