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

/*# name=A printf() for file-handling functions
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <io.h>
#include "prog.h"

int _stdc hprintf(int file,char *format,...)
{
  va_list var_args;
  char *string;
  int x,y;

  if (strlen(format) > 256)
    return -1;

  if ((string=(char *)malloc(256))==NULL)
    return(-2);

  va_start(var_args,format);
  x=vsprintf(string,format,var_args);
  va_end(var_args);

  y=write(file,string,strlen(string));
  free(string);

  return(x==y ? x : -1);
}

