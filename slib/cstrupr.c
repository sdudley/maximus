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

/*# name=Convert characters to uppercase (with Chinese additions)
*/

#include <ctype.h>
#include <string.h>
#include "prog.h"

char * _fast cstrupr(char *s)
{
  char *orig=s;
  
  for (; *s; s++)
  {
    if (ischin(s))
      s++;
    else *s=(char)toupper(*s);
  }
  
  return orig;
}


char * _fast cstrlwr(char *s)
{
  char *orig=s;
  
  for (; *s; s++)
  {
    if (ischin(s))
      s++;
    else *s=(char)tolower(*s);
  }
  
  return orig;
}

char * _stdc cfancy_str(char *str)
{
  char *s;
  int lower=FALSE;

  s=str;

  if (s)
  {
    while (*s)
    {
      if (ischin(s))
      {
        s += 2;
        lower=TRUE;
      }
      else
      {
        *s=(char)(lower ? tolower(*s) : toupper(*s));
        lower=isalnum(*s++);
      }
    }
  }

  return str;
}

