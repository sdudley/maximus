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
static char rcs_id[]="$Id: med_scrn.c,v 1.1.1.1 2002/10/01 17:52:21 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd editor: Routines for manipulating the screen
*/

#define MAX_INCL_COMMS

#include "maxed.h"

void Redraw_Text(void)
{
  word x;

  /* Make the entire refresh buffer "dirty" */
  
  Puts(msg_text_col);

  if (update_table)
    for (x=0; x < UPDATEBUF_LEN; x++)
      update_table[x]=TRUE;
}



void Redraw_StatusLine(void)
{
  Goto(usrlen, 1);

  Printf(max_status,
         mmsg->to,
         mmsg->subj+(strnicmp(mmsg->subj, "Re:", 3)==0 ? 4 : 0));

  if (insert)
  {
    Goto(usrlen, usrwidth-13);

    Puts(status_insert);
  }

  Update_Position();

  Puts(CLEOL);
  Puts(msg_text_col);

  Goto(cursor_x, cursor_y);
}


void Redraw_Quote(void)
{
  word temp;

  if (quoting)
  {
    Goto(usrlen+1,usrwidth-12);

    for (temp=0;temp < QUOTELINES;temp++)
      Printf(quote_format, usrlen+1+temp, msg_quote_col, 
             initials, quotebuf+(temp*MAX_LINELEN));

    Puts(CLEOL);
  }

  Puts(msg_text_col);
  Goto(cursor_x,cursor_y);
}



void Update_Line(word cx, word cy, word inc, word update_cursor)
{
  /* Only update lines which are on-screen! */

  if (cx > offset && cx < offset+usrlen)
  {
    if (!Mdm_keyp() || cx==cursor_x+offset)
    {
      /* If we have to save and move */

      if (cx-offset != cursor_x || cy != cursor_y)   
        Goto(cx-offset,cy);

      if (cx != max_lines)
      {
        if (cx > max_lines || screen[cx]==NULL || (cx <= num_lines &&
            screen[cx][cy]=='\0' && screen[cx][1]))
        {
/*          Putc(' '); */
        }
        else Printf("%0.*s",usrwidth-cy,screen[cx]+cy);
      }
      else Printf(end_widget, msg_text_col);

      if (cx >= max_lines || screen[cx]==NULL ||
          (cy+strlen(screen[cx]+cy) <= usrwidth))
        Puts(CLEOL);

      if (update_cursor)
        Goto(cx-offset,cy+inc);

      if ((word)update_table[cx] >= cy)
        update_table[cx]=FALSE;
    }
    else update_table[cx]=TRUE;
  }
}


/* Ultra-ugly wordwrap routine (but it works!) */

