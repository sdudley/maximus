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

/* $Id: f_over.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=File area routines: O)verride Path function
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"

void File_Override_Path(void)
{
  byte path[PATHLEN];

  WhiteN();

  InputGets(path, full_ovr_path);

  if (! *path)
    return;
  
  if (! direxist(path))
  {
    Printf(cantfind, path);
    return;
  }

  Add_Trailing(path, '\\');

  strcpy(fah.heap + fah.fa.cbHeap, path);
  fah.fa.downpath=fah.fa.cbHeap;
  fah.fa.uppath=fah.fa.cbHeap;
  fah.fa.descript=fah.fa.cbHeap;
}

