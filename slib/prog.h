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

/*# name=General include file.  Lots of machine-dependant stuff here.
*/

#ifndef __PROG_H_DEFINED
#define __PROG_H_DEFINED

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SJD
#define SJD
#endif

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "compiler.h"
#include "typedefs.h"

/*#include "typedefs.h"*/ /* now included from compiler.h */ /* not anymore - wes */
#if defined(UNIX)
# include <unistd.h>
#endif
#include "stamp.h"

#ifndef NULLL
#ifdef OS_2 /* for use with DLLs */
  #define NULLL (void far *)NULL
#else
  #define NULLL NULL
#endif
#endif

#ifdef __FARDATA__
  #include "alc.h"

  #if !defined(_lint)
    #define malloc(n)     farmalloc(n)
    #define calloc(n,u)   farcalloc(n,u)
    #define free(p)       farfree(p)
    #define realloc(p,n)  farrealloc(p,n)
  #endif

  #if defined(__TURBOC__) && !defined(__TOPAZ__)
    #define coreleft()    farcoreleft()
  #endif
#else
  unsigned cdecl coreleft(void);
  unsigned long cdecl farcoreleft(void);
#endif


#if defined(__WATCOMC__) || defined(__MSC__) || defined(_lint)

  #define farmalloc(n)    _fmalloc(n)
  #define farfree(p)      _ffree(p)
  #define farrealloc(p,n) _frealloc(p,n)
  void far *farcalloc(int n,int m);

  #ifdef _MSC_VER
    #if _MSC_VER >= 600
      #define farcalloc(a,b) _fcalloc(a,b)
    #endif /* _MSC_VER >= 600 */
  #endif /* _MSC_VER */

  #define da_year year
  #define da_day day
  #define da_mon month

  #define ti_min minute
  #define ti_hour hour
  #define ti_hund hsecond
  #define ti_sec second

  #define getdate _dos_getdate
  #define gettime _dos_gettime

  #define NO_STRFTIME

/*  #ifndef __WATCOMC__*/
  #define NO_MKTIME
/*  #endif*/

#elif defined(__TURBOC__)

  #define dosdate_t date
  #define dostime_t time

  #if (__TURBOC__ >= 0x0295) || defined(__TOPAZ__)
    /* TC++ and above include a strftime() function */
    #define NO_STRFTIME
    #define NO_MKTIME
  #endif

#endif
 
#if defined(__FLAT__)
    #undef farcalloc
    #undef farmalloc
    #undef farrealloc
    #undef farfree
    #undef _fmalloc

    #define farcalloc  calloc
    #define farmalloc  malloc
    #define farrealloc realloc
    #define farfree    free
    #define _fmalloc   malloc
#endif

#ifndef __TURBOC__

  /* For ERRNO definitions */
  #define ENOTSAM EXDEV

  int _stdc fnsplit(const char *path,char *drive,char *dir,char *name,char *ext);
  int _stdc getcurdir(int drive, char *directory);

  int fossil_wherex(void);
  int fossil_wherey(void);
  void fossil_getxy(char *row, char *col);

  #define textattr(attr)
  #define getdisk()                  get_disk()
  #define setdisk(drive)             set_disk(drive)

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
 
#ifdef OS_2
  void _fast vbuf_flush(void);
  void SnSetPipeName(char *pipename);
  void SnWrite(char *str);

  #define  Start_Shadow()
  #define  End_Shadow()
#else
  void pascal Start_Shadow(void);
  void pascal End_Shadow(void);
#endif

#ifdef __WATCOMC__  /* WC 9.0 mistakenly omits prototype for fdopen */
  #if __WATCOMC__==900
  FILE *fdopen(int __handle,const char *__mode);
  #endif
#endif



#ifndef cpp_begin
  #ifdef __cplusplus
    #define cpp_begin()   extern "C" {
    #define cpp_end()     }
  #else
    #define cpp_begin()
    #define cpp_end()
  #endif
#endif


#if !defined(UNIX) || !defined(HAVE_TIMER_T)
typedef long timer_t;
#endif

#define REGISTER

#ifndef TRUE
#define FALSE 0
#define TRUE 1
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

#if defined(UNIX)
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  if !defined(BIG_ENDIAN)
#   define BIG_ENDIAN
#  endif
#  undef LITTLE_ENDIAN
# else
#  if !defined(LITTLE_ENDIAN)
#   define LITTLE_ENDIAN
#  endif
#  undef BIG_ENDIAN
# endif
#else /* ! UNIX */
# if !defined(__POSIX__) && !defined(BIG_ENDIAN)
#  define LITTLE_ENDIAN           /* If compiling on a "back-words" (Intel)  *
                                 * Otherwise, #define BIG_ENDIAN           *//
