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
static char rcs_id[]="$Id: m_full.c,v 1.2 2003/06/04 23:31:28 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "max_msg.h"
#include "m_full.h"


static char * near Show_Attributes(long attr,char *str);
static void near DisplayMessageFrom(XMSG *msg);
static void near DisplayMessageTo(XMSG *msg);

void DrawReaderScreen(MAH *pmah, int inbrowse)
{
  Printf(inbrowse ? browse_rbox_top : reader_box_top,
         PMAS(pmah, name),
         PMAS(pmah, descript),
         TermWidth() - strlen(MAS(*pmah, descript)) - strlen(MAS(*pmah, name)) - 5);
  Printf(reader_box_mid, (prm.flags2 & FLAG2_UMSGID) ? reader_box_highest : reader_box_of);
  Printf(reader_box_from);
  Printf(reader_box_to);
  Printf(reader_box_subj);
  Printf(reader_box_bottom, TermWidth());
}


void DisplayMessageHeader(XMSG *msg, word *msgoffset, long msgnum, long highmsg, MAH *pmah)
{
  DisplayMessageNumber(msg, msgnum, highmsg);
  DisplayMessageAttributes(msg, pmah);
  DisplayMessageFrom(msg);
  DisplayMessageTo(msg);
  DisplayMessageSubj(msg, pmah);
  Puts(reader_msg_init);

  if (msgoffset)
    *msgoffset=(hasRIP()) ? 1 : 7;

}




void DisplayMessageNumber(XMSG *msg, long msgnum, long highmsg)
{
  int  i;
  long tlong;
  char tmp[64];

  Printf(rbox_msgn, UIDnum(msgnum ? msgnum : MsgGetHighMsg(sq)) + !msgnum);
  Printf(rbox_high, UIDnum(highmsg));

  i=0;
  if (msg->replyto)
  {
    tlong=(prm.flags2 & FLAG2_UMSGID) ? msg->replyto :
           MsgUidToMsgn(sq, msg->replyto, UID_EXACT);

    if (tlong)
      i+=sprintf(tmp, rbox_replyto, tlong);
  }

  if (msg->replies[0])
  {
    tlong=(prm.flags2 & FLAG2_UMSGID) ? msg->replies[0] :
          MsgUidToMsgn(sq, msg->replies[0], UID_EXACT);

    if (tlong)
      i+=sprintf(tmp+i, rbox_replies, tlong);
  }

  if (i)
    Printf(rbox_links, tmp);
}


void DisplayMessageAttributes(XMSG *msg, MAH *pmah)
{
  char temp[PATHLEN];
  long amask;
  
  /* Display the message attributes - everything EXCEPT for local */

  amask=~0 & ~MSGLOCAL;

  /* ... and strip off KILL if in an echo area, too. */

  if (pmah->ma.attribs & MA_SHARED)
    amask &= ~MSGKILL;

  Printf(rbox_attrs, Show_Attributes(msg->attr & amask, temp));
}


static char * near Show_Attributes(long attr, char *str)
{
  int i;
  long acomp;

  /* Now catenate the message atttributes... */
                     
  for (i=0, acomp=1L, *str='\0'; i < 16; acomp <<= 1, i++)
    if (attr & acomp)
    {
      strcat(str, s_ret(n_attribs0+i));
      strcat(str, " ");
    }

  Strip_Trailing(str,' ');
  return str;
}




/**************************************************************************** 
                             The TO/FROM fields field
 ****************************************************************************/

static void near DisplayMessageFrom(XMSG *msg)
{
  DisplayShowName(rbox_sho_fname, msg->from);
  DisplayShowDate(rbox_sho_fdate, (union stamp_combo *)&msg->date_written);
  DisplayShowAddress(rbox_sho_faddr, &msg->orig, &mah);
}


static void near DisplayMessageTo(XMSG *msg)
{
  DisplayShowName(rbox_sho_tname, msg->to);
  DisplayShowDate(rbox_sho_tdate, (union stamp_combo *)&msg->date_arrived);
  DisplayShowAddress(rbox_sho_taddr, &msg->dest, &mah);
}


void DisplayShowName(char *sho_name, char *who)
{
  Printf(sho_name, Strip_Ansi(who, NULL, 0L));
}

void DisplayShowDate(char *sho_date, union stamp_combo *sc)
{
  char temp[PATHLEN];
  
  Printf(sho_date, MsgDte(sc, temp));
}

void DisplayShowAddress(char *sho_addr, NETADDR *n, MAH *pmah)
{
  Printf(sho_addr, (pmah->ma.attribs & MA_NET) ? (char *)Address(n) : (char *)blank_str);
}

void DisplayMessageSubj(XMSG *msg, PMAH pmah)
{
  /* Show just "files attached" if it is a composite local file attach */

  if (!mailflag(CFLAGM_ATTRANY) && (pmah->ma.attribs & MA_NET) != 0 &&
     (msg->attr & MSGFILE) != 0 && AllowAttribute(pmah, MSGKEY_LATTACH))
    Puts(rbox_files_att);
  else
  {
    char * subjline;
    if ((pmah->ma.attribs & MA_NET) && (msg->attr & (MSGFILE | MSGFRQ | MSGURQ)))
      subjline=reader_box_file;
    else
      subjline=reader_box_subj;
    Puts(subjline);
    Printf(rbox_sho_subj, Strip_Ansi(msg->subj, NULL, 0L));
  }
}


