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
static char rcs_id[]="$Id: mb_novl.c,v 1.2 2004/01/11 19:59:50 wmcbrine Exp $";
#pragma on(unreferenced)

/*# name=Message Section: "root" overlay code for B)rowse command
*/

#define MAX_LANG_m_browse

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <mem.h>
#include <share.h>
#include "dr.h"
#include "prog.h"
#include "max_msg.h"
#include "m_browse.h"

int idling;
int last_title;
struct _lrptr *lrptr;

static int near Browse_Scan_This_Area(BROWSE *b, PMAH pmah, BARINFO *pbi);



/* Free the chain of lastread pointers to update */

void Lmsg_Free(void)
{
  struct _lrptr *lr, *next;
#ifdef MIKE_GOVE
  word n_freed=0;
#endif
  
  for (lr=lrptr; lr; next=lr->next, free(lr), lr=next)
  {
#ifdef MIKE_GOVE
    n_freed++;
#endif
    if (lr->path)
      free(lr->path);
  }
    
  lrptr=NULL;

#ifdef MIKE_GOVE
  logit("@Lmsg_Free - Freed %d pointers", n_freed);
#endif
}





/* Add a lastread pointer to the linked list of pointers to update */

void Lmsg_Set(BROWSE *b, long msgn)
{
  struct _lrptr *lp;

  /* Scan the llist for prior entries for this node. */

  for (lp=lrptr; lp; lp=lp->next)
    if (lp->path && eqstr(lp->path, b->path))
      break;


  /* If this node wasn't found in the linked list, then add it */
    
  if (lp==NULL)
  {
#ifdef MIKE_GOVE
    logit("@Lmsg_Set - Area %s added to lrptr list", b->path);
#endif

    if ((lp=malloc(sizeof(struct _lrptr)))==NULL)
      return;
    
    memset(lp, '\0', sizeof(struct _lrptr));

    lp->next=lrptr;
    lp->type=mah.ma.type;
    lrptr=lp;
  }
#ifdef MIKE_GOVE
  else
  {
    logit("@Lmsg_Set - Area %s already in lrptr list", b->path);
  }
#endif

  if (lp->path==NULL)
    lp->path=strdup(b->path);
  
#ifdef MIKE_GOVE
  logit("@Lmsg_Set - Set pointer to %ld", msgn);
#endif
  lp->msgn=msgn;
}



void Lmsg_Update(BROWSE *b)
{
  int using_sq;
  HAREA lsq;
  struct _lrptr *lp;

  NW(b);

#ifdef MIKE_GOVE
  logit("@Lmsg_Update");
#endif

  /* Walk the list of pointers, and update them all */
  
  for (lp=lrptr; lp; lp=lp->next)
  {
    if (lp->path==NULL)
      continue;

#ifdef MIKE_GOVE
    logit("@  Updating %s: set to %ld", lp->path, lp->msgn);
#endif

    /* Open the message area to access the .SQI file for translation */

    if (eqstri(lp->path, MAS(mah, path)) && sq)
    {
      using_sq=TRUE;
      lsq=sq;
    }
    else
    {
      using_sq=FALSE;

      if ((lsq=MsgOpenArea(lp->path, MSGAREA_NORMAL, lp->type))==NULL)
        continue;
    }


    /* Now adjust the LR pointer for this area only */

    FixLastread(lsq, lp->type, lp->msgn, lp->path);

#ifdef MIKE_GOVE
    if (lp->type & MSGTYPE_SQUISH)
    {
      dword msgn;
      UMSGID uid;
      char temp[PATHLEN];
      int fd;

      sprintf(temp, sq_lastread, lp->path);

      if ((fd=shopen(temp, O_RDONLY | O_BINARY))==-1)
        logit("@  Error checking %s", temp);
      else
      {
        long ofs=sizeof(UMSGID)*(long)usr.lastread_ptr;

        if (lseek(fd, ofs, SEEK_SET) != ofs)
          logit("@  Error seeking %s", temp);

        if (read(fd, (char *)&uid, sizeof uid) != sizeof uid)
          logit("@  Error reading %s", temp);

        close(fd);

        msgn=MsgUidToMsgn(lsq, uid, UID_EXACT);

        if (msgn==lp->msgn)
          logit("@  Verified pointer is at %ld", msgn);
        else
        {
          logit("@  !! Set to %ld, but read shows %ld", lp->msgn, msgn);

          msgn=MsgUidToMsgn(lsq, uid, UID_PREV);
          logit("@  UID_PREV=%ld", msgn);

          msgn=MsgUidToMsgn(lsq, uid, UID_NEXT);
          logit("@  UID_NEXT=%ld", msgn);
        }
      }
    }
#endif
      
    if (!using_sq)
      MsgCloseArea(lsq);
  }

  /* Now free the lrptr chain for next time */
  
  Lmsg_Free();
}



