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

#include <io.h>
#include <dos.h>
#include "prog.h"

#if defined(OS_2)

  #define INCL_NOPM
  #define INCL_DOS
  #include "pos2.h"

  void pascal far flush_handle2( int fh )
  {
  #ifdef __FLAT__
    DosResetBuffer((HFILE)fh);
  #else
    DosBufReset((HFILE)fh);
  #endif
  }

#elif defined(NT)
  #include "pwin.h"

  void pascal far flush_handle2( int fh )
  {
    FlushFileBuffers((HANDLE)fh);
  }

#elif !defined(__MSDOS__)  /* dos version defined in .asm file */
  #error Unknown OS
#endif


/* This makes sure that a file gets flushed to disk. */

void _fast flush_handle(FILE *fp)
{

  fflush(fp);

#if defined(OS_2) || defined(NT) || defined(MSDOS) || defined(__MSDOS__) || defined(__TURBOC__)
  flush_handle2(fileno(fp));
#else
  {
    int nfd;

    if ((nfd=dup(fileno(fp))) != -1)
      close(nfd);
  }
#endif
}


