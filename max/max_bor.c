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
static char rcs_id[]="$Id: max_bor.c,v 1.1.1.1 2002/10/01 17:51:28 sdudley Exp $";
#pragma on(unreferenced)

/*# name=BORED, the line-oriented editor
*/

#define MAX_LANG_max_bor

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <setjmp.h>
#include <string.h>
#include <mem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "max_msg.h"
#include "max_edit.h"
#include "maxedp.h"
#include "m_reply.h"


int linenum;
static struct _replyp *preply;


static void near Bored_GetLine(int max);
static void near Bored_Continue(void);

int Bored_Menu(XMSG *msg)
{
  return Display_Options(*PRM(edit_menu) ? PRM(edit_menu) : "EDIT", msg);
}

int Bored(XMSG *msg,HMSG msgh,struct _replyp *pr)
{
  long tlong;

  preply=pr;
  linenum=1;
  num_lines=0;
  usrwidth=(byte)min(TermWidth(), LINELEN);

  tlong=(coreleft()-BORED_SAVE_MEM)/(LINELEN*2L);

  max_lines=(int)min(tlong, (long)MAX_LINES);

  if (max_lines <= 0)   /* Something's really wrong here! */
  {
    logit(mem_nmsgb);
    Puts(mem_nmsgb);

    Press_ENTER();
    return ABORT;
  }
  else                /* Things look A-OK! */
  {
    if (setjmp(jumpto)==0) /* Really ugly, but the best way to handle errs */
    {
      int rc;

      if (usr.help==NOVICE)
        Display_File(0,NULL,PRM(hlp_editor));

      in_msghibit++;

      if (msgh)
        Load_Message(msgh);
      else
        Bored_Continue();

      rc = Bored_Menu(msg);

      in_msghibit--;

      return rc;
    }
    else
    {
      Free_All();
      return ABORT;
    }
  }
}




static void near Bored_Continue(void)
{
  linenum=num_lines+1;

  Putc('\n');

  Bored_GetLine(0);     /* Get as many lines as required */
}



static void near Bored_GetLine(int max)
{
  char string[LINELEN+1];
  char nextstring[MAX_WRAPLEN+2];
  char temp[LINELEN];

  int linecount;
  int wrapped;

  *nextstring='\0';
  *string='\0';

  linecount=0;

  Clear_KBuffer();

  Printf(LGREEN "   Ú\x19Ä%c¿\n",usrwidth-7);

  while (max==0 || linecount < max)
  {
    if (num_lines==max_lines-2)
      Puts(last_line);

    if (num_lines >= max_lines-1)
    {
      Puts(toomanylines);
      return;
    }

    strcpy(string, nextstring);

    sprintf(temp, GRAY "%2d: " YELLOW, linenum);

    wrapped=FALSE;
    
    if (Input(string, INPUT_NLB_LINE | INPUT_WORDWRAP, 0,
              min(usrwidth-3, LINELEN) - 4, temp)==1)
    {
      wrapped=TRUE;
      strcpy(nextstring,string+strlen(string)+1);
    }
    else *nextstring='\0';

    if (! *string)
      break;
    else
    {
      if (Allocate_Line(linenum))
        EdMemOvfl();

      if (wrapped)
        screen[linenum][0]=SOFT_CR;

      strcpy(screen[linenum]+1,string);
      *string='\0';

      linecount++;
      linenum++;
    }
  }
}





static void near Bored_List(XMSG *msg)
{
  word x;
  char nonstop;

  display_line=display_col=1;
  nonstop=FALSE;

  Puts("\n\n" CLS);

  Printf(list_from, Strip_Ansi(msg->from, NULL, 0L));
  Printf(list_to, Strip_Ansi(msg->to, NULL, 0L));
  Printf(list_subj, Strip_Ansi(msg->subj, NULL, 0L));

  for (x=1; x <= num_lines; x++)
  {
    if (MoreYnBreak(&nonstop,CYAN))
      break;

    Printf(blfmt1, x, screen[x]+1);
    Putc('\n');
  }

  Putc('\n');
}






