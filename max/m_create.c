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

/* $Id: m_create.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=Message Section: Miscellaneous message creation functions
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"


int Goodbye_Comment(void)
{
  extern int linenum;
  int aborted, rc;
  char *where;
  XMSG msg;

  /* Make sure the area exists */

  where=*PRM(cmtarea) ? PRM(cmtarea) : zero;
  
  if (!PushMsgArea(where, 0))
    return FALSE;

  /* Now prepare the message for a log-off comment */

  Blank_Msg(&msg);
  *netnode=*orig_msgid=0;

  strcpy(msg.from, usrname);

  /* Support the area's "Style RealName" or "Style Alias" as appropriate */

  if (mah.ma.attribs & MA_REAL)
    strcpy(msg.from, usr.name);
  else if ((mah.ma.attribs & MA_ALIAS) && *usr.alias)
    strcpy(msg.from, usr.alias);

  strcpy(msg.to, PRM(sysop));

  /* DLN: can include useful information for sysop using this instead */

  Parse_Outside_Cmd(comment_fr, msg.subj);

  /* If we can have pvt. msgs in this area */

  if (mah.ma.attribs & MA_PVT)
    msg.attr |= MSGPRIVATE; /* Then set MSGPRIVATE on by default */

  if (usr.bits2 & BITS2_BORED)
    Printf(inrefto, msg.subj);

  rc=Editor(&msg, NULL, 0L, NULL, NULL);

  /* Make sure that the user doesn't change the "To:" name */
  
  strcpy(msg.to, PRM(sysop));
  
  display_line=display_col=1;
  
  if (rc==ABORT)
    aborted=TRUE;
  else if (rc==LOCAL_EDIT)
    aborted=FALSE;
  else
  {
    /* If nothing entered */

    if ((last_maxed ? num_lines : linenum)==1 && screen[1] &&
        screen[1][1]=='\0')
    {
      Free_All();
      aborted=TRUE;
    }
    else
    {
      /* Make sure that user didn't readdress msg to someone else,       *
       * through the BORED menu.                                         */

      strcpy(msg.to, PRM(sysop));
      aborted=SaveMsg(&msg, NULL, FALSE, 0, FALSE, &mah, where, sq, NULL, NULL, TRUE);
    }
  }

  if (aborted)
    Puts(msg_aborted);

  PopMsgArea();
  ForceGetMsgArea();

  return !aborted;
}

