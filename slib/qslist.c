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

#include "prog.h"

/*
    This is a quicksort routine to be used to sort linked-lists.
    Written in 1990 by Jonathan Guthrie and placed in the public domain
*/

void * _fast qsortl(void *list, void * (_stdc *getnext)(void *),
                    void (_stdc *setnext)(void *, void *),
                    int (_stdc *compare)(void *, void *))
{
  void *low_list, *high_list, *sorted, *pivot, *temp;

    /* Test for empty list. */

  if (list==NULL)
    return NULL;

  high_list = NULL;
  low_list = NULL;

  pivot = list;
  list = getnext(list);
  setnext(pivot, NULL);

  while (list)
  {
    temp = list;
    list = getnext(list);

    if ((*compare)(temp, pivot) < 0)
    {
      setnext(temp, low_list);
      low_list = temp;
    }
    else
    {
      setnext(temp, high_list);
      high_list = temp;
    }
  }

  setnext(pivot, qsortl(high_list, getnext, setnext, compare));

  if (low_list)
  {
    list = sorted = qsortl(low_list, getnext, setnext, compare);

    do
    {
        temp = list;
        list = getnext(list);
    } while(list);

    setnext(temp, pivot);
  }
  else sorted = pivot;

  return sorted;
} 
