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

#include "prog.h"

#if defined(OS_2) && !defined(__FLAT__)

#define INCL_DOSMEMMGR
#include "pos2.h"

void huge * _fast h_malloc(long size)
{
  SEL sel;
  unsigned usSeg;
  unsigned usOfs;

  usSeg = (unsigned)(size / 65536L);
  usOfs = (unsigned)(size % 65536L);

  if (DosAllocHuge(usSeg, usOfs, &sel, 19, SEG_NONSHARED) != 0)
    return NULL;

  return MAKEP(sel, 0);
}

void _fast h_free(void huge *p)
{
  DosFreeSeg(SELECTOROF(p));
}


void huge * _fast h_realloc(void huge *p, long newsize)
{
  SEL sel=SELECTOROF(p);

  if (DosReallocHuge((unsigned)(newsize / 65536L),
                     (unsigned)(newsize % 65536L),
                     sel) != 0)
  {
    return NULL;
  }

  return MAKEP(sel, 0);
}

#else

#include <stdlib.h>
#include "prog.h"
#include "alc.h"

void huge * _fast h_malloc(long size)
{
  return farmalloc((size_t)size);
}

void _fast h_free(void huge *p)
{
  farfree((void *)p);
}

void huge * _fast h_realloc(void huge *p, long newsize)
{
  return farrealloc((void *)p, (size_t)newsize);
}

#endif


