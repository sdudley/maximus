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
static char rcs_id[]="$Id: med_del.c,v 1.1.1.1 2002/10/01 17:52:19 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd Editor: Routines for character deletion and wanton destruction
    name=of message data.  :-)
*/

#include "maxed.h"

void BackSpace(void)
{
  word strlc, cx, cy;

  if (cursor_y-1 > strlen(screen[offset+cursor_x]+1) && cursor_y > 1)
    Cursor_Left();
  else
  {
    if (cursor_y > 1)
    {
      strocpy(screen[offset+cursor_x]+cursor_y-1,
              screen[offset+cursor_x]+cursor_y);

      Putc('\x08');

      Update_Line(offset+cursor_x,--cursor_y,0,TRUE);
    }
    else    /* If at beginning of line */
    {
      if (offset+cursor_x != 1)
      {
        if (cursor_x==1 && offset != 0)
          Scroll_Up(SCROLL_CASUAL,
/*                  cursor_x+SCROLL_CASUAL);*/
                    cursor_x +
                    ((sword)offset-SCROLL_CASUAL >= 0 ? SCROLL_CASUAL : offset));

        strlc=strlen(screen[offset+cursor_x-1]+1)+1;

        /* Copy the soft CR/hard CR indicator */

        *screen[offset+cursor_x-1]=*screen[offset+cursor_x];

        strocpy(screen[offset+cursor_x-1]+linelen(offset+cursor_x-1)+1,
                screen[offset+cursor_x]+1);

        cx=cursor_x;

        Update_Line(offset+cursor_x-(offset+cx != 1 ? 1 : 0),
                    cursor_y=strlc,
                    0,
                    FALSE);

        cy=cursor_y;

        Goto(cx,cy);

        Delete_Line(cx);

        /* Delete_Line only decrements cursor_x if we're on last line of    *
         * document!  That means we have to if we're not.                   */

        if (offset+cx <= num_lines && cursor_x+offset != 1)
          cursor_x--;

        if (strlen(screen[offset+cursor_x]+1) >= usrwidth)
        {
          cx=cursor_x;

          Goto(cx=cursor_x,cursor_y=usrwidth);

          Word_Wrap(MODE_UPDATE);

          cursor_x=cx;

          /* If we didn't get anywhere */

          if (cy >= strlen(screen[offset+cursor_x]+1))
          {
            cursor_x++;
            cy=1;
          }
        }

        Goto(cursor_x,cursor_y=cy);
      }
    }
  }

  Update_Position();
}



void Delete_Char(void)
{
/*  byte save_cr;*/
  int cx, cy, ww;


  if (cursor_y <= linelen(offset+cursor_x))
  {
    if (linelen(offset+cursor_x) != 0)
    {
      strocpy(screen[offset+cursor_x]+cursor_y,
              screen[offset+cursor_x]+cursor_y+1);

      Update_Line(offset+cursor_x, cursor_y, 0, TRUE);
    }
  }
  else    /* At end of line, join! */
  {
    if (offset+cursor_x != num_lines && cursor_y != usrwidth)
    {
      if (cursor_y > (strlen(screen[offset+cursor_x]+1)+1))
      {
        memset(screen[offset+cursor_x]+strlen(screen[offset+cursor_x]+1)+1,
               ' ',
               cursor_y-strlen(screen[offset+cursor_x]));

        screen[offset+cursor_x][cursor_y]='\0';
      }

      strocpy(screen[offset+cursor_x]+cursor_y,
              screen[offset+cursor_x+1]+1);

      *screen[offset+cursor_x]=*screen[offset+cursor_x+1];
/*      save_cr=*screen[offset+cursor_x+1];*/
      Delete_Line(cursor_x+1);

      if (strlen(screen[offset+cursor_x]+1) >= usrwidth)
      {
/*        *screen[offset+cursor_x]=SOFT_CR;*/
            
        cx=cursor_x;
        cy=cursor_y;

        Goto(cx=cursor_x, cursor_y=usrwidth);

        ww=Word_Wrap(MODE_UPDATE);

        if (cy-ww <= 0)
          Goto(cursor_x=cx, cursor_y=cy);
        else Goto(cursor_x, cursor_y=(cy-ww)+1);
      }
      else
      {
        *screen[offset+cursor_x]=HARD_CR;
      }

      Update_Line(offset+cursor_x, cursor_y, 0, TRUE);

      Update_Position();
    }
  }
}



void Delete_Line(int cx)
{
  word line;
  char *p;

  if (num_lines != 1)
  {
    p=screen[cx+offset];

    for (line=cx+offset; line < num_lines; line++)
    {
      screen[line]=screen[line+1];
      update_table[line]=TRUE;
    }

    screen[num_lines]=p;

    update_table[num_lines]=TRUE;

    Free_Line(num_lines);

    if (cursor_x+offset > num_lines)  /* If we were on the very last line */
    {
      if (cursor_x==1 && offset != 0)
        Scroll_Up(SCROLL_CASUAL,cursor_x+SCROLL_CASUAL);
      
      cursor_x=num_lines-offset;
      cursor_y=1;
      Update_Position();
    }

    Goto(cursor_x, cursor_y);
  }
  else             /* We're on the first line, so just erase it! */
  {
    Goto(cursor_x, cursor_y=1);
    Puts(CLEOL);
    screen[offset+cursor_x][cursor_y]='\0';
  }
}






void Delete_Word(void)   /*ABK 1990-09-02 14:25:34 */
{
  if (cursor_y==1 ||
      isalnumpunct(screen[offset+cursor_x][cursor_y]) || 
      isalnumpunct(screen[offset+cursor_x][cursor_y-1]) ||
      cursor_y > strlen(screen[offset+cursor_x]+1))
  {
    while (((isalnumpunct(screen[offset+cursor_x][cursor_y])) &&
           cursor_y <= strlen(screen[offset+cursor_x]+1)) &&
           cursor_y < usrwidth)
      Delete_Char();

    if (cursor_y >= strlen(screen[offset+cursor_x]+1))
      Delete_Char();
    else
    {
      while (! isalnumpunct(screen[offset+cursor_x][cursor_y]) &&
             screen[offset+cursor_x][cursor_y] && cursor_y < usrwidth)
       Delete_Char();
   }

  }
  else
  {
    while ((! isalnumpunct(screen[offset+cursor_x][cursor_y])) &&
           cursor_y < usrwidth)
      Delete_Char();
  }
}


