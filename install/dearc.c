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
#include <string.h>
#include <stdlib.h>
#include "ar.h"
#include "arc.h"

static uchar buffer[DICSIZ];
static void near err_read(char *name);

int main(int argc, char *argv[])
{
  char temp[PATHLEN];
  char *dot, *bs;

  if (argc < 2)
    usage();

  make_crctable();

  strcpy(temp, argv[1]);

  bs=strrchr(temp, '\\');
  dot=strrchr(temp, '.');

  if ((!bs && !dot) || (bs && (dot==NULL || dot < bs)))
    strcat(temp, ".fiz");

  dearcit(temp, argc==2 ? "" : argv[2]);

  return 0;
}



void dearcit(char *arcname, char *path)
{
  struct _fizhdr fh;
  char fname[120];
  word n;

  if ((arcfile=fopen(arcname, "rb"))==NULL)
    err_open(arcname);

  while (fread((char *)&fh, sizeof(fh), 1, arcfile) == 1)
  {
    origsize=compsize=unpackable=0;

    if (fread(fname, fh.fnlen, 1, arcfile) != 1)
      err_read(arcname);

    fname[fh.fnlen]='\0';

    /* Add the path in front */

    memmove(fname+strlen(path), fname, strlen(fname)+1);
    memmove(fname, path, strlen(path));

    if ((outfile=fopen(fname, "wb"))==NULL)
      err_open(fname);

    printf("writing %s", fname);


    crc=INIT_CRC;
    origsize=fh.origsize;
    compsize=fh.compsize;

    decode_start();

    while (origsize != 0)
    {
              n = (uint)((origsize > DICSIZ) ? DICSIZ : origsize);

              #ifdef FLIP
              {
                word y;

                for (y=0; y < n; y++)
                  buffer[y]=~buffer[y];
              }
              #endif

              decode(n, buffer);
              fwrite_crc(buffer, n, outfile);

              fputc('.', stdout);

              origsize -= n;
    }

    if (crc != fh.crc)
      printf("CRC error\n");

    fclose(outfile);

    printf("\n");
  }

  fclose(arcfile);
}


static void near usage(void)
{
  printf("format: dearc arcname path\...\n");
  exit(1);
}

static void near err_open(char *name)
{
  printf("Fatal error opening `%s'!\n", name);
  exit(1);
}

static void near err_read(char *name)
{
  printf("Fatal error reading from `%s'!\n", name);
  exit(1);
}


