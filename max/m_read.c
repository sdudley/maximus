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
static char rcs_id[]="$Id: m_read.c,v 1.1.1.1 2002/10/01 17:52:48 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Message reading and display functions
*/

#define MAX_INCL_COMMS
#define MAX_LANG_m_browse

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"
#include "max_msg.h"
#include "m_readp.h"
#include "m_full.h"
#include "node.h"
#include "trackm.h"

static void near Clear_To_Screen_End();
static int near Msg_Read_RepOrig(int orig);
static int near Msg_Get_Msgs(int dir,dword startmsg,int nonstop,int exact,int show_err);
static int near MoreYnns_Read(int inbrowse);
static void near Display_Addr(char *orig_or_dest,char *type,word *msgoffset,NETADDR *addr);



struct _show_kludges
{
  char *paged;
  int msgoffset;
  int pause;
  char *nonstop;
};


static void near GotoCleos(int msgoffset)
{
  if (usr.bits2 & BITS2_CLS)
  {
    Goto(msgoffset, 1);
    Clear_To_Screen_End();
  }
}



static int near DoTheMoreThing(char *nonstop, char *paged, int msgoffset, int inbrowse)
{
  int ch;
  
  Puts(CYAN CLEOL);

  if ((ch=MoreYnns_Read(inbrowse))==YES)
  {
    *paged=TRUE;
    GotoCleos(msgoffset);
  }
  else if (ch==M_NONSTOP)
    *nonstop=TRUE;
  else /* no or other character type */
  {
    if (ch != NO) /* if we got some other menu option to process */
    {
      next_menu_char=ch;
  /*  *paged=TRUE; */
  /*  GotoCleos(msgoffset);  DLN: less busy/confusing screen without this */
      display_line=(usr.video || !(usr.bits2&BITS2_CLS)) ? 1 : msgoffset;
    }
    return FALSE;
  }

  if (*nonstop || usr.video==GRAPH_TTY || !(usr.bits2 & BITS2_CLS))
    display_line=display_col=1;
  
  return TRUE;
}


/* User callback function for displaying kludge lines to the user */

int DisplayKludges(char *line, void *args, int inbrowse)
{
  struct _show_kludges *psk=args;

  Printf("%s" CLEOL "%s\n", msg_kludge_col, line);

  if (halt())
    return -1;

  if (display_line >= (byte)TermLength() && psk->pause && !psk->nonstop)
  {
    if (! DoTheMoreThing(psk->nonstop, psk->paged, psk->msgoffset, inbrowse))
      return -1;
  }

  return 0;
}



/* Display the kludge lines in a message, calling a user-defined callback   *
 * function.                                                                */

int ShowKludgeLines(char *ctrl, int (*showfunc)(char*,void*,int), void *args, int inbrowse)
{
  char *kp, *end, *new;
  int rc;
  
  if (! GEPriv(usr.priv, prm.ctla_priv))
    return 0;
  
  for (kp=ctrl; kp && *kp=='\x01'; )
  {
    end=strchr(++kp,'\x01');
    
    if (!end)
      end=kp+strlen(kp);
    
    if (end==kp)
    {
      if (*end==0)
        break;
      else continue;
    }


    /* Need to alloc the extra 10 bytes so the callback fn can append       *
     * stuff to the end of the text line.                                   */

    if ((new=malloc((end-kp)+10))==NULL)
      return 0;
    
    memmove(new, kp, end-kp);
    new[(end-kp)]='\0';
    
    rc=(*showfunc)(new, args, inbrowse);

    free(new);

    if (rc != 0)
      return rc;

    kp += end-kp;
  }

  return 0;
}




static int near Ask_KillMsg(XMSG *msg)
{
  int x;

  if ((msg->attr & MSGPRIVATE)==0 || *linebuf)
    return FALSE;

  switch (prm.auto_kill)
  {
    case 0:               /* Don't kill message */
    default:
      return FALSE;

    case 1:               /* Ask if we should kill message */
      WhiteN();
      x=(GetyNAnswer(del_msg, CINPUT_MSGREAD | CINPUT_NOLF)==YES);
      Putc('\n');
      return x;

    case 2:               /* Automatically kill message */
      return TRUE;
  }
}

