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
#include <ctype.h>
#include "prog.h"

#ifdef OS_2
  #define INCL_DOS
  #include <os2.h>
#endif

/*
 * The get_disk() routine uses system call 0x19 to obtain the
 * current disk drive. The current drive is returned as
 * 0 = drive A, 1 = drive B, and so on.
 * P. Fitzsimmons.
 */

#if defined(OS_2)

int _fast get_disk(void)
{
  #ifdef __FLAT__
    ULONG drive, asdf;

    DosQueryCurrentDisk(&drive, &asdf);
    return ((int)drive-1);
  #else
    unsigned short drive;
    unsigned long dont_care;

    DosQCurDisk(&drive, &dont_care);
    return ((int) drive-1);
  #endif
}

#elif defined(__MSDOS__)

int _fast get_disk(void)
{
    union REGS regs;

    regs.h.ah = 0x19;
    intdos(&regs, &regs);
    return ((int) regs.h.al );
}

#elif defined(NT)

#include "pwin.h"

int _fast get_disk(void)
{
    char szPath[PATHLEN];

    GetCurrentDirectory(PATHLEN, szPath);
    return tolower(*szPath)-'a';
}

#elif defined(UNIX)
/* wes - groan */
int get_disk(void)
{
  return 0; /* Assume "c" drive for whatever reason */
}

#else
  #error Unknown OS
#endif












