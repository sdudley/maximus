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

#ifndef __TYPEDEFS_H_DEFINED
#define __TYPEDEFS_H_DEFINED

/* Handle the special defines for 32-bit flat model compilers */

#if defined(__386__) || defined(__FLAT__)
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned short word;
  typedef signed short   sword;

  typedef unsigned int  dword;
  typedef signed int    sdword;

#if !defined(NT) && !defined(UNIX)
  typedef unsigned short ushort;
#endif

  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#else
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned int   word;
  typedef signed int    sword;

  typedef unsigned long dword;
  typedef signed long   sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#endif

#endif

