/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/***************************************************************************/
/*                                                                         */
/*            DW-TEXT: Memory manager subsystem                            */
/*                     Header module                                       */
/*                                                                         */
/*            Created: 10/Jan/90                                           */
/*            Updated: 12/Jul/97                                           */
/*                                                                         */
/*            Copyright (C) 1990-1997 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#ifndef DW_MEM_DEFS
#define DW_MEM_DEFS

/////////////////////////////////////////////////////////////////////////////
// Memory manager function prototypes                                      //
/////////////////////////////////////////////////////////////////////////////

  BOOL APIENTRY  MemInitMemMgr(VOID);

  // Global heap management routines

  PVOID APIENTRY MemAlloc(size_t cb, USHORT fs);
  PVOID APIENTRY MemRealloc(PVOID p, size_t cb);
  VOID  APIENTRY MemFree(PVOID p);
  ULONG APIENTRY MemAvail(VOID);

#define MemGlobalAlloc          MemAlloc
#define MemGlobalRealloc        MemRealloc
#define MemGlobalFree           MemFree
#define MemGlobalAvail          MemAvail

  // Local heap management routines ... none so far

#define MemLocalAlloc           MemAlloc
#define MemLocalRealloc         MemRealloc
#define MemLocalFree            MemFree
#define MemLocalAvail           MemAvail

  // Memory allocation flags

#define MA_CLEAR        0x0001  // Initialize to zeros
#define MA_FIXED        0x0002  // Segment is fixed in memory
#define MA_MOVEABLE     0x0004  // Segment can be moved around
#define MA_DISCARDABLE  0x0008  // Segment can be discarded
#define MA_NOXMSSWAP    0x0010  // Segment can't be swapped out to XMS
#define MA_NOEMSSWAP    0x0020  // Segment can't be swapped out to EMS
#define MA_NODOSSWAP    0x0040  // Segment can't be swapped out to disk

/////////////////////////////////////////////////////////////////////////////
// Absolute memory address references
/////////////////////////////////////////////////////////////////////////////

#ifdef __DOS__
  extern USHORT GLOBAL _0000h;
  extern USHORT GLOBAL _0040h;
  extern USHORT GLOBAL _A000h;
  extern USHORT GLOBAL _B000h;
  extern USHORT GLOBAL _B800h;
#endif

#endif /* DW_MEM_DEFS */

/***************************************************************************
* End of DW-MEM.H                                                          *
****************************************************************************/
