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

/*# name=Zeller's congruence algorithm.  Finds day-of-week for any date
    name=through 1999.
*/

#include "prog.h"

#define CENTURY 19

/*
main()
{
  printf("weekday of 05-21-90 is %s\n",weekday[zeller(5,21,90)]);
}
*/

int _fast zeller(int m,int d,int y)
{
  int f;

  if (m > 2)
    m -= 2;
  else
  {
    m += 10;
    y--;
  }

  f=(((13*m-1) / 5) + (y/4) + (CENTURY/4) + y + d - (2*CENTURY)) % 7;

  return f;
}

