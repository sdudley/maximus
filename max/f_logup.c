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

/* $Id: f_logup.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=File area routines (log uploads to files.bbs)
*/

#define MAX_INCL_COMMS

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <time.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "max_file.h"

#ifdef BINK_PROTOCOLS
  #include "prototyp.h"
#endif


void Add_To_Upload_Log(char *path,char *fname,long bytes)
{
  FILE *ullog;
  char *do_touch;
  union stamp_combo stamp;

  int ulfile;

  char p1[PATHLEN];
  char p2[PATHLEN];

  
  Get_Dos_Date(&stamp);

  if (*PRM(ul_log))
  {
    strcpy(p1,PRM(ul_log));
    Convert_Star_To_Task(p1);

    if ((ullog=shfopen(p1,fopen_append,O_WRONLY | O_APPEND))==NULL)
      cant_open(p1);
    else
    {
      Timestamp_Format(PRM(dateformat), &stamp, p1);
      Timestamp_Format(PRM(timeformat), &stamp, p2);


      fprintf(ullog, ullog_format,
              usr.name, path, fname, bytes, p1, p2);

      fclose(ullog);
    }
  }

  /* If we are Bob Juge, skip the code that sets the file date/time to today */

  {
                      /* BOB_JUGE */
    char bob_string[] = "CPC`KVHF";
    char a_big_meanie[] = "b!cjh!nfbojf";
    char *s;

    for (s=bob_string; *s; s++)
      --(*s);

    for (s=a_big_meanie; *s; s++)
      --(*s);

    do_touch = getenv(bob_string);

    if (do_touch && eqstri(do_touch, a_big_meanie))
      return;

  }

  /* Now touch the file's date so it is set to right now. */

  sprintf(p1, ss, path, fname);

  if ((ulfile=shopen(p1, O_RDWR | O_BINARY)) != -1)
  {
    set_fdt(ulfile, (union stamp_combo *)&stamp);
    close(ulfile);
  }
}



/* Add an uploaded filename's filename/description to FILES.BBS */

void Get_File_Description(char *filename, long fsize, char *dsc)
{
  struct tm *stim;
  time_t curt;

  FILE *bbsfile;

  byte temp[PATHLEN];
  byte description[PATHLEN];
  byte nw[PATHLEN];
  byte *p;

  word desc_num, wrapped;

  if (! (local || carrier()))
    return;

  /* always use files.bbs in upload dir */
  
  sprintf(temp, ss, FAS(fah, uppath), files_bbs);

  if ((bbsfile=shfopen(temp, fopen_append, O_WRONLY | O_APPEND | O_NOINHERIT))==NULL)
  {
    cant_open(temp);
    return;
  }

  desc_num=1;

  curt=time(NULL);
  stim=localtime(&curt);

  if (!(dsc && *dsc) && (local || carrier()))
    Printf(desc_many, upper_fn(filename));

  /* Display a maximum-length bar */

  if (! (dsc && *dsc) && (local || carrier()))
    Puts(LGREEN "\n  Ú\x19Ä\x2d¿\n");

  *nw='\0';

  while (desc_num <= 3)
  {
    sprintf(temp, GRAY "%d> " YELLOW, desc_num);

    *description='\0';

    while (isblstr(description))
    {
      if (dsc && *dsc)
        strcpy(description, dsc);
      else
      {
        if (local || carrier())
        {
          if (*nw)
            strcpy(description, nw);
          else *description='\0';

          wrapped=Input(description, INPUT_NLB_LINE | INPUT_WORDWRAP,
                        0, 45, temp);

          if (wrapped==1)
            strcpy(nw, description+strlen(description)+1);
          else *nw='\0';

        }
        else *description='\0';
      }

      if (! *description && desc_num != 1)
        break;

      if (! *nw)
        strcat(description," ");
    }

    if (! *description)
      break;

    /* Only add date directly to FILES.BBS unless it's being supposed   *
     * to be determined automagically.                                  */

    if (autodate(fah))
    {
      *temp='\0';
    }
    else
    {
      sprintf(temp, "%7ld ", fsize);

      switch (prm.date_style)
      {
        case -1:  /* None, so give neither dates nor sizes */
          strcpy(temp," ");
          break;

        case 0: /* MM-DD-YY */
          sprintf(&temp[strlen(temp)], date_str,
                  stim->tm_mon+1, stim->tm_mday, (stim->tm_year % 100));
          break;

        case 1: /* DD-MM-YY */
          sprintf(&temp[strlen(temp)], date_str,
                  stim->tm_mday, stim->tm_mon+1, (stim->tm_year % 100));
          break;

        case 2: /* YY-MM-DD */
          sprintf(&temp[strlen(temp)], date_str,
                  (stim->tm_year % 100), stim->tm_mon+1, stim->tm_mday);
          break;

        case 3: /* YYMMDD */
          sprintf(&temp[strlen(temp)], datestr,
                  (stim->tm_year % 100), stim->tm_mon+1, stim->tm_mday);
          break;
      }

      strcat(temp, " ");
    }

    /* Convert all low-ASCII ctrl chars to spaces */
    
    for (p=description;*p;p++)
      if (*p < 32)
        *p=' ';

    for (p=filename;*p;p++)
      if (*p < 32)
        *p='\0';
      
    /* Make sure that a '/' isn't the first thing entered */

    for (p=description; *p; p++)
      if (*p==' ' || *p=='/')
        *p=' ';
      else break;


    if (desc_num++==1)
      fprintf(bbsfile, "%-12s %s%s", filename, temp, description);
    else
      fputs(description, bbsfile);

    if ((!local && !carrier()) || (dsc && *dsc))
      break;
  }

  fputc('\n', bbsfile);
  fclose(bbsfile);
}


