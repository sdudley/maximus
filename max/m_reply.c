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
static char rcs_id[]="$Id: m_reply.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: R)eply command
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "prog.h"
#include "max_msg.h"
#include "trackm.h"
#include "m_reply.h"

int Msg_Reply(void)
{
  return Msg_ReplyArea(NULL);  /* Reply to current area */
}

struct _mline
{
  HMSG hmsg;
  char *buf;    /* Message buffer */
  int chars;    /* Chrs in buffer */
  int pos;      /* Current position in buffer */
};

static int near readmsgline(struct _mline * pml)
{
  char *bptr;
  int i;

  if (pml->pos)
  {
    memmove(pml->buf, pml->buf+pml->pos, pml->chars-pml->pos+1);
    pml->chars -= pml->pos;
    pml->pos=0;
  }

  bptr=pml->buf;
  for (;;)
  {

    /* Find a cr */

    while (*bptr)
    {
      if (*bptr++ == '\x0d')
        break;
    }

    /* Skip over trailing junk */

    while (*(byte *)bptr == (byte)'\x8d' || *bptr=='\x0a')
      ++bptr;

    /* See if we got end of line, and if so, return length */

    if (*bptr)
      break;

    /* Otherwise, we need to read some more in */

    if (pml->chars==MSGBUFLEN)  /* Long line? */
    { /* Just junk what we have to refill buffer */
      pml->pos=pml->chars=0;
    }

    i=(int)Read_Chars(pml->hmsg, pml->buf+pml->chars, MSGBUFLEN-pml->chars);
    if (i==0)
      break;

    pml->buf[pml->chars+=i]='\0';
  }

  return pml->pos= bptr - pml->buf;
}

static int near Reply_Get_Area(struct _replyp *r, char *areaname)
{
  int gotarea;
  char temp[PATHLEN];
  BARINFO bi;
  
  strcpy(temp, areaname);

  gotarea=(*temp!=0);
  for (;;)
  {
    if (!gotarea)
    {
      RipClear();

      WhiteN();
      InputGets(temp, where_to_reply);
    }
    gotarea=FALSE;
    
    if (*temp == '?')
      ListMsgAreas(NULL, FALSE, FALSE);
    else if (!*temp || *temp == '=')
      return 1;
    else
    {
      MAH ma;
      HAREA tosq;

      memset(&ma, 0, sizeof ma);

      SetAreaName(r->toareaname, temp);
      if (eqstri(r->toareaname, usr.msg))
        return 1;

      /* Open and validate the new area */

      if (!ReadMsgArea(ham, r->toareaname, &ma) ||
          !ValidMsgArea(NULL, &ma, VA_VAL | VA_PWD, &bi) ||
          (tosq=MaxOpenArea(&ma))==NULL)
      {
        AreaError(msgapierr);

        if (ma.heap)
          DisposeMah(&ma);

        return -1;
      }

      /* Make the reply-in area "current" */

      CopyMsgArea(&mah, &ma);
      sq=tosq;
      DisposeMah(&ma);
      return 0;
    }
  }
}

