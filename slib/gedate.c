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

int _fast GEdate(union stamp_combo *s1,union stamp_combo *s2)
{
  struct _stamp st1, st2;

  st1=s1->msg_st;
  st2=s2->msg_st;

  /* Compare the date of `s1' and `s2'.  Definitely the long way around    *
   * the barn, but portable.                                               */

  if (st1.date.yr >= st2.date.yr)
  {
    if (st1.date.yr > st2.date.yr)
      return TRUE;
    else if (st1.date.mo >= st2.date.mo)
    {
      if (st1.date.mo > st2.date.mo)
        return TRUE;
      else if (st1.date.da >= st2.date.da)
      {
        if (st1.date.da > st2.date.da)
          return TRUE;
        else if (st1.time.hh >= st2.time.hh)
        {
          if (st1.time.hh > st2.time.hh)
            return TRUE;
          else if (st1.time.mm >= st2.time.mm)
          {
            if (st1.time.mm > st2.time.mm)
              return TRUE;
            else if (st1.time.ss >= st2.time.ss)
              return TRUE;
          }
        }
      }
    }
  }

  return FALSE;
}