/* Ask_KillAttach
 *
 * Possibly ask the user whether or not to kill the file attached to
 * this message.  If control specifies defaults, we use those instead.
 *
 */

int Ask_KillAttach(void)
{
  int rc;

  if (*linebuf)
    return FALSE;

  switch (prm.kill_attach)
  {
    case 0:               /* Don't kill file attach */
    default:
      return FALSE;

    case 1:               /* Ask if we should kill attach */
      WhiteN();

      if (!GEPriv(usr.priv,prm.kill_attach_priv))
        rc=TRUE;
      else
      {
        rc=(GetYnAnswer(del_attach, CINPUT_MSGREAD | CINPUT_NOLF)==YES);
        Putc('\n');
      }

      return rc;

    case 2:               /* Attach has already been removed */
      return TRUE;
  }
}


/* Ask_DownloadAttach
 *
 * Ask the user if he/she wants to download a specific file attach.
 */

static int near Ask_DownloadAttach(void)
{
  int rc=FALSE;

  if (!*linebuf)
  {
    WhiteN();
    Putc('\n');
    rc=(GetYnAnswer(dload_attach, CINPUT_MSGREAD | CINPUT_NOLF) != NO);
    Putc('\n');
  }

  return rc;
}


static void near Clear_To_Screen_End()
{
#if 0
  if (usr.video)
  {
    /* Clear rest of screen */

    if (TermLength()-current_line)
    {
      Printf(CYAN "\x16\x19\x03" CLEOL "\n%c", TermLength()-current_line);
      Puts(CLEOL);
      Goto(current_line,'\x01');
    }
  }
#else
  Puts(CLEOS);
#endif
}


#ifdef NEVER
void GotoCleol(int x,int y)
{
  Goto(x,y);
  Puts(CLEOL);
}
#endif

static void near Show_Replies(XMSG *msg)
{
  long otlong=0L;
  long tlong;
  int i;
  
  /* If the FSR is turned on, then there's no need to display links at      *
   * bottom of msg.                                                         */

  if ((usr.bits & BITS_FSR) && usr.video)
    return;

  if (msg->replyto || msg->replies[0])
    Putc('\r');

  if (msg->replyto)
  {
    otlong=(prm.flags2 & FLAG2_UMSGID) ? msg->replyto
             : MsgUidToMsgn(sq, msg->replyto, UID_EXACT);

    if (otlong)
    {
      Printf(this_reply, otlong);

      if (! msg->replies[0])
        Puts(endofline);
    }
  }
    
  if (msg->replies[0])
  {
    tlong=(prm.flags2 & FLAG2_UMSGID) ? msg->replies[0] : MsgUidToMsgn(sq, msg->replies[0], UID_EXACT);

    if (tlong)
    {
      if (msg->replyto && otlong)
        Puts(dot_spsp);
      else Putc('\n');

      Printf(see_also, tlong);
      
      for (i=1; i < MAX_REPLY && msg->replies[i]; i++)
      {
        tlong=(prm.flags2 & FLAG2_UMSGID) ? msg->replies[i] :
              MsgUidToMsgn(sq, msg->replies[i], UID_EXACT);
        
        if (tlong)
          Printf("%s %ld", (i==MAX_REPLY-1 || ! msg->replies[i+1]) ?
                 and : comma, tlong);
      }

      Puts(endofline);
    }
  }
}





