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

/*# name=Search for a specific string inside array of memory.
*/

#include <stdio.h>
#include "prog.h"
#include "fctype.h"

char * _fast memstr(char *string,char *search,unsigned lenstring,unsigned strlen_search)
{
  unsigned x;
  unsigned last_found;

  for (x=last_found=0;x < lenstring;x++)
  {
    if (string[x]==search[last_found])
      last_found++;
    else
    {
      if (last_found != 0)
      {
        last_found=0;
        x--;
        continue;
      }
    }

    if (last_found==strlen_search)
      return ((string+x)-strlen_search)+1;
  }

  return(NULL);
}

