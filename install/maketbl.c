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
    maketbl.c -- make table for decoding
***********************************************************/
#include "ar.h"

void make_table(int nchar, uchar bitlen[], int tablebits, ushort table[])
{
    ushort count[17], weight[17], start[18], *p;
    uint i, k, len, ch, jutbits, avail, nextcode, mask;

    for (i = 1; i <= 16; i++) count[i] = 0;
    for (i = 0; i < (uint)nchar; i++) count[bitlen[i]]++;

    start[1] = 0;
    for (i = 1; i <= 16; i++)
        start[i + 1] = start[i] + (count[i] << (16 - i));
    if (start[17] != (ushort)(1U << 16)) error("Bad table");

    jutbits = 16 - tablebits;
    for (i = 1; i <= (uint)tablebits; i++) {
        start[i] >>= jutbits;
        weight[i] = 1U << (tablebits - i);
    }
    while (i <= 16) weight[i++] = 1U << (16 - i);

    i = start[tablebits + 1] >> jutbits;
    if (i != (ushort)(1U << 16)) {
        k = 1U << tablebits;
        while (i != k) table[i++] = 0;
    }

    avail = nchar;
    mask = 1U << (15 - tablebits);
    for (ch = 0; ch < (uint)nchar; ch++) {
        if ((len = bitlen[ch]) == 0) continue;
        nextcode = start[len] + weight[len];
        if (len <= (uint)tablebits) {
            for (i = start[len]; i < nextcode; i++) table[i] = ch;
        } else {
            k = start[len];
            p = &table[k >> jutbits];
            i = len - tablebits;
            while (i != 0) {
                if (*p == 0) {
                    right[avail] = left[avail] = 0;
                    *p = avail++;
                }
                if (k & mask) p = &right[*p];
                else          p = &left[*p];
                k <<= 1;  i--;
            }
            *p = ch;
        }
        start[len] = nextcode;
    }
}
