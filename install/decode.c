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
    decode.c
***********************************************************/
#include "ar.h"

static int j;  /* remaining bytes to copy */

void decode_start(void)
{
    huf_decode_start();
    j = 0;
}

void decode(uint count, uchar buffer[])
    /* The calling function must keep the number of
       bytes to be processed.  This function decodes
       either 'count' bytes or 'DICSIZ' bytes, whichever
       is smaller, into the array 'buffer[]' of size
       'DICSIZ' or more.
       Call decode_start() once for each new file
       before calling this function. */
{
    static uint i;
    uint r, c;

    r = 0;
    while (--j >= 0) {
        buffer[r] = buffer[i];
        i = (i + 1) & (DICSIZ - 1);
        if (++r == count) return;
    }
    for ( ; ; ) {
        c = decode_c();
        if (c <= UCHAR_MAX) {
            buffer[r] = (uchar)c;
            if (++r == count) return;
        } else {
            j = c - (UCHAR_MAX + 1 - THRESHOLD);
            i = (r - decode_p() - 1) & (DICSIZ - 1);
            while (--j >= 0) {
                buffer[r] = buffer[i];
                i = (i + 1) & (DICSIZ - 1);
                if (++r == count) return;
            }
        }
    }
}
