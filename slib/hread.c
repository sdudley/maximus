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


#include "uni.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"

#if defined(__SMALL__) || defined(__MEDIUM__)

#else

  #ifdef OS_2
    #define INCL_DOSMEMMGR
    #include "pos2.h"
  #endif

  long _fast h_read(int fd, char huge *buf, long size)
  {
    int ret;
    long rc=0;

    while (size > 0)
    {
      unsigned long ulMax=65536L - (word)(unsigned)(char near *)buf;
      ulMax=min(ulMax, size);
      ulMax=min(ulMax, 32767L);

      if (!size)
        return 0;

      if (!ulMax)
        return 0;

      ret=read(fd, (char *)buf, (unsigned)ulMax);

      if (ret < 0)
        return (long)ret;

      /* Add the number of bytes transferred */

      buf += (long)ret;
      rc += (long)ret;
      size -= ulMax;

      if (rc==0)
        break;
    }

    return rc;
  }

  long _fast h_write(int fd, char huge *buf, long size)
  {
    int ret;
    long rc=0;

    while (size > 0)
    {
      unsigned long ulMax=65536L - (word)(unsigned)(char near *)buf;
      ulMax=min(ulMax, size);
      ulMax=min(ulMax, 32767L);

      if (!size)
        return 0;

      if (!ulMax)
        return 0;

      ret=write(fd, (char *)buf, (unsigned)ulMax);

      if (ret < 0)
        return (long)ret;

      /* Add the number of bytes transferred */

      buf += (long)ret;
      rc += (long)ret;
      size -= ulMax;

      if (rc==0)
        break;
    }

    return rc;
  }
#endif

