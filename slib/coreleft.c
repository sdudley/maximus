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

/* We need our own coreleft() for all compilers except TC.  Note that
   Topaz also needs a coreleft since it doesn't come with one either. */

#include <dos.h>
#include "prog.h"
#include "alc.h"

/* Only include coreleft if we're not using TC/DOS */

#if defined(__TOPAZ__) || !defined(__TURBOC__)

/* borland is nuts having a function that changes its return
 * type based on the memory model....but here goes
 */

  #ifdef __FARDATA__
    #if defined(OS_2)
      #define INCL_DOS
      #include <pos2.h>

      #ifdef __FLAT__
        unsigned long cdecl coreleft(void)
        {
          ULONG ulMem;

          DosQuerySysInfo(QSV_TOTAVAILMEM, QSV_TOTAVAILMEM, &ulMem, sizeof ulMem);
          return ulMem;
        }

      #else /* !FLAT */

      #ifdef __FARDATA__
        unsigned long cdecl coreleft(void)
      #else
        unsigned cdecl coreleft(void)
      #endif
        {
          unsigned long ul;

          DosMemAvail(&ul);

          if (ul < 0x10000L)
            ul = 0x10000L;    /* Lie, just in case we are using coreleft() as
                               * the basis for a malloc() call. OS/2 will make
                               * room for us if need be
                               */
          return(ul);
        }

      #endif

    #elif defined(NT)

      #include "pwin.h"

      unsigned long cdecl coreleft(void)
      {
        MEMORYSTATUS ms;

        /* Return amount of free physical memory */

        GlobalMemoryStatus(&ms);
        return ms.dwAvailPhys;
      }

    #elif defined(__MSDOS__)
      #if defined(__FLAT__)
        #include <dos.h>

        /* DOS-flat-far coreleft */

        unsigned long cdecl coreleft(void)
        {
          struct _dpmi_inf
          {
            int largest;
            int max_unlocked;   /* pages */
            int max_block;      /* pages */
            int linear_space;   /* pages */
            int free_pages;     /* pages */
            int unused_pages;   /* pages */
            int physical_pages; /* pages */
            int linear_free;    /* pages */
            int swap_size;      /* pages */
            int rsvd[3];
          } dinf;

          union REGS r;

          r.x.edi=(int)&dinf;
          r.w.ax=0x0500;
          int386(0x31, &r, &r);

          return dinf.largest;
        }
      #else

        /* DOS-real-far coreleft */

        unsigned long cdecl coreleft(void)
        {
          union REGS r;
          struct SREGS sr;

          r.h.ah=0x48;
          r.x.bx=0xffff;
          int86(0x21, &r, &r);

          if (!r.x.cflag)
          {
            r.h.ah=0x49;
            sr.es=r.x.ax;
            int86(0x21, &r, &r);
            r.x.bx=0xffff;
          }

          return (r.x.bx * 16L);
        }
      #endif
    #elif defined(UNIX)

#include <unistd.h>

unsigned long coreleft(void)
{
  long pagesAvail;
  long pageSize;

  pageSize = sysconf(_SC_PAGE_SIZE);
  pagesAvail = sysconf(_SC_AVPHYS_PAGES);

  return pageSize * pagesAvail;
}
    #else
      #error unknown operating system
    #endif
  #else /* __NEARDATA__ */

    /* real-near coreleft */

    unsigned cdecl coreleft(void)
    {
      #ifdef __WATCOMC__

        struct _heapinfo entry;
        unsigned left;

        left=0;
        entry._pentry=NULL;

        while (1)
        {
          if (_heapwalk(&entry) != _HEAPOK)
            break;

          if (entry._useflag==_FREEENTRY)
            left += entry._size;
        }

        return (left);
      #else
        return (_memavl());
      #endif
    }
  #endif /* __NEARDATA__ */
#endif  /* !__TURBOC__ */

#ifdef TEST_HARNESS
#include <stdio.h>

int main(void)
{
  long ul=coreleft();

  printf("core=%ld\n", coreleft());
}
#endif




