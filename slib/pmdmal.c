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

/* Debugging malloc() calls.  This module was lifted from another project
   of mine to help debug memory allocation routines.
*/


#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include "prog.h"
#include "alc.h"
#include "dmpvt.h"


struct _ptab * _stdc table;
int _stdc nptr;
int _stdc atdone=FALSE;
int _stdc shutup=FALSE;

#ifdef __IBMC__
FILE *_stdc efile;
#else
FILE *_stdc efile=stderr;
#endif


#ifdef OS_2
  #include <pos2.h>

  #define coreleft() 0
  #define tdelay(x) ;

  #ifndef __FLAT__
    static ULONG rsem=0;
  #endif
#endif


void dmsync(void)
{
#if defined(OS_2) && !defined(__FLAT__)
  DosSemRequest((HSEM)&rsem, SEM_INDEFINITE_WAIT);
#endif
}

void dmunsync(void)
{
#if defined(OS_2) && !defined(__FLAT__)
  DosSemClear((HSEM)&rsem);
#endif
}







void _stdc dptab(int show_nondelta)
{
  int x;

  if (!table || !efile)
    return;

  fprintf(efile, "--- pointers: %d (freemem=%ld)\n\n",nptr,(long)coreleft());

  for (x=0;x < nptr;x++)
  {
    if (table[x].delta==TRUE || show_nondelta)
      fprintf(efile, "FR=" PTR_TEXT " AT=" PTR_TEXT " SZ=" PTR_SIZE "%s",
             POINTER(table[x].aat),
             POINTER(table[x].p),
             (unsigned long)table[x].size,
             (x & 1)==1 ? "\n" : "\t");

    table[x].delta=FALSE;
  }
}

static void _stdc _dptabend(void)
{
  dptab(TRUE);
}



void _stdc dmalloc_on(int type)
{
#ifdef __IBMC__
  efile=stderr;
#endif

  if ((table=malloc(sizeof(struct _ptab) * MAX_PTR))==NULL)
  {
    fprintf(efile, "not enough memory for dmalloc()\n");
    dptab(TRUE);
    flush_handle(efile);
    exit(1);
  }

  memset(table,'\0',sizeof(struct _ptab) * MAX_PTR);
  nptr=0;

  if (! atdone)
  {
    atexit(_dptabend);
    atdone=TRUE;
  }

  if (type)
    shutup=TRUE;
}


int _stdc d_add_table(void *p, void MODTYPE * magic, int size)
{
  int x;

  for (x=0;x < nptr;x++)
    if (p==table[x].p)
      return 1;

  if (nptr==MAX_PTR)
  {
    fprintf(efile, "ERROR!  Too many pointers allocated.\n");
    return 1;
  }

  memmove(table+1,table,(nptr++)*sizeof(struct _ptab));
  table->p=p;
  table->aat=magic;
  table->delta=TRUE;
  table->size=size;
  return 0;
}

int _stdc d_remove_table(void *ptr)
{
  int x;
  char *p=ptr;

  for (x=0;x < nptr;x++)
    if (p==table[x].p)
    {
      if ((*(int *)&p[0] != KEY1) || (*(int *)&p[sizeof(int)] != KEY2) ||
          (*(int *)&p[table[x].size+sizeof(int)*2] != KEY1) ||
          (*(int *)&p[table[x].size+sizeof(int)*3] != KEY2))
      {
        fprintf(efile, "d_remove_table - memory grunged!\n");
        return 1;
      }

      memmove(table+x,table+x+1,((nptr--)-x-1)*sizeof(struct _ptab));
      return 0;
    }

  fprintf(efile, "d_remove_table - not found in table!\n");
  return 1;
}


void * cdecl dmalloc(size_t size)
{
  char *p;

  dmsync();

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY)
    {
        fprintf(efile, "heap corrupted\n");
        dptab(TRUE);
        table=NULL;
        flush_handle(efile);
        dmunsync();

        *(char far *)0=0;
        exit(1);
    }
  }
#endif

  p=malloc(size + (table ? DBEXTRA : 0));

  if (table)
  {
    if (!shutup)
      fprintf(efile, "DMAL SZ=" PTR_SIZE " FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT "\n",
             (unsigned long)size,
             POINTER(MAGICSTACK(size)),
             POINTER(p),
             (long)coreleft());

    if (p==NULL)
    {
      fprintf(efile, "dmalloc returned NULL: caller PTR_TEXT\n",
              POINTER(MAGICSTACK(size)));

      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }

    if (d_add_table(p, MAGICSTACK(size), size))
    {
      fprintf(efile, "dmalloc returned duplicate pointer: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(size)));
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }
    
    /* Now add the checking information */

    *(int *)&p[0]=KEY1;
    *(int *)&p[sizeof(int)]=KEY2;
    p += sizeof(int)*2;
    
    *(int *)&p[size]=KEY1;
    *(int *)&p[size+sizeof(int)]=KEY2;
  }

  dmunsync();

  return (void *)p;
}


char * cdecl dstrdup(char *s)
{
  char *p;
  int size=strlen(s)+1;

  dmsync();

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY){
        fprintf(efile, "heap corrupted\n");
        dptab(TRUE);
        table=NULL;
        flush_handle(efile);
        dmunsync();
        exit(1);
    }
  }
