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

/*# name=Compare one string to many (case-insensitive)
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "fctype.h"

#define MAX_SEARCH 16

int _fast stricmpm(char *string,char *search[],int stopbit)
{
  int x;
  int max_keys;
  int last_found[MAX_SEARCH];
  int strlen_search[MAX_SEARCH];
  int ret_val=0;
  char lower_string;

  for (max_keys=0;(search[max_keys] != NULL) && (max_keys < MAX_SEARCH);
       max_keys++)
  {
    strlen_search[max_keys]=strlen(search[max_keys]);
    last_found[max_keys]=0;
  }

  if (string)
  {
    while (*string)
    {
      lower_string=f_tolwr(*string);

      for (x=0;x < max_keys;x++)
      {
        if ((last_found[x] != -1) && (lower_string==f_tolwr(search[x][last_found[x]]))) last_found[x]++;
        else last_found[x]=-1;

        if (last_found[x]==strlen_search[x])
        {
          last_found[x]=0;
          ret_val |= 1 << x;

          if (stopbit & (1 << x)) return(ret_val);
        }
      }

      string++;
    }
  }

  return(ret_val);
}

