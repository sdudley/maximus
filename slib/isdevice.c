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
#include <string.h>
#include <io.h>
#include "prog.h"

#if defined(__MSDOS__)

  /* Mapping of device word:
                       6 5 4       3 2 1 0

     0 0 0 0 0 0 0 0 0 0 0 0       0 0 0 0
          reserved    \ \ \ \rsvd   \ \ \ \ if stdin
                       \ \ \raw      \ \ \if stdout
                        \ \eof        \ \if nul
                         \char device  \if clock

    Test for mask 0x47 for char|stdin|stdout|nul|clock
  */

  #define DEV_MASK 0x0047

  unsigned _fast is_device(int fd)
  {
    union REGS r;

    r.h.ah=0x44;  /* ioctl */
    r.h.al=0x00;  /* get device info */

    #ifdef __386__
      r.x.ebx=fd;

      int386(0x21, &r, &r);

      if (r.x.cflag)
        return 0;
      else return !!(r.x.edx & DEV_MASK);
    #else
      r.x.bx=fd;

      int86(0x21, &r, &r);

      if (r.x.cflag)
        return !!(r.x.dx & DEV_MASK);
      else return 0;
    #endif
  }
#elif defined(OS_2)
  #define INCL_DOS
  #include <pos2.h>

  unsigned _fast is_device(int fd)
  {
    OS2UINT usType;
    OS2UINT usDevAttr;

  #ifdef __FLAT__
    DosQueryHType((HFILE)fd, &usType, &usDevAttr);
  #else
    DosQHandType((HFILE)fd, &usType, &usDevAttr);
  #endif

    return ((usType & 0xff) != HANDTYPE_FILE);
  }
#elif defined(NT)

  #include "pwin.h"

  unsigned _fast is_device(int fd)
  {
    DWORD rc;

    rc=GetFileType((HANDLE)_os_handle(fd));

    return (rc != FILE_TYPE_DISK);
  }
#elif defined(UNIX)

  #include "uni.h"
  #include <sys/stat.h>

  unsigned _fast is_device(int fd)
  {
    struct stat st;

    if (fstat(fd, &st) != 0)
      return FALSE;

    return !!(st.st_mode & (S_IFIFO | S_IFCHR | S_IFBLK | S_IFSOCK));
  }

#else
  #error Unknown OS
#endif

#include "unistr.h"

/* Check for other special device names */

unsigned _fast is_devicename(char *filename)
{
  byte **pp;
  unsigned len;

  static byte *bad_names[]=
  { 
    "CON", "AUX", "PRN", "NUL", "LPT", "LPT1", "LPT2", "LPT3", "LPT4",
    "COM", "COM1", "COM2", "COM3", "COM4", "GATE1", "GATE2", "GATE3", 
    "GATE4", "CLOCK", "CLOCK$", "KBD$", "MOUSE$", "POINTER$", "QEMM386$",
    "SCREEN$", "..", NULL
  };

  for (pp=bad_names; *pp; pp++)
  {
    len=strlen(*pp);

    /* Filter out 'COM1' or even 'COM1.ABC' */

    if (eqstri(filename, *pp) ||
        (eqstrn(filename, *pp, len) && (*pp)[len]=='.'))
    {
      return TRUE;
    }
  }

  return FALSE;
}


