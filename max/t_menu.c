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
static char rcs_id[]="$Id: t_menu.c,v 1.2 2004/01/11 19:43:21 wmcbrine Exp $";
#pragma on(unreferenced)

#include "trackp.h"


#ifdef MAX_TRACKER

static void near TrackMenuInsert(void);
extern char szTrk[];

/* Modify the owner of an existing message */

static void near TrackModOwner(TRK t, TRK_MSG_NDX *ptmn, dword msgnum)
{
  TRK_MSG_NDX tmnNew=*ptmn;

  if (!TrackAskOwner(t, tmnNew.to))
    return;

  if (!TrkUpdateMsg(t, ptmn, &tmnNew))
    Printf(trk_err_upd_db);
  else
  {
    char comment[PATHLEN];

    strcpy(ptmn->to, tmnNew.to);

    /* Note this in the change log */

    TrackMakeACAUDIT(comment, trk_msg_audit_str,
                     ptmn->to, usrname);

    TrackInsertTracking(sq, msgnum, NULL,
                        FALSE, FALSE, NULL,
                        TRUE, comment,
                        FALSE, NULL);
  }
}



/* Modify the comment of an existing message */

static void near TrackModComment(TRK t, TRK_MSG_NDX *ptmn, dword msgnum, char *szComment)
{
  char temp[PATHLEN];
  char szNewComment[PATHLEN];

  NW(t);
  NW(ptmn);

  WhiteN();

  Printf(trk_existing_cmt, szComment ? szComment : trk_no_cmt);

  InputGetsL(temp, PATHLEN-1, trk_enter_cmt);

  sprintf(szNewComment, "\x01""ACCOMMENT: %s", temp);

  TrackInsertTracking(sq, msgnum, NULL,
                      FALSE, FALSE, NULL,
                      FALSE, NULL,
                      TRUE, *temp ? szNewComment : NULL);
}



/* Modify the status of an existing messages */

static void near TrackModStatus(TRK t, TRK_MSG_NDX *ptmn, dword msgnum,
                                HAREA ha)
{
  TRK_MSG_NDX tmnNew=*ptmn;
  char prompt[PATHLEN];
  char *szDefault=trk_status_def;
  char *szStatusKeys=trk_status_keys;
  int ch;

  sprintf(prompt, trk_status_txt,
          ptmn->ts==TS_NEW ? szDefault : blank_str,
          ptmn->ts==TS_OPEN ? szDefault : blank_str,
          ptmn->ts==TS_WORKING ? szDefault : blank_str,
          ptmn->ts==TS_CLOSED ? szDefault : blank_str);

  /* Get input from the user */

  do
  {
    WhiteN();
    ch=toupper(KeyGetRNP(prompt));
  }
  while (!strchr(szStatusKeys, ch) && ch != 13 && ch != 0);

  /* Make sure that it's a valid option */

  if (ch==szStatusKeys[0])
    tmnNew.ts=TS_NEW;
  else if (ch==szStatusKeys[1])
    tmnNew.ts=TS_OPEN;
  else if (ch==szStatusKeys[2])
    tmnNew.ts=TS_WORKING;
  else if (ch==szStatusKeys[3])
    tmnNew.ts=TS_CLOSED;
  else return;

  if (!TrkUpdateMsg(t, ptmn, &tmnNew))
    Printf(trk_err_upd_db);
  else
  {
    char comment[PATHLEN];
    ptmn->ts=tmnNew.ts;

    /* Note this in the change log */

    TrackMakeACAUDIT(comment, trk_status_set,
                     TrkGetStatus(t, ptmn), usrname);

    TrackInsertTracking(ha, msgnum, NULL,
                        FALSE, FALSE, NULL,
                        TRUE, comment,
                        FALSE, NULL);
  }
}


/* Change the status for an existing message */

static void near TrackModPriority(TRK t, TRK_MSG_NDX *ptmn, dword msgnum)
{
  TRK_MSG_NDX tmnNew=*ptmn;
  char *szPriorityKeys=trk_pri_keys;
  int ch;

  do
  {
    WhiteN();
    ch=toupper(KeyGetRNP(trk_pri_txt));
  }
  while (!strchr(szPriorityKeys, ch) && ch != 13 && ch != 0);

  if (ch==szPriorityKeys[0])
    tmnNew.tp=TP_NOTIFY;
  else if (ch==szPriorityKeys[1])
    tmnNew.tp=TP_LOW;
  else if (ch==szPriorityKeys[2])
    tmnNew.tp=TP_NORMAL;
  else if (ch==szPriorityKeys[3])
    tmnNew.tp=TP_URGENT;
  else if (ch==szPriorityKeys[4])
    tmnNew.tp=TP_CRIT;

  if (!TrkUpdateMsg(t, ptmn, &tmnNew))
    Printf(trk_err_upd_db);
  else
  {
    char comment[PATHLEN];

    ptmn->tp=tmnNew.tp;

    /* Note this in the change log */

    TrackMakeACAUDIT(comment, trk_pri_set,
                     TrkGetPriority(t, ptmn), usrname);

    TrackInsertTracking(sq, msgnum, NULL,
                        FALSE, FALSE, NULL,
                        TRUE, comment,
                        FALSE, NULL);
  }
}


