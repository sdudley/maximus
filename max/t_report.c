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
static char rcs_id[]="$Id: t_report.c,v 1.1.1.1 2002/10/01 17:53:16 sdudley Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_init
#define MAX_LANG_max_main
#include "trackp.h"

#ifdef MAX_TRACKER

#include "m_browse.h"
#include "api_brow.h"
#include "qwk.h"

extern char szTrk[];

/* Generate a report from the tracking database */

static int near RepGetArea(TRK t, PREP prep)
{
  char *szAreaKeys=trk_rep_keys;
  int ch;

  NW(t);

  WhiteN();


  /* Don't ask questions if we have already been given this information */

  if (!prep->uiArea)
  {
    *prep->szWildcard=0;

    do
    {
      ch=toupper(KeyGetRNP(trk_rep_txt));
    }
    while (!strchr(szAreaKeys, ch) && ch && ch != 13);

    if (ch==szAreaKeys[0])
      prep->uiArea=RF_AREA_CUR;
    else if (ch==szAreaKeys[1])
      prep->uiArea=RF_AREA_TAG;
    else if (ch==szAreaKeys[2])
      prep->uiArea=RF_AREA_GRP;
    else if (ch==szAreaKeys[3])
      prep->uiArea=RF_AREA_WLD;
    else if (ch==szAreaKeys[4] || ch==0 || ch==13)
      prep->uiArea=RF_AREA_ALL;
    else return FALSE;
  }

  if (prep->uiArea==RF_AREA_WLD && ! *prep->szWildcard)
  {
    WhiteN();
    InputGetsL(prep->szWildcard, PATHLEN-1, trk_rep_get_areaname);

    if (! *prep->szWildcard)
      return FALSE;
  }

  return TRUE;
}


/* Get a mask for the message status type */

static int near RepGetStatus(TRK t, PREP prep)
{
  char *szStatKeys=trk_rep_stat_keys;
  char szInput[PATHLEN], *p;

  NW(t);

  /* Ask no questions if we have already been given this information */

  if (prep->fStatus)
    return TRUE;

  WhiteN();
  InputGetsL(szInput, PATHLEN-1, trk_rep_stat_txt);
  strupr(szInput);

  prep->fStatus=0;

  if (! *szInput)
  {
    prep->fStatus=SF_NEW | SF_OPEN | SF_WORKING;
    return TRUE;
  }

  /* Process all of the option flags */

  for (p=szInput; *p; p++)
  {
    if (*p==szStatKeys[0])
      prep->fStatus |= SF_NEW;
    else if (*p==szStatKeys[1])
      prep->fStatus |= SF_OPEN;
    else if (*p==szStatKeys[2])
      prep->fStatus |= SF_WORKING;
    else if (*p==szStatKeys[3])
      prep->fStatus |= SF_CLOSED;
    else if (*p==szStatKeys[4])
      prep->fStatus |= SF_NEW | SF_OPEN | SF_WORKING | SF_CLOSED;
  }

  return !!prep->fStatus;
}


/* Get a priority mask for performing the search */

static int near RepGetPriority(TRK t, PREP prep)
{
  char *szPriKeys=trk_rep_pri_keys;
  char szInput[PATHLEN], *p;

  NW(t);

  /* Ask no questions if we have already been given this information */

  if (prep->fPriority)
    return TRUE;

  WhiteN();
  InputGetsL(szInput, PATHLEN-1, trk_rep_pri_txt);
  strupr(szInput);

  if (!*szInput)
  {
    prep->fPriority=PF_NOTIFY | PF_LOW | PF_NORMAL | PF_URGENT | PF_CRIT;
    return TRUE;
  }

  prep->fPriority=0;

  for (p=szInput; *p; p++)
  {
    if (*p==szPriKeys[0])
      prep->fPriority |= PF_NOTIFY;
    else if (*p==szPriKeys[1])
      prep->fPriority |= PF_LOW;
    else if (*p==szPriKeys[2])
      prep->fPriority |= PF_NORMAL;
    else if (*p==szPriKeys[3])
      prep->fPriority |= PF_URGENT;
    else if (*p==szPriKeys[4])
      prep->fPriority |= PF_CRIT;
    else if (*p==szPriKeys[5])
      prep->fPriority |= PF_NOTIFY | PF_LOW | PF_NORMAL | PF_URGENT | PF_CRIT;
  }

  return !!prep->fPriority;
}