static int near ShowMessageHeader(XMSG *msg, word *msgoffset, long mn)
{
  byte string[PATHLEN];
  long amask;
  long acomp;
  int i;

  /* Display a blank line, if we can't do a CLS */
  
  if (usr.video && (usr.bits2 & BITS2_CLS)==0)
    Putc('\n');



  
  /* Build the first part of the 'FROM:' line */
  
  sprintf(string, mfrom, msg_from_col, msg_from_txt_col,
          Strip_Ansi(msg->from, NULL, 0L), msg_attr_col);
  
  /* Build a mask for the message attributes, but strip off MSGLOCAL */

  amask=msg->attr & ~MSGLOCAL;
  

  /* Strip off MSGKILL too, if we're in an echo area */
  
  if (mah.ma.attribs & MA_SHARED)
    amask &= ~MSGKILL;


  /* Now catenate the message atttributes... */
                     
  for (i=0, acomp=1L; i < 16; acomp <<= 1, i++)
    if (amask & acomp)
      if (strlen(string) + strlen(s_ret(n_attribs0 + i)) < MAXLEN)
      {
        strcat(string, s_ret(n_attribs0 + i));
        strcat(string, " ");
      }

  Puts(string);
  Putc('\n');

  if (halt())
    return -1;

  Display_Addr(orig_addr, net_orig, msgoffset, &msg->orig);


  
  /* Display the To: address */
  
  Printf(mto, msg_to_col, msg_to_txt_col, Strip_Ansi(msg->to, NULL, 0L),
         msg_date_col, UIDnum(mn), MsgDate(msg, string));

  if (halt())
    return -1;

  Display_Addr(dest_addr, net_dest, msgoffset, &msg->dest);

  
  
  /* Display the subject */
  
  Printf("%s%s%s %0.*s\n\n" CYAN,
         msg_subj_col,
         (msg->attr & (MSGFILE | MSGFRQ | MSGURQ)) ? files_colon : subj_colon,
         msg_subj_txt_col,
         TermWidth()-10,
         Strip_Ansi(msg->subj, NULL, 0L));

  if (halt())
    return -1;

  if (msgoffset)
    display_line=(char)*msgoffset;
  display_col=1;
  
  return 0;
}


static int near NoMoreMessage(word this, word got, byte *outline[], byte lt[])
{
  word last, i;
  
  /* The last linein the outline[] array is... */
  
  if (got==MAX_MSGDISPLAY)
    last=got-1;
  else last=got;
  
  /* If we're more than 5 lines away from the end of the buffer, or if      *
   * the last line isn't the end of the message, then this isn't the        *
   * end...                                                                 */
     
  if (this < got-5 || (lt[last] & MSGLINE_END)==0)
    return FALSE;
          
  for (i=this; i < got; i++)
  {
    if (*outline[i] ||
        ((lt[i] & MSGLINE_SEENBY) && GEPriv(usr.priv, prm.seenby_priv)) ||
        ((lt[i] & MSGLINE_KLUDGE) && GEPriv(usr.priv, prm.ctla_priv)))
    {
      return FALSE;
    }
  }
    
  return TRUE;
}



static int near CanShowLine(char linetype)
{
  if (linetype & MSGLINE_KLUDGE)
  {
    if (! GEPriv(usr.priv, prm.ctla_priv))
      return FALSE;
    else Puts(msg_kludge_col);
  }
  else if (linetype & MSGLINE_SEENBY)
  {
    if (! GEPriv(usr.priv, prm.seenby_priv))
      return FALSE;
  }
  else if (linetype & MSGLINE_QUOTE)
    Puts(msg_quote_col);
  else Puts(msg_text_col);
  
  return TRUE;
}








static int near ShowMessageLines(int got, byte *outline[], byte lt[],
                                   char *paged, word msgoffset, int pause,
                                   char *nonstop,
                                   int inbrowse)
{
  int this;
  
  for (this=0; this < got; this++)
  {
    /* If we're near the end of the message, strip excess blank lines */

    if (NoMoreMessage(this, got, outline, lt))
      break;

    if (! CanShowLine(lt[this]))
      continue;

    Puts(CLEOL);

    Puts(outline[this]);

    if (lt[this] & MSGLINE_KLUDGE)
      Puts(msg_text_col);
    
    Putc('\n');

    if (halt())
      return -1;

    if (display_line >= TermLength() && pause && ! *nonstop)
    {
      if (! DoTheMoreThing(nonstop, paged, msgoffset, inbrowse))
        return -2;
    }
  }
  
  return 0;
}



