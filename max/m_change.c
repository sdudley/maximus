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
static char rcs_id[]="$Id: m_change.c,v 1.1.1.1 2002/10/01 17:52:39 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: C)hange command
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_msg.h"
#include "node.h"


static int Msg_Change_Err(void);


int Msg_Change(void)
{
  char *ctrl_buf=NULL;
  unsigned ctrl_len;
  NETADDR *dest;
  HMSG msgh=NULL;
  NFIND *nf;
  
  XMSG msg;
  
  long lmsg;
  long hwm;

  word aborted;
  word charge_user;
  sword res;
  word cost;
    
  lmsg=*linebuf ? atol(linebuf) : last_msg;
  *linebuf='\0';
  *orig_msgid='\0';


  if ((msgh=MsgOpenMsg(sq, MOPEN_READ, lmsg))==NULL ||
      MsgReadMsg(msgh, &msg, 0L, 0L, NULL, 0L, NULL)==-1L ||
      ! (eqstri(msg.from, usr.name) || eqstri(msg.from, usr.alias) ||
         mailflag(CFLAGM_ATTRANY)))
  {
    aborted=Msg_Change_Err();
    goto Done;
  }

  /* Determine the length of this message's control information */

  if ((ctrl_len=MsgGetCtrlLen(msgh)) != 0)
  {
    if ((ctrl_buf=malloc(ctrl_len))==NULL)
    {
      logit(mem_none);
      ctrl_len=0;
    }
    else
    {
      MsgReadMsg(msgh, NULL, 0L, 0L, NULL, ctrl_len, ctrl_buf);

      /* Remove any MSGID or INTL lines since they need to be regenerated */

      RemoveFromCtrl(ctrl_buf, "MSGID:");
      RemoveFromCtrl(ctrl_buf, "INTL");
      Strip_Trailing(ctrl_buf, '\x01');
    }
  }

  /* Now check the high-water mark, to see if the msg has been scanned  *
   * yet.                                                               */

  hwm=MsgGetHighWater(sq);

    /* Also check the REC'D bit, and the SENT bit */

  if (((mah.ma.attribs & MA_NET) && (msg.attr & MSGSENT)) ||
      (msg.attr & (MSGREAD|MSGSCANNED)) ||
      lmsg <= hwm)
  {
    RipClear();
    Display_File(0, NULL, "%sCHG_SENT", PRM(misc_path));

    /* Only allow the SysOp to edit a message which has been sent/rec'd */

    if (! mailflag(CFLAGM_ATTRANY) || GetyNAnswer(change_any,0)!=YES)
    {
      aborted=TRUE;
      goto Done;
    }
  }

  /* If the sysop is changing a message below the HWM, fix it so it can be  *
   * rescanned.                                                             */

  if (lmsg <= hwm)
    MsgSetHighWater(sq, lmsg-1);

  /* Charge user for changing msg, but only if msg has already been sent */
  
  charge_user=!!(msg.attr & MSGSENT);

  /* Strip off MSGREAD and MSGSENT bits, and add MSGLOCAL. */

  msg.attr &= ~(MSGREAD | MSGSENT | MSGSCANNED);
  msg.attr |= MSGLOCAL;

  Add_Message_Date(&msg);
  
  dest=&msg.dest;

  if (mah.ma.attribs & MA_NET)
    strcpy(netnode, Address(dest));
  else *netnode='\0';

  if (GetMsgAttr(&msg, &mah, usr.msg, lmsg, -1L)==-1)
  {
    aborted=TRUE;
    goto Done;
  }

  isareply=FALSE;
  isachange=TRUE;
  
  res=Editor(&msg, msgh, lmsg, ctrl_buf, NULL);

  isachange=FALSE;
  
  MsgCloseMsg(msgh);
  msgh=NULL;

  if (res==ABORT)
  {
    aborted=TRUE;
    goto Done;
  }
         
  if (res==LOCAL_EDIT)
    aborted=FALSE;
  else aborted=SaveMsg(&msg, NULL, FALSE, lmsg, TRUE, &mah, usr.msg, sq,
                       ctrl_buf, NULL, FALSE);

  /* Only credit the user if the original message didn't have       *
   * the MSGSENT bit set, only if the message was aborted, and      *
   * only if this is a matrix area.                                 */

  if (!charge_user && !aborted && (mah.ma.attribs & MA_NET))
  {
    if ((nf=NodeFindOpen(dest)) != NULL)
    {
      cost=nf->found.cost;

      if (cost)
      {
        Printf(balance_adjusted, cost);

        if (usr.debit >= cost)
          usr.debit -= cost;
        else usr.debit=0;
      }
      
      NodeFindClose(nf);
    }

  }

Done:

  if (ctrl_buf)
    free(ctrl_buf);
    
  if (msgh)
    MsgCloseMsg(msgh);

  if (aborted)
    Puts(msg_aborted);
  
  return (aborted ? -1 : 0);
}

static int Msg_Change_Err(void)
{
  Display_File(0, NULL, "%sCHG_NO", PRM(misc_path));
  return TRUE;
}


