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

#ifndef __FB_H_DEFINED
#define __FB_H_DEFINED

#ifndef __TYPEDEFS_H_DEFINED
#include "typedefs.h"
#endif

#define MAX_SEARCH_DAT    32
#define DMP_BUF_SIZE      2048

#define FF_FILE     0x01 /* Entry is a filename */
#define FF_OFFLINE  0x02 /* Entry is deleted */
#define FF_DELETED  0x04 /* File/comment is deleted */
#define FF_COMMENT  0x08 /* This is a comment (currently unused) */
#define FF_NOTIME   0x10 /* Don't deduct file from user's time limit */
#define FF_NOBYTES  0x20 /* Don't deduce file from user's DL bytes limit */
#define FF_DLCTR    0x40 /* Had a blank download counter */


/* Structure for FILES.IDX - each file is just a big array of these.  This  *
 * is also the structure used for the master MAXFILES.IDX.                  */

typedef struct _fidx
{
  /* Filename (NO TERMINATING NUL!) */

  byte name[MAX_FN_LEN];

  /* Record number in AREAS.DAT that this file area occupies */

  word anum;

  /* Offset into FILES.DAT (number of _fdat structs) that file occupies */

  word fpos;
} FIDX;




/* Structure for FILES.DAT in each file directory */

typedef struct _fdat
{
  byte name[MAX_FN_LEN+1];    /* Name of the file                           */
  byte struct_len;            /* Length of this structure.  USE THIS!       */

  word flag;                  /* See FF_XXXX definitions, above             */
  
  dword acs;                  /* Offset to ACS in FILES.DMP                 */
  word  rsvd1;
  
  dword times_dl;             /* # of times this file has been DLed         */
  
  union stamp_combo fdate;    /* Date of actual file (creation)             */
  union stamp_combo udate;    /* Date of upload (modification)              */
  
  dword fsize;                /* File's size, in bytes                      */

  /* Offsets in FILES.DMP */

  dword uploader;             /* Uploader's name                            */
  dword desc;                 /* File description (or comment if no file)   */
  dword path;                 /* Path of file - 0 if in normal file dir     */
  dword pwd;                  /* Password for file                          */

  char rsvd[10];              /* Reserved by Maximus for future use         */
} FDAT;


#endif /* !__FB_H_DEFINED */

