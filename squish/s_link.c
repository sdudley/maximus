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
static char rcs_id[]="$Id: s_link.c,v 1.2 2003/06/05 03:13:40 wesgarland Exp $";
#pragma on(unreferenced)

#define NOVARS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_link.h"

static char *etn=NULL;

void Link_Messages(char *etname)
{
  etn=etname;

  S_LogMsg("#Linking messages");

  (void)printf("Linking areas:\n\n");
  Do_Echotoss(etname, Link_Area, TRUE, NULL);
  (void)printf("\n");
}







static void Link_Area(struct _cfgarea *ar, HAREA opensq)
{
  HAREA sq;

  NW(opensq);
  NW(config);
  
  /* Only relink echo areas */

  if (etn==NULL)
  {
    if (NetmailArea(ar) || BadmsgsArea(ar) || DupesArea(ar))
      return;
  }

  if ((sq=MsgOpenArea(ar->path, MSGAREA_NORMAL, ar->type))==NULL)
    return;

  if (MsgHighMsg(sq) != 0)
  {
    (void)MsgLock(sq);
    LinkIt(sq, ar);
    (void)MsgUnlock(sq);
  }

  (void)MsgCloseArea(sq);
  
  return;
}


/* Convert a ^a-style string to a binary hash */

static void near LinkGetMsgid(char *s, dword *hash, dword *stamp)
{
  char *p;

  if ((p=strrchr(s, ' ')) != NULL)
    if (sscanf(p+1, "%8" INT32_FORMAT "x", stamp)==1)
      *hash=SquishHash(s+7);
}


/* Read this message area into memory */

static long near LinkReadArea(HAREA sq, struct _cfgarea *ar, struct _link **link)
{
  #define MAX_CTRL_LINK 2048

  HMSG mh;
  XMSG msg;
  char *ctrl_buf, *s;
  size_t nl;
  dword mn, max;

  /* Make sure not to relink the HWM in a *.MSG area */

  mn=(ar->type & MSGTYPE_SDM) ? 2L : 1L;
  max=MsgHighMsg(sq);

  if (config.flag2 & FLAG2_LMSGID)
    ctrl_buf=malloc(MAX_CTRL_LINK);
  else
    ctrl_buf=NULL;

  for (nl=0; mn <= max; mn++)
  {
    if ((mh=MsgOpenMsg(sq, MOPEN_READ, mn))==NULL)
      continue;

    if (MsgReadMsg(mh, &msg, 0L, 0L, NULL, MAX_CTRL_LINK, ctrl_buf)==(dword)-1)
    {
      (void)MsgCloseMsg(mh);
      continue;
    }

    link[nl]=malloc(sizeof(struct _link));

    /* If we ran out of memory, abort gracefully */

    if (!link[nl])
    {
      size_t i;

      for (i=0; i < nl; i++)
      {
        free(link[i]);
        link[i]=NULL;
      }

      (void)MsgCloseMsg(mh);

      S_LogMsg("!Not enough mem to read link data for %s", ar->name);

      nl=0;
      break;
    }

    link[nl]->mnum=mn;

    (void)strncpy(link[nl]->subj, msg.subj, sizeof(link[nl]->subj));

    link[nl]->subj[sizeof(link[nl]->subj)-1]='\0';
    link[nl]->up=msg.replyto;
    memmove(link[nl]->downs, msg.replies, MAX_REPLY * sizeof(UMSGID));

    link[nl]->msgid_hash=link[nl]->msgid_stamp=0L;

    /* If we have any ^aMSGID tokens in this control text */

    if (ctrl_buf && (s=MsgGetCtrlToken(ctrl_buf, "MSGID")) != NULL)
    {
      LinkGetMsgid(s, &link[nl]->msgid_hash, &link[nl]->msgid_stamp);
      MsgFreeCtrlToken(s);
    }

    if (ctrl_buf && (s=MsgGetCtrlToken(ctrl_buf, "REPLY")) != NULL)
    {
      LinkGetMsgid(s, &link[nl]->reply_hash, &link[nl]->reply_stamp);
      MsgFreeCtrlToken(s);
    }

    link[nl++]->delta=FALSE;

    if ((nl % 25)==0 && (config.flag2 & FLAG2_QUIET)==0)
      (void)printf("\b\b\b\b\b%" SIZET_FORMAT,nl);

    (void)MsgCloseMsg(mh);
  }

  if (ctrl_buf)
    free(ctrl_buf);

  return nl;
}


