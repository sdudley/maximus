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

/* NOTE!!!
 *
 * This header file is ONLY to be included by parts of Maximus itself,
 * namely MAX.EXE, MAXP.EXE, and so on.  Other applications which are
 * not part of the main executable should instead include max.h.
 */

#ifndef __MM_H_DEFINED
#define __MM_H_DEFINED

  #define MAXIMUS
  #define MAX200
  #define MAXVER 0x0300
  #define MAX_EXE         /* we are compiling max.exe */

  #define MAX_INCL_VARS
  #define MAX_INCL_VER
  #define MAX_INCL_PROTO
  #define MAX_INCL_LANGUAGE
  #define MAX_INCL_LANGLTH

  #define MAX_LANG_global
  #define MAX_LANG_sysop

  #ifndef __MAX_H_DEFINED
  #include "max.h"
  #endif
#endif

/*  Language file sections #includable by other modules:

#define MAX_LANG_max_init
#define MAX_LANG_max_log
#define MAX_LANG_f_area
#define MAX_LANG_max_bor
#define MAX_LANG_m_area
#define MAX_LANG_max_main
#define MAX_LANG_max_chng
#define MAX_LANG_max_chat
#define MAX_LANG_max_ued
#define MAX_LANG_m_browse
#define MAX_LANG_max_wfc
#define MAX_LANG_protocols

*/

