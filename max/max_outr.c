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
static char rcs_id[]="$Id: max_outr.c,v 1.3 2003/06/06 01:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Modem output and AVATAR translation routines
*/

#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "prog.h"
#include "mm.h"


#if !defined(OS_2) && !defined(NT)
static int timer2=FALSE;
static int set=TRUE;
#endif

extern int last_cc;
extern char strng[];

static int rip_wrap=1;

#if defined(OS_2) || defined(NT) || defined(UNIX)

    static void near CMDM_PPUTcw(int c)
    {
        if(local)
            return;

        ComPutc(hcModem, c);
    }

static void near CMDM_PPUTs(char *s)
{
  if (local)
    return;
  else
  {
#if (COMMAPI_VER > 1)
    extern HCOMM hcModem;
    BOOL lastState = ComBurstMode(hcModem, TRUE);

    ComWrite(hcModem, s, strlen(s));
    ComBurstMode(hcModem, lastState);
#else
  while (*s)
    ComPutc(hcModem, *s++);
#endif
  }
}
    #define CMDM_PPUTc(c) CMDM_PPUTcw(c)


#else /* !OS_2 */

    static void near CMDM_wait(void)
    {
      char y;

      fFlow=TRUE;
      
      /* Only reinit the input timout if we haven't sent a character yet */

      if (set)
      {
        timer2=FALSE;
        input_timeout=timerset(timeout_tics);
        set=FALSE;
      }

      /* Turn off the msgs when timeout occurs, so they don't get stuck by  *
       * ^s or hardware handshaking.                                        */

      y=shut_up;
      shut_up=TRUE;

      Check_Time_Limit(&input_timeout,&timer2);
      Mdm_check();

      shut_up=y;

      if (baud < 38400L)
        Delay((unsigned int)((38400L-baud)/1920L));
      
      fFlow=FALSE;
    }

    static void near CMDM_PPUTcw(int c)
    {
      if (local)
        return;

      set=TRUE;

      while (out_full())
        CMDM_wait();

      while (mdm_pputc(c) != 0x01)
        CMDM_wait();
      
#ifdef DEBUG_OUT
      if (dout_log)
        DebOutSentChar(c);
#endif
    }

    static void near CMDM_PPUTs(char *s)
    {
      if (local)
        return;

      while (*s)
        CMDM_PPUTcw(*s++);
    }

#endif  /* !OS_2 */



static char b36digits[]= { "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" };

static int near b36args(char *buf, int *pia, ...)
{
  int args, i, v;
  va_list argp;


  va_start(argp,pia);
  args=0;
  while (*buf && (i=va_arg(argp,int), i))
  {
    v=0;
    while (*buf && i--)
    {
      char *p=strchr(b36digits,*buf);
      if (p!=0)
        v += (v * 36) + (p-b36digits);
      ++buf;
    }
    pia[args++] = v;
  }
  va_end(argp);
  return args;
}


void RipReset(void)
{
  SetTermSize(0,0);
  SetRipOrigin(0,0);
  rip_wrap=TRUE;
  SetRipFont(0);
  display_line=display_col=current_line=current_col=1;
  mdm_attr=-1;
}


