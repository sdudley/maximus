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
static char rcs_id[]="$Id: f_kill.c,v 1.1 2002/10/01 17:51:05 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: K)ill function
*/

#include <stdio.h>
#include <mem.h>
#include <io.h>
#include <string.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"

void File_Kill(void)
{
  byte filename[PATHLEN];
  byte filespec[PATHLEN];
  byte temp[PATHLEN];

  WhiteN();

  InputGets(filename, file_to_kill);

  Strip_Path(filename);

  if (! *filename)
    return;

  sprintf(filespec, ss, FAS(fah, downpath), filename);

  if (! fexist(filespec))
  {
    if (IsInFilesBbs(&fah, filename, NULL, NULL))
      Remove_Files_Entry(filename, NULL);
    else Printf(cantfind+1, filespec);

    return;
  }


  sprintf(temp, delete_yn, filename);

  if (GetyNAnswer(temp, 0)==YES)
  {
    if (unlink(filespec)==-1)
    {
      Printf(cant_unlink, filespec);
      Press_ENTER();
    }
    else Remove_Files_Entry(filename, NULL);
  }
  else
  {
    /* if user doesn't want to kill file, but wants to remove entry */

    if (GetyNAnswer(lfbrfbe, 0)==YES)
      Remove_Files_Entry(filename, NULL);
  }
}


