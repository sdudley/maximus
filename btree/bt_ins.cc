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

/* $Id: bt_ins.cc,v 1.2 2004/01/22 09:02:29 wmcbrine Exp $ */

#include <string.h>
#include <assert.h>
#include "btree.h"

// Insert a key in a specific particular position of the node pbn.
//
// Returns the position number into which the key was inserted,
// or -1 if error.

int BTREE::insert_at(BTNODE *pbnNode, PNODE *pnParents, int pos, void *key,
                     NNUM nnChild)
{
  BTNODE bnChild(this, nnChild);

  // Move the child pointers up to make room for this one

  memmove(pbnNode->child+pos+1, pbnNode->child+pos, (uiOrder-pos)*sizeof(NNUM));

  // Do the same with the key data, too

  if (uiOrder-pos-1)
    memmove(pbnNode->key[pos+1], pbnNode->key[pos], (uiOrder-pos-1) * uiKeySize);

  pbnNode->child[pos+1]=nnChild;
  memmove(pbnNode->key[pos], key, uiKeySize);

  if (pbnNode->uiKeys+1 < uiOrder)
  {
    // Increment the number of keys in this node

    pbnNode->uiKeys++;

    // Write this back to disk

    if (!pbnNode->put())
    {
      this->bteLastError = BTE_CANTWRITE;
      return -1;
    }

    // Return the position into which this node was inserted

    return pos;
  }

  // Else this node needs to be split to make room for the new key

  BTNODE bn(this, new_node());
  char *newdata=new char[uiKeySize];
  int mid, i;

  if (!newdata)
    throw NoMem(0);

  // Find the middle value and retrieve its key value

  mid=(uiOrder-1)/2;
  memmove(newdata, pbnNode->key[mid], uiKeySize);

  // mid=1
  //
  //        [0]    [1]  [2]
  //        25     30    50
  //     [0]   [1]    [2]   [3]
  //
  //
  //          parent
  //
  //            30
  //         [0] |  [1]
  //       25    |    50
  //    [0]  [1] |  [2]  [3]
  //        bn   |    bnNode

  // Move the right half of the node to a new node

  memmove(bn.key[0], pbnNode->key[mid+1], uiKeySize * (uiOrder - mid - 1));
  memmove(bn.child, pbnNode->child+mid+1, sizeof(NNUM)*(uiOrder - mid));

  // Clear out the right half of the old node

  memset(pbnNode->key[mid], 0, uiKeySize * (uiOrder - mid));
  memset(pbnNode->child+mid+1, 0, sizeof(NNUM) * (uiOrder - mid));

  // Adjust the count of keys in these two new nodes

  pbnNode->uiKeys -= uiOrder-mid-1;
  bn.uiKeys = uiOrder-mid-1;


  // Update the two nodes that we used to shift keys

  if (!pbnNode->put() ||
      !bn.put())
  {
    this->bteLastError = BTE_CANTWRITE;
    delete [] newdata;
    return -1;
  }


  // If no parent, this is the root, so we need to make a new root
  // node instead.

  if (!pnParents->nn)
  {
    BTNODE bnRoot(this, new_node());

    // Initialize the root and make it point to us

    nnHead=bnRoot.nn;
    bnRoot.child[0]=pbnNode->nn;

    // Write the root back to disk

    if (!bnRoot.put())
    {
      this->bteLastError = BTE_CANTWRITE;
      delete [] newdata;
      return -1;
    }

    // Create a new linked list to hold the parent node, since this
    // current list must be empty

    pnParents->next=NULL;
    pnParents->nn=bnRoot.nn;
  }

  // Get parent of our node to store the wayward key

  BTNODE bnParent(this, pnParents->nn);

  if (!bnParent.get())
  {
    this->bteLastError = BTE_CANTREAD;
    delete [] newdata;
    return -1;
  }

  // Figure out where to attach it in the parent

  for (i=0; i < bnParent.uiKeys; i++)
    if (bnParent.child[i]==pbnNode->nn)
      break;

  // Make sure that we got the key okay

  assert(bnParent.child[i]==pbnNode->nn);

  // Insert this key in our parent at the specified position

  if (insert_at(&bnParent, pnParents->next, i, newdata, bn.nn)==-1)
    pos = -1;

  delete [] newdata;

  return pos;
}



// Insert the key 'key' into the B-tree in an appropriate position
//
// Returns one of the IF_xxx constants from btree.h

int CPPEXPORT BTREE::insert(void *key, unsigned flags)
{
  PALIST plParent;
  BTNODE bnNode(this, nnHead);
  unsigned i;
  int rc=IF_NORM;   // default to a normal insertion

  if (!fOpen)
    return FALSE;

  try
  {
    // Create a new tree, if necessary

    if (!nnHead)
      bnNode.nn=nnHead=new_node();

    // Repeat while we have a valid node number to insert into

    while (bnNode.nn)
    {
      // Read this node off disk

      if (!bnNode.get())
      {
        this->bteLastError = BTE_CANTREAD;
        return 0;
      }

      // Scan the current node to find out where to place the current
      // key.

      for (i=0; i < bnNode.uiKeys; i++)
      {
        if ((rc=(*kf_ins)(bnNode.key[i], key)) > 0)
          break;
        else if (rc==0)   // Got a match of the same key
        {
          if (flags & IF_REPLACE)
          {
            // Copy the new key in its place

            memmove(bnNode.key[i], key, uiKeySize);

            if (!bnNode.put())
            {
              this->bteLastError = BTE_CANTWRITE;
              return 0;
            }

            return IF_REPLACE;
          }

          // If we aren't allowed to write a dupe key, this is an error!

          if ((flags & IF_DUPE)==0)
          {
            this->bteLastError = BTE_DUPEKEY;
            return 0;
          }

          rc=IF_DUPE;
          break;
        }
      }

      // If it has a child, then we can't be at the bottom of
      // the tree.  We want to insert all keys into the tree at
      // the bottom so that changes can be propagated upward).

      if (bnNode.child[i])
      {
        plParent.add(bnNode.nn, 0);
        bnNode.nn=bnNode.child[i];
      }
      else
      {
        // If we encountered an error inserting the node, then
        // we should return an error too.

        if (insert_at(&bnNode, plParent.nodes(), i, key, (NNUM)0)==-1)
          rc = 0;

        cKeys++;
        return rc;
      }
    }
  }
  catch (NoMem)
  {
    this->bteLastError = BTE_NOMEM;
    return FALSE;
  }

  return FALSE;
}



