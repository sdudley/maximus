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

#ifndef __BFILE_H_DEFINED
#define __BFILE_H_DEFINED

#ifndef __PROG_H_DEFINED
#include "prog.h"
#endif


#define BSEEK_SET   0x01                      /* Seek to exact file posn */
#define BSEEK_CUR   0x02                      /* Seek to relative file posn */
#define BSEEK_END   0x03                      /* Seek relative to EOF */

#define BO_RDONLY   0x01                      /* File opened for reading */
#define BO_WRONLY   0x02                      /* File opened for writing */
#define BO_RDWR     (BO_RDONLY|BO_WRONLY)     /* Read/write */
#define BO_APPEND   0x04                      /* Append */
#define BO_CREAT    0x08                      /* Create if does not exist */
#define BO_TRUNC    0x10                      /* Truncate file */
#define BO_EXCL     0x20                      /* Fail if does not exist */
#define BO_BINARY   0x40                      /* REDUNDANT!  Bopen always */
                                              /* uses binary mode! */

#define BSH_COMPAT  0x01                      /* Compatibility mode */
#define BSH_DENYRW  0x02                      /* Deny read/write */
#define BSH_DENYALL BSH_DENYRW                /* Deny read/write */
#define BSH_DENYWR  0x04                      /* Deny write */
#define BSH_DENYRD  0x08                      /* Deny read */
#define BSH_DENYNO  0x10                      /* Deny none */

typedef struct _bfile
{
  #define BUF_ID  0x8152
  word id;                /* Identifier for the BFILE structure */
  
  int fd;                 /* Associated file handle */
  int fMode;              /* File-opening mode */

  byte *pcBuf;            /* File buffer */
  byte *pcBufCur;         /* Current position in buffer */
  byte *pcBufEnd;         /* End of used portion of buffer */
  size_t stBufSize;       /* Size of 'buf', in bytes */
  long lDeltaLo;          /* Index of first changed element in buffer */
  long lDeltaHi;          /* Index of last modified character in buffer */

  long lPos;              /* File position of start of buffer */

} *BFILE;

#define Bgetc(b) ((b)->pcBufCur < (b)->pcBufEnd ? *(b)->pcBufCur++ : _Bgetc(b))

BFILE _fast Bopen(char *pszName, int fMode, int fShare, size_t stBufSize);
int _fast Bclose(BFILE b);
int _fast Bread(BFILE b, void *pvBuf, unsigned uiSize);
int _fast Bwrite(BFILE b, void *pvBuf, unsigned uiSize);
int _fast _Bgetc(BFILE b);
long _fast Bseek(BFILE b, long lRelPos, int fWhere);
int _fast Bfileno(BFILE b);
long _fast Btell(BFILE b);
char * _fast Bgets(char *pszOut, unsigned uiMaxLen, BFILE b);
int _fast Bputs(BFILE b, char *pszOut);
void _stdc Bprintf(BFILE bf, char *format, ...);

#endif /* __BFILE_H_DEFINED */

