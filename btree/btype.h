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

/* $Id: btype.h,v 1.1.1.1 2002/10/01 17:49:23 sdudley Exp $ */

/* Common type definitions for B-trees.  These are used for both
 * C and C++ source files.
 */

#ifndef __BTYPE_H_DEFINED
#define __BTYPE_H_DEFINED

#include "compiler.h"

/* Error codes for error() and bteLastError */

typedef enum
{
  BTE_NONE            = 0,      /* no error                             */
  BTE_NOTOPEN         = 1,      /* file not open                        */
  BTE_CANTREAD        = 2,      /* error reading database               */
  BTE_CANTWRITE       = 3,      /* error writing database               */
  BTE_NOMEM           = 4,      /* not enough memory                    */
  BTE_DUPEKEY         = 5       /* attempt to insert duplicate key      */
} BTERROR;

typedef unsigned long NNUM;     /* Type used for holding a node number  */


// Actions for inserting a key that already exists.  The default
// action taken by insert() is to do nothing and return an error
// code.

#define IF_DUPE             0x0001    // If key of this name already
                                      // exists, insert a second using
                                      // the same key
#define IF_REPLACE          0x0002    // If key of this name already
                                      // exists, replace it.
#define IF_NORM             0x0004    // Only insert new keys (default)


#ifndef TRUE
  #define TRUE 1
  #define FALSE 0
#endif

#ifdef BEXPENTRY
  #undef BEXPENTRY
#endif


#if defined(OS_2)
  #ifdef __WATCOMC__
    #ifdef __FLAT__
      #define BEXPENTRY _System
    #else
      #define BEXPENTRY far pascal
    #endif
  #else
    #ifdef __FLAT__
      #define BEXPENTRY _System
    #else
      #ifdef __WATCOMC__
        #define BEXPENTRY far pascal __loadds
      #else
        #define BEXPENTRY far pascal _loadds
      #endif
    #endif
  #endif
#elif defined(NT)
  #define BEXPENTRY NTstdcall
#else
  #define BEXPENTRY
#endif


#ifdef __FLAT__
  #ifndef BAPIENTRY
    #if defined(OS_2)
      #define BAPIENTRY  _System
    #elif defined(NT)
      #define BAPIENTRY NTstdcall
    #else
      #define BAPIENTRY
    #endif
  #endif

  #define OS2FAR
#else
  #ifndef BAPIENTRY
    #define BAPIENTRY BEXPENTRY
  #endif

  #ifdef OS_2
    #define OS2FAR far
  #else
    #define OS2FAR
  #endif
#endif

/* All C++ class members must be exported by name only */

#if defined(OS_2)
  #ifdef __FLAT__
    #define CPPEXPORT __export
  #else
    #define CPPEXPORT far __export
  #endif
#elif defined(NT)
  #define CPPEXPORT __export
#else
  #define CPPEXPORT
#endif


/* Type of function which compares two key values */

#ifdef __MSC__
typedef int (_stdc OS2FAR * keycomp_t)(void OS2FAR *a1, void OS2FAR *a2);
#else
typedef int (OS2FAR * _stdc keycomp_t)(void OS2FAR *a1, void OS2FAR *a2);
#endif


/* Out-of-memory object */

#ifdef __cplusplus
  struct NoMem { NoMem(int) {} };
#endif

#endif /* __BTYPE_H_DEFINED */

