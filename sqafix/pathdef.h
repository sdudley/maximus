/*
 * SaqFix Version 99a9
 * Copyright 2003 by R. F. Jones.  All rights reserved.
 *
 * Portions of this file taken from ../slib/prog.h, which contains
 * the following copyright information:
 * 
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

#ifndef __PATHS_DEF_H_DEFINED
#define __PATHS_DEF_H_DEFINED

#ifndef __TURBOC__
  int _stdc fnsplit(const char *path,char *drive,char *dir,char *name,char *ext);
  int _stdc getcurdir(int drive, char *directory);

  #define textattr(attr)
// Following defined elsewhere for sqafix...
//  #define getdisk()                  get_disk()
//  #define setdisk(drive)             set_disk(drive)

  #define getvect(int)            _dos_getvect(int)
  #define setvect(int, func)      _dos_setvect(int, func)

  #ifndef inportb
    #define inportb(port)           inp(port)
  #endif

  #define inport(port)            inpw(port)

  #ifndef outportb
    #define outportb(port, byte)    outp(port, byte)
  #endif

  #define outport(port, byte)     outpw(port, byte)

  #if !defined(MK_FP) && !defined(_lint)
    #define MK_FP(seg, off)  (void far *)((unsigned long)(seg)<<16L | (off))
  #endif
#endif

#if defined(__MSC__) || defined(UNIX)
  int _fast lock(int fh, long offset, long len);
  int _fast unlock(int fh, long offset, long len);
#endif

#ifdef __MSC__
  #undef toupper
  extern unsigned char _MyUprTab[256];      /* see _ctype.c */
  #define toupper(c)  ((int)_MyUprTab[(c)])
#endif
 
#ifdef __WATCOMC__  /* WC 9.0 mistakenly omits prototype for fdopen */
  #if __WATCOMC__==900
  FILE *fdopen(int __handle,const char *__mode);
  #endif
#endif



#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

/* endian definition from configure */
#include "../slib/compiler_details.h"

#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN) && !defined(UNIX)
/* Do OS/2, DOS or Windows run on non-Intel, non-Alpha CPUs?? */
# define LITTLE_ENDIAN
#endif

#if defined(BIG_ENDIAN) && defined(LITTLE_ENDIAN)
# error BIG_ENDIAN and LITTLE_ENDIAN cannot both be defined at the same time!
#endif

#if !defined(BIG_ENDIAN) && !defined(LITTLE_ENDIAN)
# error Either BIG_ENDIAN or LITTLE_ENDIAN must be defined!
#endif

#ifdef __MSDOS__
#define WILDCARD_ALL     "*.*"
#else
#define WILDCARD_ALL     "*"
#endif

#if !defined(UNIX)
# define PATH_DELIM       '\\'   /* Default separator for path specification */
# define PATH_DELIMS      "\\"   /* string format */
# define PATHLEN           120   /* Max. length of a path                   */
# define MAX_DRIVES         26   /* Maximum number of drives on system;     *
                                  * for MS-DOS, A through Z.  Used by       *
                                  * Save_Dir()...                           */
#define NULL_DEVICE	"nul"
#else /* UNIX */
# include <limits.h>
# include "winstr.h"
# define PATH_DELIM	'/'
# define PATH_DELIMS	"/"
# define PATHLEN	PATH_MAX
# define MAX_DRIVES	1	/* Really 0, because it doesn't apply, but using 1 to avoid hidden bugs */
# define NULL_DEVICE	"/dev/null"
#endif

/* Macro to propercase MS-DOS filenames.  If your OS is case-dependent,     *
 * use "#define fancy_fn(s) (s)" instead.  Ditto for upper_fn().            */

#if !defined(UNIX)
# define fancy_fn(s)           fancy_str(s)
# define cfancy_fn(s)	       cfancy_str(s)
# define upper_fn(s)           strupr(s)
# define lower_fn(s)           strlwr(s)
#else
# define fancy_fn(s)		(s)
# define cfancy_fn(s)		(s)
# define upper_fn(s)		(s)
# define lower_fn(s)		(s)
#endif

#ifndef updcrc
#define updcrc(cp, crc)       (crctab[((crc >> 8) & 255) ^ cp] ^ (crc << 8))
#endif


#ifndef max
#define max(a,b)              (((a) > (b)) ? (a) : (b))
#define min(a,b)              (((a) < (b)) ? (a) : (b))
#endif


/* MS docs use both SH_DENYNONE and SH_DENYNO */

#if !defined(SH_DENYNONE) && defined(SH_DENYNO)
  #define SH_DENYNONE SH_DENYNO
#endif

#ifdef UNIX
unsigned long coreleft(void);
#endif

#endif /* __PATH_DEF_H_DEFINED */

