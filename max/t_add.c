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
static char rcs_id[]="$Id: t_add.c,v 1.2 2003/06/11 14:03:06 wesgarland Exp $";
#pragma on(unreferenced)

#include "trackp.h"

#ifdef MAX_TRACKER

char accomment_colon[]="ACCOMMENT:";
extern char szTrk[];

/* Ask the user to specify an owner for this message */

int TrackAskOwner(TRK t, TRK_OWNER to)
{
  char szOwner[PATHLEN];
  char prompt[PATHLEN];

  for (;;)
  {
    /* Get owner from user */

    WhiteN();
    InputGetsL(szOwner, PATHLEN-1, which_owner);

    if (! *szOwner)
      return FALSE;
    else if (eqstri(szOwner, qmark))
      TrackValidOwner(t, szOwner, to, TRUE);
    else if (TrackValidOwner(t, szOwner, to, FALSE))
      return TRUE;
    else
    {
      /* The owner is not in the database.  See if the user has a high      *
       * enough priv level to add an owner.                                 */

      if (!PrivOK(PRM(track_privmod), FALSE))
      {
        Printf(trk_owner_not_accept, szOwner);
        Press_ENTER();
        continue;
      }

      sprintf(prompt, trk_ask_owner_quest, szOwner);

      if (GetListAnswer(yCN, NULL, useyforyes, 0, prompt) != 'Y')
        continue;

      sprintf(prompt, trk_enter_4char, szOwner);

      Input(to, INPUT_LB_LINE, 0, 4, prompt);

      if (! *to)
        continue;

      if (!TrkSetOwner(t, to, szOwner))
      {
        strcpy(szOwner, qmark);
        logit(log_err_setting_owner, to, szOwner);
      }

      Printf(trk_added_owner_as, szOwner, to);
      return TRUE;
    }
  }
}




/* Generate an ACTRACK kludge string for a particular message */

void GenerateActrack(XMSG *pxmsg, char *kend)
{
  static int counter=0;
  SCOMBO sc;
  union _stampu *pm=&sc.msg_st;

  NW(pxmsg);

  Get_Dos_Date(&sc);

  sprintf(kend, "\x01""ACTRACK: %02u%02u%02u%02u%02u%02u%02x%02x",
          (pm->date.yr + 80) % 100, pm->date.mo, pm->date.da,
          pm->time.hh, pm->time.mm, pm->time.ss,
          counter++ % 0x100,
          task_num);
}


/* Add/remove ACTRACK string from existing message, and add the specified   *
 * comment to the end of the message body, if necessary.                    */

