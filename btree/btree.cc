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
static char rcs_id[]="$Id: btree.cc,v 1.1.1.1 2002/10/01 17:49:21 sdudley Exp $";
#pragma on(unreferenced)

//#define DEBUGBTREE
// Properties of a B-tree of order n:
// 1. Every node has no more than n immediate successors.
// 2. Every node, except for the root and the terminal nodes, have at
//    least n/2 immediate successors.
// 3. The root has at least two immediate successors (but can have none).
// 4. All terminal nodes are on the same level and contain no keys.
// 5. A nonterminal node with k immediate successors has k-1 keys.


// Structure of one B-tree node in the data file:
//
//   NODEHDR (containing the node number and number of keys)
//
//   child[0]
//    ...
//   child[uiOrder+1]
//
//   key[0]
//    ...
//   key[uiOrder]

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "btree.h"
#include "btreep.h"


//////////////////////////////////////////////////////////////////////////////
// B-tree file object
//////////////////////////////////////////////////////////////////////////////

// Create a new node in the B-tree.
//
// This function sets bteLastError if an I/O error occurred.
//
// Returns:
//   0 - if no block could be found
//   otherwise - the newly-allocated block number

NNUM BTREE::new_node()
{
  BTNODE bn(this, nnFree);

  // If there is no "free node" list in the file, simply add a new
  // node at the end of file

  if (!nnFree)
  {
    bn.nn=nnHighNode++;
  }
  else
  {
    bn.fFree=TRUE;

    if (!bn.get())
    {
      this->bteLastError = BTE_CANTREAD;
      return 0;
    }

    // Set the current free node to the next one in the linked list

    nnFree=bn.nnNextFree;

    // Set this node to "used"

    bn.fFree=FALSE;
  }


  // Store the node on disk.

  if (bn.put())
    return bn.nn;

  this->bteLastError = BTE_CANTWRITE;
  return 0;
}


// Delete all of the nodes in a B-tree subtree.
//
// Returns:
//  TRUE - if subtree completely destroyed.
//  FALSE - otherwise.  bteLastError is set appropriately.

int BTREE::destroy(NNUM nn)
{
  BTNODE bn(this, nn);

  // Get the node specified by this main subtree

  if (!bn.get())
  {
    this->bteLastError = BTE_CANTREAD;
    return FALSE;
  }

  for (int i=0; i <= bn.uiKeys; i++)
  {
    if (bn.child[i])
    {
      if (!destroy(bn.child[i]))
        return FALSE;

      bn.child[i]=0L;
    }
  }

  // Write this node back to disk, adding it to the "free list" in the
  // process.

  bn.fFree=TRUE;
  bn.nnNextFree=nnFree;
  nnFree=bn.nn;

  if (!bn.put())
  {
    this->bteLastError = BTE_CANTWRITE;
    return FALSE;
  }

  return TRUE;
}





int CPPEXPORT BTREE::print(FILE *fp)
{
  if (!fOpen)
    return FALSE;

  return _print(nnHead, 0, 0, fp);
}

int BTREE::_print(NNUM nn, NNUM nnParent, int level, FILE *fp)
{
  BTNODE bn(this, nn);
  int i;
  int nodes=0;

  if (bn.nn)
  {
    if (!bn.get())
    {
      fprintf(fp, "_print: can't get_node\n");
      return 0;
    }

    fprintf(fp, "Node %04lx - Level %d (%d keys) (parent=%04x)\n",
           bn.nn, level, bn.uiKeys, nnParent);

    for (i=0; i <= bn.uiKeys; i++)
    {
      if (i==bn.uiKeys)
        fprintf(fp,    "                          Child: %04lx\n", bn.child[i]);
      else fprintf(fp, "  Key: %s;  Child: %04lx\n", bn.key[i],
                   bn.child[i]);
    }

    nodes += bn.uiKeys;

    for (i=0; i <= bn.uiKeys; i++)
      nodes += _print(bn.child[i], bn.nn, level+1, fp);

    if (!level)
    {
      fprintf(fp, "Total nodes under level %d = %d\n", level, nodes);
      fflush(stdout);
    }
  }

  return nodes;
}


// Traverse a b-tree to display it in-order.

int CPPEXPORT BTREE::traverse(FILE *fp)
{
  if (!fOpen)
    return FALSE;

  try
  {
    return _traverse(nnHead, 0, fp);
  }
  catch (NoMem)
  {
    this->bteLastError = BTE_NOMEM;
    return FALSE;
  }
}


// Recursive B-tree traversal procedure

int BTREE::_traverse(NNUM nn, int level, FILE *fp)
{
  BTNODE bn(this, nn);
  int i;
  int nodes=0;

  if (bn.nn)
  {
    if (!bn.get())
    {
      fprintf(fp, "_print: can't get_node\n");
      return 0;
    }

    for (i=0; i <= bn.uiKeys; i++)
    {
      if (bn.child[i])
        _traverse(bn.child[i], level+1, fp);

      if (i==bn.uiKeys)
        fprintf(fp,    "                          Child: %04lx\n", bn.child[i]);
      else fprintf(fp, "  Key: %s;  Child: %04lx\n", bn.key[i],
                   bn.child[i]);
    }

    if (bn.child[i])
      _traverse(bn.child[i], level+1, fp);

    nodes += bn.uiKeys;

    if (!level)
    {
      fprintf(fp, "Total nodes under level %d = %d\n", level, nodes);
      fflush(stdout);
    }
  }

  return nodes;
}


// Recursive b-tree validation procedure

int BTREE::_validate(NNUM nn, int level, void *maxkey)
{
  BTNODE bn(this, nn);
  int i, rc=TRUE;

  if (nn)
  {
    if (!bn.get())
    {
      printf("_validate: can't get_node for %d\n", bn.nn);
      return FALSE;
    }

    if (bn.uiKeys < uiOrder/2-1 && bn.nn != nnHead)
      printf("Error!  Node %d has too few keys! (has=%d, required=%d)\n",
             bn.nn, bn.uiKeys, uiOrder/2-1);


    /* Make sure that all of the keys are okay */

    for (i=0; i < bn.uiKeys; i++)
    {
      if (maxkey && kf_ins(bn.key[i], maxkey) > 0)
      {
        printf("Error!  Key #%ld in node %#04lx (\"%s\") is larger than "
               "key \"%s\"!\n", (long)i, bn.nn, bn.key[i], maxkey);

        rc=FALSE;
      }
    }

    for (i=0; i <= bn.uiKeys; i++)
      if (!_validate(bn.child[i], level+1, i==bn.uiKeys ? maxkey : bn.key[i]))
        rc=FALSE;
  }

  return rc;
}



// Validate the current b-tree to ensure that it is structurally sound

int CPPEXPORT BTREE::validate(void)
{
  if (!fOpen)
    return FALSE;

  try
  {
    return _validate(nnHead, 0, NULL);
  }
  catch (NoMem)
  {
    this->bteLastError = BTE_NOMEM;
    return FALSE;
  }
}


// Return the total number of keys in this B-tree

unsigned long CPPEXPORT BTREE::size(void)
{
  return cKeys;
}


BTERROR CPPEXPORT BTREE::error(void)
{
  return fOpen ? this->bteLastError : BTE_NOTOPEN;
}

