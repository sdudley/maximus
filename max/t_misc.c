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
static char rcs_id[]="$Id: t_misc.c,v 1.2 2004/01/11 19:43:21 wmcbrine Exp $";
#pragma on(unreferenced)

/*#define TRACK_PERSISTENT*/  /* Only open tracking database once only */

#include <stdarg.h>
#include "trackp.h"

#ifdef MAX_TRACKER /* only process this file if doing the tracking system */

static char *szTrk;
static TRKLIST tl=NULL;   /* Instance of our alias list */
static EXCLIST el=NULL;


#ifdef TRACK_PERSISTENT

  static TRK trkPersistent;

  TRK TrackGet(void)
  {
    return trkPersistent;
  }

  void TrackRelease(TRK t)
  {
    NW(t);
  }

#else

  TRK TrackGet(void)
  {
    TRK t;

    if ((t=TrkOpen(szTrk, FALSE))==NULL)
    {
      logit(log_cant_open_trk);
      return NULL;
    }

    return t;
  }


  /* Release access to the tracking system */

  void TrackRelease(TRK t)
  {
    TrkClose(t);
  }

#endif


/* Get the current tracking list for areas owned by us */

TRKLIST GetTrkList(void)
{
  return tl;
}


/* Get the list of our own user's aliases */

static void near GetUsList(TRK t)
{
  BTREE *pbt;
  PALIST *ppl;
  TRK_OWNER_NDX *pton;

  pbt=TrkGetOwnerBtree(t);
  ppl=PalistNew();

  /* Scan all of the records in the owner database, creating a linked list  *
   * of all owners that represent us.                                       */

  while ((pton=BtLookup(pbt, NULL, ppl)) != NULL)
    if (eqstri(pton->szOwner, usr.name))
    {
      TRKLIST ptl=malloc(sizeof(*ptl));

      if (ptl)
      {
        strcpy(ptl->to, pton->to);
        ptl->next=tl;
        tl=ptl;
      }
    }

  PalistDelete(ppl);
}



/* This function returns TRUE if the named user should be excluded from     *
 * having messages entered in the tracking database.                        */

int IsUserExcluded(char *szName)
{
  EXCLIST pel;

  for (pel=el; pel; pel=pel->next)
    if (eqstri(szName, pel->pszName))
      return TRUE;

  return FALSE;
}

/* Get the list of people to exclude from the tracking system */

static void near GetExcludeList(void)
{
  FILE *fp;
  char line[PATHLEN];
  char *name=PRM(track_exclude);

  if (!*name)
    return;

  /* Open the exclusion list */

  if ((fp=fopen(name, fopen_read))==NULL)
    return;

  /* Read in each line */

  while (fgets(line, PATHLEN, fp))
  {
    EXCLIST pel;

    Strip_Trailing(line, '\n');

    /* Add this user to the linked list */

    if ((pel=malloc(sizeof *pel)) != NULL)
    {
      pel->pszName=strdup(line);
      pel->next=el;
      el=pel;
    }
  }

  fclose(fp);
}




/* Initialize the tracking subsystem */

void InitTracker(void)
{
  char *szNewTrack=PRM(track_base);
  TRK t;

  /* If the user has specified a different name for the tracking system,    *
   * use it instead of just "TRK".                                          */

  if (*szNewTrack)
    szTrk = szNewTrack;
  else
    szTrk = "trk";

  if ((t=TrkOpen(szTrk, TRUE))==NULL)
  {
    logit(log_cant_open_trk);
    return;
  }
  
  /* Get the list of our names, plus the list of people to exclude */

  GetUsList(t);
  GetExcludeList();

#ifdef TRACK_PERSISTENT
  trkPersistent=t;
#else
  TrkClose(t);
#endif
}



/* Deallocate the memory used by the tracking subsystem */

