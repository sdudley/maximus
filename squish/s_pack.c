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
static char rcs_id[]="$Id: s_pack.c,v 1.8 2003/10/05 13:52:50 paltas Exp $";
#pragma on(unreferenced)

#define NOVARS
/*#define NO_MSGH_DEF*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "alc.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_pack.h"

/*#include "api_sdm.h"*/

static char area_col[]="AREA:";
static char file_delim[]=" ;,";

void Pack_Messages(struct _cfgarea *nets)
{
  struct _cfgarea *ar;

  /* Pack all netmail areas */

  for (ar=nets; ar; ar=ar->next_type)
    if ((ar->type & MSGTYPE_ECHO)==0)
      PackIt(ar);
    
  if (trklog)
  {
    (void)fclose(trklog);
    trklog=NULL;
  }
}


static void near PackIt(struct _cfgarea *ar)
{
  HAREA sq=NULL;
  dword mn;

  /* No packing action necessary for black-hole mailers */
  
  if (config.flag & FLAG_FRODO)
    return;
  
  n_notsent=n_packed=0L;
  
 
  Alloc_Outbuf();

  if (ar==NULL)
  {
    S_LogMsg("!No netmail path defined");
    Free_Outbuf();
    return;
  }
  
  if ((sq=MsgOpenArea(ar->path, MSGAREA_NORMAL, ar->type))==NULL)
  {
    (void)printf("Invalid netmail area: `%s'\n",ar->name);
    Free_Outbuf();
    return;
  }

  (void)printf("\nSquashing messages in %s...\n",ar->path);
  
  S_LogMsg("#Packing from %s (%ld msgs)",ar->path,
           MsgNumMsg(sq));

  for (mn=1L; mn <= MsgHighMsg(sq); mn++)
    if (!Pack_Netmail_Msg(sq, &mn, ar))
      break;
  
  (void)MsgCloseArea(sq);

  (void)printf("\r                                            \r");
  Flush_Outbuf();
  Free_Outbuf();

  if (n_notsent)
    S_LogMsg(":  Packed=%ld  NotSent=%ld",n_packed,n_notsent);
  else S_LogMsg(":  Packed=%ld",n_packed);
}

#if defined(OS_2) || defined(UNIX)
/* Call all of the external DLL features */

static word near InvokeFeatures(HAREA ha, HMSG hmsg, XMSG *pMsg, char *pCtrl,
                                char *pszMsgBuf, dword *pulMsgNum,
                                int *dokill, int *logkill, int *rewriteit)
{
  struct _feat_netmsg fn;
  struct _feature *pf;
  word rc;

  fn.struct_len=sizeof fn;
  fn.ha=ha;
  fn.hmsg=hmsg;
  fn.pMsg=pMsg;
  fn.pszCtrl=pCtrl;
  fn.pszMsgTxt=pszMsgBuf;
  fn.ulMsgNum=*pulMsgNum;
  SblistToNetaddr(config.addr, &fn.us);

  rc=TRUE;  /* Continue processing message */


  /* Loop through linked list of names and call each one in turn */

  for (pf=config.feat; pf; pf=pf->pfNext)
  {
    /* Only call the feature if it has a netmail handler defined */

    if (pf->pfnNetMsg)
    {
      /* Don't call feature if it doesn't want sent messages */

      if ((pMsg->attr & MSGSENT) && (pf->ulFlag & FFLAG_NETSENT))
        continue;

      if ((pMsg->attr & MSGREAD) && (pf->ulFlag & FFLAG_NETRECD))
        continue;

      if (!DestIsHereA(&pMsg->dest) && (pf->ulFlag & FFLAG_NETTOUS))
        continue;

      if (DestIsHereA(&pMsg->dest) && (pf->ulFlag & FFLAG_NETNOTTOUS))
        continue;

      /* Clear action field before calling feature */

      fn.ulAction=0;  

      /* Call the feature itself */

      if ((*pf->pfnNetMsg)(&fn))
        exit(ERL_ERROR);

      /* Kill message? */

      if (fn.ulAction & FACT_KILL)
      {
        *dokill=TRUE;
        *logkill=FALSE;
      }

      /* Don't pack message? */

      if (fn.ulAction & FACT_SKIP)
        rc=FALSE;

      /* Rewrite message header */

      if (fn.ulAction & FACT_RWMSG)
        *rewriteit=TRUE;

      /* Don't process any more handlers */

      if (fn.ulAction & FACT_HIDE)
        break;
    }
  }

  return rc;  /* continue processing msg, depending on return code */
}
#endif

static char *front;

