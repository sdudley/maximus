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
static char rcs_id[]="$Id: f_titles.c,v 1.1.1.1 2002/10/01 17:51:10 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: F)ile Titles function
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"
#include "display.h"



void File_Titles(void)
{
  char filepath[PATHLEN];
  char temp[BUFLEN];
  char *s;
  char nonstop=0;
  
  word attr;


  WhiteN();

  InputGetsLL(temp, 0, file_title);
  
  if (*FAS(fah, filesbbs))
    strcpy(filepath, FAS(fah, filesbbs));
  else sprintf(filepath, ss, FAS(fah, downpath), sfiles);
  
  attr=DISPLAY_FILESBBS;
  matches=0;
  first_search=TRUE;


  if (! *temp || eqstr(temp,"="))
    ;
  else if (*temp=='*')
  {
    if ((s=firstchar(temp+1, cmd_delim, 1)) != NULL && isdigit(*s))
      strcpy(linebuf,s);

    if (Get_New_Date(&new_date, &date_newfile, dtsf)==-1)
      return;

    /* Store the time of this newfiles search */

    Get_Dos_Date(&usr.date_newfile);

    attr |= DISPLAY_NEWFILES;
  }
  else if (strchr(temp,'*') || strchr(temp,'?'))
    Dont_Use_Wildcards(ze_file_cmd);
  else
  {
    Printf(searchingfor, temp, '\n');
    strcpy(searchfor, temp);

    first_search=TRUE;
    attr |= DISPLAY_SEARCH;
  }


  /* Now display the files.bbs itself */
  
  Puts(CYAN "\n");
  
  display_line=display_col=1;

  Display_File(attr, &nonstop, filepath);
  
  /* If doing a newfiles or a search, display the # of matches */

  if (attr & (DISPLAY_NEWFILES | DISPLAY_SEARCH))
    Printf(located, matches, matches==1 ? blank_str : pl_match);

  first_search=FALSE;

  if (!nonstop && MenuNeedMore())
  {
    Puts(CYAN);
    display_line=TermLength(); /* force a more[y,n,=,t] */
    DispMoreYnBreak(&nonstop, CYAN, DISPLAY_FILESBBS);
  }
}

