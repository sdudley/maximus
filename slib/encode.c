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

/*# name=Encode/decore char string into array of ints.
*/

#include <stdio.h>
#include "prog.h"

#define MAXSLEN 255

void _fast encode(char *str,int *iarray,int len,int key)
{
  int x,c,inc,in[MAXSLEN];

  for (x=0,inc=TRUE,c=1; x < len;x++)
  {
    in[x]=str[x];
    if (inc)
    {
      iarray[x]=in[x]-key+c;
      inc=FALSE;
    }
    else
    {
      iarray[x]=in[x]+key-c;
      inc=TRUE;
      c++;
    }
  }
  iarray[x]=-1;
}



void _fast decode(int *iarray,char *str,int key)
{
  int x,c,inc,in[MAXSLEN];

  for (x=0,inc=TRUE,c=1; iarray[x] != -1;x++)
  {
    if (inc)
    {
      in[x]=iarray[x]+key-c;
      inc=FALSE;
    }
    else
    {
      in[x]=iarray[x]-key+c;
      inc=TRUE;
      c++;
    }
    str[x]=(char)in[x];
  }
  str[x]='\0';
}



