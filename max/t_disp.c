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
static char rcs_id[]="$Id: t_disp.c,v 1.1.1.1 2002/10/01 17:53:13 sdudley Exp $";
#pragma on(unreferenced)

#define MAX_LANG_m_area
#include "trackp.h"

#ifdef MAX_TRACKER

extern char szTrk[];

/* This function is called when Max has identified that the user in         *
 * question is the owner of a message, after that message has been          *
 * displayed.                                                               */

static void near TrackDisplayStatus(TRK_MSG_NDX *ptmn, int update_status)
{
  TRK t;
  char szOwner[PATHLEN];

  if ((t=TrackGet())==NULL)
    return;

  if (!TrkGetOwner(t, ptmn->to, szOwner))
    strcpy(szOwner, ptmn->to);

  Printf(trk_msg_info,
         " "+!(usr.bits & BITS_FSR),
         TrkGetStatus(t, ptmn), TrkGetPriority(t, ptmn), szOwner,
         " "+!(usr.bits & BITS_FSR));

  /* If the message was originally a new message, change its status         *
   * to "open" since we have now seen it.                                   */

  if (update_status)
    TrackOwnerReadMsg(t, ptmn);

  TrackRelease(t);
}




/* This is called when a user finishes reading a message on-line */

void TrackShowInfo(HAREA ha, PMAH pmah, XMSG *pxmsg, dword msgnum, char *kludges, int row, int col, char *colour)
{
  char *new_kludges=NULL;
  TRK_MSG_NDX tmn;
  int update_status;

  /* Add a remotely-originated message to the database, if necessary */

  if (TrackNeedToInsertRemoteMsg(pmah, pxmsg, kludges))
  {
    char add_kludge[PATHLEN];
    new_kludges=malloc((kludges ? strlen(kludges) : 0)+PATHLEN);

    if (!new_kludges)
      logit(log_trk_kludges);

    /* Add the actrack line to the existing kludges */

    if (new_kludges && TrackInsertMessage(ha, msgnum, blank_str, add_kludge))
    {
      if (kludges)
        strcpy(new_kludges, kludges);
      else *new_kludges=0;

      Strip_Trailing(new_kludges, '\x01');
      strcat(new_kludges, add_kludge);

      /* Set it so that the trackareweowner routine checks the new kludges */

      kludges=new_kludges;
    }
  }

  if (TrackAreWeOwner(kludges, &tmn, &update_status))
  {
    Goto(row, col);
    Puts(colour);

    TrackDisplayStatus(&tmn, update_status);
    Puts(msg_text_col);
    Putc('\n');
  }

  /* Free the modified kludges */

  if (new_kludges)
    free(new_kludges);
}


#endif

