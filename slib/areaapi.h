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

#ifndef __AREAAPI_H_DEFINED
#define __AREAAPI_H_DEFINED

#define AFFO_DIV    0x0001    /* Allow findopen to return division entries */

/* Handle for accessing an area file */

typedef struct
{
  void *dat;            /* BFILE handle for the physical file */
  void *idx;            /* BFILE handle for the index file */
                        /* The above are declared as pv to avoid
                           having to include bfile.h */
  int msg_area;         /* boolean: msg area or file area? */
  char *idxname;        /* Name of the index file */
} *HAF;


/* Handle for performing a search on an area file */

typedef struct
{
  HAF haf;                  /* Handle for the area file itself */
  char *name;               /* Text to search for */
  long ofs;                 /* Current file position */
  long start_ofs;           /* Position of the start of this area */
  int heap_size;            /* Size of heap in this area */
  int num_override;         /* Number of overrides in this area */
  unsigned wrapped;         /* True if the search has wrapped once */
  int flags;

  union
  {
    MAH mah;                /* The individual message/file area structs */
    FAH fah;
  } ah;
} *HAFF;


HAF _fast AreaFileOpen(char *name, int msg_area);
HAFF _fast AreaFileFindOpen(HAF haf, char *name, int flags);
void _fast AreaFileFindChange(HAFF haff, char *newname, int newflags);
int _fast AreaFileFindNext(HAFF haff, void *v, unsigned wrap);
int _fast AreaFileFindPrior(HAFF haff, void *v, unsigned wrap);
int _fast AreaFileFindClose(HAFF haff);
int _fast AreaFileClose(HAF haf);
void _fast DisposeMah(MAH *pmah);
void _fast DisposeFah(FAH *pfah);
int _fast ReadMsgArea(HAF haf, char *name, MAH *pmah);
int _fast ReadFileArea(HAF haf, char *name, FAH *pfah);
int _fast AreaFileFindSeek(HAFF haff, void *v, unsigned uiNum);
void _fast AreaFileFindReset(HAFF haff);
int _fast CopyMsgArea(PMAH to, PMAH from);
int _fast CopyFileArea(PFAH to, PFAH from);

#endif