int Msg_Display(HMSG msgh,
                XMSG *msg,
                int pause,
                char *areano,
                long msgnum,
                int offset,
                char *ctrl,
                int inbrowse)
{
  struct _show_kludges sk;
  word msgoffset, got;
  word wid, n_ol;
  sword kret;

  byte *ol[MAX_MSGDISPLAY];
  byte lt[MAX_MSGDISPLAY];
  byte last_attr;
  byte nonstop;
  byte paged;

#ifdef MAX_TRACKER
  int track_ofs;
  char *track_col;
#endif

  last_attr=0;
  msgoffset=5+offset;
  msgeof=nonstop=paged=FALSE;
  wid=TermWidth()-1;
  usr.msgs_read++;
  ci_read();

  if ((usr.bits & BITS_FSR) && usr.video && (usr.bits2 & BITS2_CLS) && pause)
  {
    byte was_no_output;

    DrawReaderScreen(&mah, inbrowse);
    DisplayMessageHeader(msg, &msgoffset, msgnum, MsgGetHighMsg(sq), &mah);

    if (hasRIP() && strstr(reader_box_top,"!|"))
    {
      was_no_output=no_remote_output;
      no_remote_output=TRUE;

      /* Display this locally so that the display makes sense */

      ShowMessageHeader(msg, NULL, msgnum);
      Puts(CLEOS);

      no_remote_output=was_no_output;

    }

#ifdef MAX_TRACKER
    track_ofs=3;
    track_col=fsr_border_col;
#endif
  }
  else
  {
    if (ShowMessageHeader(msg, &msgoffset, msgnum)==-1)
      return -1;

#ifdef MAX_TRACKER
    track_ofs=1;
    track_col=WHITE;
#endif
  }

#ifdef MAX_TRACKER
  TrackShowInfo(sq, &mah, msg, msgnum, ctrl, msgoffset-1, track_ofs, track_col);
#endif

  logit(log_read_msg, msgnum, areano);
  
  /* Display the message kludge lines to the user */

  sk.paged=&paged;
  sk.msgoffset=msgoffset;
  sk.pause=pause;
  sk.nonstop=&nonstop;

  if ((kret=ShowKludgeLines(ctrl, DisplayKludges, &sk, inbrowse))==-1)
    return -1;
  else if (kret < -1)
    return 0;


  /* Now allocate memory for displaying the message text itself */

  if ((n_ol=Alloc_Outline(ol)) < 1)
    return -2;
  

  /* Keep getting lines and displaying them until we run out of msg text */

  while ((got=Msg_Read_Lines(msgh, n_ol, wid, 0, ol,
                             lt, &last_attr, MRL_QEXP)) > 0)
  {
    if ((kret=ShowMessageLines(got,
                               ol,
                               lt,
                               &paged,
                               msgoffset,
                               pause,
                               &nonstop,
                               inbrowse)) < 0)
    {
      Dealloc_Outline(ol);

      /* Return SUCCESS if user answered 'N' to More Y/N, but return        *
       * FAILURE if we got a ^c.                                            */

      if (kret==-1)
        return -1;
      else return 0;
    }
  }

  Dealloc_Outline(ol);
 
  if (display_line >= (byte)(TermLength()-1) && pause && !nonstop)
  {
    if (! DoTheMoreThing(&nonstop,&paged,msgoffset,inbrowse))
      return -1;
  }
  else
    Clear_To_Screen_End();

  if ((msg->replies[0] || msg->replyto) && display_line >= TermLength()-3)
  {
    if ((usr.bits2 & BITS2_CLS) && pause)
      Goto(TermLength(), 1);

    if (pause && !nonstop)
      DoTheMoreThing(&nonstop, &paged, msgoffset,inbrowse);
  }

  if (! *linebuf)
    Show_Replies(msg);


  /* Make sure that the menu doesn't push us off-screen. */
  
  if (!nonstop && pause && (usr.bits2 & BITS2_MORE) &&
      display_line > (byte)(TermLength()-(menu_lines+5+
                             ((usr.bits & BITS_FSR) ? -2 /*2u*/ : 0u))))
  {
    if (display_line < (byte)(TermLength()-1))
      Putc('\n');

    if ((usr.bits2 & BITS2_CLS) && pause && !hasRIP())
      Goto(TermLength()-1, 1);

    if (pause)
      DoTheMoreThing(&nonstop, &paged, msgoffset, inbrowse);
  }

  return 0;  
}




