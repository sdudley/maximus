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

/*# name=Long integer to IEEE conversion routines (integer only)
*/

#include "prog.h"

/* Convert a long integer to an IEEE-format single-precision float.         *
 * See comments in IEEELONG.C.                                              */

unsigned long _fast long_to_ieee(unsigned long l)
{
  unsigned long origtest, test, f;

  /* Special cases for zero and one */

  if (l==0)
    return 0;
  else if (l==1)
    return 0x3f800000L;

  /* Scan the number, to find the greatest divider which is a power of 2 */

  for (test=31L; ; test--)
  {
    if (l >= (unsigned long)(1L << test))
    {
      /* Got it. */

      test--;

      /* Set up the starting value, adding in the exponent */

      f=(0x40000000L | (test << 23));

      /* Now set the appropriate bits, based on powers of two in our long. */
      for (origtest=test; ; test--)
      {
        if (l & (1L << test))
          f |= (1L << (22-(origtest-test)));

        /* We've looped through all the bits, so we can quit */

        if (test==0)
          break;
      }

      return f;
    }

    if (test==0)
      break;
  }

  return 0L;
}


