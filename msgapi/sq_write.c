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
static char rcs_id[]="$Id: sq_write.c,v 1.1.1.1 2002/10/01 17:54:34 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"




/* This function searches the list of free frames to find one which is      *
 * large enough to hold a message of size dwLen.                            *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishProbeFreeChain(HAREA ha, dword dwLen, FOFS *pfo,
                                           SQHDR *psqh, dword *pdwFrameLen)
{
  FOFS foThis, foLast;

  assert(Sqd->fHaveExclusive);


  /* Assume that we haven't found anything */

  *pfo=NULL_FRAME;
  *pdwFrameLen=0L;


  foLast=NULL_FRAME;

  /* Look through all of the entries in the free chain */

  for (foThis=Sqd->foFree;
       foThis != NULL_FRAME;
       foLast=foThis, foThis=psqh->next_frame)
  {
    /* Try to read the header at this point in the list */

    if (!_SquishReadHdr(ha, foThis, psqh))
      return FALSE;


    /* Verify that this is a valid frame and that we aren't going around    *
     * in circles.                                                          */

    if (psqh->frame_type != FRAME_FREE ||
        foLast != psqh->prev_frame ||
        psqh->next_frame==foThis)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

    /* If the frame is long enough, we can use it */

    if (psqh->frame_length >= dwLen)
    {
      *pdwFrameLen=psqh->frame_length;
      *pfo=foThis;
      break;
    }
  }

  return TRUE;
}



/* This function removes the frame at offset 'fo' from the free chain.      *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishRemoveFreeChain(HAREA ha, FOFS fo, SQHDR *psqh)
{
  assert(Sqd->fHaveExclusive);

  /* Validate that the frames which pretend to be list heads/tails are      *
   * actually what they seem.                                               */

  if ((psqh->prev_frame==NULL_FRAME && fo != Sqd->foFree) ||
      (psqh->next_frame==NULL_FRAME && fo != Sqd->foLastFree))
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }


  /* If there is a frame before this one, set it to skip over this frame */

  if (psqh->prev_frame)
    if (!_SquishSetFrameNext(ha, psqh->prev_frame, psqh->next_frame))
      return FALSE;


  /* Do the same for the other side of the linked list */

  if (psqh->next_frame)
    if (!_SquishSetFramePrev(ha, psqh->next_frame, psqh->prev_frame))
      return FALSE;


  /* Now update the head and tail pointers for the free message list */

  if (Sqd->foFree==fo)
    Sqd->foFree=psqh->next_frame;

  if (Sqd->foLastFree==fo)
    Sqd->foLastFree=psqh->prev_frame;

  return TRUE;
}



/* Allocate a frame of length dwLen at end-of-file, and store result in pfo.*
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish        *
 * base.                                                                    */

static unsigned near _SquishGetFrameEOF(HAREA ha, FOFS *pfo, dword dwLen)
{
  char nul=0;
  long ofs;

  assert(Sqd->fHaveExclusive);

  /* Find the last byte that we will have to write for this frame, to       *
   * ensure that we have enough disk space to write this message.           */

  ofs=Sqd->foEnd + (long)Sqd->cbSqhdr + (long)dwLen - 1L;

  /* Now try to write it, just to make sure... */

  if (lseek(Sqd->sfd, ofs, SEEK_SET) != ofs ||
      write(Sqd->sfd, &nul, 1) != 1)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  /* We got it!  So, update the frame offset, and modify the end-of-file    *
   * offset appropriately.                                                  */

  *pfo=Sqd->foEnd;
  Sqd->foEnd=ofs+1;
  return TRUE;
}



/* This function searches the free chain to find a message of at least the  *
 * specified size.  If that fails, we allocate a new frame at EOF.          *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishGetNewFrame(HMSG hmsg, dword dwLen, FOFS *pfoNew,
                                        dword *pdwFrameLen)
{
  SQHDR sqh;
  FOFS fo;

  assert(HSqd->fHaveExclusive);

  /* Assume we got a new frame */

  *pdwFrameLen=0L;

  /* Check the free chain */

  if (!_SquishProbeFreeChain(hmsg->ha, dwLen, &fo, &sqh, pdwFrameLen))
    return FALSE;

  /* If we got a frame from the free chain, remove it */

  if (fo)
  {
    if (!_SquishRemoveFreeChain(hmsg->ha, fo, &sqh))
      return FALSE;

    *pfoNew=fo;
    return TRUE;
  }

  /* Nothing in the free chain, so we will add a new frame at EOF, and      *
   * make it as long as necessary.                                          */

  *pdwFrameLen=0;
  return _SquishGetFrameEOF(hmsg->ha, pfoNew, dwLen);
}



