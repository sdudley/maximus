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

/*# name=make_dir() -- a make-direction function which will make entire
    name=directory trees.  (In other words, given the path
    name=`C:\USER\HOBWITZ\TEST\NATCH\DIR1', it will create the entire
    name=tree, even if \USER (or any of the intermediate directories)
    name=don't exist.)
*/

#include <string.h>
#include "prog.h"
#include "dr.h"

int _fast make_dir(char *dirC)
{
  char temp[PATHLEN],
       save[PATHLEN],
       *s;
  char *dir;
  int x;

  dir = fixPathDup(dirC);

  strcpy(save,dir);

  if (save[x=strlen(save)-1]=='\\' && strlen(save) != 3)
    save[x]='\0';

  strcpy(s=temp,save);

  while (s)
  {
    if ((s=strchr(s,'\\'))==NULL)
      break;

    if (s==temp || (s==temp+2 && *temp && temp[1]==':'))
    {
      s++;
      continue;
    }

    *s++='\0';

    if (!direxist(temp) && mkdir(temp)==-1)
    {
      fixPathDupFree(dirC, dir);
      return -1;
    }

    strcpy(temp,save);
  }

  fixPathDupFree(dirC, dir);
  return (mkdir(temp));
}

