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

/* $Id: bt_rem.cc,v 1.3 2004/01/22 09:02:29 wmcbrine Exp $ */

#include <string.h>
#include <assert.h>
#include "btree.h"


// Move one key from node pbnFrom to node pbnTo, shifting that key through
// the key in position parent_idx of the parent node pbnToParent.
//
// Returns number of keys in nnTo after the move.

unsigned BTREE::_move_key(BTNODE *pbnTo, BTNODE *pbnFrom, BTNODE *pbnToParent,
                          int parent_idx, int delete_right)
{
  int idxTo=delete_right ? pbnTo->uiKeys : 0;
  int idxFrom=delete_right ? 0 : pbnFrom->uiKeys-1;

  // Make sure that there are enough keys in the source node

  assert(pbnFrom->uiKeys > 0);

  // If we are inserting at the beginning of a node's keys, shift the
  // existing keys over to make room for this one.

  assert(pbnTo->uiKeys+1 < uiOrder);

  if (!delete_right)
  {
    memmove(pbnTo->key[1], pbnTo->key[0], uiKeySize * pbnTo->uiKeys);
    memset(pbnTo->key[0], 0, uiKeySize);

    memmove(pbnTo->child+1, pbnTo->child, sizeof(NNUM) * (pbnTo->uiKeys+1));
    memset(pbnTo->child, 0, sizeof(NNUM));
  }

  // Move the data from the original parent key into this node

  memmove(pbnTo->key[idxTo],
          pbnToParent->key[parent_idx],
          uiKeySize);

  // Move the data on the edge of the other key into the parent node

  memmove(pbnToParent->key[parent_idx],
          pbnFrom->key[idxFrom],
          uiKeySize);

  // Move the child pointer from the right-hand node to the left-hand node

  memmove(pbnTo->child + idxTo + !!delete_right,
          pbnFrom->child + idxFrom + !delete_right,
          sizeof(NNUM));

  // Now shift the data in the original node appropriately.  If we removed
  // the key on the right hand side of the tree, we don't need to do
  // the shift, since the key is already where we want it to be.

  if (delete_right)
  {
    // Shift all of the keys back by one element

    memmove(pbnFrom->key[0],
            pbnFrom->key[1],
            uiKeySize * (pbnFrom->uiKeys-1));

    // Shift all of the children back by one element too.

    memmove(pbnFrom->child, pbnFrom->child+1, sizeof(NNUM)*pbnFrom->uiKeys);
  }
  else
  {
    // Make sure that we are blanking out the right spot

    assert(idxFrom==pbnFrom->uiKeys-1);
  }

  // Set the last key and child elements to zero

  memset(pbnFrom->key[pbnFrom->uiKeys-1],
         0,
         uiKeySize);

  memset(pbnFrom->child + pbnFrom->uiKeys, 0, sizeof(NNUM));

  // Decrement the number of keys being used by the "from" node

  pbnFrom->uiKeys--;
  pbnTo->uiKeys++;

  // Return the number of keys in the final node

  return pbnTo->uiKeys;
}


// A key was just removed from this, the root.  Adjust accordingly,
// and write pbnNode to disk when we are done.
//
// Returns:
//  TRUE - if removed successfully.
//  FALSE - otherwise.  bteLastError is set accordingly

int BTREE::_remove_from_root(BTNODE *pbnNode, PNODE *pnNodeParents)
{
  // If we have at least one key, everything is ok.  But if
  // we have no more keys, delete the root node and make
  // the child into the root!

  if (pbnNode->uiKeys)
  {
    if (!pbnNode->put())
    {
      this->bteLastError = BTE_CANTWRITE;
      return FALSE;
    }

    return TRUE;
  }

  // The root has no keys - save a pointer to the node below us

  NNUM nnNewHead=pbnNode->child[0];

  // Set the child pointer to zero so that the nodes below
  // us do not get killed by this destroy call.

  pbnNode->child[0]=0;

  if (!pbnNode->put())
  {
    this->bteLastError = BTE_CANTWRITE;
    return FALSE;
  }

  // Try to destroy the subtree.  destroy() will set the error code
  // properly if it fails.

  if (!destroy(pbnNode->nn))
    return FALSE;

  nnHead=nnNewHead;

  // If there is anything left in this tree, set our 'parents'
  // chain so that it shows us as being at the top of the
  // tree, just in case something else tries to access
  // this linked list after we return.

  if (nnHead)
  {
    pnNodeParents->nn=0L;
    delete(pnNodeParents->next);
    pnNodeParents->next=NULL;
  }

  return TRUE;
}



// Shift keys from pbnSteal to pbnNode until we have a properly-balanced
// tree.  Write pbnNode to disk when done.
//
// Returns TRUE if successful.  Otherwise, sets bteLastError accordingly

