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

/* test code for doing memory allocation using 16-bit OS/2 APIs */

#undef malloc
#undef free
#undef realloc
#undef strdup

void     *qmalloc( size_t __size )
{
  USHORT rc;
  SEL sel;
  void *p;

  rc = DosAllocSeg(((__size + 1) / 2) * 2, &sel, SEG_NONSHARED);
  p = rc==0 ? MAKEP(sel, 0) : 0;

  return p;
}

void      qfree( void *__ptr )
{
  USHORT rc;

//  assert(OFFSETOF(__ptr)==0);
  rc=DosFreeSeg(SELECTOROF(__ptr));
  assert(rc==0);
}

void     *qrealloc( void *__ptr, size_t __size )
{
  void *p;

  p = qmalloc(__size);

  if (!p)
    return NULL;

  memcpy(p, __ptr, __size);
  qfree(__ptr);
  return p;
}

char *qstrdup( const char *__string )
{
  char *p = qmalloc(strlen(__string) + 1);

  if (p)
    strcpy(p, __string);

  return p;
}


