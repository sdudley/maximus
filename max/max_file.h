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

#ifndef MAX_FILE_H_DEFINED__
#define MAX_FILE_H_DEFINED__

#define MAX_LANG_f_area /* Include file-area language items */

#include "mm.h"
#include "max_area.h"

/* The following structure is used for the API to the file-handling API
 */

typedef struct _file_entry
{
  char *szName;         /* Filename and path */
  char *szDesc;         /* File description; used for xtr protocol ULs */
  dword ulSize;         /* Estimated size of file */
  word fFlags;          /* File flags.  See FFLAG_xxx in max.h */
} FENTRY;

int Alloc_Filenames_Buffer(void);
void Free_Filenames_Buffer(word usLeave);
/* int Alloc_Filename_Buf(int n); */

void Init_File_Buffer(void);
word FileEntries(void);
int CanAddFileEntry(void);
int RemoveFileEntry(word n);
int AddFileEntry(char *fname, word flags, long size);
int GetFileEntry(word n, FENTRY *fent);
int UpdFileEntry(word n, FENTRY *fent);
void Free_File_Buffer(void);

  /* `autodate' macro which is also area override flag aware */

#define autodate(f)   (((prm.flags & FLAG_autodate) && !((f).fa.attribs & (FA_MANDATE|FA_LISTDATE))) || ((f).fa.attribs & FA_AUTODATE))
#define manualdate(f) ((!(prm.flags & FLAG_autodate) && !((f).fa.attribs & (FA_AUTODATE|FA_LISTDATE))) || ((f).fa.attribs & FA_MANDATE))
#define listdate(f)   (!!((f).fa.attribs & FA_LISTDATE))

#endif