int BTREE::_remove_shift_keys(BTNODE *pbnNode, BTNODE *pbnSteal,
                              BTNODE *pbnParent, int steal_idx,
                              int delete_right)
{
  int total_keys=pbnNode->uiKeys + pbnSteal->uiKeys;

  while (pbnNode->uiKeys < total_keys/2)
    _move_key(pbnNode, pbnSteal, pbnParent, steal_idx, delete_right);

  // Write our changes back to disk

  if (!pbnParent->put() ||
      !pbnNode->put() ||
      !pbnSteal->put())
  {
    this->bteLastError = BTE_CANTWRITE;
    return FALSE;
  }

#ifndef NDEBUG

  // Property of B-tree

  assert(pbnNode->uiKeys >= uiOrder/2-1);
  assert(pbnSteal->uiKeys >= uiOrder/2-1);
  assert(pbnParent->nn==nnHead || pbnParent->uiKeys >= uiOrder/2-1);

#endif

  return TRUE;
}



// Merge us, sibling and parent all into one.  Write pbnNode to disk
// when done.

int BTREE::_remove_merge_3(BTNODE *pbnNode, BTNODE *pbnParent,
                           BTNODE *pbnSteal, PNODE *pnNodeParents,
                           int steal_idx, int parent_idx)
{
  // If that sibling has uiOrder/2-1 keys, we can't steal any more
  // keys from it, but:
  //
  // total_keys = his_keys + our_keys + parent
  //            = uiOrder/2-1 + uiOrder/2-2 + 1
  //            = uiOrder-2
  //
  // which is more than uiOrder/2-1, so we can merge our keys,
  // his keys, and the parent all into one node, and then
  // delete the parent.

  BTNODE *pbnFrom;
  BTNODE *pbnTo;

  if (steal_idx==parent_idx)
  {
    pbnFrom=pbnSteal;
    pbnTo=pbnNode;
  }
  else
  {
    pbnFrom=pbnNode;
    pbnTo=pbnSteal;
  }


  // Add the parent to the left node

  memmove(pbnTo->key[pbnTo->uiKeys++], pbnParent->key[steal_idx], uiKeySize);

  // Add all of the keys from the right node.

  memmove(pbnTo->key[pbnTo->uiKeys], pbnFrom->key[0], pbnFrom->uiKeys * uiKeySize);

  // Copy the child pointers as well

  memmove(pbnTo->child + pbnTo->uiKeys,
          pbnFrom->child,
          (pbnFrom->uiKeys+1) * sizeof(NNUM));

  // Increment the number of keys in the left node

  pbnTo->uiKeys += pbnFrom->uiKeys;


  // Delete all of the child pointers from pbFrom so that
  // destroy() does not delete them.

  for (int i=0; i <= pbnFrom->uiKeys; i++)
    pbnFrom->child[i]=0;

  // Save these changes to disk

  if (!pbnTo->put() ||
      !pbnFrom->put())
  {
    this->bteLastError = BTE_CANTWRITE;
    return FALSE;
  }

  // Now kill the node that we just deleted

  if (!destroy(pbnFrom->nn))
    return FALSE;

  // We just deleted the node tree to the right, so set it to NULL

  pbnParent->child[steal_idx+1]=0;
  pbnParent->put();

  // Now remove the data element in question from the parent.

  return _remove(pbnParent, pnNodeParents->next, steal_idx);
}



// Replace the current node with its successor, then remove the
// successor.

int BTREE::_remove_replace_successor(BTNODE *pbnNode, PNODE *pnNodeParents,
                                     int replace_idx)
{
  BTNODE bn(this, pbnNode->child[replace_idx+1]);
  PNODE *pnStart, *pnOrig, *pn, *pnNext;

  if (!bn.get())
  {
    this->bteLastError = BTE_CANTREAD;
    return FALSE;
  }

  // Create a dummy linked list of parents, just for this subtree

  pnStart=pnOrig=pnNodeParents;

  // Add bnNode to the list of parents

  pn=new PNODE;

  if (!pn)
    throw NoMem(0);

  pn->nn=pbnNode->nn;;
  pn->next=pnStart;
  pnStart=pn;

  assert(bn.nn); // property of B-tree

  // Find successor of this key

  while (bn.child[0])
  {
    pn=new PNODE;

    if (!pn)
      throw NoMem(0);

    pn->nn=bn.nn;
    pn->next=pnStart;
    pnStart=pn;

    bn.nn=bn.child[0];

    if (!bn.get())
    {
      this->bteLastError = BTE_CANTREAD;
      return FALSE;
    }
  }

  // Copy it into this space

  memmove(pbnNode->key[replace_idx], bn.key[0], uiKeySize);

  // Write this stuff back to disk

  if (!bn.put() ||
      !pbnNode->put())
  {
    this->bteLastError = BTE_CANTWRITE;
    return FALSE;
  }

  // Now delete the successor

  int rc=_remove(&bn, pnStart, 0);

  // Now free only those entries that we added to the list of
  // parents.

  for (pn=pnStart; pn && pn != pnOrig; pnNext=pn->next, delete pn, pn=pnNext)
    ;

  return rc;
}



// We have too few keys in this node.  Remedy the situation
// by moving or merging, and then make sure that pbnNode
// is written to disk.

