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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: f_misc.c,v 1.3 2004/01/27 21:00:28 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=File area routines: Miscellaneous functions
*/

#include <stdio.h>
#include <ctype.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"

void Dont_Use_Wildcards(char *name)
{
  Printf(no_wildcard,name);

  Puts(no_wc1);
  Puts(no_wc2);
  Press_ENTER();
}


char * Remove_Files_Entry(char *filename,char *description)
{
  FILE *infilesbbs, *outfilesbbs;

  byte *line;
  byte *temp;
  byte poo_name[PATHLEN];
  byte fromname[PATHLEN];
  
  word fnlen;

  fnlen=strlen(filename);
  upper_fn(filename);
  
  if (description)
    *description='\0';

  if (*FAS(fah, filesbbs))
    strcpy(fromname, FAS(fah, filesbbs));
  else sprintf(fromname, ss, FAS(fah, downpath), files_bbs);


  if ((infilesbbs=shfopen(fromname, fopen_readb, O_RDONLY | O_BINARY))==NULL)
  {
    cant_open(fromname);
    return NULL;
  }

  sprintf(poo_name, files_poo, PRM(temppath), task_num);

  if ((outfilesbbs=shfopen(poo_name, fopen_write,
                           O_WRONLY | O_TRUNC | O_CREAT))==NULL)
  {
    cant_open(poo_name);
    fclose(infilesbbs);
    return NULL;
  }
  
  if ((line=malloc(MAX_FBBS_ENTRY))==NULL ||
      (temp=malloc(PATHLEN))==NULL)
  {
    if (line)
      free(line);

    logit(mem_none);
    fclose(outfilesbbs);
    fclose(infilesbbs);
    return NULL;
  }

  while (fbgets(line, MAX_FBBS_ENTRY, infilesbbs))
  {
    strcat(line, "\n");

    /* If this is the file... */
    
    if (eqstrni(line, filename, fnlen) && line[fnlen] <= ' ')
    {
      /* Return the file entry, maybe for H)url? */
      
      if (description)
        strcpy(description,line); 
    }
    else
    {
      if (fputs(line, outfilesbbs)==EOF)
      {
        Printf(err_writ, poo_name);

        free(temp);
        free(line);
        fclose(outfilesbbs);
        fclose(infilesbbs);

        return NULL;
      }
    }
  }

  free(line);

  fclose(outfilesbbs);
  fclose(infilesbbs);


  /* Delete the backup file, if any */

  sprintf(temp, ss, FAS(fah, downpath), files_bak);
  unlink(temp);


  /* Save the original file in case we screwed up...  (But don't do so     *
   * if we have an explicit FILES.BBS path, since we might be running      *
   * on a WORM, and we don't want to write to it in that case.)            */

  if (*FAS(fah, filesbbs))
    unlink(fromname);
  else move_file(fromname, temp);

  /* And put the new one in its place! */

  move_file(poo_name, fromname);

  free(temp);
  
  RemoveFilesDat(&fah, filename);
  
  return ((description && *description) ? description : NULL);
}



/* Create a date with the format specified in the .prm file */

void CreateDate(char *out, union stamp_combo *date)
{
  switch (prm.date_style)
  {
    /* Even if no default date style, we need to display something, so      *
     * use USA format.                                                      */

    default:
    case -1: 
    case 0:
      sprintf(out,
              date_str,
              date->msg_st.date.mo,
              date->msg_st.date.da,
              (date->msg_st.date.yr+80) % 100);
      break;

    case 1:
      sprintf(out,
              date_str,
              date->msg_st.date.da,
              date->msg_st.date.mo,
              (date->msg_st.date.yr+80) % 100);
      break;

    case 2:
    case 3:
      sprintf(out,
              prm.date_style==2 ? date_str : datestr,
              (date->msg_st.date.yr+80) % 100,
              date->msg_st.date.mo,
              date->msg_st.date.da);
      break;
  }
}



/* Get a date from the user */

