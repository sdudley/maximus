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

#if defined(UNIX)
# include "compiler_unix.h"
#else
/*# name=Compiler-determination and memory-model-determination routines
    name=Support for WATCOM C (DOS, OS/2 and 386 flat), Microsoft C
    name=(all memory models, DOS & OS/2), Turbo C 2.0, Turbo/Borland C++
    name=and IBM C Set/2 (flat model).
*/

/* Non-DOS systems...  Just do a "#define __FARCODE__",                     *
 * "#define __FARDATA__" and "#define __LARGE__" in place of this file.     */

#ifndef __COMPILER_H_DEFINED
#define __COMPILER_H_DEFINED


#ifdef _lint  /* default to large segmented model when linting */
#define __LARGE__
#endif

/* Topaz - Borland C++ for OS/2 */

#if defined(__BORLANDC__) && defined(__OS2__)
  #define __TOPAZ__  __BORLANDC__
  #define __FLAT__
  #define __386__
#endif

#ifdef __GNUC__
  #define __FLAT__
  #define __386__
  #define __POSIX__
#endif

#ifdef __IBMC__ /* Set default library options for IBM's C Set/2 */
  #define __FLAT__
  #define __386__
#endif

#if defined(NT) && !defined(__FLAT__)
  #define __FLAT__
#endif

#if defined(__WATCOMC__) && defined(NT)
#if __WATCOMC__ >= 1100
#define NTstdcall __cdecl
#else
#define NTstdcall __stdcall
#endif
#endif

/* WATCOM includes both M_I86xxx and __modeltype__ macros */

#ifndef __WATCOMC__

  #if (!defined(_lint) && (defined(M_I86SM) || defined(M_I86MM) || \
                           defined(M_I86CM) || defined(M_I86LM) || \
                           defined(M_I86HM) || defined(_MSC_VER)))
    #define __MSC__
  #endif

  #ifdef M_I86SM
    #define __SMALL__
  #endif

  #ifdef M_I86MM
    #define __MEDIUM__
  #endif

  #ifdef M_I86CM
    #define __COMPACT__
  #endif

  #ifdef M_I86LM
    #define __LARGE__
  #endif

  #ifdef M_I86HM
    #define __HUGE__
  #endif

#endif /* ! __WATCOMC__ */




/* Handle 386 "flat" memory model, currently for IBM C Set/2 and WC386 */

#ifdef __FLAT__

  /* Other macros may get defined by braindead compilers */

  #ifdef __SMALL__
    #undef __SMALL_
  #endif

  #ifdef __TINY__
    #undef __TINY__
  #endif

  #ifdef __MEDIUM__
    #undef __MEDIUM__
  #endif

  #ifdef __COMPACT__
    #undef __COMPACT__
  #endif

  #ifdef __LARGE__
    #undef __LARGE__
  #endif

  #ifdef __HUGE__
    #undef __HUGE__
  #endif

  /* Code is really "near", but "far" in this context means that we want    *
   * a 32 bit ptr (vice 16 bit).                                            */

  #define __FARCODE__
  #define __FARDATA__

  /* Everything should be "near" in the flat model */

  #ifdef far
    #undef far
  #endif

  #ifdef near
    #undef near
  #endif

  #ifdef huge
    #undef huge
  #endif

  #define far
  #define near
  #define huge


  /* Since we're using flat model OS/2, we never need to load DS */

  #ifdef OS_2
    #ifdef _loadds
      #undef _loadds
    #endif

    #ifdef __loadds
      #undef __loadds
    #endif

    #define _loadds
    #define __loadds
  #endif

#else /* !FLAT */
  #define __far16 far
#endif


/* Now handle the segmented memory models */

#if defined(__SMALL__) || defined(__TINY__)
  #define __NEARCODE__
  #define __NEARDATA__
#endif

#ifdef __MEDIUM__
  #define __FARCODE__
  #define __NEARDATA__
#endif

#ifdef __COMPACT__
  #define __NEARCODE__
  #define __FARDATA__
#endif

#if defined(__LARGE__) || defined(__HUGE__)
  #define __FARCODE__
  #define __FARDATA__
#endif


/* Default to MS-DOS compile */

#if !defined(OS_2) && !defined(__MSDOS__) && !defined(NT) && !defined(UNIX)
  #define __MSDOS__