static unsigned near Pack_Netmail_Msg(HAREA sq, dword *mn, struct _cfgarea *ar)
{
  HMSG mh, out_msg;
  NETADDR olddest, destnop;
  XMSG msg, original;
  char *ctrl, *ct, *buf;

  UMSGID this;
  dword bytes;
  dword killmsgn;
  dword offset;

  unsigned ctlen, got;
  int sentit=FALSE, rewriteit=FALSE, dokill=FALSE, logkill=FALSE;
  
  killmsgn=*mn;

  if ((mh=MsgOpenMsg(sq, MOPEN_RW, *mn))==NULL)
  {
    if (msgapierr != MERR_NOENT)
      S_LogMsg("!Can't open netmail msg #%lu (err#%d)", *mn, msgapierr);

    return TRUE;
  }

  if (MsgGetTextLen(mh) >= maxmsglen)
  {
    S_LogMsg("!Message %lu too large to pack (%#lx)", *mn, (long)maxmsglen);
    (void)MsgCloseMsg(mh);
    return TRUE;
  }

  ctlen=(unsigned)MsgGetCtrlLen(mh);

  if ((ctrl=malloc(ctlen+5))==NULL ||
      (msgbuf=(char *)malloc(maxmsglen + ctlen + 180u))==NULL)
  {
    S_LogMsg("!Not enough memory to pack msg %lu", *mn);

    if (ctrl)
      free(ctrl);

    (void)MsgCloseMsg(mh);
    return TRUE;
  }

  /* Read everything in */

  bytes=MsgReadMsg(mh, &msg, 0L, (dword)(maxmsglen-1), msgbuf,
                   (dword)ctlen, ctrl);

  original=msg;
 
  /* Make sure that it's nul-terminated */

  if (bytes != (dword)-1)
    msgbuf[(int)bytes]='\0';

  /* Never process SENT messages, or those addressed to 0/0. */

  if ((msg.attr & MSGSENT) || (msg.dest.net==0 && msg.dest.node==0))
  {
    /* do not pack, but call DLL handler anyway */

#if defined(OS_2) || defined(UNIX)
    (void)InvokeFeatures(sq, mh, &msg, ctrl, msgbuf, mn, &dokill, &logkill,
                         &rewriteit);
#endif
  }
  else
  {
    char *i, *isave;

    /* If a message was gaterouted "to" us, remove the gaterouting stuff */

    destnop=msg.dest;
    destnop.point=0;

    if (DestIsHereA(&destnop) &&
        (i=isave=GetCtrlToken(ctrl, "INTL")) != NULL)
    {
      NETADDR norig, ndest;
      
      norig=msg.orig;
      ndest=msg.dest;
      
      i += 5;
      
      Parse_NetNode(i, &ndest.zone, &ndest.net, &ndest.node, &ndest.point);

      while (*i != ' ' && *i)
        i++;

      if (*i)
        i++;

      Parse_NetNode(i, &norig.zone, &norig.net, &norig.node, &norig.point);

      MsgFreeCtrlToken(isave);

      /* Remove the intl kludge and properly put the info in the header */

      RemoveFromCtrl(ctrl, "INTL");
      
      msg.dest=ndest;
      msg.orig=norig;
    }

    /* If the DLL says it's okay to process this msg */

#if defined(OS_2) || defined(UNIX)
    if (InvokeFeatures(sq, mh, &msg, ctrl, msgbuf, mn, &dokill, &logkill,
                       &rewriteit))
#endif
    {
      if (! Remap_Message(&msg, *mn))
        Point_To_Fakenet_Dest(&msg);

      Point_To_Fakenet_Orig(&msg);

      /* If the message originated here, or if it's OK to forward, then     *
       * process it.                                                        */

      if (!DestIsHereA(&msg.dest))
      {
        if ((msg.attr & MSGLOCAL)==0 && !OkToForward(&msg))
        {
          if ((config.flag2 & FLAG2_QUIET)==0)
            (void)printf("Not forwarded: #%lu\n", *mn);

          n_notsent++;
        }
        else
        {
          /* If this message did NOT originate from here, strip off the     *
           * crash/hold/killsent bits.  These were left on when tossing     *
           * to the local message base (so that the sysop would be able     *
           * to see them), but we should strip them off now before          *
           * trying to process the mssage.                                  */

          if ((msg.attr & MSGLOCAL) && *msg.__ftsc_date &&
              InvalidDate(msg.__ftsc_date))
          {
            S_LogMsg("!Grunged date in %s:%ld", ar->name, *mn);
          }
          else
          {
            if ((msg.attr & MSGLOCAL)==0)
            {
              struct _sblist *sb, *sbe;

              /* See if this node falls into the "strip attributes" list */

              for (sb=config.stripattr; sb; sb=sb->next)
              {
                if (AddrMatchNS(&msg.orig, sb))
                {
                  /* Now make sure that it doesn't fall in the "except" list */

                  for (sbe=config.stripattr_except; sbe; sbe=sbe->next)
                    if (AddrMatchNS(&msg.orig, sbe))
                      break;

                  /* If it was in the stripattr list but not the            *
                   * except list, then the attributes need to be            *
                   * stripped.                                              */

                  if (!sbe)
                    msg.attr &= ~(MSGCRASH | MSGHOLD);
                }
              }

              TrackMessage(&msg, ctrl);
              AddViaLine(msgbuf, ctrl, msg);

              /* Now kill in-transit netmail */

              if (config.flag & FLAG_KILLFWD)
                msg.attr |= MSGKILL;
            }

            /* Bo: we also add via lines to messages which come from here */
	    if (msg.attr & MSGLOCAL)
	      AddViaLine(msgbuf, ctrl, msg);

            front=msgbuf;

  #ifdef NEVER
            if (msgbuf[0]=='A' && msgbuf[1]=='R' && msgbuf[2]=='E' &&
                msgbuf[3]=='A' && msgbuf[4]==':')
            {
              /* Skip over text until we find a '\r' */

              while (*front && *front != '\r')
                front++;

              /* Then keep skipping until we find the end of the newlines     *
               * and returns...                                               */

              while (*front=='\r' || *front=='\n')
                front++;
            }
  #endif

            /* Dump tke kludges in the front of the message */

            if ((ct=CvtCtrlToKludge(ctrl)) != NULL)
            {
              ctlen=(unsigned)strlen(ct);
              (void)strocpy(front+ctlen, front);
              (void)memmove(front, ct, ctlen);
              MsgFreeCtrlBuf(ct);
            }

            /* Only write zone/point kludge lines if NOT doing gaterouting */
	    
            if (! GateRouteMessage(&msg, *mn, &olddest))
              (void)WriteZPInfo(&msg, AddToMsgBuf, ctrl);

            /*S_LogMsg("@Sending message %ld to %s",
                     (long)*mn, Address(&msg.dest));*/

            if (Send_Message(mh, &msg, bytes, *mn, ar))
            {
              rewriteit=sentit=TRUE;

              if (msg.attr & MSGKILL)
                dokill=logkill=TRUE;

              n_packed++;

            } /* if Send_Message */
          } /* if date_okay */
        } /* if okay to forward */
      } /* if dest is not here */
    } /* if invoke_features */
  } /* if not sent */

  free(msgbuf);

  /* Do we need to update the message header? */
  
  if (rewriteit)
  {
    /*S_LogMsg("@Message #%ld has SENT bit set: %s", (long)*mn,
             (msg.attr & MSGSENT) ? "yes" : "no!!!!!!!!!!!!!");*/


    /* If the address wasn't changed, it's OK to write */
    
    if (AddrMatch(&msg.dest, &original.dest) &&
        AddrMatch(&msg.orig, &original.orig))
    {
      MsgWriteMsg(mh, FALSE, &msg, NULL, 0L, 0L, 0L, NULL);

      /*S_LogMsg("@MsgWriteMsg rc=%d (err=%d)", rc, msgapierr);*/
    }
    else
    {
      dword oldmn;

      /*S_LogMsg("@Rewriting entire message");*/

      /* Save a copy of the current message number */

      this=MsgMsgnToUid(sq, *mn);

      if ((out_msg=MsgOpenMsg(sq, MOPEN_CREATE, 0L)) != NULL)
      {
        if ((buf=malloc(REMAP_BUF_SIZE))==NULL)
          S_LogMsg("!No mem for point remap");
        else
        {
          /* Otherwise, we need to physically rewrite the entire message! */

          (void)MsgWriteMsg(out_msg, FALSE, &msg, NULL, 0L, MsgGetTextLen(mh),
                            (dword)ctlen, ctrl);

          for (offset=0L; offset < MsgGetTextLen(mh);)
          {
            got=(unsigned)MsgReadMsg(mh, NULL, offset, (dword)REMAP_BUF_SIZE,
                                     buf, 0L, NULL);

            if (got==0 || got==(unsigned)-1)
              break;

            (void)MsgWriteMsg(out_msg, TRUE, NULL, buf, (dword)got,
                              MsgGetTextLen(mh), 0L, NULL);

            offset += got;
          }

          free(buf);
        }

        (void)MsgCloseMsg(out_msg);
        (void)MsgCloseMsg(mh);

        mh=NULL;

        /* Now kill the old message, since we've just made a copy with        *
         * the right header.                                                  */

        if ((oldmn=MsgUidToMsgn(sq, this, UID_EXACT)) != 0)
          (void)MsgKillMsg(sq, oldmn);

        *mn=MsgUidToMsgn(sq, this, UID_PREV);

        /* The message to kill (if we need to) is now the highest msg */

        killmsgn=MsgGetHighMsg(sq);
      }
    }
  }
  
  free(ctrl);

  if (mh)
    (void)MsgCloseMsg(mh);


#if 0 /* debugging code */
  if (rewriteit)
  {
    if ((mh=MsgOpenMsg(sq, MOPEN_RW, *mn))==NULL)
      S_LogMsg("@Error re-opening message #%ld", (long)*mn);
    else
    {
      int rc;

      rc=MsgReadMsg(mh, &msg, 0L, 0L, NULL, 0L, NULL);

      S_LogMsg("@Verify: message #%ld has SENT bit set: %s (rc=%d)", (long)*mn,
               (msg.attr & MSGSENT) ? "yes" : "no!!!!!!!!!!!!!",
               rc);

      MsgCloseMsg(mh);
    }
  }
#endif


  /* If we're supposed to delete the message, then do so... */
  
  if (dokill)
  {
    this=MsgMsgnToUid(sq, *mn);
    (void)MsgKillMsg(sq, killmsgn);

    if (logkill)
    {
      if ((config.flag2 & FLAG2_QUIET)==0)
        (void)printf(" (KILL)");

      S_LogMsg(":  Sent/killed #%ld (dest=%s)", *mn, Address(&msg.dest));
    }

    *mn=MsgUidToMsgn(sq, this, UID_PREV);
  }

  if (sentit && (config.flag2 & FLAG2_QUIET)==0)
    (void)printf("\n");

  return TRUE;
}





