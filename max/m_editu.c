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

/* $Id: m_editu.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

#include "prog.h"
#include "max_msg.h"

int Msg_Edit_User(void)
{
  HMSG mh;
  XMSG msg;

  if ((mh=MsgOpenMsg(sq, MOPEN_READ, last_msg))==NULL ||
      MsgReadMsg(mh, &msg, 0L, 0L, NULL, 0L, NULL)==-1)
  {
    if (mh)
      MsgCloseMsg(mh);

    return -1;
  }

  MsgCloseMsg(mh);

  User_Edit(msg.from);
  return 0;
}

