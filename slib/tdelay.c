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

#include <dos.h>

#define INCL_NOPM
#define INCL_DOS    /* must be before prog.h */

#include "prog.h"

#if defined(OS_2)

  #include "pos2.h"

  void _fast tdelay(int msecs)
  {
      DosSleep((ULONG)msecs);
  }

#elif defined(__MSDOS__)
  #include <time.h>

  void _fast tdelay(int msecs)
  {
    clock_t ctEnd;

    ctEnd = clock() + (long)msecs * (long)CLK_TCK / 1000L;

    while (clock() < ctEnd)
      ;
  }

#elif defined(NT)

  #include "pwin.h"

  void _fast tdelay(int msecs)
  {
    Sleep((DWORD)msecs);
  }

#else
  #error Unknown OS
#endif