/* Link this message area using by-subject linking */

static void near LinkSubject(HAREA sq, long nl, struct _link **link)
{
  char *s1, *s2;
  long lnk, max;
  UMSGID up, down;

  qsort(link, (size_t)nl, sizeof(struct _link *), msgcomp);

  if (!nl)
    return;

  for (lnk=0, max=nl-1; lnk < max; lnk++)
  {
    s1=link[(size_t)lnk]->subj;
    s2=link[(size_t)lnk+1]->subj;

    if (toupper(s1[0])=='R' && toupper(s1[1])=='E' && s1[2]==':')
      s1 += 4;

    if (toupper(s2[0])=='R' && toupper(s2[1])=='E' && s2[2]==':')
      s2 += 4;

    if (eqstri(s1, s2))
    {
      up=MsgMsgnToUid(sq, link[(size_t)lnk]->mnum);
      down=MsgMsgnToUid(sq, link[(size_t)lnk+1]->mnum);
    }
    else
    {
      up=0L;
      down=0L;
    }

    if ((UMSGID)link[(size_t)lnk+1]->up != up)
    {
      link[(size_t)lnk+1]->up=up;
      link[(size_t)lnk+1]->delta=TRUE;
    }

    if ((UMSGID)link[(size_t)lnk]->downs[0] != down)
    {
      link[(size_t)lnk]->downs[0]=down;
      link[(size_t)lnk]->delta=TRUE;
    }
  }
}


/* Link this message area using by-subject linking */

static void near LinkMsgid(HAREA sq, long nl, struct _link **link)
{
  UMSGID new_down, new_up;
  size_t lnk;
  int up;
  int i;

  qsort(link, (size_t)nl, sizeof(struct _link *), msgidcomp);

  if (!nl)
    return;

  for (lnk=0; lnk < nl; lnk++)
  {
    /* Skip msgs which are not replies */

    if (link[lnk]->reply_hash==0 && link[lnk]->reply_stamp==0)
      continue;

    /* If the parent no longer exists, skip. */

    if ((up=(int)msgidsearch(link, nl, link[lnk]))==-1)
      continue;

    /* Update our messages' "up" link */

    new_up=MsgMsgnToUid(sq, link[up]->mnum);

    if (link[lnk]->up != new_up)
    {
      link[lnk]->up=new_up;
      link[lnk]->delta=TRUE;
    }

    new_down=MsgMsgnToUid(sq, link[lnk]->mnum);

    /* See if this link is already stored */

    for (i=0; i < MAX_REPLY; i++)
      if (link[up]->downs[i]==new_down)
        break;

    /* If we need to add a pointer... */

    if (i==MAX_REPLY)
    {
      link[up]->delta=TRUE;

      /* Now add this message number to its 'this is a reply to' list */

      for (i=0; i < MAX_REPLY; i++)
        if (!link[up]->downs[i])
        {
          link[up]->downs[i]=new_down;
          break;
        }
    }
  }
}



/* Update the message headers which have 'delta' marked */

