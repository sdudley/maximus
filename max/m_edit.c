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
static char rcs_id[]="$Id: m_edit.c,v 1.3 2004/01/27 21:00:31 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Routines used by msg section and editor stuff
*/

#define MAX_LANG_max_bor

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"


/* Allocate a block of memory for another line in MaxEd/BORED */

sword Allocate_Line(int linenum)
{
  if ((screen[linenum]=(char *)malloc(MAX_LINELEN+2))==NULL)
  {
    /* Try to leave a bit of memory left over, for our return process. */
    
    if (linenum && screen[linenum-1])
      free(screen[linenum-1]);

    return -1;
  }

  screen[linenum][0]=HARD_CR;
  screen[linenum][1]='\0';

  num_lines++;

  return 0;
}


/* Free the memory for a line in MaxEd/BORED */

void Free_Line(int linenum)
{
  if (screen[linenum])
    free(screen[linenum]);

  screen[linenum]=NULL;
  num_lines--;
}


/* Free all of the currently-allocated lines in MaxEd/BORED */

void Free_All(void)
{
  int i;

  for (i=num_lines; i >= 1; i--)
    Free_Line(i);

  if (screen[max_lines] && max_lines > 0)
  {
    Free_Line(max_lines);
    screen[max_lines]=NULL;
  }
}

/* Take a string (such as that from the message 'from' field), and convert  *
 * it into the initials of the sender.                                      */

void Parse_Initials(char *msgfrom, char *initials)
{
  char *p, *s;
  char os[36];
  word quota;


  strncpy(os, msgfrom, 35);
  msgfrom[35]='\0';

  /* Copy the user's initials into the string initials[]. */

  p=strtok(os,ctl_delim);

  for (s=initials, quota=MAX_INITIALS-2;
       p && s < initials+9 && quota-- > 0;
       p=strtok(NULL, ctl_delim))
  {
    *s++=*p;
  }

  *s='\0';
}


/* Determines whether or not a line needs to be quoted.  (If it's blank,    *
 * or if there's another quote marker in the first five characters of       *
 * the line, then it doesn't.)                                              */

word QuoteThisLine(char *txt)
{
  char *pointy_thing;
  
  pointy_thing=strchr(txt, '>');
  
  return (*txt != '\0' && (!pointy_thing || pointy_thing >= txt+5) &&
          !eqstri(txt, end_widget2));
}

