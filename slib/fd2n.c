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

/* Wrapper to convert normal file-handle I/O to new-style I/O */

#include "nopen.h"

char __fd2n;

#if !defined(NT) && !defined(UNIX)

  #ifdef __TOPAZ__  /* BC++/2 does things a bit differently */
    #define IOTYPE    _RTLENTRYF _EXPFUNC
  #else
    #define IOTYPE    _stdc
  #endif

  /* Add a "extern char __fd2n" to cause your app to be linked with these     *
   * routines.                                                                */

  int _stdc sopen(const char *name, int mode, int shacc, ...)
  {
    return nsopen(name, mode, shacc, S_IREAD | S_IWRITE);
  }

  int _stdc open(const char *name, int mode, ...)
  {
    return nopen(name, mode, S_IREAD | S_IWRITE);
  }

  #if defined(__TOPAZ__) || defined(__BORLANDC__)
  int IOTYPE read(int fd, void *buf, unsigned len)
  #else
  int IOTYPE read(int fd, char *buf, unsigned len)
  #endif
  {
    return nread(fd, buf, len);
  }

  #if defined(__TOPAZ__) || defined(__BORLANDC__)
  int IOTYPE write(int fd, const void *buf, unsigned len)
  #else
  int IOTYPE write(int fd, char *buf, unsigned len)
  #endif
  {
    return nwrite(fd, buf, len);
  }

  long IOTYPE lseek(int fd, long ofs, int pos)
  {
    return nlseek(fd, ofs, pos);
  }

  long IOTYPE tell(int fd)
  {
    return ntell(fd);
  }

  int IOTYPE close(int fd)
  {
    return nclose(fd);
  }

  int IOTYPE dup(int fd)
  {
    return ndup(fd);
  }

  int IOTYPE dup2(int fd1, int fd2)
  {
    return ndup2(fd1, fd2);
  }
#endif