static void near Bored_To(XMSG *msg)
{
  Printf(bed_to, Strip_Ansi(msg->to, NULL, 0L));

  if (!(prm.flags & FLAG_no_ulist) && !(mah.ma.attribs & MA_NET))
    Puts(bed_lu);

  for (;;)
  {
    Puts(CYAN);
    InputGetsL(msg->to, XMSG_TO_SIZE-1, bto);

    if (eqstr(msg->to, qmark) && (prm.flags & FLAG_no_ulist)==0)
      UserList();
    else if (eqstri(msg->to, sysop_txt) && (mah.ma.attribs & MA_NET)==0)
    {
      strcpy(msg->to, PRM(sysop));
      break;
    }
    else if (*msg->to)
    {
      if ((mah.ma.attribs & (MA_NET | MA_SHARED))==0 &&
          (msg->attr & MSGPRIVATE) &&
          !IsInUserList(msg->to, TRUE))
      {
        Puts(userdoesntexist);
      }
      else break;
    }
  }

  Clear_KBuffer();
}





static void near Bored_From(XMSG *msg)
{
  Printf(bfrom, Strip_Ansi(msg->from, NULL, 0L));

  do
  {
    Puts(CYAN);
    InputGetsL(msg->from, XMSG_FROM_SIZE-1, bfromsp);
  }
  while (*msg->from=='\0');

  Clear_KBuffer();
}





static void near Bored_Subject(XMSG *msg)
{
  Printf(bsubj, Strip_Ansi(msg->subj, NULL, 0));

  if ((mah.ma.attribs & (MA_NET|MA_ATTACH))==(MA_NET|MA_ATTACH) &&
      !mailflag(CFLAGM_ATTRANY))
  {
    Puts(cant_change_field);
    Press_ENTER();
    return;
  }

  do
  {
    Puts(CYAN);
    InputGetsL(msg->subj, XMSG_SUBJ_SIZE-1, bsubject);
  }
  while (*msg->subj=='\0');

  Clear_KBuffer();
}




static void near Bored_Handling(XMSG *msg)
{
  word x;
  byte ch;

  if (inmagnet)
    Puts(CLS);
  
  do
  {
    if (! *linebuf)
    {
      Puts("\n" CLS);

      for (x=0; x < 16; x++)
      {
        Printf(YELLOW "%c" GRAY ")%s",
               s_ret(n_msg_attr0+x)[0],
               &(s_ret(n_msg_attr0+x)[1]) );

        Printf(rle_str, '.', 20-(strlen(s_ret(n_msg_attr0+x))+1));

        Printf(WHITE " %s\n", (msg->attr & (1 << x)) ? yep : nope);
      }

      Puts(edlist_quit);
    }

    ch=(byte)toupper(KeyGetRNP(select_p));

    for (x=0; x < 16; x++)
      if (ch==(s_ret(n_msg_attr0+x))[0])
      {
        if (msg->attr & (1 << x))
          msg->attr &= ~(1 << x);
        else
        {
          /* If user tries to convert a message with subject line into
           * a file attach (which could allow them to specify a full
           * filename/path to download), null out the subject line so
           * that it does not cause a security problem.
           */

          if ((1 << x) == MSGFILE &&
              (msg->attr & MSGFILE)==0 &&
              (mah.ma.attribs & (MA_NET|MA_ATTACH))==(MA_NET|MA_ATTACH) &&
              !mailflag(CFLAGM_ATTRANY))
          {
            *msg->subj = 0;
          }


          msg->attr |= (1 << x);
        }

        break;
      }
  }
  while (ch != 'Q' && ch != '\0' && ch != '|' && ch != '\r');
}