/* Add a string to the front of the message buffer */

static void EXPENTRY AddToMsgBuf(byte OS2FAR *str)
{
  unsigned slen=strlen(str);
  char *where=(front ? front : msgbuf);

  (void)memmove(where+slen, where, strlen(where)+1);
  (void)memmove(where, str, slen);
}




/* Track an in-transit message and add it to msgtrack.log */

static void near TrackMessage(XMSG *msg, byte *ctrl)
{
  char temp[PATHLEN];
  char *p;
  
  /* Make an entry in the outbound netmail tracking log */

  if (! *config.tracklog)
    return;
  
  if (! trklog)
    if ((trklog=shfopen(config.tracklog, "a", O_CREAT | O_WRONLY | O_APPEND))==NULL)
      ErrOpening("track log", config.tracklog);

  /* Add a note to the log if it's an echomail msg */

  if ((p=GetCtrlToken(ctrl, area_col)) != NULL)
  {
    (void)fprintf(trklog, "*** ECHOMAIL ***\n");
    MsgFreeCtrlToken(p);
  }

  (void)fprintf(trklog, "Date: %s\n",
                sc_time((union stamp_combo *)&msg->date_arrived, temp));

  (void)fprintf(trklog, "From: %-36s (%s)\n", msg->from, Address(&msg->orig));
  (void)fprintf(trklog, "  To: %-36s (%s)\n", msg->to,   Address(&msg->dest));
  (void)fprintf(trklog, "Subj: %s\n\n", msg->subj);
}




