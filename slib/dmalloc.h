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

/*# name=Header for dmalloc() functions.
*/

#ifndef __DMALLOC_H_DEFINED
#define __DMALLOC_H_DEFINED

#ifndef DMALLOC
#define DMALLOC
#endif

#include <stddef.h>
#include "alc.h"    /* so the real malloc() functions can't be re-included */
#include "compiler.h"

void _stdc dmalloc_on(int shut_up);
char * cdecl dstrdup(char *s);
void * cdecl dmalloc(size_t size);
void * cdecl dcalloc(size_t numitems,size_t size);
void cdecl dfree(void *block);
void * cdecl drealloc(void *block,size_t size);
void _stdc dptab(int show_nondelta);
void * cdecl dsmalloc(size_t);
char * cdecl dsstrdup(char *s);
void heapchk(void);


#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif

#ifdef strdup
#undef strdup
#endif

#define malloc(p)     dmalloc(p)
#define realloc(p,n)  drealloc(p,n)
#define calloc(n,m)   dcalloc(n,m)
#define free(p)       dfree(p)
#define strdup(p)     dstrdup(p)
#define smalloc(p)    dsmalloc(p)
#define sstrdup(p)    dstrdup(p)

#endif /* !__DMALLOC_H_DEFINED */