/* Modify message information */

static void near TrackMenuModify(void)
{
  char *mod_keys=trk_mod_keys;
  HMSG hmsg;
  dword msgnum;
  int ch;
  TRK t;

  if (!GetMsgNum(modify_which, &msgnum))
    return;

  if ((t=TrackGet())==NULL)
    return;

  do
  {
    ch=0;

    if ((hmsg=MsgOpenMsg(sq, MOPEN_READ, msgnum)) != NULL)
    {
      long clen=MsgGetCtrlLen(hmsg);
      char *kludges;
      TRK_MSG_NDX tmn;
      char *actrack;


      if ((kludges=malloc((size_t)clen)) != NULL)
        MsgReadMsg(hmsg, NULL, 0L, 0L, NULL, (size_t)clen, kludges);

      MsgCloseMsg(hmsg);

      if (kludges)
      {
        if ((actrack=MsgGetCtrlToken(kludges, actrack_colon))==NULL ||
            !TrkLookupMsg(t, actrack+9, NULL, NULL, NULL, &tmn))
        {
          Printf(trk_no_actrack_for_msg);
        }
        else
        {
          TRK_MSG_NDX tmnTemp;

          if (!TrackAreWeOwner(kludges, &tmnTemp, NULL))
          {
            Printf(trk_not_owner);
            Press_ENTER();
          }
          else
          {
            do
            {
              extern char accomment_colon[];
              char szOwner[PATHLEN];
              char *szComment;

              if (!TrkGetOwner(t, tmn.to, szOwner))
                strcpy(szOwner, tmn.to);

              WhiteN();

              Printf(trk_msg_stat_owner, szOwner);
              Printf(trk_msg_stat_status, TrkGetStatus(t, &tmn));
              Printf(trk_msg_stat_pri, TrkGetPriority(t, &tmn));

              if ((szComment=MsgGetCtrlToken(kludges, accomment_colon)) != NULL)
                Printf(trk_msg_stat_cmt, szComment+11);

              Putc('\n');

              do
              {
                ch=toupper(KeyGetRNP(trk_mod_txt));
              }
              while (!strchr(mod_keys, ch) && ch != 13 && ch != 0);

              if (ch==mod_keys[0])
                TrackModOwner(t, &tmn, msgnum);
              else if (ch==mod_keys[1])
                TrackModStatus(t, &tmn, msgnum, sq);
              else if (ch==mod_keys[2])
                TrackModPriority(t, &tmn, msgnum);
              else if (ch==mod_keys[3])
                TrackModComment(t, &tmn, msgnum, szComment ? szComment+11 : NULL);

              if (szComment)
                MsgFreeCtrlToken(szComment);

              /* Don't loop if we have modified the comment */
            }
            while (ch != mod_keys[3] && ch != mod_keys[4] &&
                   ch != 13 && ch != 0);
          }
        }

        if (actrack)
          MsgFreeCtrlToken(actrack);

        free(kludges);
      }
    }

    /* If the user modified the comment, we need to reread the original msg */
  }
  while (ch==mod_keys[3]);

  TrackRelease(t);
}

/* Remove a message from the tracking database */

static void near TrackAdminRemove(void)
{
  char actrack[PATHLEN];
  char comment[PATHLEN];
  dword msgnum;

  /* Ask which message we are to insert */

  if (!GetMsgNum(remove_which, &msgnum))
    return;

  TrackMakeACAUDIT(comment, trk_rem_cmt, usrname);

  if (!TrackInsertTracking(sq, msgnum, NULL,
                           TRUE, FALSE, actrack,
                           TRUE, comment,
                           FALSE, NULL))
  {
    Printf(trk_rem_err, msgnum);
  }
  else
  {
    TRK t;

    if ((t=TrackGet()) != NULL)
    {
      TRK_MSG_NDX tmn;

      if (TrkLookupMsg(t, actrack, NULL, NULL, NULL, &tmn))
      {
        if (TrkDeleteMsg(t, &tmn))
          Printf(trk_rem_done, msgnum);
        else
          Printf(trk_rem_err, msgnum);
      }

      TrackRelease(t);
    }
  }
}




