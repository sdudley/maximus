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
static char rcs_id[]="$Id: t_qwk.c,v 1.1.1.1 2002/10/01 17:53:15 sdudley Exp $";
#pragma on(unreferenced)

#include "trackp.h"
#include "api_brow.h"
#include "qwk.h"

#ifdef MAX_TRACKER

extern char szTrk[];
static char szLeftAry[]="[{";
static char szRightAry[]="]}";

/* If we need to add message tracking information to the beginning of the   *
 * message, do so now.                                                      */

void TrackAddQWKFirst(TRK_MSG_NDX *ptmn, int *pdo_we_own,
                      char *blpos, char *ctrl)
{
  int real_owner;
  TRK t;

  if (!ctrl)
    return;

  /* If we are allowed to see the tracking information for this         *
   * message....                                                        */

  if (TrackAreWeOwner(ctrl, ptmn, &real_owner))
  {
    *pdo_we_own=TRUE;

    /* If we're the real owner, change TS_NEW to TS_OPEN */

    if (real_owner && (t=TrackGet()) != NULL)
    {
      TrackOwnerReadMsg(t, ptmn);
      TrackRelease(t);
    }

    /* Add the tracking information to the top, so it can be changed    *
     * by the off-line reader, if necessary.                            */

    sprintf(blpos,
            "^!> ACTrack: %s" QWK_EOL_STR
            "^!> Status: %c %c New; %c %c Open; %c %c Working; %c %c Closed"
            QWK_EOL_STR QWK_EOL_STR,
            ptmn->szTrackID,
            szLeftAry[ptmn->ts==TS_NEW], szRightAry[ptmn->ts==TS_NEW],
            szLeftAry[ptmn->ts==TS_OPEN], szRightAry[ptmn->ts==TS_OPEN],
            szLeftAry[ptmn->ts==TS_WORKING], szRightAry[ptmn->ts==TS_WORKING],
            szLeftAry[ptmn->ts==TS_CLOSED], szRightAry[ptmn->ts==TS_CLOSED]);
  }
}


void TrackAddQWKTail(TRK_MSG_NDX *ptmn, char *blpos, char *ctrl)
{
  extern char accomment_colon[];
  char *comment=NULL;

  if (ctrl)
    comment=MsgGetCtrlToken(ctrl, accomment_colon);

  sprintf(blpos, QWK_EOL_STR
          "^!> Owner: %s" QWK_EOL_STR
          "^!> Prty: %c %c Notify; %c %c Low; %c %c Normal; %c %c Urgent; %c %c Critical" QWK_EOL_STR
          "^!> Comment: [%-62.62s]" QWK_EOL_STR
          "^!> Discard reply text [ ]" QWK_EOL_STR,
          ptmn->to,
          szLeftAry[ptmn->tp==TP_NOTIFY], szRightAry[ptmn->tp==TP_NOTIFY],
          szLeftAry[ptmn->tp==TP_LOW], szRightAry[ptmn->tp==TP_LOW],
          szLeftAry[ptmn->tp==TP_NORMAL], szRightAry[ptmn->tp==TP_NORMAL],
          szLeftAry[ptmn->tp==TP_URGENT], szRightAry[ptmn->tp==TP_URGENT],
          szLeftAry[ptmn->tp==TP_CRIT], szRightAry[ptmn->tp==TP_CRIT],
          comment ? comment+11 : blank_str);

  if (comment)
    MsgFreeCtrlToken(comment);
}


/* QWK: Process any ACTRACK info messages in the body of a .QWK message */

