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
static char rcs_id[]="$Id: disp_qu.c,v 1.3 2004/01/27 21:00:26 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=.BBS-file display routines (Questionnaire commands)
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "mm.h"
#include "display.h"

static void near DisplayQuestIgnore(DSTK *d, sword until)
{
  sword ch;

  for (;;)
  {
    /* End of ANSI/AVATAR or TTY only */
    if ((ch=DispSlowGetChar(d))==DISP_EOF)
      break;
    else if (ch=='\x0f')
    {
      if (DispPeekChar()==until)
      {
        (void)DispSlowGetChar(d);
        break;
      }
    }
  }
}

/*
int Parse_Questionnaire(FILE *bbsfile,FILE **questfile,char *onexit,char *filename,int *ck_abort,int *recd_chars,int *skipcr,char **filebufr,char *nonstop,int allanswers)
*/

word DisplayQuestionnaire(DSTK *d)
{
  time_t longtime;
  byte *p=NULL;
  long offset;
  sword ch, rr;

  static char was_no_local_output;

  switch (ch=DispSlowGetChar(d))
  {
    case 'L':     /* change language */
      Chg_Language();
      break;

    case 'G':     /* What follows is RIP only */
      if (hasRIP())
      {
        was_no_local_output=no_local_output;
        no_local_output=1;
      }
    case 'H':     /* What follows is non-RIP only */
      if ((ch=='G' && !hasRIP()) || (ch=='H' && hasRIP()))
        DisplayQuestIgnore(d, 'I');
      break;

    case 'I':     /* End RIP section, turn local back on */
      if (hasRIP())
        no_local_output=was_no_local_output;
      break;

    case '^':     /* What follows is TTY only */
    case 'E':     /* What follows is ANSI/AVATAR only */
      if ((ch=='E' && usr.video==GRAPH_TTY) ||
          (ch=='^' && usr.video != GRAPH_TTY))
        DisplayQuestIgnore(d, 'e');
      break;

    case 'M':     /* Append last MENU CHOICE to answer file */
      DispGetToBlank(d, d->scratch);

      if (d->questfile)
        fprintf(d->questfile, "%s: %c\n", cstrupr(d->scratch), lastmenu);
      break;

    case 'N':     /* Get line and store in answer file! */
      DispGetToBlank(d, d->scratch);

      do
      {
        Input(d->temp, INPUT_LB_LINE, 0, MAXLEN, NULL);

        if (d->allanswers)
        {
          for (p=d->temp; *p; p++)
            if (isalnum(*p) || *p >= 0x7f)
              break;

          if (*p=='\0')
          {
            Puts(tryagain);
            Clear_KBuffer();
          }
        }
      }
      while (d->allanswers && *p=='\0');
      
      strcpy(last_readln, d->temp);

      display_line=1;

      if (d->questfile && *d->temp)
        fprintf(d->questfile, "%s: %s\n", cstrupr(d->scratch), d->temp);
      break;

    case 'o':
    case 'O':     /* Open answer file */
      DispGetToBlank(d, d->scratch);

      Convert_Star_To_Task(d->scratch);

      if (strchr(d->scratch,'%'))
      {
        Parse_Outside_Cmd(d->scratch,d->temp);
        strcpy(d->scratch,d->temp);
      }

      Add_Full_Path(d->scratch, d->temp);

      if (d->questfile)
      {
        fputc('\n', d->questfile);
        fclose(d->questfile);
      }
      
      d->questfile=shfopen(d->temp, fopen_append, O_WRONLY | O_APPEND | O_NOINHERIT);
      break;

    case 'P':     /* Post user info to answer file! */
      if (d->questfile)
      {
        fprintf(d->questfile, "* %s\t%s\t", usrname, usr.city);

                        /* Mon Nov 21 11:31:54 1983\n */

        strftime(d->scratch, 26, "%a %b %d %H:%M:%S %Y\n",
                 localtime(((longtime=time(NULL)), &longtime)));

        /* asctime_format already appends a C/R, no need to add one! */

        fputs(d->scratch, d->questfile);
      }
      break;

    case 'C':     /* Call DOS with command 'p' (in '^OCp') */
      {
        byte was_no_output=no_local_output;

        if (d->type & DISPLAY_NOLOCAL)
          no_local_output=FALSE;

        if (d->questfile)
          flush_handle(d->questfile);

        DispGetString(d, d->scratch, MAX_FBBS_ENTRY);
      
        if (d->ck_abort)
          Mdm_flush_ck();
        else Mdm_flush();

        rst_offset=DispGetPos();
        p=d->scratch;
        rr=0;

        if (*p=='@')
        {
          rr |= OUTSIDE_REREAD;
          p++;
        }

        Outside(NULL, NULL, OUTSIDE_DOS | rr, p, FALSE,
                CTL_NONE, RESTART_DOTBBS, d->scratch);

        Clear_KBuffer();

        rst_offset=-1L;
        no_local_output=was_no_output;
      }
      break;

    case 'D':     /* Change current directory */
      DispGetToBlank(d, d->scratch);
      Save_Directory2(d->scratch);
      break;

    case 'F':     /* On exit filename */
      d->recd_chars=FALSE;

      DispGetToBlank(d, d->onexit);
      strcpy(last_onexit, d->onexit);
      break;

    case 'Q':     /* Quit the file immediately */
      return SKIP_FILE;

    case 'R':     /* Read in menu */
      *d->nonstop=FALSE;
      display_line=1;

      DispGetToBlank(d, d->scratch);
      
      lastmenu='\0';

      do
      {
        /* If we entered an invalid response the last time */
        
        if (lastmenu)
        {
          Puts(tryagain);
          Clear_KBuffer();
        }

        Input_Char(CINPUT_DISPLAY | CINPUT_ACCEPTABLE | CINPUT_LASTMENU |
                   (d->allanswers ? CINPUT_ALLANSWERS : 0) | CINPUT_DUMP,
                   d->scratch);
      }
      while (d->allanswers &&
             (lastmenu=='\0' || lastmenu==' ' || lastmenu=='|') &&
             !strchr(d->scratch,'|'));
      break;

    case 'S':     /* Show another file */
      d->skipcr=FALSE;

      if (d->recd_chars)
      {
        *d->onexit='\0';
        *last_onexit='\0';
      }

      DispGetToBlank(d, d->filename);
      return SKIP_FILE;

    case 'T':     /* Top of file! */
      d->skipcr=FALSE;

      lseek(d->bbsfile, 0L, SEEK_SET);
      d->bufp=d->highp=d->filebufr;
      break;

    case 'U':     /* User choice - If not the same choice, eat the line */
      ch=DispSlowGetChar(d);

      if (lastmenu != (byte)toupper(ch))
        DispSkipLine(d);
      break;

    case 'V':     /* Goto */
      d->skipcr=FALSE;

      DispGetToBlank(d, d->scratch);

      sscanf(d->scratch, "%5ld", &offset);
      
      lseek(d->bbsfile, offset, SEEK_SET);
      d->bufp=d->highp=d->filebufr;
      break;

  }

  return SKIP_NONE;
}




