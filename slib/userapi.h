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

#ifndef __USERAPI_H_DEFINED
#define __USERAPI_H_DEFINED

#define UNDX_BLOCK  512           /* Number of USER.NDX entries to buffer */
#define UBBS_BLOCK  16            /* Hold 16 user records at a time */

#define ID_HUF    0x89098faeL
#define ID_HUFF   0x8520aed2L

typedef struct _huf
{
  dword id_huf;                   /* Unique identifier */
  int fdbbs;                      /* Handle of the user.bbs file */
  int fdndx;                      /* Handle of the user.ndx file */
} *HUF;



typedef struct
{
  dword id_huff;                  /* Unique identifier for handle */

  HUF huf;                        /* Handle to the original user file */
  long lLastUser;                 /* Index # of user last returned */

  unsigned long ulStartNum;       /* User# of first user in pusr array */
  unsigned int cUsers;            /* Number of users in pusr array */

  struct _usr *pusr;              /* Array of users being searched */

  struct _usr usr;                /* Found user! */
} *HUFF;


typedef HUFF HUFFS;

HUF _fast UserFileOpen(char *name, int mode);
long _fast UserFileSize(HUF huf);
int _fast UserFileFind(HUF huf, char *name, char *alias, struct _usr *pusr);
HUFF _fast UserFileFindOpen(HUF huf, char *name, char *alias);
int _fast UserFileFindNext(HUFF huff, char *name, char *alias);
int _fast UserFileFindPrior(HUFF huff, char *name, char *alias);
int _fast UserFileFindClose(HUFF huff);
int _fast UserFileSeek(HUF huf, long rec, struct _usr *pusr, int sz);
int _fast UserFileUpdate(HUF huf, char *name, char *alias, struct _usr *pusr);
int _fast UserFileCreateRecord(HUF huf, struct _usr *pusr, int fCheckUnique);
int _fast UserFileRemove(HUF huf, struct _usr *pusr);
int _fast UserFileClose(HUF huf);

#define UserFileFindOpenR(huf, n, a) UserFileFindOpen(huf, n, a)
#define UserFileFindSeqOpen(huf)     UserFileFindOpen(huf, NULL, NULL)
#define UserFileFindSeqNext(huffs)   UserFileFindNext(huffs, NULL, NULL)
#define UserFileFindSeqClose(huffs)  UserFileFindClose(huffs)

#endif /* __USERAPI_H_DEFINED */


