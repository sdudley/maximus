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
static char rcs_id[]="$Id: max_outl.c,v 1.3 2003/11/21 03:31:02 paltas Exp $";
#pragma on(unreferenced)

/*# name=Local output and AVATAR translation routines
*/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdarg.h>
#include "prog.h"
#include "mm.h"

extern int last_cc;
extern char strng[];

#ifdef MCP_VIDEO
  #define INCL_DOS
  #include "pos2.h"
  #include "mcp.h"

  #define MCP_OUT_HDR   (sizeof(USHORT))

  static byte mcp_out_buf[MCP_OUT_BUF_MAX];
  static byte * const mcp_beg=mcp_out_buf + MCP_OUT_HDR;
  static byte *mcp_cur=mcp_out_buf + MCP_OUT_HDR;
  static byte * const mcp_end=mcp_out_buf+MCP_OUT_BUF_MAX;

  extern HPIPE hpMCP;


  /* Flush the output video pipe to the server */

  void mcp_out_flush(void)
  {
    static OS2UINT rc; /* static for speed */
    static OS2UINT usSent;

    if (mcp_cur > mcp_beg)
    {
      if (hpMCP)
      {
        /* Store the message type and the task number at the beginning */

        *(USHORT *)mcp_out_buf=PMSG_VIO;

        if ((rc=DosWrite(hpMCP, mcp_out_buf, mcp_cur-mcp_out_buf, &usSent)) != 0)
          fprintf(stderr, "SYS%04d: DosWrite(VIO Pipe)\n", rc);
      }

      mcp_cur=mcp_beg;
    }
  }

  void near Lputc_Mcp(int ch);

  void Lputc(int ch)
  {
    if (mcp_video)
    {
      *mcp_cur++=(byte)ch;

      if (mcp_cur==mcp_end)
        mcp_out_flush();
    }

    Lputc_Mcp(ch);
  }

  /* So that the nested calls to Lputc don't get stuff sent over pipe twice */
  #define Lputc near Lputc_Mcp
  #define Lputs Lputs_Mcp

  static void near Lputs(char *s)
  {
    while (*s)
      Lputc_Mcp(*s++);
  }
#endif



