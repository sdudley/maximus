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

#define REMAP_BUF_SIZE  4096

static char *msgbuf;
static long n_packed;
static long n_notsent;
static FILE *trklog=NULL;

static void near PackIt(struct _cfgarea *ar);
static void EXPENTRY AddToMsgBuf(byte OS2FAR *str);
static int near GateRouteMessage(XMSG *msg,dword mn,NETADDR *olddest);
static unsigned near Pack_Netmail_Msg(HAREA sq,dword *mn, struct _cfgarea *ar);
static int near Send_Message(HMSG mh,XMSG *msg,dword bytes,dword mn, struct _cfgarea *ar);
static int near OkToForward(XMSG *msg);
static int near Remap_Message(XMSG *msg,dword mn);
static void near Point_To_Fakenet_Dest(XMSG *msg);
static void near Point_To_Fakenet_Orig(XMSG *msg);
static void near Process_AttReqUpd(XMSG *msg,char *filename,word manual);
static void near Process_OneAttReqUpd(XMSG *msg,char *filename, int tflag, char *orig_fspec, char *pwd);
static void near TrackMessage(XMSG *msg, byte *ctrl);
static void near AddViaLine(byte *msgbuf, byte *ctrl, XMSG xmsg);
static void near ExpandAndSend(XMSG *msg, int tflag, char *filename, char *pwd, char *szName);

