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

#ifndef __POS2_H_DEFINED
#define __POS2_H_DEFINED

#ifndef UNIX
#ifdef __FLAT__
/*#pragma pack(4)*/
#pragma pack(1)
#else
#pragma pack(1)
#endif

#ifdef __FLAT__
  #define OS2UINT  ULONG
  #define DosTransactNmPipe DosTransactNPipe
  #define DosPeekNmPipe DosPeekNPipe
  #define DosSetNmPHandState DosSetNPHState

  #ifdef NULL
    #undef NULL
  #endif
#else
  #define OS2UINT  USHORT
#ifndef __WATCOMC__
  #define APIENTRY16 APIENTRY
#endif
#endif

#include "pm32.h"
#include <os2.h>
#pragma pack()

#undef ADDRESS

#ifdef __WATCOMC__
  #ifdef __FLAT__
    #pragma library("os2386.lib");
  #else
    #pragma library("os2.lib");
  #endif
#endif
#endif /* ! UNIX */

#endif /* __POS2_H_DEFINED */

