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

/*# name=FFIND.C include file.
    credit=Thanks to Peter Fitzsimmons for this module.
*/

#ifndef __FFIND_H_DEFINED
#define __FFIND_H_DEFINED

#include "compiler.h"
#include "typedefs.h"
#include "stamp.h"
#include "prog.h"

cpp_begin()

struct _ffind;
typedef struct _ffind FFIND;

#if defined(__MSDOS__)

  /* Layout of the DOS Disk Transfer Area */

  struct _dosdta
  {
    char achReserved[21];
    byte bAttr;
    word usTime;
    word usDate;
    dword ulSize;
    char achName[13];
  };

  unsigned far pascal __dfindfirst(char far *name, unsigned attr, struct _dosdta far *dta);
  unsigned far pascal __dfindnext(struct _dosdta far *dta);

#elif defined(NT)
  #include "pwin.h"
#endif


struct _ffind
{
  word usAttr;
  SCOMBO scCdate;
  SCOMBO scAdate;
  SCOMBO scWdate;
  dword ulSize;

#if defined(__MSDOS__)

  char szName[13];
  struct _dosdta __dta;

#elif defined(NT)

  char szName[PATHLEN];
  HANDLE hdir;
  unsigned uiAttrSearch;        /* the attribute that we are searching for */

#elif defined(OS_2)

  char szName[PATHLEN];

  #ifdef __FLAT__ /* OS/2 2.0 or NT */
    unsigned long hdir;
  #else
    unsigned short hdir;       /* directory handle from DosFindFirst */
  #endif

#endif
};

FFIND * _fast FindOpen(char *filespec,unsigned short attribute);
FFIND * _fast FindInfo(char *filespec); /*PLF Thu  10-17-1991  18:03:09 */
int _fast FindNext(FFIND *ff);

#ifdef NT /* NT's abortion of a naming convention conflicts with our usage! */
#define FindClose FndClose
#endif

void _fast FindClose(FFIND *ff);

#define ATTR_READONLY  0x01
#define ATTR_HIDDEN    0x02
#define ATTR_SYSTEM    0x04
#define ATTR_VOLUME    0x08
#define ATTR_SUBDIR    0x10
#define ATTR_ARCHIVE   0x20
#define ATTR_RSVD1     0x40
#define ATTR_RSVD2     0x80

#define MSDOS_READONLY  ATTR_READONLY
#define MSDOS_HIDDEN    ATTR_HIDDEN
#define MSDOS_SYSTEM    ATTR_SYSTEM
#define MSDOS_VOLUME    ATTR_VOLUME
#define MSDOS_SUBDIR    ATTR_SUBDIR
#define MSDOS_ARCHIVE   ATTR_ARCHIVE
#define MSDOS_RSVD1     ATTR_RSVD1
#define MSDOS_RSVD2     ATTR_RSVD2

cpp_end()

#endif /* __FFIND_H_DEFINED */

