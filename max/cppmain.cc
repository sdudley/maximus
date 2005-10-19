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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: cppmain.cc,v 1.4 2005/10/19 10:57:09 paltas Exp $";
#pragma on(unreferenced)
#endif

#include "prog.h"

extern "C" { 
	     int _stdc c_main(int argc, char *argv[]); 
	     unsigned far _ovl_openflags(unsigned sharing_mode); 
}

int main(int argc, char *argv[])
{
#if defined(__WATCOMC__) && defined(__MSDOS__) && !defined(ORACLE) && defined(OVERLAY_BUILD)
  /* Tell the overlay manager to open the file in SH_DENYNO mode */
  _ovl_openflags(0x04);
#endif

  return c_main(argc, argv);
}

