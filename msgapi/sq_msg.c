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
static char rcs_id[]="$Id: sq_msg.c,v 1.1 2002/10/01 17:54:32 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"


/* Allocate a new message handle */

static HMSG near NewHmsg(HAREA ha, word wMode)
{
  HMSG hmsg;

  /* Allocate memory for the message handle */

  if ((hmsg=palloc(sizeof *hmsg))==NULL)
    return NULL;

  (void)memset(hmsg, 0, sizeof *hmsg);

  /* Initialize the handle to the standard defaults */

  hmsg->ha=ha;
  hmsg->id=MSGH_ID;
  hmsg->bytes_written=0;
  hmsg->cur_pos=0;
  hmsg->foRead=NULL_FRAME;
  hmsg->foWrite=NULL_FRAME;
  hmsg->wMode=wMode;
  hmsg->fDiskErr=FALSE;
  hmsg->dwMsg=0L;
  hmsg->uidUs=(UMSGID)0L;
  hmsg->hmsgNext=NULL;

  return hmsg;
}





/* Returns TRUE if this is a valid message which can be read */

static unsigned near _SquishHeaderValidRead(HAREA ha, SQHDR *psqh)
{
  if (psqh->next_frame > Sqd->foEnd ||
      psqh->prev_frame > Sqd->foEnd)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  /* Now make sure that it's okay to read from this frame type */

  if (psqh->frame_type==FRAME_NORMAL)
    return TRUE;
  else if (psqh->frame_type==FRAME_FREE)
  {
    msgapierr=MERR_BADMSG;
    return FALSE;
  }
  else if (psqh->frame_type==FRAME_UPDATE)
  {
    msgapierr=MERR_SHARE;
    return FALSE;
  }
  else
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }
}



/* Translate an absolute message number into a relative one */

static dword near _SquishTranslateNum(HAREA ha, dword dwMsg)
{
  if (dwMsg==MSGNUM_CUR)
    return ha->cur_msg;
  else if (dwMsg==MSGNUM_PREV)
    return ha->cur_msg-1;
  else if (dwMsg==MSGNUM_NEXT)
    return ha->cur_msg+1;
  else return dwMsg;
}


/* Set the HAREA struct to show this as the current message being read */

static unsigned near _SquishSetCurFrameRead(HMSG hmsg, dword dwMsg, FOFS foCur, SQHDR *psqh)
{
  hmsg->ha->cur_msg=dwMsg;
  HSqd->foCur=foCur;
  HSqd->foPrev=psqh->prev_frame;
  HSqd->foNext=psqh->next_frame;

  hmsg->foRead=foCur;
  hmsg->sqhRead=*psqh;

  return TRUE;
}



/* Open an existing message */

static unsigned near _SquishOpenMsgExisting(HMSG hmsg, dword dwMsg)
{
  SQHDR sqh;
  FOFS foMsg;

  /* If the message number is invalid, return an appropriate error */

  if (dwMsg==0 || dwMsg > hmsg->ha->num_msg)
  {
    /* If the user tries to go to message zero, reset pointers appropriately */

    if (dwMsg==0)
    {
      HSqd->foPrev=NULL_FRAME;
      HSqd->foCur=NULL_FRAME;
      HSqd->foNext=HSqd->foFirst;
      hmsg->ha->cur_msg=0;
    }

    msgapierr=MERR_NOENT;
    return FALSE;
  }

  /* Store the message number in the message handle */

  hmsg->dwMsg=dwMsg;

  /* Get the frame offset for this message */

  if ((foMsg=_SquishGetFrameOfs(hmsg->ha, dwMsg))==NULL_FRAME)
    return FALSE;

  /* Read the frame header for this message and make sure that it's okay    *
   * to read from this message.                                             */

  if (!_SquishReadHdr(hmsg->ha, foMsg, &sqh) ||
      !_SquishHeaderValidRead(hmsg->ha, &sqh))
  {
    return FALSE;
  }

  /* Adjust our area header to show this as the current message */

  return _SquishSetCurFrameRead(hmsg, dwMsg, foMsg, &sqh);
}



/* We are creating a new message on top of an old one.  We have not         *
 * written anything yet, but we need to indicate to other tasks that this   *
 * mesage is in the process of being written.  To do this:                  *
 *                                                                          *
 * 1) Invalidate the index record for this message.                         *
 *                                                                          *
 * 2) Set a flag in the SQHDR indicating that the message is currently      *
 *    being processed.                                                      *
 *                                                                          *
 * 3) Read in the SQHDR for the frame being killed, so that SquishWriteMsg  *
 *    can use it at a later point in time.                                  *
 *                                                                          *
 * This function assumes that we have exclusive access to the base.         */

