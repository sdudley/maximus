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
static char rcs_id[]="$Id: med_move.c,v 1.1.1.1 2002/10/01 17:52:20 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd editor: Routines for moving cursor around the screen
*/

#include "maxed.h"

static void near Up_a_Line(word *cx, word *cy);
static void near Down_a_Line(word *cx, word *cy);

void Cursor_Left(void)
{
  if (cursor_y > 1)
  {
    Puts(LEFT);
    cursor_y--;
    Update_Position();
  }
}




void Cursor_Right(void)
{
  if (cursor_y < usrwidth)
  {
    Puts(RIGHT);
    cursor_y++;
    Update_Position();
  }
}




void Cursor_Up(void)
{
  if (offset+cursor_x > 1)
  {
    if (cursor_x==1 && offset != 0)
      Scroll_Up(SCROLL_CASUAL,cursor_x+SCROLL_CASUAL);

    Puts(UP);
    cursor_x--;
    Update_Position();
  }
}






void Cursor_Down(int update_pos)
{
  if (cursor_x+offset < num_lines)
  {
    if (cursor_x+1 >= usrlen)
      Scroll_Down(SCROLL_CASUAL,cursor_x-SCROLL_CASUAL);

    cursor_x++;

    if (update_pos)
    {
      Puts(DOWN);
      Update_Position();
    }
  }
}




void Cursor_BeginLine(void)
{
  Goto(cursor_x,cursor_y=1);
  Update_Position();
}




void Cursor_EndLine(void)
{
  Goto(cursor_x,cursor_y=strlen(screen[offset+cursor_x]+1)+1);
  Update_Position();
}



void Word_Left(void)
{
  word cx=cursor_x;
  word cy=cursor_y;

  if (cy > 2)
    cy--;

  if (!isalnumpunct(screen[offset+cx][cy]) || 
      !isalnumpunct(screen[offset+cx][cy-1]) || 
      cy==1 ||
      cy > strlen(screen[offset+cx]+1))
  {
    while (((! isalnumpunct(screen[offset+cx][cy])) ||
            cy > strlen(screen[offset+cx]+1)) &&
            cy > 1)
      cy--;

    if (cy==1 && (isalnumpunct(screen[offset+cx][1]) ?
                  ((isalnumpunct(screen[offset+cx][2]) || cursor_y==1)) :
                  TRUE))
    {
      Up_a_Line(&cx,&cy);
    }
    else
    {
      while (isalnumpunct(screen[offset+cx][cy]) && cy > 1)
        cy--;

      if ((cy==1 && ! isalnumpunct(screen[offset+cx][1])) || cy > 1)
        cy++;
    }
  }
  else
  {
    while ((isalnumpunct(screen[offset+cx][cy])) && cy > 1)
      cy--;

    if ((cy==1 && ! isalnumpunct(screen[offset+cx][1])) || cy > 1)
      cy++;
  }

  if (cx != cursor_x || cy != cursor_y)
  {
    cursor_x=cx;
    cursor_y=cy;

    if ((sword)cursor_x <= 0 && offset != 0)
      Scroll_Up(SCROLL_CASUAL,cursor_x+SCROLL_CASUAL);

    Goto(cursor_x,cursor_y);
    Update_Position();
  }
}


void Word_Right(void)
{
  word cx=cursor_x;
  word cy=cursor_y;

  if (isalnumpunct(screen[offset+cx][cy]) ||
      isalnumpunct(screen[offset+cx][cy-1]) ||
      cy==1 ||
      cy > strlen(screen[offset+cx]+1))
  {
    while (((isalnumpunct(screen[offset+cx][cy])) &&
             cy <= strlen(screen[offset+cx]+1)) &&
             cy < usrwidth)
      cy++;

    if (cy >= strlen(screen[offset+cx]+1) && cy <= cursor_y)
      Down_a_Line(&cx,&cy);
    else while (! isalnumpunct(screen[offset+cx][cy]) &&
                screen[offset+cx][cy] && cy < usrwidth)
      cy++;
  }
  else while ((! isalnumpunct(screen[offset+cx][cy])) && cy < usrwidth)
      cy++;

  if (cx != cursor_x || cy != cursor_y)
  {
    cursor_x=cx;
    cursor_y=cy;

    if (cursor_x >= usrlen)
      Scroll_Down(SCROLL_CASUAL,cursor_x-SCROLL_CASUAL);

    Goto(cursor_x,cursor_y);
    Update_Position();
  }
}




static void near Up_a_Line(word *cx, word *cy)
{
  if ((*cx)+offset != 1)
  {
    *cx=*cx-1;
    *cy=strlen(screen[offset+(*cx)]+1)+1;
  }
  else *cy=1;
}



static void near Down_a_Line(word *cx, word *cy)
{
  if (*cx+offset != num_lines)
  {
    *cx=*cx+1;

    for (*cy=1;screen[offset+*cx][*cy];(*cy)++)
      if (isalnumpunct(screen[offset+*cx][*cy]))
        break;
  }
  else *cy=strlen(screen[offset+*cx]+1)+1;
}



void Scroll_Up(int n,int location)
{
  if ((sword)offset-(sword)n >= 0)
    offset -= n;
  else offset=0;

  cursor_x=location;
  Redraw_Text();
}




void Scroll_Down(int n,int location)
{
  if ((sword)offset+n >= (sword)num_lines)
  {
    offset=num_lines;
    location=1;
  }
  else offset += n;

  cursor_x=location;
  Redraw_Text();
}





void Page_Up(void)
{
  if (quoting)
    Quote_Up();
  else
  {
    if (offset==0)
    {
      Goto(cursor_x=1,cursor_y);
    }
    else Scroll_Up(usrlen-1,cursor_x);

    Update_Position();
  }
}




void Page_Down(void)
{
  if (quoting)
    Quote_Down();
  else
  {
    if (offset+usrlen > num_lines)
      Goto(cursor_x=num_lines-offset,cursor_y);
    else
    {
      offset += usrlen-1;

      if (offset > num_lines)
        offset=num_lines-1;

      if (offset+cursor_x > num_lines)
        cursor_x=num_lines-offset;

      Redraw_Text();
    }

    Update_Position();
  }
}