/* Add a ^aVia line to the end of a netmail message */

static void near AddViaLine(byte *mbuf, byte *ctrl, XMSG xmsg)
{
  NETADDR n;
  time_t gmt;
  struct tm *lt;
  byte temp[160], *s;
  char *artag;
  int match = FALSE;
  
  struct _sblist * tmps;
  
  /* Now tack on the "^aVia" line, if not an echomail message */

  if ((artag=GetCtrlToken(ctrl, area_col)) != NULL)
  {
    MsgFreeCtrlToken(artag);
    return;
  }

  gmt=time(NULL);
  lt=gmtime(&gmt);

  /* ^aVia SquishMail 1:249/106.0, Sat Oct 13 1990 at 07:33 UTC\r */
  
  /* Bo: Added new Via: */
  /* ^aVia 2:236/100 @20030723.160020.UTC Squish/Linux 1.11\r */
  
  for(tmps=config.addr; tmps; tmps=tmps->next)
    if(tmps->zone == xmsg.dest.zone)
    {
	break;
	match = TRUE;
    }
    
  if(!match)
  {
    tmps = config.addr;
  }
  
  (void)sprintf(temp,
                "\x01Via %s @%04d%02d%02d.%02d%02d%02d.UTC " SQNAME " " SQVERSION "\r",
                Address(SblistToNetaddr(tmps, &n)),
                lt->tm_year+1900,
		lt->tm_mon+1,
		lt->tm_mday,
                lt->tm_hour,
                lt->tm_min,
		lt->tm_sec
		);


  /* Set 's' to last character of message buffer */

  s=mbuf+strlen(mbuf)-1;

  /* Decrement pointer until we find a non-LF and non-soft-CR */

  while (s >= mbuf && (*s=='\n' || *s==(byte)0x8d))
    s--;

  /* If this character is NOT a '\r', then we must be in the middle   *
   * of a prior line, so we should add one on our own.                */

  if (*s != '\r')
    (void)strcat(mbuf, "\r");

  (void)strcat(mbuf, temp);
}