void Lputc(int ch)
{

  static char str2[25];
  static char state=-1;
  static char newattr;

  static word s2, s3;

  static byte uch;
  static byte save_cx;
  static char rip_state=-1; /* -1=Not RIP 0=At bol 1=Got '!' 2=Got'|' 3=Got'\' at eol */

  static word x, y, z, a;

  if (state==-1)
  {

    /* Sift out RIP sequences */

    switch (rip_state)
    {
      case 0:               /* Looking for '!' */
        if (ch == '!')
        {
          rip_state=1;
          return;
        }
        if (ch != 10 && ch != 13 && ch != 12)
          rip_state=-1;
        break;

      case 1:               /* Looking for '|' */
        if (ch == '|')      /* Looks like RIP */
        {
          rip_state=2;      /* So ignore until eol */
          return;
        }

        rip_state=5;
        Lputc('!');         /* Otherwise, backtrack to ! */
        break;

      case 3:               /* Previous chr was backslash in RIP sequence */
        if (ch != 13)       /* Ignore a CR here */
          rip_state=2;      /* Reset to "in RIP sequence state */
        return;

      case 2:               /* We're in a RIP sequence */
        if (ch == '\\')     /* Newline might be escaped */
        {
          rip_state=3;
          return;
        }
        if (ch==10 || ch==13)  /* Looking for linefeed or cr */
          rip_state=0;      /* Look for RIP sequences at start of line */
        return;

      default:  /* -1 */
        if (ch == 1)       /* Might be a RIP sequence */
        {
          rip_state=4;
          return;
        }
        if (ch == 10 || ch == 13)
          rip_state=0;
        else if (ch=='!' && current_col==1)   /* Fooled by a sequence? */
        {
          rip_state=1;
          return;
        }
        break;

      case 4:               /* Got ^A not at bol, looking for a '!' */
        if (ch == '|')
        {
          rip_state=2;
          return;
        }

        rip_state=5;        /* Output ^a, but don't parse it */
        Lputc(1);
        break;

      case 5:
        rip_state=-1;
        break;
    }

    switch (ch)
    {
      case 9:
        /* So tabs will work reliably in lputs() */

        if (last_cc==-1)
        {
          fossil_getxy(&save_cx,strng);
          last_cc=(*strng)+1;
        }

        for (x=0; x < (word)(9-(last_cc % 8)); x++)
          lputc(' ');

        last_cc=-1;
        break;

      case 10:
#ifdef TTYVIDEO
        if (displaymode==VIDEO_IBM)
        {
#endif
          WinPutc(win,13);
          WinPutc(win,10);
#ifdef TTYVIDEO
        }
        else
        {
          #if defined(OS_2) || defined(UNIX)   /* Non-IBM video mode for MS-DOS adds '\r'         */
          lputc(13);    /* automatically, but OS/2 doesn't.                */
          #endif

          lputc(10);
        }
#endif
        break;

      case 12:
        if (usr.bits2 & BITS2_CLS)
        {
          vbuf_flush();

          curattr=DEFAULT_ATTR;

#ifdef TTYVIDEO
          if (displaymode==VIDEO_IBM)
          {
#endif
            WinCls(win,CGRAY);
            WinSetAttr(win, curattr);
#ifdef TTYVIDEO
          }
          else lputs(ansi_cls);
#endif

          /* If user has graphics turned off, then make local display GRAY! */
          curattr=CGRAY;
          
          if (! usr.video)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinSetAttr(win, curattr);
#ifdef TTYVIDEO
            else lputs(ansi_gray);
#endif
          }

          vbuf_flush();
          curattr=3;
        }
        else
        {
          lputc(13);
          lputc(10);
        }
        rip_state=0;
        break;

      case 13:
        lputc(13);
        break;

      case 22:
        state=0;
        break;

      case 25:
        state=25;
        break;

      case 7:   /* Strip out any beeps intended for the user */
        if (caller_online || local)
          break;
        else Local_Beep(1);
        break;

      default:
        lputc((usr.bits2 & BITS2_IBMCHARS) ? ch : nohibit[(byte)ch]);
        break;
    }
  }
  else switch (state)
  {
    case 0:
      switch (ch)
      {
        case 1:                   /* Attribute, get another character */
          state=1;
          break;

        case 2:
          if (usr.video)
          {
            curattr |= _BLINK;
            
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinSetAttr(win, curattr);   /* Add high bit */
#ifdef TTYVIDEO
            else lputs(ansi_blink);
#endif
          }
          state=-1;
          break;

        case 3:
          if (usr.video || in_file_xfer)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinGotoXY(win, WinGetRow(win)-1, WinGetCol(win), FALSE);
#ifdef TTYVIDEO
            else lputs(ansi_up);
#endif
          }
          state=-1;
          break;

        case 4:
          if (usr.video)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinGotoXY(win, WinGetRow(win)+1, WinGetCol(win), FALSE);
#ifdef TTYVIDEO
            else lputs(ansi_down);
#endif
          }
          state=-1;
          break;

        case 5:
          if (usr.video)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinGotoXY(win, WinGetRow(win), WinGetCol(win)-1, FALSE);
#ifdef TTYVIDEO
            else lputs(ansi_left);
#endif
          }
          state=-1;
          break;

        case 6:
          if (usr.video)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinGotoXY(win, WinGetRow(win), WinGetCol(win)+1, FALSE);
#ifdef TTYVIDEO
            else lputs(ansi_right);
