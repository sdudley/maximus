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

/* $Id: sq_uid.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stddef.h>
#include <string.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"



/* This function returns the UMSGID that will be used by the next message   *
 * to be created.                                                           */

UMSGID MAPIENTRY SquishGetNextUid(HAREA ha)
{
  return Sqd->uidNext;
}



/* This function converts the message number 'dwMsg' into a unique          *
 * message idenfitier (UMSGID).                                             */

UMSGID MAPIENTRY SquishMsgnToUid(HAREA ha, dword dwMsg)
{
  SQIDX sqi;

  if (MsgInvalidHarea(ha))
    return (UMSGID)0L;

  /* Make sure that it's a valid message number */

  if (dwMsg==0 || dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;
    return (UMSGID)0L;
  }

  if (!SidxGet(Sqd->hix, dwMsg, &sqi))
    return (UMSGID)0L;

  return sqi.umsgid;
}


/* This function converts the UMSGID in 'uid' into a real message number */

dword MAPIENTRY SquishUidToMsgn(HAREA ha, UMSGID uid, word wType)
{
  SQIDX sqi;
  dword rc=0;
  sdword stLow, stHigh, stTry;
  dword dwMax;

  if (MsgInvalidHarea(ha))
    return (UMSGID)0L;


  /* Don't let the user access msg 0 */

  if (uid==(UMSGID)0L)
  {
    msgapierr=MERR_NOENT;
    return 0L;
  }

  /* Read the index into memory */

  if (! _SquishBeginBuffer(Sqd->hix))
    return (dword)0;

  /* Set up intial bounds (inclusive) */

  dwMax=_SquishIndexSize(Sqd->hix) / sizeof(SQIDX);
  stLow=1;
  stHigh=(sdword)dwMax;
  stTry=1;

  /* Start off with a 0 umsgid */

  (void)memset(&sqi, 0, sizeof sqi);

  /* While we still have a search range... */

  while (stLow <= stHigh)
  {
    stTry=(stLow+stHigh) / 2;

    /* If we got an exact match */

    if (!SidxGet(Sqd->hix, (dword)stTry, &sqi))
      break;

    if (sqi.umsgid==uid)
    {
      rc=(dword)stTry;
      break;
    }
    else if (uid > sqi.umsgid)
      stLow=stTry+1;
    else stHigh=stTry-1;
  }


  /* If we couldn't find it exactly, try the next/prior match */

  if (!rc)
  {
    if (wType==UID_PREV)
    {
      if (sqi.umsgid < uid)
        rc=(dword)stTry;
      else if (stTry==1)
        rc=(dword)0;
      else
        rc=(dword)(stTry-1L);
    }
    else if (wType==UID_NEXT)
    {
      if (uid >= Sqd->uidNext)
        rc=0L;
      else if (sqi.umsgid > uid || stTry==(long)dwMax)
        rc=(dword)stTry;
      else
        rc=(dword)(stTry+1L);
    }
    else
      msgapierr=MERR_NOENT;
  }


  /* Free the memory used by the index */

  if (! _SquishEndBuffer(Sqd->hix))
    rc=(dword)0;

  return rc;
}



