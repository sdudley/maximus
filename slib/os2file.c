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

/*# name=OS/2-specific routines
*/

#if defined(OS_2)

  #include <errno.h>
  #include "prog.h"

  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  int far pascal farread(int handle,char far *buf,unsigned int len)
  {
  #ifdef __FLAT__
    ULONG bytesRead;
  #else
    USHORT bytesRead;
  #endif

    if (DosRead(handle, buf, len, &bytesRead))
    {
      errno=EBADF;
      return -1;
    }
    else return (int)bytesRead;
  }

  int far pascal farwrite(int handle,char far *buf,unsigned int len)
  {
  #ifdef __FLAT__
    ULONG bytesWrite;
  #else
    USHORT bytesWrite;
  #endif

    if (DosWrite(handle, buf, len, &bytesWrite))
    {
      errno=EBADF;
      return -1;
    }
    else return (int)bytesWrite;
  }

#elif defined(NT)
  #include "prog.h"
  #include <io.h>
  #include <fcntl.h>

  int far pascal farread(int handle, char far *buf, unsigned int len)
  {
    return read(handle, buf, len);
  }

  int far pascal farwrite(int handle, char far *buf, unsigned int len)
  {
    return write(handle, buf, len);
  }
#else
  #error Unknown OS!
#endif

