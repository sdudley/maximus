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

/*# name=Regular expression compiler.  See also STRSTRX.C
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "alc.h"
#include "rep.h"

int _fast Compile_REP(char *exp,struct _rep *rep)
{
  char found,
       neg,
       start,
       end;

  int ch,
      x;

  for (ch=0;*exp && ch < MAX_REP;ch++)
  {
    rep->table[ch]=NULL;
    rep->type[ch]=0;

    switch(*exp)
    {
      case '^':
        rep->type[ch] |= CHAR_SOL;
        break;

      case '$':
        rep->type[ch] |= CHAR_EOL;
        break;

      case '.':
        rep->type[ch] |= CHAR_ANY;
        break;

      case '*':
        if (ch)
        {
          if (rep->type[ch-1] & (CHAR_ZMORE | CHAR_1MORE))
            return CERR_2R;

          rep->type[ch-1] |= CHAR_ZMORE;
        }
        else return CERR_NOREPEAT;

        ch--;
        break;

      case '+':
        if (ch)
        {
          if (rep->type[ch-1] & (CHAR_ZMORE | CHAR_1MORE))
            return CERR_2R;

          rep->type[ch-1] |= CHAR_1MORE;
        }
        else return CERR_NOREPEAT;

        ch--;
        break;

      case '[':   /* Start bracket search! */
        rep->type[ch] |= CHAR_TABLE;

        /* Allocate and set table to FALSE */
        if ((rep->table[ch]=(char *)malloc(TABLE_LEN))==NULL)
          return CERR_NOMEM;

        memset(rep->table[ch],'\x00',TABLE_LEN);

        if (*++exp=='^')
        {
          neg=TRUE;
          exp++;
        }
        else neg=FALSE;

        if (*exp==']')
          return CERR_EMPTYSET;

        found=FALSE;

        while (*exp)
        {
          if (*exp==']' && ((*(exp-1) != QUOTE) || (*(exp-2) == QUOTE)))
          {
            found=TRUE;
            break;
          }

          switch(*exp)
          {
            case '^':
              rep->type[ch] |= CHAR_SOL;
              break;

            case '$':
              rep->type[ch] |= CHAR_EOL;
              break;

            case QUOTE:
              if (! *++exp) /* If the next char is the end of string! */
                break;
              /* else fall-through */

            default:
              if (*(exp+1)=='-')
              {
                start=*exp;
                end=*(exp+=2);

                if (start < end)
                {
                  for (x=start;x <= end;x++)
                    rep->table[ch][x]=TRUE;
                }
                else
                {
                  for (x=end;x >= start;x--)
                    rep->table[ch][x]=TRUE;
                }
              }
              else
                rep->table[ch][*exp]=TRUE;
              break;
          }

          exp++;
        }

        if (! found)
          return CERR_NOTERM;

        if (neg)
          for (x=0;x < TABLE_LEN;x++)
            rep->table[ch][x]=(char)((rep->table[ch][x]==FALSE) ? TRUE : FALSE);
        break;

      case QUOTE:
        if (! *++exp) /* If the next char is the end of string! */
          return CERR_NOQUOTE;
        /* else fall-through */

      default:
        rep->type[ch] |= CHAR_NORM;
        rep->c[ch]=*exp;
        break;
    }

    exp++;
  }

  rep->max_ch=ch;

  if (ch==MAX_REP)
    return -1;
  else return 0;
}

