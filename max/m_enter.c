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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: m_enter.c,v 1.3 2004/01/27 21:00:31 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Message Section: E)nter command
*/

#include "prog.h"
#include "max_msg.h"

int Msg_Enter(void)
{
  int aborted, rc;
  XMSG msg;

  isareply=isachange=FALSE;

  Blank_Msg(&msg);
  *netnode=*orig_msgid='\0';

  if (GetMsgAttr(&msg, &mah, usr.msg, 0L, -1L)==-1)
    aborted=TRUE;
  else if ((rc=Editor(&msg, NULL, 0L, NULL, NULL))==ABORT)
    aborted=TRUE;
  else if (rc==LOCAL_EDIT)
    aborted=FALSE;
  else
    aborted=SaveMsg(&msg, NULL, FALSE, 0L, FALSE, &mah, usr.msg, sq, NULL, NULL, FALSE);

  if (aborted)
  {
    Puts(msg_aborted);
    return -1;
  }

  return 0;
}

