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
static char rcs_id[]="$Id: msgtrack.c,v 1.2 2003/10/05 13:52:50 paltas Exp $";
#pragma on(unreferenced)

/* Copyright 1992 by Lanius Corporation.
   Portions copyright 1987-1991 by Bit Bucket Software.
*/

#define INCL_NOPM
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <os2.h>
#include <time.h>
#include "msgapi.h"
#include "sqfeat.h"
#include "squish.h"
#include "msgtrack.h"
#include "vsev.h"

typedef NETADDR *NETADDRP;


static char szNodexName[120]="nodex.ndx";   /* Nodelist index name */
static USHORT fKill=FALSE;                  /* Kill bounced msgs? */
void (cdecl far *pfnLogMsg)(char far *line);/* Write to Squish log. */

union stamp_combo * _fast TmDate_to_DosDate(struct tm *tmdate,
                                             union stamp_combo *dosdate)
{
  dosdate->msg_st.date.da=tmdate->tm_mday;
  dosdate->msg_st.date.mo=tmdate->tm_mon+1;
  dosdate->msg_st.date.yr=tmdate->tm_year-80;

  dosdate->msg_st.time.hh=tmdate->tm_hour;
  dosdate->msg_st.time.mm=tmdate->tm_min;
  dosdate->msg_st.time.ss=tmdate->tm_sec >> 1;

  return dosdate;
}


union stamp_combo * _fast Get_Dos_Date(union stamp_combo *st)
{
  time_t timeval;
  struct tm *tim;

  timeval=time(NULL);
  tim=localtime(&timeval);

  return (TmDate_to_DosDate(tim,st));
}

/* Turn a binary address into a string */

byte * _fast Address(NETADDR *a)
{
  static char temp[30];
  char point[10];

  sprintf(point, ".%hu", (unsigned)a->point);

  sprintf(temp, "%hu:%hu/%hu%s",
          (unsigned)a->zone, (unsigned)a->net, (unsigned)a->node,
          a->point ? point : "");

  return temp;
}


/* Initialization routine: entrypoint */

word FEATENTRY _export FeatureInit(struct _feat_init far *pfi)
{
  char far *p="MsgTrack";

  pfnLogMsg=pfi->pfnLogMsg; /* Save log message function pointer */
  strcpy(pfi->szConfigName, p);
  pfi->ulFlag=FFLAG_NETSENT | /*FFLAG_NETRECD |*/ FFLAG_NETNOTTOUS;
  return 0;
}


/* Configuration routine: entrypoint */

word FEATENTRY _export FeatureConfig(struct _feat_config far *pfc)
{
  NW(pfc);

  if (stricmp(pfc->ppszArgs[1], "Nodelist")==0)
    strcpy(szNodexName, pfc->ppszArgs[2]);
  else if (stricmp(pfc->ppszArgs[1], "Kill")==0)
    fKill=TRUE;
  else
  {
    char temp[120];

    sprintf(temp, "!Unknown keyword: \"MsgTrack %s\"\n", pfc->ppszArgs[1]);
    (*pfnLogMsg)(temp);
    return 1;
  }

  return 0;
}


/* Determine whether or not the message's destination address is            *
 * listed in our nodelist.                                                  */

static unsigned near FeatListed(struct _feat_netmsg far *pfn)
{
  NETADDR n=pfn->pMsg->dest;
  long rec;


  rec=v7lookupnode(&n, szNodexName);

  return (rec);
}



/* Bounce a message back to the sender */