/* Get a message area name from the user */

static int near GetValidArea(char *szArea, int sizeofarea, char *szAreaPrompt)
{
  BARINFO bi;

  do
  {
    WhiteN();
    InputGetsL(szArea, sizeofarea-1, szAreaPrompt);

    if (eqstri(szArea, qmark))
    {
      ListMsgAreas(NULL, TRUE, FALSE);
      continue;
    }

    if (! *szArea)
      return FALSE;
  }
  while (!ValidMsgArea(szArea, NULL, VA_VAL, &bi));

  return TRUE;
}




/* Assign an owner to a particular area */

static void near TrackAdminOwnerAssign(TRK t)
{
  TRK_OWNER_NDX ton;

  /* Get a new owner name */

  WhiteN();

  memset(&ton, 0, sizeof ton);
  InputGetsL(ton.szOwner, sizeof(ton.szOwner)-1, trk_new_owner);

  if (! *ton.szOwner)
    return;

  /* Get an alias name */

  WhiteN();
  Input(ton.to, INPUT_LB_LINE, 0, 4, trk_new_alias);

  if (! *ton.to)
    return;

  if (TrkSetOwner(t, ton.to, ton.szOwner))
    Printf(trk_set_alias, ton.to, ton.szOwner);
  else
    Printf(trk_err_alias, ton.to);

  return;
}



/* List all of the current owner/alias links */

static void near TrackAdminOwnerList(TRK t)
{
  TRK_OWNER_NDX *pton;
  PALIST *ppl;
  BTREE *pbt;

  pbt=TrkGetOwnerBtree(t);

  ppl=PalistNew();

  Puts(trk_owner_header);

  while ((pton=BtLookup(pbt, NULL, ppl)) != NULL)
    Printf(trk_owner_fmt, pton->to, pton->szOwner);

  PalistDelete(ppl);
}

/* Delete a particular area/owner link */

static void near TrackAdminOwnerDelete(TRK t)
{
  TRK_OWNER to;

  do
  {
    WhiteN();
    InputGetsL(to, sizeof(to)-1, trk_remalias_which);

    if (eqstri(to, qmark))
      TrackAdminOwnerList(t);
  }
  while (eqstri(to, qmark));

  if (TrkSetOwner(t, to, NULL))
    Printf(trk_remalias_done, to);
  else
    Printf(trk_remalias_err, to);
}


/* Add or delete an owner/alias link */

static void near TrackAdminOwner(void)
{
  char *owner_keys=trk_owner_keys;
  int ch;
  TRK t;

  do
  {
    do
    {
      WhiteN();
      ch=toupper(KeyGetRNP(trk_owner_txt));

      if (!ch || ch==13)
        return;
    }
    while (!strchr(owner_keys, ch));

    if ((t=TrackGet()) != NULL)
    {
      if (ch==owner_keys[0])
        TrackAdminOwnerAssign(t);
      else if (ch==owner_keys[1])
        TrackAdminOwnerDelete(t);
      else if (ch==owner_keys[2])
        TrackAdminOwnerList(t);

      TrackRelease(t);
    }
  }
  while (ch != owner_keys[3]);
}




/* Assign an owner to a particular area */

static void near TrackAdminAreaAssign(TRK t)
{
  TRK_AREA_NDX tan;

  /* Get the user to give us an area name to assign */

  if (!GetValidArea(tan.szArea, sizeof(tan.szArea), trk_owner_area))
    return;

  /* We now have a valid area, so find an owner to link with it */

  if (TrackAskOwner(t, tan.to))
  {
    if (TrkSetDefaultOwner(t, tan.szArea, tan.to))
    {
      Printf(trk_owner_assgn_done, tan.to, tan.szArea);
    }
    else
    {
      Printf(trk_owner_assgn_err, tan.szArea);
    }

  }

  return;
}


/* Delete a particular area/owner link */

static void near TrackAdminAreaDelete(TRK t)
{
  char szArea[MAX_ALEN];

  if (!GetValidArea(szArea, sizeof(szArea), trk_owner_lnk_rem))
    return;

  if (TrkSetDefaultOwner(t, szArea, NULL))
    Printf(trk_owner_lnk_done, szArea);
  else
    Printf(trk_owner_lnk_err, szArea);
}



/* List all of the current area/owner links */