int Word_Wrap(word mode)
{
  word cx, cy, y;
  byte *scx, *lastpos;
  word zerolen;
  word col;

  
  /* Get over any ending whitespace on this line */
  
  scx=screen[offset+cursor_x];

  for (col=usrwidth; is_wd(scx[col]); col--)
    ;

  /* Now find the beginning of the word */

  for (; col >= (word)(usrwidth-MAX_WRAPLEN); col--)
  {
    if (is_wd(scx[col]) || col==(word)(usrwidth-MAX_WRAPLEN))
    {
      /* Found it! */
      
      /* 'col' used to point to the whitespace; we want it to point to the    *
       * start of the word.                                                 */
      
      col++;

      /* Put the cursor here, unless we're told not to move it */
      
      if (mode != MODE_NOUPDATE)
        Goto(cursor_x, cursor_y-(usrwidth-col));

      /* Update virtual cursor position to reflect placement of new word */
      
      if ((cursor_y -= (usrwidth-col)) < 1)
        cursor_y=1;

      cx=cursor_x+1;

      /* If we need to allocate any more lines at the end, then do so. */
      
      if (offset+cx > num_lines && offset+cx < max_lines)
      {
        for (y=num_lines+1; y <= cx+offset; y++)
        {
          if (Allocate_Line(y))
            EdMemOvfl();
        }
      }
      else if (offset+cx==max_lines || screen[offset+cursor_x][0]==HARD_CR)
      {
        /* else we're trying to insert a line in the middle of the text,   *
         * so plop it in the right spot.                                   */
        
        if (! Insert_Line_Before(cx)) /* If we can't insert (end of text?) */
        {
          screen[offset+cx][col]='\0';
          Update_Line(offset+cx,1,0,TRUE);
          return col;
        }
      }
      
      scx=screen[offset+cursor_x];
      
      lastpos=scx+strlen(scx)-1;
      
      /* Make sure that there's a space between words when wrapping */
      
      if (! is_wd(*lastpos) && scx[0]==SOFT_CR)
      {
        *++lastpos=' ';
        *++lastpos='\0';
      }

      /* Now state that we've wordwrapped */

      scx[0]=SOFT_CR;

      /* If there's something on the next line to wrap... */

      if (screen[offset+cx][1])
      {
        if (strlen(scx+col) >= usrwidth)
          *(scx+col+usrwidth)='\0';

        /* Move line over, to make room for new word */

        strocpy(screen[offset+cx]+1 + strlen(scx+col),
                screen[offset+cx]+1);
              
        /* Put new word in place */

        memmove(screen[offset+cx]+1,
                scx+col,
                y=strlen(scx+col));

        /* Remove the first word from the prior line */

        scx[col]='\0';

        /* Display that on-screen */

        if (cursor_y < usrwidth && mode != MODE_NOUPDATE)
          Puts(CLEOL);

        /* If this causes a scroll, then do so */

        if ((mode==MODE_SCROLL /*|| mode==MODE_UPDATE*/) &&
            (cursor_x+1 >= usrlen))
          Scroll_Down(SCROLL_LINES,cursor_x-SCROLL_LINES);

        /* If the word we just moved was zero-length */

        if (y==0 || (col <= (word)strlen(screen[offset+cursor_x]+1)+1 &&
                     ! is_wd(screen[offset+cursor_x][col]) &&
                     screen[offset+cx][1]))
        {
          zerolen=TRUE;
        }
        else zerolen=FALSE;

        /* Go to a new line. */

        New_Line(y);
      }
      else /* else there's nothing on the next line, so just copy */
      {
        memmove(screen[offset+cx]+1,
                scx+col,
                y=strlen(scx+col)+1);

        /* Get rid of the original word */

        scx[col]='\0';

        /* ...and show the deletion on-screen. */

        if (cursor_y < usrwidth)
          Puts(CLEOL);

        /* Scroll, if necessary */
        
        if ((mode==MODE_SCROLL /*|| mode==MODE_UPDATE*/) &&
            (cursor_x+1 >= usrlen))
        {
          Scroll_Down(SCROLL_LINES,cursor_x-SCROLL_LINES);
          cx=cursor_x;
        }

        /* Go to a new line */

        New_Line(--y);

        /* ...and set a flag, based on the length of this word */

        if (y==0)
          zerolen=TRUE;
        else zerolen=FALSE;
      }

      /* If this has caused the *current* line to be in need of a wrap... */
      if (strlen(screen[offset+cursor_x]+1) >= usrwidth)
      {
        /* Dummy up the cursor position accordingly */

        cy=cursor_y;
        cx=cursor_x;
        cursor_y=usrwidth;

        /* And make a recursive call to ourselves */

        Word_Wrap(MODE_NOUPDATE);

        /* Fix cursor position back to where we were */

        cursor_x=cx;
        cursor_y=cy+1;
      }
      
      /* Put cursor in final spot */

      if (cx < TermLength())
        Goto(cx,cursor_y);

      /* Indicate that this line was updated */
      update_table[offset+cursor_x]=TRUE;

      /* And move cursor appropriately. */
      if (! zerolen)
        cursor_y++;
      break;
    }
  }

  /* If the word was the maximum length allowable */
  
  #ifdef NEVER
  if (col==usrwidth-MAX_WRAPLEN)
  {
    /* Chop off anything */
    col=cursor_y;
    screen[offset+cursor_x][usrwidth-1]='\0';
    
    if (cursor_y==usrwidth)
      Carriage_Return(FALSE);
    else
    {
      screen[offset+cursor_x][usrwidth-2]='\0';
      Update_Line(offset+cursor_x, cursor_y, 0, FALSE);
    }
  }
  #endif

  /* If this is the last recursive call, then update our position */
  if (mode==MODE_UPDATE || mode==MODE_SCROLL)
    Update_Position();

  return col;
}




void Toggle_Insert(void)
{
  insert=(char)!insert;

  Goto(usrlen,usrwidth-13);

  Puts(insert ? status_insert : insrt_ovrwrt);

  Goto(cursor_x, cursor_y);
  Puts(msg_text_col);
}





void Update_Position(void)
{
#ifdef UPDATE_POSITION
  if (! Mdm_keyp() && !skip_update)
  {
    pos_to_be_updated=FALSE;

    Goto(usrlen,70);
    Printf("%u:%u  ",offset+cursor_x,cursor_y);
    Goto(cursor_x,cursor_y);
  }
  else pos_to_be_updated=TRUE;
#endif

#ifdef OS_2
#ifdef TTYVIDEO
  if (displaymode==VIDEO_IBM)
#endif
    Vidfcur();
#endif
}


void NoFF_CLS(void)
{
  Goto(1,1);

  Printf(CYAN "\x16\x19\x03" CLEOL "\n%c", TermLength());

  Goto(1,1);
}




void Fix_MagnEt(void)
{
  Puts(maxed_init);

  if (!(usr.bits2 & BITS2_CLS))
    NoFF_CLS();

  Redraw_Text();
  Redraw_StatusLine();
  Redraw_Quote();

  (void)mdm_ctrlc(0);
  Mdm_Flow_Off();
}




void Do_Update(void)
{
  static word lastofs=-1;
  word x, y;
  word updated;
  word last_update=FALSE;

  updated=FALSE;

  y=offset+usrlen;

  for (x=offset; x < y && ! Mdm_keyp(); x++)
  {
    /* If it's in the update buffer, update it in the normal manner */

    if (x < UPDATEBUF_LEN)
    {
      if (update_table[x])
      {
        if (updated && offset+cursor_x==x)
          Goto(cursor_x,cursor_y);

        Update_Line(x, 1, 0, FALSE);
        updated=TRUE;
        last_update=TRUE;
      }
      else if (x != max_lines)
        last_update=FALSE;
    }
    else
    {
      /* Otherwise, it's not on the update buffer, so just clear it, if   *
       * the last line on-screen WAS updated.                             */

      if (last_update || offset != lastofs)
      {
        Goto(x-offset, 1);
        Puts(CLEOL);
      }
    }
  }

  lastofs=offset;

  if (updated)
  {
    Goto(cursor_x, cursor_y);
    vbuf_flush();
  }
}
