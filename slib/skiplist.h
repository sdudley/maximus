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

#ifndef __SKIPLIST_H_DEFINED
#define __SKIPLIST_H_DEFINED

#define SKIP_OK      0    /* No error */
#define SKIP_ENOMEM -1    /* Error codes returned by Skip...() functions */
#define SKIP_EEXIST -2    /* Node already exists */
#define SKIP_ENOENT -3    /* Node does not exist */

struct _slnode;
struct _skiplist;

typedef struct _slnode SLNODE;
typedef struct _skiplist SLIST;

/* An individual node in the skip list */


struct _slnode
{
  void *data;     /* Pointer to the application's data */
  SLNODE **next;  /* Pointer to array of level pointers */
};

struct _skiplist
{
  SLNODE *first;  /* The first header node in the skip list */
  SLNODE *last;   /* Used for the FindFirst/Next functions */

  word maxlevel;  /* Maximum levels of pointers */
  word partition; /* Ratio of nodes for level 0 to level 1, lev 1 to 2, etc */
                  /* ie. If partiton==4, level 1 will have (on average)
                   * one fourth as many nodes as level one */
  word highlevel; /* The highest level currently used */
  word nodes;     /* Number of nodes in the skip list */
  
  int (_stdc *comp)(void *, void *);  /* Comparison function to use */
};

SLIST * _fast SkipCreateList(word maxlevel, word partition, int (_stdc *comp)(void *, void *));
void * _fast SkipAddNode(SLIST *sl, void *data, word *exists);
void * _fast SkipSearchList(SLIST *sl, void *data);
sword _fast SkipDeleteNode(SLIST *sl, void *data);
sword _fast SkipDestroyList(SLIST *sl);
void * _fast SkipFirst(SLIST *sl);
void * _fast SkipNext(SLIST *sl);

#endif /* __SKIPLIST_H_DEFINED */


