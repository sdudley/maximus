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

/*# name=IEEE to long integer conversion routines (integer only)
*/

#include "prog.h"

/* Convert a single-precision IEEE number (ie. a 'float') to a long         *
 * integer.  This function only works on whole numbers, and positive        *
 * whole numbers, at that.  This was written as an alternative to           *
 * doing a "long_var=(long)float_var", so that we don't have to link        *
 * in the floating-point libraries...                                       *
 *                                                                          *
 * The arguments for both this and long_to_ieee() are all passed as         *
 * 'unsigned long's, so that the floating point code isn't linked in at     *
 * all.  If you have a float-type variable that you wish to convert, then   *
 * you've got the FP stuff in there anyway, so just use a typecast!         */

unsigned long _fast ieee_to_long(unsigned long f)
{
  unsigned long l;
  unsigned char exp;
  signed short bt;

  /* Special cases for zero and one */

  if ((f & 0xff000000L)==0)
    return 0L;
  else if ((f & 0x3f800000L)==0x3f800000L)
    return 1L;

  /* Exponent is extracted by using the bit mask 0x3f8..., and then         *
   * shifted to the far right so we cn use it as an integer.                */

  exp=(unsigned char)(((f & 0x3f800000L) >> 23)+1);

  /* We start with 2^exponent, and add to it later */

  l=(1L << exp);

  /* Exponent takes up bits 30-23.  (bit 31 is the sign bit) */

  bt=22;

  while (bt >= 0)
  {
    /* Scan each bit, and if it's set, add the appropriate number to our    *
     * value.                                                               */

    if (f & (1L << bt))
      l += 1L << (exp-(22-bt)-1);

    bt--;
  }

  return l;
}

#ifdef TEST_HARNESS

/* When compiling test harness, make sure that FP is linked in! */

main()
{
  float f;
  long l;
  long f2;

  for (f=0;f < 130000.0;f++)
  {
    l=ieee_to_long(*(long *)&f);
    f2=long_to_ieee(l);

    printf("f=%f, ieee_to_long(f)=%ld long_to_ieee()=%f\n",
           f,
           l,
           *(float *)&f2);

    if ((long)f != l || f != *(float *)&f2 /* || l != (long)(*(float *)&f2)*/)
    {
      fprintf(stderr,"\aErr at %f!\n",f);
      exit(1);
    }
  }

  return 0;
}

#endif


