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

#ifndef _COMPILER_ALIGN_H
#define _COMPILER_ALIGN_H
/* Packing the structures with the compiler allows us a certain
 * measure of binary compatibility with files (lastuser.bbs,
 * fidonet packets, squish bases, user.bbs, ipc.bbs...) written
 * on other platforms. The structures in the Maximus3_02 release
 * literally describe the "disk version", and read/written 
 * directory from/to disk. This is not portable, but fudging it
 * helps under x86 and Alpha(linux only). Unfortunately, other
 * CPUs (e.g. sparc) blow up when you try to do things like
 * access a misaligned pointer to a union of structs.. (SIGBUS)
 * ..so we don't support packing under any circumstances on those
 * CPUs.
 */

#if defined(__GNUC__)
# if !defined(__sparc)
#  define PACKED         __attribute__((packed))
#  define ALIGNED        __attribute__((aligned))
#  define PACKED_ALIGNED __attribute__((packed, aligned))
# else
#  ifdef SOLARIS
#   include <sys/isa_defs.h>
#  endif
#  define PACKED
#  ifdef _MAX_ALIGNMENT
#   define PACKED_ALIGNED        __attribute__((aligned(_MAX_ALIGNMENT * 8)))
#   define ALIGNED               __attribute__((aligned(_MAX_ALIGNMENT * 8)))
#  else
#   define PACKED_ALIGNED        __attribute__((aligned))
#   define ALIGNED               __attribute__((aligned))
#  endif
# endif
#else
# ifdef UNIX
#  warning Unrecognized compiler; structure packing may be incorrect!
# endif
# define ALIGNED 
# define PACKED_ALIGNED 
# define PACKED 
#endif

#if defined(__sparc) || defined(sparc) || defined(__mips) || defined(__PPC__) || defined(__powerpc__)
# define SLOPPY_ALIGNMENT_OKAY  0
#endif
 
#if defined(__alpha__)
# if defined(LINUX)
#  define SLOPPY_ALIGNMENT_OKAY 1       /* Let the kernel trap it */
# else
#  define SLOPPY_ALIGNMENT_OKAY 0
# endif
#endif
 
#if defined(__i386) || defined(i386) || defined(I386) || defined(__386__)
# define SLOPPY_ALIGNMENT_OKAY  1
#endif
 
#if defined(__x86) || defined(x86) || defined(__x86__)
# define SLOPPY_ALIGNMENT_OKAY  1
#endif
 
#if !defined(SLOPPY_ALIGNMENT_OKAY)
# if defined(__WATCOMC__) || defined(MSC_VER) || defined(__TURBOC__) || defined(__IBMC__) || defined(__TOPAZ__)
#  define SLOPPY_ALIGNMENT_OKAY 1
# endif
#endif
    
#if !defined(SLOPPY_ALIGNMENT_OKAY)
# define SLOPPY_ALIGNMENT_OKAY 0        /* default: slow but safe */
#endif

#if defined(_GNUC_)
# define returnCast(a)	(typeof(a))
# define alignof(type)	offsetof(struct { char c; type member; }, member}
#else
# define returnCast(a)	
#endif

#if !SLOPPY_ALIGNMENT_OKAY
void *_aligndup(const void *unaligned, size_t alignment, size_t size);
void _unaligndup_free(void *unaligned, void *aligned, size_t size);
void *_alignStatic(const void *unaligned, size_t size);
void _unalign(void *unaligned, void *aligned, size_t size);

# define aligndup(a)		returnCast(a) _aligndup(a, alignof(*(a)), sizeof(*(a))))
# define unaligndup_free(a,b)	_unalignfree(a, b, sizeof(*(a)))
# define alignStatic(a)		returnCast(a) _alignStatic(a, sizeof(*(a)))
# define unalign(a,b)		_unalign(a, b, sizeof(*(a)))
#else
# define aligndup(a)		(a + 0)
# define unaligndup_free(a)	do { ; } while(0)
# define alignStatic(a)		(a + 0)
# define unalign(a)		do { ; } while(0)
#endif

#if !defined(_MAX_ALIGNMENT)
# if defined _GNUC_
static union
{
  char		c;
  short		sh;
  int		i;
  double	d;
  float		f;
  struct 
  { 
    char a[1];
  }		st;
  void		*v;
} alignmentTest;
#  define _MAX_ALIGNMENT alignof(alignmentTest)
# else
#  define _MAX_ALIGNMENT sizeof(long) /* just a guess, but often right */
# endif
#endif

#endif /* _COMPILER_ALIGN_H */