/* Ask for an owner's name to find */

static int near RepGetOwner(TRK t, PREP prep)
{
  char szOwner[PATHLEN];

  /* If we have already been given this information, don't ask for anything *
   * else.                                                                  */

  if (*prep->to)
    return TRUE;

  /* If the user doesn't have enough priv to view other peoples' messages,  *
   * only show our own messages.                                            */

  if (!PrivOK(PRM(track_privview), FALSE))
  {
    prep->fOnlyOurs=TRUE;
    *prep->to=0;
    return TRUE;
  }

  /* Otherwise, ask for a list of users */

  do
  {
    WhiteN();
    InputGetsL(szOwner, PATHLEN-1, trk_rep_s_owner);

    if (eqstri(szOwner, qmark))
    {
      TrackValidOwner(t, szOwner, prep->to, TRUE);
      continue;
    }

    if (! *szOwner || eqstri(szOwner, asterisk))
    {
      strcpy(prep->to, asterisk);
      return TRUE;
    }
  }
  while (!TrackValidOwner(t, szOwner, prep->to, FALSE));

  return TRUE;
}



/* See if we are browsing a message from the right area */

static int near RepMatchArea(PREP prep, TRK_MSG_NDX *ptmn)
{
  char szGrp[PATHLEN];
  int uiGrpLen;

  MessageSection(ptmn->tl.szArea, szGrp);
  uiGrpLen = strlen(szGrp);

  return (prep->uiArea==RF_AREA_CUR && eqstri(ptmn->tl.szArea, usr.msg)) ||
         (prep->uiArea==RF_AREA_TAG && TagQueryTagList(&mtm, ptmn->tl.szArea)) ||
         (prep->uiArea==RF_AREA_GRP && (!uiGrpLen || strncmp(ptmn->tl.szArea, szGrp, uiGrpLen)==0)) ||
         (prep->uiArea==RF_AREA_WLD && BrowseWCMatch(prep->szWildcard, ptmn->tl.szArea)) ||
         (prep->uiArea==RF_AREA_ALL);
}


/* See if we are browsing a message of the right status type */

static int near RepMatchStatus(PREP prep, TRK_MSG_NDX *ptmn)
{
  return (ptmn->ts==TS_NEW && (prep->fStatus & SF_NEW)) ||
         (ptmn->ts==TS_OPEN && (prep->fStatus & SF_OPEN)) ||
         (ptmn->ts==TS_WORKING && (prep->fStatus & SF_WORKING)) ||
         (ptmn->ts==TS_CLOSED && (prep->fStatus & SF_CLOSED));
}


/* See if the status of this message matches the one that we are            *
 * looking for.                                                             */

static int near RepMatchPriority(PREP prep, TRK_MSG_NDX *ptmn)
{
  return (ptmn->tp==TP_NOTIFY && (prep->fPriority & PF_NOTIFY)) ||
         (ptmn->tp==TP_LOW && (prep->fPriority & PF_LOW)) ||
         (ptmn->tp==TP_NORMAL && (prep->fPriority & PF_NORMAL)) ||
         (ptmn->tp==TP_URGENT && (prep->fPriority & PF_URGENT)) ||
         (ptmn->tp==TP_CRIT && (prep->fPriority & PF_CRIT));
}

/* See if we are browsing messages owned by the right person */

