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
#include "prog.h"

/* Open a file - CREATE only, and fail if file already exists */

#if defined(__MSDOS__)

  int _fast cshopen(const char *path, int access)
  {
    union REGS r;
    int ret;

    NW(access);

    r.h.ah=0x5b;  /* Create new file */
    r.h.al=0xc2;  /* 1 100 0 010 */
                  /*         ^^^ read/write access                *
                   *       ^     reserved, set to zero            *
                   *   ^^^       SH_DENYNONE                      *
                   * ^           Not inherited by child processes */


    #ifdef __386__
      r.w.cx=0x00;  /* Normal access mode */
      r.x.edx=(unsigned)path;
      ret=int386(0x21, &r, &r);
    #else
    {
      struct SREGS s;
      r.x.cx=0x00;  /* Normal access mode */

      s.ds=FP_SEG(path);
      r.x.dx=FP_OFF(path);

      ret=int86x(0x21, &r, &r, &s);
    }
    #endif


    return (r.x.cflag ? -1 : ret);
  }

#elif defined(OS_2)

  #include <os2.h>

  /*
  #ifndef OS2DEF_INCLUDED
  #include <os2def.h>
  #include <bsedos.h>
  #endif
  */

  int _fast cshopen(const char *path, int access)
  {
      HFILE hf;
  #ifdef __FLAT__
      ULONG rc, usAction;
  #else
      USHORT rc, usAction;
  #endif

      NW(access);
      rc = DosOpen((PSZ)path, &hf, &usAction, 0L, FILE_NORMAL, FILE_CREATE,
              OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE |
              OPEN_FLAGS_NOINHERIT, 0L);
      return( rc? -1 : hf );
  }
#elif defined(NT)
  #include <io.h>
  #include <fcntl.h>
  #include <sys/types.h>
  #include <sys/stat.h>
  #include <share.h>

  int _fast cshopen(const char *path, int access)
  {
    return sopen(path, access | O_EXCL, SH_DENYNO, S_IREAD | S_IWRITE);
  }
#else
  #error unknown operating system. (posix: try "open(path, O_EXCL|access)")
#endif


