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

/*# name=Canonicalize filename or path
*/

#include <dos.h>
#include "prog.h"

#if defined(OS_2)

  #define INCL_DOS
  #include <os2.h>

  char * _fast canon(char *orig, char *dest)
  {
  #ifdef __FLAT__ /* OS/2 2.0 */
    return (DosQueryPathInfo(orig, FIL_QUERYFULLNAME, dest, 128) ? NULL : dest);
  #else
    return (DosQPathInfo(orig, FIL_QUERYFULLNAME, dest, 128, 0) ? NULL : dest);
  #endif
  }
#elif defined(NT)
  #include "pwin.h"

  char * _fast canon(char *orig, char *dest)
  {
    char *pszFinal;

    *dest='\0';
    return (GetFullPathName(orig, PATHLEN, dest, &pszFinal) ? dest : NULL);
  }

#elif defined(__MSDOS__)
  #if defined(__386__)

  char * _fast canon(char *orig, char *dest)
  {
    union REGS r;

    r.h.ah=0x60;
    r.x.esi=(unsigned)orig;
    r.x.edi=(unsigned)dest;

    int386(0x21, &r, &r);

    return (r.x.cflag ? NULL : dest);
  }

  #else /* normal 8086 dos */

  char * _fast canon(char *orig, char *dest)
  {
    union REGS r;
    struct SREGS s;

    r.h.ah=0x60;

    s.ds=  FP_SEG(orig);
    r.x.si=FP_OFF(orig);

    s.es=  FP_SEG(dest);
    r.x.di=FP_OFF(dest);

    int86x(0x21, &r, &r, &s);

    return (r.x.cflag ? NULL : dest);

  }

  #endif /* DOS/8086 */
#elif defined(UNIX)
#include <sys/param.h>
char *canon(char *orig, char *dest)
{
#warning interface ripe for buffer overrun
  char *o, *d;
  char rpath[MAXPATHLEN + 1];

  if (!dest)
    return orig;

  if (orig[1] == ':')
    orig += 2;

  for (o=orig, d=dest; *o; o++, d++)
  {
    if (*o == '\\')
      *d = '/';
    else
      *d = *o;
  }

  d = realpath(dest, rpath);
  if (d && (d != dest))
  {
    rpath[sizeof(rpath) - 1] = (char)0;
    strcpy(dest, d);
  }

  return dest;
}
#else
  #error Unknown OS
#endif

#ifdef TEST
main()
{
  char buf[128];

  printf("%s\n", canon("filename.ext", buf));
  printf("%s\n", canon("..\\filename.ext", buf));
  printf("%s\n", canon("..\\..\\filename.ext", buf));
  printf("%s\n", canon("C:config.sys", buf));
  printf("%s\n", canon("E:..\\para\\sample\\..\\..\\util\\config.sys", buf));
}
#endif

