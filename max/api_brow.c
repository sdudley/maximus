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

/* $Id: api_brow.c,v 1.3 2004/01/22 08:04:26 wmcbrine Exp $ */

#define NO_MSGH_DEF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "alc.h"
#include "prog.h"
#include "mm.h"
#include "max_area.h"
#include "msgapi.h"
#include "old_msg.h"
#include "api_sq.h"
#include "api_brop.h"
#include "m_browse.h"
#include "scanbld.h"



sword EXPENTRY MsgBrowseArea(BROWSE *b)
{
  SEARCH *s;
  int bcsf_ret;
  int ret;
  int x;

  /* If we're only scanning for OUR mail, then we only need the             *
   * message subject and/or attributes, so we can use SCANFILE.DAT          *
   * or the .SQuishIndex file.                                              */

  if ((b->bflag & BROWSE_NEW) &&
      (b->bflag & BROWSE_EXACT) &&
      b->first->where==WHERE_TO)
  {
    /* Now, scan all of the elements in the chain.  Unless we're ONLY       *
     * searching for entries in the 'To' field, and unless we're only       *
     * doing an OR operation, then we can't use the ScanFile method.        */

    for (s=b->first; s; s=s->next)
      if (b->first->where != WHERE_TO || (b->first->flag & SF_OR)==0)
        break;

    if (s==NULL)
    {
      bcsf_ret=BrowseCheckScanFile(b);
  
      if (bcsf_ret <= 0)
        return bcsf_ret;
      /* else do normal scan of area */
    }
  }

  /* Add one to bdata to compensate for the fact that if we're reading      *
   * NEW messages, then we don't want to start reading until AFTER the      *
   * lastread pointer.  We've subtracted one from the other bdata           *
   * cases above, so it all works out evenly.                               */

  for (ret=0,b->msgn=b->bdata+1; b->msgn <= MsgHighMsg(b->sq); b->msgn++)
  {
    if ((x=Browse_Scan_Message(b))==-1)
    {
      /* User hit ^c or replied "N" to the next msg question. */

      ret=-1;
      break;
    }
    else if (x==3)
    {
      /* User hit "*" to go to next area, so return a SUCCESS error code */

      ret=0;
      break;
    }
  }

  return ret;
}


