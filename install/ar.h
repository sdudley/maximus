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
	ar.h
***********************************************************/

#ifdef NT
typedef unsigned short ushort;
#endif

#include <stdio.h>
#include <limits.h>
#include "typedefs.h"
#include "compiler.h"
typedef unsigned char  uchar;   /*  8 bits or more */


/* The decompress routine doesn't like to work unless uint is a
 * short, but compress doesn't like to work unless uint is a
 * longword.  The code compiles fine if int == uint == 16bits,
 * but when int != 16bits, it gets into trouble.  There is some
 * 32-bit unclean code in here, but it is not in an obvious
 * location.
 */

#ifdef MAX_INSTALL
typedef unsigned short uint;    /* 16 bits or more */
#else
typedef unsigned int   uint;    /* 16 bits or more */
#endif

/* ar.c */

extern int unpackable;
extern ulong origsize, compsize;

/* io.c */

#define INIT_CRC  0  /* CCITT: 0xFFFF */
extern FILE *arcfile, *infile, *outfile;
extern uint crc, bitbuf;
#define BITBUFSIZ (CHAR_BIT * sizeof bitbuf)

void _stdc error(char *fmt, ...);
void make_crctable(void);
void fillbuf(int n);
uint getbits(int n);
/* void putbit(int bit); */
void putbits(int n, uint x);
int fread_crc(uchar *p, int n, FILE *f);
void fwrite_crc(uchar *p, int n, FILE *f);
void init_getbits(void);
void init_putbits(void);

/* encode.c and decode.c */

#define DICBIT    13    /* 12(-lh4-) or 13(-lh5-) */
#define DICSIZ (1U << DICBIT)
#define MATCHBIT   8    /* bits for MAXMATCH - THRESHOLD */
#define MAXMATCH 256    /* formerly F (not more than UCHAR_MAX + 1) */
#define THRESHOLD  3    /* choose optimal value */
#define PERC_FLAG 0x8000U

void encode(void);
void decode_start(void);
void decode(uint count, uchar text[]);

/* huf.c */

#define NC (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
	/* alphabet = {0, 1, 2, ..., NC - 1} */
#define CBIT 9  /* $\lfloor \log_2 NC \rfloor + 1$ */
#define CODE_BIT  16  /* codeword length */

extern ushort left[], right[];

void huf_encode_start(void);
void huf_decode_start(void);
uint decode_c(void);
uint decode_p(void);
void output(uint c, uint p);
void huf_encode_end(void);

/* maketbl.c */

void make_table(int nchar, uchar bitlen[],
				int tablebits, ushort table[]);

/* maketree.c */

int make_tree(int nparm, ushort freqparm[],
				uchar lenparm[], ushort codeparm[]);
