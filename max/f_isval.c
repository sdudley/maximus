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
static char rcs_id[]="$Id: f_isval.c,v 1.1.1.1 2002/10/01 17:51:05 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Routine to validate file areas
*/

#define MAX_LANG_max_main
#include <string.h>
#include "prog.h"
#include "max_file.h"

static int near _ValidFileArea(PFAH pfah, unsigned flags, BARINFO *pbi)
{
  char *bar;

  if ((flags & VA_OVRPRIV)==0 && !PrivOK(PFAS(pfah, acs), TRUE))
    return FALSE;

  /* Make sure there's a file area here... */
      
  if (isblstr(PFAS(pfah, downpath)))
    return FALSE;

  if ((flags & VA_OVRPRIV)==0 &&
      *(bar=PFAS(pfah, barricade)) && (flags & VA_PWD))
  {
    if (!GetBarPriv(bar, FALSE, NULL, pfah, pbi, !!(flags & VA_EXTONLY)))
      return FALSE;
  }

  return TRUE;
}


int ValidFileArea(char *name, FAH *pfah, unsigned flags, BARINFO *pbi)
{
  FAH fa;
  int rc;

  memset(&fa, 0, sizeof fa);
  pbi->use_barpriv=FALSE;

  /* Use the current area, if supplied */

  if (pfah)
    fa=*pfah;
  else if (!ReadFileArea(haf, name, &fa))
    return FALSE;

  rc=_ValidFileArea(&fa, flags, pbi);

  if (!pfah)
    DisposeFah(&fa);

  return rc;
}


void ForceGetFileArea(void)
{
  BARINFO bi;
  int bad=FALSE;  /* To ensure that the begin_filearea is valid */
  int first;

  do
  {
    first=TRUE;

    /* Enable the VA_PWD check only on the first time through this loop.
     * On second and subsequent times, the password checking will be done
     * within File_Area itself, but we need to prompt for the password
     * at least once.
     */

    while (!ValidFileArea(usr.files, NULL, VA_VAL | (first ? VA_PWD : 0), &bi)||
           bad)
    {
      first=FALSE;
      Puts(inval_cur_file);
      File_Area();
      return;       /* File_Area will have pushed a valid area on the stack */
    }

    bad=TRUE;
  }
  while (!PushFileArea(usr.files, &bi));
}


