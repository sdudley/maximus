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

/* Public entrypoints to tracking subsystem */

#include "trackc.h"

/* Data structure passed between QWK and tracking routines */

typedef struct
{
  dword msgn;         /* Number of this msg (for trackqwkupdatetracking only */
  TRK_MSG_NDX tmn;    /* Record to insert in tracking database */
  int kill_after;     /* Delete this message after tossing */
  int update_comment; /* Update the comment for this msg? */

  char szNewComment[PATHLEN]; /* New comment for this msg */
} QWKTRACKINFO;

int TrackAfterReply(char *kludges, dword msgnum, HAREA ha);
int TrackAreWeOwner(char *kludges, TRK_MSG_NDX *ptmn, int *pupdate_status);
int TrackValidOwner(TRK t, char *szOwner, TRK_OWNER to, int do_list);
int TrackReportOurMessages(char *qwk_path);
int TrackKillMsg(char *ctrl);
void TrackOwnerReadMsg(TRK t, TRK_MSG_NDX *ptmn);
void InitTracker(void);
void DeinitTracker(void);
void TrackAddMessage(PMAH pmah, XMSG *msg, char *kludges, HAREA ha);
void TrackShowInfo(HAREA ha, PMAH pmah, XMSG *pxmsg, dword msgnum, char *kludges, int row, int col, char *colour);
int TrackNeedToAdd(PMAH pmah, XMSG *pxmsg);
void TrackMenu(void);
void GenerateActrack(XMSG *pxmsg, char *kend);

void TrackProcessACInfo(char *actinfo, QWKTRACKINFO *pqti);
int TrackQWKUpdateTracking(QWKTRACKINFO *pqti);
void TrackAddQWKFirst(TRK_MSG_NDX *ptmn, int *pdo_we_own, char *blpos, char *ctrl);
void TrackAddQWKTail(TRK_MSG_NDX *ptmn, char *blpos, char *ctrl);

