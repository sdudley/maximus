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
static char rcs_id[]="$Id: sq_misc.c,v 1.2 2003/06/05 22:54:50 wesgarland Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdio.h>
#include <ctype.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"

/* Set the "current position" pointer in a message handle */

sword MAPIENTRY SquishSetCurPos(HMSG hmsg, dword dwOfs)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return -1;

  hmsg->cur_pos=dwOfs;
  return 0;
}



/* Return the current read position within a message */

dword MAPIENTRY SquishGetCurPos(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1;

  return hmsg->cur_pos;
}


/* Return the length of the text body of this message */

dword MAPIENTRY SquishGetTextLen(HMSG hmsg)
{
  long rc;

  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;

  rc = (long)hmsg->sqhRead.msg_length - (long)XMSG_SIZE -
       (long)hmsg->sqhRead.clen;

  return (rc < 0) ? (dword)0L : (dword)rc;
}




/* Return the length of this message's control information */

dword MAPIENTRY SquishGetCtrlLen(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;

  return hmsg->sqhRead.clen;
}


/* Return the number of the high water marker */

dword MAPIENTRY SquishGetHighWater(HAREA ha)
{
  if (MsgInvalidHarea(ha))
    return (dword)-1L;

  return SquishUidToMsgn(ha, ha->high_water, UID_PREV);
}




/* Set the high water marker for this area */

sword MAPIENTRY SquishSetHighWater(HAREA ha, dword dwMsg)
{
  if (MsgInvalidHarea(ha))
    return -1;

  /* Make sure that the message exists */

  if (dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  if (!_SquishExclusiveBegin(ha))
    return -1;

  ha->high_water=SquishMsgnToUid(ha, dwMsg);

  if (!_SquishExclusiveEnd(ha))
    return -1;

  return 0;
}



/* Function to set the highest/skip message numbers for a *.SQ? base */

void MAPIENTRY SquishSetMaxMsg(HAREA ha, dword dwMaxMsgs, dword dwSkipMsgs, dword dwMaxDays)
{
  if (MsgInvalidHarea(ha))
    return;

  /* Update base only if max msg settings have changed */

  if ((dwMaxMsgs  != (dword)-1L && dwMaxMsgs  != Sqd->dwMaxMsg) ||
      (dwSkipMsgs != (dword)-1L && dwSkipMsgs != Sqd->wSkipMsg) ||
      (dwMaxDays  != (dword)-1L && dwMaxDays  != Sqd->wMaxDays))
  {
    if (!_SquishExclusiveBegin(ha))
      return;

    if (dwMaxMsgs != (dword)-1L)
      Sqd->dwMaxMsg=dwMaxMsgs;

    if (dwSkipMsgs != (dword)-1L)
      Sqd->wSkipMsg=(word)dwSkipMsgs;

    if (dwMaxDays != (dword)-1L)
      Sqd->wMaxDays=(word)dwMaxDays;

    (void)_SquishExclusiveEnd(ha);
  }
}


/* Hash function used for calculating the hashes in the .sqi file */

dword MAPIENTRY SquishHash(byte OS2FAR *f)
{
  dword hash=0, g;
  char *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + (dword)tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }
  

  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}

