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
static char rcs_id[]="$Id: med_misc.c,v 1.1.1.1 2002/10/01 17:52:19 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd editor: Miscellaneous routines (overlaid)
*/

#define MAX_INCL_COMMS

#include "maxed.h"
#include <stddef.h>


/*static char * pig(char *s,char *temp);*/


void MagnEt_Help(void)
{
  Mdm_Flow_On();
  display_line=display_col=1;
  Display_File(0,NULL,PRM(oped_help));
  Fix_MagnEt();
}




void MagnEt_Menu(void)
{
  if (usr.bits2 & BITS2_CLS)
    Puts(CLS);
  else NoFF_CLS();

  Mdm_Flow_On();
  Bored_Menu(mmsg);
  Fix_MagnEt();
}



#ifdef NEVER /* can cause system crashes */

void Piggy(void)
{
  word x, ofs, cx, cy;

  byte temp[MAX_LINELEN*2];
  byte temp2[PATHLEN];
  byte *o, *s, *p, *l;

  if (no_piggy)
    return;

  ofs=offset;
  cx=cursor_x;
  cy=cursor_y;

  for (x=1; x <= num_lines; x++)
  {
    for (s=screen[x]+1; *s; s++)
    {
      if (isalpha(*s))
      {
        o=s;

        p=temp;

        while (isalpha(*s) || *s=='\'')
          *p++=*s++;

        *p='\0';

        /* Make room for "ay" */

        strocpy(s+2, s);

        l=pig(temp, temp2);
        memmove(o, l, strlen(l));
        s++;

        if (s-(screen[x]+1)+strlen(l) >= (ptrdiff_t)usrwidth)
        {
          offset=x-1;
          cursor_x=1;
          cursor_y=strlen(screen[x]+1)+1;

          Word_Wrap(MODE_NOUPDATE);            /* Scroll if necessary */

          x=offset+cursor_x;
          s=screen[x];    /* the "s++" incs this later */

          if (x > num_lines || x > max_lines)
            break;
        }
      }
    }
  }

  offset=ofs;
  cursor_x=cx;
  cursor_y=cy;

  Redraw_Text();
}



static char * pig(char *s,char *temp)
{
  char *p;

  strcpy(temp,s+1);
  p=temp+strlen(temp);

  if (isupper(*s))
    *p++=(char)tolower(*s);
  else *p++=*s;

  strcpy(p,"ay");

  if (isupper(*s))
    *temp=(char)toupper(*temp);

  return temp;
}

#endif /* NEVER */



void MagnEt_Bad_Keystroke(void)
{
  Goto(usrlen,1);

  Printf(max_no_understand,ck_for_help);

  Goto(cursor_x,cursor_y);
  vbuf_flush();

  if (Mdm_getcwcc()==0)
    Mdm_getcwcc();

  /*
  while (! Mdm_keyp() && !brk_trapped)
    Giveaway_Slice();
  */

  Redraw_StatusLine();
}



