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

/***********************************************************
    io.c -- input/output
***********************************************************/

#ifdef MAX_INSTALL
  #include <stdlib.h>
  #include <stdarg.h>
  #include "prog.h"
  #include "ar.h"
  #include "prog.h"
  #include "tui.h"
  #include "install.h"
#else
  #include "ar.h"
  #include <stdlib.h>
  #include <stdarg.h>
#endif

#define CRCPOLY  0xA001  /* ANSI CRC-16 */
                         /* CCITT: 0x8408 */
#define UPDATE_CRC(c) \
    crc = crctable[(crc ^ (c)) & 0xFF] ^ (crc >> CHAR_BIT)

FILE *arcfile, *infile, *outfile;
uint crc, bitbuf;

static ushort crctable[UCHAR_MAX + 1];
static uint  subbitbuf;
static int   bitcount;

void _stdc error(char *fmt, ...)
{
#ifdef MAX_INSTALL
  char szError[180];
  va_list args;

  va_start(args, fmt);
  vsprintf(szError, fmt, args);
  va_end(args);

  WinErr(szError);
  WinExit(1);
#else
  va_list args;

  va_start(args, fmt);
  putc('\n', stderr);
  vfprintf(stderr, fmt, args);
  putc('\n', stderr);
  va_end(args);
  exit(EXIT_FAILURE);
#endif
}

void make_crctable(void)
{
    ushort i, j, r;

    for (i = 0; i <= UCHAR_MAX; i++) {
        r = i;
        for (j = 0; j < CHAR_BIT; j++)
            if (r & 1) r = (r >> 1) ^ CRCPOLY;
            else       r >>= 1;
        crctable[i] = r;
    }
}

void fillbuf(int n)  /* Shift bitbuf n bits left, read n bits */
{
    bitbuf <<= n;
    while (n > bitcount) {
        bitbuf |= subbitbuf << (n -= bitcount);
        if (compsize != 0) {
            compsize--;  subbitbuf = (uchar) getc(arcfile);
        } else subbitbuf = 0;
        bitcount = CHAR_BIT;
    }
    bitbuf |= subbitbuf >> (bitcount -= n);
}

uint getbits(int n)
{
    uint x;

    x = bitbuf >> (BITBUFSIZ - n);  fillbuf(n);
    return x;
}

void putbits(int n, uint x)  /* Write rightmost n bits of x */
{
    if (n < bitcount) {
        subbitbuf |= x << (bitcount -= n);
    } else {
        if (compsize < origsize) {
            putc(subbitbuf | (x >> (n -= bitcount)), outfile);
            compsize++;
        } else unpackable = 1;
        if (n < CHAR_BIT) {
            subbitbuf = x << (bitcount = CHAR_BIT - n);
        } else {
            if (compsize < origsize) {
                putc(x >> (n - CHAR_BIT), outfile);
                compsize++;
            } else unpackable = 1;
            subbitbuf = x << (bitcount = 2 * CHAR_BIT - n);
        }
    }
}

int fread_crc(uchar *p, int n, FILE *f)
{
    int i;

    i = n = fread(p, 1, n, f);  origsize += n;
    while (--i >= 0)
    {
      UPDATE_CRC(*p++);
/*      *p=~*p;*/
    }
    return n;
}

void fwrite_crc(uchar *p, int n, FILE *f)
{
    if ((sword)fwrite(p, 1, n, f) < n)
      error("Unable to write to file");

    while (--n >= 0)
    {
/*      *p=~*p;*/
      UPDATE_CRC(*p++);
    }
}

void init_getbits(void)
{
    bitbuf = 0;  subbitbuf = 0;  bitcount = 0;
    fillbuf(BITBUFSIZ);
}

void init_putbits(void)
{
    bitcount = CHAR_BIT;  subbitbuf = 0;
}

