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

/*# name=Save and restore-directory functions
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"
#include "dr.h"
#include "alc.h"

int _fast Save_Dir(char *orig_disk,char *orig_path[],char *path)
{
  char temp[PATHLEN];
  char dn;
  char *p;

  if (*orig_disk==-1)
    *orig_disk=(char)getdisk();

  strcpy(temp,path);

  if (isalpha(*temp) && temp[1]==':')
    dn=(char)(toupper(*temp)-'A');
  else
    dn=(char)getdisk();

  if (! orig_path[dn])  /* Do this if it's NULL, otherwise skip it */
  {
    setdisk(dn);

    if (getdisk() != dn) /* If we didn't change drives successfully... */
      return 0;          /* We return 0 because -1 indicates out of memory */
    else
    {
      if ((orig_path[dn]=(char *)malloc(PATHLEN))==NULL)
        return -1;

      if (getcwd(orig_path[dn], PATHLEN)==NULL)
      {
        free(orig_path[dn]);
        orig_path[dn]=NULL;
      }
    }
  }

  if (strlen(temp) > 3 && temp[strlen(temp)-1]=='\\')
    temp[strlen(temp)-1]='\0';

  /* Now change to the right directory... */
  if ((p=strchr(temp, ':')) != NULL)
  {
    if (p==temp+1)
    {
      setdisk(toupper(*temp)-'A');
      chdir(temp+2);
    }
  }
  else chdir(temp);

  return 0;
}



void _fast Restore_Dir(char *orig_disk,char *orig_path[])
{
  int x;

  for (x=0;x < MAX_DRIVES;x++)
  {
    if (orig_path[x] && *orig_path[x])
    {
      chdir(orig_path[x]);

      free(orig_path[x]);
      orig_path[x]=NULL;
    }
  }

  setdisk(*orig_disk);

  *orig_disk=-1;
}




