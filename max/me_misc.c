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
static char rcs_id[]="$Id: me_misc.c,v 1.2 2003/06/05 23:26:49 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message section: message entry routines (miscellaneous)
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"
#include "m_for.h"
#include "node.h"
#include "trackm.h"

#define MAX_KLUDGE_LEN  512

static int near Handle_Matrix_Charges(NETADDR *dest,int total);

/* Called after a user writes a message.  Used to update internal pointers  *
 * and to set flags based on where the user wrote the message.              */

word WroteMessage(PMAH pmah, XMSG *msg, char *kludges, HAREA ha, int chg)
{
  word ok=TRUE;
  
  if (pmah->ma.attribs & MA_NET)  /* Handle charging and matrix costs! */
  {
    written_matrix=TRUE;
    
    if (msg)
      ok=(Handle_Matrix_Charges(&msg->dest, TRUE) != -1);
    
    /* Send a msg if user doesn't have enough credit */
    
    if (!ok)
      Puts(ncredit);
  }
  else if (pmah->ma.attribs & MA_ECHO)
    written_echomail=TRUE;
  else if (pmah->ma.attribs & MA_CONF)
    written_conf=TRUE;
  else
    written_local=TRUE;

  /* Set the bit field, for the echo tosslog */

  if (*PMAS(pmah, echo_tag))
    AddToEchoToss(PMAS(pmah, echo_tag));

#ifdef MAX_TRACKER
  /* Add to tracking database, if we're not changing an existing message */

  if (!chg)
    TrackAddMessage(pmah, msg, kludges, ha);
#endif

  ci_posted();
  usr.msgs_posted++;
  bstats.msgs_written++;

  return ok;
}



void GenerateOriginLine(char *text, PMAH pmah)
{
  /* Now tack on OUR origin */

  sprintf(text, tearline,
          /* tear */
          us_short, tear_ver,

          /* origin */
          pmah->ma.origin ? PMAS(pmah, origin) : PRM(system_name),
          Address(&pmah->ma.primary),

          /* seen-by */
          pmah->ma.seenby.net, pmah->ma.seenby.node);
}


static int near Handle_Matrix_Charges(NETADDR *dest,int total)
{
  NFIND *nf;
  word cost;

  if ((nf=NodeFindOpen(dest))==NULL)
  {
    /* What the hell happened to our node? */
    
    if (! GEPriv(usr.priv, prm.unlisted_priv))
      return -1;

    cost=prm.unlisted_cost;
  }
  else
  {
    cost=nf->found.cost;
    NodeFindClose(nf);
  }

  if (mailflag(CFLAGM_NETFREE))
    cost=0;

  if (cost)  /* If there was a charge for this message */
  {
    if (total)
    {
      Puts(ms_1);
      Printf(ms_2, usr.credit);
      Printf(ms_3, usr.debit);
      Printf(ms_4, cost);
    }

    usr.debit += cost;

    if (usr.debit > usr.credit && !mailflag(CFLAGM_ATTRANY))
    {
      usr.debit -= cost;
      return -1;
    }

    if (total)
    {
      Printf(ms_5, usr.credit-usr.debit);

      logit(log_charge, cost);

      if (usr.credit-usr.debit < 100)
        logit(log_bal, usr.credit-usr.debit);
    }
  }

  return 0;
}



/* Determine whether or not a specified kludge is in the control buffer */

static int near InCtrlBuf(char *buf, char *kludge)
{
  char *p;

  if ((p=GetCtrlToken(buf, kludge)) != NULL)
  {
    MsgFreeCtrlToken(p);
    return TRUE;
  }

  return FALSE;
}



/* Generate the kludges to go with a specific message */