/* Remaps messages based on name in 'to' field. */

static int near Remap_Message(XMSG *msg,dword mn)
{
  struct _remap *re;
  NETADDR na;
  byte sb1[80];
  byte sb2[80];
  byte *p;
  unsigned comp_lim;
  int match, sdx;


  /* Check to see if it's addressed to here, regardless of point */

  na=msg->dest;
  na.point=0;

  if (DestIsHereA(&na))
  {
    (void)strcpy(sb1, soundex(msg->to));

    match=FALSE;
    
    for (re=config.remap; re; re=re->next)
    {
      if ((p=strchr(re->name,'*')) != NULL)
        comp_lim=(unsigned)(p-re->name);
      else comp_lim=0;

      (void)strcpy(sb2, soundex(re->name));

      sdx=FALSE;

      if (eqstrin(msg->to, re->name, comp_lim ? comp_lim : strlen(msg->to)))
        match=TRUE;
      else if (eqstri(sb1,sb2) && (config.flag2 & FLAG2_NOSNDX)==0)
      {
        sdx=TRUE;
        match=TRUE;
      }
      else if (eqstri(re->name,"*"))
        match=TRUE;

      if (match)
      {
        msg->dest.zone=re->node.zone;
        msg->dest.net=re->node.net;
        msg->dest.node=re->node.node;
        msg->dest.point=re->node.point;

        S_LogMsg("#  Msg#%ld remapped to %s%s",
               mn,
               Address(&msg->dest),
               sdx ? " (soundex)" : "");

        return TRUE;
      }
    }
  }
  
  return FALSE;
}



/* Convert point adressing info to fakenet address */

static void near Point_To_Fakenet_Dest(XMSG *msg)
{
  NETADDR na;

  /* Readdress pointnode message traffic to fakenet */
 
  na=msg->dest;
  na.point=0;

  if (DestIsHereA(&na))
  {
    if (msg->dest.point && config.pointnet)
    {
      msg->dest.net=config.pointnet;
      msg->dest.node=msg->dest.point;
      msg->dest.point=0;
    }
  }
}

static void near Point_To_Fakenet_Orig(XMSG *msg)
{
  /* Remap messages which are FROM a point in our pointnet */
  
  if (config.pointnet && msg->orig.net==config.pointnet)
  {
    msg->orig.net=config.addr->net;
    msg->orig.point=msg->orig.node;
    msg->orig.node=config.addr->node;
  }
}




static int near GateRouteMessage(XMSG *msg,dword mn,NETADDR *olddest)
{
  NETADDR tempnet;
  struct _groute *gr;
  struct _sblist *sb;
  char intl[80];
  
  
  /* Only gateroute messages which are of the NORMAL flavour */
  
  if ((msg->attr & (MSGCRASH | MSGHOLD)) != 0)
    return FALSE;


  /* Handle gaterouting commands */

  for (gr=config.gate; gr; gr=gr->next)
  {
    int cont=FALSE;

    /* Process exceptions.  If we get a match, skip down to the next        *
     * gateroute statement.                                                 */

    for (sb=gr->except; sb; sb=sb->next)
      if (AddrMatchNS(&msg->dest, sb))
      {
        cont=TRUE;
        break;
      }

    if (cont)
      continue;

    for (sb=gr->nodes; sb; sb=sb->next)
      if (AddrMatchNS(&msg->dest, sb))
      {
        if ((config.flag2 & FLAG2_QUIET)==0)
        {
          (void)printf("GateRoute #%lu: %s -> ", mn, Address(&msg->dest));
          (void)printf("%s\n", Address(SblistToNetaddr(&gr->host,&tempnet)));
        }


        /* Now give this message the proper flavour */

        msg->attr |= FlavourToMsgAttr(gr->flavour);

        *olddest=msg->dest;
        msg->dest=*SblistToNetaddr(&gr->host, &tempnet);
        
        (void)sprintf(intl,"\x01INTL %hu:%hu/%hu %hu:%hu/%hu\r",
                      olddest->zone, olddest->net, olddest->node,
                      msg->orig.zone, msg->orig.net, msg->orig.node);

        AddToMsgBuf(intl);
        return TRUE;
      }
  }
  
  return FALSE;
}