static void near BounceMessage(struct _feat_netmsg far *pfn)
{
  char far *p;
  char *bounce;
  char trailer[]="\r================= e n d   o f   b o u n c e d   m e s s a g e ================\r\r";
  XMSG msg;
  HMSG hmsg;

  /* Set the orig and dest addresses accordingly */

  memset(&msg, 0, sizeof msg);
  msg.orig=pfn->us;
  msg.dest=pfn->pMsg->orig;

  printf("msg orig=%s\n", Address(&msg.orig));
  printf("msg dest=%s\n", Address(&msg.dest));

  /* Reverse the to/from fields */

  strcpy(msg.from, "SquishMail");
  strcpy(msg.to, pfn->pMsg->from);
  sprintf(msg.subj, "NetMail bounced by %s", Address(&msg.orig));

  Get_Dos_Date(&msg.date_written);
  msg.date_arrived=msg.date_written;
  msg.attr=MSGLOCAL | MSGPRIVATE;

  if ((bounce=malloc(1024)) != NULL)
  {
    sprintf(bounce,
      "The destination address of the following message was not listed "
      "in the nodelist at %s, so the message below was not sent.  "
      "Please correct the destination address before attempting to send "
      "the message again.\r\r"

      "- SquishMail at %s\r\r"

      "================ b o u n c e d   m e s s a g e   f o l l o w s ===============\r\r"

      "From: %s (%u:%u/%u.%u)\r"
      "  To: %s (%u:%u/%u.%u)\r"
      "Subj: %s\r\r",

      Address(&msg.orig), Address(&msg.orig),
      pfn->pMsg->from,                    
      pfn->pMsg->orig.zone, pfn->pMsg->orig.net, pfn->pMsg->orig.node,
      pfn->pMsg->orig.point,
      pfn->pMsg->to,
      pfn->pMsg->dest.zone, pfn->pMsg->dest.net, pfn->pMsg->dest.node,
      pfn->pMsg->dest.point,
      pfn->pMsg->subj);
  }


  if ((hmsg=MsgOpenMsg(pfn->ha, MOPEN_CREATE, 0))==NULL)
  {
    printf("MsgTrack:  Couldn't create bounce message.\n");
    return;
  }


  /* Write the first part of the bounced message */

  MsgWriteMsg(hmsg, FALSE, &msg, bounce,
              strlen(bounce),
              strlen(bounce)+strlen(pfn->pszMsgTxt)+strlen(trailer)+1,
              0, NULL);

  if (bounce)
    free(bounce);

  /* Convert all control-A's to at-signs */

  for (p=pfn->pszMsgTxt; (p=strchr(p, '\x01')) != NULL; p++)
    *p='@';

  /* Now write the rest of the message body */

  MsgWriteMsg(hmsg, TRUE, NULL, pfn->pszMsgTxt, strlen(pfn->pszMsgTxt),
              0, 0, NULL);

  MsgWriteMsg(hmsg, TRUE, NULL, trailer, strlen(trailer)+1,
              0, 0, NULL);

  /* Close the message */

  MsgCloseMsg(hmsg);

  /* Make sure that this message is not processed */

  pfn->ulAction=FACT_SKIP | FACT_HIDE;

  if (fKill)
    pfn->ulAction |= FACT_KILL;
  else
  {
    pfn->ulAction |= FACT_RWMSG;
    pfn->pMsg->attr |= MSGSENT | MSGORPHAN;
  }
}



/* Entry point - packing netmail message */

word FEATENTRY _export FeatureNetMsg(struct _feat_netmsg far *pfn)
{
  char temp[120];
  static int configured=FALSE;
  static int done_err=FALSE;

  /* Validate the nodelist index */

  if (!configured)
  {
    if (access(szNodexName, 0)==-1)
    {
      if (!done_err)
      {
        printf("Error!  Nodelist index `%s' does not exist!\n", szNodexName);
        done_err=TRUE;
      }

      return 0;
    }

    configured=TRUE;
  }

/*  printf("Scanning msg from: %-20.20s, to: %s\n",
         pfn->pMsg->from, pfn->pMsg->to);
*/

  /* If the message is addressed to a listed address, get out */

  if (FeatListed(pfn))
  {
    sprintf(temp, "@Listed dest address %u:%u/%u.%u",
            pfn->pMsg->dest.zone, pfn->pMsg->dest.net,
           pfn->pMsg->dest.node, pfn->pMsg->dest.point);

    (*pfnLogMsg)(temp);

    return 0;
  }

  sprintf(temp, "@UNLISTED dest address %u:%u/%u.%u",
          pfn->pMsg->dest.zone, pfn->pMsg->dest.net,
          pfn->pMsg->dest.node, pfn->pMsg->dest.point);

  (*pfnLogMsg)(temp);

  fflush(stdout);

  /* Bounce this message */

  BounceMessage(pfn);

  return 0;
}


word FEATENTRY _export FeatureTossMsg(struct _feat_toss far *pft)
{
  NW(pft);
  return 0;
}


word FEATENTRY _export FeatureScanMsg(struct _feat_scan far *pfs)
{
  NW(pfs);
  return 0;
}

word FEATENTRY _export FeatureTerm(struct _feat_term far *pft)
{
  NW(pft);
  return 0;
}


#ifdef __FLAT__
void FEATENTRY _export Feature32Bit(void)
#else
void FEATENTRY _export Feature16Bit(void)
#endif
{
}