void Mdm_putc(int ch)
{
  static char str2[25];
  static char str3[25];
  static int args[6];
  static char state=-1;
  static char rip_state=-1;
            
  static word s2, s3;

  static byte save_cx, uch, b36;
  static byte newattr;

  static word x, y, z, a;

  if (state==-1)
  {

    /* Look for RIP sequences */
    /* Simple RIP state machine to adjust for line counting */

    if (!hasRIP())
      rip_state=-1;
    else
    {

      switch (rip_state)
      {

        case 3:             /* Last chr was CR, if not LF then reset */
          if (ch==10)
          {
            rip_state=0;
            mdm_attr=-1;    /*SJD 95.05.03 - workaround for RIPterm bug */
          }
          else if (ch=='!') /* Might be a RIP */
            rip_state=1;
          else rip_state=-1;
          break;

        case 1:
          if (ch=='|')      /* Definitely RIP */
          {
            rip_state=5;
            break;
          }

        default:
          if (ch==1)        /* Might be a RIP sequence */
            rip_state=1;    /* Look for it */
          else if (ch==10 || ch==13)
            rip_state=0;
          break;

        case 4:
          mdm_attr=-1;    /*SJD 95.05.03 - workaround for RIPterm bug */
          rip_state=0;
          /* Fallthru */

        case 0:             /* Looking for '!' */
          if (ch=='!')
            rip_state=1;    /* Smells like... */
          else if (ch != 10 && ch != 13 && ch != 12)
            rip_state=-1;
          break;

        case 6:             /* Collecting data for textwindow */
          if (b36 < 10 && isalnum(ch))
          {
            str3[b36++]=ch;
            break;
          }
          str3[b36]='\0';
          if (b36args(str3,args, 2, 2, 2, 2, 1, 1, 0)==6)
          {
            if (RipOriginX()!=args[0] || RipOriginY()!=args[1] ||
                TermWidth()!=args[2]  || TermLength()!=args[3])
              display_line=display_col=current_line=current_col=1;
            SetRipOrigin(args[0], args[1]);
            SetTermSize(args[2]-args[0], args[3]-args[1]);
            rip_wrap=args[4]?TRUE:FALSE;
            SetRipFont(args[5]);
          }
          rip_state=2;
          /* Fallthru */

        case 2:             /* In RIP sequence, look for linefeed */
          if (ch==10)
            rip_state=4;
          else if (ch == 13)
            rip_state=3;
          else if (ch == '|') /* Starts another RIP command */
            rip_state=5;
          break;

        case 5:             /* Collect RIP command */
          rip_state=2;
          switch (ch)
          {
            case 'w':
              rip_state=6;
              b36=0;
              break;

            case '*':
              RipReset();
              /* Fallthru */

            case 'e':           /* Need to reset attribute on erasetextwindow */
              mdm_attr=-1;
              rip_state=2;
              /* Fallthru */

            case 'H':
              display_line=display_col=current_line=current_col=1;
              break;
          }
          break;

      }
    }

    switch (ch)
    {
      case 9:
        last_cc=current_col;

        if (usr.bits & BITS_TABS)
        {
          x=9-(last_cc % 8);
          CMDM_PPUTcw('\x09');
        }
        else
        {
          for (x=0; x < (word)(9-(last_cc % 8)); x++)
            CMDM_PPUTcw(' ');
        }

        if ((current_col += (char)x) > TermWidth())
        {
          wrap=TRUE;
          current_line++;
          display_line++;

          if (current_line > TermLength())
            current_line=TermLength();       /* Scroll! */

          current_col=1;
          display_col=1;
        }
        break;

      case 10:                        /* Handle '\n' */
        if (rip_state != 4)           /* Don't adjust line if end of RIP */
        {
          current_line++;
          display_line++;
        }

        current_col=display_col=1;
        wrap=FALSE;

        if (current_line > TermLength())
          current_line=TermLength();       /* Scroll! */

        CMDM_PPUTs("\r\n");

        for (x=0; x < usr.nulls; x++)
          CMDM_PPUTcw('\x00');

        /* Only check for the important one (time limit expired) here.
           We check for 2 and 5 min. left on all input calls */

        if (timeleft() <= 0 && do_timecheck)
          TimeLimit();

        if (!local && !carrier())
          Lost_Carrier();

        if (loc_kbhit() && !keyboard && !local)
          Parse_Local(loc_getch());
        break;

      case 12:
        if (usr.bits2 & BITS2_CLS)
        {
          if (usr.video)
            display_line=display_col=current_line=current_col=1;

          if (usr.video==GRAPH_ANSI)
            CMDM_PPUTs(ansi_cls);
          else CMDM_PPUTcw('\x0c');

          if (hasRIP())   /* RIPTERM seems to need this to know that */
          {               /* Its at the start of a new line. Duh! */
            CMDM_PPUTs("\r");
            rip_state=0;
          }

          mdm_attr=DEFAULT_ATTR;
        }
        else CMDM_PPUTs("\r\n");
        break;

      case 13:
        wrap=FALSE;
        CMDM_PPUTcw('\r');
        current_col=display_col=1;
        break;

      case 25:
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw('\x19');

        state=25;
        break;

      case '\x08':
        display_col--;
        current_col--;
        goto OutP;

      case 22:
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw('\x16');

        state=0;
        break;
        
      default:
        if (rip_state==-1 || rip_state < 2)
        {
          display_col++;

          if (++current_col > TermWidth())
          {
            if (hasRIP() && !rip_wrap)
            {
              display_col--;
              current_col--;
            }
            else
            {
              wrap=TRUE;
              current_line++;
              display_line++;

              if (current_line > TermLength())
                current_line=TermLength();       /* Scroll! */

              current_col=1;
              display_col=1;
            }
          }
        }

      case 7:

      OutP:
        CMDM_PPUTcw( (int) ((usr.bits2 & BITS2_IBMCHARS) ? ch
                                        : nohibit[(unsigned char)ch]));
        break;
    }
  }
  else
  {

    rip_state=-1;

    switch (state)
    {
      case 0:
        if (usr.video==GRAPH_AVATAR && ch != 15)
          CMDM_PPUTcw(ch);

        switch (ch)
        {
          case 1:                   /* Attribute, get another character */
            state=1;
            break;

          case 2:
            mdm_attr |= 0x80;

            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_blink);

            state=-1;
            break;

          case 3:
            current_line--;
            display_line--;

            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_up);

            state=-1;
            break;

          case 4:
            current_line++;
            display_line++;

            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_down);

            state=-1;
            break;

          case 5:
            current_col--;
            display_col--;

            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_left);

            state=-1;
            break;

          case 6:
            current_col++;
            display_col++;

            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_right);
            else if (! usr.video)
              CMDM_PPUTcw(' ');

            state=-1;
            break;

          case 7:
            if (usr.video==GRAPH_ANSI)
              CMDM_PPUTs(ansi_cleol);

            state=-1;
            break;

          case 8:                   /* Goto, get another character */
            state=8;
            break;

          case 9:                   /* Insert mode on */
          case 10:                  /* Scroll area up */
          case 11:                  /* Scroll area down */
            break;

          case 12:                  /* Clear area */
            state=12;
            break;

          case 13:                  /* Fill area */
            state=15;
            break;

          case 14:                  /* Delete character at cursor */
            break;

          case 15:                  /* Clear to end of screen */
            if (usr.video)
            {
              int wasline=current_line;
              int wascol=current_col;

              byte seq[10];

              /* We have to do this the long way for AVATAR callers */

              Mdm_putc('\x07');

              if (wasline != TermLength())
              {
                sprintf(seq, "\x16\x19\x03\n" CLEOL "%c", TermLength()-wasline);
                Mdm_puts(seq);
                sprintf(seq, "\x16\x08%c%c", (byte)wasline, (byte)wascol);
                Mdm_puts(seq);
              }
            }
            state=-1;
            break;

          case 25:                  /* Repeat pattern */
            state=30;
            break;

          default:
            state=-1;
        }
        break;

      case 1:
        if (ch != DLE)              /* Attribute */
        {
          if (usr.video==GRAPH_AVATAR)
            CMDM_PPUTcw(ch & 0x7f);

          if (usr.video==GRAPH_ANSI)
            CMDM_PPUTs(avt2ansi(ch, mdm_attr, strng));

          mdm_attr=(char)ch;

          /* Translate the non-standard practice of putting the 'blink' bit   *
           * into the high attribute to something that normal avatar systems  *
           * can handle.  (This is correctly handled by the StdNumToAnsi      *
           * routine for ansi callers.)                                       */
           
          if (ch & 0x80u && usr.video==GRAPH_AVATAR)
            CMDM_PPUTs("\x16\x02");

          state=-1;
        }
        else state=2;
        break;

      case 2:                       /* Attribute DLE */
        newattr=(char)(ch & 0x7f);

        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(newattr);
        else if (usr.video==GRAPH_ANSI)
          CMDM_PPUTs(avt2ansi(newattr, mdm_attr, strng));

        mdm_attr=newattr;
        state=-1;
        break;

      case 8:                       /* Goto1 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        save_cx=(unsigned char)ch;
        state=10;
        break;

      case 10:                       /* Goto2 */
        display_line=current_line=save_cx;
        display_col=current_col=(char)ch;

        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);
        else if (usr.video==GRAPH_ANSI)
        {
          sprintf(strng,ch==1 ? ansi_goto1 : ansi_goto,save_cx,ch);
          CMDM_PPUTs(strng);
        }
        else if (ch==1)
          CMDM_PPUTs("\r\n");

        state=-1;
        break;

      case 12:                        /* Clear 1 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        save_cx=(unsigned char)ch;
        state=13;
        break;

      case 13:                        /* Clear 2 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        s2=ch+1;
        state=14;
        break;

      case 14:                        /* Clear 3 */
        if (usr.video==GRAPH_AVATAR)
        {
          CMDM_PPUTcw(ch);
          state=-1;
        }
        else if (usr.video)
        {
          y=current_line;
          z=current_col;
          a=save_cx;
          state=-1;

          Mdm_printf(attr_string, a);

          for (x=0;x < s2;x++)
            Mdm_printf(clear_string, y+x, z, (char)ch+1);

          Mdm_printf(goto_str, y, z);
        }
        break;

      case 15:                        /* Fill 1 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        save_cx=(unsigned char)ch;
        state=16;
        break;

      case 16:                        /* Fill 2 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        s2=ch;
        state=17;
        break;

      case 17:                        /* Fill 3 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        s3=ch+1;
        state=18;
        break;

      case 18:                        /* Fill 4 */
        if (usr.video==GRAPH_AVATAR)
        {
          CMDM_PPUTcw(ch);
          state=-1;
        }
        else if (usr.video)
        {
          y=current_line;
          z=current_col;
          a=save_cx;
          state=-1;

          Mdm_printf(attr_string, a);

          for (x=0; x < s3; x++)
            Mdm_printf(fill_string, y+x, z, s2, ch+1);

          Mdm_printf(goto_str, y, z);
        }
        break;


      case 25:                      /* RLE1 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        save_cx=(unsigned char)ch;
        state=27;
        break;

      case 27:                      /* RLE2 */
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);
        else
        {
          word x;
          byte c;

          c=(usr.bits2 & BITS2_IBMCHARS) ? (byte)save_cx
                       : nohibit[(byte)save_cx];

          uch=(unsigned char)ch;

          for (x=0; x < uch; x++)
            CMDM_PPUTcw(c);

        }
      
        /* For backspaces, current col counter should go backwards! */

        if ((byte)save_cx==8)
          ch=-ch;

        display_col += (char)ch;
        current_col += (char)ch;
        state=-1;
        break;

      case 30:
        if (usr.video==GRAPH_AVATAR)
          CMDM_PPUTcw(ch);

        save_cx=(unsigned char)ch;
        x=0;
        state=31;
        break;

      case 31:
        if (x < 24 && x < save_cx)
        {
          if (usr.video==GRAPH_AVATAR)
            CMDM_PPUTcw(ch);

          str2[x++]=(char)ch;
        }
        else
        {
          state=-1;

          if (usr.video==GRAPH_AVATAR)
            CMDM_PPUTcw(ch);
          else
          {
            word y;

            str2[x]='\0';

            uch=(unsigned char)ch;

            for (y=0; y < uch; y++)
              Mdm_puts(str2);
          }
        }
        break;

      default:
        state=-1;
        break;
    }
  }
}


/* Reset the modem's colour to what it should be */

void ResetAttr(void)
{
  byte last_attr=mdm_attr;
  
  mdm_attr=-1;
  
  if (last_attr != (byte)-1)
    Printf(attr_string, last_attr);
}

 