static void near Bored_Delete(void)
{
  char temp[BUFLEN];
  word from, to, ln, move;
  word old_num_lines;

  WhiteN();

  InputGets(temp, line_del_from);

  from=atoi(temp);

  if (*temp=='\0' || from==0 || from > num_lines)
    return;

  InputGets(temp, line_del_to);
  
  to=atoi(temp);
  
  if (*temp=='\0' || to==0 || to > num_lines || to < from)
    return;
  
  old_num_lines=num_lines;

  for (ln=from; ln <= to; ln++)
    Free_Line(ln);

  /* Now shift 'move' lines back by the appropriate amount */

  move=old_num_lines-to;
  memmove(screen+from, screen+to+1, move * sizeof(char *));

  /* Set the remaining lines to null */

  memset(screen+num_lines+1, '\0', (old_num_lines-num_lines) * sizeof(char *));
}


static void near Bored_Insert(void)
{
  char temp[BUFLEN];
  word cx, x;


  WhiteN();
  
  InputGets(temp, ins_bef);

  cx=atoi(temp);

  if (! (cx > 0 && cx <= num_lines && num_lines < max_lines-1))
  {
    Puts(toomanylines);
    return;
  }

  
  if (Allocate_Line(num_lines+1))
    EdMemOvfl();

  for (x=num_lines-1;x >= cx;x--)
  {
    screen[x+1][0]=screen[x][0];
    strocpy(screen[x+1]+1, screen[x]+1);
  }

  screen[cx][0]='\r';
  screen[cx][1]='\0';

  if (! *linebuf)
    Puts(line_ins);

  /* Now allow the user to enter text on that line. */

  Puts(editl1);
  Puts(editl2);

  Puts(CYAN);

  InputGetsL(screen[cx]+1, min(BUFLEN,LINELEN)-7, new_st);
}



static void near Bored_Edit(void)
{
  char temp[PATHLEN];
  char search[PATHLEN];
  char replace[PATHLEN];
  char *p;

  word rep_len, cx;

  WhiteN();

  InputGets(temp,line_edit_num);

  cx=atoi(temp);

  if (cx > 0 && cx <= num_lines)
  {
    if (! *linebuf)
    {
      Putc('\n');
      Printf(blfmt1, cx, screen[cx]+1);
    
      WhiteN();
      WhiteN();
    }

    if (usr.priv==NOVICE)
      Display_File(0, NULL, PRM(hlp_replace));

    InputGetsL(search, BUFLEN, rep_what);

    if (*search && ! stristr(screen[cx]+1, search))
    {
      Printf(word_not_found, search);
      return;
    }

    rep_len=(LINELEN-strlen(screen[cx]+1)-6)+strlen(search);

    if (rep_len <= 0)
    {
      Puts(noroom);
      return;
    }

    Puts(editl1);

    if (! *search)
      Puts(editl2);
    else Printf(editl3, search);

    Printf(e_numch, rep_len);

    InputGetsL(replace,
              min(BUFLEN, rep_len),
              new_st);

    if (! *search)
    {
      strocpy(screen[cx]+1+strlen(replace), screen[cx]+1);

      memmove(screen[cx]+1, replace, strlen(replace));

      Putc('\n');
      Printf(blfmt1, cx, screen[cx]+1);
      Putc('\n');
    }
    else
    {
      /* Substitute the replacement for the search. */
      
      if ((p=stristr(screen[cx]+1, search)) != NULL)
      {
        if (strlen(replace) >= strlen(search))
        {
          if (strlen(replace) != strlen(search))
            strocpy(p+(strlen(replace)-strlen(search)), p);

          memmove(p, replace, strlen(replace));
        }
        else
        {
          memmove(p, replace, strlen(replace));

          memmove(p+strlen(replace),
                  p+strlen(search),
                  strlen(p+strlen(replace)));
        }
      }

      Putc('\n');
      Printf(blfmt1, cx, screen[cx]+1);
      Putc('\n');
    }
  }
}


struct _hinfo
{
  char nonstop;
  char display_msg;
  word total_lines;
  word startline;
  word endline;
  byte *initials;
  word lc;
};

    

