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

/* Private header for t_*.c files */

#ifndef __TRACKP_H_DEFINED
#define __TRACKP_H_DEFINED

#define MAX_LANG_track

#include <string.h>
#include <ctype.h>
#include <io.h>
#include "mm.h"
#include "max_msg.h"
#include "trackc.h"
#include "trackm.h"

typedef struct
{
  #define RF_AREA_CUR     0x0001
  #define RF_AREA_TAG     0x0002
  #define RF_AREA_ALL     0x0003
  #define RF_AREA_GRP     0x0004
  #define RF_AREA_WLD     0x0005

  unsigned int uiArea;                    /* Area type */

  #define SF_NEW          0x0001
  #define SF_OPEN         0x0002
  #define SF_WORKING      0x0004
  #define SF_CLOSED       0x0008

  unsigned int fStatus;                   /* Status types */

  #define PF_NOTIFY       0x0001
  #define PF_LOW          0x0002
  #define PF_NORMAL       0x0004
  #define PF_URGENT       0x0008
  #define PF_CRIT         0x0010

  unsigned int fPriority;                 /* Priority types */

  #define QF_DO_PACK      0x0001
  #define QF_DONT_PACK    0x0002

  unsigned int fDoQWK;                    /* Put msgs in QWK packet */

  TRK_OWNER to;                           /* Owner to search for */
  unsigned fOnlyOurs;                     /* Only display msgs owned by us */

  char szWildcard[PATHLEN];               /* Wildcard for RF_AREA_WLD */
  unsigned fFirstArea;                    /* Processing the first area */

} REP, *PREP;



/* Linked list for storing our own user's owner aliases */

typedef struct _trklist
{
  TRK_OWNER to;
  struct _trklist *next;
} *TRKLIST;


typedef struct _excludelist
{
  char *pszName;
  struct _excludelist *next;
} *EXCLIST;

int TrackAskOwner(TRK t, TRK_OWNER to);
int TrackAddRemoteMsgToDatabase(PMAH pmah, HAREA ha, dword msgnum, char *out_actkludge);
int TrackNeedToInsertRemoteMsg(PMAH pmah, XMSG *pxmsg, char *kludges);
TRKLIST GetTrkList(void);
int TrackInsertTracking(HAREA ha, dword msgnum, XMSG *pxmsg,
                        int modify_actrack, int add_act, char *actrack,
                        int modify_audit, char *audit,
                        int modify_comment, char *comment);
int TrackMenuReport(void);
int TrackInsertMessage(HAREA ha, dword msgnum, TRK_OWNER to, char *out_actkludge);
void TrackMakeACAUDIT(char *out, char *fmt, ...);
int IsUserExcluded(char *szName);
TRK TrackGet(void);
void TrackRelease(TRK t);
int TrackAreWeOwnerOfActrack(char *actrack, TRK_MSG_NDX *ptmn,
                             int *pupdate_status);

#endif /* __TRACKP_H_DEFINED */