static void near Display_Addr(char *orig_or_dest,char *type,word *msgoffset,NETADDR *addr)
{
  NFIND *nf;

  if (! (mah.ma.attribs & MA_NET))
    return;
  
  if (MsgToUs(addr))
    return;
  
  if ((nf=NodeFindOpen(addr)) != NULL)
  {
    Printf(addrfmt, msg_addr_col, orig_or_dest, msg_locus_col,
           nf->found.name, Address(addr), nf->found.city);

    NodeFindClose(nf);
  }
  else Printf(unlisted_system, type, Address(addr));

  if (msgoffset)
    (*msgoffset)++;
}






int Msg_Current(void)
{
  return Msg_Get_Msgs(DIRECTION_NEXT, last_msg, FALSE, TRUE, TRUE);
}


int Msg_Next(long startmsg)
{
  return Msg_Get_Msgs(direction=DIRECTION_NEXT, startmsg+1, FALSE,
                      FALSE, TRUE);
}





int Msg_Previous(long startmsg)
{
  return Msg_Get_Msgs(direction=DIRECTION_PREVIOUS,
                      startmsg ? startmsg-1 : 0xffffffffLu, 
                      FALSE, FALSE, TRUE);
}





int Msg_Nonstop(void)
{
  Puts(CLS);
  Printf(message_name, msgar_name, usr.msg, MAS(mah, descript));
  Putc('\n');
  
  while (Msg_Get_Msgs(direction,
                      last_msg+(direction==DIRECTION_NEXT ? 1 : -1),
                      TRUE,
                      FALSE,
                      FALSE)==0)
  {
    Puts(n_n);

    if (Mdm_flush_ck() || brk_trapped || mdm_halt())
    {
      brk_trapped=0;
      mdm_dump(DUMP_ALL);
      ResetAttr();
      return -1;
    }
  }

  return 0;
}






static int near Msg_Get_Msgs(int dir,dword startmsg,int nonstop,int exact,int show_err)
{
  HMSG msgh;
  char *ctrl;
  XMSG msg;
  long clen;
  int ret;

  /*direction=dir;*/

  if (startmsg > MsgHighMsg(sq) && dir==DIRECTION_PREVIOUS &&
      startmsg != 0xffffffffLu)
    startmsg=MsgHighMsg(sq);
  
  while (startmsg > 0 && startmsg <= MsgHighMsg(sq) &&
         startmsg != 0xffffffffLu)
  {
    if ((msgh=MsgOpenMsg(sq, MOPEN_RW, startmsg)) != NULL)
    {
      if (MsgReadMsg(msgh, &msg, 0L, 0L, NULL, 0L, NULL) != -1 &&
          CanSeeMsg(&msg))
      {
        clen=MsgGetCtrlLen(msgh);
        
        if (clen <= 0)
          ctrl=NULL;
        else if ((ctrl=malloc((int)clen+5)) != NULL)
        {
          MsgReadMsg(msgh,NULL,0L,0L,NULL,clen,ctrl);
          ctrl[(int)clen]='\0';
        }

        last_msg=MsgCurMsg(sq);

        if (!nonstop)
        {
          if (!((usr.bits & BITS_FSR) && usr.video))
            Puts(CLS);
          else if (hasRIP())
            Lputs(CLS);
        }

        ret=Msg_Display(msgh,
                        &msg,
                        nonstop ? FALSE : (usr.bits2 & BITS2_MORE),
                        usr.msg,
                        last_msg,
                        0,
                        ctrl,
                        FALSE);

        Recd_Msg(msgh, &msg, TRUE);

        MsgCloseMsg(msgh);
        
        /* The Msg_Display() result of -1 means that either ^c was          *
         * hit, or someone answered "n" to a "More" question.  Xlat the     *
         * return value, since OUR -1 means that the message was not found. */

        if (ret==-1)
        {
          if (ctrl)
            free(ctrl);

          return -2;
        }

        if (!nonstop)
        {
          int isnetmsg = !!(mah.ma.attribs & MA_NET);

          if (MsgToThisUser(msg.to) && Msg_UnreceivedAttach(&msg, ctrl, isnetmsg))
          {
            if (Ask_DownloadAttach())
              Msg_AttachDownload(&msg, ctrl, isnetmsg);
            if (Ask_KillAttach())
              Msg_AttachKill(&msg, ctrl, isnetmsg);
          }

          if ((msg.attr & MSGREAD) &&
            (MsgToThisUser(msg.to) || MsgToThisUser(msg.from)) &&
            Ask_KillMsg(&msg))
            Msg_Kill(last_msg);
        }

        if (ctrl)
          free(ctrl);

        return 0;
      }

      MsgCloseMsg(msgh);
    }

    if (exact)  /* If looking for a specific msg, let caller handle error */
      break;
    
    if (dir==DIRECTION_NEXT)
      startmsg++;
    else startmsg--;
  }

  if (!show_err)  /* Don't display error message for nonstop mode */
    return -1;
  
  /* If we got this far, then we obviously didn't get a message */
  
  if (!exact)
  {
    if (dir==DIRECTION_NEXT && startmsg != 0)
      last_msg=MsgHighMsg(sq);
    else last_msg=0;
  }

  Printf(endavailmsg, (mah.ma.attribs & MA_PVT)==0 ? blank_str
                                                   : remain_pvt);
  Clear_KBuffer();

  /* We dump input here because new users (who aren't used to            *
   * Maximus/Opus-style message bases) may go whacking return when there *
   * is even a slight pause.  Inserting this command should make sure    *
   * we don't get into a long loop, searching for the last message.      */

  mdm_dump(DUMP_INPUT);

  return -1;
}