char * GenerateMessageKludges(XMSG *msg, PMAH pmah, char *ctrl_buf)
{
  char temp[PATHLEN];
  char *kludge, *kend;

  /* Allocate some space to stuff the kludges in */

  if ((kludge=malloc(MAX_KLUDGE_LEN + (ctrl_buf ? strlen(ctrl_buf)+1 : 0)))==NULL)
    return NULL;

  *kludge='\0';
  kend=kludge;

  if (ctrl_buf)
  {
    if (*ctrl_buf != '\x01' && *ctrl_buf)
      strcpy(kludge, "\x01");

    strcat(kludge, ctrl_buf);
    kend=kludge+strlen(kludge);
  }

#ifdef MAX_TRACKER
  /* Add tracking information for this message */

  if (!InCtrlBuf(kludge, actrack_colon) && TrackNeedToAdd(pmah, msg))
  {
    GenerateActrack(msg, kend);
    kend += strlen(kend);
  }
#endif

  /* Add the PID kludge, but only for conference-style areas */

  if ((pmah->ma.attribs & MA_CONF) && !InCtrlBuf(kludge, "PID:"))
  {
    sprintf(kend, "\x01PID: %s %s%s", us_short, version_short,
#ifdef TEST_VER
    s_statdict
#else
    blank_str
#endif
    );

    kend += strlen(kend);
  }

  /* If this system has a point number, then add it.  Otherwise, make it    *
   * blank, to avoid any ".0"'s.                                            */

  if (pmah->ma.primary.point)
    sprintf(temp, ".%u", pmah->ma.primary.point);
  else *temp='\0';


  /* Do MSGID's for echomail, but only do ^aREPLYs for conference mail */

  if (pmah->ma.primary.node != (word)-1)
  {
    char *msgid_str="\x01MSGID: %u:%u/%u.%u %08lx";
    NETADDR *a;

    a=(pmah->ma.attribs & MA_NET) ? &msg->orig : &pmah->ma.primary;

    if (!InCtrlBuf(kludge, "MSGID:"))
    {
      sprintf(kend, msgid_str,
              a->zone, a->net, a->node, a->point,
              verylongtime(&msg->date_written));

      kend += strlen(kend);
    }

    if (*orig_msgid && !InCtrlBuf(kludge, "REPLY:"))
    {
      sprintf(kend, "\x01REPLY: %s", orig_msgid);
      kend += strlen(kend);
    }
  }


  /* Only add the realname kludge if the user's name is changed, the user's *
   * priv is LESS than sysop, and if the 'NoRealNameKludge' flag in both    *
   * the area AND the PRM file are either set or NOT set.                   */

  if (! eqstri(msg->from, usr.name) && !mailflag(CFLAGM_NOREALNM) &&
      !(pmah->ma.attribs & MA_NORNK) && !InCtrlBuf(kludge, "REALNAME:"))
  {
    sprintf(kend, "\x01REALNAME: %s", usr.name);
    kend += strlen(kend);
  }
  
  /* If we're s'posta gateroute messages, address it to the right person */

  if ((prm.flags2 & FLAG2_gate) && msg->dest.zone != msg->orig.zone &&
      (msg->attr & (MSGCRASH|MSGHOLD))==0)
  {
    /* Add the true address to the ^aINTL kludge */

    sprintf(kend, "\x01""INTL %u:%u/%u %u:%u/%u",
            msg->dest.zone, msg->dest.net, msg->dest.node,
            msg->orig.zone, msg->orig.net, msg->orig.node);


    /* Now perform the mapping on the TO address.  For example, messages    *
     * to 3:123/456 should be routed to 1:1/3 (if we're in zone 1).         *
     * If we were in zone 2, that message should be sent to 2:2/3, and      *
     * so forth.                                                            */

    msg->dest.node=msg->dest.zone;
    msg->dest.net=msg->orig.zone;
    msg->dest.zone=msg->orig.zone;
    msg->dest.point=0;
  }

  /* Cap the kludge lines */

  /* strcat(kludge, "\x01"); */ /* WHY?! -- Bo */

  return kludge;
}



void AddToEchoToss(char *tag)
{
  char temp[PATHLEN];
  int found=FALSE;
  FILE *fptr;

  if ((prm.flags & FLAG_log_echo)==0)
    return;

  if (strchr(PRM(echotoss_name), '%'))
    Parse_Outside_Cmd(PRM(echotoss_name), temp);
  else strcpy(temp, PRM(echotoss_name));


  if ((fptr=shfopen(temp, fopen_writep, O_CREAT | O_RDWR | O_NOINHERIT))==NULL)
  {
    cant_open(temp);
    return;
  }

  while (fgets(temp, PATHLEN, fptr))
  {
    Strip_Trailing(temp, '\n');
    Strip_Trailing(temp, ' ');

    if (eqstri(temp, tag))
    {
      found=TRUE;
      break;
    }
  }

  if (!found)
  {
    fseek(fptr, 0L, SEEK_END);
    fprintf(fptr, "%s\n", strupr(tag));
  }

  fclose(fptr);
}

