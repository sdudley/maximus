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

/* $Id: f_raw.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=File area routines: R)aw function
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"

void File_Raw(void)
{
  FFIND *ff;
  union stamp_combo stamp;

  byte filename[PATHLEN];
  byte filespec[PATHLEN];
  byte size[PATHLEN];
  byte date[PATHLEN];
  byte nonstop;

  int ret;


  if (! *linebuf)
    Display_File(0, NULL, "%sRAWDIR", FAS(fah, downpath));

  WhiteN();
  
  InputGets(filename, fname_mask);

  WhiteN();

  Strip_Path(filename);

  if (! *filename)  /* If user pressed Enter */
    strcpy(filename, "*.*");

  sprintf(filespec, ss, FAS(fah, downpath), filename);

  display_line=display_col=1;
  nonstop=FALSE;

  for (ff=FindOpen(filespec, ATTR_SUBDIR), ret=0;
       ff && ret==0 && (display_col != 1 || !MoreYnBreak(&nonstop, NULL));
       ret=FindNext(ff))
  {
    stamp=ff->scWdate;

    sprintf(date, "%s %d, %d", months[stamp.msg_st.date.mo-1],
                               stamp.msg_st.date.da,
                               1980+stamp.msg_st.date.yr);

    if (ff->usAttr & ATTR_SUBDIR)
      strcpy(size, "  <DIR>");
    else
      sprintf(size, "%7ld", ff->ulSize);

    Printf("%-12.12s %s     %-18s %02d:%02d\n",
           ff->szName,
           size,
           date,
           stamp.msg_st.time.hh,
           stamp.msg_st.time.mm);
  }

  FindClose(ff);
}

