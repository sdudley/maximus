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
static char rcs_id[]="$Id: maxed.c,v 1.3 2004/01/27 21:00:45 paltas Exp $";
#pragma on(unreferenced)

/*# name=Full-screen editor
*/

#define INIT_MAXED
#define MAX_INCL_COMMS

#include "maxed.h"
#include "keys.h"
#include "m_reply.h"

static word near Process_Scan_Code(struct _replyp *pr);
static word near Process_Cursor_Key(void);
static void near Process_Control_Q(void);
static word near Process_Control_K(struct _replyp *pr);
static void near Init_Vars(void);


#if 1
/* Get a character and parse local ^c's as real chars */

int Mdm_getcwcc(void)
{
  int timer2;

  /* Convert any local ^Cs or ^breaks to ASCII 3, which is what we *
   * want.                                                         */

  timer2=FALSE;
  input_timeout=timerset(timeout_tics);

  while (! Mdm_keyp() && !brk_trapped)
  {
    Check_Time_Limit(&input_timeout, &timer2);
    Check_For_Message(NULL, NULL);
    Giveaway_Slice();
  }

  if (brk_trapped)
  {
    brk_trapped=0;
    return '\x03';
  }

  return (Mdm_getcw());
}
#else

int Mdm_getcwcc(void)
{
  while (Mdm_kpeek_tic(timeout_tics)==-1 && !brk_trapped)
    ;  /* Check_Time_Limit/Check_For_Message/Giveaway_Slice already done */
  if (brk_trapped)
  {
    brk_trapped=0;
    return '\x03';
  }
  return Mdm_getcw();
}
#endif

