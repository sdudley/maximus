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

#pragma off(unreferenced)
static char rcs_id[]="$Id: m_util.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "max_msg.h"
#include "dr.h"

int Make_Clean_Directory(char *szDirName)
{
  Strip_Trailing(szDirName, PATH_DELIM);
  fixPathMove(szDirName);

  /* Make sure that it exists */

  if (!direxist(szDirName))
    if (mkdir(szDirName) != 0)
    {
      logit(cantcreate, szDirName);
      return -1;
    }

  /* Add the final trailing backslash */

  Add_Trailing(szDirName, PATH_DELIM);

  Clean_Directory(szDirName, FALSE);
  return 0;
}

extern char * qwk_path;

void Clean_Directory(char * szDirName, int rdir)
{
  FFIND *ff;
  char temp[PATHLEN];
  char *dot;
  int isQwk;

  fixPathMove(szDirName);
  isQwk=(szDirName==qwk_path);

  /* Now clear it out of any excess garbage by deleting *.* */

  sprintf(temp, "%s" WILDCARD_ALL, szDirName);

  if ((ff=FindOpen(temp,0)) != NULL)
  {
    do
    {

      /* Don't delete the <boardname>.QWK file, since we just created it */

      if (isQwk && !rdir &&
          eqstrin(ff->szName, PRM(olr_name), strlen(PRM(olr_name))) &&
          (dot=strrchr(ff->szName, '.')) &&
          eqstrin(dot+1, "QW", 2))
        continue;

      strcpy(temp, szDirName);
      strcat(temp, ff->szName);

      unlink(temp);
    }
    while (FindNext(ff)==0);

    FindClose(ff);
  }

  /* If we're s'posta remove the directory, too */

  if (rdir)
  {
    strcpy(temp, szDirName);
    Strip_Trailing(temp, PATH_DELIM);
    rmdir(temp);
  }

}

