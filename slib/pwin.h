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

#ifndef __PWIN_H_DEFINED
#define __PWIN_H_DEFINED

#ifdef NT
  #define WIN32_LEAN_AND_MEAN
  #define _INC_DDEMLH
/*  #define NOGDI*/
  #define NOUSER
  #define NOMM
  #define NOREG
  #define NONETWK
  #define NOCDERR
  #define NOCOMMDLG
  #define NODDE
  #define NODLGS
  #define NODRIVINIT
  #define NOLZEXPAND
  #define NONB30
  #define NOOLE
  #define NORPC
  #define NOSHELLAPI
  #define NOPERF
  #define NOSOCK
  #define NOSPOOL
  #define NOSERVICE

  #ifndef YESWINERROR
    #define NOWINERROR
  #endif

  #define NOATOM
  #define NOCLIPBOARD
  #define NOCOLOR
  #define NOSOUND
  #define NOPROFILER
  #include <windows.h>

  /* !@#$!@$# windows header tries to redefine "pascal" and gets
   * it wrong.  (It tries to change it to "__stdcall"!)
   */

  #undef pascal
  #define pascal __pascal
  
  #undef cdecl
  #define cdecl __cdecl

  #pragma pack()
#endif

#endif /* __PWIN_H_DEFINED */