int TrackInsertTracking(HAREA ha, dword msgnum, XMSG *pxmsg,
                        int modify_actrack, int add_act, char *actrack,
                        int modify_audit, char *audit,
                        int modify_comment, char *comment)
{
  XMSG xmsg;
  char *txt, *ctrl;
  dword txt_len, ctrl_len;
  HMSG hm;
  int rc=FALSE;
  int len_audit=modify_audit ? strlen(audit) + 30 : 0;
  int len_comment=modify_comment && comment ? strlen(comment) : 0;

  /* If the caller doesn't want an XMSG as output, use our own temp variable */

  if (!pxmsg)
    pxmsg=&xmsg;

  if ((hm=MsgOpenMsg(ha, MOPEN_READ, msgnum))==NULL)
    return FALSE;

  /* Read the message header and generate an ACTRACK string based on that */

  MsgReadMsg(hm, pxmsg, 0L, 0L, NULL, 0L, NULL);

  /* If adding, actrack is an input param.  If deleting, actrack is output. */

  if (modify_actrack)
  {
    if (add_act)
      GenerateActrack(pxmsg, actrack);
    else *actrack=0;
  }

  /* Figure out how much memory to allocate for the buffers */

  txt_len=MsgGetTextLen(hm) + len_audit + 10;
  ctrl_len=MsgGetCtrlLen(hm) + (actrack ? strlen(actrack) : 0) +len_comment+10;

  /* Allocate memory for message */

  if ((txt=malloc((size_t)txt_len)) != NULL)
  {
    if ((ctrl=malloc((size_t)ctrl_len)) != NULL)
    {
      char *p;

      /* Read in the entire message */

      MsgReadMsg(hm, pxmsg, 0L, (size_t)txt_len, txt, (size_t)ctrl_len, ctrl);

      /* Make sure that the text is nul-terminated */

      txt[MsgGetTextLen(hm)]=0;

      MsgCloseMsg(hm);

      /* If we're doing the ACTRACK string, make sure that it is (or is not)*
       * there, as appropriate.                                             */

      p=MsgGetCtrlToken(ctrl, actrack_colon);

      if (modify_actrack)
      {
        if (add_act && p)
        {
          Puts(trk_msg_already_in_db);

          MsgFreeCtrlToken(p);
          free(ctrl);
          free(txt);
          return FALSE;
        }

        if (!add_act && !p)
        {
          Puts(trk_msg_not_in_db);
          free(ctrl);
          free(txt);
          return FALSE;
        }
      }

      if (modify_actrack)
      {
        if (add_act)
        {
          /* Add on the new actrack string */

          Strip_Trailing(ctrl, '\x01');
          strcat(ctrl, actrack);
          Add_Trailing(ctrl, '\x01');
        }
        else
        {
          /* Copy this ACTRACK string so the caller can use it, then remove   *
           * it from the message body.                                        */

          strcpy(actrack, p+9);
          MsgRemoveToken(ctrl, actrack_colon);
        }
      }

      MsgFreeCtrlToken(p);


      /* Now process the ACCOMMENT token, if it exists */

      if (modify_comment)
      {
        MsgRemoveToken(ctrl, accomment_colon);

        if (comment)
        {
          Strip_Trailing(ctrl, '\x01');
          strcat(ctrl, comment);
          Add_Trailing(ctrl, '\x01');
        }
      }

      /* If necessary, add an audit trail to the end of the message body */

      if (modify_audit)
      {
        Add_Trailing(txt, '\r');
        sprintf(txt+strlen(txt), "\x01%s\r", audit);
      }


      /* Rewrite the message with appropriate control info */

      if ((hm=MsgOpenMsg(ha, MOPEN_CREATE, msgnum))==NULL)
      {
        logit(cantwrite, MAS(mah, name));
        free(ctrl);
        free(txt);
        return FALSE;
      }

      MsgWriteMsg(hm, FALSE, pxmsg, txt, strlen(txt)+1, txt_len,
                  strlen(ctrl)+1, ctrl);

      /* Rewrite succeeded */

      rc=TRUE;
      free(ctrl);
    }

    free(txt);
  }

  MsgCloseMsg(hm);
  return rc;
}





/* Insert the actual tracking record in the database */

static int near TrackInsertDB(char *actrack, char *name, TRK_OWNER to, dword uid, XMSG *pxmsg)
{
  TRK_MSG_NDX tmn;
  int rc;
  TRK t;

  /* Fill out the database header */

  memset(&tmn, 0, sizeof tmn);

  strcpy(tmn.szTrackID, actrack);
  strcpy(tmn.to, to);
  strcpy(tmn.tl.szArea, name);
  tmn.tl.uid=uid;
  tmn.ts=TS_NEW;
  tmn.tp=TP_NORMAL;
  tmn.scDateWritten.msg_st=pxmsg->date_written;

  /* Add to the .db file */

  if ((t=TrackGet()) != NULL)
  {
    /* If we have a blank owner, see if the message is addressed to one     *
     * of our moderators, and if so, assign it automatically to that        *
     * moderator.                                                           */

    if (! *tmn.to)
      TrackValidOwner(t, pxmsg->to, tmn.to, FALSE);

    if ((rc=TrkAddMsg(t, &tmn)) != 0)
      logit(log_tracking, name, uid, actrack);
    else
    {
      logit(log_track_cantadd1, name, uid);
      logit(log_track_cantadd2);
    }

    TrackRelease(t);
  }

  return rc;
}


/* Insert a message into the tracking database, not only by adding          *
 * the ACTRACK line, but also by placing it in the .db file itself.         */

