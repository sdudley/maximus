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
static char rcs_id[]="$Id: med_add.c,v 1.1.1.1 2002/10/01 17:52:19 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd editor: Routines for adding characters and lines to message
*/

#include "maxed.h"

static int near Insert_Line_Before_CR(int cx);
static void near Insert_At(word cx, word cy, char ch, word inc);


/* Make sure that line is filled with real cahracters up to current          *
 * cursor posn.                                                              */

static void near AddFill(void)
{
  if (cursor_y > strlen(screen[offset+cursor_x]+1)+1)
  {
    memset(screen[offset+cursor_x]+strlen(screen[offset+cursor_x]+1)+1,
           ' ',
           cursor_y-strlen(screen[offset+cursor_x]+1)); /*SJD Mon  07-06-1992  16:03:35 */

    screen[offset+cursor_x][cursor_y]='\0';
  }
}


/* Add a character to the current line, handling wordwrap and parawrap */

void Add_Character(int ch)
{
  word cx, cy;
  word oldoffset;
  word col;

  AddFill();

  if (cursor_y==usrwidth)
  {
    cy=cursor_y;
    col=Word_Wrap(MODE_SCROLL);            /* Scroll if necessary */
    cursor_y=(cy-col)+1;
  }

  AddFill();

  /* If insert mode is on, or we're in the last column */

  if (insert || screen[offset+cursor_x][cursor_y]=='\0')
  {
    Insert_At(cursor_x,cursor_y,(char)ch,1);

    if (screen[offset+cursor_x][cursor_y]=='\0')
      Putc(ch);

    if (strlen(screen[offset+cursor_x]+1) >= usrwidth)
    {
      cx=cursor_x;
      cy=cursor_y;
      oldoffset=offset;

      cursor_y=usrwidth;

      Goto(cx,cursor_y);

      col=Word_Wrap(MODE_UPDATE);
      
      if (cursor_x >= usrlen)
        Scroll_Down(SCROLL_LINES,cursor_x-SCROLL_LINES);

      if (cy > strlen(screen[oldoffset+cx]+1))
      {
        /*if (cy-x+1 >= 1 && cy-col+1 <= usrwidth)*/
          Goto(cursor_x, cursor_y=cy-col+1);
      }
      else
      {
        Goto(cursor_x=cx-(offset-oldoffset), cursor_y=cy);
      }
    }
  }
  else
  {
    screen[offset+cursor_x][cursor_y++]=(char)ch;
    Putc(ch);
  }

  Update_Position();
}


/* Insert a character at a specific position and update screen display */

static void near Insert_At(word cx, word cy, char ch, word inc)
{
  strocpy(screen[offset+cx]+cy+1,
          screen[offset+cx]+cy);

  screen[offset+cx][cy]=ch;

  screen[offset+cx][usrwidth+1]='\0';

  if (screen[offset+cx][cy+1] != '\0')
    Update_Line(offset+cursor_x, cursor_y, inc, TRUE);

  cursor_y += inc;
}


/* Sends us to a new virtual line, no matter what!  Doesn't do             *
 * word-wrapping, splitting, etc.                                          */

void New_Line(int col)
{                         
  word x;

  if (cursor_x+offset+1 < max_lines)
  {
    cursor_x++;
    cursor_y=(col==0 ? 1 : col);

    if (cursor_x+offset > num_lines)
    {
      for (x=num_lines+1; x <= cursor_x+offset; x++)
      {
        if (Allocate_Line(x))
          EdMemOvfl();
      }
    }
  }
}



word Carriage_Return(int hard)
{
  word cx;
  word temp;
  word added_line;
  word line;
    
  byte save_cr;

  if (eqstri(screen[cursor_x]+1, "^z"))
  {
    screen[cursor_x][1]='\0';
    return TRUE;
  }

  added_line=FALSE;

  if (cursor_x+1 >= usrlen)
    Scroll_Down(SCROLL_LINES,cursor_x-SCROLL_LINES);

  cx=cursor_x+1;

  /* If we're on last line of message, allocate enuf lines to fill to end */
  
  if (cx+offset+1 >= num_lines && (num_lines < max_lines-1))
  {
    /* For each one until num_lines+1, allocate a line */
    
    for (line=num_lines+1; line <= cx+offset; line++)
    {
      added_line=TRUE;

      if (Allocate_Line(line))
        EdMemOvfl();
    }
  }

  temp=1;

  /* If we need to insert a line in the middle, and there's enough room */
  
  if (insert && num_lines < max_lines-1)
  {
    save_cr=*screen[offset+cursor_x];
    
    /* If we're supposed to do a hard C/R, then do so */

    if (hard)
      *screen[offset+cursor_x]=HARD_CR;

    if (! (cx+offset > num_lines) && !added_line)
    {
      if (Allocate_Line(num_lines+1))
        EdMemOvfl();
    }

    if (cursor_y < usrwidth)
      Puts(CLEOL);

    if ((temp=Insert_Line_Before_CR(cx)) != 0)
    {
      /* If we need to split this line in two... */

      if (cursor_y <= strlen(screen[offset+cursor_x]+1))
      {
        strocpy(screen[offset+cx]+1,
                screen[offset+cursor_x]+cursor_y);
      }
      else screen[offset+cx][1]='\0'; /* else blank out the next line */

      /* Copy the attribute to the next line */

      *screen[offset+cx]=save_cr;

      /* Chop off this line, where we split it. */

      screen[offset+cursor_x][cursor_y]='\0';

      Update_Line(offset+cx, 1, 0, FALSE);
    }
  }

  if (temp && offset+cursor_x != max_lines-1)
  {
    Goto(++cursor_x, cursor_y=1);

    Update_Position();
  }

  return FALSE;
}


static int near Insert_Line_Before_CR(int cx)
{
  word x;
  char *p;

  if (num_lines < max_lines)
  {
    if (screen[num_lines][1] != '\0')
    {
      if (Allocate_Line(num_lines+1))
        EdMemOvfl();
    }

    p=screen[num_lines];

    for (x=min(num_lines-1,max_lines-1); x >= offset+cx; x--)
    {
      screen[x+1]=screen[x];
      update_table[x+1]=TRUE;
    }

    screen[offset+cx]=p;

    return 1;
  }
  else
  {
    Goto(cursor_x,cursor_y=1);
    return 0;
  }
}



int Insert_Line_Before(int cx)
{
  word x;
  char *p;

  if (num_lines < max_lines-1)
  {
    if (screen[num_lines][1] != '\0')
    {
      if (Allocate_Line(num_lines+1))
        EdMemOvfl();
    }

    p=screen[num_lines];

    for (x=min(num_lines-1,max_lines-1); x >= offset+cx; x--)
    {
      screen[x+1]=screen[x];
      update_table[x+1]=TRUE;
    }

    screen[offset+cx]=p;

    return 1;
  }
  else
  {
    Goto(cursor_x,cursor_y=1);
    return 0;
  }
}