/* Call a user-defined external program to check an upload for              *
 * viruses or other nasties...                                              */

word LookForVirus(char *path, char *name)
{
  char fname[PATHLEN];
  char cmd[PATHLEN];
  char stem[12], ext[5], *ep;
  word foundvir;
  
  if (! *PRM(viruschk))
    return FALSE;

  /* Create the full filename of the file */

  strcpy(fname, path);
  strcat(fname, name);

  /* Convert everything to upper-case, so we can be consistent */

  upper_fn(fname);
  
  
  /* Copy out the file into the readln buffer, so that it can be            *
   * accessed with the external program translation characters.             */

  strncpy(last_readln, name, MAXLEN);
  last_readln[MAXLEN]='\0';
  
  
  /* The filename comes after the last path delimiter */

  ep=strrstr(fname, path_delim);

  strncpy(stem, ep ? ep+1 : fname, 8);
  stem[8]='\0';
  

  /* Chop off the dot for the main filename */
  
  if ((ep=strchr(stem, '.')) != NULL)
    *ep='\0';


  /* Now copy the extension separately */

  ep=strrchr(fname, '.');

  if (ep)
  {
    strncpy(ext, ep, 4);
    ext[4]='\0';
  }
  else
  {
    strcpy(ext, ".");   /* so that an extensionless file does not screw us up */
  }


  /* Create the command to run */

  /* VIRCHECK.BAT d:\file\upload\ vgademo .zip d:\max\misc */

  sprintf(cmd, "%s %s %s %s %s %d",
          PRM(viruschk), path, stem, ext, PRM(misc_path), task_num);
  

  /* Tell the user what we're doing */
  
  Printf(checking_ul, name);

  
  /* Run the command */

  Outside(NULL, NULL, OUTSIDE_DOS, cmd, FALSE, CTL_NONE, RESTART_MENU, NULL);

  
  /* Now display a file to let the user know about the outcome of the       *
   * operation.                                                             */

  foundvir=!fexist(fname);
  
  Display_File(0, NULL, foundvir ? "%sfile_bad" : "%sfile_ok",
               PRM(misc_path));
             
  return foundvir;
}