static int near RepMatchOwner(PREP prep, TRK_MSG_NDX *ptmn)
{
  /* Handle the special case wherein we are only allowed to generate a      *
   * report on messages that we own ourselves.                              */

  if (prep->fOnlyOurs)
  {
    TRKLIST ptl;

    /* Scan the linked list for messages addressed to us */

    for (ptl=GetTrkList(); ptl; ptl=ptl->next)
      if (eqstri(ptl->to, ptmn->to))
        return TRUE;

    return FALSE;
  }

  return *prep->to=='*' || eqstri(prep->to, ptmn->to);
}


/* Ask if the user wants to pack the specified messages in a QWK packet */

static int near RepGetDownload(TRK t, PREP prep)
{
  NW(t);

  if (!prep->fDoQWK)
  {
    WhiteN();

    if (GetyNAnswer(trk_rep_dl_related_qwk, 0)==YES)
      prep->fDoQWK=QF_DO_PACK;
    else prep->fDoQWK=QF_DONT_PACK;
  }

  return TRUE;
}


/* See if this message matches all of our display criteria */

static int near RepMatch(PREP prep, TRK_MSG_NDX *ptmn)
{
  return RepMatchArea(prep, ptmn) &&
         RepMatchStatus(prep, ptmn) &&
         RepMatchPriority(prep, ptmn) &&
         RepMatchOwner(prep, ptmn);
}


/* This function returns TRUE if the message was written more than          *
 * a week ago.                                                              */

static int near EntryTooOld(SCOMBO *pscMsg)
{
  #define NUM_DAYS 7

  SCOMBO scNow;
  struct tm tmMsg, tmNow;
  time_t tNow, tMsg;

  Get_Dos_Date(&scNow);
  DosDate_to_TmDate(&scNow, &tmNow);
  DosDate_to_TmDate(pscMsg, &tmMsg);

  tNow=mktime(&tmNow);
  tMsg=mktime(&tmMsg);

  /* 60 secs in minute, 60 mins in hour, 24 hours in a day */

  return (tMsg < tNow - (60L * 60L * 24L * (long)NUM_DAYS));
}



/* Show an output line for the tracking report */

static unsigned near RepShow(TRK t, TRK_MSG_NDX *ptmn, FILE *fp, int *piFirst, char *pcNonstop)
{
  char buf[PATHLEN];
  char line[PATHLEN];

  if (fp)  /* if writing output to a file */
  {
    if (*piFirst)
    {
      fputs(trk_rep_hdr, fp);

      *piFirst=FALSE;
    }
  }
  else if (display_line==1 && !*pcNonstop)
  {
    Puts(trk_rep_hdr_graph);
  }

  /* Display the name/number of msg as "areaname#msgnum" */

  sprintf(buf, trk_rep_msgnum, ptmn->tl.szArea,
          (prm.flags2 & FLAG2_UMSGID)
           ? ptmn->tl.uid : MsgUidToMsgn(sq, ptmn->tl.uid, UID_EXACT));

  /*
  ID               Date     Owner Priority     Status  Message
  컴컴컴컴컴컴컴컴 컴컴컴컴 컴컴� 컴컴컴컴컴컴 컴컴컴� 컴컴컴컴컴컴컴컴컴컴컴컴컴
  9307231122330001 93-07-23 SJD   Notification Working DISCUSS.MMEDIA#421
                   From: Bob Uruguai, Subj: How to run OS/2 Device Drivers
                   Changed status from Open to Criticl on 07-23-93
                   Changed owner from mikg to sjd on 07-23-93
                   Comment: this user is a jerk and a dweeb
  9306241242330001 93-06-24 AL    Critical     New     DAP.SYSCMTS#334
                   From: Lawrie Nichols, Subj: What next?
  */




  sprintf(line, trk_rep_fmt,
          ptmn->szTrackID,
          (ptmn->scDateWritten.msg_st.date.yr + 80) % 100,
          ptmn->scDateWritten.msg_st.date.mo,
          ptmn->scDateWritten.msg_st.date.da,
          EntryTooOld((SCOMBO *)&ptmn->scDateWritten) ? '*' : ' ',
          ptmn->to,
          TrkGetPriority(t, ptmn),
          TrkGetStatus(t, ptmn),
          buf);

  /* Output to the appropriate place */

  if (fp)
    fputs(line, fp);
  else
  {
    Puts(line);

    if (MoreYnBreak(pcNonstop, CYAN))
      return FALSE;
  }

  return TRUE;
}




