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

/*# name=MS-Binary to IEEE conversion routines
*/

#include "prog.h"


/* See comments in IEEE2MSB.C */

int _fast msbin_to_ieee(void *source,void *dest)
{
  unsigned short second, first;
  unsigned char second_hi, second_lo;
  unsigned short *sourceint=source;
  unsigned short *destint=dest;

  /* Grab the input from the source */

  first=*sourceint++;
  second=*sourceint;

  /* Split into component parts */

  second_lo=(unsigned char)(second & 0xff);
  second_hi=(unsigned char)(second >> 8);

  if (second_hi <= 2)
  { 
    /* This means a value of zero again */
    second_hi=second_lo=0;
    first=0;
  }
  else
  {
    second_hi += 0xfe;
    second_lo <<= 1;
  }

  /* Recombine the parts */

  second=second_lo | (second_hi << 8);

  second >>= 1;

  /* And store the output */

  *destint++=first;
  *destint=second;

  return 0;
}