#endif


/* Compiler-specific stuff:                                                 *
 *                                                                          *
 *  _stdc - Standard calling sequence.  This should be the type of          *
 *          function required for function ptrs for qsort() et al.          *
 *                                                                          *
 *  _vstdc- Same as _stdc, but for variables.  In some compilers (WC, MSC), *
 *          this determines whether or not a function has a leading         *
 *          underscore.  Unfortunately, using _stdc is invalid when working *
 *          with IBM C Set/2.                                               *
 *                                                                          *
 *  _fast - Fastest calling sequence supported.  If the default             *
 *          calling sequence is the fastest, or if your compiler            *
 *          only has one, define this to nothing.                           *
 *                                                                          *
 *  _intr - For defining interrupt functions.  For some idiotic             *
 *          reason, MSC requires that interrupts be declared                *
 *          as "cdecl interrupt", instead of just "interrupt".              */


/****************************************************************************
                      Turbo C 2.0, Turbo C++ and Borland C++
 ****************************************************************************/

#if defined(__TURBOC__)

  #define _stdc     cdecl
  #define _vstdc    cdecl
  #define _intcast  void (_intr *)()
  #define _veccast  _intcast

  #ifdef __TOPAZ__
    #define _fast     __fastcall
    #define _intr     __interrupt __far
    #define _System   _syscall
  #else
    #define _fast     pascal
    #define _intr     interrupt far
  #endif

  #define _loadds

  #define NW(var) (void)var
  /* structs are packed in TC by default, accd to TURBOC.CFG */

/****************************************************************************
                             Microsoft C 6.0
 ****************************************************************************/

#elif defined(__MSC__)

  #define _stdc     cdecl
  #define _vstdc    cdecl
  #define _intr     cdecl interrupt far
  #define _intcast  void (_intr *)()
  #define _veccast  _intcast

  #if _MSC_VER >= 600
    #define _fast _fastcall
  #else
    #define _fast pascal
  #endif

  #pragma pack(1)                 /* Structures should NOT be padded */
  #define NW(var)  var = var      /* NW == No Warning */

/****************************************************************************
                             WATCOM C 8.0/8.5
 ****************************************************************************/

#elif defined(__WATCOMC__)

  #define _stdc
  #define _vstdc
  #define _intr     cdecl interrupt __far
  #define _intcast  void (_intr *)()
  #define _veccast  void (__interrupt __far *)()
  #define _fast

  #pragma pack(1)                 /* Structures should NOT be padded */
  #define NW(var)   (void)var


/****************************************************************************
                                IBM C Set/2
 ****************************************************************************/

#elif defined(__IBMC__)

#ifndef __MIG__
  #define cdecl     /*_Cdecl*/
  #define pascal    /*_Pascal*/
#endif

  #define _stdc     /*_Optlink*/
  #define _vstdc
  #define _intr     /*_System*/
  #define _intcast  void (_intr *)()
  #define _veccast  void (_intr *)()
  #define _fast

  #pragma pack(1)                 /* Structures should NOT be padded */
  #define NW(var)   (void)var

/****************************************************************************
                                GNU C/C++
 ****************************************************************************/

#elif defined(__GNUC__)
  #define _stdc
  #define _vstdc
  #define _intr
  #define _intcast
  #define _veccast
  #define _fast
  #define _loadds

  #define pascal
  #define cdecl
  #define far
  #define near

  #define NW(var) (void)var

/****************************************************************************
                                    LINT
 ****************************************************************************/

#elif defined(_lint)

  #define _stdc
  #define _vstdc
  #define _intr     cdecl interrupt __far
  #define _intcast  void (_intr *)()
  #define _veccast  void (__interrupt __far *)()
  #define _fast

  #define NW(var)   (void)var

#else
  #error Unknown compiler!

  #define _stdc
  #define _intr     interrupt
  #define _intcast  void (_intr *)()
  #define _veccast  intr
  #define _fast
  #define NW(var)   (void)var
  #define __MSDOS__
#endif

#ifndef __TYPEDEFS_H_DEFINED
#include "typedefs.h"
#endif

#endif /* ! __COMPILER_H_DEFINED */

#endif /* ! UNIX */
