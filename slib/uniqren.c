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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"

sword _fast uniqrename(char *from, char *toorig, char *final, int (_fast *pfnMove)(char *fromfile, char *tofile))
{
  int tsize=strlen(toorig)+30;
  char *to=(char *)malloc(tsize);
  char *bs, *dot, *add;
  sword ret;
  unsigned fex;
  word i;
  
  memset(to, '\0', tsize);
  strcpy(to, toorig);
  
  ret=0;

  if (!fexist(from))
    ret=-1;
  else while ((fex=fexist(to)) != FALSE ||
              (pfnMove ? (*pfnMove)(from, to) : rename(from, to)) != 0)
  {
    if (!fex && (errno==ENOTSAM || !fexist(to)
#ifndef __TURBOC__
        || errno==EXDEV
#endif
        ))
    {
      ret=-1;
      break;
    }
    
    /* Check to see if the filename has an extension */
    
    bs=strrstr(to, "/\\");
    dot=strrchr(to, '.');
    
    /* If not... */
    
    if (dot==NULL || dot < bs)
      strcat(to, ".000");
    
    /* Find the location of the new dot */
    
    dot=strrchr(to, '.');

    /* Make sure that the ext has three zeroes */

    for (i=1; i <= 3; i++)
      if (dot[i]=='\0')
        dot[i]='0';

    add=dot+3;

    for (add=dot+3; add >= to; add--)
    {
      if (*add >= '0' && *add <= '9')
      {
        if (*add == '9')
          *add='0';
        else
        {
          (*add)++;
          break;
        }
      }
      else if (*add=='/' || *add=='\\' || *add==':')
      {
        free(to);
        return -1; /* can't rename */
      }
      else if (*add != '.')
      {
        *add='0';
        break;
      }
    }
  }
  
  if (final)
    strcpy(final, to);

  free(to);
  return ret;
}


