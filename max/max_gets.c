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

/* $Id: max_gets.c,v 1.6 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=Maximus get-string function
*/

#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <mem.h>
#include <time.h>
#include <stdarg.h>
#include "prog.h"
#include "keys.h"
#include "mm.h"

#ifdef EMSI
#include "emsi.h"
#endif



static char *str;
static int num_ch;
static int cur_pos;

#ifdef TEST_VER
  void Zoquo(void);
#else
  #define Zoquo()
#endif



static void near Mdmgets_Home(void)
{
  if (cur_pos)
  {
    Printf(left_x,cur_pos);
    cur_pos=0;
  }
}

static void near Mdmgets_End(int type,int c)
{
  char temp[PATHLEN];
  if (str[cur_pos])
  {
    Puts(Show_Pwd(str+cur_pos,temp,
                  (char)((type & INPUT_ECHO) ? c : 0)));
    cur_pos += strlen(str+cur_pos);
  }
}


static void near Mdmgets_Cleft(void)
{
  int orig_pos;

  if (cur_pos)
  {
    orig_pos=cur_pos;

    cur_pos--;

    while (cur_pos && !isalnum(str[cur_pos]))
      cur_pos--;

    while (cur_pos && isalnum(str[cur_pos]))
      cur_pos--;

    if (! isalnum(str[cur_pos]))
      cur_pos++;

    if (orig_pos-cur_pos)
      Printf("\x16\x19\x02" LEFT "%c", orig_pos-cur_pos);
  }
}


static void near Mdmgets_Cright(void)
{
  int orig_pos;

  if (cur_pos < num_ch)
  {
    orig_pos=cur_pos;

    while (cur_pos < num_ch && isalnum(str[cur_pos]))
      cur_pos++;

    while (cur_pos < num_ch && !isalnum(str[cur_pos]))
      cur_pos++;

    if (cur_pos-orig_pos)
      Printf("\x16\x19\x02" RIGHT "%c",cur_pos-orig_pos);
  }
}




static void near Mdmgets_Left(void)
{
  if (cur_pos)
  {
    Puts(LEFT);
    cur_pos--;
  }
}


static void near Mdmgets_Right(int type,int c)
{
  if (str[cur_pos])
  {
    if (type & INPUT_ECHO)
      Putc(c);
    else Putc(str[cur_pos]);

    cur_pos++;
  }
}


  
static void near Rewrite_Line(int type,int c)
{
  char temp[PATHLEN];
  byte last_col;
  
  /* If there's something to write over */

  if (usr.video)
  {
    last_col=current_col;
    
    Puts(Show_Pwd(str+cur_pos,temp,
                  (char)((type & INPUT_ECHO) ? c : 0)));

    Putc(' ');

    Goto(current_line, last_col);
    
    /*Printf("\x19\x08%c",strlen(str+cur_pos)+1);*/
  }
}



static void near Mdmgets_Bs(int type,int c)
{
  if (! cur_pos)
    return;

  strocpy(str+cur_pos-1, str+cur_pos);


  Puts(bs_sp_bs);
  num_ch--;
  cur_pos--;

  /* If we need to shift the rest of the string... */

  if (str[cur_pos])
    Rewrite_Line(type,c);
}


static void near Mdmgets_Del(int type,int c)
{
  if (num_ch==0 || cur_pos==num_ch)
    return;

  strocpy(str+cur_pos, str+cur_pos+1);

  num_ch--;

  Rewrite_Line(type,c);
}


static void near Mdmgets_Clear(int type)
{
  if ((type & INPUT_MSGENTER) && usr.video && num_ch)
  {
    /* First go back to beginning of line, and clear it. */

    if (cur_pos)
      Printf("\x19\x08%c", cur_pos);

    if ((type & INPUT_NOCLEOL)==0)
      Puts(CLEOL);
    else Printf("\x19 %c\x19\x08%c", num_ch, num_ch);

    str[num_ch=cur_pos=0]='\0';
  }
}


int DoEditKey(int type, char *str, int key, int c)
{
  if (type & INPUT_SCAN)
    return key << 8;

  if (!usr.video)
    return FALSE;

  switch (key)
  {
    case K_HOME:
      if (str)
        Mdmgets_Home();
      break;

    case K_END:
      if (str)
        Mdmgets_End(type, c);
      break;

    case K_DEL:
      if (str)
        Mdmgets_Del(type, c);
      break;

    case K_LEFT:
      if (str)
        Mdmgets_Left();
      break;

    case K_RIGHT:
      if (str)
        Mdmgets_Right(type, c);
      break;

    case K_UP:            /* UP */
    case K_STAB:          /* Shift-TAB */
      if (type & INPUT_MSGENTER)
        return MSGENTER_UP;
      break;

    case K_DOWN:          /* DOWN */
      if (type & INPUT_MSGENTER)
        return MSGENTER_DOWN;
      break;

    case K_CLEFT:
      if (str)
        Mdmgets_Cleft();
      break;

    case K_CRIGHT:
      if (str)
        Mdmgets_Cright();
      break;
  }
  
  return 0;
}