/* Create a browse handle for use while doing the report */

static BROWSE * RepConstructBrowse(PREP prep, char *nonstop)
{
  BROWSE *b;

  if ((b=malloc(sizeof(*b)))==NULL)
  {
    logit(mem_none);
    return NULL;
  }

  memset(b, 0, sizeof *b);

  /* Fill out the structure */

  b->first=NULL;
  b->nonstop=nonstop;
  b->menuname=menuname;
  b->wildcard=NULL;

  b->Begin_Ptr=QWK_Begin;
  b->Status_Ptr=QWK_Status;
  b->Idle_Ptr=QWK_Idle;
  b->Display_Ptr=QWK_Display;
  b->After_Ptr=QWK_After;
  b->End_Ptr=QWK_End;
  b->Match_Ptr=Match_All;
  b->fSilent=TRUE;

  /* Rev up the .QWK packer */

  if (QWK_Begin(b)==-1)
  {
    free(b);
    return NULL;
  }

  /* First msg packed in area */

  prep->fFirstArea=TRUE;

  return b;
}


/* Try to switch the current area to the one specified by ptmn->tl */

static unsigned near RepNewArea(PREP prep, TRK_MSG_NDX *ptmn, BROWSE *b)
{
  BARINFO bi;

  if (!prep->fFirstArea)
    if (QWK_After(b)==-1)
      return FALSE;

  /* Now switch the physical message area */

  memset(&bi, 0, sizeof bi);

  if (!PopPushMsgArea(ptmn->tl.szArea, &bi))
  {
    prep->fFirstArea=TRUE;
    return FALSE;
  }

  if (QWK_Status(b, ptmn->tl.szArea, 0)==-1)
    return FALSE;

  prep->fFirstArea=FALSE;
  return TRUE;
}


/* Pack the specified message into a .QWK packet */

static unsigned near RepQWKPack(PREP prep, TRK_MSG_NDX *ptmn, BROWSE *b)
{
  unsigned rc;

  /* If we need to change message areas... */

  if (prep->fFirstArea || !eqstri(ptmn->tl.szArea, MAS(mah, name)))
    if (!RepNewArea(prep, ptmn, b))
      return TRUE;

  /* Now fill out the parts of this struct that are specific to this area */

  b->sq=sq;
  b->type=mah.ma.type;
  b->path=MAS(mah, path);

  /* Convert the UMSGID into a real message number for the browse subsystem */

  b->bdata=MsgUidToMsgn(sq, ptmn->tl.uid, UID_EXACT);

  /* If we can't find this message, just keep on going */

  if (!b->bdata)
    return TRUE;

  rc=FALSE;


  /* Open the message and pack it into the .QWK packet */

  if ((b->m=MsgOpenMsg(sq, MOPEN_READ, b->bdata)) != NULL)
  {
    if (MsgReadMsg(b->m, &b->msg, 0L, 0L, NULL, 0L, NULL) != -1)
      rc=QWK_Display(b) != -1;

    MsgCloseMsg(b->m);
  }

  return rc;
}


/* Finish a QWK downloading/report session */

static unsigned near RepFinishBrowse(PREP prep, BROWSE *b)
{
  unsigned rc;

  if (!prep->fFirstArea)
    QWK_After(b);

  rc=QWK_End(b) != -1;
  free(b);

  return rc;
}

/* Now that we have all of the required options, do a lookup on the         *
 * tracking database.                                                       */

