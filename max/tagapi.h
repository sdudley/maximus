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

#ifndef __TAGAPI_H_DEFINED
#define __TAGAPI_H_DEFINED

#define MAX_MTAG_FRE    128   /* Allow up to 128 free spots before a pack */
#define MTAG_PAD_SIZE   128   /* Allow 128 bytes in mtag record for expansion */
#define PACK_BUF_SIZE   1024  /* Use 1K buffer for copying tag data */

/* Structure of a record within the mtag index file */

struct _mtagidx
{
  dword dwUserHash;       /* Hash of user's name */
  dword dwOffset;         /* Offset of data within dat file */
  dword dwLen;            /* Length of data segment in dat file */
  dword dwUsed;           /* Amount of data actually used within dwLen */
};

/* Structure of records within the mtag.fre file */

struct _mtagfre
{
  dword dwOffset;       /* Offset of free frame */
  dword dwLen;          /* Length of free frame */
};


/* Memory-based representation of the user's tagged data */

struct _mtagmem
{
  char *pbAreas;      /* Pointer to area data */
  word dwUsed;        /* Amount of data used in pbAreas */
  word dwLen;         /* Length of memory allocated for pbAreas */
};

int TagReadTagFile(struct _mtagmem *pmtm);
int TagWriteTagFile(struct _mtagmem *pmtm);
int TagQueryTagList(struct _mtagmem *pmtm, char *pszArea);
int TagAddTagList(struct _mtagmem *pmtm, char *pszArea);
int TagDeleteTagList(struct _mtagmem *pmtm, char *pszArea);

#endif /* __TAGAPI_H_DEFINED */