/* Get string from modem:  Max. length of `max', and specific options
   (having characters already entered, echoing a specific character) are
   contained in `type'.  The value of `c' is also determined by `type'.   */

int mdm_gets(char *string, int type, int c, int max, char *prompt)
{
  char temp[PATHLEN];
  char *msgprompt;
  int timer2, rc;
  byte ch=0;

  str=string;
  num_ch=0;

#ifdef EMSI
  EmsiCheck(0);
#endif

  if (max <= 0)
    max=BUFLEN;

  if (type & (INPUT_MSGENTER | INPUT_WORDWRAP | INPUT_DEFAULT))
  {
    if (!(type & INPUT_NOECHO))
    {
      if ((type & INPUT_NOCLEOL)==0)
        Puts(CLEOL);

      Puts(string);
    }

    num_ch=strlen(string);
  }

  /* If a character was already entered by the user */
  if (type & INPUT_ALREADYCH)
  {
    string[num_ch++]=(char)c;
    if (!(type & INPUT_NOECHO))
      Putc(c);
  }

  vbuf_flush();
  cur_pos=num_ch;

  while (ch != K_RETURN)
  {
    /* We have to do this timer-checking stuff manually, since Mdm_keyp() *
     * isn't active for long enough to sense a timeout...  Mdm_getcw()    *
     * *does* do the checking normally, but we only call it once we       *
     * finally get a character...                                         */

    timer2=FALSE;
    input_timeout=timerset(timeout_tics);

    string[num_ch]='\0';
    msgprompt=prompt ? prompt : blank_str;

    while (! Mdm_keyp())
    {
      if (halt())
      {
        if (prompt && !(type & (INPUT_NOCTRLC|INPUT_NOECHO)))
        {
          mdm_dump(DUMP_ALL);
          ResetAttr();

          num_ch=cur_pos=0;

          Putc('\n');
          Puts(prompt);

          vbuf_flush();
          string[num_ch]='\0';
        }
      }

      Check_Time_Limit(&input_timeout, &timer2);
      Check_For_Message(msgprompt, string);
      Giveaway_Slice();
    }

    ch=(unsigned char)Mdm_getcw();
#warning Potential security problem? Can remote send local scan code?

#ifdef EMSI
    /* Check for IEMSI caller.  If we're supposed to eat the character,     *
     * setting it to '1' will cause the loop below to ignore it.            */

    if (EmsiCheck(ch))
    {
      string[num_ch=0]='\0';
      ch=1;
    }
#endif

    timer2=FALSE;

    string[num_ch]='\0';

    switch (ch)
    {
      case K_ONEMORE:      /* IBM extended key code */
        while (! Mdm_keyp())
        {
          Check_For_Message(NULL, NULL);
          Giveaway_Slice();
        }

        if (Mdm_kpeek()==K_ALTY)
        {
          Mdm_getcw();
          Zoquo();
        }

#if K_ONEMORE == K_ESC /* UNIX */
	if (Mdm_kpeek() == K_ESC)
	{
	  ch=(unsigned char)Mdm_getcw();
	  if (ch == K_ESC)
	    goto realEscape;
	}
#endif

#if defined(TEST_VER) && defined(OS_2)
        else if (loc_peek()==K_ALTB)
        {
          char far *null = 0L;

          loc_getch();
          *null = 'z';                    /* cause GP fault on purpose */
        }
#endif
        else if (loc_peek()==K_ALTJ) /* Local shell to DOS */
        {
          loc_getch();

          Shell_To_Dos();

          Printf("\n%s%s", prompt ? prompt : blank_str, string);
        }
        else if ((rc=DoEditKey(type, string, Mdm_getcw(), c)) != FALSE)
          return rc;
        break;

#ifdef OS_2 /*PLF Sun  09-15-1991  16:29:12 */
      case K_CTRLC:
      case K_CTRLK:
         {
            mdm_dump(DUMP_ALL);
            brk_trapped=TRUE;
         }
         break;
#endif

      case K_CTRLE:
        if ((type & INPUT_MSGENTER) && usr.video)
          return MSGENTER_UP;
        break;

#ifndef UNIX
       case K_VTDEL:         /* VT-100 DEL! */
          if (usr.bits2 & BITS2_IBMCHARS)
          {
            DoEditKey(type, string, K_DEL, c);
            break;
          }
        /* else fall-thru */
#endif

#ifdef UNIX
      case K_BS:            /* BackSpace! */
      case K_VTDEL:            /* BackSpace! */
#else
      case K_BS:            /* BackSpace! */      
#endif
        if (!(type & INPUT_NOECHO))
          Mdmgets_Bs(type,c);
        break;

      case K_RETURN:            /* Return */
        EatNulAfterCr();

        if (!(type & INPUT_NOECHO))
        {
          if (type & INPUT_NOLF)
            Putc('\r');
          else Putc('\n');
        }

        if (type & INPUT_MSGENTER)
          return MSGENTER_DOWN;
        break;

      case K_TAB:
      case K_CTRLX:          /* Ctrl-X */
        if ((type & INPUT_MSGENTER) && usr.video)
          return MSGENTER_DOWN;
        else if (ch==K_CTRLX)
        {
          if (!(type & INPUT_NOECHO))
          {
            Puts(bs_sn);

            if (prompt)
              Puts(prompt);
          }

          string[num_ch=cur_pos=0]='\0';
        }
        break;
        
      case K_CTRLY:
        if (!(type & INPUT_NOECHO))
          Mdmgets_Clear(type);
        break;

#if K_ONEMORE == K_ESC /* UNIX */
      realEscape:
#else
      case K_ESC:      /* ESC */
#endif
        if ((ch=Mdm_getcw())==K_ESC)
        {
          if (type & INPUT_MSGENTER)
            return -1;
        }
        else if (ch=='[' || ch=='O')
        {
          ch=Mdm_getcw();

          rc=DoEditKey(type,
                       string,
                       ch=='A' ? K_UP :    ch=='B' ? K_DOWN  :
                       ch=='C' ? K_RIGHT : ch=='D' ? K_LEFT  :
                       ch=='H' ? K_HOME  : ch=='K' ? K_END   : 0,
                       c);

          if (rc != FALSE)
            return rc;
        }
        break;


      /* So a "+++" at a command prompt won't cause OUR modem to go        *
       * into cmd mode                                                     */

      case '+':
        if (!(type & INPUT_NOECHO))
          Puts(sp_bs);
        /* fall-through */

      default:
        if (ch < 32)
          break;
        
        if (num_ch >= max && (!(type & INPUT_WORDWRAP) || (cur_pos != num_ch)))
        {
          if (!(type & INPUT_NOECHO))
            Putc('\a');
        }
        else
        {
          if (! Highbit_Allowed())
            ch &= 0x7f;

          strocpy(string+cur_pos+1, string+cur_pos);
          string[cur_pos]=ch;

          if (!(type & INPUT_NOECHO))
          {
            Puts(Show_Pwd(string+cur_pos,temp,
                          (char)((type & INPUT_ECHO) ? c : 0)));

            if (string[cur_pos+1] != '\0')
              Printf(left_x, strlen(string+cur_pos)-1);
          }

          num_ch++;
          cur_pos++;

          if (num_ch > max && (type & INPUT_WORDWRAP))
          {
            int idx, num_bs;

            for (idx=num_ch; idx >= max-MAX_WRAPLEN; idx--)
            {
              if (is_wd(string[idx]))
              {
                if (is_wd(ch)) /* Since WD's don't get wrapped, print here */
                {
                  if (!(type & INPUT_NOECHO))
                    Putc(ch);
                }
                else
                {
                  /* Back over word -- print 'y' backspaces, and then       *
                   * another 'y' spaces to clear it.                        */
                  
                  num_bs=num_ch-idx-1;
                  
                  if (num_bs && !(type & INPUT_NOECHO))
                    Printf((usr.video==GRAPH_TTY || (type & INPUT_NOCLEOL))
                            ? "\x19\x08%c\x19 %c"
                            : "\x19\x08%c" CLEOL,
                           num_bs, num_bs);
                }

                strocpy(string+idx+2, string+idx+1);
                string[idx+1]='\0';
                break;
              }
            }

            if (!(type & INPUT_NOECHO))
              Putc('\n');
            vbuf_flush();
            return 1;
          }
        }
        break;
    }

    vbuf_flush();
  }

  vbuf_flush();

  return *string;
}


