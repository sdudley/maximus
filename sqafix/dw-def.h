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
/*            DW-text: Common declarations                                 */
/*                                                                         */
/*            Created: 16/Jan/90                                           */
/*            Updated: 10/Jul/97                                           */
/*                                                                         */
/*            Copyright (C) 1990-1997 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#ifndef DW_ALL_DEFS
#define DW_ALL_DEFS

#include "dw-saa.h"

/////////////////////////////////////////////////////////////////////////////
// Compiler dependant declarations                                         //
/////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4068)           // unknown pragma
#pragma warning(disable:4761)           // integral type mismatch
#pragma warning(disable:4146)           // unary minus to unsigned type
#endif

#ifdef __FLAT__
#define DW_STR_STDCALLS
#endif

/////////////////////////////////////////////////////////////////////////////
// Basic type and structure declarations                                   //
/////////////////////////////////////////////////////////////////////////////

  // Standard procedure calling conventions

#ifndef OS2DEF_INCLUDED
#define APIENTRY PASCAL FAR     // API routine entry
#define EXPENTRY PASCAL FAR     // Exported routine entry
#endif

#define HLPENTRY PASCAL FAR     // Helper routine entry
#define APPENTRY PASCAL FAR     // Application routine entry

#ifndef __DOSOVL__
#define SUBENTRY PASCAL NEAR    // Subroutine entry
#else
#define SUBENTRY PASCAL FAR     // Most ovl managers require far calls through
#endif

  // Standard variable naming conventions

#define GLOBAL  PASCAL          // Global variable

  // Common handle prototypes

  typedef unsigned short SHANDLE;
  typedef void FAR * LHANDLE;
  typedef LHANDLE  HANDLE;

  // This defines a point object

  typedef struct _POINT {  /* pt */
    SHORT x;
    SHORT y;
  } POINT, FAR * PPOINT;

  // This defines a rectangle object

  typedef struct _RECT {  /* rc */
    SHORT xLeft;
    SHORT yTop;
    SHORT xRight;
    SHORT yBottom;
  } RECT, FAR * PRECT;

  // This defines a size object

  typedef struct _SIZE {  /* size */
    SHORT cx;
    SHORT cy;
  } SIZE, FAR * PSIZE;

  // Video input/output subsystem common stuff

  typedef HANDLE HDC;           // Display context handle
  typedef USHORT CMAP;          // Clipping map logical word
  typedef USHORT CELL;          // Vio buffer cell
  typedef CELL FAR * PCELL;     // Vio buffer pointer
  typedef CMAP FAR * PCMAP;     // Clipping map pointer

/////////////////////////////////////////////////////////////////////////////
// Color attribute definitions                                             //
/////////////////////////////////////////////////////////////////////////////

#define CLR_BLACK        0x0000
#define CLR_BLUE         0x0001
#define CLR_GREEN        0x0002
#define CLR_CYAN         0x0003
#define CLR_RED          0x0004
#define CLR_MAGENTA      0x0005
#define CLR_BROWN        0x0006
#define CLR_LIGHTGRAY    0x0007
#define CLR_DARKGRAY     0x0008
#define CLR_LIGHTBLUE    0x0009
#define CLR_LIGHTGREEN   0x000a
#define CLR_LIGHTCYAN    0x000b
#define CLR_LIGHTRED     0x000c
#define CLR_LIGHTMAGENTA 0x000d
#define CLR_YELLOW       0x000e
#define CLR_WHITE        0x000f

#define MAKECELL(attr, char) ((CELL)MAKEUSHORT(char, attr))
#define MAKEATTR(bg, fg) ((((BYTE)(bg)) << 4) | (((BYTE)(fg)) & 0x0f))
#define SETATTR(cell, attr)  ((cell) = ((cell) & 0x00ffu) | (attr << 8))
#define COPYATTR(cell1, cell2) ((cell1) = ((cell1) & 0x00ffu) | ((cell2) & 0xff00u))

  // ATTN: This is color scheme dependant: blink/bkgr bit ... revise later

#define MAKEINVERTEDCOLOR(clr) ((USHORT)(clr)<<4) | (((USHORT)(clr)>>4) & \
                                (USHORT)0x0F00)

/////////////////////////////////////////////////////////////////////////////
// Miscellaneous declarations                                              //
/////////////////////////////////////////////////////////////////////////////

  // Macro to calculate rectangle dimension

#define WIDTH(rc)  ((rc).xRight - (rc).xLeft + 1)
#define HEIGHT(rc) ((rc).yBottom - (rc).yTop + 1)

  // Macro to calculate rectangle center position

#define HCENTER(rc,cx) ((WIDTH(rc) - (cx)) / 2)
#define VCENTER(rc,cy) ((HEIGHT(rc) - (cy)) / 2)

  // Debugging stuff

#define ABORT(text) (AuxAbort((PSZ)__FILE__,__LINE__,(PSZ)text))

  VOID APIENTRY AuxAbort(PSZ pszFile, USHORT usLine, PSZ pszText);
  VOID AuxLog(PSZ pszFormat, ...);

  // Redefine the assertion macro

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef NDEBUG
#  define ASSERT(p) ((void)0)
#else
#  define ASSERT(p) ((p) ? (void)0 : (void) AuxLog(                     \
                    "Assertion failed: %s, file %s, line %d",           \
                    #p, __FILE__, __LINE__ ) )
#endif

  // Include all the memory and string management stuff

#include "dw-mem.h"
#include "dw-str.h"

#endif /* DW_ALL_DEFS */

/***************************************************************************
* End of DW-DEF.H                                                          *
****************************************************************************/