void DeinitTracker(void)
{
  TRKLIST ptl, ptlnext;
  EXCLIST pel, pelnext;

  /* Free the 'owner' linked list */

  for (ptl=tl; ptl; ptlnext=ptl->next, free(ptl), ptl=ptlnext)
    ;

  /* Free the 'exclusion' linked list */

  for (pel=el; pel; pelnext=pel->next, free(pel), pel=pelnext)
    if (pel->pszName)
      free(pel->pszName);

#ifdef TRACK_PERSISTENT

  if (trkPersistent)
    TrkClose(trkPersistent);

  trkPersistent=0;
#endif
}

/* Determine whether or not we "own" a particular ACTRACK: message */

int TrackAreWeOwner(char *kludges, TRK_MSG_NDX *ptmn, int *pupdate_status)
{
  char *p;
  int rc;

  if (pupdate_status)
    *pupdate_status=FALSE;

  /* If we are not related to any owner token, return FALSE right away */

  p=PRM(track_privview);

  if (*p==0 || (!tl && !PrivOK(p, FALSE)))
    return FALSE;

  if (!kludges || (p=MsgGetCtrlToken(kludges, actrack_colon))==NULL)
    return FALSE;

  rc = TrackAreWeOwnerOfActrack(p+9, ptmn, pupdate_status);

  MsgFreeCtrlToken(p);

  return rc;
}


/* Determine if the user owns (or is allowed to access) a specific ACTRACKed
 * message.  Pass in the ACTRACK id of the message, not the kludge lines.
 */

int TrackAreWeOwnerOfActrack(char *actrack, TRK_MSG_NDX *ptmn,
                             int *pupdate_status)
{
  TRK t;
  TRK_MSG_NDX tmn;
  int rc=FALSE;

  if ((t=TrackGet())==NULL)
    return FALSE;

  /* Try to find msg in tracking database */

  if (TrkLookupMsg(t, actrack, NULL, NULL, NULL, &tmn))
  {
    TRKLIST ptl;

    for (ptl=tl; ptl; ptl=ptl->next)
    {
      if (eqstri(ptl->to, tmn.to))
      {
        if (pupdate_status)
          *pupdate_status=TRUE;

        rc=TRUE;
        break;
      }
    }

    /* If user has high enough priv level, let him/her see it anyway */

    if (PrivOK(PRM(track_privview), FALSE))
      rc=TRUE;
  }

  TrackRelease(t);

  /* If the caller requested a copy of the msg, give it to them */

  if (rc && ptmn)
    memmove(ptmn, &tmn, sizeof tmn);

  return rc;
}

/* Determines if szOwner is a valid owner, and if so, place alias in 'to' */
/* If do_list is true, we simply just list valid owners. */

int TrackValidOwner(TRK t, char *szOwner, TRK_OWNER to, int do_list)
{
  BTREE *pbt;
  PALIST *ppl;
  TRK_OWNER_NDX *pton;

  if (do_list)
    Puts(trk_list_hdr);

  *to=0;

  pbt=TrkGetOwnerBtree(t);
  ppl=PalistNew();

  while ((pton=BtLookup(pbt, NULL, ppl)) != NULL)
  {
    if (do_list)
      Printf(trk_list_type, pton->to, pton->szOwner);
    else if (eqstri(pton->szOwner, szOwner) || eqstri(pton->to, szOwner))
    {
      strcpy(szOwner, pton->szOwner);
      strcpy(to, pton->to);
      break;
    }
  }

  PalistDelete(ppl);

  return !!*to;
}


/* Make an ACAUDIT string based on the given information */

void TrackMakeACAUDIT(char *out, char *fmt, ...)
{
  va_list va;
  SCOMBO sc;

  Get_Dos_Date(&sc);

  strcpy(out, "ACAUDIT: [");
  sc_time(&sc, out + strlen(out));
  strcat(out, "] ");

  va_start(va, fmt);
  vsprintf(out + strlen(out), fmt, va);
  va_end(va);
}


#endif /* MAX_TRACKER */

