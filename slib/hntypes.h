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

/*# name=Type definitions for Hungarian notation
*/

#ifndef __HNTYPES_H_DEFINED
#define __HNTYPES_H_DEFINED

typedef ushort		USHORT;
typedef sshort		SHORT;
typedef ulong		ULONG;
typedef slong		LONG;
typedef ushort		BOOL;
typedef uint8		BYTE;

typedef ushort *	PUSHORT;
typedef sshort *	PSHORT;
typedef ulong *		PULONG;
typedef slong *		PLONG;
typedef ushort *	PBOOL;
typedef uint8 *		PBYTE;

typedef dword		DWORD;
typedef sword		SWORD;
typedef word		WORD;
typedef void		VOID;

typedef dword *		PDWORD;
typedef sword *		PSWORD;
typedef word *		PWORD;

#ifdef __GNUC__
typedef void *		PVOID;
#else
typedef char *		PVOID;
#endif

typedef char *		LPTSTR;
typedef const char *	LPCSTR;
#endif


