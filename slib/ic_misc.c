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

#include "prog.h"
#include "progprot.h"

#define INCL_DOS
#include <os2.h>

int _fast lock(int fh, long offset, long len)
{
  FILELOCK fl;

  fl.lOffset=offset;
  fl.lRange=len;

  return (DosSetFileLocks((HFILE)fh, NULL, &fl, 0, 0)==0 ? 0 : -1);
}

int _fast unlock(int fh, long offset, long len)
{
  FILELOCK fl;

  fl.lOffset=offset;
  fl.lRange=len;

  return (DosSetFileLocks((HFILE)fh, &fl, NULL, 0, 0)==0 ? 0 : -1);
}


