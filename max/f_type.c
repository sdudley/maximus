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
static char rcs_id[]="$Id: f_type.c,v 1.1 2002/10/01 17:51:10 sdudley Exp $";
#pragma on(unreferenced)

/*# name=File area routines: T)ype command
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "alc.h"
#include "max_file.h"


void File_Type(void)
{
  FILE *typefile;

  char path[PATHLEN];
  byte filename[PATHLEN];
  byte filespec[PATHLEN];
  byte buf[PATHLEN];
  char nonstop;

  word got, bad;
  word garbage;
  int ch, save;
  

  
  
  WhiteN();

  InputGets(filename, type_which);

  WhiteN();

  Strip_Path(filename); /* Make sure user doesn't specify path or device */

  if (! *filename)
    return;

  if (! File_Okay(filename))
    return;
  

  path[0]=0;

  if (IsInFilesBbs(&fah, filename, NULL, path) && *path)
    sprintf(filespec, ss, path, filename);
  else sprintf(filespec, ss, FAS(fah, downpath), filename);

  if ((typefile=shfopen(filespec, fopen_readb, O_RDONLY | O_BINARY))==NULL)
  {
    cant_open(filespec);
    return;
  }


  /* Get a line into buf, and check for binary characters */

  got=fread(buf, 1, sizeof(buf)-1, typefile);

  bad=FALSE;
  garbage=0;

  /* If it's an ARC, EXE or ZIP header */
  
  if (*buf=='\x1a' || 
      eqstrn(buf, "MZ", 2) ||
      eqstrn(buf, "PK", 2) ||
      eqstrn(buf, "GIF", 3) ||
      stristr(filespec, dot_zip) ||
      stristr(filespec, dot_pak) ||
      stristr(filespec, dot_lzh) ||
      stristr(filespec, dot_arc) ||
      stristr(filespec, dot_arj))
  {
    bad=TRUE;
  }
  else
  {
    byte *s, *hi;
    
    /* Count the number of below ASCII 32 chars, with the exception of      *
     *  returns, linefeeds, and ESCape characters (for ANSI!)               */

    hi=buf+min(got, PATHLEN-1);
    
    for (s=buf; s < hi; s++)
      if (*s && *s < ' ' && *s != '\r' && *s != '\n' && *s != '\x1b')
        garbage++;
  }

  if (bad || garbage >= 15)
  {
    Puts(type_for_text);
    Press_ENTER();

    fclose(typefile);
    return;
  }

  logit(log_disp, blank_str, filespec);

  nonstop=FALSE;

  fseek(typefile, 0L, SEEK_SET);

  display_line=display_col=1;

  while ((ch=fgetc(typefile)) != EOF && ch != '\x1b' && ch != '\x1a')
  {
    /* Handle lines with only '\r' delimiters... */

    if (ch=='\r')
    {
      if ((save=fgetc(typefile)) != '\n')
        ch='\n';

      ungetc(save, typefile);
    }

    Putc(ch);

    if (display_col==1 && MoreYnBreak(&nonstop,NULL))
      break;
  }

  fclose(typefile);
}