int OkToFixLastread(BROWSE *b)
{
  return ( (b->bflag & (BROWSE_NEW | BROWSE_ALL | BROWSE_FROM)) &&
           b->first->txt==NULL && b->first->next==NULL &&
           b->first->attr==0L &&
           (b->first->flag & (SF_NOT_ATTR | SF_HAS_ATTR))==0 );
}

static char whirlygig[]="|/-\\";
static int whirly=FALSE;

int List_Idle(BROWSE *b)
{
  static int gig=0;
  static int ctr=0;
  
  NW(b);
  
  /* Display a dot every 32 msgs */

  if (!b->fSilent && (++ctr % 32)==0)
  {
    idling=TRUE;

    Putc(whirly ? '\x08' : ' ');
    
    whirly=TRUE;
    Putc(whirlygig[gig++]);

    if (gig >= 4)
      gig=0;

    vbuf_flush();
  }

  Mdm_check();

  if (halt())
    return -1;

  return 0;
}

int List_Status(BROWSE *b,char *aname,int colour)
{
  /* Display "Searching:xx" if not doing just the current area */
  
  if ((b->bflag & BROWSE_ACUR)==0)
  {
    Rev_Up();
    Printf(srchng,(colour % 7)+9,aname);
    last_title=TRUE;
    vbuf_flush();
  }

  Mdm_check();

  if (halt())
    return -1;

  return 0;
}








/* Erase any dots displayed by List_Idle()... */

void Rev_Up(void)
{
  int x;
  
  if (usr.video)      /* Fast way */
    Puts("\r" CLEOL);
  else if (!hasRIP())
  {
    /* Slow way -- space over everything */

    Putc('\r');

    /* Erase the "searching:" msg, too */

    for (x=strlen(srchng)+11; x--;)
      Putc(' ');

    Putc('\r');
  }

  whirly=FALSE;
}


/* fix !@#$!@#$ internal compiler error */

#ifdef __MSC__
#pragma optimize("e", off)
#endif


