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

/* $Id: api_sdmp.h,v 1.2 2003/06/11 14:03:06 wesgarland Exp $ */

static sword MAPIENTRY SdmCloseArea(HAREA mh);
static HMSG  MAPIENTRY SdmOpenMsg(HAREA mh,word mode,dword msgnum);
static sword MAPIENTRY SdmCloseMsg(HMSG msgh);
static dword MAPIENTRY SdmReadMsg(HMSG msgh,PXMSG msg,dword offset,dword bytes,byte *text,dword clen,byte *ctxt);
static sword MAPIENTRY SdmWriteMsg(HMSG msgh,word append,PXMSG msg,byte *text,dword textlen,dword totlen,dword clen,byte *ctxt);
static sword MAPIENTRY SdmKillMsg(HAREA mh,dword msgnum);
static sword MAPIENTRY SdmLock(HAREA mh);
static sword MAPIENTRY SdmUnlock(HAREA mh);
static sword MAPIENTRY SdmSetCurPos(HMSG msgh,dword pos);
static dword MAPIENTRY SdmGetCurPos(HMSG msgh);
static UMSGID MAPIENTRY SdmMsgnToUid(HAREA mh,dword msgnum);
static dword MAPIENTRY SdmUidToMsgn(HAREA mh,UMSGID umsgid,word type);
static dword MAPIENTRY SdmGetHighWater(HAREA mh);
static sword MAPIENTRY SdmSetHighWater(HAREA sq,dword hwm);
static dword MAPIENTRY SdmGetTextLen(HMSG msgh);
static dword MAPIENTRY SdmGetCtrlLen(HMSG msgh);
static UMSGID MAPIENTRY SdmGetNextUid(HAREA ha);

static void Convert_Fmsg_To_Xmsg(struct _omsg *fmsg,PXMSG msg,word def_zone);
static void Convert_Xmsg_To_Fmsg(PXMSG msg,struct _omsg *fmsg);
static void Init_Xmsg(PXMSG msg);
static sword near _SdmRescanArea(HAREA mh);
static sword near _Grab_Clen(HMSG msgh);
static void MAPIENTRY WriteToFd(byte OS2FAR *str);
static void near Get_Binary_Date(union _stampu *todate, union _stampu *fromdate,byte *asciidate);


static int statfd; /* file handle for WriteToFd */
static byte *sd_msg="%s%u.msg";

/* Pointer to 'struct _sdmdata' so we can get Turbo Debugger to use         *
 * the _sdmdata structure...                                                */

#ifdef __TURBOC__
static struct _sdmdata *_junksqd;
#endif

static struct _apifuncs sdm_funcs=
{
  SdmCloseArea,
  SdmOpenMsg,
  SdmCloseMsg,
  SdmReadMsg,
  SdmWriteMsg,
  SdmKillMsg,
  SdmLock,
  SdmUnlock,
  SdmSetCurPos,
  SdmGetCurPos,
  SdmMsgnToUid,
  SdmUidToMsgn,
  SdmGetHighWater,
  SdmSetHighWater,
  SdmGetTextLen,
  SdmGetCtrlLen,
  SdmGetNextUid
};