/* Given some blank space at offset hmsg->foWrite, we are to create a new   *
 * message frame there and link it into the message chain.  If hmsg->foRead *
 * is blank, just append the message to the end of the chain.  However,     *
 * if foRead is NOT blank, we should set the new frame so that it will      *
 * appear between hmsg->sqhRead.prev and hmsg->sqhRead.next, as we are      *
 * overwriting an old message.                                              *
 *                                                                          *
 * At exit, the header that we just created will be left in hmsg->sqhWrite. *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishLinkMessageFrame(HMSG hmsg, dword dwTotal,
                                             dword dwCtrlLen, dword dwFrameLen)
{
  assert(HSqd->fHaveExclusive);
  assert(dwFrameLen==0 || dwFrameLen >= dwTotal);

  /* Fill out default values for the frame header */

  hmsg->sqhWrite.id=SQHDRID;
  hmsg->sqhWrite.frame_length=dwFrameLen ? dwFrameLen : dwTotal;
  hmsg->sqhWrite.msg_length=dwTotal;
  hmsg->sqhWrite.clen=dwCtrlLen;
  hmsg->sqhWrite.frame_type=FRAME_NORMAL;
  hmsg->sqhWrite.rsvd=0;


  /* If we have to link it into the middle of the base */

  if (hmsg->foRead)
  {
    /* Set the links for our own frame */

    hmsg->sqhWrite.prev_frame=hmsg->sqhRead.prev_frame;
    hmsg->sqhWrite.next_frame=hmsg->sqhRead.next_frame;

    /* Fix the link for the message after us, if any */

    if (hmsg->sqhWrite.next_frame != NULL_FRAME)
    {
      if (!_SquishSetFramePrev(hmsg->ha, hmsg->sqhWrite.next_frame,
                               hmsg->foWrite))
      {
        return FALSE;
      }
    }
  }
  else
  {
    /* Append this frame at the end of the list */

    hmsg->sqhWrite.prev_frame=HSqd->foLast;
    hmsg->sqhWrite.next_frame=NULL_FRAME;
  }

  /* Set the links for the message before us */

  if (hmsg->sqhWrite.prev_frame != NULL_FRAME)
  {
    if (!_SquishSetFrameNext(hmsg->ha, hmsg->sqhWrite.prev_frame,
                             hmsg->foWrite))
    {
      return FALSE;
    }
  }


  /* If this has just become the head of this list, fix pointers... */

  if (hmsg->sqhWrite.prev_frame==NULL_FRAME)
  {
    /* Sanity check: if we are becoming the beginning frame, we must        *
     * have either replaced the first message, or we are simply creating    *
     * the first message in a Squish base.                                  */

    assert(hmsg->foRead==HSqd->foFirst || HSqd->foFirst==NULL_FRAME);
    HSqd->foFirst=hmsg->foWrite;
  }


  /* If this has just become the tail of the list, fix pointers... */

  if (hmsg->sqhWrite.next_frame==NULL_FRAME)
  {
    /* Sanity check: if we are becoming the EOF frame, we must have         *
     * either added a new message, or replaced the old end-of-file message  */

    if (hmsg->foRead)
      assert(hmsg->foRead==HSqd->foLast);

    HSqd->foLast=hmsg->foWrite;
  }


  /* If we wrote the message before or after the lastread msg, update ptrs */

  if (hmsg->dwMsg==hmsg->ha->cur_msg)
    HSqd->foCur=hmsg->foWrite;
  else if (hmsg->dwMsg==hmsg->ha->cur_msg+1)
    HSqd->foNext=hmsg->foWrite;
  else if (hmsg->dwMsg==hmsg->ha->cur_msg-1)
    HSqd->foPrev=hmsg->foWrite;


  /* Now update the frame for the message that we just wrote */

  return _SquishWriteHdr(hmsg->ha, hmsg->foWrite, &hmsg->sqhWrite);
}



/* Find a frame within the Squish file for writing this message, then       *
 * allocate it.                                                             *
 *                                                                          *
 * This function assumes that we have exclusive access to the Squish base.  */

