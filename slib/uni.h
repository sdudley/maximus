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

/* include file to be used wherever io.h or share.h would normally          *
 * be used.                                                                 */

/* wes -- I'm pulling this wierd override. My code has at least SOME file locking semantics.. */
#if 0
#if defined(__POSIX__) || defined(__GNUC__)
  #include <unistd.h>

  #define sopen(name, mode, share, imode) open(name, mode, imode)

  #define SH_DENYNO  0
  #define SH_DENYRW  0
  #define SH_DENYRD  0
  #define SH_DENYWR  0
  #define SH_DENYALL 0
  #define SH_COMPAT  0

  #define O_NOINHERIT 0

#else
  #include <io.h>
  #include <share.h>
#endif
#endif

#include <io.h>
#include <share.h>