int MagnEt(XMSG *msg,HMSG msgh,struct _replyp *pr)
{
  long now_cl;

  sword ret;
  sword ch;
  sword state;
  word redo_status;

  char break_loop;

  quoting=FALSE;
  qmh=NULL;
  mmsg=msg;
  state=0;

  if ((update_table=malloc(UPDATEBUF_LEN))==NULL)
    return ABORT;

  if ((quotebuf=malloc(QUOTELINES*MAX_LINELEN))==NULL)
  {
    free(update_table);
    return ABORT;
  }

  if ((quote_pos=malloc(MAX_QUOTEBUF*sizeof(long)))==NULL)
  {
    free(update_table);
    free(quotebuf);
    return ABORT;
  }

  now_cl=coreleft();

  max_lines = (now_cl-MAXED_SAVE_MEM) / (long)MAX_LINELEN;

  if ((sword)max_lines < 0 || (sword)max_lines > MAX_LINES)
    max_lines = MAX_LINES;

  if (max_lines==0)   /* Something's really wrong here! */
  {
    logit(mem_nmsgb);
    Puts(mem_nmsgb);

    Press_ENTER();
  }
  else                /* Things look A-OK! */
  {
    /* Allow high-bit chars while in the editor */

    in_msghibit++;

    /* Allocate the last line, for our spiffy '-end-' widget. */

    if (Allocate_Line(max_lines))
      EdMemOvfl();

    /* Turn off modem flow controls, so we can interpret characters such  *
     * as control-C, control-S, etc.                                      */

    (void)mdm_ctrlc(0);
    Mdm_Flow_Off();

    update_table[max_lines]=TRUE;

    Init_Vars();

    Puts(maxed_init);

    if (!(usr.bits2 & BITS2_CLS))
      NoFF_CLS();

    if (Allocate_Line(offset+1))
      EdMemOvfl();

    break_loop=FALSE;

    if (msgh)
    {
      Load_Message(msgh);
      Redraw_Text();
      Do_Update();
    }

    redo_status=FALSE;

    /* If the user has a novice help level, print help msg on status line */

    if (usr.help==NOVICE)
    {
      redo_status=TRUE;

      Goto(usrlen, 1);
      Puts(ck_for_help);
      Puts(CLEOL);
      Printf(msg_text_col);
      Goto(cursor_x, cursor_y);
    }
    else
    {
      Redraw_StatusLine();
    }

    if (setjmp(jumpto)==0) /* Really ugly, but the best way to handle errs */
    {
      for (;;)
      {
        /* This loop is to update lines on the screen.  If the user wants  *
         * to do a few page-down's in a row or something, we don't want to *
         * do a FULL page-down if there is another character waiting,      *
         * which usually means the user wants something NOW.               */

        while (!break_loop)
        {
          screen[max_lines][0]=SOFT_CR;
          screen[max_lines][1]='\0';

          /* So we'll display the -end- whenever we get a chance.  Since   *
           * we don't want the '-end' continiously displaying, we only set *
           * the update table to restore the line if it isn't on-screen,   *
           * so it DOES get displayed once we DO come back on-screen.      *
           * (Confused yet?)                                               */

          if (! (offset+cursor_x >= max_lines-usrlen))
            update_table[max_lines]=TRUE;

          /* Stick the cursor in the right spot */
          if (!Mdm_keyp() && pos_to_be_updated)
            Update_Position();

          vbuf_flush();

          Do_Update();

          ch=Mdm_getcwcc();
          
          /* Redraw the status line for the user's help msg */

          if (redo_status)
          {
            Redraw_StatusLine();
            redo_status=FALSE;
          }

          switch (ch)
          {
            case 0:    /* Scan key! */
              switch (Process_Scan_Code(pr))
              {
                case SAVE:
                  ret=SAVE;
                  goto BackToCaller;

                case ABORT:
                  break_loop=TRUE;
                  break;
              }
              break;

            case K_CTRLA:
              Word_Left();
              break;

            case K_CTRLC:
              Page_Down();
              break;

            case K_CTRLD:
              Cursor_Right();
              break;

            case K_CTRLE:
              Cursor_Up();
              break;

            case K_CTRLF:
              Word_Right();
              break;

            case K_CTRLG:    /* Delete character */
              Delete_Char();
              break;

            case K_VTDEL:    /* VT 100 delete */

              /* Delete char under cusor if user has ibmchars turned on */

              if (usr.bits2 & BITS2_IBMCHARS)
              {
                Delete_Char();
                break;
              }
              /* else fall-through */

            case K_BS:    /* Backspace */
              BackSpace();
              break;

            case K_TAB:    /* Tab */
            {
              word chrs;
              
              skip_update=TRUE;

              for (chrs=8 - ((cursor_y-1) % 8); chrs--; )
              {
                if (insert)
                  Add_Character(' ');
                else Cursor_Right();
              }

              skip_update=FALSE;
              break;
            }

            case K_CTRLK:
              switch (Process_Control_K(pr))
              {
                case SAVE:
                  ret=SAVE;
                  goto BackToCaller;

                case ABORT:
                  break_loop=TRUE;
                  break;
              }
              break;

            case K_CTRLL:
              update_table[offset+cursor_x]=TRUE;
              break;

            case K_CR:    /* Return */
              if (Carriage_Return(TRUE))
              {
                ret=SAVE;
                goto BackToCaller;
              }
              break;

            case K_CTRLN:
            case K_CTRLO:
              MagnEt_Help();
              break;

            case K_CTRLQ:
              Process_Control_Q();
              break;

            case K_CTRLR:
              Page_Up();
              break;

            case K_CTRLS:
              Cursor_Left();
              break;

            case K_CTRLT:
              Delete_Word(); 
              break;

            case K_CTRLV:
              Toggle_Insert();
              break;

            case K_CTRLW:
              Redraw_Text();
              Redraw_StatusLine();
              Redraw_Quote();
              break;

            case K_CTRLX:
              Cursor_Down(TRUE);
              break;

            case K_CTRLY:
              /* If we're on the last line, simply blank it out */

              if (offset+cursor_x >= num_lines)
              {
                screen[offset+cursor_x][0]='\r';
                screen[offset+cursor_x][1]='\0';
                update_table[offset+cursor_x]=TRUE;
              }
              else
              {
                /* Otherwise, delete it and move up a line */

                Delete_Line(cursor_x);
              }

              cursor_y=1;

              Goto(cursor_x,cursor_y);
              break;

            case K_CTRLZ:
              ret=SAVE;
              goto BackToCaller;

            case K_ESC:
              if (Process_Cursor_Key()==ABORT)
                break_loop=TRUE;
              break;

            case K_CTRLB:
            case K_CTRLU:
            case '\x1c':
            case '\x1e':
            case '\x1f':
              MagnEt_Bad_Keystroke();

            case K_CTRLJ:  /* Linefeed, just ignore */
              break;

            case '+':
              Puts(sp_bs);
              /* fall-through */

            default:        /* Normal character */
              {
                if (state==0 && (ch==':' || ch==';' || ch=='B' || ch=='8'))
                  state=1;
                else if (state==1 && ch=='-')
                  state=2;
                else if (state==2 && (ch=='(' || ch=='{' || ch=='['))
                {
                  Goto(usrlen,10);

                  /* Tell this deadbeat to lighten up! */
                  Puts(happy);
                  Printf(msg_text_col);


                  Goto(cursor_x,cursor_y);
                  state=0;
                }
                else state=0;

                if (ch > 31 && (ch < 127 || ((mah.ma.attribs & MA_HIBIT))))
                  Add_Character(ch);
              }

              break;
          }
        }

        Puts("\n" CLEOL);

        WhiteN();

        /* Make sure s/he really means it */
        if (GetyNAnswer(isachange ? abortchange : abortmsg, 0)==YES)
          break;  /* Get out of loop */
        else
        {
          (void)mdm_ctrlc(0);
          Mdm_Flow_Off();

          Redraw_Text();
          Redraw_StatusLine();
          Redraw_Quote();

          break_loop=FALSE; /* Keep on truckin' */
        }
      }   /* while (1) */
    }     /* if (setjmp()==0) */
  }       /* max_lines != 0 */


 /* Free everything, then return an ABORT code, since we only get this far *
  * if something craps out.                                                */

  Free_All();

  if (usr.bits2 & BITS2_CLS)
    Puts(CLS);
  else NoFF_CLS();

  ret=ABORT;

BackToCaller:

  in_msghibit--;

  Mdm_Flow_On();

  if (quoting)
    MsgCloseMsg(qmh);

  free(update_table);
  free(quotebuf);
  free(quote_pos);

  return ret;
}