# endif
#endif

#ifdef __MSDOS__
#define WILDCARD_ALL     "*.*"
#else
#define WILDCARD_ALL     "*"
#endif

#define _PRIVS_NUM         12   /* Maximum priv levels for Maximus         */
#define CHAR_BITS           8   /* Number of bits in a `char' variable     */

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

#if !defined(UNIX) 		 /* UNIX version will not use assembly code, so lets find errors during cpp */
# define INTBIT_C        0x0001  /* Carry */
# define INTBIT_P        0x0004  /* Parity */
# define INTBIT_AUX      0x0010  /* Aux carry */
# define INTBIT_Z        0x0040  /* Zero flag */
# define INTBIT_SIG      0x0080  /* Sign flag */
# define INTBIT_TRC      0x0100  /* Trace flag */
# define INTBIT_INT      0x0200  /* Interrupt flag */
# define INTBIT_D        0x0400  /* Direction flag */
# define INTBIT_OVF      0x0800  /* Overflow flag */
#endif

#define ZONE_ALL  56685u
#define NET_ALL   56685u
#define NODE_ALL  56685u
#define POINT_ALL 56685u


#define THIS_YEAR "2003"

#define Hello(prog,desc,version,year) (void)printf("\n" prog "  " desc ", Version %s.\nCopyright " year " by Lanius Corporation.  All rights reserved.\n\n",version)
#define shopen(path,access)   sopen(path,access,SH_DENYNONE,S_IREAD | S_IWRITE)
#define GTdate(s1, s2) (GEdate(s1, s2) && (s1)->ldate != (s2)->ldate)
#define carrier_flag          (prm.carrier_mask)
#define BitOff(a,x)           ((void)((a)[(x)/CHAR_BITS] &= ~(1 << ((x) % CHAR_BITS))))
#define BitOn(a,x)            ((void)((a)[(x)/CHAR_BITS] |= (1 << ((x) % CHAR_BITS))))
#define IsBit(a,x)            ((a)[(x)/CHAR_BITS] & (1 << ((x) % CHAR_BITS)))

/*#define lputs(handle,string)  write(handle,string,strlen(string))*/

#define dim(a)                (sizeof(a)/sizeof(a[0]))
#define eqstr(str1,str2)      (strcmp(str1,str2)==0)
#define eqstrn(str1,str2,n)   (strncmp(str1,str2,n)==0)

#define eqstri(str1,str2)	(stricmp(str1,str2)==0)
#define eqstrni(str1,str2,n)	(strnicmp(str1,str2,n)==0)

#define eqstrin(str1,str2,n)  eqstrni(str1,str2,n)

#define divby(num,div)        ((num % div)==0)
#define f_tolwr(c)            (_to_lwr[c])
#define f_toupr(c)            (_to_upr[c])

#include "growhand.h"

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


/* Don't change this struct!  The code in win_pick.c and max_locl.c relies  *
 * on it as being the same as PLIST...                                      */

struct __priv
{
  char *name;
  int priv;
};



extern char _vstdc months[][10];
extern char _vstdc weekday[][10];

extern char _vstdc months_ab[][4];
extern char _vstdc weekday_ab[][4];

extern struct __priv _vstdc _privs[];



#include "progprot.h"


#ifndef NO_STRFTIME
  /* If compiler doesn't include a strftime(), use our own */

  #include <time.h>
  #include <sys/types.h>

  size_t _stdc strftime(char *,size_t,const char *,const struct tm *);
#endif


#ifndef NO_MKTIME
  /* If compiler doesn't include a mktime(), use our own */

  #include <time.h>
  #include <sys/types.h>

  time_t _stdc mktime(struct tm * tm_ptr);
#endif

/* MS docs use both SH_DENYNONE and SH_DENYNO */

#if !defined(SH_DENYNONE) && defined(SH_DENYNO)
  #define SH_DENYNONE SH_DENYNO
#endif

#ifdef UNIX
unsigned long coreleft(void);
#endif

#if !defined(offsetof) && !defined(__WATCOMC__) && !defined(_MSC_VER) && !defined(__TURBOC__) && !defined(__IBMC__) && !defined(__TOPAZ__) && !defined(_lint)
#define offsetof(typename,var) (size_t)(&(((typename *)0)->var))
#endif

#ifdef __TURBOC__
#if __TURBOC__ <= 0x0200
#define offsetof(typename,var) (size_t)(&(((typename *)0)->var))
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PROG_H_DEFINED */

















