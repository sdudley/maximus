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
#include "prog.h"
#include "ar.h"
#include "arc.h"
#include "prog.h"
#include "tui.h"
#include "install.h"

static uchar buffer[DICSIZ];
static void near err_read(char *name);

VWIN *arcwin=NULL;

extern FILE *ilog;



static void near err_open(char *name)
{
  WinErr("Fatal error opening `%s'", name);
  WinExit(1);
}

static void near err_read(char *name)
{
  WinErr("Fatal error reading from `%s'", name);
  WinExit(1);
}

void ArcWinOpen(void)
{
  if ((arcwin=WinOpen(0, 0, 15, 60, BORDER_DOUBLE, col(CWHITE | _BLUE),
                      col(CYELLOW | _BLUE), WIN_CENTRE))==NULL)
  {
    NoMem();
  }

  make_crctable();
}

void ArcWinClose(void)
{
  WinClose(arcwin);
  arcwin=NULL;
}


void dearcit(char *arcname, char *path, word (*dupefile)(char*,char*,char*), word (near *doit)(char *fn))
{
  struct _fizhdr fh;
  char fname[120], orig[120];
  word j, n;
  word skip;

  if (ilog)
    fprintf(ilog, "Decompressing %s\n", arcname);
  
  WinPrintf(arcwin, " Decompressing %s\r\n", arcname);
  WinSync(arcwin, TRUE);

  if ((arcfile=fopen(arcname, "rb"))==NULL)
    err_open(arcname);
  
  while (fread((char *)&fh, sizeof(fh), 1, arcfile) == 1)
  {
    origsize=compsize=unpackable=0;

    if (fread(orig, fh.fnlen, 1, arcfile) != 1)
      err_read(arcname);

    orig[fh.fnlen]='\0';
    strcpy(fname, orig);

    /* Add the path in front */

    memmove(fname+strlen(path), fname, strlen(fname)+1);
    memmove(fname, path, strlen(path));
    
    /* Handle duplicate files */

    skip=FALSE;

    if (fexist(fname))
      skip=dupefile ? (*dupefile)(fname, path, orig) : TRUE;

    if (doit && !skip)
      skip=! (*doit)(orig);

    /* If we're not going to extract this one, skip over the compr. version */

    if (skip)
      fseek(arcfile, fh.compsize, SEEK_CUR);
    else
    {
      if ((outfile=fopen(fname, "wb"))==NULL)
        err_open(fname);
    }

    if (ilog)
      fprintf(ilog, "  %s %s\n", skip ? "Skipping" : " Writing", fname);

    WinPrintf(arcwin, "   %s %-*s", skip ? "Skipping" : " Writing",
              strlen(path)+13, fname);

    WinSync(arcwin, TRUE);

    
    if (!skip)
    {
      crc=INIT_CRC;
      origsize=fh.origsize;
      compsize=fh.compsize;

      if (fh.method != 0)
        decode_start();

      while (origsize != 0)
      {
        n = (word)((origsize > DICSIZ) ? DICSIZ : (word)origsize);

        if (fh.method==0)
           fread((char *)buffer, 1, n, arcfile);
        else
        {

          decode(n, buffer);

          #if 0
          { /*SJD Sun  09-08-1991  20:01:27 */
            char *p, *e;

            for (p=buffer, e=p+n; p < e; p++)
              *p=~*p;
          }
          #endif
        }

        fwrite_crc(buffer, n, outfile);

        origsize -= n;

        if (origsize)
        {
          for (j=10; j; j--)
            WinPutc(arcwin, (byte)(fh.origsize*(long)j >= origsize*10L
                               ? 'Û' : '°'));

          for (j=10; j; j--)
            WinPutc(arcwin, '\b');

          WinSync(arcwin, TRUE);
        }
      }


      if (crc != fh.crc)
        WinErr("CRC error");

      flush_handle(outfile);
      set_fdt(fileno(outfile), &fh.date);

      fclose(outfile);
    }

    WinPuts(arcwin, "Done.     \r\n");
    WinSync(arcwin, TRUE);
  }

  fclose(arcfile);
}

/*
main()
{
  WinApiOpen(TRUE);
  ArcWinOpen();
  dearcit("TEST.FIZ", "x\\");
  ArcWinClose();
  WinApiClose();
  return 0;
}
*/


