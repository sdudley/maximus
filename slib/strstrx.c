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

/*# name=Case-insensitive, regular-expression pattern matching
    name=routine.  See also CREP.C.
*/

#define PRODUCTION
#define INSENSITIVE

#include <stdio.h>
#include <ctype.h>
#include "prog.h"
#include "rep.h"

/* If no match, check exp-- with last_found++ */

char * _fast stristrx(char *string,struct _rep *rep)
{
  static int last_found,
             match,
             madeone;

  static char *orig_str;

  static char rtlf;

  orig_str=string;
  last_found=0;

  while (*string)
  {
    madeone=FALSE;
    rtlf=rep->type[last_found];

    for (;;)
    {
      match=FALSE;

      if (rtlf & (CHAR_SOL | CHAR_EOL))
      {
        switch(*string)
        {
          case '\x00':
            if (rtlf & CHAR_SOL)
            {
              match=FALSE;
              break;
            }

            string--;
            /* fall-through! */

          case '\x0a':
          case '\x0d':
          case '\x8d':
            match=TRUE;

            while (*string=='\x0a' || *string=='\x0d' || *string=='\x8d')
              string++;

            string--;

            break;

          default:
            if (string==orig_str)
            {
              match=TRUE;
              string--;
            }
            else
              match=FALSE;
            break;
        }
      }

      switch(rtlf & (CHAR_NORM | CHAR_TABLE | CHAR_ANY))
      {
        case CHAR_NORM:
          if ((tolower(*string))==rep->c[last_found])
            match=TRUE;
          break;

        case CHAR_TABLE:
          if (rep->table[last_found][tolower(*string)])
            match=TRUE;
          break;

        case CHAR_ANY:
          match=TRUE;
          break;

        #ifndef PRODUCTION
        default:
          lputs("Bizarre error parsing compiled regular expression!");
          exit(ERROR_BIZARRE);
          break;
        #endif
      }


      if (rtlf & CHAR_ZMORE)
      {
        if (! match)
        {
          match=TRUE;

          if (last_found)
            string--;

          break;
        }

        string++;
      }
      else if (rtlf & CHAR_1MORE)
      {
        if (madeone)
        {
          if (! match)
          {
            match=TRUE;

            if (last_found)
              string--;

            break;
          }

          string++;
        }
        else
        {
          if (! match)
          {
            match=FALSE;
            break;
          }

          string++;
          madeone=TRUE;
        }
      }
      else break;
    }

    if (match)
      last_found++;
    else
    {
      if (last_found)
      {
        last_found=0;
        continue;
      }
    }

    string++;

    if (last_found==rep->max_ch)
      return(string-last_found);
  }

  /* Terminating NULL! */
  if ((rep->type[last_found] & (CHAR_SOL | CHAR_EOL)) && ++last_found==rep->max_ch)
    return(string-last_found);

  return(NULL);
}

