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

/*# name=Copy a file
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "alc.h"

#define COPY_SIZE 16384    /* Default buffer size for file_copy() */

int _fast lcopy(char *fromfile,char *tofile)
{
  int file1,
      file2;

  unsigned copy_size;

  char *temp;
  unsigned rbytes;
  unsigned wbytes;
  union stamp_combo sc;

  for (copy_size=COPY_SIZE;
       (temp=malloc(copy_size+1))==NULL && copy_size >= 128;
       copy_size /= 2)
    ;

  if (temp==NULL || copy_size < 128)
  {
    if (temp)
      free(temp);

    return -1;
  }

  if ((file1=shopen(fromfile,O_RDONLY | O_BINARY))==-1)
  {
    free(temp);
    return -1;
  }

  if ((file2=sopen(tofile,O_WRONLY | O_CREAT | O_TRUNC | O_BINARY,SH_DENYWR,S_IREAD | S_IWRITE))==-1)
  {
    close(file1);
    free(temp);
    return -1;
  }

  do
  {
    rbytes=read(file1,temp,copy_size);

    if ((int)rbytes < 0)
    {
      wbytes=1; /* so that rbytes != wbytes and we return failure */
      break;
    }

    wbytes=write(file2,temp,rbytes);

  } while (rbytes==copy_size && rbytes==wbytes);

  if (rbytes==wbytes)
  {
    if (get_fdt(file1,&sc)==0)
      set_fdt(file2,&sc);
  }

  close(file2);
  close(file1);

  free(temp);
  return ((rbytes==wbytes) ? 0 : -1);
}