static int near RepDoLookup(TRK t, PREP prep, FILE *fp)
{
  BROWSE *b;
  PALIST *ppl;
  TRK_MSG_NDX tmn;
  char nonstop=FALSE;
  unsigned rc=TRUE;
  int first=TRUE;
  int fDidPush=FALSE;

  if (prep->fDoQWK==QF_DO_PACK)
  {
    /* Create a browse handle for this session */

    if ((b=RepConstructBrowse(prep, &nonstop))==NULL)
      return FALSE;

    /* Push the current area on the stack */

    if (PushMsgArea(usr.msg, 0))
      fDidPush=TRUE;
  }

  ppl=PalistNew();

  WhiteN();
  display_line=1;

  while (TrkLookupMsgI(t, NULL, NULL, NULL, ppl, &tmn,
                       prep->fDoQWK==QF_DO_PACK ? 2 : 0))
  {
    /* Only display valid messages */

    if (!RepMatch(prep, &tmn))
      continue;

    if (!RepShow(t, &tmn, fp, &first, &nonstop))
      break;

    if (prep->fDoQWK==QF_DO_PACK)
      if (!RepQWKPack(prep, &tmn, b))
        break;
  }


  /* Stop this lookup session */

  PalistDelete(ppl);


  /* Finish the QWK download, if necessary */

  if (prep->fDoQWK==QF_DO_PACK)
  {
    if (!RepFinishBrowse(prep, b))
      rc=FALSE;

    if (fDidPush)
      PopMsgArea();
  }

  return rc;
}


/* Ask questions about the type of message we want */

static int near TrackMenuReportInternal(TRK t, PREP prep, FILE *fp)
{
  static int doing_report=FALSE;
  unsigned rc;

  /* We are non-reentrant! */

/*  if (doing_report)
    return FALSE;*/

  if (!RepGetArea(t, prep))
    return FALSE;

  if (!RepGetStatus(t, prep))
    return FALSE;

  if (!RepGetPriority(t, prep))
    return FALSE;

  if (!RepGetOwner(t, prep))
    return FALSE;

  if (!RepGetDownload(t, prep))
    return FALSE;

  /* Now perform the actual search */

  doing_report=TRUE;
  rc=RepDoLookup(t, prep, fp);
  doing_report=FALSE;

  return rc;
}


/* Generate a report on tracked messages, for inclusion in a .QWK mail      *
 * packet.                                                                  */

int TrackReportOurMessages(char *qwk_path)
{
  char fname[PATHLEN];
  int rc=FALSE;
  FILE *fp;
  TRK t;

  /* If we aren't a database moderator, no need to generate a report */

  if (!GetTrkList())
    return FALSE;

  if ((t=TrackGet()) != NULL)
  {
    REP rep=
    {
      RF_AREA_ALL,
      SF_NEW | SF_OPEN | SF_WORKING,
      PF_NOTIFY | PF_LOW | PF_NORMAL | PF_URGENT | PF_CRIT,
      QF_DONT_PACK,
      "*",
      TRUE
    };

    /* Generate the message report as bulletin #99. */

    sprintf(fname, trk_rep_qwk_bulletin, qwk_path);

    if ((fp=fopen(fname, fopen_write))==NULL)
      cant_open(fname);
    else
    {
      Puts(trk_rep_generating);
      vbuf_flush();
      rc=TrackMenuReportInternal(t, &rep, fp);
      fclose(fp);

      if (rc)
        Putc('\n');
      else
      {
        Printf(trk_rep_gen_err);
        unlink(fname);
      }
    }

    TrackRelease(t);
  }

  return rc;
}

/* Generate a report from the tracking database */

int TrackMenuReport(void)
{
  TRK t;
  int rc=FALSE;

  if ((t=TrackGet()) != NULL)
  {
    REP rep;

    memset(&rep, 0, sizeof rep);
    rc=TrackMenuReportInternal(t, &rep, NULL);
    TrackRelease(t);
  }

  return rc;
}


#endif

