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

/*# name=Easy-to-use skip list API
*/

#include <stdlib.h>
#include "prog.h"
#include "skiplist.h"
#include "skiplisp.h"


/* Create a new skip list */

SLIST * _fast SkipCreateList(word maxlevel, word partition, int (_stdc *comp)(void *, void *))
{
  SLIST *sl;
  
  if ((sl=malloc(sizeof(SLIST)))==NULL)
    return (SLIST *)NULL;
  
  /* Use default parameters if nothing has been specified */
  
  sl->maxlevel=(maxlevel && maxlevel <= MAXLEVEL) ? maxlevel : MAXLEVEL;
  sl->partition=partition ? partition : PARTITION;
  sl->comp=comp;

  /* Create the first header node */
  
  if ((sl->first=_SkipCreateNode(sl->maxlevel, NULL))==NULL)
  {
    free(sl);
    sl=NULL;
  }

  /* Make the skip list blank */
  
  _SkipBlank(sl);
  return sl;
}





/* Add a node to the skip list.  If the node already exists, the            *
 * 'exists' word will be set to TRUE, and the function will return the      *
 * address of the existing data for that node.  If the node does NOT        *
 * exist, the function will create a new node, set 'exists' to FALSE,       *
 * and return the address passed to it in 'data'.  On error, SkipAdd()      *
 * returns NULL.                                                            */

void * _fast SkipAddNode(SLIST *sl, void *data, word *exists)
{
  SLNODE *this;
  SLNODE *ptrs[MAXLEVEL];
  word level, n;
  
  if (exists)
    *exists=FALSE;

  /* Find the node just before the node we want */

  this=_SkipFindNode(sl, data, ptrs);


  /* Skip along to the actual node in question  */
  
  if (this)
    this=this->next[0];
  

  /* If we found the node in the list */
  
  if (this && (*sl->comp)(data, this->data)==0)
  {
    if (exists)
      *exists=TRUE;

    return this->data;
  }

  /* Get a new node, but make sure that it isn't more than one above the    *
   * current highest level.                                                 */
     
  if ((level=_SkipRandomLevel(sl)) > sl->highlevel)
  {
    /* Increment the level counter */
    
    level=++sl->highlevel;
    
    /* Make the FIRST pointer point to the list header */
    
    ptrs[level]=sl->first;
  }

  /* Allocate enough memory to hold the new node */
  
  if ((this=_SkipCreateNode(level+1, data))==NULL)
    return NULL;
  
  /* Now link this node into the list */
  
  for (n=0; n <= level; n++)
  {
    this->next[n]=ptrs[n]->next[n];
    ptrs[n]->next[n]=this;
  }
  
  sl->nodes++;
  
  return this->data;
}





/* Search the skip list for the data object in queston, and return a        *
 * pointer to it if found, otherwise return NULL.                           */

void * _fast SkipSearchList(SLIST *sl, void *data)
{
  SLNODE *this;

  /* Find the node just before the node we want */
  
  this=_SkipFindNode(sl, data, NULL);

  if (this)
    this=this->next[0];
  
  /* Not found */
  
  if (this==NULL || (*sl->comp)(data, this->data) != 0)
    return NULL;
  
  return this->data;
}




/* Delete a node from the skip list */

sword _fast SkipDeleteNode(SLIST *sl, void *data)
{
  SLNODE *this;
  SLNODE *ptrs[MAXLEVEL];
  word n;

  /* Find the node just before the node we want */
  
  this=_SkipFindNode(sl, data, ptrs);
  

  /* Skip along to the actual node in question  */
  
  if (this)
    this=this->next[0];
  
  
  /* Now see if they're equal.  If not, the specified node doesn't exist. */
  
  if (this==NULL || (*sl->comp)(data, this->data) != 0)
    return SKIP_ENOENT;
  

  /* Skip all of the pointers over this node */

  for (n=0; n <= sl->highlevel && ptrs[n]->next[n]==this; n++)
    ptrs[n]->next[n]=this->next[n];


  /* If we deleted the last node for this level, decrement the level        *
   * counter.                                                               */

  while (sl->highlevel && sl->first->next[sl->highlevel]==NULL)
    sl->highlevel--;
  
  sl->nodes--;
  
  return SKIP_OK;
}



/* Walk the skip list and deallocate everything */

sword _fast SkipDestroyList(SLIST *sl)
{
  SLNODE *sn, *next=NULL;

  /* Visit each node */
  
  for (sn=sl->first->next[0]; sn; sn=next)
  {
    next=sn->next[0];
    _SkipFreeNode(sn);
  }
  
  free(sl);
  
  return SKIP_OK;
}


/* Used to scan all of the nodes in a skip list - call First for the        *
 * first call, and call Next to get each remaining node.                    */

void * _fast SkipFirst(SLIST *sl)
{
  sl->last=sl->first->next[0];
  return sl->last ? sl->last->data : NULL;
}



/* Find the next entry in the list */

void * _fast SkipNext(SLIST *sl)
{
  if (!sl->last)
    return NULL;
  
  sl->last=sl->last->next[0];

  return (sl->last ? sl->last->data : NULL);
}








static SLNODE * near _fast _SkipFindNode(SLIST *sl, void *data, SLNODE *ptrs[])
{
  SLNODE *this;
  word lvl;

  /* Skip along the list, descending levels as necessary to find the        *
   * node we're looking for.                                                */
  
  for (this=sl->first, lvl=sl->highlevel+1; lvl;)
  {
    lvl--;
    
    while (this->next[lvl] && (*sl->comp)(this->next[lvl]->data, data) < 0)
      this=this->next[lvl];
    
    /* Save a copy of the pointer at this stage, so that we can update them *
     * when adding or deleting a node.                                      */

    if (ptrs)
      ptrs[lvl]=this;
  }
  
  return this;
}





/* Create a new node with the specified number of levels, and attaching     *
 * the specified data to it.                                                */
   
static SLNODE * near _fast _SkipCreateNode(word levels, void *data)
{
  SLNODE *sn;
  
  /* Allocate memory for the node itself */
  
  if ((sn=malloc(sizeof(SLNODE)))==NULL)
    return NULL;
  
  sn->data=data;
  
  /* Now allocate memory for the next pointers */

  if ((sn->next=malloc(sizeof(SLNODE *) * levels))==NULL)
  {
    free(sn);
    sn=NULL;
  }
  
  return sn;
}



/* Make a skip list blank */

static void near _fast _SkipBlank(SLIST *sl)
{
  word n;
  
  /* Now set all of the next ponetrs to NULL */

  for (n=0; n < sl->maxlevel; n++)
    sl->first->next[n]=NULL;

  sl->highlevel=0;
}


static void near _fast _SkipFreeNode(SLNODE *sn)
{
  /* Free the memory for the array of next[] pointers */

  free(sn->next);
    

  /* Free the node itself */
    
  free(sn);
}



static word near _fast _SkipRandomLevel(SLIST *sl)
{
  word level=0;

  while ((word)(rand() % sl->maxlevel) < sl->partition && 
         level < sl->maxlevel)
  {
    level++;
  }
  
  return level;
}


#ifdef NEVER
void SkipCountLevel(SLIST *sl)
{
  struct _cfgarea *ar;
  SLNODE *sn;
  int i, cnt;
  static int asdf=0;
  
  for (i=0; i < sl->highlevel; i++)
  {
    for (cnt=0, sn=sl->first->next[i]; sn; sn=sn->next[i])
      cnt++;
    
    printf("Level %d: %d\n", i, cnt);
  }
}
#endif


