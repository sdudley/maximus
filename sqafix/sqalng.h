/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*
 * SQAFIX: Squish Area Fix Utility For Squish Echo Mail Processor
 *
 * Header module for compiler dependant stuff
 *
 * Created: 22/Mar/92
 * Updated: 22/Mar/92
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

  // Make up the compiler dependant embedded assembler code prefix

#if   defined (__TURBOC__)
#define ASM asm
#elif defined (__MSC__)
#define ASM _asm
#elif defined (__ZTC__)
#error cant compile embedded assembler under ZTC
#endif

  // Directory management definition which are missing under Microsoft C

#if !defined( __DIR_H )

#if defined (__OS2__)
#define MAXPATH   CCHMAXPATH
#define MAXDRIVE  3
#define MAXDIR    CCHMAXPATHCOMP
#define MAXFILE   CCHMAXPATHCOMP
#define MAXEXT    CCHMAXPATHCOMP
#define MAXNAME   CCHMAXPATHCOMP
#elif defined (__W32__)
#define MAXPATH   260
#define MAXDRIVE  3
#define MAXDIR    260
#define MAXFILE   260
#define MAXEXT    260
#elif defined (UNIX)
#define MAXPATH   1024
#define MAXDRIVE  3
#define MAXDIR    66
#define MAXFILE   250
#define MAXEXT    250
#else
#define MAXPATH   80
#define MAXDRIVE  3
#define MAXDIR    66
#define MAXFILE   9
#define MAXEXT    5
#endif

#define WILDCARDS 0x01
#define EXTENSION 0x02
#define FILENAME  0x04
#define DIRECTORY 0x08
#define DRIVE     0x10

#define _CType
#define _FAR

void        _CType fnmerge( char _FAR *__path,
                            const char _FAR *__drive,
                            const char _FAR *__dir,
                            const char _FAR *__name,
                            const char _FAR *__ext );

int         _CType fnsplit( const char _FAR *__path,
                            char _FAR *__drive,
                            char _FAR *__dir,
                            char _FAR *__name,
                            char _FAR *__ext );

#ifndef UNIX
#define setdisk(d)  (_chdrive((d) + 1))
#define getdisk()   (_getdrive() - 1)
#endif

#endif

/*
 * End of SQALNG.H
 */
