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

/* name=Function to dynamically change the size of a file
*/

#include <dos.h>
#include "prog.h"

#if defined(__MSDOS__)

  #include <stdio.h>
  #include <stdlib.h>
  #include <io.h>
  #include <fcntl.h>

  int _fast setfsize(int fd, long size)
  {
    union REGS r;
    long pos=tell(fd);

    lseek(fd, size, SEEK_SET);

  #ifdef __386__
    r.h.ah=0x40;
    r.x.ebx=fd;
    r.x.ecx=0;
    r.x.edx=0;

    int386(0x21, &r, &r);
  #else
    r.h.ah=0x40;
    r.x.bx=fd;
    r.x.cx=0;
    r.x.dx=0;

    int86(0x21, &r, &r);
  #endif

    lseek(fd, pos, SEEK_SET);

    return 0;
  }
#elif defined(OS_2)

  #define INCL_DOSFILEMGR
  #include <os2.h>

  int _fast setfsize(int fd, long size)
  {
    return ((int)DosNewSize((HFILE)fd, (ULONG)size));
  }

#elif defined(NT)

  #include "pwin.h"

  int _fast setfsize(int fd, long size)
  {
    SetFilePointer((HANDLE)fd, size, NULL, FILE_BEGIN);
    return (!SetEndOfFile((HANDLE)fd));
  }
#elif defined(UNIX)
int setfsize(int fd, long size)
{
  off_t off;

  if (!size)
    return 1;

  /* I'm guessing this extends, but doesn't truncate the file -- wes */
  off = lseek(fd, size - 1, SEEK_SET);
  if (off < 0)
    return 1;

  /* Force a one-byte write of nothing to insure file gets extended */    
  write(fd, "", 1);
  return 0;
}
#else
  #error Unknown OS
#endif