/* WARNING!  This function must be declared global, even though it is
 * only referenced locally!  WATCOM's WHOOSH (dynamic overlay manager)
 * only creates thunks for global functions, so this would bomb
 * (if declared static) when called by a pointer from another module.
 */

/*static*/ word BodyToMsg(byte *txt, void *info, word lt)
{
  struct _hinfo *hi=(struct _hinfo *)info;
  char line[MAX_LINELEN];
  
  if ((lt & MSGLINE_SEENBY) && ! GEPriv(usr.priv, prm.seenby_priv))
    return TRUE;
  else if (lt & MSGLINE_KLUDGE && ! GEPriv(usr.priv, prm.ctla_priv))
    return TRUE;
  else if (lt & MSGLINE_END)
    return TRUE;


  if (++hi->total_lines >= hi->startline && num_lines < max_lines-1)
  {
    if (hi->total_lines > hi->endline)
      return FALSE;

    if (QuoteThisLine(txt))
      sprintf(line, " %s> %s", hi->initials, txt);
    else strcpy(line, txt);

    if (! hi->display_msg)
    {
      if (Allocate_Line(num_lines+1))
        EdMemOvfl();

      linenum=num_lines;

      screen[linenum][0]='\r';
      strcpy(screen[linenum]+1, line);
    }

    Printf(GRAY "%2d: " CYAN "%s\n",
           hi->display_msg ? hi->lc++ : linenum++, line);

    if (hi->display_msg && MoreYnBreak(&hi->nonstop,CYAN))
      return FALSE;
  }
  
  return TRUE;
}


static void near Bored_Quote(void)
{
  HMSG qmh;
  XMSG msg;

  char temp[PATHLEN];
  char initials[10];

  struct _hinfo hi;
  int wid, swid;

  if (!preply)
  {
    Puts(not_reply);
    return;
  }

  for (hi.display_msg=TRUE; hi.display_msg; )
  {
    WhiteN();
    InputGets(temp,qstart);

    if (! *temp)
      return;
    else if (eqstri(temp,qmark))
    {
      hi.display_msg=TRUE;
      hi.startline=1;
      hi.endline=0x7fff;
    }
    else
    {
      hi.display_msg=FALSE;
      hi.startline=atoi(temp);

      InputGets(temp, qend);

      if (! *temp)
        return;

      hi.endline=atoi(temp);

      if (hi.startline < 1)
        hi.startline=1;

      if (hi.endline < hi.startline)
        hi.endline=hi.startline;

      if (hi.endline-hi.startline >= 20)
        Puts(sstmt);
    }

    display_line=display_col=0;

    Putc('\n');

    qmh=MsgOpenMsg(preply->fromsq,
                   MOPEN_READ,
                   MsgUidToMsgn(preply->fromsq, preply->original, UID_EXACT));
    if (qmh==NULL)
      return;

    if (MsgReadMsg(qmh, &msg, 0L, 0L, NULL, 0L, NULL)==-1)
    {
      MsgCloseMsg(qmh);
      return;
    }

    Parse_Initials(msg.from, initials);

    hi.nonstop=FALSE;
    hi.initials=initials;
    hi.total_lines=0;
    hi.lc=1;

    wid=usrwidth-strlen(initials)-HARD_SAFE-6;
    swid=usrwidth-strlen(initials)-SOFT_SAFE-6;

    ShowBody(qmh, BodyToMsg, (void *)&hi, wid, swid, MRL_QEXP);

    MsgCloseMsg(qmh);
  }
}






