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

#define MAX_LANG_f_area
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include "ffind.h"
#include "mm.h"
#include "max_file.h"

/* Skip until the next \r (plus possibly \n) in a display-type file */

static void near Skip_Line(FILE *fp)
{
  int ch;

  while ((ch=fgetc(fp)) != '\n' && ch != EOF)
    ;
}


int IsInFilesBbs(PFAH pfah, char *filename, word *flag, char *path)
{
  FILE *filesbbs;

  long offset;

  sword ret;
  word breakout;

  byte temp[255];
  byte *filebufr;
  byte *next, *p;


  ret=DL_NOTFOUND;
  filebufr=NULL;

  if (path)
    *path='\0';

  /* Find the name of the FILES.BBS entry to use */
  
  if (*FAS(*pfah, filesbbs))
    strcpy(temp, FAS(*pfah, filesbbs));
  else sprintf(temp, ss, FAS(*pfah, downpath), files_bbs);

  if ((filesbbs=shfopen(temp, fopen_readb, O_RDONLY | O_BINARY | O_NOINHERIT))==NULL)
  {
/*      cant_open(temp);*/
    return ret;
  }
  
  if ((filebufr=malloc(FILEBUFSIZ)) != NULL)
    setvbuf(filesbbs,filebufr,_IOFBF,FILEBUFSIZ);

  breakout=FALSE;

  while (!breakout &&
         (offset=ftell(filesbbs), fgets(temp, 255, filesbbs)))
  {
    switch(*temp)
    {
      case '\x10':        /* Priv. command! */
        fseek(filesbbs, offset+1L, SEEK_SET);

        switch(Parse_Priv(filesbbs))
        {
          case SKIP_FILE:
            breakout=TRUE;
            break;

          case SKIP_LINE:             /* Go back for another line */
            Skip_Line(filesbbs);
            /* fall-through */
            
          case SKIP_NONE:
            continue;
        }
        break;

      case '@':
        /* Skip rest of file */
        if (! acsflag(CFLAGA_FHIDDEN))
        {
          breakout=TRUE;
          break;
        }
        else
        {
          /* Remove the '@' */
          strocpy(temp, temp+1);
        }

        /* fall-through */

      default:
        if (*temp > ' ' && *temp != '-')
        {
          /* Strip off filename specification */

          if (temp[0]=='\\' || temp[1]==':' || temp[0]=='/')
          {
            char *sp=strchr(temp, ' ');
            char *tb=strchr(temp, '\t');
            char *pth;

            /* Find the first space or tab */

            if (!sp)
              sp=tb;
            else if (tb && tb < sp)
              sp=tb;

            /* Change the space to a NUL so we can do our path search */

            if (sp)
              *sp='\0';

            pth=strrstr(temp, "\\/");

            if (sp) /* Now change it back to a space */
              *sp=' ';

            if (pth)
            {
              if (path)
              {
                strncpy(path, temp, (char *)pth-(char *)temp+1);
                path[(char *)pth-(char *)temp+1]='\0';
              }
              
              strocpy(temp, pth+1);
            }
          }

          temp[12+5]='\0';

          /* Get everything up 'till the first space, the fast way. */

          for (p=temp, next=p+strlen(p); *p; p++)
            if (isspace(*p))
            {
              next=p;
              *p='\0';
              break;
            }

          /* If this filename matches that wildcard */
            
          if (MatchWC(temp, filename))
          {
            ret=DL_FOUND;
            
            next++;

            while (*next==' ')
              next++;

            /* Allocate free time/bytes regardless if area-wide */

            if (flag)
            {
              if (pfah->fa.attribs & FA_FREETIME)
                *flag |= FFLAG_NOTIME;
              if (pfah->fa.attribs & FA_FREESIZE)
                *flag |= FFLAG_NOBYTES;
            }

            /* Free time/bytes downloads */

            if (*next=='/')
            {
              while (*next && *++next != ' ')
              {
                if (tolower(*next)=='b' && flag)
                  *flag |= FFLAG_NOBYTES;
                else if (tolower(*next)=='t' && flag)
                  *flag |= FFLAG_NOTIME;
              }
            }

            breakout=TRUE;
          }
        }
        break;
    }
  }

  if (filebufr)
    free(filebufr);

  fclose(filesbbs);

  if (ret==DL_NOTFOUND && flag)
    *flag |= FFLAG_NOLIST;
    
  return ret;
}


/* Determines if it's okay to download the specified filename (no path) */

word File_Okay(char *filename)
{
  char path[PATHLEN];
  char filespec[PATHLEN];
  FFIND *ff;
  word okay;

  okay=TRUE;

  /* Add the path to this filename */
  
  sprintf(filespec, ss, FAS(fah, downpath), filename);


  *path=0;

  if (IsInFilesBbs(&fah, filename, NULL, path))
  {
    /* If there is a full path for this file */

    if (*path)
      sprintf(filespec, ss, path, filename);

    ff=FindOpen(filespec, 0);

    if (!ff)
    {
      File_IsOffline(filename);
      okay=FALSE;
    }
    else if (!eqstri(filename, ff->szName))
    {
      Puts(file_no_wc);
      okay=FALSE;
    }

    FindClose(ff);
  }
  else
  {
    /* Priv. needed to download file not in FILES.BBS */

    if (acsflag(CFLAGA_FLIST))
    {
      ff=FindOpen(filespec, 0);

      if (!ff || !eqstri(filename, ff->szName))
      {
        File_NotExist(filename);
        okay=FALSE;
      }

      FindClose(ff);
    }
    else
    {
      File_NotExist(filename);
      okay=FALSE;
    }
  }

  return okay;
}


