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

/* $Id: btree.h,v 1.1.1.1 2002/10/01 17:49:22 sdudley Exp $ */

#ifndef __BTREE_H_DEFINED
#define __BTREE_H_DEFINED

#include <stdio.h>
#include <stdlib.h>
#include "btype.h"
#include "palist.h"
#include "btnode.h"
#include "blkiobuf.h"

// B-tree class.  Derived from the buffered block I/O class.

class BTREE : private BLKIOBUF
{
  // Private data from the file header

  BTERROR bteLastError;         // Most recent error code
  NNUM nnHead;                  // Node of tree's head
  NNUM nnFree;                  // Linked list of free records
  unsigned uiKeySize;           // Size of one key
  unsigned uiOrder;             // Order of this tree
  unsigned long cKeys;          // Total number of keys in tree

  // Information that is generated and is only needed at runtime

  unsigned fOpen;               // Is a b-tree file currently open?
  unsigned uiNodeSize;          // Size of one node, as stored in the file
  NNUM nnHighNode;              // Highest EOF node assnd by new_node

  // Functions used to compare key values.
  //
  // kf_ins is used for functions which wish to insert essentially
  // "duplicate" key values into a B-tree.  The values must have
  // some sort of unique identifier so that the key can be kept in
  // order, so kf_ins is used to compare based on the main key value,
  // and following that, on the node number.  This is typically
  // used for inserting keys.
  //
  // kf_srch simply compares on the main key value, without checking
  // the unique identifier.  This is typically used when searching
  // for an existing key.

  keycomp_t kf_ins;
  keycomp_t kf_srch;

  // Private helper functions

  int write_file_header();
  int insert_at(BTNODE *pbnNode, PNODE *pnParents, int pos, void *key, NNUM nnChild);
  int _remove_from_root(BTNODE *pbnNode, PNODE *pnNodeParents);
  int _remove_shift_keys(BTNODE *pbnNode, BTNODE *pbnSteal, BTNODE *pbnParent, int steal_idx, int delete_right);
  int _remove_merge_3(BTNODE *pbnNode, BTNODE *pbnParent, BTNODE *pbnSteal, PNODE *pnNodeParents, int steal_idx, int parent_idx);
  int _remove_replace_successor(BTNODE *pbnNode, PNODE *pnNodeParents, int replace_idx);
  int _remove_low_keys(BTNODE *pbnNode, PNODE *pnNodeParents);
  int _remove(BTNODE *pbnNode, PNODE *pnParents, int idx);
  unsigned _move_key(BTNODE *pbnTo, BTNODE *pbnFrom, BTNODE *pbnToParent, int parent_idx, int delete_right);
  int destroy(NNUM nn);
  int _validate(NNUM nn, int level, void *maxkey);
  NNUM new_node(void);

  // Internal, recursed versions of print() and traverse()

  int _print(NNUM nn, NNUM nnParent, int level, FILE *fp);
  int _traverse(NNUM nn, int level, FILE *fp);

  // Friends which need access to our B-tree internals.  The btnode
  // is created from a B-tree structure and can be used to write
  // to our B-tree file with get and put.

  friend BTNODE::BTNODE(BTREE *pbt, NNUM n);
  friend int BTNODE::get();
  friend int BTNODE::put();

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
  // Diagnostic, public-callable functions

  int CPPEXPORT print(FILE *fp);
  int CPPEXPORT traverse(FILE *fp);
  int CPPEXPORT validate();

  // Standard entrypoints:

  CPPEXPORT BTREE();
  virtual CPPEXPORT ~BTREE();
  int CPPEXPORT open(char *pszFile, keycomp_t kf_ins, keycomp_t kf_srch, unsigned uiKeySz, unsigned new_file, unsigned uiOrd = 8, unsigned fBuffer = TRUE);
  int CPPEXPORT close();
  void * CPPEXPORT lookup(void *key, PALIST *pl);
//  void * CPPEXPORT lookupr(void *key, PALIST *pl);
  int CPPEXPORT insert(void *key, unsigned flags);
  int CPPEXPORT remove(void *key);
  int CPPEXPORT removep(PALIST *ppl);
  unsigned long CPPEXPORT size();
  BTERROR CPPEXPORT error();
};

#endif // __BTREE_H_DEFINED