/* Send a message to its final destination, processing frq/urq along the way */

static int near Send_Message(HMSG mh, XMSG *msg, dword bytes, dword mn, struct _cfgarea *ar)
{
  struct _sblist scanto;
  char fname[PATHLEN];
  char *p;
  int okay_to_scan=TRUE;

  NW(bytes);
  NW(mh);

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("Sending (#%lu): %s", mn, Address(&msg->dest));


  (void)NetaddrToSblist(&msg->dest, &scanto);

  if (msg->attr & (MSGFILE | MSGFRQ | MSGURQ))
  {
    /* Try to obtain a lock for this node */

    if (BusyFileOpenNN(&msg->dest, FALSE)==-1)
    {
      S_LogMsg(":%s busy - %s:%ld will be packed later",
               Address(&msg->dest),
               ar->name,
               mn);

      okay_to_scan=FALSE;
      printf(" (busy -- queued for later)\n");
    }
    else
    {
      (void)strcpy(fname, msg->subj);
      *msg->subj='\0';

      for (p=strtok(fname, file_delim); p; p=strtok(NULL, file_delim))
        Process_AttReqUpd(msg, p, FALSE);

      BusyFileCloseNN(&msg->dest);
    }
  }


  if (okay_to_scan)
  {
    /* Clean up the message status bits before they get sent */

    msg->attr &= (long)~(MSGSENT | MSGLOCAL | MSGXX2);

    Scan_To(&scanto, msg, msgbuf, msg->attr, ar, NULL);

    /* Now update the "sent" bit */

    msg->attr |= MSGSENT;
  }

  return okay_to_scan;
}


void HandleAttReqPoll(word action, byte **toscan)
{
  XMSG msg;
  NETADDR d;
  byte **p;

  if (! *toscan)
    usage();

  p=toscan;

  (void)memset(&msg, '\0', sizeof msg);

  (void)SblistToNetaddr(config.addr, &msg.orig);
  (void)SblistToNetaddr(&config.def, &msg.dest);

  msg.attr=MSGLOCAL | (action==ACTION_GET    ? MSGFRQ :
                       action==ACTION_UPDATE ? MSGURQ :
                       MSGFILE);


  /* If we're not doing a poll, get a filename */

  if (action != ACTION_POLL)
  {
    /* Filename is first word */

    (void)strcpy(msg.subj, *p++);

    if (! *p)
      usage();

    /* Skip over "to" or "from" */

    if (eqstri(*p, "to") || eqstri(*p, "from"))
      p++;
  } 
    

  if (! *p)
    usage();

  /* Parse net address accordingly */

  (void)SblistToNetaddr(&config.def, &d);
  ParseNN(*p++, &d.zone, &d.net, &d.node, &d.point, FALSE);
  msg.dest=d;


  /* Handle different flavours too */

  if (*p)
    msg.attr |= (dword)FlavourToMsgAttr(**p);

#ifdef UNIX
  if (action == ACTION_POLL)
  {
    Process_OneAttReqUpd(&msg, "", 0, "", "");  
    (void)printf("Created poll to: %s\n ", Address(&msg.dest));
    return;
  }
#endif

  /* Tell the user about what we're doing */

  (void)printf("%s %s: ", action==ACTION_GET    ? "Request file from" :
                          action==ACTION_UPDATE ? "Update request file from" :
                          action==ACTION_SEND   ? "Send file to" :
                                                "Poll",
                          Address(&msg.dest));

  /* Now send the file */

  Process_AttReqUpd(&msg, msg.subj, TRUE);
}



/* Process one attach/filereq/updreq filename, expanding any wildcards */

