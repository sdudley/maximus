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

#ifndef __UED_H_DEFINED
#define __UED_H_DEFINED

#include "userapi.h"

#ifdef INIT_UED
  #define ued_extern
#else
  #define ued_extern extern
#endif

#define PROMPT_LINE 23

ued_extern struct _usr user;      /* Users currently being modified */
ued_extern struct _usr olduser;   /* Saved copy for 'undo' */

ued_extern HUF huf;               /* Handle to user file */
ued_extern int changes;           /* Has the user been modified? */
ued_extern int disp_pwd;          /* Display user's password? */

ued_extern char *find_name;       /* Name that we're looking for */

int Init_Ued(void);
int Find_Next_Lastread(HUF huf);
void UedLast(void);
void UedPlus(void);
void UedMinus(void);
void UedGetShowUlist(void);
void UedGetDebit(void);
void UedGetCredit(void);
void UedGetAlias(void);
void UedGetCity(void);
void UedGetName(void);
void UedGetVoicePhone(void);
void UedGetDataPhone(void);
void UedGetSex(void);
void UedGetBday(void);
void UedGetGroup(void);
void UedGetPointCredit(void);
void UedGetPointDebit(void);
void UedGetHotkeys(void);
void UedGetNerd(void);
void UedGetTabs(void);
void UedGetRIP(void);
void UedGetIBMChars(void);
void UedGetPause(void);
void UedGetCalledBefore(void);
void UedGetScrnClr(void);
void UedGetAvailChat(void);
void UedGetFSR(void);
void UedGetWidth(void);
void UedGetLength(void);
void UedGetNulls(void);
void UedGetMsgArea(void);
void UedGetFileArea(void);
void UedGetLanguage(void);
void UedGetProtocol(void);
void UedGetCompress(void);
void UedGetDl(void);
void UedGetTodayDl(void);
void UedGetUploads(void);
void UedGetPostMsgs(void);
void UedGet1stCall(void);
void UedGetCurTime(void);
void UedGetAddedTime(void);
void UedGetNumCalls(void);
void UedGetReadMsgs(void);
void UedGetPwdDate(void);
void UedGetHelp(void);
void UedGetKeys(void);
void UedGetVideo(void);
void UedGetPwd(void);
void UedGetPriv(void);
void UedShowHelp(void);
void UedFindLastUser(void);
void UedFindFirstUser(void);
void UedDelete(void);
int Add_User(void);
void UedFindUser(int begin,int exact);
void UedGetExpireBy(void);
void UedGetExpireAction(void);
void UedGetExpireDate(void);
void UedUndoChanges(void);
int UedPurgeUsers(void);
void DrawUserScreen(void);
void DisplayUser(void);
void UedGetMaxed(void);


#endif /* __UED_H_DEFINED */

