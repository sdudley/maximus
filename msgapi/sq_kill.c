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
static char rcs_id[]="$Id: sq_kill.c,v 1.1.1.1 2002/10/01 17:54:31 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <assert.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"


/* Kill the specified message number.                                       *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static sword _SquishKill(HAREA ha, dword dwMsg, SQHDR *psqh, FOFS fo)
{
  assert(Sqd->fHaveExclusive);


  /* Link the existing messages over this one */

  if (psqh->prev_frame)
    if (!_SquishSetFrameNext(ha, psqh->prev_frame, psqh->next_frame))
      return FALSE;

  if (psqh->next_frame)
    if (!_SquishSetFramePrev(ha, psqh->next_frame, psqh->prev_frame))
      return FALSE;


  /* Delete this message from the index file */

  if (!_SquishRemoveIndexEntry(Sqd->hix, dwMsg, NULL, psqh, TRUE))
    return FALSE;


  /* Finally, add the freed message to the free frame list */

  return (sword)_SquishInsertFreeChain(ha, fo, psqh);
}



/* This function is used to delete a message from a Squish message base */

sword MAPIENTRY SquishKillMsg(HAREA ha, dword dwMsg)
{
  SQHDR sqh;
  sword rc;
  FOFS fo;

  /* Validate parameters */

  if (MsgInvalidHarea(ha))
    return -1;


  /* Make sure that the message actually exists */

  if (dwMsg==0 || dwMsg > ha->num_msg)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  /* Get the offset of the frame to delete */

  if ((fo=_SquishGetFrameOfs(ha, dwMsg))==NULL_FRAME)
    return -1;


  /* Read that into memory */

  if (!_SquishReadHdr(ha, fo, &sqh))
    return -1;


  /* Now get exclusive access for the delete operation */

  if (!_SquishExclusiveBegin(ha))
    return FALSE;

  /* Let _SquishKill to the dirty work */

  rc=_SquishKill(ha, dwMsg, &sqh, fo);

  /* Let go of the base */

  if (!_SquishExclusiveEnd(ha))
    rc=FALSE;

  return rc ? 0 : -1;
}

