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

#ifndef __STAMP_H_DEFINED
#define __STAMP_H_DEFINED

#include "typedefs.h"
#include "compiler_details.h"
#include "compiler_align.h"

#define STAMP_BITFIELD uint32
#define HALF_STAMP_BITFIELD uint16

/** DOS-style datestamp */
struct _timestamp
{
#if defined(LITTLE_ENDIAN)
  STAMP_BITFIELD	da : 5;
  STAMP_BITFIELD	mo : 4;
  STAMP_BITFIELD	yr : 7;
  STAMP_BITFIELD 	ss : 5;
  STAMP_BITFIELD 	mm : 6;
  STAMP_BITFIELD 	hh : 5;
#elif defined(BIG_ENDIAN)
  STAMP_BITFIELD 	ss : 5;
  STAMP_BITFIELD 	mm : 6;
  STAMP_BITFIELD 	hh : 5;
  STAMP_BITFIELD	da : 5;
  STAMP_BITFIELD	mo : 4;
  STAMP_BITFIELD	yr : 7;
#else
# error Neither BIG_ENDIAN nor LITTLE_ENDIAN are defined!
#endif
} PACKED;

union _stampu
{
  struct _timestamp date;
  struct _timestamp time;
};

struct _dos_st
{
#if defined(LITTLE_ENDIAN)
  HALF_STAMP_BITFIELD date PACKED;
  HALF_STAMP_BITFIELD time PACKED;
#elif defined(BIG_ENDIAN)
  HALF_STAMP_BITFIELD time PACKED;
  HALF_STAMP_BITFIELD date PACKED;
#else
# error Neither BIG_ENDIAN nor LITTLE_ENDIAN are defined!
#endif
} PACKED;

/* Union so we can access stamp as "int" or by individual components */

union stamp_combo   
{
  STAMP_BITFIELD ldate;
  union _stampu msg_st;
  struct _dos_st dos_st;
} PACKED;

typedef union stamp_combo SCOMBO;

/* strange union-union-struct to preserve original
 * semantic as much as possible; it was union-struct-struct.
 *
 * so, hopefully, most code that looks like this:
 * stamp_combo.msg_st.date.mo will still work with
 * a simple variable declaration change..
 * 
 * Original syntax was no good because on some machines
 * (e.g. sparc) you can't declare structs on less than
 * a 32-bit boundary (gcc 2.95.2) no matter how hard you
 * try (packed, aligned)
 *
 * 		old					new
 * stamp_combo	struct _stamp				union _stampu
 * msg_st	{ struct _date; struct _time }		struct _timestamp
 * dos_st	{ int16; int16; }			{ int16; int16; }
 */
#endif /* __STAMP_H_DEFINED */

