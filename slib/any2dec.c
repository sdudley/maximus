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

/*# name=Converts a number of any base to decimal.
*/

#include <stdio.h>
#include <ctype.h>
#include "prog.h"

int _fast any2dec(char *str,unsigned int radix)                /* convert string to integer */
{
  word digit=0;
  word number=0;

  while (*str)
  {
    if (isdigit(*str))
      digit = *str - '0';
    else if (islower(digit))
      digit = *str - 'a' + 10;
    else
      digit = *str - 'A' + 10;

    number = number * radix + digit;
    str++;
  }

  return(number);
}

char * _fast dec2any(unsigned int number,unsigned int radix) /* convert integer to string */
{
  static char buffer[64];      /* allow for up to 64-bit integers */

  char *bufptr = buffer + 64;  /* point to end of buffer */

  int digit;

  *bufptr-- = '\0';

  do
  {
    digit = number % radix;
    number /= radix;
    *bufptr-- = (char)((digit < 10) ? (digit + '0') : (digit + 'A' - 10));
  }
  while (number);

  return(++bufptr);
}

