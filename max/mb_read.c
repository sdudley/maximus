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
static char rcs_id[]="$Id: mb_read.c,v 1.1.1.1 2002/10/01 17:52:17 sdudley Exp $";
#pragma on(unreferenced)

/*# name=R)ead function for the BROWSE command
*/

#define MAX_LANG_m_browse

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "max_msg.h"
#include "m_browse.h"

static int near Read_Get_Option(BROWSE *b);

int Read_Begin(BROWSE *b)
{
  NW(b);
  return 0;
}


int Read_Display(BROWSE *b)
{
  char savearea[MAX_ALEN];
  char *ctrl;

  long clen;

  int save_mlines;
  int ret;
  
  strcpy(savearea, usr.msg);
  strcpy(usr.msg, MAS(mah, name));
  
  do
  {
    byte was_no_remote_output=no_remote_output;

    if (hasRIP())
      no_remote_output=TRUE;

    if (hasRIP() || !usr.video || (usr.bits & BITS_FSR)==0)
    {
      Puts(CLS);
      Printf(br_area_banner,
             MAS(mah, name),
             MAS(mah, descript));
    }

    no_remote_output=was_no_remote_output;

    MsgSetCurPos(b->m, 0L);

    save_mlines=menu_lines;
    menu_lines=1;


    clen=MsgGetCtrlLen(b->m);

    if (clen <= 0)
      ctrl=NULL;
    else if ((ctrl=malloc((int)clen+5)) != NULL)
    {
      MsgReadMsg(b->m, NULL, 0L, 0L, NULL, clen, ctrl);
      ctrl[(int)clen]='\0';
    }

    Msg_Display(b->m,
                &b->msg,
                usr.bits2 & BITS2_MORE,
                MAS(mah, name),
                b->msgn,
                1,
                ctrl,
                TRUE);

    if (ctrl)
      free(ctrl);

    menu_lines=save_mlines;

    Recd_Msg(b->m, &b->msg, TRUE);

    /* Only update the lastread pointer if we're doing a read               *
     * all/new/from, and only if the message we're reading is               *
     * higher than the current lastread pointer for that area.              */

    if (OkToFixLastread(b))
      Lmsg_Set(b, b->msgn);
  }
  while ((ret=Read_Get_Option(b))==2);

  strcpy(usr.msg, savearea);
  
  return ret;
}


int Read_After(BROWSE *b)
{
  NW(b);
  return 0;
}



/* MsgInUnreceivedAttach
 *
 * Returns TRUE if the message currently selected by the BROWSE
 * structure is a file attach that has not yet been received.
 */

static int near MsgIsUnreceivedAttach(BROWSE *b)
{
  int rc;
  long clen;
  char *ctrl;

  rc=0;

  if ((b->msg.attr & MSGFILE)!=0)
  {
    clen=MsgGetCtrlLen(b->m);

    if (clen <= 0)
      ctrl=NULL;
    else if ((ctrl=malloc((int)clen+5)) != NULL)
    {
      MsgReadMsg(b->m, NULL, 0L, 0L, NULL, clen, ctrl);
      ctrl[(size_t)clen]='\0';
    }

    if (Msg_UnreceivedAttach(&b->msg, ctrl, !!(mah.ma.attribs&MA_NET)))
      rc=1;

    if (ctrl)
      free(ctrl);
  }

  return rc;
}


/* MsgAttachDownload
 *
 * This function is called as part of a browse to ask the user
 * whether or not he/she wants to download and/or kill the file
 * attached to the message indicated in 'b'.
 */

static void near MsgAttachDownload(BROWSE *b)
{
  long clen;
  char *ctrl;

  clen=MsgGetCtrlLen(b->m);

  if ((clen > 0) && (ctrl=malloc((int)clen+5)) != NULL)
  {
    int isnetmsg = !!(mah.ma.attribs&MA_NET);

    MsgReadMsg(b->m, NULL, 0L, 0L, NULL, clen, ctrl);
    ctrl[(size_t)clen]='\0';

    Msg_AttachDownload(&b->msg,ctrl,isnetmsg);

    if (Ask_KillAttach())
      Msg_AttachKill(&b->msg,ctrl,isnetmsg);

    free(ctrl);
  }  
}

