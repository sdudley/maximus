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

/*# name=Direct-video routines for Maximus
*/

#include "prog.h"
#include "dv.h"

#ifdef __MSDOS__

int _fast VidNumRows(void)
{
  return Vid_NumRows;
}

int _fast VidNumCols(void)
{
  return Vid_NumCols;
}

int _fast VidWhereX(void)
{
  return Vid_Col+1;
}

int _fast VidWhereY(void)
{
  return Vid_Row+1;
}

char _fast VidGetPage(void)
{
  return Vid_Page;
}

#endif /* __MSDOS__ */

