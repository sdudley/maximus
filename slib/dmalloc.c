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

/*# name=Debugging malloc() calls
*/

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include "prog.h"
#include "alc.h"

#if defined(__FLAT__)

#define MAX_DPTR      8192
#define FIRST_KEY   0x12345678L
#define LAST_KEY    0x87654321L
#define KEY_SIZES   (sizeof(unsigned long)*2)

typedef struct
{
  void *p;
  unsigned long ulFrom;
  size_t size;
} PTAB;


static PTAB *ptab;
static int fQuiet;
static int cPtrs;


#if defined(__WATCOMC__) && defined(__FLAT__)
  unsigned long get_bp(void);
  #pragma aux get_bp = "mov eax,ebp" value [eax];

  #define get_caller() *(((unsigned long *)get_bp())+1)

#else
  #define get_caller() 0
#endif



static void near dmalloc_err(char *fmt, ...)
{
  static char szErr[PATHLEN];
  va_list va;

  va_start(va, fmt);
  vsprintf(szErr, fmt, va);
  va_end(va);

  fprintf(stderr, "%s\n", szErr);

  for (;;)
    ;
}


void _stdc _dptabend(void)
{
  int i;

  setbuf(stderr, NULL);

  for (i=0; i < cPtrs; i++)
  {
    fprintf(stderr, "Ptr=%08lx; from=%08lx\n",
            ptab[i].p,
            ptab[i].ulFrom);
  }
}

void _stdc dmalloc_on(int fQ)
{
  static int fDoneAtExit = FALSE;

  fQuiet = fQ;

  if ((ptab = malloc(sizeof(PTAB) * MAX_DPTR))==NULL)
    dmalloc_err("Can't allocate memory for pointer table");

  memset(ptab, 0, sizeof(PTAB) * MAX_DPTR);

  if (! fDoneAtExit)
  {
    atexit(_dptabend);
    fDoneAtExit = TRUE;
  }
}


static void add_table(void *p, unsigned long ulWhere, size_t size)
{
  int i;

  for (i=0; i < cPtrs; i++)
  {
    if (ptab[i].p==p)
    {
      dmalloc_err("add_table - pointer %08lx already in table (from %08lx)",
                  p, ptab[i].ulFrom);
    }
  }

  if (i==MAX_DPTR)
    dmalloc_err("add_table - too many pointers");

  ptab[i].p = p;
  ptab[i].ulFrom = ulWhere;
  ptab[i].size = size;
  cPtrs++;
}

static void remove_table(void *p)
{
  char *pszP = p;
  int i;

  for (i=0; i < cPtrs; i++)
  {
    if (p==ptab[i].p)
    {
      if (*(unsigned long *)pszP != FIRST_KEY ||
          *(unsigned long *)(pszP + sizeof(unsigned long) + ptab[i].size) != LAST_KEY)
      {
        dmalloc_err("remove_table - key grunged for pointer %08lx - from %08lx",
                    p, ptab[i].ulFrom);

      }

      memmove(ptab+i, ptab+i+1, (cPtrs - i - 1) * sizeof(PTAB));
      cPtrs--;
      return;
    }
  }

  if (i==cPtrs)
    dmalloc_err("remove_from_table - could not find %08lx", p);
}

void check_table()
{
  int i;
  char *pszP;

  for (i=0; i < cPtrs; i++)
  {
    pszP = ptab[i].p;

    if (*(unsigned long *)pszP != FIRST_KEY ||
        *(unsigned long *)(pszP + sizeof(unsigned long) + ptab[i].size) != LAST_KEY)
    {
      dmalloc_err("check_heap - key grunged for pointer %08lx - from %08lx",
                  pszP, ptab[i].ulFrom);
    }
  }
}



void check_heap(void)
{
#if defined(__MSC__) || defined(__WATCOMC__)
  int h;

  if (!ptab)
    return;

  check_table();

  h = _heapchk();

  if (h != _HEAPOK && h != _HEAPEMPTY)
    dmalloc_err("heap is corrupted");
#endif
}



static void * dmalloc_int(size_t size, unsigned long ulCaller)
{
  char *p;

  check_heap();

  p=malloc(size + (ptab ? KEY_SIZES : 0));

  if (ptab)
  {
    if (p==NULL)
      dmalloc_err("dmalloc() returned NULL");

    add_table(p, ulCaller, size);
    
    /* Now add the checking information */

    *(unsigned long *)p = FIRST_KEY;
    p += sizeof(unsigned long);

    *(unsigned long *)(p+size) = LAST_KEY;
  }

  return (void *)p;
}

void * cdecl dmalloc(size_t size)
{
  return dmalloc_int(size, get_caller());
}

static char * dstrdup_int(char *s, unsigned long ulCaller)
{
  char *p;

  p = dmalloc_int(strlen(s) + 1, ulCaller);
  strcpy(p, s);
  return p;
}

char * cdecl dstrdup(char *s)
{
  return dstrdup_int(s, get_caller());
}

static void * dcalloc_int(size_t numitems,size_t size, unsigned long ulCaller)
{
  char *p;
  int sz=numitems*size;

  p = dmalloc_int(sz, ulCaller);
  memset(p, 0, sz);

  return (void *)p;
}

void * cdecl dcalloc(size_t numitems,size_t size)
{
  return dcalloc_int(numitems, size, get_caller());
}

void cdecl dfree(void *b)
{
  char *block=b;

  if (ptab)
  {
    block -= sizeof(unsigned long);

    check_heap();
    remove_table(block);
  }

  free(block);
}

static void * drealloc_int(void *b,size_t size, unsigned long ulCaller)
{
  char *new;

  new = dmalloc_int(size, ulCaller);

  if (!new)
    return NULL;

  memcpy(new, b, size);
  dfree(b);
  return new;
}

void * cdecl drealloc(void *b,size_t size)
{
  return drealloc_int(b, size, get_caller());
}

#ifdef TEST
#include "dmalloc.h"

main()
{
  char *p1, *p2, *p3;

  dmalloc_on(0);

  p1=malloc(5);
  p2=malloc(5);
  p3=malloc(5);

  free(p1);
  free(p3);

  p1=realloc(p2, 10);
  free(p1);

  p3 = malloc(5);
  p2=malloc(100);
  p1=malloc(5);

  free(p1);
  free(p2);

  p2=strdup("asdfasdf");
  *p2=1;
  free(p2);


  return 0;
}
#endif /* TEST */

#endif /* __FLAT__ */

