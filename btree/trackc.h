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

/* $Id: trackc.h,v 1.1.1.1 2002/10/01 17:49:31 sdudley Exp $ */

#ifndef __TRACKC_H_DEFINED
#define __TRACKC_H_DEFINED

#include "btype.h"

#ifdef __cplusplus
  #include "dbase.h"
#else
  #include "dbasec.h"
#endif

#include "trackcom.h"

#if 0
  #define OS2FAR far
#else
  #ifdef OS2FAR
    #undef OS2FAR
  #endif

  #define OS2FAR
#endif

#ifdef __cplusplus
  class TRACKER;
  typedef TRACKER OS2FAR *TRK;

extern "C"
{
#else
  typedef char OS2FAR *TRK;
#endif
  TRK BAPIENTRY TrkOpen(char *szFile, unsigned fNewFile);
  void BAPIENTRY TrkClose(TRK t);

  int   BAPIENTRY TrkAddMsg(TRK t, TRK_MSG_NDX OS2FAR *ptmn);
  char * BAPIENTRY TrkGetStatus(TRK t, TRK_MSG_NDX OS2FAR *ptmn);
  char * BAPIENTRY TrkGetPriority(TRK t, TRK_MSG_NDX OS2FAR *ptmn);
  char * BAPIENTRY TrkGetAreaOwner(TRK t, char *szArea);
  int   BAPIENTRY TrkLookupMsg(TRK t, char OS2FAR *szTrackID, char OS2FAR *szOwner, char OS2FAR *szLocation, PALIST OS2FAR *ppl, TRK_MSG_NDX OS2FAR *ptmn);
  int   BAPIENTRY TrkLookupMsgI(TRK t, char OS2FAR *szTrackID, char OS2FAR *szOwner, char OS2FAR *szLocation, PALIST OS2FAR *ppl, TRK_MSG_NDX OS2FAR *ptmn, unsigned uiIdx);
  int   BAPIENTRY TrkSetOwner(TRK t, TRK_OWNER OS2FAR to, char OS2FAR *szOwner);
  int   BAPIENTRY TrkGetOwner(TRK t, TRK_OWNER OS2FAR to, char OS2FAR *szOwner);
  int   BAPIENTRY TrkSetDefaultOwner(TRK t, char OS2FAR *szArea, TRK_OWNER OS2FAR to);
  int   BAPIENTRY TrkGetDefaultOwner(TRK t, char OS2FAR *szArea, TRK_OWNER OS2FAR to);
  int   BAPIENTRY TrkUpdateMsg(TRK t, TRK_MSG_NDX *ptmnOld, TRK_MSG_NDX *ptmnNew);
  int   BAPIENTRY TrkDeleteMsg(TRK t, TRK_MSG_NDX *ptmn);

  BTREE * BAPIENTRY TrkGetOwnerBtree(TRK t);
  BTREE * BAPIENTRY TrkGetAreaBtree(TRK t);
  DBASE * BAPIENTRY TrkGetMsgDbase(TRK t);

#ifdef __cplusplus
};
#endif

#endif // __TRACKC_H_DEFINED

