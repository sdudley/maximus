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

/*# name=Case-insensitive search-for-str-inside-another routine
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"

/* Code to handle chinese characters */

#define ISLEFT(c) ((c) > (byte)0x80 && (c) < (byte)0xff)
#define ISRIGHT(c) (((c) >= (byte)0x40 && (c) <= (byte)0x7e) || \
                    ((c) >= (byte)0xa1 && (c) <= (byte)0xfe))

word _fast ischin(byte *buf)
{
  return (ISLEFT(buf[0]) && ISRIGHT(buf[1]));
}

char * _fast stristr(char *string,char *search)
{
  /* "register" keyword used to fix the brain-dead MSC (opti)mizer */

  word last_found=0;
  word strlen_search=strlen(search);
  byte l1, l2;
  word i;

  if (string)
  {
    while (*string)
    {
      /**** start chinese modifications *****/
      l1=(byte)(ischin(string) ? 2 : 1);
      l2=(byte)(ischin(search+last_found) ? 2 : 1);

      if (l1==l2)
        i=(l1==1) ? memicmp(string, search+last_found, l1) : 1;
      else i=1;
      
      if (!i)
        last_found += l1;
      /**** end chinese modifications *****/
      /* old code: if ((tolower(*string))==(tolower(search[last_found])))
                     last_found++;
      */
      else
      {
        if (last_found != 0)
        {
          string -= last_found-1;
          last_found=0;
          continue;
        }
      }

      string += l1;

      if (last_found==strlen_search) return(string-last_found);
    }
  }

  return(NULL);
}

