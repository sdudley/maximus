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
static char rcs_id[]="$Id: m_intrin.c,v 1.2 2003/06/11 14:03:06 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Intrinsic functions
*/

#define MAX_LANG_max_bor

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <time.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"
#include "trackm.h"


int Exec_Msg(int type, char **result, int key, char *arg, char *menuname)
{
  *result=NULL;

  switch (type)
  {
    case same_direction:
      if (direction==DIRECTION_NEXT)
        Msg_Next(last_msg);
      else Msg_Previous(last_msg);
      break;
      
    case msg_area:            Msg_Area();                     break;
    case read_next:           Msg_Next(last_msg);             break;
    case read_previous:       Msg_Previous(last_msg);         break;
    case read_individual:     Msg_Read_Individual(key);       break;
    case msg_current:         Msg_Current();                  break;
    case read_original:       Msg_Read_Original();            break;
    case read_reply:          Msg_Read_Reply();               break;
    case msg_kill:            Msg_Kill(-1L);                  break;
    case forward:             Msg_Forward(NULL);              break;
    case enter_message:       Msg_Enter();                    break;
    case msg_reply:           Msg_Reply();                    break;
    case msg_edit_user:       Msg_Edit_User();                break;
    case read_nonstop:        Msg_Nonstop();                  break;
    case msg_list:            Msg_List(menuname);             break;
    case msg_scan:            Msg_Scan(menuname);             break;
    case msg_checkmail:       Msg_Checkmail(menuname);        break;
    case msg_inquir:          Msg_Inquire(menuname);          break;
    case msg_hurl:            Msg_Hurl();                     break;
    case msg_upload:          Msg_Upload();                   break;
    case msg_upload_qwk:      QWK_Upload();                   break;
    case xport:               Msg_Xport();                    break;
    case msg_change:          Msg_Change();                   break;
    case msg_tag:             Msg_Tag();                      break;
    case msg_browse:          Msg_Browse(0, NULL, menuname);  break;
    case msg_unreceive:       Msg_Unreceive(last_msg, NULL);  break;
    case msg_restrict:        Msg_Restrict();                 break;
    case msg_toggle_kludges:  Msg_Toggle_Kludges();           break;

#ifdef MAX_TRACKER
    case msg_track:           TrackMenu();                    break;
#endif

    case msg_dload_attach:    Msg_Attach_Download();          break;
    case msg_reply_area:      Msg_ReplyArea(arg);             break;

    default:              logit(bad_menu_opt, type); return 0;
  }
  
  return 0;
}


int MsgToUs(NETADDR *d)
{
  NETADDR *n, *e;

  for (n=prm.address, e=n+ALIAS_CNT; n < e; n++)
  {
    if ((d->zone==0 || n->zone==d->zone) &&
        d->net==n->net &&
        d->node==n->node &&
        d->point==n->point)
      return TRUE;
  }

  return FALSE;
}




void Add_Message_Date(XMSG *msg)
{
  Get_Dos_Date((SCOMBO *)&msg->date_written);
  msg->date_arrived=msg->date_written;
}

void Blank_Msg(XMSG *msg)
{
  memset(msg,'\x00',sizeof(XMSG));

  Add_Message_Date(msg);

  strcpy(msg->from, usrname);
  msg->attr=MSGLOCAL;

  msg->dest=msg->orig=mah.ma.primary;
}


static void near Max_Parse_NetNode(char *netnode, word *zone, word *net,
                                   word *node, word *point)
{
  if (zone)
    *zone=prm.address[0].zone;

  if (net)
    *net=prm.address[0].net;

  if (node)
    *node=prm.address[0].node;

  if (point)
    *point=0;

  Parse_NetNode(netnode, zone, net, node, point);
}

void MaxParseNN(char *netnode, NETADDR *pna)
{
  Max_Parse_NetNode(netnode, &pna->zone, &pna->net, &pna->node, &pna->point);
}


long verylongtime(union _stampu *sc)
{
    struct tm t;

    DosDate_to_TmDate((union stamp_combo *)sc,&t);
    return (mktime(&t));
}




/* Determine if a user can see a particular message */

word CanSeeMsg(XMSG *msg)
{
  return ((msg->attr & MSGPRIVATE)==0 || ToOrFromUs(msg) ||
          GEPriv(usr.priv, prm.pvt_priv) || mailflag(CFLAGM_PVT));
}


/* If it's to or from us in a local area, or if it's to us (and addressed   *
 * to us) in a netmail area, or from us (and addressed from us) in a        *
 * netmail area.                                                            */

word ToOrFromUs(XMSG *msg)
{
  return ((MsgToThisUser(msg->to) && 
          ((mah.ma.attribs & MA_NET)==0 || MsgToUs(&msg->dest))) ||
          (MsgToThisUser(msg->from) &&
          ((mah.ma.attribs & MA_NET)==0 || MsgToUs(&msg->orig))));
}



word MsgToThisUser(char *s)
{
  return (eqstri(Strip_Trailing_Blanks(s), usr.name) ||
          (*usr.alias && eqstri(Strip_Trailing_Blanks(s), usr.alias)));
}

void Recd_Msg(HMSG msgh, XMSG *msg, word set_recd)
{
  NW(set_recd);

  /* Only mark msg as received if it's to us (or our alias), and            *
   * if we're in a matrix area, only mark it as received if it's            *
   * addressed TO one of our AKA's.                                         */

  if (MsgToThisUser(msg->to) &&
      ((mah.ma.attribs & MA_NET)==0 || MsgToUs(&msg->dest)) &&
      (msg->attr & MSGREAD)==0)
  {
    msg->attr |= MSGREAD;
    MsgWriteMsg(msgh, FALSE, msg, NULL, 0L, 0L, 0L, NULL);
  }
}


#ifndef ORACLE


word Alloc_Outline(byte *outline[])
{
  word n, fill;
  
  for (n=0; n < MAX_MSGDISPLAY; n++)
    if ((outline[n]=malloc(MAX_LINELEN))==NULL)
      break;
    
  for (fill=n; fill < MAX_MSGDISPLAY; )
    outline[fill++]=NULL;

  return n;
}


void Dealloc_Outline(byte *outline[])
{
  word n;
  
  for (n=0; n < MAX_MSGDISPLAY; n++)
    if (outline[n])
    {
      free(outline[n]);
      outline[n]=NULL;
    }
}





#endif /* !ORACLE */


/* Returns TRUE if a more prompt needed before we can return to menu */

int MenuNeedMore(void)
{
  return ((usr.bits2 & BITS2_MORE) &&
           display_line > (byte)(TermLength()-(menu_lines+4)));
}

/* Called when we can't open a message area */

void AreaError(int err)
{
  Puts(err_entering_msg_area);

  switch (err)
  {
    case MERR_NOENT:  Puts(areadoesntexist); break;
    case MERR_NOMEM:  Puts(merr_nomem);   break;
    case MERR_NODS:   Puts(merr_nods);    break;
    case MERR_NOLOCK: Puts(merr_nolock);  break;
    case MERR_SHARE:  Puts(merr_share);   break;
    case MERR_BADH:
    case MERR_BADF:
    case MERR_BADA:
    case MERR_EOPEN:  Puts(merr_corrupt); break;
    default:
    case MERR_NONE:   Printf(merr_unknown, err);  break;
  }

  Putc('\n');
  Press_ENTER();
}


