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

/*# name=Replacement functions for WATCOM C
*/

// This should only be used for WC!

#ifdef __WATCOMC__

#include <mem.h>
#include <dos.h>
#include <conio.h>
#include <errno.h>
#include "prog.h"
#include "alc.h"

int _fast wc_grow_handles(int n)
{
  return _grow_handles(n > 101 ? n : 101);
}

#if defined(__MSDOS__)
  int _stdc getcurdir(int drive, char *directory)
  {
    union REGS r;

    r.h.ah=0x47;
    r.h.dl=(byte)drive;

  #ifdef __FLAT__
    r.x.esi=FP_OFF(directory);
    int386(0x21, &r, &r);
  #else
    {
      struct SREGS sr;

      sr.ds= FP_SEG(directory);
      r.x.si=FP_OFF(directory);
      int86x(0x21, &r, &r, &sr);
    }
  #endif

    return (r.x.cflag==0 ? 0 : -1);
  }
#elif defined(OS_2)

  #define INCL_DOS
  #include <os2.h>

  int _stdc getcurdir(int drive, char *directory)
  {
  #ifdef __FLAT__
    ULONG buflen=PATHLEN;

    return (DosQueryCurrentDir((ULONG)drive, directory, &buflen)==0 ? 0 : -1);
  #else
    USHORT buflen=120;

    return (DosQCurDir(drive, directory, &buflen)==0 ? 0 : -1);
  #endif
  }

#elif defined(NT)

  #include "pwin.h"

  int _stdc getcurdir(int drive, char *directory)
  {
    int old_disk, rc;

    old_disk=getdisk();

    setdisk(drive);
    rc=GetCurrentDirectory(PATHLEN, directory);
    setdisk(old_disk);

    return (rc > 0 ? 0 : -1);
  }

#endif


#ifndef __FLAT__
  void far *farcalloc(int n,int m)
  {
    void far *p;

    p=malloc(n*m);

    if (!p)
      return NULL;

  #ifdef __FLAT__
    memset(p,'\0',m*n);
  #else
    _fmemset(p,'\0',m*n);
  #endif

    return p;
  }
#endif /* !__FLAT__ */

#endif /* __WATCOMC__ */
