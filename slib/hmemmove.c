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

#include <stdio.h>
#include "prog.h"

#ifdef OS_2
#define INCL_DOS
#include "pos2.h"
#endif

/* Perform a memmove() with an object larger than 64K */

#ifdef __FLAT__

#include <string.h>

void _fast h_memmove(void huge *to, void huge *from, long size)
{
  memmove(to, from, size);
}

#else

/* Do a memmove on a segmented memory architecture */

void _fast h_memmove(void huge *vto, void huge *vfrom, long size)
{
  char huge *to=vto;
  char huge *from=vfrom;

  while (size > 0)
  {
    unsigned long ulMaxTo=65536L - (word)(char near *)to;
    unsigned long ulMaxFrom=65536L - (word)(char near *)from;
    unsigned long ulTransfer=(unsigned long)size;

    if (!ulMaxTo || !ulMaxFrom)
    {
#ifdef OS_2
      DosBeep(500, 100);
      DosBeep(2500, 100);
      DosBeep(2000, 100);
      DosBeep(2000, 100);
      DosBeep(2000, 100);
#endif
      return;
    }

    ulTransfer=min(ulTransfer, ulMaxTo);
    ulTransfer=min(ulTransfer, ulMaxFrom);

    f_memmove(to, from, (word)ulTransfer);

    to += ulTransfer;
    from += ulTransfer;
    size -= ulTransfer;
  }
}

#endif