static int near Read_Get_Option(BROWSE *b)
{
  UMSGID here;
  long savelast, new;
  char msginf[32];
  char temp[32];
  char prompt[PATHLEN*4];
  char *mkeys=mchk_keys;
  char *nkeys=mchk_nmsgk;
  int ch;

  for (;;)
  {
    WhiteN();
    
    display_line=display_col=1;

    /* sprintf()'d for possible positioning */

    sprintf(prompt, mchk_nmsg, TermLength()-1);
    strcpy(msginf, nkeys);

    if (GEPriv(usr.priv, prm.mc_reply_priv) &&
        CanAccessMsgCommand(&mah, msg_reply, 0))
    {
      temp[0]=mkeys[0];
      temp[1]=0;

      strcat(prompt, mchk_reply);
      strcat(msginf, temp);
    }

    if (MsgToThisUser(b->msg.to) &&
        MsgIsUnreceivedAttach(b))
    {
      temp[0]=mkeys[5];
      temp[1]=0;

      strcat(prompt, mchk_dload);
      strcat(msginf, temp);
    }
    else if (GEPriv(usr.priv, prm.mc_kill_priv) &&
        CanKillMsg(&b->msg) &&
        CanAccessMsgCommand(&mah, msg_kill, 0))
    {
      temp[0]=mkeys[1];
      temp[1]=0;

      strcat(prompt, mchk_kill);
      strcat(msginf, temp);
    }


    /* Handle the "unreceive" command */

    if (MsgToThisUser(b->msg.to) &&
        CanAccessMsgCommand(&mah, msg_unreceive, 0))
    {
      strcat(prompt, mchk_unreceive);

      temp[0]=mkeys[4];
      temp[1]='\0';

      strcat(msginf, temp);
    }


    /* Handle the kludge toggle command */

    if (CanAccessMsgCommand(&mah, msg_toggle_kludges, 0))
    {
      temp[0]=mkeys[3];
      temp[1]='\0';

      strcat(prompt, mchk_kludge);
      strcat(msginf, temp);
    }

    /* Option to go to next area, only if we're scanning > 1 area. */

    if ((b->bflag & BROWSE_ACUR)==0)
    {
      temp[0]=mkeys[2];
      temp[1]=0;

      strcat(prompt, mchk_nextar);
      strcat(msginf, temp);
    }

    strcat(prompt, mchk_end_prompt);

    ch=toupper(GetListAnswer(msginf, NULL, mchk_nunder,
                             CINPUT_FULLPROMPT, prompt));

    if (ch==toupper(nkeys[0]))
      return 0;
    else if (ch==toupper(nkeys[1]))
      return -1;
    else if (ch==toupper(nkeys[2]))
      return 2;
    else if (ch==toupper(mkeys[2]))
      return 3;
    else if (ch==toupper(mkeys[1]))
    {
      MsgCloseMsg(b->m);
      b->m=NULL;
      
      /* Preserve message number, in case kill modifies the message Nos */
      
      here=MsgMsgnToUid(sq, b->msgn);
      Msg_Kill(b->msgn);
      new=MsgUidToMsgn(sq, here, UID_PREV);

      if (new > 0)
        b->msgn=new;

      /* If we killed a msg, re-update our lr ptr */
      
      if (OkToFixLastread(b))
        Lmsg_Set(b, b->msgn);

      return 0;
    }
    else if (ch==toupper(mkeys[0]))
    {
      chkmail_reply=TRUE;
      savelast=last_msg;
      last_msg=b->msgn;
      
      here=MsgMsgnToUid(sq, b->msgn);
      Msg_Reply();
      new=MsgUidToMsgn(sq, here, UID_PREV);

      if (new > 0)
        b->msgn=new;

      last_msg=savelast;
      chkmail_reply=FALSE;
    }
    else if (ch==toupper(mkeys[3]))
    {
      Msg_Toggle_Kludges();
      return 2;
    }
    else if (ch==toupper(mkeys[4]))
    {
      Msg_Unreceive(0L, b->m);
      b->msg.attr &= ~MSGREAD;
    }
    else if (ch==toupper(mkeys[5]))
      MsgAttachDownload(b);
  }
}


int Read_End(BROWSE *b)
{
  Lmsg_Update(b);
  return List_End(b);
}

