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
/*            DW-text: Standard SAA types declarations                     */
/*                     Largely derived from OS/2 headers                   */
/*                                                                         */
/*            Created: 16/Jan/90                                           */
/*            Updated: 19/Jul/97                                           */
/*                                                                         */
/*            Copyright (C) 1990-1997 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#ifndef DW_SAA_DEFS
#define DW_SAA_DEFS

/////////////////////////////////////////////////////////////////////////////
// C o m p i l e r   d e p e n d a n t   d e c l a r a t i o n s           //
/////////////////////////////////////////////////////////////////////////////
//  __BORLANDC__  Borland C++ 2.x and later                                //
//  _MSC_VER      Microsoft C 6.0 and later                                //
//  __WATCOMC__   Watcom C++ 10.0 and later                                //
//  __ZTC__       Zortech C++ 3.x and later                                //
/////////////////////////////////////////////////////////////////////////////

#if defined(__BORLANDC__)          /* Borland C++ 2.x and later */
    #define __FAR     far
    #define __NEAR    near
    #define __EXPORT _export
    #define __PASCAL  pascal
#elif defined(_MSC_VER)            /* Microsoft C 6.0 and later */
    #define __FAR    _far
    #define __NEAR   _near
    #define __EXPORT _export
    #if (_MSC_VER > 800) || defined(_STDCALL_SUPPORTED)
        #define __PASCAL __stdcall
    #else
        #define __PASCAL _pascal
    #endif
#elif defined(__WATCOMC__)         /* Watcom C++ 10.0 and later */
    #define __FAR    __far
    #define __NEAR   __near
    #define __EXPORT __export
    #define __PASCAL __pascal
#elif defined(__ZTC__)             /* Zortech C++ 3.x */
    #define __FAR    _far
    #define __NEAR   _near
    #define __EXPORT _export
    #define __PASCAL _pascal
#elif defined(__GNUC__)
    #define __FAR
    #define __NEAR
    #define __EXPORT
    #define __PASCAL
    #define near
    #define pascal
#ifndef P_WAIT
    #define P_WAIT 100
#endif
#else
    #error Unsupported compiler
#endif

/////////////////////////////////////////////////////////////////////////////
// P l a t f o r m   d e p e n d a n t   d e c l a r a t i o n s           //
/////////////////////////////////////////////////////////////////////////////
//  __DOS__     Define if compiling for DOS (default)                      //
//  __W32__     Define if compiling for WIN32                              //
//  __OS2__     Define if compiling for OS/2                               //
//  __DPMI__    Define if compiling for DPMI                               //
//  __DOSOVL__  Define if compiling for with DOS overlay support           //
//  __FLAT__    Define if compiling for flat memory model                  //
/////////////////////////////////////////////////////////////////////////////

#if defined(__OS2__)
    #ifdef __DOSOVL__
        #error Overlays are not supported under OS2
    #endif

#elif defined(__W32__) || defined(WIN32)
    #ifdef __DOSOVL__
        #error Overlays are not supported under WIN32
    #endif
    #ifndef __W32__
        #define __W32__
    #endif
    #ifndef __FLAT__
        #define __FLAT__
    #endif

#else
    #ifndef __DOS__
        #define __DOS__
    #endif
#endif

  // Define pointer types

#ifndef FAR
    #ifndef __FLAT__
        #define FAR  __FAR
    #else
        #define FAR
    #endif
#endif

#ifndef NEAR
    #ifndef __FLAT__
        #define NEAR  __NEAR
    #else
        #define NEAR
    #endif
#endif

  // Define naming conventions

#define PASCAL __PASCAL

  // Define basic types

#define VOID    void
#define CHAR    char                /* ch  */
#define SHORT   short               /* s   */
#define LONG    long                /* l   */
#define INT     int                 /* i   */

  // Declare basic types

  // Since integrating with Maximus, use Max's def's....
  // #include <hntypes.h>

#ifndef __HNTYPES_H_DEFINED
  typedef unsigned char  UCHAR;     /* uch */
  typedef unsigned short USHORT;    /* us  */
  typedef unsigned long  ULONG;     /* ul  */
  typedef unsigned int   UINT;      /* ui  */
  
  typedef unsigned char  BYTE;      /* b   */
  typedef unsigned short WORD;      /* w   */
  typedef unsigned long  DWORD;     /* dw  */

  typedef BYTE   FAR * PBYTE;
  typedef SHORT  FAR * PSHORT;
  typedef LONG   FAR * PLONG;
  typedef UCHAR  FAR * PUCHAR;
  typedef USHORT FAR * PUSHORT;
  typedef ULONG  FAR * PULONG;
  typedef VOID   FAR * PVOID;
  typedef unsigned short BOOL;      /* f   */
  typedef BOOL FAR * PBOOL;