static unsigned near _SquishGetWriteFrame(HMSG hmsg, dword dwTxtTotal,
                                          dword dwCtrlLen)
{
  dword dwTotal=(dword)sizeof(XMSG)+dwTxtTotal+dwCtrlLen;
  dword dwFrameLen=0;

  assert(HSqd->fHaveExclusive);

  /* If we're writing over an existing message, verify that the             *
   * total and control lengths are not more than what we have already.      */

  if (hmsg->wMode==MOPEN_RW || hmsg->wMode==MOPEN_WRITE)
  {
    if (dwTotal > hmsg->sqhRead.msg_length)
    {
      msgapierr=MERR_TOOBIG;
      return FALSE;
    }

    /* Copy the read-mode parameters, since we are now writing over this    *
     * message at the specified offset.                                     */

    hmsg->foWrite=hmsg->foRead;
    hmsg->sqhWrite=hmsg->sqhRead;
  }
  else if (hmsg->wMode==MOPEN_CREATE)
  {
    /* First, if we are replacing an existing message, release its frame    *
     * to the free chain.                                                   */

    if (hmsg->foRead)
    {
      if (! _SquishInsertFreeChain(hmsg->ha, hmsg->foRead, &hmsg->sqhRead))
        return FALSE;
    }

    /* We are creating a new message, so we just have to find a frame       *
     * big enough to hold this message.                                     */

    if (! _SquishGetNewFrame(hmsg, dwTotal, &hmsg->foWrite, &dwFrameLen))
    {
      /* That failed, so we can't write the message.  However, if we were   *
       * trying to replace an existing message, we have added that message  *
       * to the free chain, but we have NOT linked the other messages       *
       * to skip over it.  The only recourse is to do the linking and       *
       * remove it from the index file (effectively deleting that message)  */


      if (hmsg->foRead)
      {
        /* Link the old messages over this one */

        (void)_SquishSetFrameNext(hmsg->ha, hmsg->sqhRead.prev_frame,
                                  hmsg->sqhRead.next_frame);

        (void)_SquishSetFramePrev(hmsg->ha, hmsg->sqhRead.next_frame,
                                  hmsg->sqhRead.prev_frame);

        /* Now remove the old one from the index */

        (void)_SquishRemoveIndexEntry(HSqd->hix, hmsg->dwMsg, NULL,
                                      &hmsg->sqhRead, TRUE);
      }

      hmsg->foWrite=NULL_FRAME;
      return FALSE;
    }

    /* Now link this new message frame into our list of messages to write */

    if (!_SquishLinkMessageFrame(hmsg, dwTotal, dwCtrlLen, dwFrameLen))
    {
      hmsg->foWrite=NULL_FRAME;
      return FALSE;
    }
  }

  hmsg->dwWritePos=0L;
  return TRUE;
}



/* Write the XMSG header to the Squish file */

static unsigned near _SquishWriteXmsg(HMSG hmsg, PXMSG pxm, dword *pdwOfs)
{
  XMSG xmsg;
  long ofs=hmsg->foWrite + HSqd->cbSqhdr;

  /* If we don't know our UMSGID, retrieve it from the index file */

  if (!hmsg->uidUs)
  {
    SQIDX sqi;

    /*if (_SquishReadIndexRecord(hmsg->ha, hmsg->dwMsg, &sqi))*/
    if (SidxGet(HSqd->hix, hmsg->dwMsg, &sqi))
      hmsg->uidUs=sqi.umsgid;
  }

  /* Make local copy of XMSG struct */

  xmsg=*pxm;

  /* KLUDGE: Store the UMSGID in the message header so that SQFIX can       *
   * use it to restore the index file, if necessary.  However, if the       *
   * umsgid is not known, make sure that the MSGUID bit is not set.         */

  if (!hmsg->uidUs)
  {
    xmsg.attr &= ~MSGUID;
    xmsg.umsgid=(UMSGID)0L;
  }
  else
  {
    xmsg.attr |= MSGUID;
    xmsg.umsgid=hmsg->uidUs;
  }

  /* Write the message to disk */

  if (ofs != (long)*pdwOfs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }

  if (write(HSqd->sfd, (char *)&xmsg, sizeof xmsg) != (int)sizeof xmsg)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  *pdwOfs = (dword)ofs + (dword)sizeof xmsg;

  return TRUE;
}



/* Write the control information to the Squish file */

static unsigned near _SquishWriteCtrl(HMSG hmsg, byte OS2FAR *szCtrl,
                                      dword dwCtrlLen, dword *pdwOfs)
{
  long ofs;

  /* We can only write control information on the first pass */

  if (hmsg->fWritten)
    return TRUE;


  /* Make sure that the control information is not longer than
   * what we wrote the first time, if we're updating a message.
   */

  if (dwCtrlLen > hmsg->sqhWrite.clen)
    dwCtrlLen=hmsg->sqhWrite.clen;


  /* Make sure that we don't try to do a write with len==0 */

  if (!dwCtrlLen)
    return TRUE;


  /* Now seek to the appropriate offset */

  ofs=hmsg->foWrite + HSqd->cbSqhdr + sizeof(XMSG);


  /* Write the control information at the appropriate offset */

  if (ofs != (long)*pdwOfs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }

  if (write(HSqd->sfd, (char *)szCtrl, (unsigned)dwCtrlLen) != (int)dwCtrlLen)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  *pdwOfs=(dword)ofs + dwCtrlLen;

  return TRUE;
}