void TrackProcessACInfo(char *actinfo, QWKTRACKINFO *pqti)
{
  TRK t;
  char acbuf[PATHLEN];
  char *end=strchr(actinfo, '\r');

  if (!end)
    return;

  /*
  01234567890123456789012345678
  ^!> ACTRACK: 9301231122330001
  */

  if ((t=TrackGet())==NULL)
    return;

  if (strncmp(actinfo+4, "ACTrack: ", 9)==0)
  {
    strncpy(acbuf, actinfo+13, 16);
    acbuf[16]=0;

    Printf(trk_got_actrack, acbuf);

    {
      if (!TrkLookupMsg(t, acbuf, NULL, NULL, NULL, &pqti->tmn))
      {
        Printf(trk_err_msg_notfound, acbuf);

        *pqti->tmn.szTrackID=0;
      }
    }
  }
  else
  {
    if (! *pqti->tmn.szTrackID)
    {
      Puts(trk_err_uploading);
    }
    else if (strncmp(actinfo+4, "Status: ", 8)==0)
    {
      TRK_STATUS tsOld=pqti->tmn.ts;

      /*           1         2         3         4         5 */
      /* 012345678901234567890123456789012345678901234567890123 */
      /* ^!> STATUS: [ ] New; [ ] Open; [ ] Working; [ ] Closed" */

      if (actinfo[13]=='x' || actinfo[13]=='X')
        pqti->tmn.ts=TS_NEW;
      else if (actinfo[22]=='x' || actinfo[22]=='X')
        pqti->tmn.ts=TS_OPEN;
      else if (actinfo[32]=='x' || actinfo[32]=='X')
        pqti->tmn.ts=TS_WORKING;
      else if (actinfo[45]=='x' || actinfo[45]=='X')
        pqti->tmn.ts=TS_CLOSED;

      if (pqti->tmn.ts != tsOld)
        Printf(trk_new_status, TrkGetStatus(t, &pqti->tmn));
    }
    else if (strncmp(actinfo+4, "Owner: ", 6)==0)
    {
      TRK_OWNER toNew;
      int len;

      /* 012345678901234567890123 */
      /* ^!> OWNER: Scott Dudley\r" */

      len=end-actinfo-11;
      len=min(len, sizeof(acbuf)-1);

      strncpy(acbuf, actinfo+11, len);
      acbuf[len]=0;

      /* Strip trailing spaces from the name */

      while (acbuf[len=strlen(acbuf)-1]==' ')
        acbuf[len]=0;

      /* Ensure that the specified owner is valid */

      if (!TrackValidOwner(t, acbuf, toNew, FALSE))
        Printf(trk_bad_owner, acbuf);
      else
      {
        if (!eqstri(pqti->tmn.to, toNew))
        {
          strcpy(pqti->tmn.to, toNew);
          Printf(trk_new_owner_is, acbuf);
        }
      }
    }
    else if (strncmp(actinfo+4, "Comment: ", 9)==0)
    {
      char *p;
      int len;

      /*               v14          v78 */
      /* ^!> Comment: [62 chars here] */

      strncpy(pqti->szNewComment, actinfo+14, 62);
      pqti->szNewComment[62]=0;

      if ((p=strchr(pqti->szNewComment, '\r')) != NULL)
        *p=0;

      len=strlen(pqti->szNewComment)-1;

      while (pqti->szNewComment[len]==' ')
        pqti->szNewComment[len--]=0;

      pqti->update_comment=TRUE;
    }
    else if (strncmp(actinfo+4, "Prty: ", 6)==0)
    {
      TRK_PRIORITY tpOld=pqti->tmn.tp;

      /*          1         2         3         4         5         6         7 */
      /* 1234567890123456789012345678901234567890123456789012345678901234567890 */
      /* ^!> PRTY: [ ] Notify; [ ] Low; [ ] Normal; [ ] Urgent; [ ] Critical" QWK_EOL_STR, */


      if (actinfo[12]=='x' || actinfo[12]=='X')
        pqti->tmn.tp=TP_NOTIFY;
      else if (actinfo[24]=='x' || actinfo[24]=='X')
        pqti->tmn.tp=TP_LOW;
      else if (actinfo[33]=='x' || actinfo[33]=='X')
        pqti->tmn.tp=TP_NORMAL;
      else if (actinfo[45]=='x' || actinfo[45]=='X')
        pqti->tmn.tp=TP_URGENT;
      else if (actinfo[57]=='x' || actinfo[57]=='X')
        pqti->tmn.tp=TP_CRIT;

      if (pqti->tmn.tp != tpOld)
        Printf(trk_new_priority, TrkGetPriority(t, &pqti->tmn));
    }
    else if (strncmp(actinfo+4, "Discard ", 8)==0)
    {
      /* Do not save this message, so return a corresponding code to        *
       * the caller.                                                        */

      /* 01234567890123456789012345 */
      /* ^!> Discard reply text [ ] */

      if (actinfo[24]=='x' || actinfo[24]=='X')
        pqti->kill_after=TRUE;
    }
    else
    {
      Printf(trk_bad_cmd, end-actinfo, end-actinfo, actinfo);
    }
  }

  TrackRelease(t);
  return;
}


/* QWK: Update the tracking database with the new information */

int TrackQWKUpdateTracking(QWKTRACKINFO *pqti)
{
  TRK_MSG_NDX tmnOld;
  TRK t;

  /* Do nothing if we have no tracking information to update */

  if (! *pqti->tmn.szTrackID)
    return FALSE;

  if (!TrackAreWeOwnerOfActrack(pqti->tmn.szTrackID, NULL, NULL))
  {
    Puts(trk_admin_noaccess);
    return FALSE;
  }

  if ((t=TrackGet())==NULL)
    return FALSE;

  if (!TrkLookupMsg(t, pqti->tmn.szTrackID, NULL, NULL, NULL, &tmnOld))
    Printf(trk_bad_old_id, pqti->tmn.szTrackID);

  if (!TrkUpdateMsg(t, &tmnOld, &pqti->tmn))
    Printf(trk_err_updating_db, pqti->tmn.szTrackID);
  else
    Printf(trk_info_updated, pqti->tmn.szTrackID);

  /* Modify the comment, if necessary */

  if (pqti->update_comment && *pqti->szNewComment)
  {
    char szNewComment[PATHLEN];

    sprintf(szNewComment, "\x01""ACCOMMENT: %s", pqti->szNewComment);

    TrackInsertTracking(sq, MsgUidToMsgn(sq, pqti->tmn.tl.uid, UID_EXACT), NULL,
                        FALSE, FALSE, NULL,
                        FALSE, NULL,
                        TRUE, szNewComment);
  }

  TrackRelease(t);

  return pqti->kill_after;
}


#endif /* MAX_TRACKER */

