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

#include <string.h>
#include "mm.h"
#include "max_msg.h"

/* Prompt the user to enter a message number */

int GetMsgNum(char *prompt, dword *pmsgnum)
{
  char input[PATHLEN];

  WhiteN();
  InputGets(input, prompt);

  if (eqstri(input, eq))
    *pmsgnum=last_msg;
  else
  {
    *pmsgnum=atoi(input);

    /* If we need to convert between UMSGIDs and message numbers */

    if (prm.flags2 & FLAG2_UMSGID)
      *pmsgnum=MsgUidToMsgn(sq, *pmsgnum, UID_EXACT);
  }

  return !!*pmsgnum;
}