static void near LinkUpdateMsgs(HAREA sq, struct _link **link, long nl)
{
  size_t lnk;
  XMSG msg;
  HMSG mh;

  for (lnk=0; lnk < (size_t)nl; lnk++)
  {
    if ((lnk % 25)==0 && (config.flag2 & FLAG2_QUIET)==0)
      (void)printf("\b\b\b\b\b%" SIZET_FORMAT, lnk);

    if (! link[lnk]->delta ||
        (mh=MsgOpenMsg(sq, MOPEN_RW, link[lnk]->mnum))==NULL)
    {
      free(link[lnk]);
      continue;
    }

    if (MsgReadMsg(mh, &msg, 0L, 0L, NULL, 0L, NULL) != (dword)-1)
    {
      msg.replyto=link[lnk]->up;

      /* Sort the replies so that the reply chain shows up in order */

      memmove(msg.replies, link[lnk]->downs, sizeof(UMSGID) * MAX_REPLY);

      if (config.flag2 & FLAG2_LMSGID)
      {
        int i;

        /* Find the first zero reply */

        for (i=0; i < MAX_REPLY; i++)
          if (!msg.replies[i])
            break;

        qsort(msg.replies, i, sizeof(UMSGID), umsgidcomp);
      }


      (void)MsgWriteMsg(mh, TRUE, &msg, NULL, 0L, 0L, 0L, NULL);
    }

    (void)MsgCloseMsg(mh);
    free(link[lnk]);
  }

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("\b\b\b\b\b%" POINTER_FORMAT, (void *)nl);
}


/* Link the area specified by 'sq' */

static void near LinkIt(HAREA sq, struct _cfgarea *ar)
{
  struct _link **link;
  long nl;

  (void)printf("Linking area %-40s", ar->name);

  link=malloc((size_t)MsgNumMsg(sq)*sizeof(struct _link *));

  if (!link)
  {
    S_LogMsg("!Not enough memory to link area %s", ar->name);
    return;
  }

  (void)memset(link, '\0', (size_t)MsgNumMsg(sq)*sizeof(struct _link *));
  
  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf(" - Read -----");

  nl=LinkReadArea(sq, ar, link);

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("\b\b\b\b\b\b\b\b\b\bSort      ");

  if (nl)
  {
    if (config.flag2 & FLAG2_LMSGID)
      LinkMsgid(sq, nl, link);
    else
      LinkSubject(sq, nl, link);
  }

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("\b\b\b\b\b\b\b\b\b\bLink -----");

  LinkUpdateMsgs(sq, link, nl);
  free(link);

  (void)printf("\n");
}





static int _stdc msgcomp(const void *i1, const void *i2)
{
  struct _link **l1, **l2;
  char *subj1;
  char *subj2;
  int eq;

  l1=(struct _link **)i1;
  l2=(struct _link **)i2;
  
  subj1=(*l1)->subj;
  subj2=(*l2)->subj;

  if (toupper(subj1[0])=='R' && toupper(subj1[1])=='E' && subj1[2]==':')
    subj1 += 4;

  if (toupper(subj2[0])=='R' && toupper(subj2[1])=='E' && subj2[2]==':')
    subj2 += 4;

  eq=stricmp(subj1,subj2);

  if (eq)
    return eq;
  else return ((int)((*l1)->mnum - (*l2)->mnum));
}


/* Sort the array based on MSGIDs */

static int _stdc msgidcomp(const void *i1, const void *i2)
{
  struct _link **l1, **l2;
  long eq;

  l1=(struct _link **)i1;
  l2=(struct _link **)i2;
  
  eq=(*l1)->msgid_hash - (*l2)->msgid_hash;

  if (eq==0)
    eq=(*l1)->msgid_stamp-(*l2)->msgid_stamp;

  return (eq==0) ? 0 : (eq > 0) ? 1 : -1;
}

/* Peform a comparison based on messages' UMSGIDs */

static int _stdc umsgidcomp(const void *i1, const void *i2)
{
  long eq;

  eq=*(dword *)i1 - *(dword *)i2;

  return eq==0 ? 0 : eq > 0 ? 1 : -1;
}


/* Perform a binary search for a MSGID hash in the 'link' array */

static size_t msgidsearch(struct _link **link, long nl, struct _link *find)
{
  struct _link *tryl;
  size_t lo, hi, try, last_try;
  long eq;

  lo=0;
  hi=(size_t)nl-1;
  try=-1;

  do
  {
    last_try=try;
    try=((hi - lo) >> 1) + lo;

    tryl=link[try];

    eq=tryl->msgid_hash - find->reply_hash;

    if (eq==0)
      eq=tryl->msgid_stamp - find->reply_stamp;

    if (eq==0)
      return try;

    if (eq < 0)
      lo=try;
    else hi=try;
  }
  while (lo < hi && try != last_try);

  return -1;
}

