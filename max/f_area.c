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
static char rcs_id[]="$Id: f_area.c,v 1.1.1.1 2002/10/01 17:50:58 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File Section: A)rea Change command and listing of file areas
*/

#define INITIALIZE_FILE    /* Intialize message-area variables */

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_file.h"
#include "max_menu.h"

/* Search for the next or prior file area */

static int near SearchArea(int search, char *input, PFAH pfahDest, BARINFO *pbi, int *piDidValid)
{
  FAH fa;
  HAFF haff;

  memset(&fa, 0, sizeof fa);
  *piDidValid=FALSE;
  strcpy(linebuf, input+1);

  /* Try to find the current file area */

  if ((haff=AreaFileFindOpen(haf, usr.files, 0))==NULL)
    return TRUE;

  /* Peform the first search to make sure that usr.files exists */

  if (AreaFileFindNext(haff, &fa, FALSE) != 0)
  {
    AreaFileFindClose(haff);
    return TRUE;
  }

  /* Change the search parameters to find the next area */

  AreaFileFindChange(haff, NULL, 0);

  /* Search for the prior or next area, as appropriate */

  while ((search==-1 ? AreaFileFindPrior : AreaFileFindNext)(haff, &fa, TRUE)==0)
  {
    if ((fa.fa.attribs & FA_HIDDN)==0 &&
        ValidFileArea(NULL, &fa, VA_VAL | VA_PWD | VA_EXTONLY, pbi))
    {
      *piDidValid=TRUE;
      search=0;
      SetAreaName(usr.files, FAS(fa, name));
      CopyFileArea(pfahDest, &fa);
      break;
    }
  }

  AreaFileFindClose(haff);
  DisposeFah(&fa);

  /* If it was found, get out. */

  return (search==0);
}


/* Change to a named message area */

static int near ChangeToArea(char *group, char *input, int first, PFAH pfahDest)
{
  FAH fa;
  char temp[PATHLEN];
  HAFF haff;

  memset(&fa, 0, sizeof fa);

  if (! *input)
  {
    if (first)
      ListFileAreas(group, !!*group);
    else return TRUE;
  }
  else if ((haff=AreaFileFindOpen(haf, input, AFFO_DIV)) != NULL)
  {
    int rc;

    /* Try to find this area relative to the current division */

    strcpy(temp, group);

    /* If we have a non-blank group, add a dot */

    if (*temp)
      strcat(temp, dot);

    /* Add the specified area */

    strcat(temp, input);

    AreaFileFindChange(haff, temp, AFFO_DIV);
    rc=AreaFileFindNext(haff, &fa, FALSE);

    if (rc==0)  /* got it as a qualified area name */
      strcpy(input, temp);
    else
    {
      /* Try to find it as a fully-qualified area name */

      AreaFileFindReset(haff);
      AreaFileFindChange(haff, input, AFFO_DIV);

      rc=AreaFileFindNext(haff, &fa, FALSE);
    }

    if (rc==0 && (fa.fa.attribs & FA_DIVBEGIN))
    {
      strcpy(group, FAS(fa, name));
      AreaFileFindClose(haff);
      ListFileAreas(group, !!*group);
    }
    else
    {
      SetAreaName(usr.files, input);
      CopyFileArea(pfahDest, &fa);
      AreaFileFindClose(haff);
      DisposeFah(&fa);
      return TRUE;
    }
  }

  DisposeFah(&fa);
  return FALSE;
}


static int near FileAreaMenu(PFAH pfah, char *group, BARINFO *pbi)
{
  char input[PATHLEN];
  unsigned first=TRUE;    /* Display the area list 1st time <enter> is hit */
  int did_valid=FALSE;

  WhiteN();

  for (;;)
  {
    int search=0;

    Puts(WHITE);
    
    InputGets(input, file_prmpt, PRM(achg_keys)[0],
                                 PRM(achg_keys)[1],
                                 PRM(achg_keys)[2]);
    cstrupr(input);

    /* See if the user wishes to search for something */

    if (*input==PRM(achg_keys)[1] || *input==']' || *input=='>' || *input=='+')
      search=1;
    else if (*input==PRM(achg_keys)[0] || *input=='[' || *input=='<' || *input=='-')
      search=-1;



    if (search) /* Search for a specific area */
    {
      if (SearchArea(search, input, pfah, pbi, &did_valid))
        return did_valid;
    }
    else if (*input=='\'' || *input=='`' || *input=='"')
      Display_File(0, NULL, ss, PRM(misc_path), quotes_misunderstood);
    else if (*input=='#')              /* Maybe the user misunderstood? */
      Display_File(0, NULL, ss, PRM(misc_path), numsign_misunderstood);
    else if (*input=='/' || *input=='\\')
    {
      *group=0;
      strcpy(linebuf, input+1);

      if (! *linebuf)
        ListFileAreas(group, !!*group);
    }
    else if (*input=='.')   /* go up one or more levels */
    {
      char *p=input;
      int up_level=0;

      /* Count the number of dots */

      while (*++p=='.')
        up_level++;

      /* Add any area names which may come after this */

      if (*p)
        strcpy(linebuf, p);

      /* Now go up the specified number of levels */

      while (up_level--)
        if ((p=strrchr(group, '.')) != NULL)
          *p=0;
        else *group=0;

      if (! *linebuf)
        ListFileAreas(group, !!*group);
    }
    else if (*input==PRM(achg_keys)[2] || *input=='?')
    {
      strcpy(linebuf, input+1);
      ListFileAreas(group, !!*group);
    }
    else if (*input=='=')
      ListFileAreas(NULL, FALSE);
    else if (! *input ||
             (*input >= '0' && *input <= '9') ||
             (*input >= 'A' && *input <= 'Z'))
    {
      if (ChangeToArea(group, input, first, pfah))
        return did_valid;
    }
    else Printf(dontunderstand, *input);

    first=FALSE;
  }
}


