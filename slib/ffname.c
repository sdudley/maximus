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

/*# name=Expand a filename to its full path
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dos.h>
#include <stdlib.h>
#include "prog.h"
#include "dr.h"

char * _fast make_fullfname(char *path)
{
  static char full[PATHLEN*3];
  char dir[PATHLEN];
  char *updir;
  char *supdir;
  int len;

  if (*path=='$')
    path++;

  /* Return UNC paths untouched */

  if (path[0]==PATH_DELIM && path[1]==PATH_DELIM)
  {
    strcpy(full, path);
    return full;
  }

  if (path[1] != ':')
  {
    full[0]=(char)(getdisk()+'A');
    full[1]=':';
    full[2]=path[0];
  }
  else
  {
    full[0]=path[0];
    full[1]=':';
    full[2]=path[2];
  }

  if (full[2] != PATH_DELIM)
  {
#ifndef UNIX
    getcurdir((toupper(full[0])-'A')+1, dir);
#else
    {
      char *s = getcwd(dir, sizeof(dir));
      if (!s)
        *dir = (char)0;
    }
#endif

    if (*dir)
    {
      full[2]=PATH_DELIM;
      strcpy(full+3, dir);
    }
    else full[2]='\0';

    strcat(full, PATH_DELIMS);
    strcat(full, path+(path[1]==':' ? 2 : 0));
  }
  else
  {
    strcpy(full+2, path+(path[1]==':' ? 2 : 0));
  }

  while ((updir=strstr(full, PATH_DELIMS "." PATH_DELIMS)) != NULL)
  {
    if (updir[2] != '\0')
      memmove(updir, updir+2, strlen(updir)+1);
    else *updir='\0';
  }

  while ((supdir=updir=strstr(full,"..")) != NULL)
  {
    for (updir -= 2; (updir > full) && (*updir != PATH_DELIM); updir--)
      ;

    if (updir==full)
      break;

    memmove(updir, supdir+2, strlen(updir)+1);

    if (full[2]=='\0')
      strcat(full, PATH_DELIMS);
  }

  while ( ((updir=strstr(full, PATH_DELIMS ".")) != NULL) ||
          ((updir=strstr(full, "." PATH_DELIMS)) != NULL) )
  {
    if (updir[2] != '\0')
      memmove(updir, updir+2, strlen(updir)+1);
    else *updir='\0';
  }

  if (full[len=strlen(full)-1]=='.')
    full[len]='\0';

  fixPathMove(full);
  return full;
}

