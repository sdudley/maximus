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

/*# name=MSC-specific functions
    credit=Thanks to Peter Fitzsimmons for this module
*/

#ifndef UNIX
#include <errno.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include "prog.h"


/* MSC replacement for the TC file locking functions. */

#include <io.h>
#include <stdio.h>
#include <sys/locking.h>

int _fast lock(int fh, long offset, long len)
{
    if(-1L != lseek(fh, offset, SEEK_SET))
        return locking(fh, LK_NBLCK, len);
    else
        return(-1);
}

int _fast unlock(int fh, long offset, long len)
{
    if(-1L != lseek(fh, offset, SEEK_SET))
        return locking(fh, LK_UNLCK, len);
    else
        return(-1);
}

#endif
