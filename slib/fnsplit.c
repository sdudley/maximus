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

/*# name=Replacement for BoreLand's fnsplit() function
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "dr.h"

#ifndef __TURBOC__

#define WILDCARDS   0x01
#define EXTENSION   0x02
#define FILENAME    0x04
#define DIRECTORY   0x08
#define DRIVE       0x10

#endif

#if defined(__MSC__) || defined(__WATCOMC__)
#define MAXPATH _MAX_PATH
#endif

#if defined(UNIX)
# include <sys/param.h>
# define MAXPATH MAXPATHLEN
#endif

#ifdef __TOPAZ__
int         _RTLENTRYF _EXPFUNC fnsplit( const char * path,
                            char * drive,
                            char * dir,
                            char * name,
                            char * ext )
#else
int _stdc fnsplit(const char *path,char *drive,char *dir,char *name,char *ext)
#endif
{
  int flag=FILENAME,
      x;

  char temp[MAXPATH],
       *s;

  if (strchr(path,':'))
  {
    if (drive)
    {
      strncpy(drive,path,2);
      drive[2]='\0';
    }

    flag |= DRIVE;
    path += 2;
  }
  else if (drive)
    *drive='\0';

  s = strrchr(path, PATH_DELIM);
#if defined(UNIX)
  if (!s)
    s = strrchr(path, '\\'); /* maybe dos path? */
#endif

  if (s != NULL)
  {
    if (dir)
    {
      strncpy(dir,path,x=(int)(s-(char *)path)+1);
      dir[x]='\0';
    }

    flag |= DIRECTORY;
    path=s+1;
  }
  else if (dir)
    *dir='\0';

  strcpy(temp,path);

  if ((s=strchr(temp,'.')) != NULL)
  {
    if (ext)
      strcpy(ext,s);

    flag |= EXTENSION;
    *s='\0';
  }
  else if (ext)
    *ext='\0';

  if (name)
    strcpy(name,temp);

  return flag;
}