static void near Process_AttReqUpd(XMSG *msg, char *filename, word manual)
{
  FFIND *ff;
  char tname[PATHLEN];
  char pwd[PATHLEN];
  
  #define TFL_NONE  0
  #define TFL_TRUNC 1
  #define TFL_DEL   2

  int tflag;
  char *p;

  /* If we're doing a file-request, just do it (no wildcard expansion) */

  tflag=TFL_NONE;

  /* Move the password into the password field and delete it from filename */

  *pwd=0;

  if ((p=strrchr(filename, '!')) != NULL && *p != '!')
  {
    strcpy(pwd, p+1);
    *p=0;
  }
  
  if (msg->attr & MSGFRQ)
    Process_OneAttReqUpd(msg, filename, tflag, filename, pwd);
  else
  {
    /* Otherwise, we must be doing an attach or update request, so we need  *
     * to expand wildcards and in-transit files.                            */

    if (*filename=='^')
    {
      tflag=TFL_DEL;
      (void)strocpy(filename, filename+1);
    }
    else if (*filename=='#')
    {
      tflag=TFL_TRUNC;
      (void)strocpy(filename, filename+1);
    }
    else
      tflag=TFL_NONE;


    /* If no path specification, automatically use netfile area */
  
    if (!manual && *filename)
    {
      if (filename[1] != ':' && !strchr(filename,'\\')) /* wes - not changing; \\ is in packet (?) */
      {
        struct _tosspath *tp, *lasttp;


        /* Check all of the tosspaths to see if it's there */

        for (tp=config.tpath, lasttp=tp; tp; lasttp=tp, tp=tp->next)
        {
          (void)sprintf(tname, "%s" PATH_DELIMS "%s", tp->path, filename);
	  fixPathMove(tname);

          if (fexist(tname))
          {
            filename=tname;
            break;
          }
        }


        /* If not found, default to the main toss path */

        if (!tp)
        {
          (void)sprintf(tname, "%s" PATH_DELIMS "%s", lasttp->path, filename);
          filename=fixPath(tname);
        }
      }
    }



    /* Try to find file */

    if ((ff=FindOpen(filename, 0))==NULL)
      Process_OneAttReqUpd(msg, filename, tflag, filename, pwd);
    else
    {
      /* If it's an update request, find the NEWEST file */

      if (msg->attr & MSGURQ)
      {
        SCOMBO sc;
        char szFname[PATHLEN];

        sc.ldate=0;
        (void)strcpy(szFname, filename);

        do
        {
          if (GEdate(&ff->scWdate, &sc))
          {
            (void)strcpy(szFname, ff->szName);
            sc=ff->scWdate;
          }
        }
        while (FindNext(ff)==0);

        ExpandAndSend(msg, tflag, filename, pwd, szFname);
      }
      else
      {
        do
        {
          ExpandAndSend(msg, tflag, filename, pwd, ff->szName);
        }
        while ((msg->attr & MSGFILE) && FindNext(ff)==0);
      }
      
      FindClose(ff);
    }
  }
}


/* Expand the filename szName returned by FFIND into a fully-qualified path,*
 * given the original name 'filename'                                       */

static void near ExpandAndSend(XMSG *msg, int tflag, char *szOrig,
                               char *pwd, char *szNew)
{
  char *p;
  char temp[PATHLEN];

  /* Strip off path and add in found filename */

  if ((p=strrstr(szOrig, "\\/:"))==NULL)
    (void)strcpy(temp, szNew);
  else
  {
    (void)strcpy(temp, szOrig);

    if ((p=strrstr(temp, "\\/:")) != NULL)
      p[1]='\0';

    (void)strcat(temp, szNew);
  }


  Process_OneAttReqUpd(msg, temp, tflag, szOrig, pwd);
}



/* Process one single attach/filereq/updreq filename */

