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
static char rcs_id[]="$Id: sq_read.c,v 1.1.1.1 2002/10/01 17:54:33 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdio.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"



/* Read in the binary message header from the data file */

static unsigned near _SquishReadXmsg(HMSG hmsg, PXMSG pxm, dword *pdwOfs)
{
  long ofs=hmsg->foRead + HSqd->cbSqhdr;

  if (*pdwOfs != (dword)ofs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

  if (read(HSqd->sfd, (char *)pxm, sizeof *pxm) != (int)sizeof *pxm)
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  /* Update our position */

  *pdwOfs=(dword)ofs + (dword)sizeof *pxm;

  /* If there is a UMSGID associated with this message, store it in         *
   * memory in case we have to write the message later.  Blank it           *
   * out so that the application cannot access it.                          */

  if (pxm->attr & MSGUID)
  {
    hmsg->uidUs=pxm->umsgid;
    pxm->attr &= ~MSGUID;
    pxm->umsgid=0L;
  }

  return TRUE;
}


/* Read in the control information for the current message */

static unsigned near _SquishReadCtrl(HMSG hmsg, byte OS2FAR *szCtrl,
                                     dword dwCtrlLen, dword *pdwOfs)
{
  long ofs=hmsg->foRead + HSqd->cbSqhdr + sizeof(XMSG);
  unsigned uMaxLen=(unsigned)min(dwCtrlLen, hmsg->sqhRead.clen);

  /* Read the specified amount of text, but no more than specified in       *
   * the frame header.                                                      */


  if (*pdwOfs != (dword)ofs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }


  if (read(HSqd->sfd, (char *)szCtrl, uMaxLen) != (int)uMaxLen)
  {
    *szCtrl=0;
    msgapierr=MERR_BADF;
    return FALSE;
  }

  /* Make sure that the field is properly NUL-terminated */

  szCtrl[uMaxLen ? uMaxLen-1 : 0]=0;

  *pdwOfs=(dword)ofs + (dword)uMaxLen;

  return TRUE;
}


/* Read in the text body for the current message */

static dword near _SquishReadTxt(HMSG hmsg, byte OS2FAR *szTxt, dword dwTxtLen,
                                 dword *pdwOfs)
{
  /* Start reading from the cur_pos offset */

  long ofs=hmsg->foRead + (long)HSqd->cbSqhdr + (long)sizeof(XMSG)
                        + (long)hmsg->sqhRead.clen;

  /* Max length is the size of the msg text inside the frame */

  unsigned uMaxLen=(unsigned)(hmsg->sqhRead.msg_length -
                              hmsg->sqhRead.clen - sizeof(XMSG));

  /* Make sure that we don't try to read beyond the end of the msg */

  if (hmsg->cur_pos > uMaxLen)
    hmsg->cur_pos=uMaxLen;

  /* Now seek to the position that we are supposed to read from */

  ofs += (long)hmsg->cur_pos;

  /* Decrement the max length by the seek posn, but don't read more than    *
   * the user asked for.                                                    */

  uMaxLen -= (unsigned)hmsg->cur_pos;
  uMaxLen=min(uMaxLen, (unsigned)dwTxtLen);

  /* Now try to read that information from the file */

  if (ofs != (long)*pdwOfs)
    if (lseek(HSqd->sfd, ofs, SEEK_SET) != ofs)
    {
      msgapierr=MERR_BADF;
      return (dword)-1L;
    }


  if (read(HSqd->sfd, (char *)szTxt, uMaxLen) != (int)uMaxLen)
  {
    msgapierr=MERR_BADF;
    return (dword)-1L;
  }

  *pdwOfs = (dword)ofs + (dword)uMaxLen;


  /* Increment the current position by the number of bytes that we read */

  hmsg->cur_pos += (dword)uMaxLen;

  return (dword)uMaxLen;
}

 
/* Read a message from the Squish base */

dword MAPIENTRY SquishReadMsg(HMSG hmsg, PXMSG pxm, dword dwOfs,
                              dword dwTxtLen, byte OS2FAR *szTxt,
                              dword dwCtrlLen, byte OS2FAR *szCtrl)
{
  dword dwSeekOfs=(dword)-1L; /* Current offset */
  unsigned fOkay=TRUE;        /* Any errors so far? */
  dword dwGot=0;              /* Bytes read from file */

  /* Make sure that we have a valid handle (and that it's in read mode) */

  if (MsgInvalidHmsg(hmsg) || !_SquishReadMode(hmsg))
    return (dword)-1L;


  /* Make sure that we can use szTxt and szCtrl as flags controlling what   *
   * to read.                                                               */

  if (!dwTxtLen)
    szTxt=NULL;

  if (!dwCtrlLen)
    szCtrl=NULL;


  /* Now read in the message header, the control information, and the       *
   * message text.                                                          */

 if (pxm)
    fOkay=_SquishReadXmsg(hmsg, pxm, &dwSeekOfs);

  if (fOkay && szCtrl)
    fOkay=_SquishReadCtrl(hmsg, szCtrl, dwCtrlLen, &dwSeekOfs);

  if (fOkay && szTxt)
  {
    hmsg->cur_pos=dwOfs;

    if ((dwGot=_SquishReadTxt(hmsg, szTxt, dwTxtLen, &dwSeekOfs))==(dword)-1L)
      fOkay=FALSE;
  }

  /* If everything worked okay, return the number bytes that we read        *
   * from the message body.                                                 */

  return fOkay ? dwGot : (dword)-1L;
}