int Msg_Read_Reply(void)
{
  return (Msg_Read_RepOrig(FALSE));
}

int Msg_Read_Original(void)
{
  return (Msg_Read_RepOrig(TRUE));
}
 

static int near Msg_Read_RepOrig(int orig)
{
  HMSG msgh;
  XMSG msg;
  long thismsg;
  
  if ((msgh=MsgOpenMsg(sq, MOPEN_READ, last_msg))==NULL)
  {
    Puts(msgnotavail);
    return -1;
  }
  
  MsgReadMsg(msgh, &msg, 0L, 0L, NULL, 0L, NULL);
  MsgCloseMsg(msgh);
  
  thismsg=MsgUidToMsgn(sq, orig ? msg.replyto : msg.replies[0], UID_EXACT);

  if (thismsg==0L || 
      Msg_Get_Msgs(DIRECTION_NEXT, thismsg, FALSE, TRUE, FALSE)==-1)
  {
    Puts(msgnotavail);
    return -1;
  }
    
  return 0;
}



int Msg_Read_Individual(int ch)
{
  char temp[PATHLEN];

  int msgnum;

  if (! isdigit(ch))
    ch='0';

  if ((usr.bits & BITS_HOTKEYS) && ! *linebuf)
  {
    InputGetsC(temp, ch, blank_str); /* else std hotkey, no hotflash */

    if (isdigit(*temp))
      msgnum=atoi(temp);
    else
    {
      strcpy(linebuf,temp);
      return -1;
    }
  }
  else
  {
    strocpy(linebuf+1, linebuf);
    *linebuf=(char)ch;
    msgnum=atoi(linebuf);
  }

  *linebuf='\0';

  /* If we need to convert between UMSGIDs and message numbers */

  if (prm.flags2 & FLAG2_UMSGID)
    msgnum=MsgUidToMsgn(sq, msgnum, UID_NEXT);

  if (msgnum && Msg_Get_Msgs(direction, msgnum, FALSE, FALSE, FALSE)==0)
    return 0;
  
  Puts(msgnotavail);
  return -1;
}


static int near MoreYnns_Read(int inbrowse)
{
  int c;

  c= GetListAnswer(Yne, NULL, useyforyesns,
                   inbrowse ? CINPUT_NOLF : (CINPUT_NOLF | CINPUT_MSGREAD | CINPUT_ANY | CINPUT_SCAN),
                   inbrowse ? browse_more_prompt : reader_more_prompt);

  if (usr.video==GRAPH_TTY)
    Puts(moreynns_blank);
  else Puts(inbrowse ? browse_more_blank : reader_more_blank);

  return c;
}

