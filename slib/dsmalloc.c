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

#include <string.h>
#include <stdlib.h>
#include <dos.h>
#include "prog.h"
#undef offsetof
#include "dmalloc.h"
#undef offsetof
#define offsetof(typename,var) (size_t)(&(((typename *)0)->var))
#include "dmpvt.h"
#include "fpseg.h"


#undef dmalloc
#undef dsmalloc

void _fast NoMem(void);


extern struct _ptab * _stdc table;
extern int _stdc nptr;
extern int _stdc atdone;
extern int _stdc shutup;
extern FILE *_stdc efile;



void * cdecl dsmalloc(size_t size)
{
  char *p;

#if defined(__MSC__) || defined(__WATCOMC__)
  if (table)
  {
    int h = _heapchk();
    if( h != _HEAPOK && h != _HEAPEMPTY)
    {
        fprintf(efile, "heap corrupted\n");
        exit(1);
    }
  }
#endif

  p=malloc(size + (table ? DBEXTRA : 0));

  if (table)
  {
    if (!shutup)
      fprintf(efile, "DSML SZ=%04lx FR=%04x:%04x MM=%04x:%04x FR=%6ld\n",
              (unsigned long)size,
              FP_SEG(MAGICSTACK(size)), FP_OFF(MAGICSTACK(size)),
              FP_SEG(p), FP_OFF(p),
              (long)coreleft());


    if (p==NULL || d_add_table(p,MAGICSTACK(size), size))
    {
      fprintf(efile, "dsmalloc ERROR: caller %p\n",MAGICSTACK(size));
      exit(1);
    }
    
    /* Now add the checking information */

    *(int *)&p[0]=KEY1;
    *(int *)&p[2]=KEY2;
    p += 4;
    
    *(int *)&p[size]=KEY1;
    *(int *)&p[size+2]=KEY2;
  }
  
  if (p)
    memset(p, '\0', size);

  return (void *)p;
}