static void near Process_OneAttReqUpd(XMSG *msg,
                                      char *filename, int tflag,
                                      char *orig_fspec, char *pwd)
{
  FILE *updfile;
  struct stat statbuf;
  char fname[PATHLEN];
  char floname[PATHLEN];
  char temp[PATHLEN];
  char *p, *s;
  int fd;
  

  /* Make sure that the attach filename is the FULL filename */

  if ((msg->attr & MSGFRQ)==0 && *filename)
    filename=make_fullfname(filename);
  

  /* Generate an xxxxyyyy.?LO filename.  We'll need this for both           *
   * f'attaches and f'reqs, since Bink won't poll for a freq without        *
   * a .?lo file being present.                                             */

  FloName(floname, &msg->dest, (byte)MsgAttrToFlavour(msg->attr),
          (config.flag & FLAG_ADDMODE));


  if (msg->attr & MSGFILE)
  {
    if (*filename)
    {
      if ((config.flag2 & FLAG2_QUIET)==0)
        (void)printf(" (W/FILE %s%s)",
                     (tflag==TFL_DEL ? "DEL " : (tflag==TFL_TRUNC ? "TRUNC " : "")),
                     filename);

      if (! fexist(filename))
        S_LogMsg("!File attach `%s' does not exist!", filename);
    }

        /* the things i do for lint :-( -------v */
    (void)strcpy(fname + ((tflag != TFL_NONE) ? 1 : 0), filename);
    
    /* Handle the truncation or deletion of the file */
    
    if (tflag==TFL_DEL)
      *fname='^';
    else if (tflag==TFL_TRUNC)
      *fname='#';

    if (*pwd)
      sprintf(fname+strlen(fname), " !%s", pwd);
  }
  else /* File/Update request */
  {
    if ((config.flag2 & FLAG2_QUIET)==0)
      (void)printf(" (%sREQ %s)", (msg->attr & MSGFRQ) ? "F'" : "UPD",
                   filename);

    /* Create an empty ?LO file, if one doesn't already exist */

    if ((fd=sopen(floname, O_CREAT | O_WRONLY | O_BINARY, SH_DENYNO,
                  S_IREAD | S_IWRITE)) != -1)
    {
      (void)close(fd);
    }

    /* Generate the xxxxyyyy.REQ filename */

    FloName(floname, &msg->dest, 'F', FALSE);

    (void)strcpy(floname+strlen(floname)-3, "req");


    /* If we're not doing an update request, just copy the filename.        *
     * Otherwise, get the file's date, and put it in the .REQ file.         */


    if (msg->attr & MSGURQ)
    {
      if ((updfile=shfopen(filename, "r", O_RDONLY))==NULL)
      {
        S_LogMsg("!Update file `%s' not found!", filename);
        statbuf.st_atime=0L;
      }
      else
      {
        if (stat(filename, &statbuf) != 0)
          (void)memset(&statbuf, '\0', sizeof(statbuf));

        (void)fclose(updfile);
      }

      /* Skip over the path when writing to the .REQ file */

      if ((p=strrchr(orig_fspec, '\\')) != NULL)
        p++;
      else
        p=orig_fspec;


      (void)strcpy(fname, p);

      if (*pwd)
        sprintf(fname+strlen(fname), " !%s", pwd);

      sprintf(fname+strlen(fname), " +%ld", statbuf.st_atime);
    }
    else
    {
      (void)strcpy(fname, filename);

      if (*pwd)
        sprintf(fname+strlen(fname), " !%s", pwd);
    }
  }

  (void)Add_To_FloFile(fname, NULL, floname);


  /* Remove the path specification from the file */

  (void)strcpy(temp, filename);

  if ((s=strrchr(temp, PATH_DELIM)) != NULL)
    (void)strocpy(temp, s+1);

  /* Now add it to the message's subject, if there's enough room */

  if (strlen(msg->subj)+1+strlen(temp) < sizeof(msg->subj)-1)
  {
    if (*msg->subj)
      (void)strcat(msg->subj, " ");

    (void)strcat(msg->subj, temp);
  }
}

static void near strip_msg_wcs(XMSG *msg)
{
  char *p;

  /* Strip any path delimiters or wildcards from the msg to be sent. */

  if ((p=strpbrk(msg->subj, "?*:\\/")) != NULL)
    *p='\0';
}


/* Check for an individual forwarding match, and strp off the subject       *
 * field and attach bits if necessary.                                      */

static word near FwdMatch(struct _fwdlist *fwd, NETADDR *o, XMSG *msg)
{
  if (! AddrMatchNS(o, &fwd->node))
    return FALSE;

  /* If we're not allowed to forward files, then strip the attach bit */

  if (!fwd->file)
  {
    msg->attr &= ~(MSGFILE | MSGFRQ | MSGURQ);
    return TRUE;
  }

  if (msg->attr & (MSGFILE | MSGFRQ | MSGURQ))
  {
    strip_msg_wcs(msg);

    /* If we're supposed to kill forwarded files, add a "^" in front    *
     * of each filename.                                                */

    if (config.flag2 & FLAG2_KFFILE)
    {
      char temp[XMSG_SUBJ_SIZE+80];
      char *p;

      *temp='\0';

      /* Pick out all of the filenames */

      for (p=strtok(msg->subj, file_delim); p; p=strtok(NULL, file_delim))
      {
        /* Use a space as a delimiter */

        if (*temp)
          (void)strcat(temp, " ");

        /* Add a caret and the name of the file itself */

        (void)strcat(temp, "^");
        (void)strcat(temp, p);
      }

      /* Now copy back to original subject field */

      (void)strncpy(msg->subj, temp, XMSG_SUBJ_SIZE-1);
      msg->subj[XMSG_SUBJ_SIZE-1]='\0';
    }
  }

  return TRUE;
}


/* Returns TRUE if it's okay to forward this in-transit message */

static int near OkToForward(XMSG *msg)
{
  struct _fwdlist *fwd;
  NETADDR o;

  
  for (fwd=config.fwdto; fwd; fwd=fwd->next)
  {
    o=msg->dest;

    if (!DestIsHereA(&o))
      o.point=0;

    if (FwdMatch(fwd, &o, msg))
      return TRUE;
  }
    
  for (fwd=config.fwdfrom; fwd; fwd=fwd->next)
  {
    o=msg->orig;

    if (!DestIsHereA(&o))
      o.point=0;
    
    if (FwdMatch(fwd, &o, msg))
      return TRUE;
  }

  msg->attr &= ~(MSGFILE | MSGFRQ | MSGURQ);
  return FALSE;
}