/* Now write the message body, appending if necessary */

static unsigned near _SquishWriteTxt(HMSG hmsg, unsigned fAppend,
                                     byte OS2FAR *szTxt, dword dwTxtLen,
                                     dword *pdwOfs)
{
  dword dwMaxWrite;
  long ofs;

  /* Figure out where to start writing text */

  ofs=hmsg->foWrite + (long)HSqd->cbSqhdr + (long)sizeof(XMSG)
                    + (long)hmsg->sqhWrite.clen;

  /* Figure out how much we can write, at most */

  dwMaxWrite=hmsg->sqhWrite.msg_length - sizeof(XMSG) - hmsg->sqhWrite.clen;

  /* If we're appending to existing text, make sure that we adjust properly */

  if (fAppend)
  {
    ofs += (long)hmsg->dwWritePos;
    dwMaxWrite -= hmsg->dwWritePos;
  }

  dwMaxWrite=min(dwMaxWrite, dwTxtLen);

  /* Now seek to the right spot and write the message text */

  if (*pdwOfs != (dword)ofs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }

  if (write(HSqd->sfd, (char *)szTxt, (unsigned)dwMaxWrite) != (int)dwMaxWrite)
  {
    msgapierr=MERR_NODS;
    return FALSE;
  }

  *pdwOfs = (dword)ofs + dwMaxWrite;

  /* Update the pointer in case we append to this msg in the future */

  hmsg->dwWritePos += dwMaxWrite;
  return TRUE;
}



/* Update the index at offset hmsg->dwMsg with the information given in     *
 * in the XMSG structure.                                                   */

static unsigned near _SquishUpdateIndex(HMSG hmsg, PXMSG pxm)
{
  SQIDX sqi;

  if (!SidxGet(HSqd->hix, hmsg->dwMsg, &sqi))
    return FALSE;

  /* Update this with the position of the message and the contents          *
   * of the 'To:' field.                                                    */

  sqi.ofs=hmsg->foWrite;
  sqi.hash=SquishHash(pxm->to);

  /* If the message has been read, set the high bit of the hash */

  if (pxm->attr & MSGREAD)
    sqi.hash |= IDXE_MSGREAD;

  return (unsigned)SidxPut(HSqd->hix, hmsg->dwMsg, &sqi);
}


/* Write a message to a Squish base */

sword MAPIENTRY SquishWriteMsg(HMSG hmsg, word fAppend, PXMSG pxm,
                               byte OS2FAR *szTxt, dword dwTxtLen,
                               dword dwTxtTotal,
                               dword dwCtrlLen, byte OS2FAR *szCtrl)
{
  dword dwOfs=(dword)-1L;

  /* Make sure that we have appropriate access to this message */

  if (MsgInvalidHmsg(hmsg) || !_SquishWriteMode(hmsg))
    return -1;

  /* Do sanity checking on our parameters */

  if ((sdword)dwCtrlLen < 0)
    dwCtrlLen=0L;

  if ((sdword)dwTxtTotal < 0)
    dwTxtTotal=0L;

  /* Make sure that szTxt and szCtrl are consistent */

  if (!dwTxtLen)
    szTxt=NULL;

  if (!dwCtrlLen)
    szCtrl=NULL;

  /* If we need to get a frame offset, allocate it here... */

  if (!hmsg->foWrite)
  {
    unsigned rc;

    /* We need to have an XMSG on the first write to any message */

    if (!pxm)
    {
      msgapierr=MERR_BADA;
      return -1;
    }

    if (! _SquishExclusiveBegin(hmsg->ha))
      return -1;

    rc=_SquishGetWriteFrame(hmsg, dwTxtTotal, dwCtrlLen);

    if (! _SquishExclusiveEnd(hmsg->ha) || !rc)
      return -1;
  }

  assert(hmsg->foWrite);

  /* Now write the various bits of the message */

  if (pxm)
    if (!_SquishWriteXmsg(hmsg, pxm, &dwOfs))
      return -1;

  if (szCtrl)
    if (!_SquishWriteCtrl(hmsg, szCtrl, dwCtrlLen, &dwOfs))
      return -1;

  if (szTxt)
    if (!_SquishWriteTxt(hmsg, fAppend, szTxt, dwTxtLen, &dwOfs))
      return -1;

  hmsg->fWritten=TRUE;

  /* If this was our first write to this message, update the index entry    *
   * appropriately.                                                         */

  if (pxm)
    if (! _SquishUpdateIndex(hmsg, pxm))
      return -1;

  return 0;
}