#endif
          }
          else lputc(' ');
          state=-1;
          break;

        case 7:
          if (usr.video)
          {
#ifdef TTYVIDEO
            if (displaymode==VIDEO_IBM)
#endif
              WinCleol(win, WinGetRow(win), WinGetCol(win), WinGetAttr(win));
#ifdef TTYVIDEO
            else lputs(ansi_cleol);
#endif
          }

          state=-1;
          break;

        case 8:                   /* Goto, get another character */
          state=8;
          break;

        case 9:                   /* Insert mode on -- not supported */
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
            int wasline=WinGetRow(win)+1;
            int wascol=WinGetCol(win)+1;

            Lputc('\x07');    /* Clear rest of current line */
            /* Now clear the rest of the lines */
            strcpy(str2, "\n" CLEOL);
            save_cx=x=3;
            state=31;         /* Shortcut to repeat sequence state */
            Lputc(TermLength()-wasline);

            state=10;         /* 'goto' state */
            save_cx=(byte)wasline;  /* save_cx? Hmm :-) */
            Lputc(wascol);    /* Restore position */
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
        if (usr.video)
        {
#ifdef TTYVIDEO
          if (displaymode==VIDEO_IBM)
#endif
            WinSetAttr(win, (byte)ch);
#ifdef TTYVIDEO
          else lputs(avt2ansi(ch, curattr, strng));
#endif

          curattr=(char)ch;
        }

        state=-1;
      }
      else state=2;
      break;

    case 2:                         /* Attribute DLE */
      if (usr.video)
      {
        newattr=(char)(ch & 0x7f);

#ifdef TTYVIDEO
        if (displaymode==VIDEO_IBM)
#endif
          WinSetAttr(win, newattr);
#ifdef TTYVIDEO
        else lputs(avt2ansi(newattr, curattr, strng));
#endif

        curattr=newattr;
      }

      state=-1;
      break;

    case 8:                         /* Goto1 */
      save_cx=(unsigned char)ch;
      state=10;
      break;

    case 10:                        /* Goto2 */
      state=-1;

      if (usr.video || in_file_xfer)
      {
#ifdef TTYVIDEO
        if (displaymode==VIDEO_IBM)
#endif
          WinGotoXY(win, save_cx-1, ch-1, FALSE);
#ifdef TTYVIDEO
        else Lprintf(ch==1 ? ansi_goto1 : ansi_goto, save_cx, ch);
#endif
      }
      else if (ch==1)
        lputs("\r\n");
      break;

    case 12:                        /* Clear 1 */
      save_cx=(unsigned char)ch;
      state=13;
      break;

    case 13:                        /* Clear 2 */
      s2=ch+1;
      state=14;
      break;

    case 14:                        /* Clear 3 */
      y=current_line;
      z=current_col;
      a=save_cx;
      state=-1;

      if (usr.video)
      {
        Lprintf(attr_string,a);

        for (x=0; x < s2; x++)
          Lprintf(clear_string, y + x, z, (char)ch+1);

        Lprintf(goto_str, y, z);
      }
      break;

    case 15:                        /* Fill 1 */
      save_cx=(unsigned char)ch;
      state=16;
      break;

    case 16:                        /* Fill 2 */
      s2=ch;
      state=17;
      break;

    case 17:                        /* Fill 3 */
      s3=ch+1;
      state=18;
      break;

    case 18:                        /* Fill 4 */
      y=current_line;
      z=current_col;
      a=save_cx;
      state=-1;

      if (usr.video)
      {
        Lprintf(attr_string, a);

        for (x=0; x < s3; x++)
          Lprintf(fill_string, y+x, z, s2, ch+1);

        Lprintf(goto_str, y, z);
      }
      break;

    case 25:                      /* RLE1 */
      save_cx=(byte)ch;
      state=27;
      break;

    case 27:                      /* RLE2 */
      {
        word x;
        byte c;

        c=(usr.bits2 & BITS2_IBMCHARS) ? (byte)save_cx:nohibit[(byte)save_cx];

        uch=(byte)ch;

        for (x=0; x < uch; x++)
          lputc(c);

        state=-1;
      }
      break;

    case 30:
      save_cx=(unsigned char)ch;
      x=0;
      state=31;
      break;

    case 31:
      if (x < 24 && x < save_cx)
        str2[x++]=(char)ch;
      else
      {
        word y;

        str2[x]='\0';
        state=-1;

        uch=(byte)ch;

        for (y=0; y < uch; y++)
          Lputs(str2);
      }
      break;

    default:
      state=-1;
      break;
  }
}

