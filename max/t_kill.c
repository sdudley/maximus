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
static char rcs_id[]="$Id: t_kill.c,v 1.1.1.1 2002/10/01 17:53:13 sdudley Exp $";
#pragma on(unreferenced)

#include "trackp.h"

#ifdef MAX_TRACKER

extern char szTrk[];

/* We are deleting a message with the specified control information,        *
 * so remove it from the tracking database if necessary.                    */

int TrackKillMsg(char *ctrl)
{
  char *actrack=MsgGetCtrlToken(ctrl, actrack_colon);
  TRK t;

  if (!actrack)
    return FALSE;

  if ((t=TrackGet()) != NULL)
  {
    TRK_MSG_NDX tmn;

    if (TrkLookupMsg(t, actrack+9, NULL, NULL, NULL, &tmn))
    {
      if (!TrkDeleteMsg(t, &tmn))
        Printf(trk_err_removing, tmn.szTrackID);
    }

    TrackRelease(t);
  }

  MsgFreeCtrlToken(actrack);
  return TRUE;
}

#endif /* MAX_TRACKER */

