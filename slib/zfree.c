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

/*# name=Calculates amount of drive space free
    credit=Thanks to Peter Fitzsimmons for the OS/2 version of this routine.
*/

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <dos.h>
#include "prog.h"

static long CalcFreeSpace(long lSectorsPerCluster, long lBytesPerSector, long lFreeClusters);

#if defined(OS_2)

  #define INCL_NOPM
  #include <os2.h>

  long _stdc zfree(char *drive)
  {
    int driveno;
    FSALLOCATE dt;

    if (!drive || !*drive || drive[1] != ':')
      driveno=0;                             /* Default drive */
    else
      driveno=(unsigned char)(toupper(*drive)-'A'+1);

    DosQFSInfo(driveno, 1, (char far *)&dt, sizeof(FSALLOCATE));

    return CalcFreeSpace(dt.cSectorUnit, dt.cbSector, dt.cUnitAvail);
  }

#elif defined(__MSDOS__)

  long _stdc zfree (char *drive)
  {
    union REGS r;
    unsigned char driveno;

    if (!drive || !*drive || drive[1] != ':')
      driveno=0;                             /* Default drive */
    else
      driveno=(unsigned char)(toupper(*drive)-'A'+1);

  #ifdef __386__
    r.x.eax=0x3600;
    r.h.dl=driveno;                             /* on this drive */
    intdos(&r, &r);                             /* go do it */

    if (r.x.eax==0xffffffffu)                   /* error return? */
      return 0L;

    return((long)r.x.ebx*(long)r.x.eax*(long)r.x.ecx);
  #else
    r.x.ax=0x3600;                              /* get free space */
    r.h.dl=driveno;                             /* on this drive */
    intdos(&r,&r);                              /* go do it */

    if (r.x.ax==0xffff)                         /* error return? */
      return 0L;

    return CalcFreeSpace((long)r.x.bx, (long)r.x.ax, (long)r.x.cx);
  #endif
  }

#elif defined(NT)
  #include "pwin.h"

  long _stdc zfree(char *path)
  {
    char szDrive[PATHLEN];
    DWORD lSectorsPerCluster;
    DWORD lBytesPerSector;
    DWORD lFreeClusters;
    DWORD lClusters;

    if (path && path[1]==':')
      sprintf(szDrive, "%c:\\", *path);
    else
    {
      GetCurrentDirectory(PATHLEN, szDrive);

      if (szDrive[1]==':')
        szDrive[3]=0;
    }

    GetDiskFreeSpace(szDrive, &lSectorsPerCluster, &lBytesPerSector,
                     &lFreeClusters, &lClusters);

    return CalcFreeSpace(lSectorsPerCluster, lBytesPerSector, lFreeClusters);
  }
#else
  #error Unknown OS
#endif



static long CalcFreeSpace(long lSectorsPerCluster, long lBytesPerSector, long lFreeClusters)
{
  long rc;
  long lBytesPerCluster;

  lBytesPerCluster = lSectorsPerCluster * lBytesPerSector;
  rc = lFreeClusters * lBytesPerCluster;

  /* Saturate the result by checking the value in megs */

  if (lBytesPerCluster >= 1024)
  {
    lBytesPerCluster >>= 10;
    lFreeClusters >>= 10;
  }
  else
  {
    lFreeClusters >>= 12;
    lBytesPerCluster >>= 8;
  }

  if (lBytesPerCluster * lFreeClusters > rc ||
      ((lBytesPerCluster * lFreeClusters) & 0xfffff000))
  {
    rc = INT_MAX;
  }

  return rc;
}