int BTREE::_remove_low_keys(BTNODE *pbnNode, PNODE *pnNodeParents)
{
  // This means that our node has uiOrder/2-2 keys.  We must have
  // either a right or a left sibling (property of B-tree, since
  // only the root is allowed to have no siblings).

  // First, find our index in the parent's child[] array

  BTNODE bnParent(this, pnNodeParents->nn);

  if (!bnParent.get())
  {
    this->bteLastError = BTE_CANTREAD;
    return FALSE;
  }

  int parent_idx;

  for (parent_idx=0; parent_idx <= bnParent.uiKeys; parent_idx++)
    if (bnParent.child[parent_idx]==pbnNode->nn)
      break;

  // Make sure that we got our own node number

  assert(bnParent.child[parent_idx]==pbnNode->nn);

  BTNODE bnSteal(this);
  int steal_idx;

  // Find a sibling from which we can steal keys

  if (parent_idx==0)
  {
    bnSteal.nn=bnParent.child[parent_idx+1];
    steal_idx=parent_idx;
  }
  else
  {
    bnSteal.nn=bnParent.child[parent_idx-1];
    steal_idx=parent_idx-1;
  }


  // Now get the node number of the sibling from which we
  // are going to steal a key.

  if (!bnSteal.get())
  {
    this->bteLastError = BTE_CANTREAD;
    return FALSE;
  }


  // If that sibling has more than uiOrder/2-1 keys, we can simply
  // move keys from his node to ours (shifting through the parent),
  // until we have a balanced number of nodes in each.

  if (bnSteal.uiKeys > uiOrder/2-1)
    return _remove_shift_keys(pbnNode, &bnSteal, &bnParent,
                              steal_idx, parent_idx==0);

  // Otherwise, we must merge the sibling, the parent, and us all
  // into one node.

  return _remove_merge_3(pbnNode, &bnParent, &bnSteal, pnNodeParents,
                         steal_idx, parent_idx);
}




// Remove a key from nnNode, at the specified index

int BTREE::_remove(BTNODE *pbnNode, PNODE *pnNodeParents, int idx)
{
  // If we are removing from a non-leaf, just replace with successor

  if (pbnNode->child[idx+1])
    return _remove_replace_successor(pbnNode, pnNodeParents, idx);

  // Since we are working on a leaf node, just shift everything over!

  memmove(pbnNode->key[idx],
          pbnNode->key[idx+1],
          (pbnNode->uiKeys-idx-1) * uiKeySize);

  memmove(pbnNode->child+idx+1,
          pbnNode->child+idx+2,
          (pbnNode->uiKeys-idx-1) * sizeof(NNUM));

  // Decrement number of keys

  pbnNode->uiKeys--;

  memset(pbnNode->key[pbnNode->uiKeys], 0, uiKeySize);
  memset(pbnNode->child + pbnNode->uiKeys+1, 0, sizeof(NNUM));

  // See if this number of keys would unbalance the tree.  Note that
  // this check does not apply to the root.

  if (pbnNode->uiKeys >= (uiOrder/2)-1)
  {
    if (!pbnNode->put())
    {
      this->bteLastError = BTE_CANTWRITE;
      return FALSE;
    }

    return TRUE;
  }

  // It's OK for the root not to have the "right" number of keys.
  // However, we may need to make some adjustments if we are
  // the root.

  if (pbnNode->nn==nnHead)
    return _remove_from_root(pbnNode, pnNodeParents);

  // Handle a standard node which has too few keys

  return _remove_low_keys(pbnNode, pnNodeParents);
}


// Remove the last node that was returned by a lookup command

int CPPEXPORT BTREE::removep(PALIST *ppl)
{
  BTNODE bnNode(this, ppl->current_node);

  this->bteLastError = BTE_NONE;

  // Try to read in the specified node

  if (!bnNode.get())
  {
    this->bteLastError = BTE_CANTREAD;
    return FALSE;
  }

  // Now remove the key from that node

  _remove(&bnNode, ppl->nodes(), ppl->current_idx-1);
  cKeys--;
  return TRUE;
}


// Remove a specified key

int CPPEXPORT BTREE::remove(void *key)
{
  PALIST plParent;
  BTNODE bnNode(this, nnHead);
  unsigned i;

  if (!fOpen)
    return FALSE;

  if (!nnHead)
  {
    this->bteLastError = BTE_NOTOPEN;
    return 0;
  }

  this->bteLastError = BTE_NONE;

  try
  {
    while (bnNode.nn)
    {
      if (!bnNode.get())
      {
        this->bteLastError = BTE_CANTREAD;
        return 0;
      }

      for (i=0; i < bnNode.uiKeys; i++)
      {
        int rc=(*kf_srch)(key, bnNode.key[i]);

        if (rc==0)
        {
          _remove(&bnNode, plParent.nodes(), i);
          cKeys--;
          return 1;
        }
        else if (rc < 0)
          break;
      }

      plParent.add(bnNode.nn, 0);
      bnNode.nn=bnNode.child[i];
    }
  }
  catch (NoMem)
  {
    // No memory, so return FALSE

    this->bteLastError = BTE_NOMEM;
    return 0;
  }

  return 0;
}


