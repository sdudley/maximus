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

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H
/**  @file typedefs.h Portable type definitions.
 * 
 *  In theory, these are the only type-related defines
 *  to change when switching hardware platforms.
 *
 *  The need for these definitions may seem odd,
 *  until you realize that on some platforms,
 *  long is 64 bits; on others it is 32... etc.
 *
 *  These definitions should be used when representing
 *  data which is written to disk or must go over the wire;
 *  otherwise, native types should be used.
 *
 *  These definitions should work for Sparc 32/64, Alpha, x86,
 *  and probably others. Sparc 64 and Alpha are both LP64.
 */
#include <sys/types.h>

#define INT8		char
#define INT16		short
#define INT32		int
#define INT64		long long

#define INT16_FORMAT	"hi"
#define INT32_FORMAT	"i"
#define INT64_FORMAT	"lli"

#define UINT16_FORMAT	"hu"
#define UINT32_FORMAT	"u"
#define UINT64_FORMAT	"llu"

#define UINT16_XFORMAT	"hx"
#define UINT32_XFORMAT	"x"
#define UINT64_XFORMAT	"llx"

#define SIZET_FORMAT	"lu"
#define POINTER_FORMAT	"p"

#include <sys/types.h>

#if defined(LINUX) || defined(SOLARIS)
# define HAVE_ULONG
# define HAVE_USHORT
#endif

/* These legacy max types imply a particular size */
typedef unsigned INT8	byte;
typedef signed INT8	sbyte;
typedef unsigned INT16	word;
typedef signed INT16	sword;
typedef unsigned INT32	dword;
typedef signed INT32	sdword;

/* I prefer these for new code */
typedef signed INT64	int64;
typedef signed INT32	int32;
typedef signed INT16	int16;
typedef signed INT8	int8;
typedef unsigned INT64	uint64;
typedef unsigned INT32	uint32;
typedef unsigned INT16	uint16;
typedef unsigned INT8	uint8;

#if defined(SYSV)
# include <stddef.h>
#endif

#if (!defined(LINUX) && !defined(_PTRDIFF_T)) || defined(NEED_PTRDIFF_T)
typedef ptrdiff_t	typeof((char *)1 - (char *)0) /* will that work? */
#endif

/* These legacy max types sort of imply a machine interface */
#ifndef HAVE_USHORT
typedef unsigned short ushort;
#endif

#ifndef HAVE_ULONG
typedef unsigned long	ulong;
#endif

typedef signed short	sshort;
typedef signed long	slong;
typedef	unsigned int	bit;

#include "hntypes.h"
#endif /* _TYPEDEFS_H */
