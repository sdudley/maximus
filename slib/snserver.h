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


#ifndef APIENTRY
#include <os2def.h>
#endif

#ifndef CALLBACK
  #ifdef __FLAT__
    #define CALLBACK pascal __far16
  #else
    #include "os2fubar.h"
  #endif
#endif

#if !defined(_QC)
#pragma comment(lib, "snserver.lib")
#endif

typedef SEL  HSNOOP;
typedef PSEL PHSNOOP;
typedef int   (pascal far  *PFNSN)(int flag, char far *str);


USHORT CALLBACK SnoopOpen(PSZ pszPipeName,
                          PHSNOOP phSnoop,
                          PSZ pszAppName,
                          PFNSN NotifyFunc);


USHORT CALLBACK SnoopWrite(HSNOOP hsn, PSZ pszMessage);

USHORT CALLBACK SnoopClose(HSNOOP hsn);