static unsigned near _SquishBlankOldMsg(HMSG hmsg, dword dwMsg)
{
  SQIDX sqi;

  assert(HSqd->fHaveExclusive);

  if (! SidxGet(HSqd->hix, dwMsg, &sqi))
    return FALSE;


  /* Make sure that this frame contains a valid message */

  if (sqi.ofs==NULL_FRAME)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }


  /* Save the location of the SQHDR */

  hmsg->foRead=sqi.ofs;

  /* Now try to read in the message header */

  if (! _SquishReadHdr(hmsg->ha, hmsg->foRead, &hmsg->sqhRead))
    return FALSE;


  /* Two tasks cannot update the message at the same time! */

  if (hmsg->sqhRead.frame_type==FRAME_UPDATE)
  {
    msgapierr=MERR_SHARE;
    return FALSE;
  }


  /* Invalidate this index, but leave the UMSGID alone */

  sqi.ofs=NULL_FRAME;
  sqi.hash=(dword)-1L;

  /* Save a copy of the UMSGID so we know which message we are updating */

  hmsg->uidUs=sqi.umsgid;

  /* Write the record back to disk */

  if (! SidxPut(HSqd->hix, dwMsg, &sqi))
    return FALSE;


  hmsg->sqhRead.frame_type=FRAME_UPDATE;

  if (! _SquishWriteHdr(hmsg->ha, hmsg->foRead, &hmsg->sqhRead))
    return FALSE;

  return TRUE;
}



/* We are creating a new message and appending it to the end of the         *
 * Squish base.  In this case, we only need to initialize the entry in      *
 * the Squish index file.                                                   *
 *                                                                          *
 * This function assumes that we have exclusive access to the base.         */

static unsigned near _SquishBlankNewMsg(HMSG hmsg)
{
  SQIDX sqi;

  assert(HSqd->fHaveExclusive);

  sqi.ofs=NULL_FRAME;
  sqi.hash=(dword)-1L;
  sqi.umsgid=HSqd->uidNext++;
  hmsg->uidUs=sqi.umsgid;

  return (unsigned)SidxPut(HSqd->hix, hmsg->dwMsg, &sqi);
}


/* This function removes as many messages as necessary (and places them     *
 * on the free chain) such that we have no more than maxmsgs messages.      */

static unsigned near _SquishReduceMaxInternal(HAREA ha,
                                              dword *pdwDeleted,
                                              FOFS *pfoFirst,
                                              FOFS *pfoFirstPrior)
{
  SQIDX sqi;
  SQHDR sqh;

  /* Delete messages while we can... */

  while (ha->num_msg >= Sqd->dwMaxMsg)
  {
    ha->num_msg--;
    ha->high_msg--;

    if (! SidxGet(Sqd->hix, Sqd->wSkipMsg+1, &sqi) ||
        ! _SquishReadHdr(ha, sqi.ofs, &sqh) ||
        ! _SquishInsertFreeChain(ha, sqi.ofs, &sqh))
    {
      return FALSE;
    }

    /* Record the new starting point for the beginning of the base */

    *pfoFirst=sqh.next_frame;

    if (*pfoFirstPrior==NULL_FRAME)
      *pfoFirstPrior=sqh.prev_frame;

    /* Delete this message from the index */

    if (!_SquishRemoveIndexEntry(Sqd->hix, Sqd->wSkipMsg+1L, NULL, &sqh, FALSE))
      return FALSE;

    (*pdwDeleted)++;
  }

  return TRUE;
}




/* This function adjusts our pointers after a massive deletion by           *
 * _SquishReduceMaxInternal.                                                */

