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

/*# name=Area-name kludge routine for Maximus' usr.msg/usr.files vars
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"

/* Converts the area number 'area' into a string that can be read by       *
 * humans.                                                                 */

char * _fast Area_Name(int area)
{
  static char aname[3];

  aname[0]=(char)(area >> CHAR_BITS);
  aname[1]=(char)(area & 0x00ff);
  aname[2]='\0';

  if (aname[0]=='0' && aname[1])
  {
    aname[0]=aname[1];
    aname[1]='\0';
  }

  return strupr(aname);
}


/* Converts the string 'aname' into a number that is used by Maximus       *
 * internal functions.                                                     */


int _fast Set_Area_Name(char *aname)
{
  int area;

  strupr(aname);

  if ((aname[0] >= 'A' && aname[0] <= 'Z') ||
      (aname[0] >= '0' && aname[0] <= '9') )
  {
    area=aname[0] << CHAR_BITS;

    if ((aname[1] >= 'A' && aname[1] <= 'Z') ||
        (aname[1] >= '0' && aname[1] <= '9') )
      area |= aname[1];
    else area=('0' << CHAR_BITS) | aname[0];
  }
  else area=('0' << CHAR_BITS) | '0';

  return area;
}