int TrackInsertMessage(HAREA ha, dword msgnum, TRK_OWNER to, char *out_actkludge)
{
  char inserted_audit[PATHLEN];
  char actrack[PATHLEN];
  XMSG xmsg;
  SCOMBO sc;
  int rc=FALSE;

  Get_Dos_Date(&sc);


  /* Indicate when the message was placed in the tracking database */

  TrackMakeACAUDIT(inserted_audit, trk_added_to_db);


  /* Add the ACTRACK line into the physical message */

  if (TrackInsertTracking(ha, msgnum, &xmsg,
                          TRUE, TRUE, actrack,
                          TRUE, inserted_audit,
                          FALSE, NULL))
  {
    rc=TrackInsertDB(actrack+10, MAS(mah, name), to,
                     MsgMsgnToUid(ha, msgnum),
                     &xmsg);
  }

  /* If the caller wants to know the actrack line, give it to 'em */

  if (rc && out_actkludge)
    strcpy(out_actkludge, actrack);

  return rc;
}



/* Determine whether or not we need to add a particular message to the      *
 * tracking database.                                                       */

int TrackNeedToAdd(PMAH pmah, XMSG *pxmsg)
{
  TRK t;
  char *szTag;
  char szOwner[TRK_ASCII_OWNER_SIZE];
  int rc=TRUE;

  /* Make sure that this is a tracking area */

  if ((pmah->ma.attribs & MA_AUDIT)==0)
    return FALSE;


  /* Check the exclusion list */

  if (IsUserExcluded(pxmsg->from))
    return FALSE;


  if ((t=TrackGet())==NULL)
    return FALSE;

  /* See if we are the owner of this area */

  szTag=TrkGetAreaOwner(t, PMAS(pmah, name));

  if (szTag && TrkGetOwner(t, szTag, szOwner) && eqstri(szOwner, pxmsg->from))
    rc=FALSE;

  TrackRelease(t);
  return TRUE;
}


/* Add tracking information for this message to the tracking database */

void TrackAddMessage(PMAH pmah, XMSG *msg, char *kludges, HAREA ha)
{
  char *actrack=MsgGetCtrlToken(kludges, actrack_colon);

  /* Make sure that we are only tracking messages in specified areas,       *
   * or if we are the moderator...                                          */

  if (!actrack || (pmah->ma.attribs & MA_AUDIT)==0)
    return;

  TrackInsertDB(actrack+9, PMAS(pmah, name), blank_str,
                MsgMsgnToUid(ha, MsgGetHighMsg(ha)), msg);

  MsgFreeCtrlToken(actrack);
}





/* This function is called whenever the owner of a message reads that message */

void TrackOwnerReadMsg(TRK t, TRK_MSG_NDX *ptmn)
{
  TRK_MSG_NDX tmnNew;

  if (ptmn->ts==TS_NEW)
  {
    tmnNew=*ptmn;
    tmnNew.ts=TS_OPEN;

    if (!TrkUpdateMsg(t, ptmn, &tmnNew))
      logit(log_err_updating_stat);

    *ptmn=tmnNew;
  }
}



/* We are about to display this message to the on-line user, but there is   *
 * a chance that the message originated on a remote system, and from that,  *
 * it wouldn't get added to the tracking database.  We have to deal with    *
 * that by seeing if the message appeared to originate remotely, and if     *
 * that is the case, we must dynamically add it to the tracking database    *
 * to ensure that it will be tracked.                                       */

int TrackNeedToInsertRemoteMsg(PMAH pmah, XMSG *pxmsg, char *kludges)
{
  char *p;

  /* If the message already contains an ACTRACK kludge, there is no need    *
   * to add it.                                                             */

  if (kludges && (p=MsgGetCtrlToken(kludges, actrack_colon)) != NULL)
  {
    MsgFreeCtrlToken(p);
    return FALSE;
  }

  /* Make sure that it doesn't have an ACGHOST kludge either */

  if (kludges && (p=MsgGetCtrlToken(kludges, "ACGHOST")) != NULL)
  {
    MsgFreeCtrlToken(p);
    return FALSE;
  }

  return ((pxmsg->attr & MSGLOCAL)==0 &&
          (((SCOMBO *)&pxmsg->date_written)->ldate != ((SCOMBO *)&pxmsg->date_arrived)->ldate ||
          ((SCOMBO *)&pxmsg->date_written)->ldate==0) &&
          TrackNeedToAdd(pmah, pxmsg));
}


#endif /* MAX_TRACKER */