static unsigned near _SquishReduceMaxPointers(HAREA ha, FOFS foFirst,
                                              dword dwDeleted,
                                              FOFS foFirstPrior)
{
  unsigned rc=TRUE;

  /* Now adjust the 'first message' pointer.  If we have no skip_msgs,      *
   * just set the pointer for the beginning of the area.  If we do have     *
   * skip_msgs, we will have to update the 'next' pointer of the highest    *
   * skip_msg to pointer over the messages that we just deleted.            */

  if (Sqd->wSkipMsg==0)
  {
    Sqd->foFirst=foFirst;
    foFirstPrior=NULL_FRAME;
  }
  else
  {
    /* Link up the skipmsgs message and the one after it */

    if (foFirstPrior)
      if (! _SquishSetFrameNext(ha, foFirstPrior, foFirst))
        rc=FALSE;
  }

  if (foFirst)
    if (! _SquishSetFramePrev(ha, foFirst, foFirstPrior))
      rc=FALSE;

  /* If we were just outside the deleted range, adjust appropriately */

  if (ha->cur_msg==(dword)Sqd->wSkipMsg+dwDeleted+1)
  {
    /* Adjust our 'previous' pointer, if we come just after the deleted     *
     * range.                                                               */

    if (Sqd->wSkipMsg==0 || ha->num_msg==0)
      Sqd->foPrev=NULL_FRAME;
    else
      Sqd->foPrev=_SquishGetFrameOfs(ha, (dword)Sqd->wSkipMsg);
  }
  else if (ha->cur_msg==(dword)Sqd->wSkipMsg)
  {
    /* If we come just before the deleted range, adjust our 'next' ptr */

    Sqd->foNext=foFirst;
  }


  /* If the current message number was greater than skip_msg, we will       *
   * need to adjust our pointers.                                           */

  if (ha->cur_msg > (dword)Sqd->wSkipMsg)
  {
    if (ha->cur_msg <= (dword)Sqd->wSkipMsg+dwDeleted)
    {
      SQHDR sqh;
      FOFS fo;

      /* We were inside the range of messages that was deleted.  To handle  *
       * this, simply set our pointer to the first message outside of that  *
       * range.                                                             */

      if (Sqd->wSkipMsg &&
          (fo=_SquishGetFrameOfs(ha, (dword)Sqd->wSkipMsg)) != NULL_FRAME &&
          _SquishReadHdr(ha, fo, &sqh))
      {
        Sqd->foCur=fo;
        Sqd->foPrev=sqh.prev_frame;
        Sqd->foNext=sqh.next_frame;
        ha->cur_msg=Sqd->wSkipMsg;
      }
      else
      {
        Sqd->foNext=Sqd->foFirst;
        Sqd->foCur=NULL_FRAME;
        Sqd->foPrev=NULL_FRAME;
        ha->cur_msg=0;
      }
    }
    else
    {
      /* We were above the deleted range, so we will have to decrement our  *
       * message number.                                                    */

      ha->cur_msg -= dwDeleted;
    }
  }

  /* Adjust the HWM, if necessary */

  if (ha->high_water >= Sqd->wSkipMsg)
  {
    long lNewHWM = (long)ha->high_water - (long)dwDeleted;

    if (lNewHWM < (long)Sqd->wSkipMsg)
      ha->high_water = Sqd->wSkipMsg;
    else ha->high_water=(dword)lNewHWM;
  }


  /* If we deleted all of the messages in the area, clean up */

  if (ha->num_msg==0)
  {
    Sqd->foFirst=NULL_FRAME;
    Sqd->foLast=NULL_FRAME;
    Sqd->foNext=NULL_FRAME;
    Sqd->foPrev=NULL_FRAME;
    Sqd->foCur=NULL_FRAME;
  }

  return rc;
}




/* Delete enough messages in this area so that we fall below the            *
 * dwMaxMsg limit.                                                          *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishReduceMaxMsgs(HAREA ha)
{
  FOFS foFirstPrior=NULL_FRAME;
  FOFS foFirst=NULL_FRAME;
  dword dwDeleted=0;
  unsigned rc=TRUE;

  assert(Sqd->fHaveExclusive);


  /* If we don't have too many messages, just return */

  if (!Sqd->dwMaxMsg ||
      ha->num_msg < Sqd->dwMaxMsg ||
      ha->num_msg <= (dword)Sqd->wSkipMsg)
  {
    return TRUE;
  }


  /* Read the index into memory */

  if (! _SquishBeginBuffer(Sqd->hix))
    return FALSE;

  /* Move all of the messages to the free list */

  if (!_SquishReduceMaxInternal(ha, &dwDeleted, &foFirst, &foFirstPrior))
    rc=FALSE;

  /* Make sure that our pointers are okay */

  if (!_SquishReduceMaxPointers(ha, foFirst, dwDeleted, foFirstPrior))
    rc=FALSE;

  /* Write the index back */

  if (!_SquishEndBuffer(Sqd->hix))
    rc=FALSE;

  return rc;
}



/* Create a new Squish message, possibly overwriting an old one             *
 * (if dwMsg==0).                                                           */

