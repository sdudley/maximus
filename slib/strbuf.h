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

/*
 * Strings buffer
 * Sub-allocator for strings
 */

#ifndef STRBUF_H_DEFINED
#define STRBUF_H_DEFINED

#define STRBUFSZ  2048                                   /* Default size */
#define STRDELMK  '\xff'                        /* Deleted string marker */

typedef struct _strbuf
{
  word    usSize;                                  /* String buffer size */
  word    usOffset;                              /* String buffer offset */
} strbuf;

strbuf * sb_new(int sz);                            /* Init a new buffer */
char * sb_alloc(strbuf *sb, char *s);               /* Allocate a string */
void sb_free(strbuf *sb, char *s);                    /* 'free' a string */
void sb_reset(strbuf *sb);                                      /* Reset */
                                                           /* Reallocate */
strbuf * sb_realloc(strbuf *sb, int sz, int (*reloc)(char*mold,char*mnew));
word sb_inbuf(strbuf *sb, char *s, int icase);/* ofs of string in buffer */

#define sbsize(s)   (s->usSize)

#endif

