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
static char rcs_id[]="$Id: m_kill.c,v 1.1.1.1 2002/10/01 17:52:44 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: K)ill command
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "trackm.h"


static void near cantkillit(void)
{
  Puts(cantkill);
  Press_ENTER();
}


void Msg_Kill(long n)
{
  HMSG msgh;

  char *ctrl;
  size_t ctrl_len;

  XMSG msg;
  char temp[PATHLEN];

  UMSGID saveid, uidnum;
  long msgnum;
  long save_mn;

  if (n != -1)
    msgnum=n;
  else
  {
    RipClear();

    WhiteN();

    InputGets(temp, kill_which);

    if (eqstri(temp, eq))
      msgnum=last_msg;
    else
    {
      msgnum=atol(temp);

      /* If we need to convert between UMSGIDs and message numbers */

      if (prm.flags2 & FLAG2_UMSGID)
        msgnum=MsgUidToMsgn(sq, msgnum, UID_EXACT);
    }
  }

  if (!msgnum)
    return;
  
  if ((msgh=MsgOpenMsg(sq, MOPEN_READ, msgnum))==NULL)
  {
    cantkillit();
    return;
  }

  ctrl=NULL;
  ctrl_len=MsgGetCtrlLen(msgh);

  if (ctrl_len && (ctrl=malloc(ctrl_len))==NULL)
    ctrl_len=0;

  if (MsgReadMsg(msgh, &msg, 0L, 0L, NULL, ctrl_len, ctrl)==-1 ||
      !CanKillMsg(&msg))
  {
    cantkillit();

    if (ctrl)
      free(ctrl);

    if (msgh)
      MsgCloseMsg(msgh);

    return;
  }

  MsgCloseMsg(msgh);

#ifdef MAX_TRACKER
  /* Delete this message from the tracking database */

  TrackKillMsg(ctrl);

#endif

  /* Delete any files attached to this message */

  Msg_AttachKill(&msg, ctrl, !!(mah.ma.attribs & MA_NET));

  free(ctrl);

  /* Make sure that our lastread pointer gets saved appropriately */

  saveid=MsgMsgnToUid(sq, last_msg);
  save_mn=last_msg;

  /* Save the UID before the message is killed */

  uidnum=UIDnum(msgnum);

  if (MsgKillMsg(sq, msgnum) != 0)
  {
    Puts(cantkill);
    Press_ENTER();
    return;
  }

  /* Now set it to the right value */

  last_msg=MsgUidToMsgn(sq, saveid, UID_PREV);

  /* If we just killed the message we're on, fix it appropriately! */

  if ((sdword)last_msg < 0)
    last_msg=save_mn;

  /* Too high */

  if (last_msg > MsgHighMsg(sq))
    last_msg=MsgHighMsg(sq);

  logit(log_kill, uidnum, usr.msg);
  Printf(kill_done, uidnum);
}



/* Check to see if the user has enough privs to kill a message. */

int CanKillMsg(XMSG *msg)
{
  return (ToOrFromUs(msg) || GEPriv(usr.priv, prm.pvt_priv) || mailflag(CFLAGM_PVT));
}