static word near Process_Scan_Code(struct _replyp *pr)
{
  if (loc_peek()=='\x24')    /* Alt-J */
  {
    loc_getch();
    Shell_To_Dos();
    Fix_MagnEt();
  }
  else switch(Mdm_getcwcc())
  {
    case 59:
      MagnEt_Help();
      break;

    case 16:      /* Quote */
      Quote_OnOff(pr);
      break;

    case 31:      /* Exit/Save */
      return SAVE;

    case 45:      /* Exit/NoSave */
      return ABORT;

    case 46:      /* Quote copy */
      if (quoting)
        Quote_Copy();
      break;

     case 68:
      MagnEt_Menu();
      break;

    case 71:
      Cursor_BeginLine();
      break;

    case 72:
      Cursor_Up();
      break;

    case 73:
      Page_Up();
      break;

    case 75:
      Cursor_Left();
      break;

    case 77:
      Cursor_Right();
      break;

    case 79:
      Cursor_EndLine();
      break;

    case 80:
      Cursor_Down(TRUE);
      break;

    case 81:
      Page_Down();
      break;

    case 82:
      Toggle_Insert();
      break;

    case 83:
      Delete_Char();
      break;

    case 115:
      Word_Left();
      break;

    case 116:
      Word_Right();
      break;

    default:
      MagnEt_Bad_Keystroke();
      break;
  }

  return NOTHING;
}



