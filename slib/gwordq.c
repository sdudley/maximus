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

/*# name=getword() function which recognizes quotes
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"

/*#define REMOVE_QUOTES  Needed for TopX. */

int _fast getwordq(char *strng,char *dest,char *delim,char quote,int findword)
{
  int x,
      isw,
      sl_d,
      sl_s,
      wordno=0;

  char *string,
       *oldstring,
       *firstchar;

#ifdef REMOVE_QUOTES
  char *in,*out;
#endif

  string=oldstring=strng;

  sl_d=strlen(delim);

  for (string=strng;*string;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++)
      if (*string==delim[x]) isw=1;

    if (string != strng && *(string-1)==quote)
      isw=0;

    if (isw==0)
    {
      oldstring=string;
      break;
    }
  }

  sl_s=strlen(string);

  for (firstchar=NULL,wordno=0;(string-oldstring) <= sl_s;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++)
      if (*string==delim[x])
      {
        isw=1;
        break;
      }

    if (string != oldstring && *(string-1)==quote)
      isw=0;

    if ((! isw) && (string==oldstring)) wordno++;

    if (wordno != findword)
    {
      if (isw && (string != oldstring))
      {
        for (x=0,isw=0;x <= sl_d;x++) if (*(string+1)==delim[x])
        {
          isw=1;
          break;
        }

        if (string != oldstring && *(string-1)==quote)
          isw=0;

        if (isw==0) wordno++;
      }
    }
    else
    {
      if (isw && (string != oldstring))
      {
        for (x=0,isw=0;x <= sl_d;x++)
          if (*(string-1)==delim[x])
          {
            isw=1;
            break;
          }

          if (*(string-1)==quote)
            isw=0;

        if (isw==0) wordno++;
      }
    }

    if ((wordno==findword) && (firstchar==NULL)) firstchar=string;

    if (wordno==(findword+1))
    {
      strncpy(dest,(firstchar==oldstring ? firstchar : ++firstchar),
              (int)(string-firstchar));
      dest[(int)(string-firstchar)]='\0';
      break;
    }
  }

  if (firstchar==NULL || (firstchar && *firstchar=='\0')) dest[0]='\0';

#ifdef REMOVE_QUOTES
  in=out=dest;

  while (*in)
  {
    if (*in != '\\')
      *out++=*in;

    in++;
  }

  *out='\0';
#endif

  return ((int)(firstchar-strng));
}

