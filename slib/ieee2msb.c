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

/*# name=IEEE to MS-Binary conversion routines
*/

#include "prog.h"


/* Convert an IEEE number to MicroSlush Binary format.  This is a really     *
 * funky format, wherein the number is made up of                            *
 * 0.1*fraction*2^(exp-128)*-1^(sign bit).  Really yucky, IMHO.  The         *
 * input and output of the function is done through 'void' pointers, which   *
 * makes it possible to pass something other than a float-type argument      *
 * (such as an object of equal size, such as a long integer), which can      *
 * prevent the floating-point code from being linked in.  See comments at    *
 * top of file about the same.                                               */

int _fast ieee_to_msbin(void *source,void *dest)
{
  unsigned int second, first;
  #if 0
  unsigned char second_high, second_lo;
  #endif
  unsigned short *sourceint=(unsigned short *)source;
  unsigned short *destint=(unsigned short *)dest;


  first=*sourceint++;

  second=*sourceint;


#if 0
  /* Split into high/low bytes */

  second_high=(unsigned char)(second >> 8);
  second_lo=(unsigned char)(second & 0xff);

  /* Mask off the upper bit */

  second_high &= 0x7f;

  /* Now combine the two again... */

  second=(second_high << 8) | second_lo;
#else
  second &= 0x7fff;
#endif

  /* Err! */

  if (second==0x7f00)
    return 1;

  if ((second & 0x7f80)==0)   /* This means a value of zero */
    first=second=0;
  else
  {
    second += 0x0100;
    second += second;

#if 0
    /* Split into component parts again */

    second_high=(unsigned char)(second >> 8);
    second_lo=(unsigned char)(second & 0xff);

    second_lo >>= 1;

    /* Re-combine */

    second=(second_high << 8) | second_lo;
#else
    second=((second & 0xff00) | ((second & 0xff) >> 1));
#endif
  }

  /* Now write the output dword */

  *destint++=first;
  *destint=second;
  return 0;
}