int Msg_ReplyArea(char *areaname)
{
  char *ctrl, *p;

  long ctlen;
  int aborted, editrc, rc, i;
  int isnet, wasecho, wasnet;
  HMSG omsgh;
  XMSG omsg, nmsg;

  struct _replyp rp;

  memset(&rp, 0, sizeof rp);

  if ((omsgh=MsgOpenMsg(sq, MOPEN_READ, last_msg))==NULL ||
      MsgReadMsg(omsgh, &omsg, 0L, 0L, NULL, 0L, NULL)==-1 ||
      ! CanSeeMsg(&omsg))
  {
    if (omsgh)
      MsgCloseMsg(omsgh);
    
    return (Msg_Enter());
  }

  rp.original=MsgMsgnToUid(sq, last_msg);

  /* Initialize the new message structure */

  Blank_Msg(&nmsg);

  /* Area we are replying from */

  wasecho=!!(mah.ma.attribs & MA_ECHO);
  wasnet=!!(mah.ma.attribs & MA_NET);

  /* Now, try and dig out the 'MSGID' kludge from this message, for use     *
   * in a R)eply situation.                                                 */

  *orig_msgid='\0';

  ctlen=MsgGetCtrlLen(omsgh);
  ctrl=NULL;

  if (ctlen)
  {
    if ((ctrl=malloc((int)ctlen+1))==NULL)
      ctlen=0;
    else
    {
      MsgReadMsg(omsgh, NULL, 0L, 0L, NULL, ctlen, ctrl);
      ctrl[(size_t)ctlen]='\0';

      if ((p=MsgGetCtrlToken(ctrl, "MSGID")) != NULL)
      {
        /* If we found a msgid, copy it to a buffer for later use */
        
        strncpy(orig_msgid, p+7, ORIG_MSGID_LEN-1);
        orig_msgid[ORIG_MSGID_LEN-1]='\0';
        MsgFreeCtrlToken(p);

      }
    }
  }

  /* Now, save the area being replied from to our structure */

  CopyMsgArea(&rp.fromarea, &mah);
  rp.fromsq=sq;

  if (!areaname)
  {
    isnet=!!(mah.ma.attribs & MA_NET);
    strcpy(rp.toareaname, MAS(mah,name));
  }
  else
  {

    int got;

    /* Save the current area name for the "Reply to message from area..." */

    if (wasecho)
      strcpy(rp.fromareaname, MAS(mah,echo_tag));
    if (!wasecho || !*rp.fromareaname)
      strcpy(rp.fromareaname, MAS(mah,name));

    /* If replying to a different area select it and make it current */

    got=Reply_Get_Area(&rp, areaname);

    if (got != 0)
    {
      /* User re-selected current area or selected an invalid area */

      MsgCloseMsg(omsgh);
      DisposeMah(&rp.fromarea);

      if (ctrl)
        free(ctrl);

      if (got > 0)
        return Msg_Reply();   /* user selected current area */
      else
        return FALSE;         /* user selected invalid area */
    }

    isnet=!!(mah.ma.attribs & MA_NET);

    if (!wasnet && isnet)
    {

      /* Stomp on the originating address so we know it is invalid */

      memset(&omsg.orig, 0, sizeof omsg.orig);

      /* If we're replying via netmail, and the original area wasn't netmail */
      /* then see if we can parse a FidoNet address from the MSGID           */

      if (*orig_msgid)
        ParseNNN(orig_msgid, &omsg.orig, FALSE);

      /* See if we still need to hunt for the origin line to get the address */

      if (wasecho && omsg.orig.net==0)
      {
        struct _mline m;

        if ((m.buf=malloc(BMSGBUFLEN))==NULL)
          logit(mem_nrmb);
        else
        {

          /* Read in buffer and split off one line at a time */

          m.hmsg=omsgh;
          m.buf[m.chars=m.pos=0]='\0';

          while (readmsgline(&m))
          {

            /* Skip too small lines */

            if (m.pos > 10 && strncmp(m.buf, " * Origin:", 10)==0)
            {

              /* It sure looks like an origin line */

              char *p;
              char ch=m.buf[m.pos];   /* NUL terminate just this line */
              m.buf[m.pos]='\0';

              /* Looks like one, so find the address part */

              p=strrchr(m.buf, '(');
              if (p !=NULL)
              {
                while (*++p && !isdigit(*p))
                  ;
                if (*p)
                {

                  /* In theory at least, this should be our address */

                  ParseNNN(p, &omsg.orig, FALSE);
                }
              }

              /* Fix the next line again - we may need it */
              m.buf[m.pos]=ch;
            }
          }  
          free(m.buf);
        }
      }
    }
  }

  MsgCloseMsg(omsgh);

  /* If we're replying to a private message, make our message private    *
   * by default.  (However, if the area we're in doesn't allow private   *
   * messages, then this setting doesn't matter.)                        *
   * Also assume private reply to netmail should be marked as private    */

  if (omsg.attr & MSGPRIVATE || (areaname && isnet))
    nmsg.attr |= MSGPRIVATE;

  /* Fix the reply-chain unless we're replying to a different area */

  if (!areaname)
    nmsg.replyto=rp.original;

  /* Now copy the From: line off the old message into our 'To:' line */

  strcpy(nmsg.to, omsg.from);

  /* Copy the subject line */

  strncpy(nmsg.subj, omsg.subj, XMSG_SUBJ_SIZE-1);
  nmsg.subj[XMSG_SUBJ_SIZE-1]='\0';

  /* Copy the origination address from the other message */
  
  nmsg.dest=omsg.orig;

  /* Now copy it into the default address buffer */
  
  if (isnet && omsg.orig.net!=0)
    strcpy(netnode, Address(&omsg.orig));
  else
    *netnode='\0';

  /* Get the header information */
  
  isareply=TRUE;
  isachange=FALSE;

  rc=0;

  if (GetMsgAttr(&nmsg, &mah, usr.msg, 0L, -1L)==-1)
    aborted=TRUE;
  else
  {
    editrc=Editor(&nmsg, NULL, 0L, NULL, &rp);

    isareply=FALSE;

    if (editrc==ABORT)
      aborted=TRUE;
    else
    {
      if (editrc==LOCAL_EDIT)
        aborted=FALSE;
      else
        aborted=SaveMsg(&nmsg, NULL, FALSE, 0L, FALSE, &mah,
                        rp.toareaname, sq, NULL, rp.fromareaname, FALSE);

      /* Write original message again, to set the REPLY link correctly. */
      /* But don't ! do this if we're replying in a different area      */

      omsgh=NULL;

      if (!areaname && !aborted &&
          (omsgh=MsgOpenMsg(rp.fromsq, MOPEN_RW, MsgUidToMsgn(rp.fromsq,rp.original,UID_EXACT))) != NULL &&
          MsgReadMsg(omsgh, &omsg, 0L, 0L, NULL, 0L, NULL) != -1)
      {
        /* Drop the reply link in the correct place */

        for (i=0; i < MAX_REPLY; i++)
          if (! omsg.replies[i] || i==MAX_REPLY-1)
          {
            omsg.replies[i]=MsgMsgnToUid(sq, MsgHighMsg(sq));
            break;
          }

        MsgWriteMsg(omsgh, FALSE, &omsg, NULL, 0L, 0L, 0L, NULL);
      }

      if (omsgh)
        MsgCloseMsg(omsgh);

#ifdef MAX_TRACKER
      /* Allow user to adjust tracking status, if necessary */

      TrackAfterReply(ctrl, last_msg, rp.fromsq);
#endif
    }
  }

  if (aborted)
  {
    Puts(msg_aborted);
    rc=-1;
  }

  if (ctrl)
    free(ctrl);

  if (areaname)
  {
    CopyMsgArea(&mah, &rp.fromarea);
    MsgCloseArea(sq);
    last_msg=MsgUidToMsgn(rp.fromsq, rp.original, UID_PREV);
  }

  DisposeMah(&rp.fromarea);
  sq=rp.fromsq;

  isareply=FALSE;

  return rc;
}