static word near Process_Cursor_Key(void)
{
  switch(Mdm_getcwcc())
  {
    case '\x1b':      /* Abort message */
      return ABORT;

    case '[':         /* Start of ANSI sequence */
    case 'O':
      switch (Mdm_getcwcc())
      {
        case 'A':     /* Cursor Up */
          Cursor_Up();
          break;

        case 'B':     /* Cursor Down */
          Cursor_Down(TRUE);
          break;

        case 'C':     /* Cursor Right */
          Cursor_Right();
          break;

        case 'D':     /* Cursor Left */
          Cursor_Left();
          break;
#ifdef UNIX
	case '1':
	  Mdm_getcwcc();
#else
        case 'H':     /* Home */
#endif	
          Cursor_BeginLine();
          break;
#ifdef UNIX
	case '4':
	  Mdm_getcwcc();
#else	            
        case 'K':     /* End */
#endif	
          Cursor_EndLine();
          break;

#ifdef UNIX
	case '5':
	  Mdm_getcwcc();
	  Page_Up();
	  break;
	case '6':
	  Mdm_getcwcc();
	  Page_Down();
	  break;
#endif    

        default:
          MagnEt_Bad_Keystroke();
          break;

      }
      break;

    default:
      MagnEt_Bad_Keystroke();
      break;
  }

  return NOTHING;
}


static void near Process_Control_Q(void)
{
  int ch;

  Goto(usrlen,usrwidth-3);
  Puts(YELONBLUE "^Q");
  Goto(cursor_x,cursor_y);
  Printf(msg_text_col);
  vbuf_flush();

  ch=Mdm_getcwcc();

  switch(toupper(ch))
  {
    case 0:         /* Scan code... */
      Mdm_getcwcc();  /* Throw it away */
      break;

    case 19:
    case 'S':       /* Beginning of line */
      Cursor_BeginLine();
      break;

    case 4:
    case 'D':       /* End of line */
      Cursor_EndLine();
      break;

    case 25:
    case 'Y':       /* End of line */
      Puts(CLEOL);
      screen[offset+cursor_x][cursor_y]='\0';
      break;

    default:
      MagnEt_Bad_Keystroke();
      break;
  }

  Goto(usrlen,usrwidth-3);
  Puts(YELONBLUE "  ");
  Goto(cursor_x,cursor_y);
  Printf(msg_text_col);
  vbuf_flush();
}


static word near Process_Control_K(struct _replyp *pr)
{
  int ret,
      ch;

  Goto(usrlen,usrwidth-3);
  Puts(YELONBLUE "^K");
  Goto(cursor_x,cursor_y);
  Printf(msg_text_col);
  vbuf_flush();

  ret=NOTHING;

  ch=Mdm_getcwcc();

  switch(toupper(ch))
  {
    case 0:         /* Scan code... */
      Mdm_getcw();  /* Throw it away */
      break;

    case 4:
    case 19:
    case 'D':     /* Exit/Save */
    case 'S':
      ret=SAVE;
      break;

    case '?':
      MagnEt_Help();
      break;

    case 8:
    case 'H':
      MagnEt_Menu();
      break;

#ifdef NEVER
    case 16:
    case 'P':
      Piggy();
      break;
#endif

    case 18:
    case 'R':
      Quote_OnOff(pr);
      break;

    case 3:
    case 'C':
      if (quoting)
        Quote_Copy();
      break;


    case 17:
    case 'Q':     /* Exit/NoSave */
      ret=ABORT;
      break;

    default:
      MagnEt_Bad_Keystroke();
  }

  Goto(usrlen,usrwidth-3);
  Puts(YELONBLUE "  ");
  Goto(cursor_x,cursor_y);
  Printf(msg_text_col);
  vbuf_flush();

  return ret;
}






static void near Init_Vars(void)
{
  word line;

  offset=0;
  num_lines=offset;
  cursor_x=cursor_y=1;
  quoting=pos_to_be_updated=skip_update=FALSE;
  insert=update_table[max_lines]=TRUE;

  usrwidth=min((byte)LINELEN, TermWidth());

  /* Make sure that the user's screen is never larger than the Sysop's,    *
   * to avoid problems.                                                    */

  usrlen=(byte)(TermLength()-((prm.flags & FLAG_statusline) ? 1 : 0));
  usrlen=min(TermLength(), usrlen);

  for (line=0; line < max_lines; line++)
    update_table[line]=FALSE;
}


void EdMemOvfl(void)
{
  logit(mem_nmsgb);
  Puts(mem_nmsgb+1);

  Press_ENTER();

  longjmp(jumpto,-1);       /* I know, I know, I'm ashamed about it too... */
                            /* In here, we can return anything but zero.   */
}