int Browse_Scan_Areas(BROWSE *b)
{
  BARINFO bi;
  MAH ma;
  HAFF haff;
  int ret, rc;
  int colour=0, stop, bret;
  word ixnum;
 
  memset(&ma, 0, sizeof ma);

  display_line=display_col=1;
  whirly=FALSE;

  /* If we're searching other areas besides our own */

  if ((b->bflag & BROWSE_QWK)==0)
    logit(log_start_browse);

  if ((*b->Begin_Ptr)(b)==-1)
    return -1;

  ret=0;
  stop=FALSE;

  /* Open the tag data file for speedy t)ag access */

  ixnum=0;

  if ((haff=AreaFileFindOpen(ham, NULL, 0))==NULL)
    return -1;

  while (AreaFileFindNext(haff, &ma, FALSE)==0 && !stop)
  {
    if (Browse_Scan_This_Area(b, &ma, &bi))
    {
      if (!PopPushMsgArea(MAS(ma, name), &bi))
      {
        errno=-1;
        cant_open(MAS(ma, path));
        continue;
      }

      b->sq=sq;
      b->type=ma.ma.type;
      b->path=MAS(ma, path);

      if (b->bflag & BROWSE_NEW)
      {
        b->bdata=last_msg;

        #ifdef MIKE_GOVE
          logit("@BSA - lr for %s is %ld", b->path, b->bdata);
        #endif
      }
      else if (b->bflag & BROWSE_FROM)
        b->bdata--;
      else b->bdata=0L;


      /* Only display area title if we're not doing just this area */

      if ((rc=(*b->Status_Ptr)(b, MAS(ma, name), colour++))==-1)
      {
        ret=-1;
        stop=TRUE;
        break;
      }


      /* If the status program returned 1, skip the current area */

      if (rc==1)
        continue;


      bret=MsgBrowseArea(b);

      if (b->After_Ptr)
        (*b->After_Ptr)(b);


      if (bret==-1 || halt())
      {
        ret=-1;
        stop=TRUE;
        break;
      }
    }
  }

  AreaFileFindClose(haff);
  Puts(space_over);

  /* Close the tag data file */

  /*
  if ((*b->End_Ptr)(b)==-1)
  {
    ret=-1;
    stop=TRUE;
  }
   f:\max\src\mb_novl.c
   f:\max\src\mb_novl.c(327) : fatal error C1001: Internal Compiler Error
   		(compiler file '@(#)regMD.c:1.100', line 3837)
   		Contact Microsoft Product Support Services
        
   !@#$!@#$ microslush compiler!  :-(
     
  */

    
  if ((*b->End_Ptr)(b)==-1)
  {
    ret=-1;
    stop=TRUE;
  }
  
  if (b->matched==0 && !stop)
  {
    if ((b->bflag & (BROWSE_READ|BROWSE_EXACT))==(BROWSE_READ|BROWSE_EXACT))
      Display_File(0, NULL, PRM(nomail));
    else
    {
      Puts(br_notfound);
      vbuf_flush();
    }
  }
  
  Lmsg_Free();

  return ret;
}

#ifdef __MSC__
#pragma optimize("", on)
#endif


static int near Browse_Scan_This_Area(BROWSE *b, PMAH pmah, BARINFO *pbi)
{
  if (b->bflag & BROWSE_ACUR)        /* Only scan current area */
  {
    if (! eqstri(usr.msg, PMAS(pmah, name)))
      return FALSE;
  }
  else
  {
    if ((pmah->ma.attribs_2 & MA2_NOMCHK) && b->first &&
        (b->first)->attr==MSGREAD && (b->first)->flag==(SF_NOT_ATTR|SF_OR) &&
        (b->first)->where==WHERE_TO)
    {
      return FALSE;   /* Skip this area if this is a mailcheck */
    }

    if (b->bflag & BROWSE_ATAG)   /* Only scan tagged areas */
    {
      if (!TagQueryTagList(&mtm, PMAS(pmah, name)))
        return FALSE;
    }
    else if (b->bflag & BROWSE_AGRP)
    {
      int wc_len=strlen(b->wildcard);

      /* If we're doing a group browse, only browse areas at the same       *
       * level (or "under") this area.                                      */

      if (wc_len && strnicmp(PMAS(pmah, name), b->wildcard, wc_len) != 0)
        return FALSE;
    }
    else if (b->bflag & BROWSE_AWLD)
    {
      if (!BrowseWCMatch(b->wildcard, PMAS(pmah, name)))
        return FALSE;
    }
    else if (b->bflag & BROWSE_ALNT)
    {
      if (pmah->ma.attribs & (MA_ECHO|MA_CONF))
        return FALSE;
    }
  }

  /* else must be BROWSE_AALL, so scan all areas */


  /* Now check to make sure that we have the correct access level */

  return (ValidMsgArea(NULL, pmah, VA_NOVAL | VA_PWD | VA_EXTONLY, pbi));
}




int Match_All(BROWSE *b)
{
  return (CanSeeMsg(&b->msg));
}

  

int List_End(BROWSE *b)
{
  NW(b);

  if (last_title)
    Puts(space_over);

  Rev_Up();

  return 0;
}

