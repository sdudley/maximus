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

/*
   COMPAT.H: EXEC function with memory swap - 
             Borland/Microsoft compatibility header file.

   Public domain software by

        Thomas Wagner
        Ferrari electronic GmbH
        Beusselstrasse 27
        D-1000 Berlin 21
        Germany
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <dos.h>

#if defined(__TURBOC__)
#define TURBO  1
#define MSC    0
#else
#define TURBO  0
#define MSC    1
#endif

#if (TURBO)
#include <dir.h>
#include <alloc.h>
#include <sys\stat.h>

#define OS_MAJOR  (_version & 0xff)

#endif

#if (MSC)
#include <malloc.h>
#include <direct.h>
#include <sys\types.h>
#include <sys\stat.h>

#define fnsplit   _splitpath
#define fnmerge   _makepath

#define MAXPATH   128
#define MAXDRIVE  _MAX_DRIVE
#define MAXDIR    128
#define MAXFILE   _MAX_FNAME
#define MAXEXT    _MAX_EXT

#define FA_RDONLY _A_RDONLY
#define FA_HIDDEN _A_HIDDEN
#define FA_SYSTEM _A_SYSTEM
#define FA_ARCH   _A_ARCH

#define findfirst(a,b,c) _dos_findfirst(a,c,b)
#define ffblk find_t

#define OS_MAJOR  _osmajor

#define farmalloc(x)    ((void far *)halloc(x,1))
#define farfree(x)      hfree((void huge *)x)

#define stpcpy(d,s)     (strcpy (d, s), d + strlen (s))

#ifndef MK_FP
#define MK_FP(seg,ofs)  ((void far *)(((unsigned long)(seg) << 16) | (unsigned short)(ofs)))
#endif

#endif

