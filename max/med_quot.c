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
static char rcs_id[]="$Id: med_quot.c,v 1.1.1.1 2002/10/01 17:52:20 sdudley Exp $";
#pragma on(unreferenced)

/*# name=MaxEd editor: Routines for quoting and copying
*/

#include "maxed.h"
#include "m_reply.h"

static int near Quote_Read(void);


void Quote_OnOff(struct _replyp *pr)
{
  XMSG msg;

  /* Null out attribute for line in quoted message */
  last_msg_attr=0;

  if (pr!=NULL)
  {
    if (quoting)  /* If we were already quoting */
    {
      MsgCloseMsg(qmh);

      quoting=FALSE;
      qmh=NULL;

      usrlen += QUOTELINES;
      Fix_MagnEt();
    }
    else
    {
      qmh=MsgOpenMsg(pr->fromsq,
                     MOPEN_READ,
                     MsgUidToMsgn(pr->fromsq,pr->original, UID_EXACT));
      if (qmh==NULL)
        return;

      if (MsgReadMsg(qmh,&msg,0L,0L,NULL,0L,NULL)==-1)
      {
        MsgCloseMsg(qmh);
        return;
      }
      
      Parse_Initials(msg.from,initials);

      quoting=TRUE;
      usrlen -= QUOTELINES;

      if (cursor_x >= usrlen)
      {
        offset += (cursor_x-usrlen)+1;
        cursor_x=usrlen-1;
      }

      if (usr.bits2 & BITS2_CLS)
        Puts(CLS);
      else NoFF_CLS();

      Redraw_Text();
      Redraw_StatusLine();

      cur_quotebuf=-1;
      last_quote=0x7fff;

      Quote_Down();           /* Read the 1st four lines into window */
    }
  }
  else /* Message isn't a reply, so say so */
  {
    Goto(usrlen,1);
    Puts(max_not_reply);
    Goto(cursor_x,cursor_y);

    vbuf_flush();

    while (! Mdm_keyp())
      Giveaway_Slice();

    Redraw_StatusLine();
  }

  vbuf_flush();
}


/* Move the quote window up by four lines */

void Quote_Up(void)
{
  if (cur_quotebuf >= 1)
  {
    last_msg_attr=MSGLINE_NORMAL;
    cur_quotebuf--;
    Quote_Read();
  }
}


/* Move the quote window down by four lines */

void Quote_Down(void)
{
  last_msg_attr=MSGLINE_NORMAL;

  if (cur_quotebuf < MAX_QUOTEBUF-1 && cur_quotebuf < last_quote)
    quote_pos[++cur_quotebuf]=MsgGetCurPos(qmh);

  /* Can't go down if we've reached end-of-message */

  if (Quote_Read())
    last_quote=cur_quotebuf; 
}




static int near Quote_Read(void)
{
  byte linetype[QUOTELINES];
  byte *p[4];
  byte *s2;

  sword temp, qbl, done;
  sword lines_displayed;
  sword wid, swid, len, x;

  lines_displayed=0;

  done=FALSE;

  MsgSetCurPos(qmh, quote_pos[cur_quotebuf]);

  wid=usrwidth-strlen(initials)-HARD_SAFE-2;
  wid=max(20,wid);

  swid=usrwidth-strlen(initials)-SOFT_SAFE-2;
  swid=max(20, swid);

  while (lines_displayed < QUOTELINES)
  {
    for (x=0; x < 4; x++)
      p[x]=quotebuf+((lines_displayed+x)*MAX_LINELEN);

    qbl=Msg_Read_Lines(qmh,
                       QUOTELINES-lines_displayed,
                       wid, swid,
                       p,
                       linetype+lines_displayed,
                       &last_msg_attr,
                       MRL_QEXP);

    if (qbl != QUOTELINES-lines_displayed)
      done=TRUE;

    if (lines_displayed==0)
      for (temp=qbl;temp < QUOTELINES;temp++)
        *(quotebuf+(qbl*MAX_LINELEN))='\0';


    for (temp=lines_displayed; temp < QUOTELINES; temp++)
    {
      s2=quotebuf+(temp*MAX_LINELEN);

      if (linetype[temp] & MSGLINE_SEENBY &&
          !GEPriv(usr.priv,prm.seenby_priv))
        continue;
      else if (linetype[temp] & MSGLINE_KLUDGE &&
               !GEPriv(usr.priv,prm.ctla_priv))
        continue;
      else if (linetype[temp] & MSGLINE_END)
        strcpy(s2,end_widget2);

      len=usrlen+1+lines_displayed;
      
      if (QuoteThisLine(s2))
        Printf(quote_format, len, msg_quote_col, initials, s2);
      else Printf(norm_format, len, msg_quote_col, s2);

      if (temp > lines_displayed)
        strcpy(quotebuf+(lines_displayed*MAX_LINELEN),s2);

      lines_displayed++;

      if (linetype[temp] & MSGLINE_END)
      {
        while (lines_displayed < QUOTELINES)
          Printf(norm_format, usrlen+1+lines_displayed++, msg_quote_col,
                 blank_str);

        /* Break out of loop */

        temp=QUOTELINES;
        break;
      }
    }
  }

  Printf(msg_text_col);

  Goto(cursor_x,cursor_y);  /* Restore original cursor position */
  vbuf_flush();

  return done;
}



void Quote_Copy(void)
{
  word ln, x;

  byte save, *s, *p, *n;

  save=insert;
  insert=TRUE;

  /* Make sure there's nothing in front of us on the current line, and if  *
   * there is, send it to the bit-bucket.                                  */

  for (ln=0; ln < QUOTELINES && num_lines < max_lines-1; ln++)
  {
    if (*(quotebuf+(ln*MAX_LINELEN)) != '\x16')
    {
      /* Add a line */
      if (Allocate_Line(num_lines+1))
        EdMemOvfl();


      /* Now shift everything else down one, so that the new line is       *
       * where we want it to be (the current line).                        */

      p=screen[num_lines];

      for (x=num_lines-1; x >= offset+cursor_x; x--)
      {
        screen[x+1]=screen[x];
        update_table[x+1]=TRUE;
      }

      /* Now put the newly-allocated line right here, after moving         *
       * everything else out of the way...                                 */

      screen[offset+cursor_x]=n=p;

      /* Should be hard-CR delimited */

      *n++=HARD_CR;

      /* Now copy the quote in place */

      if (QuoteThisLine(quotebuf+(ln*MAX_LINELEN)))
      {
        *n++=' ';

        for (s=initials; *s; s++)
          *n++=*s;

        *n++='>';
        *n++=' ';
      }

      strcpy(n,quotebuf+(ln*MAX_LINELEN));

      update_table[offset+cursor_x]=TRUE;
      cursor_x++;
    }
    else break;
  }

  /* Auto scroll quote window down */

  Quote_Down(); 

  if (cursor_x >= usrlen)
    Scroll_Down(SCROLL_CASUAL, cursor_x-SCROLL_CASUAL);

  cursor_y=1;

  insert=save;
}