sword Get_New_Date(union stamp_combo *new_date, union stamp_combo *last, char *promptprefix)
{
  sword rc;

  WhiteN();

  rc=GetNewDateDiscrete(new_date, last, promptprefix,
                        NULL, NULL, NULL);
  WhiteN();

  return rc;
}

sword GetNewDateDiscrete(union stamp_combo *new_date, union stamp_combo *last,
                         char *promptprefix, int *piOrigYear, int *piOrigMonth,
                         int *piOrigDay)
{
  char temp[PATHLEN];
  char dtfmt[32];
  char *scanfmt;

  int mm, dd, yy;       /* must be 'int' for sscanf() */
  int *a1, *a2, *a3;    /* ditto */

  int *piYear=piOrigYear ? piOrigYear : &yy;
  int *piMonth=piOrigMonth ? piOrigMonth : &mm;
  int *piDay=piOrigDay ? piOrigDay : &dd;

  /* Add the first part of the prompt string */
  
  scanfmt=scan_str;

  if (last->msg_st.date.mo==0)
    last->msg_st.date.mo=1;
  
  if (last->msg_st.date.da==0)
    last->msg_st.date.da=1;

  /* Now add formatting codes, based on the selected date fmt */

  CreateDate(dtfmt, last);

  /* Now figure out how to parse the date entered by the user */

  switch (prm.date_style)
  {
    default:
    case -1:
    case 0:   a1=piMonth; a2=piDay; a3=piYear; break;
    case 1:   a1=piDay; a2=piMonth; a3=piYear; break;
    case 2:   a1=piYear; a2=piMonth; a3=piDay; break;
    case 3:   a1=piYear; a2=piMonth; a3=piDay; scanfmt=datestr; break;
  }

  InputGets(temp, "%s%s: ", promptprefix, dtfmt);

  /* Q to quit */

  if (tolower(*temp)=='q')
    return -1;

  /* Fill out the default hours, minutes and seconds */

  new_date->msg_st.time.hh=0;
  new_date->msg_st.time.mm=0;
  new_date->msg_st.time.ss=0;

  
  /* If <Enter>, use date of last call */

  if (! *temp)
  {
    *piMonth=last->msg_st.date.mo;
    *piDay=last->msg_st.date.da;
    *piYear=last->msg_st.date.yr+80;

    /* Copy the time bits too */

    new_date->msg_st.time=last->msg_st.time;
  }

  if (! *temp ||
      (sscanf(temp,scanfmt,a1,a2,a3)==3 &&
       *piMonth >= 1  && *piMonth <= 12 && *piDay >= 1 &&
       *piDay <= 31))
  {
    new_date->msg_st.date.mo=*piMonth;
    new_date->msg_st.date.da=*piDay;

    if (*piYear >= 1980)                            /* 1992 */
      new_date->msg_st.date.yr = *piYear - 1980;
    else if (*piYear >= 200)                        /* invalid - choose today */
      new_date->msg_st.date.yr = last->msg_st.date.yr;
    else if (*piYear >= 80 && *piYear <= 199)       /* 100 = year 2000 */
      new_date->msg_st.date.yr = *piYear - 80;
    else                                            /* 60 = 2060 */
      new_date->msg_st.date.yr=*piYear + 20;

    return 0;
  }
  else
  {
    Puts(bad_date);
    return -1;
  }
}


/* See if two file names match, taking wildcards into account */

word MatchWC(char *pat,char *fn)
{
  while (*pat && *fn)
  {
    if (*pat=='*' || *fn=='*')
    {
      while (*fn && *fn != '.')
        fn++;
      
      if (*pat != '.')
        pat++;
      
       while (*pat && *pat != '.')
        pat++;
    }
    else if (*pat=='?' || *fn=='?')
    {
      if (*pat != '.')
        pat++;
      
      if (*fn != '.')
        fn++;
    }
    else if (toupper(*pat++) != toupper(*fn++))
    {
      return FALSE;
    }
  }
  
  return (*pat=='\0' && *fn=='\0');
}
