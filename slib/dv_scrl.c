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

/*# name=Direct-video routines (scroll region)
*/

#include <dos.h>
#include "prog.h"
#include "dv.h"

#ifdef __MSDOS__

void far pascal RegScrollUp(int *col,int *row)
{
  NW(col);
  RegScroll(1);

  *row=Vid_NumRows-1;
}

void _fast RegScroll(int lines)
{
  union REGS r;

  r.h.ah=6;                             /* Scroll up */
  r.h.al=(byte)lines;                   /* Lines to scroll */
  r.h.ch=0;                             /* Upper row */
  r.h.cl=0;                             /* Upper column */
  r.h.dh=(byte)(Vid_NumRows-1);         /* Bottom row */
  r.h.dl=(byte)(Vid_NumCols-1);         /* Bottom column */
  r.h.bh=Vid_Attribute;                 /* BH */

#ifdef __386__
  int386(0x10, &r, &r);
#else
  int86(0x10, &r, &r);
#endif
}


#endif /* __MSDOS__ */

