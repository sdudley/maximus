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
static char rcs_id[]="$Id: s_sflo.c,v 1.3 2004/01/13 00:42:14 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "prog.h"
#include "squish.h"

#if 0

#define MAX_SFLO  255

#define BSTAT_NONE  'N'
#define BSTAT_BUSY  'B'


short OutToSflo(void)
{
  MATCHOUT *moOut;
  NETADDR naFind={ZONE_ALL, NET_ALL, NODE_ALL, POINT_ALL};
  FILE *fpSflo;
  byte flav;
  char szHeader[MAX_SFLO];
  char *szDomain="fidonet.org";
  char *szType;
  
#ifndef UNIX
  if ((fpSflo=shfopen("SUPERFLO.DAT", "w+b",
#else
  if ((fpSflo=shfopen("superflo.dat", "w+b",
#endif
                      O_CREAT | O_TRUNC | O_WRONLY | O_BINARY))==NULL)
  {
    (void)printf("Error creating SuperFLO!\n");
    return -1;
  }
  
  if ((moOut=MatchOutOpen(&config, &naFind, MATCH_ALL, 0))==NULL)
  {
    (void)fclose(fpSflo);
    return -1;
  }
  
  do
  {
    /* Find the file extension */

    szType=strrchr(moOut->name, '.');
    
    if (szType==NULL || strlen(++szType) < 3)
      continue;

    if (!eqstri(szType+1, "lo") &&
        !eqstri(szType+1, "ut") &&
        !eqstri(szType+1, "eq"))
       continue;

    switch(tolower(szType[0]))
    {
      case 'c': flav='C'; break;
      case 'h': flav='H'; break;
      case 'd': flav='D'; break;
      default:
      case 'o':
      case 'f': flav='N'; break;
    }

    (void)sprintf(szHeader, "%c,%s@%s,%s,%c,",
                  BSTAT_NONE,
                  Address(&moOut->found),
                  szDomain,
                  eqstr(szType+1, "LO") ? "FLO" :
                    eqstri(szType+1, "UT") ? "OUT" : szType,
                  flav);

    if (eqstri(szType+1, "LO") || eqstr(szType, "REQ"))
    {
      FILE *fpFlo=shfopen(moOut->name, "r", O_RDONLY);
      char szLbuf[MAX_SFLO];
      
      if (fpFlo==NULL)
      {
        (void)printf("Error opening %s file `%s'\n", szType, moOut->name);
        continue;
      }
      
      while (fgets(szLbuf, MAX_SFLO, fpFlo))
      {
        if (eqstri(szType+1, "LO"))
        {
          (void)fprintf(fpSflo, "%s%c,%s\n", szHeader,
                        *szLbuf=='#' ? 'T' : *szLbuf=='~' ? 'K' : 'N',
                        Strip_Trailing(szLbuf, '\n')+
                          (*szLbuf=='~' || *szLbuf=='#'));
        }
        else
        {
          (void)fprintf(fpSflo, "%s%s\n", szHeader, Strip_Trailing(szLbuf, '\n'));
        }
      }
          
      (void)fclose(fpFlo);
    }
    else if (eqstr(szType+1, "UT"))
      (void)fprintf(fpSflo, "%s%s\n", szHeader, moOut->name);
  }
  while (MatchOutNext(moOut));
  
  (void)fclose(fpSflo);

  return 0;
}

#endif

