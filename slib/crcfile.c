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

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "crc.h"

dword *cr3tab;

#define CRCBUFSIZE 16384

main(int argc, char *argv[])
{
  int fd, got;
  char *buf;
  dword crc;
  char *p;

  cr3tab=mkcrc32tab();

  if (argc < 2 || (fd=shopen(argv[1], O_RDONLY | O_BINARY))==NULL)
  {
    printf("Can't open `%s'\n", argv[1]);
    return 1;
  }

  if ((buf=malloc(CRCBUFSIZE))==NULL)
  {
    printf("Out of memory!\n");
    return 1;
  }

  printf("CRCing %s...\n", argv[1]);

  for (crc=~0; (got=read(fd, (char *)buf, CRCBUFSIZE)) > 0; )
  {
    char *e=buf+got;

    for (p=buf; p < e; )
      crc=xcrc32(*p++, crc);
  }

/*  crc=xcrc32(0, xcrc32(0, crc));*/
  crc=~crc;

  free(buf);
  close(fd);

  printf("CRC is %08lx\n", crc);

  return 0;
}

