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


#define INCL_DOS
#define INCL_DOSERRORS
#include <os2.h>
#include <string.h>
#include "ffind.h"

/* A "bound" replacement for DosQPathInfo that only supports FIL_STANDARD */

USHORT APIENTRY DosQPathInfo(PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
                             USHORT cbInfoBuf, ULONG ulReserved)
{
  FFIND *ff;
  FILESTATUS fs;

  NW(ulReserved);

  if (usInfoLevel != FIL_STANDARD)
    return ERROR_INVALID_LEVEL;

  if ((ff=FindOpen((char *)pszPath, 0))==NULL)
    return ERROR_PATH_NOT_FOUND;

  fs.attrFile=ff->usAttr;
  fs.cbFile=ff->ulSize;

  *((USHORT *)&fs.ftimeLastWrite)=ff->scWdate.dos_st.time;
  *((USHORT *)&fs.fdateLastWrite)=ff->scWdate.dos_st.date;

  *((USHORT *)&fs.ftimeCreation)=ff->scCdate.dos_st.time;
  *((USHORT *)&fs.fdateCreation)=ff->scCdate.dos_st.date;

  *((USHORT *)&fs.ftimeLastAccess)=ff->scAdate.dos_st.time;
  *((USHORT *)&fs.fdateLastAccess)=ff->scAdate.dos_st.date;

  FindClose(ff);

  memmove(pInfoBuf, &fs, min(sizeof fs, cbInfoBuf));
}