#endif
  
  typedef unsigned char FAR  * PSZ;
  typedef unsigned char FAR  * PCH;
  typedef CHAR   FAR * PCHAR;

  typedef int (PASCAL FAR * PFN2)(); /* pfn */
  typedef PFN2 FAR * PPFN;

#ifndef OS2DEF_INCLUDED
  typedef int   HFILE;              /* hfile */
  typedef HFILE FAR * PHFILE;
#endif

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s  d e c l a r a t i o n s                      //
/////////////////////////////////////////////////////////////////////////////

#ifndef OS2DEF_INCLUDED

  // Cast any variable to an instance of the specified type

#define MAKETYPE(v, type)  (*((type FAR *)&v))

  // Make up a far pointer from segment/offset pair

#define MAKEP(seg, off) ((PVOID)MAKEULONG((off),(seg)))

  // Calculate the byte offset of a field in a structure of type type

#define FIELDOFFSET(type, field) ((SHORT)&(((type *)0)->field))

  // Combine l & h to form a 32 bit quantity

#define MAKEULONG(l, h) ((ULONG)(((USHORT)(l))|(((ULONG)((USHORT)(h)))<<16)))
#define MAKELONG(l, h)  ((LONG)MAKEULONG(l, h))

  // Combine l & h to form a 16 bit quantity

#define MAKEUSHORT(l, h) (((USHORT)(l)) | (((USHORT)(h)) << 8))
#define MAKESHORT(l, h)  ((SHORT)MAKEUSHORT(l, h))

  // Extract high and low order parts of 16 and 32 bit quantity

#define LOBYTE(w)       LOUCHAR(w)
#define HIBYTE(w)       HIUCHAR(w)
#define LOUCHAR(w)      ((UCHAR)(w))
#define HIUCHAR(w)      ((UCHAR)(((USHORT)(w) >> 8) & 0x0ff))
#define LOUSHORT(l)     ((USHORT)(l))
#define HIUSHORT(l)     ((USHORT)(((ULONG)(l) >> 16) & 0x0ffff))

#endif /* OS2DEF_INCLUDED */

#define LOWORD(l)       LOUSHORT(l)
#define HIWORD(l)       HIUSHORT(l)

  // Undefine buggy MSC far pointer manipulation

#if (_MSC_VER >= 600)
    #ifdef FP_OFF
        #undef FP_OFF
    #endif
    #ifdef FP_SEG
        #undef FP_SEG
    #endif
#endif

  // Define far pointer manipulation

#ifndef __FLAT__
    #ifndef FP_OFF
        #define FP_OFF(fp) ((unsigned)(fp))
    #endif
    #ifndef FP_SEG
        #define FP_SEG(fp) ((unsigned)((unsigned long)(fp) >> 16))
    #endif
    #ifndef MK_FP
        #define MK_FP(seg,off) ((void FAR *)(((unsigned long)(seg)<<16) | (unsigned)(off)))
    #endif
#endif

  // Define min/max/lim

#ifndef max
    #define max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
    #define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef lim
    #define lim(x,lo,hi) (min(max((x),(lo)),(hi)))
#endif

  // Define standard constants

#ifndef FALSE
    #define FALSE   0
#endif
#ifndef TRUE
    #define TRUE    1
#endif
#ifndef NULL
    #define NULL    0l
#endif

  // Declare size_t type

#if !defined(_SIZE_T) &&                /* Borland C */         \
    !defined(_SIZE_T_DEFINED) &&        /* Microsoft C */       \
    !defined(_SIZE_T_DEFINED_)          /* Watcom C */
    #define _SIZE_T
    #define _SIZE_T_DEFINED
    #define _SIZE_T_DEFINED_
  typedef unsigned int size_t;
#endif

  // Redefine the assertion macro

#ifdef ASSERT
#undef ASSERT
#endif

#ifdef NDEBUG
#  define ASSERT(p) ((void)0)
#else
#  define ASSERT(p) ((p) ? (void)0 : (void) fprintf(stderr,             \
                    "Assertion failed: %s, file %s, line %d\n\a",       \
                    #p, __FILE__, __LINE__ ) )
#endif

  // Shut the Borland compiler complaining about unused assertion vars

#if !defined(NDEBUG) && defined(__BORLANDC__)
#pragma warn -aus
#endif

  // Define useful macros

#define loop        for (;;)                    // Endless loop until break
#define numbof(a)  (sizeof(a)/sizeof(a[0]))     // Number of array elements
#define lengof(s)  (sizeof(s) - 1)              // Length of the const string

#define SETFLAG(f,b)     (f)|= (b)
#define RESETFLAG(f,b)   (f)&=~(b)
#define SETFLAGTO(f,b,c) if (c) SETFLAG(f,b); else RESETFLAG(f,b);

#endif  /* DW_SAA_DEFS */

/***************************************************************************
* End of DW-SAA.H file                                                     *
****************************************************************************/
