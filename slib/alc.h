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

/*# name=Include file to pick between MALLOC.H and ALLOC.H
*/

#ifndef __ALC_H_DEFINED
#define __ALC_H_DEFINED

#include "compiler.h"

#if defined(_lint) || defined(__MSC__) || defined(__WATCOMC__) || defined(__IBMC__) || defined(__TOPAZ__)
  #include <malloc.h>

  #ifdef __FARDATA__

  /* for some insane reason the turbo-c coreleft() function changes
   * it's return value based on the memory model.
   */

    unsigned long cdecl coreleft   (void);
  #else
    unsigned cdecl coreleft        (void);
  #endif

#elif defined(__TURBOC__)
  #include <alloc.h>
#else
  #include <string.h>
#endif

#ifdef __TURBOC__
#define halloc(x,y) ((char far *)farmalloc(x*y))
#define hfree(p)    farfree(p)
#endif

#endif /* __ALC_H_DEFINED */