#endif

  p=malloc(size+(table ? DBEXTRA : 0));

  if (table)
  {
    if (!shutup)
      fprintf(efile, "STRD SZ=" PTR_SIZE " FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT " \"%s\"\n",
             (unsigned long)size,
             POINTER(MAGICSTACK(s)),
             POINTER(p),
             (long)coreleft(),
             s);

    if (p==NULL || d_add_table(p,MAGICSTACK(s), size))
    {
      fprintf(efile, "dstrdup ERROR: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(s)));
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }

    /* Now add the checking information */

    *(int *)&p[0]=KEY1;
    *(int *)&p[sizeof(int)]=KEY2;
    p += sizeof(int)*2;
    
    *(int *)&p[size]=KEY1;
    *(int *)&p[size+sizeof(int)]=KEY2;
  }

  strcpy(p,s);

  dmunsync();
  return p;
}

void * cdecl dcalloc(size_t numitems,size_t size)
{
  char *p;
  int sz=numitems*size;

  dmsync();

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY){
        fprintf(efile, "heap corrupted\n");
        dptab(TRUE);
        table=NULL;
        flush_handle(efile);
        dmunsync();
        exit(1);
    }
  }
#endif

  p=malloc(sz + (table ? DBEXTRA : 0));

  if (p)
    memset(p, '\0', sz + (table ? DBEXTRA : 0));

  if (table)
  {
    if (!shutup)
      fprintf(efile, "DCAL SZ=" PTR_SIZE " FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT "\n",
             (unsigned long)sz,
             POINTER(MAGICSTACK(numitems)),
             POINTER(p),
             (long)coreleft());

    if (p==NULL || d_add_table(p, MAGICSTACK(numitems), sz))
    {
      fprintf(efile, "dcalloc ERROR: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(size)));
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }

    /* Now add the checking information */

    *(int *)&p[0]=KEY1;
    *(int *)&p[sizeof(int)]=KEY2;
    p += sizeof(int)*2;
    
    *(int *)&p[sz]=KEY1;
    *(int *)&p[sz+sizeof(int)]=KEY2;
  }

  dmunsync();
  return (void *)p;
}

void cdecl dfree(void *b)
{
  char *block=b;

  dmsync();

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY){
        fprintf(efile, "heap corrupted\n");
        dptab(TRUE);
        table=NULL;
        flush_handle(efile);
        dmunsync();
        exit(1);
    }
  }
#endif

  if (table)
  {
    block -= DB_SIZE;

    if (!shutup)
#ifdef __FLAT__
      fprintf(efile, "DFRE ----------- FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT "\n",
#else
      fprintf(efile, "DFRE ------- FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT "\n",
#endif
             POINTER(MAGICSTACK(b)),
             POINTER(block),
             (long)coreleft());

    if (d_remove_table(block))
    {
      fprintf(efile, "dfree ERROR: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(b)));
      tdelay(2000);
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      *(char far *)0=0;
      exit(1);
    }
  }

  free(block);

  dmunsync();
}

void * cdecl drealloc(void *b,size_t size)
{
  char *block=b;
  char *p;

  dmsync();

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY){
        fprintf(efile, "heap corrupted\n");
        dptab(TRUE);
        table=NULL;
        flush_handle(efile);
        dmunsync();
        exit(1);
    }
  }
#endif

  if (table)
  {
    block -= DB_SIZE;

    if (d_remove_table(block))
    {
      fprintf(efile, "drealloc(1) error: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(size)));
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }
  }

  p=realloc(block, size+(table ? DBEXTRA : 0));

  if (table)
  {
    if (!shutup)
      fprintf(efile, "DRLC SZ=" PTR_SIZE " FR=" PTR_TEXT " MM=" PTR_TEXT " SP=" SPC_TEXT " OL=" PTR_TEXT "\n",
             (unsigned long)size,
             POINTER(MAGICSTACK(b)),
             POINTER(p),
             (long)coreleft(),
             POINTER(b));

    if (d_add_table(p,MAGICSTACK(block), size))
    {
      fprintf(efile, "drealloc(2) error: caller " PTR_TEXT "\n",
              POINTER(MAGICSTACK(size)));
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();
      exit(1);
    }
    
    p += sizeof(int)*2;

    *(int *)&p[size]=KEY1;
    *(int *)&p[size+sizeof(int)]=KEY2;
  }

  dmunsync();
  return (void *)p;
}

#include "dmalloc.h"

#ifdef TEST
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

  p2=malloc(100);
  p1=malloc(5);

  free(p1);
  free(p2);

  p2=strdup("asdfasdf");
  *p2=1;
  free(p2);

  return 0;
}
#endif

void heapchk(void)
{
  int h = _heapchk();
  if( h != _HEAPOK && h != _HEAPEMPTY)
  {
      fprintf(efile, "heap corrupted\n");
      dptab(TRUE);
      table=NULL;
      flush_handle(efile);
      dmunsync();

      *(char far *)0=0;
      exit(1);
  }
}

