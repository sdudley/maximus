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

/*# name=Concatenate string & allocate memory for said string.
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "alc.h"

char * _fast strrcat(char *string1,char *string2)
{
  static char *buffer=NULL;
  char *ptr;

  if (string1 && string2)     /* Check for NULL pointers */
  {
    if (buffer != NULL)       /* Free memory if we've been called before */
      free(buffer);

                              /* Allocate some memory */
    if ((buffer=(char *)malloc(strlen(string1)+strlen(string2)+1))==NULL)
      return(NULL);

    ptr=buffer;

    while (*string1)          /* Copy first string */
      *ptr++=*string1++;

    while (*string2)          /* Copy second string */
      *ptr++=*string2++;

    *ptr='\0';                /* Finish off by null-terminating */

    return(buffer);           /* And return to the user */
  }
  else return(NULL);
}

