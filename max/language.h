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

#ifndef __LANGUAGE_H_DEFINED
#define __LANGUAGE_H_DEFINED


#ifdef MAX_INCL_LANGUAGE

/*
    Format of the Maximus .LTF (Language Translation File):

    _gheapinf                   ; Global heap information

    _heapnames[..]              ; Heap names (text)

    _heapdata[0]                ; Info block for heap 0
    _heapdata[1]                ; Info block for heap 1
    _heapdata[2]                ; Info block for heap 2
        ...                                          ...
    _heapdata[n]                ; Info block for heap n

    HOFS[0][0] ... HOFS[0][m]   ; Offsets for heap 0
    hstr[0][0] ... hstr[0][m]   ; Strings for heap 0

    HOFS[1][0] ... HOFS[1][m]   ; Offsets for heap 1
    hstr[1][0] ... hstr[1][m]   ; Strings for heap 1

       ................

    HOFS[n][0] ... HOFS[n][m]   ; Offsets for heap n
    hstr[n][0] ... hstr[n][m]   ; Strings for heap n
*/

#define MAX_HEAP      32
#define MAX_HEAPNAMES 256

typedef sword HOFS;


struct _gheapinf
{
  byte language[32];                /* Language name */
  byte n_heap;                      /* Number of primary heaps */
  byte rsvd;
  word max_gptrs_len;               /* Largest size of offsets array */
  word max_gheap_len;               /* Largest heap length */
  word file_ptrs;                   /* Total # of strings in non-user heaps */
  word user_ptrs;                   /* Total # of strings in user heaps */
  word hn_len;                      /* Length of heapnames heap */
};

struct _heapdata
{
  sdword start_ofs;                 /* Start offset of heap in file */
  word heapname;                    /* Heap name offset */
  word start_num;                   /* Numeric ID of first string in this heap */
  word ndefs;                       /* Number of strings in this heap */
  word adefs;                       /* Alternate strings in heap */
  word hlen;                        /* Length of this heap */
};

struct _lang
{
  struct _gheapinf inf;             /* Global heap information */

  char *heapnames;                  /* Heapnames heap */

  struct _heapdata hdat[MAX_HEAP];  /* Data for all heaps */
  struct _heapdata *ch;             /* Pointer to current heap */

  HOFS *ptrs;                       /* Heap offsets array (incl alternates) */
  char *heap;                       /* Heap data (strings) */

  word max_ptrs;                    /* Largest number of strings in heaps */
  word max_heap;                    /* Largest heap size */
};


#ifdef MAX_INCL_LANGLTH
  #include "english.lth"
#endif


#define InHeap(sn,h)  (sn >= (h)->start_num && sn < (h)->start_num+(h)->ndefs)

cpp_begin()
  char *s_ret(word strn);
cpp_end()

#endif /* MAX_INCL_LANGUAGE */

#endif /* __LANGUAGE_H_DEFINED */

