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

/*# name=A fopen() function which uses the SH_DENYNONE sharing attribute,
    name=with a modifiable sharing attribute
*/

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <errno.h>
#include "compiler.h"
#include "prog.h"

FILE * _fast sfopen(char *name, char *fpmode, int fdmode, int access)
{
  FILE *fp;
  int fd;

  fd=sopen(name, fdmode, access, S_IREAD | S_IWRITE);
  
  if (fd==-1 && errno==ENOENT && (fdmode & (O_APPEND | O_WRONLY)))
    fd=sopen(name, fdmode | (O_WRONLY | O_CREAT | O_TRUNC),
             access, S_IREAD | S_IWRITE);
  
  if (fd==-1)
    return NULL;
  
  if ((fp=fdopen(fd, fpmode))==NULL)
    close(fd); 
  
  return fp;
}



