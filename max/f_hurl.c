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
static char rcs_id[]="$Id: f_hurl.c,v 1.1 2002/10/01 17:51:03 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: H)url function
*/

#define MAX_LANG_m_area

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"

void File_Hurl(void)
{
  BARINFO bi;
  FAH fa={0};

  FILE *outfilesbbs;

  byte an[MAX_ALEN];

  byte filename[PATHLEN];
  byte fbbs[PATHLEN];
  byte temp[MAX_FBBS_ENTRY];
  byte from[PATHLEN];
  byte to[PATHLEN];

  WhiteN();

  InputGets(filename, hurl_what);

  Strip_Path(filename); /* Make sure user doesn't specify path or device */

  if (! *filename || ! File_Okay(filename))
    return;


  /* Figure out where to hurl it to */

  do
  {
    InputGets(temp, which_area);

    if (eqstri(temp, qmark))
      ListFileAreas(NULL, FALSE);
  }
  while (eqstri(temp, qmark));

  if (! *temp)
    return;

  SetAreaName(an, temp);

  memset(&fa, 0, sizeof fa);

  if (!ReadFileArea(haf, an, &fa) ||
      !ValidFileArea(NULL, &fa, VA_PWD, &bi))
  {
    DisposeFah(&fa);

    if (*PRM(areanotexist))
      Display_File(0, NULL, PRM(areanotexist));
    else Puts(areadoesntexist);
    
    *linebuf='\0';

    return;
  }

  /* Find out the name to use for the specified area's FILES.BBS list */

  if (*FAS(fa, filesbbs))
    strcpy(fbbs, FAS(fa, filesbbs));
  else sprintf(fbbs, ss, FAS(fa, downpath), files_bbs);


  /* Open it for appending */

  if ((outfilesbbs=shfopen(fbbs, fopen_append, O_WRONLY | O_APPEND))==NULL)
  {
    cant_open(fbbs);
    DisposeFah(&fa);

    *linebuf='\0';
    return;
  }

  /* Now move the actual file... */

  sprintf(from, ss, FAS(fah, downpath), filename);
  sprintf(to, ss, FAS(fa, downpath), filename);

  Printf(hurled_file, from, to);

  if (move_file(from, to)==-1)
    cant_open(to);
  else
  {
    /* Remove it from OUR files.bbs, and put the description into temp. */

    *temp='\0';
    Remove_Files_Entry(filename, temp);

    /* ...and put it in theirs. */

    if (*temp)
      fputs(temp, outfilesbbs);
    else fprintf(outfilesbbs, "%s\n", upper_fn(filename));

    Puts(done_ex);
  }

  fclose(outfilesbbs);
  
  DisposeFah(&fa);
  return;
}