static int near MessageToFile(HAREA sq, dword lmsg, HMSG msgh, char *name, word flag)
{
  HMSG imsgh;
  char initials[MAX_INITIALS];
  FILE *out;
  XMSG imsg;

  
  if (msgh)    /* If it's a change */
    imsgh=msgh;
  else imsgh=MsgOpenMsg(sq, MOPEN_READ, lmsg);

  if (!imsgh)
    return FALSE;
  
  MsgReadMsg(imsgh, &imsg, 0L, 0L, NULL, 0L, NULL);
  Parse_Initials(imsg.from, initials);

  if ((out=shfopen(name, fopen_write, O_WRONLY | O_CREAT | O_TRUNC | O_NOINHERIT))==NULL)
  {
    if (!msgh)
      MsgCloseMsg(imsgh);

    return FALSE;
  }

  MsgBodyToFile(imsgh, out, msgh==NULL, msgh != NULL, initials, flag);

  fclose(out);

  if (msgh==NULL)    /* Only close if it's a reply */
    MsgCloseMsg(imsgh);
  
  return TRUE;
}



word Local_Editor(XMSG *msg, HMSG msgh, long msgnum, char *ctrl_buf,struct _replyp *pr)
{
  union stamp_combo d1, d2;
  FILE *out;
  
  byte temp[PATHLEN];
  byte msgtemp[PATHLEN];
  byte *ed;

  word aborted;
  word erlv;


  sprintf(msgtemp, msgtemp_name, PRM(temppath), task_num);

  /* If we need to copy the message to a file, do so now */

  if (pr)
    MessageToFile(pr->fromsq,
                  MsgUidToMsgn(pr->fromsq, pr->original, UID_EXACT),
                  msgh, msgtemp, MRL_QEXP);
  else if (msgh)
    MessageToFile(sq, last_msg, msgh, msgtemp, 0);

  /* Create the editor command */
    
  ed=PRM(local_editor);

  sprintf(temp, ed+(*ed=='@'), msgtemp);
  
  /* Get the date of the file as it stands now */
  
  FileDate(msgtemp, &d1);

  erlv=Outside(NULL, NULL, OUTSIDE_RUN | OUTSIDE_STAY, temp, FALSE, CTL_NONE,
               RESTART_MENU, NULL);


  /* Now get the date of the file as it stands now.  If nothing has been    *
   * changed, assume that it's an abort.                                    */
     
  FileDate(msgtemp, &d2);
  
  if ((d1.ldate==d2.ldate && pr!=NULL) || erlv)
  {
    unlink(msgtemp);
    return ABORT;
  }
  
  /* Now try to open the reply file */

  if ((out=shfopen(msgtemp, fopen_readb, O_RDONLY | O_BINARY | O_NOINHERIT))==NULL)
    return ABORT;

  aborted=SaveMsg(msg, out, TRUE, msgh ? msgnum : 0L,
                  msgh != NULL, &mah, usr.msg, sq,
                  ctrl_buf, (pr) ? pr->fromareaname : NULL, FALSE);

  fclose(out);

  if (aborted)
    return ABORT;
  else
  {
    unlink(msgtemp);
    return LOCAL_EDIT;
  }
}

int Exec_Edit(int type, char **result, XMSG *msg, unsigned *puiFlag)
{
  *result=NULL;
  
  if (!msg)
    return 0;

  switch (type)
  {
    case edit_abort:
      WhiteN();

      if (GetyNAnswer(abortmsg,0)==YES)
      {
        *puiFlag |= RO_QUIT;
        Free_All();
        return 0;
      }
      break;
      
    case edit_continue: 
      if (inmagnet)
      {
        *puiFlag |= RO_SAVE;
        return 0;
      }

      Bored_Continue();
      break;

    case edit_save:     *puiFlag |= RO_SAVE;return 0;
    case edit_list:     Bored_List(msg);    break;
    case edit_edit:     Bored_Edit();       break;
    case edit_insert:   Bored_Insert();     break;
    case edit_delete:   Bored_Delete();     break;
    case edit_to:       Bored_To(msg);      break;
    case edit_from:     Bored_From(msg);    break;
    case edit_subj:     Bored_Subject(msg); break;
    case edit_handling: Bored_Handling(msg);break;
    case read_diskfile: Read_DiskFile();    break;
    case edit_quote:    Bored_Quote();      break;
    default:            logit(bad_menu_opt, type);  return 0;
  }
  
  return 0;
}