static void near TrackAdminAreaList(TRK t)
{
  TRK_AREA_NDX *ptan;
  PALIST *ppl;
  BTREE *pbt;

  pbt=TrkGetAreaBtree(t);

  ppl=PalistNew();

  Puts(trk_owner_lnk_head);

  while ((ptan=BtLookup(pbt, NULL, ppl)) != NULL)
    Printf(trk_owner_lnk_fmt, ptan->to, ptan->szArea);

  PalistDelete(ppl);
}


/* Add or delete area/owner links */

static void near TrackAdminArea(void)
{
  char *area_keys=trk_area_keys;
  int ch;
  TRK t;

  do
  {
    do
    {
      WhiteN();
      ch=toupper(KeyGetRNP(trk_area_txt));

      if (!ch || ch==13)
        return;
    }
    while (!strchr(area_keys, ch));

    if ((t=TrackGet()) != NULL)
    {
      if (ch==area_keys[0])
        TrackAdminAreaAssign(t);
      else if (ch==area_keys[1])
        TrackAdminAreaDelete(t);
      else if (ch==area_keys[2])
        TrackAdminAreaList(t);

      TrackRelease(t);
    }
  }
  while (ch != area_keys[3]);
}


/* Administration commands for the tracking database */

static void near TrackMenuAdmin(void)
{
  char *admin_keys=trk_admin_keys;

  int ch;

  if (!PrivOK(PRM(track_privmod), FALSE))
  {
    Puts(trk_admin_noaccess);
    Press_ENTER();
    return;
  }

  do
  {
    WhiteN();

    ch=toupper(KeyGetRNP(trk_admin_txt));

    if (!ch || ch==13)
      return;
  }
  while (!strchr(admin_keys, ch));

  if (ch==admin_keys[0])
    TrackAdminOwner();
  else if (ch==admin_keys[1])
    TrackAdminArea();
  else if (ch==admin_keys[2])
    TrackAdminRemove();
}


/* Tracking options menu */

void TrackMenu(void)
{
  char *trak_keys=trk_menu_keys;

  int ch;

#if 0
  Insert Message in Database
    {adds to tracking database, prompts for owner (defaults to self)}
  Modify Message Information
    {get message number, or wildcard}
    {Owner, Status - calls TrackModifyStatus, Priority}
  Report
    Areas:     {All, Tagged, This}
    Msg Type:  {any or all of tracking status fields}
    Owner:     {you, all, name}
    Destination: {on-line report, file}
  Administration
    Owner/alias links
      Assign Link
      List Links
    Area owner links
      Assign Link
      List Links
    Remove message from database
#endif

  RipClear();

  WhiteN();

  do
  {
    ch=toupper(KeyGetRNP(trk_menu_txt));

    if (!ch || ch==13)
      return;
  }
  while (!strchr(trak_keys, ch));

  if (ch==trak_keys[0])
    TrackMenuInsert();
  else if (ch==trak_keys[1])
    TrackMenuModify();
  else if (ch==trak_keys[2])
    TrackMenuReport();
  else if (ch==trak_keys[3])
    TrackMenuAdmin();
}

/* Insert an existing message in the tracking database */

static void near TrackMenuInsert(void)
{
  TRK t;
  TRK_OWNER to;
  dword msgnum;

  /* User must be in tracking owner database, or have priv to modify        *
   * messages for others.                                                   */

  if (!GetTrkList() && !PrivOK(PRM(track_privmod), FALSE))
  {
    Printf(trk_admin_noaccess);
    Press_ENTER();
    return;
  }

  /* Ask which message we are to insert */

  if (!GetMsgNum(insert_which, &msgnum))
    return;

  /* Ask who is to own this msg */

  if ((t=TrackGet()) != NULL)
  {
    int rc=TrackAskOwner(t, to);
    TrackRelease(t);

    if (rc)
      Printf(TrackInsertMessage(sq, msgnum, to, NULL)
              ? trk_ins_ok
              : trk_ins_err,
              MAS(mah, name), UIDnum(msgnum));
  }
}



/* This function is invoked after a moderator replies to a tracked message */

int TrackAfterReply(char *kludges, dword msgnum, HAREA ha)
{
  TRK_MSG_NDX tmn;
  int real_owner;

  /* See if we actually own this message */

  if (TrackAreWeOwner(kludges, &tmn, &real_owner) && real_owner)
  {
    TRK t=TrackGet();

    if (t)
    {
      TrackModStatus(t, &tmn, msgnum, ha);
      TrackRelease(t);
    }
  }

  return TRUE;
}


#endif /* MAX_TRACKER */