int File_Area(void)
{
  FAH fa;
  BARINFO bi;
  char group[PATHLEN];
  char savearea[MAX_ALEN];
  int ok=FALSE;
  int did_valid;

  memset(&fa, 0, sizeof fa);
  strcpy(savearea, usr.files);
  FileSection(usr.files, group);

  do
  {
    CopyFileArea(&fa, &fah);
    did_valid=FileAreaMenu(&fa, group, &bi);

    if (!fa.heap || !(did_valid || ValidFileArea(NULL, &fa, VA_VAL | VA_PWD, &bi)))
    {
      logit(denied_access, deny_file, usr.files);

      strcpy(usr.files, savearea);

      if (*PRM(areanotexist))
        Display_File(0, NULL, PRM(areanotexist));
      else Puts(areadoesntexist);

      continue;
    }

    if (!PopPushFileAreaSt(&fa, &bi))
      Puts(areadoesntexist);
    else ok=TRUE;
  }
  while (!ok);

  logit(log_farea, usr.files);
  DisposeFah(&fa);

  return 0;
}


static int near FoundOurFileDivision(HAFF haff, char *division, PFAH pfah)
{
  if (!division || *division==0)
    return TRUE;

  return (AreaFileFindNext(haff, pfah, FALSE)==0 &&
          (pfah->fa.attribs & FA_DIVBEGIN) &&
          eqstri(PFAS(pfah, name), division));
}


void ListFileAreas(char *div_name, int show_help)
{
  BARINFO bi;
  FAH fa;
  HAFF haff=0;
  char nonstop=FALSE;
  char headfoot[PATHLEN];
  char *file;

  memset(&fa, 0, sizeof fa);

  if (*PRM(file_area_list))
  {
    if (!div_name || *div_name==0 ||
        (haff=AreaFileFindOpen(haf, div_name, AFFO_DIV))==NULL ||
        !FoundOurFileDivision(haff, div_name, &fa) ||
        eqstri(FAS(fa, filesbbs), dot))
    {
      file=PRM(file_area_list);
    }
    else
    {
      file=FAS(fa, filesbbs);
    }

    Display_File(0, NULL, file);
  }
  else
  {
    Puts(CLS);

    display_line=display_col=1;


    ParseCustomFileAreaList(NULL, div_name, PRM(file_header), headfoot, TRUE);
    Puts(headfoot);

    if ((haff=AreaFileFindOpen(haf, div_name, AFFO_DIV))==NULL)
      return;

    /* Ensure that we have found the beginning of our division */

    if (!FoundOurFileDivision(haff, div_name, &fa))
    {
      AreaFileFindReset(haff);
      div_name = "";
    }

    {
      sword this_div=div_name && *div_name ? fa.fa.division : -1;

      /* Now find anything after the current division */

      AreaFileFindChange(haff, NULL, AFFO_DIV);

      while (AreaFileFindNext(haff, &fa, FALSE)==0)
      {
        /* If we're just doing a flat area list, don't display              *
         * division names.                                                  */

        if (!div_name && (fa.fa.attribs & FA_DIVBEGIN))
          continue;

        /* If we have reached the end of our division, break out of the     *
         * loop.                                                            */

        if (fa.fa.attribs & FA_DIVEND)
        {
          if (div_name && fa.fa.division==this_div)
            break;
          else continue;
        }

        /* If we're in the right division and the area is valid, display    *
         * its name.                                                        */

        if ((!div_name || fa.fa.division==this_div+1) &&
            (fa.fa.attribs & FA_HIDDN)==0 &&
            (((fa.fa.attribs & FA_DIVBEGIN) && PrivOK(FAS(fa, acs), TRUE)) ||
             ValidFileArea(NULL, &fa, VA_NOVAL, &bi)))
        {
          ParseCustomFileAreaList(&fa, div_name, PRM(file_format), headfoot, FALSE);

          Puts(headfoot);
          vbuf_flush();
        }

        if (halt() || MoreYnBreak(&nonstop, CYAN))
          break;
      }
    }


    ParseCustomFileAreaList(NULL, div_name, PRM(file_footer), headfoot, FALSE);
    Puts(headfoot);

    Putc('\n');

    /* If necessary, display help for changing areas */

    if (show_help)
      Puts(achg_help);

    vbuf_flush();
  }

  if (haff)
    AreaFileFindClose(haff);

  DisposeFah(&fa);
}


char *FileSection(char *current, char *szDest)
{
  char *p;

  strcpy(szDest, current);

  if ((p=strrchr(szDest, '.')) != NULL)
    *p=0;
  else
    *szDest=0;

  return szDest;
}