static unsigned near _SquishOpenMsgCreate(HMSG hmsg, dword dwMsg)
{
  unsigned rc=TRUE;

  /* If we are creating a completely-new message, we need to adjust the     *
   * header and index file IMMEDIATELY to show that we want this            *
   * new message number.                                                    */

  if (! _SquishExclusiveBegin(hmsg->ha))
    return FALSE;

  /* If we are creating a new message, make sure that dwMsg is zero! */

  if (dwMsg > hmsg->ha->num_msg)
    dwMsg=0;

  /* Make sure that we don't overrun the max_msgs limit! */

  if (dwMsg==0)
    rc=_SquishReduceMaxMsgs(hmsg->ha);

  /* Set our message number */

  hmsg->dwMsg=dwMsg ? dwMsg : hmsg->ha->num_msg+1;

  /* Now fix up the index and data files to indicate that message creation  *
   * is in process.                                                         */

  if (rc)
  {
    if (dwMsg)
      rc=_SquishBlankOldMsg(hmsg, dwMsg);
    else rc=_SquishBlankNewMsg(hmsg);
  }

  /* If we are creating a message, increment the total number of messages */

  if (rc && !dwMsg)
  {
    hmsg->ha->num_msg++;
    hmsg->ha->high_msg++;
  }

  /* End exclusive access */

  if (! _SquishExclusiveEnd(hmsg->ha))
    rc=FALSE;

  return rc;
}




/* Open a Squish message */

HMSG MAPIENTRY SquishOpenMsg(HAREA ha, word wMode, dword dwMsg)
{
  HMSG hmsg;
  unsigned fOpened=FALSE;

  if (MsgInvalidHarea(ha))
    return NULL;

  /* Allocate a handle for this message */

  if ((hmsg=NewHmsg(ha, wMode))==NULL)
    return NULL;

  /* Translate dwMsg into a real message number, if necessary */

  dwMsg=_SquishTranslateNum(hmsg->ha, dwMsg);

  /* Create a new message, or open an existing message, as specified */

  if (wMode==MOPEN_CREATE)
    fOpened=_SquishOpenMsgCreate(hmsg, dwMsg);
  else
    fOpened=_SquishOpenMsgExisting(hmsg, dwMsg);

  /* If the open succeeded, add this to the list of open msgs for this area */

  if (fOpened)
  {
    hmsg->hmsgNext=Sqd->hmsgOpen;
    Sqd->hmsgOpen=hmsg;
  }
  else
  {
    /* Otherwise, free memory and get out */

    pfree(hmsg);
    hmsg=NULL;
  }

  return hmsg;
}

/* This function undoes what SquishOpenMsg did when creating a new          *
 * message.  This is only called if the write was not completed.            */

static unsigned near _SquishCloseUndoWrite(HMSG hmsg)
{
  if (! _SquishExclusiveBegin(hmsg->ha))
    return FALSE;

  /* Check again, just in case something happened on another node */

  if (hmsg->dwMsg==hmsg->ha->num_msg)
  {
    hmsg->ha->num_msg--;
    hmsg->ha->high_msg--;
  }

  if (! _SquishExclusiveEnd(hmsg->ha))
    return FALSE;

  return TRUE;
}


/* Remove the message 'hmsg' from the linked list of open messages */

static unsigned near _SquishCloseRemoveList(HMSG hmsg)
{
  HMSG hm=HSqd->hmsgOpen;

  if (!hm)
  {
    msgapierr=MERR_BADA;
    return FALSE;
  }

  /* If our message was at the head of the list, just adjust main ptr */

  if (HSqd->hmsgOpen==hmsg)
  {
    HSqd->hmsgOpen=hmsg->hmsgNext;
    return TRUE;
  }

  /* Otherwise, try to find this message in the linked list of msgs */

  while (hm)
  {
    /* If we found us, just skip the list over to the next msg */

    if (hm->hmsgNext==hmsg)
    {
      hm->hmsgNext=hmsg->hmsgNext;
      return TRUE;
    }

    hm=hm->hmsgNext;
  }

  msgapierr=MERR_BADA;
  return FALSE;
}



/* Close an open message handle */

sword MAPIENTRY SquishCloseMsg(HMSG hmsg)
{
  if (MsgInvalidHmsg(hmsg))
    return -1;


  /* If we allocated a new number for this message, but we did not use it... */

  if (hmsg->wMode==MOPEN_CREATE && !hmsg->fWritten &&
      hmsg->dwMsg==hmsg->ha->num_msg)
  {
    if (!_SquishCloseUndoWrite(hmsg))
      return -1;
  }

  /* Remove this msg from the list of open msgs for this area */

  (void)_SquishCloseRemoveList(hmsg);

  /* Reset the ID so that our functions will not accept the freed hmsg */

  hmsg->id=0L;

  /* Deallocate memory and return to caller */

  pfree(hmsg);
  return 0;
}


