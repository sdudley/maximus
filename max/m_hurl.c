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
static char rcs_id[]="$Id: m_hurl.c,v 1.1.1.1 2002/10/01 17:52:43 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: H)url command
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"

#define MAX_HURL 4096

static sword near DoTheHurlThing(HMSG omh, dword msgnum, byte *aname)
{
  BARINFO bi;
  HMSG nmh;
  HAREA nma;
  dword dwCtrlLen;
  char *szCtrl=NULL;
  
  XMSG msg;
  MAH to_mah={0};
  long bytes;

  char input[PATHLEN];
  char *msgb;

  sword hurl_to_same;
  long got;


  if (MsgReadMsg(omh, &msg, 0L, 0L, NULL, 0L, NULL)==-1 || !CanKillMsg(&msg))
    return -1;
  
  do
  {
    InputGets(input, which_area);

    if (eqstri(input, qmark))
      ListMsgAreas(NULL, FALSE, FALSE);
  }
  while (eqstri(input, qmark));

  if (! *input)
    return -1;

  SetAreaName(aname, input);

  if (!ReadMsgArea(ham, aname, &to_mah) ||
      !ValidMsgArea(NULL, &to_mah, VA_VAL | VA_PWD, &bi))
  {
    Puts(areadoesntexist);
    return -1;
  }

  if (AreaIsReadOnly(&to_mah))
  {
    DisposeMah(&to_mah);
    return -1;
  }

  /* If the area names match, then the area to hurl to is this one */

  hurl_to_same=eqstri(MAS(to_mah, name), MAS(mah, name));
  
  if (hurl_to_same)
    nma=sq;
  else if ((nma=MaxOpenArea(&to_mah))==NULL)
  {
    AreaError(msgapierr);
    DisposeMah(&to_mah);
    return -1;
  }

  if ((nmh=MsgOpenMsg(nma, MOPEN_CREATE, 0L))==NULL)
    Puts(hurl_cant);
  else
  {
    if ((msgb=malloc(MAX_HURL))==NULL)
      Puts(hurl_cant);
    else
    {
      Printf(hurling, msgnum, MAS(mah, name), MAS(to_mah, name));

      /* Ace the reply links */

      msg.replyto=0L;
      memset(msg.replies, '\0', sizeof(UMSGID)*MAX_REPLY);

      /* Now fix the private bit to conform to the area's flags */

      if ((mah.ma.attribs & (MA_PVT|MA_PUB))==MA_PVT)
        msg.attr |= MSGPRIVATE;
      else if ((mah.ma.attribs & (MA_PVT|MA_PUB))==MA_PUB)
        msg.attr &= ~MSGPRIVATE;


      /* Allocate memory for copying the message control information. */

      if ((dwCtrlLen=MsgGetCtrlLen(omh)) != 0)
      {
        if ((szCtrl=malloc((size_t)dwCtrlLen))==NULL)
          dwCtrlLen=0;
        else
          MsgReadMsg(omh, NULL, 0L, 0L, NULL, dwCtrlLen, szCtrl);
      }


      /* Write the message header */

      MsgWriteMsg(nmh, FALSE, &msg, NULL, 0L, MsgGetTextLen(omh),
                  dwCtrlLen, szCtrl);


      /* Free memory for control info */

      if (szCtrl)
        free(szCtrl);


      /* Now copy the message text */

      for (bytes=0L;
           (got=MsgReadMsg(omh, NULL, bytes, MAX_HURL, msgb, 0L, NULL)) > 0;
           bytes += (long)got)
      {
        MsgWriteMsg(nmh, TRUE, NULL, msgb, got, 0L, 0L, NULL);
      }

      free(msgb);
    }
    
    MsgCloseMsg(nmh);
  }

  DisposeMah(&to_mah);

  if (!hurl_to_same)
    MsgCloseArea(nma);
  
  return 0;
}



void Msg_Hurl(void)
{
  UMSGID uid;
  UMSGID last;
  HMSG omh;
  dword msgnum, newlast;
  
  byte aname[PATHLEN];
  sword ret;

  RipClear();

  if (!GetMsgNum(hurl_which, &msgnum))
    return;

  uid=MsgMsgnToUid(sq, msgnum);

  if ((omh=MsgOpenMsg(sq, MOPEN_READ, msgnum))==NULL)
  {
    Puts(hurl_cant);
    return;
  }

  ret=DoTheHurlThing(omh, msgnum, aname);

  MsgCloseMsg(omh);

  if (ret==-1)
  {
    Puts(hurl_cant);
    return;
  }

  /* Now kill the old one */

  last=MsgMsgnToUid(sq, last_msg);
  
  if (MsgKillMsg(sq, MsgUidToMsgn(sq, uid, UID_EXACT)) != 0)
  {
    Puts(cantkill);
    return;
  }

  if ((newlast=MsgUidToMsgn(sq, last, UID_PREV)) != 0)
    last_msg=newlast;


  /* Adjust the lastread ptr down, if necessary */
  
  if (last_msg > MsgHighMsg(sq))
    last_msg=MsgHighMsg(sq);
  
  /* Set the bit field, for the echo tosslog */

  if (*MAS(mah, echo_tag))
    AddToEchoToss(MAS(mah, echo_tag));

  logit(log_hurl, MAS(mah, name), UIDnum(msgnum), aname);
  return;
}
  


