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

/*# name=Binary string-search routine
    name=
    name=A binary search-n-compare routine.  Array of pointers in
    name='base[]' must be in alphabetical order, and the string must
    name=have identical case.  sbsearch() returns an index into
    name=the array of pointers 'base[]'.
*/

#include <stdio.h>
#include "prog.h"

#define LESS -1
#define EQUAL 0
#define MORE 1

int _fast sbsearch(char *key,char *base[],unsigned int num)
{
  int x, lastx=-1, lasthi, lastlo;

  char *s, *t;

  lasthi=num;
  lastlo=0;

  for (;;)
  {
    x=((lasthi-lastlo) >> 1)+lastlo;

    if (lastx==x)
      return -1;

    lastx=x;

    for (s=key,t=base[x];*s==*t;s++,t++)
      if (! *s)
        return (x);                       /* Found a match */

    if (*s > *t)
      lastlo=x;
    else
      lasthi=x;
  }
}

