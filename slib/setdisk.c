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

#ifdef OS_2
#define INCL_DOS
#include <pos2.h>
#endif

#if defined(OS_2)

int _fast set_disk(int drive)
{
  #ifdef __FLAT__
    DosSetDefaultDisk(drive+1);
  #else
    DosSelectDisk(drive+1);  /* in OS/2, drive 1=A, 2=B etc */
  #endif
    return 26;
}

#elif defined(__MSDOS__)

int _fast set_disk(int drive)
{
    union REGS regs;

    regs.h.ah = 0x0E;     /* dos select disk */
    regs.h.dl = (unsigned char) drive;  /* 0=A, 1=B etc */
    intdos(&regs, &regs);
    return (int)regs.h.al;
}

#elif defined(NT)

#include "pwin.h"

int _fast set_disk(int drive)
{
    char szPath[10];

    if (drive != -1)
    {
      sprintf(szPath, "%c:.", drive+'A');

      if (!SetCurrentDirectory(szPath))
        printf("@\aSetCurrentDirectory(%s) returned FALSE!\n", szPath);
    }

    return 26;
}

#else
  #error Unknown OS
#endif