static int near BrowseCheckScanFile(BROWSE *b)
{
  SBREC *psh;
  SQIDX *siptr=NULL;
  NETADDR newdest;

  char temp[PATHLEN];
  char *block=NULL;
  
  UMSGID low_uid=0L;
  int recsize, blsiz, got, in;
  int ret, sfd;

  /* If we're searching for messages above the lastread, make sure that     *
   * we don't waste time looking at those below.  Calculate the lowest      *
   * UID that we should start looking at.                                   */
     
  if (b->bflag & BROWSE_NEW)
    low_uid=MsgMsgnToUid(b->sq, b->bdata);

  ret=0;

  if (b->type==MSGTYPE_SDM)
  {
    sprintf(temp, scanfile_name, b->path);
    recsize=sizeof(SBREC);
    blsiz=SCAN_BLOCK_SBREC;
  }
  else if (b->type==MSGTYPE_SQUISH)
  {
#ifndef UNIX
    sprintf(temp, "%s.SQI", b->path);
#else
    sprintf(temp, "%s.sqi", b->path);
#endif

    recsize=sizeof(SQIDX);
    blsiz=SCAN_BLOCK_SQUISH;
  }
  else return 1;


  /* Allocate a big buffer for reading in the scanfile */

  for (; blsiz > 0 && (block=(char *)malloc(blsiz*recsize))==NULL; blsiz /= 2)
    ;

  /* If we couldn't allocate enough memory, then return */
  if (!block)
    return 1;


  /* If we can't open it, proceed with a normal scan of the area */

  if ((sfd=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    free(block);
    return 1;
  }


  /* Seek over the preliminary header for the SCANFILE.DAT file */

  if (b->type==MSGTYPE_SDM)
    lseek(sfd, sizeof(SBHDR), SEEK_SET);

  newdest=prm.address[0];

  while ((got=read(sfd, block, blsiz*recsize)) >= recsize)
  {
    got /= recsize;

    for (in=0; in < got; in++)
    {
      b->msg.dest=newdest;

      if (b->type==MSGTYPE_SDM)
      {

        psh=(SBREC *)block+in;
        
        strcpy((char *)b->msg.to, (char *)psh->to);
        b->msg.attr=(long)psh->attr;
        
        if ((b->bflag & BROWSE_NEW) && (UMSGID)psh->msgnum < low_uid)
          continue;
      }
      else if (b->type==MSGTYPE_SQUISH)
      {
        siptr=(SQIDX *)(block+(sizeof(SQIDX)*in));

        *(dword *)b->msg.to=siptr->hash;
        b->msg.attr=(siptr->hash & 0x80000000Lu) ? MSGREAD : 0;
        b->bflag |= BROWSE_HASH;

        if ((b->bflag & BROWSE_NEW) && siptr->umsgid < low_uid)
          continue;
      }

      *b->msg.from='\0';
      *b->msg.subj='\0';

      /* Double-check with the actual message, to ensure that the message   *
       * attributes haven't been changed.                                   */

      if (BrowseMatchMessage(b, NULL, FALSE))
      {
        b->bflag &= ~BROWSE_HASH;
        
        b->msgn=MsgUidToMsgn(b->sq,
                             (b->type==MSGTYPE_SQUISH) ? siptr->umsgid
                                                       : psh->msgnum,
                             UID_EXACT);

        /* Now scan the message itself, if it was a true match */

        switch (Browse_Scan_Message(b))
        {
          case -1:  ret=-1; goto DoRet;
          case 3:   ret=0;  goto DoRet;
        }
      }

      b->bflag &= ~BROWSE_HASH;
    }
  }

DoRet:

  close(sfd);
  free(block);
  
  return ret;
}





static int near Browse_Scan_Message(BROWSE *b)
{
  HMSG m;
  char *msgtxt;
  long curtl;
  int ret=0;
  int x;
  word bfsave=b->bflag;

  if ((m=MsgOpenMsg(b->sq, MOPEN_RW, b->msgn))==NULL)
    return 0;

  msgtxt=NULL;

  /* If we're supposed to read the message text, first check to see         *
   * that the message HAS text to be read, and also check that we           *
   * have enough memory to read it.  If not, turn off the text-browsing     *
   * feature for this message.                                              */

  curtl=MsgGetTextLen(m);
  
  #if defined(__MSDOS__) || defined(OS_2)

  /* Make sure that message isn't over a segment in length, so that we      *
   * don't overflow the browse buffer.                                      */

  curtl=(word)curtl;
  #endif

  if (b->bflag & BROWSE_GETTXT)
    if (curtl <= 0L || (msgtxt=(char *)malloc((word)curtl+10))==NULL)
      b->bflag &= ~BROWSE_GETTXT;

  if ((b->bflag & BROWSE_GETTXT)==0)
    curtl=0L;
  
#ifdef OS_2
  /* MSGAPI.DLL is in the large memory model, nd a (char near *)NULL    *
   * doesn't get automtically expanded by the compiler                  */

  if (MsgReadMsg(m, &b->msg, 0L, curtl,
                 msgtxt==NULL ? (byte far *)NULL : (byte far *)msgtxt,
                 0L, NULL)==-1)
#else
  if (MsgReadMsg(m, &b->msg, 0L, curtl, msgtxt, 0L, NULL)==-1)
#endif
  {
    if (msgapierr==MERR_NOMEM)
      ret=-1;
  }
  else
  {
    if (msgtxt)
      msgtxt[(unsigned)curtl]='\0';

    if (BrowseMatchMessage(b, msgtxt, TRUE))
    {
      b->matched++;
      b->m=m;

      if (b->Display_Ptr==NULL || (x=(*b->Display_Ptr)(b))==-1)
        ret=-1;
      else if (x==3)
        ret=3;

      m=b->m;
    }
    else if (b->Idle_Ptr && (*b->Idle_Ptr)(b)==-1)
      ret=-1;

  }

  if (msgtxt)
    free(msgtxt);

  if (m)
    MsgCloseMsg(m);
  
  b->bflag=bfsave;

  return ret;
}

int near StringMatchInStr(char *msg, char *search)
{
  char *p;
  char *orig;

  p=stristr(msg,search);

  if (p==NULL)
    return FALSE;

  orig=stristr(msg," * Origin:");

  if (orig && orig < p)
    return FALSE;
  else return (p != NULL);
}


int near HashMatchEqual(char *msg, char *search)
{
  return ((*(dword *)msg & 0x7fffffffLu)==SquishHash((byte *)search));
}

int near StringMatchEqual(char *msg, char *search)
{
  return (eqstri(msg, search));
}


static int near BrowseMatchMessage(BROWSE *b, char *msgtxt, word checkaddr)
{
  typedef int (near *eq_func)(char *msg, char *search);
  eq_func equal;
  SEARCH *s;
  int doit=FALSE;
  int match;

  if (b->bflag & BROWSE_HASH)
    equal=HashMatchEqual;
  else if (b->bflag & BROWSE_EXACT)
    equal=StringMatchEqual;
  else
    equal=StringMatchInStr;

  /* First check to make sure that the message matches our search           *
   * criteria...                                                            */
     
  for (s=b->first;s;s=s->next)
  {
    match=FALSE;

    if (s->txt && *s->txt)
    {
      if (s->where & WHERE_TO)
        if ((*equal)((char *)b->msg.to, s->txt))
        {
          /* If we're not doing an exact search, or if we ARE doing an      *
           * exact search and it's addressed to our net address, also       *
           * mark it.                                                       */
          
          if ((mah.ma.attribs & MA_NET)==0 ||
              (b->bflag & BROWSE_EXACT)==0 ||
              !checkaddr ||
              MsgToUs(&b->msg.dest))
          {
            match=TRUE;
          }
        }

      if (s->where & WHERE_FROM)
        if ((*equal)((char *)b->msg.from, s->txt))
          match=TRUE;

      if (s->where & WHERE_SUBJ)
        if ((*equal)((char *)b->msg.subj, s->txt))
          match=TRUE;

      if ((s->where & WHERE_BODY) && (b->bflag & BROWSE_GETTXT) && msgtxt)
        if ((*equal)(msgtxt, s->txt))
          match=TRUE;
    }
      
    if ((s->where & WHERE_ALL)==0)
      match=TRUE;


    /* Now check to make sure that the attributes match */
    
    if (match)
    {
      if (s->flag & SF_HAS_ATTR)
        if (! (b->msg.attr & s->attr))
          match=FALSE;
        
      if (s->flag & SF_NOT_ATTR)
        if (b->msg.attr & s->attr)
          match=FALSE;
    }
    
    /* Finally, factor this into the grand scheme of things, by adding      *
     * in the AND/OR logic for this record.                                 */
      
    if (s->flag & SF_OR)
    {
      if (match && !doit)
        doit=TRUE;
    }
    else if (s->flag & SF_AND)
    {
      if (match && doit)
        doit=TRUE;
      else doit=FALSE;
    }
  }

  return doit && b->Match_Ptr && (*b->Match_Ptr)(b);
}


