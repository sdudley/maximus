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
static char rcs_id[]="$Id: mb_list.c,v 1.1.1.1 2002/10/01 17:52:10 sdudley Exp $";
#pragma on(unreferenced)

/*# name=One-per-line (L)ist) code for the BROWSE command
*/

#define MAX_LANG_m_browse

#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_msg.h"
#include "m_browse.h"



extern int last_title;
extern int idling;

int List_Begin(BROWSE *b)
{
  NW(b);

  Puts(br_list_head1);
  Puts(br_list_head2);
  
  return 0;
}







int List_Display(BROWSE *b)
{
/*
1234 12345678901234567890 12345678901234567890 1234567890123456789012345678901
  14 Scott Dudley         Joaquim Homrighausen Subject matter at hand
  15 Haruyasu Yoshizaki   Other long name here This is a test.
Area 14: Fowl Weather Post Questions and Answers
*/

  /* Get rid of any dots... */

  if (last_title || idling)
    Rev_Up();
  
  if (halt())
    return -1;

  if (last_title && (b->bflag & BROWSE_ACUR)==0)
  {
    Printf(br_this, MAS(mah, name), MAS(mah, descript));

    if (MoreYnBreak(b->nonstop, CYAN))
      return -1;

    if (display_line==1 && !*b->nonstop)
    {
      Puts(br_list_head1);
      Puts(br_list_head2);
    }
  }

  last_title=FALSE;
  
  Printf(br_list_format,
         (prm.flags2 & FLAG2_UMSGID) ? MsgMsgnToUid(b->sq, b->msgn) : b->msgn,
         (MsgToThisUser(b->msg.to) &&
          ((mah.ma.attribs & MA_NET)==0 || MsgToUs(&b->msg.dest)) &&
          (b->msg.attr & MSGREAD)==0)
           ? br_msg_new 
           : ((b->msg.attr & MSGPRIVATE) ? br_msg_pvt : br_msg_notnew),
         MsgToThisUser(b->msg.from) ? LRED : YELLOW,
         Strip_Ansi(b->msg.from, NULL, 0),
         MsgToThisUser(b->msg.to) ? LRED : LGREEN,
         Strip_Ansi(b->msg.to, NULL, 0),
         Strip_Ansi(b->msg.subj, NULL, 0));

  if (MoreYnBreak(b->nonstop, CYAN))
    return -1;

  if (display_line==1 && !*b->nonstop)
  {
    Puts(br_list_head1);
    Puts(br_list_head2);
  }

  /*vbuf_flush();*/
  
  return 0;
}
  

int List_After(BROWSE *b)
{
  NW(b);
  return 0;
}

