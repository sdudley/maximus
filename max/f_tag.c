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
static char rcs_id[]="$Id: f_tag.c,v 1.1.1.1 2002/10/01 17:51:10 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: T)ag functions
*/

#define MAX_LANG_max_main

#include <stdio.h>
#include <stdlib.h>
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

static void near File_Tag_Delete(void);
static void near File_Tag_List(void);

void File_Tag(int dl_cmd)
{
  byte ch;

  if (local)
    baud=38400L;

  if (!*linebuf)
    File_Tag_List();

  for (;;) 
  {
    WhiteN();

    ch=(byte)toupper(KeyGetRNP(dl_cmd ? file_edit_menu : tag_menu));

    WhiteN();

    if (ch==tag_keys[0] && !dl_cmd)
    {
      File_Get_Download_Names(TAG_VERBOSE, PROTOCOL_ZMODEM);
      return;       /* Automatically return after adding tag */
    }
    else if (ch==tag_keys[1])
      File_Tag_List();
    else if (ch==tag_keys[2])
      File_Tag_Delete();
    else if (ch==tag_keys[3])
    {
      Free_Filenames_Buffer(0);
      Puts(all_untagged);
    }
    else if (ch==tag_keys[4])
      Display_File(0, NULL, "%stag_file", PRM(misc_path));
    else if (ch=='\0' || ch=='|' || ch=='\r' || ch==tag_keys[5])
      return;
    else Printf(dontunderstand,ch);
  }
}


static void near File_Tag_List(void)
{
  word n;
  long total_size;
  long this_time;

  FENTRY fent;

  Putc('\n');

  if (!FileEntries())
  {
    Puts(no_tagged);
    return;
  }

  for (n=0, total_size=0L; GetFileEntry(n, &fent); n++)
  {
    this_time=XferTime(PROTOCOL_ZMODEM, fent.ulSize);

    Printf(file_stats,
           n+1,
           No_Path(fent.szName),
           (int)(this_time/60L),
           (int)(this_time % 60L),
           (long)fent.ulSize);

    Putc('\n');
    total_size += fent.ulSize;

    vbuf_flush();
  }
  
  Printf(file_tag_total,
         total_size,
         XferTime(PROTOCOL_ZMODEM, total_size) / 60L,
         XferTime(PROTOCOL_ZMODEM, total_size) % 60L);
}

static void near File_Tag_Delete(void)
{
  byte temp[PATHLEN];
  byte fname[PATHLEN];
  word fn, ft;

  WhiteN();

  if (! FileEntries())
  {
    Puts(no_tagged);
    return;
  }

  InputGets(temp, file_untag, FileEntries());

  WhiteN();

  if (toupper(*temp)=='A')
    Free_Filenames_Buffer(0);
  else if ((fn=(word)atoi(temp)) != 0)
  {
    char *p=temp;

    ft=fn;

    /* See if the user entered a range from?to */

    while (*p && isdigit(*p))
      ++p;

    if (*p && (++p, isdigit(*p)))
    {
      ft=(word)atoi(p);
      if (ft <= fn)
        ft=fn;
    }

    if (ft > FileEntries())
      ft=FileEntries();

    --fn;
    --ft;

    /* If the number is within allowable bounds... */

    while (fn <= ft)
    {
      FENTRY fent;

      if (!GetFileEntry(fn, &fent))
        ++fn;
      else
      {
        /* Save the filename we deleted */

        strcpy(fname,No_Path(fent.szName));
        RemoveFileEntry(fn);

        /* And inform the user */

        Printf(file_untagged, fname);

        if (ft)
          --ft;
        else
          ++fn;
      }
    }
  }
  
  if (FileEntries()==0)
    Puts(all_untagged);
}


/*
 * Tag save/restore
 * Saves/restores the list of files currently tagged
 * to/from an external file
 */

#ifndef ORACLE

static char *last_tag_name=NULL;

static void near make_tag_save_name(char *buf, char *pszFname)
{
  if (!pszFname || !*pszFname)
    pszFname=default_tag_save;

  if (strchr(pszFname,'%'))
    Parse_Outside_Cmd(pszFname,buf);
  else
    strcpy(buf,pszFname);

  Convert_Star_To_Task(buf);
}

int save_tag_list(char *pszFname)
{
  FILE *fp;
  char temp[PATHLEN];

  if (last_tag_name)
  {
    free(last_tag_name);
    last_tag_name=NULL;
  }
  make_tag_save_name(temp, pszFname);
  if ((fp=shfopen(temp, fopen_write, O_CREAT | O_TRUNC | O_WRONLY))!=NULL)
  {
    FENTRY fent;
    int n;

    for(n=0; GetFileEntry(n, &fent); ++n)
      fprintf(fp, "%s %ld %04x\n", fent.szName, fent.ulSize, fent.fFlags);

    fclose(fp);
    last_tag_name=strdup(temp);
    return TRUE;
  }
  return FALSE;
}

int restore_tag_list(char *pszFname,int fNoDelete)
{
  FILE *fp;
  char temp[PATHLEN];

  Free_Filenames_Buffer(0);

  if (pszFname==NULL && last_tag_name)
    strcpy(temp,last_tag_name);
  else make_tag_save_name(temp, pszFname);

  if ((fp=shfopen(temp, fopen_read, O_RDONLY))!=NULL)
  {
    char buf[PATHLEN];

    while (fgets(buf,PATHLEN-1,fp)!=NULL)
    {
      char fname[PATHLEN];
      long size;
      int flag=0;

      flag = 0;

      if (sscanf(buf, "%s %ld %x", fname, &size, &flag) >= 2)
        AddFileEntry(fname, flag, size);
    }

    fclose(fp);

    if (!fNoDelete)
      unlink(temp);

    return TRUE;
  }

  return FALSE;
}

#endif /* !ORACLE */


