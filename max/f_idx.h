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

#ifndef __F_IDX_H_DEFINED
#define __F_IDX_H_DEFINED

#ifndef __FB_H_DEFINED
  #include "fb.h"
#endif

struct _idxf
{
  FIDX fidx;
  char *pat;
  long lo, hi, last, next_found_ofs;
  int found, pat_len;
  int ifd;
};

typedef struct _idxf IDXF;

IDXF * IndexOpen(char *pat);
int IndexNext(IDXF *ix);
void IndexClose(IDXF *ix);
word FidxIsOkay(FIDX *fidx, FDAT *fdat, char *name, char *path, word check_cur,
                word check_priv, PFAH pfahIn);


#endif /* !__F_IDX_H_DEFINED */
