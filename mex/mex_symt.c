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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: mex_symt.c,v 1.3 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Symbol table routines.
    name=
    name=This symbol table package is implemented as a hash table (with
    name=a size defined at runtime), using chaining for collision
    name=resolution.
*/

#include <mem.h>
#include <string.h>
#include "alc.h"
#include "prog.h"
#include "mex.h"

static VMADDR near _st_hash(SYMTAB *st, byte *name);
static void near _st_kill_attribute_rec(ATTRIBUTES *ap);

/* Function to create a symbol table, with enough space to hold 'len' objs */

SYMTAB *st_create(unsigned len, SYMTAB *parent)
{
  SYMTAB *st;

  if ((st=malloc(sizeof(SYMTAB)))==NULL)
    return NULL;

  st->parent=parent;

  /* Allocate enough memory for a list of pointers to the objects */
  
  if ((st->table=malloc(len*sizeof(ATTRIBUTES *)))==NULL)
  {
    free(st);
    return NULL;
  }

  /* Make them all into NULL pointers */
  
  memset(st->table,'\0',len*sizeof(ATTRIBUTES *));
  st->len=len;
  
  return st;
}

/* Destroy the symbol table pointed to by 'st' */

void st_destroy(SYMTAB *st)
{
  if (st)
  {
    free(st->table);
    st->table=NULL;
    free(st);
  }
}


/* Enter a name into the symbol table 'st', and return a pointer to the     *
 * newly-created attributes record, waiting to be filled out.  If the       *
 * entry is already in the table, the 'present' flag will be set to         *
 * TRUE, and the attribute record corresponding to that entry will          *
 * be returned.                                                             */

ATTRIBUTES * st_enter(SYMTAB *st, byte *name, byte *present)
{
  ATTRIBUTES *ap, *last;
  unsigned pos;
  
  if (present)
    *present=FALSE;

  if (!st)
    bug("Garbled symbol table");
  
  pos=_st_hash(st, name);
  

  /* Search through the linked list, to see if it's already there...  If  *
   * we find an entry that doesn't belong to our scope, then it's safe    *
   * to stop looking.                                                     */

  for (ap=st->table[pos]; ap && ap->scope==scope; ap=ap->_next)
  {
    /* If it equals the name we're trying to add, it must've been there   *
     * already, so return it.  (Probably an error!)                       */

    if (ap->name && eqstr(ap->name,name))
    {
      if (present)
        *present=TRUE;

      return ap;
    }
  }

  /* If we got this far, it must not have been there, so tack it        *
   * onto the BEGINNING of the chain.                                   */

  last=st->table[pos];
  
  ap=st->table[pos]=smalloc(sizeof(ATTRIBUTES));
  
  /* We've created a new entry in the symbol table, so return it. */

  ap->_next=last;
  ap->name=strdup(name);
  ap->scope=scope;
  return ap;
}



/* Search for the named symbol in the symbol table, and return a pointer    *
 * to its attributes record.                                                */

ATTRIBUTES * st_find(SYMTAB *st, byte *name, int search_parent)
{
  ATTRIBUTES *ap;
  unsigned pos;
  
  if (!st)
    return NULL;

  do
  {
    /* Get hash value of name */
  
    pos=_st_hash(st,name);

    /* Now search through linked list, looking for a match */
  
    for (ap=st->table[pos]; ap; ap=ap->_next)
      if (ap->name && eqstr(name, ap->name))
        return ap;

    /* If we got this far, it wasn't found.  Walk up to the parent          *
     * symbol table, if necessary, to find the requisite items.             */
  }
  while (search_parent && (st=st->parent) != NULL);

  return NULL;
}

/* Perform a search'n'destroy mission on the specified symbol table, and    *
 * remove any entries with a scope number which is >= scope.  This fucntion *
 * returns the number of bytes of storage which were freed.  (This count    *
 * is utilized to reuse space in the allocation record for blocks within    *
 * functions.)                                                              */

VMADDR st_killscope(SYMTAB *st, VMADDR scope)
{
  ATTRIBUTES **chain, *last, *ap;
  VMADDR storage_freed=0;
  
  for (chain=st->table; chain < st->table+st->len; chain++)
    for (ap=*chain, last=NULL; ap; ap=ap->_next)
    {
      /* If we don't need to kill this one, indicate that it was our        *
       * prior entry in the table.  Otherwise, kill it!                     */

      if (ap->scope < scope /*|| ap->class==ClassArg*/)
      {
        last=ap;
      }
      else
      {
        if (ap->class==ClassArg || ap->class==ClassValidate)
        {
          /* If it's a function argument, we need to keep it in the symbol  *
           * table to store type information.  However, we don't want it to *
           * be findable by any of the other symtab routines, so we clear   *
           * its name entry (and set to NULL), which will force any         *
           * name comparisons to fail.                                      */

          if (ap->name)
            _st_kill_attribute_rec(ap);

          last=ap;
          continue;
        }
        else if (ap->class==ClassFunc)
        {
          /* We never want to remove a function declaration from the table */
          last=ap;
          continue;
        }
        else if (ap->class==ClassVariable)
        {
          if (ap->typedesc)
          {
            #ifdef DECLDEBUG
            debug("Deallocating '%s' (%d / %d)",
                  ap->name, ap->typedesc->size, storage_freed);
            #endif

            storage_freed += ap->typedesc->size;
          }
        }

        /* Perform any necessary cleanup on the contents of the attrib rec */

        _st_kill_attribute_rec(ap);


        /* If there is no 'last' (ie. this is the first entry in the        *
         * chain), then just set the beginning of the chain itself to the   *
         * next entry.                                                      */
        
        if (!last)
          *chain=ap->_next;
        else last->_next=ap->_next;


        /* Free the memory allocated to this attribute record */

        free(ap);
      }

    }

  return storage_freed;
}

static VMADDR near _st_hash(SYMTAB *st,byte *name)
{
  VMADDR hash;
  char *p;
  
  for (p=name, hash=1; *p; p++)
    hash=(hash * *p)+*p;
  
  return (hash % st->len);
}



/* Delete the attribute record pointed to by 'ap', and handle any special   *
 * internal deletions and memory management needed.                         */

static void near _st_kill_attribute_rec(ATTRIBUTES *ap)
{
  DATAOBJ *o;

  if (ap->typedesc==&StringType && ap->name && ap->class != ClassValidate &&
      ap->ref==FALSE)
  {
    o=idref(ap->name);

    /* Only slay direct strings */

    if (!o->form.addr.indirect || o->form.addr.segment != SEG_TEMP)
      Generate(QOP_SKILL, o, NULL, NULL);
  }

  #ifdef DECLDEBUG
  debug("KILLING attrib for '%s'", ap->name);
  #endif
  
  if (ap->name)
    free(ap->name);

  ap->name=NULL;
}


