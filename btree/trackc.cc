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
static char rcs_id[]="$Id: trackc.cc,v 1.1.1.1 2002/10/01 17:49:31 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include "dbase.h"
#include "track.h"
#include "trackc.h"
#include "prog.h"
#include "growhand.h"


TRK BEXPENTRY TrkOpen(char *szName, unsigned fNewFile)
{
  grow_handles(40);

  TRK t=new TRACKER;

  // Try to open the tracking file with the specified parameters

  if (t && !t->open(szName, fNewFile))
  {
    delete t;
    t=NULL;
  }

  return t;
}

void BEXPENTRY TrkClose(TRK t)
{
  delete t;
}

int BEXPENTRY TrkAddMsg(TRK t, TRK_MSG_NDX *ptmn)
{
  return t->AddMsg(ptmn);
}

char * BEXPENTRY TrkGetStatus(TRK t, TRK_MSG_NDX *ptmn)
{
  return t->GetStatus(ptmn);
}

char * BEXPENTRY TrkGetPriority(TRK t, TRK_MSG_NDX *ptmn)
{
  return t->GetPriority(ptmn);
}

int   BEXPENTRY TrkLookupMsg(TRK t, char *szTrackID, char *szOwner, char *szLocation, PALIST *ppl, TRK_MSG_NDX *ptmn)
{
  return t->LookupMsg(szTrackID, szOwner, szLocation, ppl, ptmn);
}

int   BEXPENTRY TrkLookupMsgI(TRK t, char *szTrackID, char *szOwner, char *szLocation, PALIST *ppl, TRK_MSG_NDX *ptmn, unsigned uiIdx)
{
  return t->LookupMsg(szTrackID, szOwner, szLocation, ppl, ptmn, uiIdx);
}

int   BEXPENTRY TrkSetOwner(TRK t, TRK_OWNER to, char *szOwner)
{
  return t->SetOwner(to, szOwner);
}

int   BEXPENTRY TrkGetOwner(TRK t, TRK_OWNER to, char *szOwner)
{
  return t->GetOwner(to, szOwner);
}

int   BEXPENTRY TrkSetDefaultOwner(TRK t, char *szArea, TRK_OWNER to)
{
  return t->SetDefaultOwner(szArea, to);
}

int   BEXPENTRY TrkGetDefaultOwner(TRK t, char *szArea, TRK_OWNER to)
{
  return t->GetDefaultOwner(szArea, to);
}

BTREE * BEXPENTRY TrkGetOwnerBtree(TRK t)
{
  return t->GetOwnerBtree();
}

BTREE * BEXPENTRY TrkGetAreaBtree(TRK t)
{
  return t->GetAreaBtree();
}

DBASE * BEXPENTRY TrkGetMsgDbase(TRK t)
{
  return t->GetMsgDbase();
}


int   BEXPENTRY TrkUpdateMsg(TRK t, TRK_MSG_NDX *ptmnOld, TRK_MSG_NDX *ptmnNew)
{
  return t->UpdateMsg(ptmnOld, ptmnNew);
}

int   BEXPENTRY TrkDeleteMsg(TRK t, TRK_MSG_NDX *ptmn)
{
  return t->DeleteMsg(ptmn);
}

char * BEXPENTRY TrkGetAreaOwner(TRK t, char *szArea)
{
  return t->GetAreaOwner(szArea);
}

