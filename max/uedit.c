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
static char rcs_id[]="$Id: uedit.c,v 1.1.1.1 2002/10/01 17:53:18 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Internal user editor
*/

#define INIT_UED
#define MAX_LANG_max_ued

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "alc.h"
#include "mm.h"
#include "ued.h"






static void near ClearScreenBottom(void)
{
  if (*linebuf==0)
  {
    Goto(PROMPT_LINE,   1);
    Puts(CLEOS);
  }
  else Goto(PROMPT_LINE, 1);
}



typedef struct
{
  int ch;
  void (*pf)(void);
} KEYHANDLER;

static KEYHANDLER khUser[]=
{
  {'N', UedGetName},
  {'C', UedGetCity},
  {'P', UedGetPwd},
  {'A', UedGetAlias},
  {'V', UedGetVoicePhone},
  {'D', UedGetDataPhone},
  {'S', UedGetSex},
  {'B', UedGetBday},
  {0,   0}
};

static KEYHANDLER khAccess[]=
{
  {'P', UedGetPriv},
  {'K', UedGetKeys},
  {'G', UedGetGroup},
  {'A', UedGetPointCredit},
  {'C', UedGetCredit},
  {'D', UedGetDebit},
  {'S', UedGetShowUlist},
  {'U', UedGetPointDebit},
  {'N', UedGetNerd},
  {0,   0}
};

static KEYHANDLER khFlags[]=
{
  {'H', UedGetHotkeys},
  {'T', UedGetTabs},
  {'I', UedGetIBMChars},
  {'P', UedGetPause},
  {'C', UedGetCalledBefore},
  {'S', UedGetScrnClr},
  {'A', UedGetAvailChat},
  {'F', UedGetFSR},
  {'M', UedGetMaxed},
  {'R', UedGetRIP},
  {0,   0}
};


static KEYHANDLER khSettings[]=
{
  {'W', UedGetWidth},
  {'L', UedGetLength},
  {'N', UedGetNulls},
  {'M', UedGetMsgArea},
  {'F', UedGetFileArea},
  {'V', UedGetVideo},
  {'H', UedGetHelp},
  {'!', UedGetLanguage},
  {'P', UedGetProtocol},
  {'C', UedGetCompress},
  {0,   0}
};

static KEYHANDLER khInformation[]=
{
  {'D', UedGetDl},
  {'T', UedGetTodayDl},
  {'U', UedGetUploads},
  {'P', UedGetPostMsgs},
  {'1', UedGet1stCall},
  {'C', UedGetCurTime},
  {'A', UedGetAddedTime},
  {'#', UedGetNumCalls},
  {'R', UedGetReadMsgs},
  {'!', UedGetPwdDate},
  {0,   0}
};

static KEYHANDLER khExpiry[]=
{
  {'E', UedGetExpireBy},
  {'A', UedGetExpireAction},
  {'D', UedGetExpireDate},
};


static void near UedKeyHandler(KEYHANDLER *pkhMenu, char *prompt)
{
  KEYHANDLER *pkh;
  int cmd;

  Goto(PROMPT_LINE, 1); Puts(CLEOL);

  cmd=Input_Char(CINPUT_PROMPT | CINPUT_NOXLT | CINPUT_DUMP,
                 prompt);

  Goto(PROMPT_LINE, 1); Puts(CLEOL);

  cmd=toupper(cmd);

  for (pkh=pkhMenu; pkh->ch; pkh++)
    if (cmd==pkh->ch)
    {
      (*pkh->pf)();
      break;
    }
}


void User_Edit(char *search)
{
  int cmd;

  if (Init_Ued()==-1)
    return;

  cmd='\0';

  if (search)
  {
    strcpy(linebuf, search);
    UedFindUser(TRUE,TRUE);
    ClearScreenBottom();
    search=NULL;
  }
  else
  {
    UedFindFirstUser();
  }

  while (cmd != 'Q')
  {
    DisplayUser();

    Puts(WHITE);
    Goto(PROMPT_LINE, 1);

    cmd=Input_Char(CINPUT_PROMPT | CINPUT_NOXLT | CINPUT_DUMP, ued_prompt);

    if (toupper(cmd) != 'Q')
    {
      Goto(PROMPT_LINE, 1);
      Puts(CLEOL);
    }
    else if (usr.video==GRAPH_TTY && (usr.bits & BITS_HOTKEYS))
      Putc('\n');

    switch (cmd=toupper(cmd))
    {
      case 'U':   UedKeyHandler(khUser, ued_user_prompt);         break;
      case 'A':   UedKeyHandler(khAccess, ued_access_prompt);     break;
      case 'I':   UedKeyHandler(khInformation, ued_info_prompt);  break;
      case 'F':   UedKeyHandler(khFlags, ued_flags_prompt);       break;
      case 'S':   UedKeyHandler(khSettings, ued_settings_prompt); break;
      case 'E':   UedKeyHandler(khExpiry, ued_expiry_prompt);     break;
      case ']':   /* fall-through */
      case '+':   UedPlus();                    break;
      case '[':   /* fall-through */
      case '-':   UedMinus();                   break;
      case '~':   UedFindUser(TRUE, FALSE);     break;
      case '`':   UedFindUser(FALSE, FALSE);    break;
      case 'C':   if (Add_User()==-1) cmd='Q';  break;
      case 'D':   UedDelete();                  break;
      case 'J':   UedLast();                    break;

      case '"':   UedUndoChanges();             break;
      case '=':   disp_pwd=!disp_pwd;           break;
      case '/':   DrawUserScreen();             break;
      case '|':   UedPurgeUsers();              break;
      case '?':   UedShowHelp();                break;
      case '\'':
      case '\x0d':
      case '\x00':
      case 'Q':                                 break;
    }

    ClearScreenBottom();
  }

  Update_User();
  
  UserFileClose(huf);
  free(find_name);
}

