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
static char rcs_id[]="$Id: bt_lookr.cc,v 1.1.1.1 2002/10/01 17:49:24 sdudley Exp $";
#pragma on(unreferenced)

#include "btree.h"

#define INDEX_END  -3


// Browse the B-tree starting with the specified key.  If key==NULL,
// then return the next available record.  pl is used to store the
// parents of the current node so that the tree can be traversed
// properly

void * CPPEXPORT BTREE::lookupr(void *key, PALIST *ppl)
{
  BTNODE bnNode(this);
  PALIST plParent;
  int i;
  int rc;

  // Make sure that the file is open

  if (!fOpen || !nnHead)
  {
    this->bteLastError = BTE_NOTOPEN;
    return FALSE;
  }

  this->bteLastError = BTE_NONE;

  // Check for memory exception

  try
  {
    // If the caller didn't provide a search list, start at the
    // beginning.

    if (!ppl)
      ppl=&plParent;

    // Read from the beginning, or the last node number, whichever
    // came first.

    if (ppl->nodes() && ppl->nodes()->next)
      bnNode.nn=ppl->current_node;
    else
    {
      bnNode.nn=nnHead;

      if (!ppl->fSearched)
        ppl->current_idx=INDEX_END;
    }

    ppl->fSearched=TRUE;

    // Repeat while we have a valid node number to search for

    while (bnNode.nn && bnNode.get())
    {
      // If the current index is INDEX_END, it means to start searching from
      // the right hand side.

      if (ppl->current_idx==INDEX_END)
          ppl->current_idx=bnNode.uiKeys-1;

      // Loop through all nodes to try to find this key

      for (i=ppl->current_idx; i >= 0; i--)
        if (!key || (rc=(*kf_srch)(bnNode.key[i], key))==0)
        {
          // If no child was found, just return this key and set next
          // search to start just after us.

          if (!bnNode.child[i+1])
          {
            ppl->current_node=bnNode.nn;
            ppl->current_idx=i-1;
            return bnNode.key[i];
          }

          // We found the key, but it isn't necessarily the leftmost
          // key to be matched, since we have children.  Consequently,
          // we must jump down to the successor to see if we are
          // the leftmost.

          break;
        }
        else if (rc < 0)
          break;

      // If we have a child, descend into it

      if (i >= -1 && bnNode.child[i+1])
      {
        // If it has a child, then we can't be at the bottom of
        // the tree.  We want to insert all keys into the tree at
        // the bottom so that changes can be propagated upward).

        ppl->add(bnNode.nn, i);
        bnNode.nn=bnNode.child[i+1];
        ppl->current_idx=INDEX_END;
        ppl->current_node=bnNode.nn;
      }
      else
      {
        // Go up the tree one level

        bnNode.nn=ppl->pop(&ppl->current_idx);
        ppl->current_node=bnNode.nn;

        // By default, start searching just after we left off...

        i=ppl->current_idx--;

        if (bnNode.nn && bnNode.get())
        {
          // ... unless the key node between two trees happens
          // to match our key.

          if (i >= 0 && (!key || (*kf_srch)(bnNode.key[i], key)==0))
          {
            return bnNode.key[i];
          }
        }
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

