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
static char rcs_id[]="$Id: s_scan.c,v 1.1.1.1 2002/10/01 17:56:37 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <time.h>
#include <ctype.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "bfile.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_scan.h"
#include "s_hole.h"
#include "s_dupe.h"


#ifdef DJ
extern FILE *dj;
#endif

static word full_scan=0;
static dword scan_ctr;
static char maxmsgs_scan[]="MAXMSGS2.DAT";

static void near cleanup_exit(int erl)
{
  MsgCloseApi();
  S_LogMsg("!Squish exiting abnormally");
  S_LogClose();
  exit(erl);
}

void Scan_Messages(char *etname, NETADDR *scan, time_t start)
{
  FILE *fp;
  char area[MAX_TAGLEN];
  time_t secs;

  /* Set rescan flag, if necessary */

  resc=scan;
  full_scan=TRUE;
  
  outbuf=cur_ob=end_ob=NULL;
  msgbuf=NULL;
  begin_sb=end_sb=NULL;
  scan_ctr=0;


  S_LogMsg("#Scanning messages%s", etname ? " from echo tosslog" : "");

  Alloc_Buffer(maxmsglen+sizeof(XMSG));
  msgbuf=buffer+MAX_TAGLEN;
  Alloc_Outbuf();

  Zero_Statistics();

  *area='\0';

  if ((fp=shfopen(maxmsgs_scan, "r", O_RDONLY)) != NULL)
  {
    fgets(area, MAX_TAGLEN, fp);
    fclose(fp);

    (void)unlink(maxmsgs_scan);

    (void)Strip_Trailing(area, '\n');
  }

  Do_Echotoss(etname, Scan_Area, !resc, area);

  Flush_Outbuf();
  Free_Outbuf();
  Free_Buffer();

  secs=time(NULL)-start;

  if (secs==0)
    secs=1;

  if (nmsg_scanned)
  {
    (void)printf("\nScanned %lu messages (%lu.%lu/second) and "
                 "sent %lu (%lu.%lu/second)\n",
                 nmsg_scanned,
                 (unsigned long)nmsg_scanned/secs,
                 (unsigned long)(nmsg_scanned*10Lu/secs) % 10Lu,
                 (unsigned long)nmsg_sent,
                 (unsigned long)nmsg_sent/secs,
                 (unsigned long)(nmsg_sent*10Lu/secs) % 10Lu);
  }

  Report_Statistics();

  full_scan=FALSE;

  /* Flush duplicate buffer */

  DupeFlushBuffer();
}





/* Scan all outgoing messages in a message area */

void Scan_Area(struct _cfgarea *ar, HAREA opensq)

{
  extern word mode;
  dword dwNewHWM=(dword)-1L;

  struct _sblist *sb;
  dword old_attr;
  XMSG msg;

  HAREA sq=NULL;
  HMSG mh;
  UMSGID uid;

  dword hmsg, hwm, bytes;

  unsigned num_sb, cl;
  int delete_me;
  word errcnt=0;
  unsigned added;

  
  /* Don't scan past max_msgs */

  if (erl_max)
    return;

  /* Don't scan BAD_MSGS, NETMAIL, or areas we don't need to touch */

  if (NetmailArea(ar) || BadmsgsArea(ar) || DupesArea(ar) ||
      (ar->num_scan==0 && !resc))
    return;

  if ((mode & MODE_toss) && (ar->flag & AFLAG_TOSSEDTO))
    return;

  /* If we're just scanning the residual stuff after an IN OUT (collecting  *
   * the junk left in echotoss.log), don't process this area if it was      *
   * already processed.                                                     */
  
 if (full_scan && (config.flag2 & FLAG2_NPTHRU) &&
     (ar->flag & AFLAG_PASSTHRU))
   return;

  if (!opensq)
    sq=MsgOpenArea(ar->path, MSGAREA_NORMAL, ar->type);
  else sq=opensq;

  if (! sq)
  {
    if (msgapierr != MERR_NOENT)
      (void)printf("Invalid area: `%s'\n", ar->name);

    return;
  }

  hwm=MsgGetHighWater(sq);
  hmsg=MsgHighMsg(sq);
  

  /* If we're rescanning, do it from the beginning */
  
  if (resc)
    hwm=0;

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("Scanning: %-30s (%05lu-%05lu)  -----",ar->name,
                 ((hwm+1 > hmsg) ? hmsg : hwm+1),hmsg);

  if (hwm < hmsg)
  {
    while (++hwm <= hmsg)
    {
      if (config.max_msgs && scan_ctr >= config.max_msgs)
        break;
         
      /* Don't scan 1.MSG in *.MSG areas */

      if (hwm==1 && (ar->type & MSGTYPE_SDM))
        continue;

      if ((mh=MsgOpenMsg(sq, MOPEN_RW, hwm))==NULL)
      {
        /* Only report an error if the problem WASN'T caused by the         *
         * non-existance of the requested message.  Gaps in the message     *
         * base will typically appear in *.MSG and other message-base       *
         * types.                                                           */

        if (msgapierr != MERR_NOENT)
        {
          S_LogMsg("!Can't read %s#%ld (err#%d)", ar->name, hwm, msgapierr);

          /* Get out if we encounter too many fatal errors */

          if (errcnt++ >= 50)
            break;
        }

        continue;
      }

      /* If the message is too long to handle... */

      if (MsgGetTextLen(mh) >= maxmsglen-256)
      {
        /* spit puke cough */

        S_LogMsg("!Msg %s#%ld too large to process (%ldb)", ar->name, hwm,
                 MsgGetTextLen(mh));

        (void)MsgCloseMsg(mh);
        continue;
      }

      cl=(word)(MsgGetCtrlLen(mh)+100L);
      
      if ((ctext=(byte *)malloc((size_t)(cl+128)))==NULL)
      {
        S_LogMsg("!Not enough free memory to complete processing (ctext)");
        (void)MsgCloseMsg(mh);
        continue;
      }

      /* Read everything in */

      bytes=MsgReadMsg(mh, &msg, 0L, (dword)maxmsglen-1, msgbuf,
                       (dword)CTEXT_LEN, ctext);

      if (bytes != (dword)-1L && bytes != 0L)
        msgbuf[(int)bytes]='\0';

      delete_me=FALSE;

      /* Only echo the msg if it's not zero length, if it needs to be       *
       * echoed (ie. doesn't have a 'NOSCAN' at the beginning), and         *
       * only if it hasn't been scanned before.  The only exception to      *
       * the last rule is if we're doing a rescan...                        */

      if (bytes != (dword)-1L && bytes != 0L &&
          NeedToEcho(msgbuf) &&
          ((msg.attr & MSGSCANNED)==0 || resc))
      {
        word scanit=TRUE;

        /* Convert seen-bys to a binary linked list */

        sb=Digitize_Seenbys(ar, msgbuf, &num_sb);

        /* Apply a check on the date field if the msg originated here */

        if (1 /*!resc*/)
        {
          if ((msg.attr & MSGLOCAL) || DestIsHereA(&msg.orig))
          {
            if (*msg.__ftsc_date && InvalidDate(msg.__ftsc_date))
            {
              S_LogMsg("!Grunged date in %s:%ld", ar->name, hwm);
              scanit=FALSE;
            }
          }
          else
          {
            /* If the message didn't come from here, make sure that it has  *
             * SEEN-BYs...                                                  */

            if (sb==NULL || num_sb==0)
            {
              S_LogMsg("!No SEEN-BYs in %s:%ld", ar->name, hwm);
              scanit=FALSE;
            }
          }
        }
        
        if (scanit)
        {
          /* Sort the list, and convert linked list into a flat array.
           * We need enough extra space to hold the scan nodes, the number
           * of add-to-seen nodes (from "-+"), the number of
           * add-to-seen nodes (from squish.cfg "addtoseen"),
           * an extra node if we're rescanning, plus our primary
           * address.
           */

          sb=Sort_And_Flatten(sb, 
                              num_sb, 
                              ar->num_scan + config.num_ats + ar->num_add +
                              !!resc + 1);

          old_attr=msg.attr;  /* save copy of message attribute */

          /* Scan the message to everyone.  If we actually scanned one,     *
           * then write the SEEN-BYs back to the message file (for          *
           * *.MSG systems only).                                           */

          if ((added=Scan_To_All(ar, sb, num_sb, &msg,
                                 ctext, NULL,
                                 MsgMsgnToUid(sq, hwm),
                                 &delete_me, sq, &hwm)) != 0 && !resc)
          {
            msg.attr=old_attr | MSGSCANNED;

            strlen_msgbuf=strlen(msgbuf+added-1);

            (void)MsgWriteMsg(mh,
                              FALSE,
                              (ar->type & MSGTYPE_SDM) ? NULL : &msg,
                              (ar->type & MSGTYPE_SDM) ? msgbuf+added-1  : NULL,
                              (dword)((ar->type & MSGTYPE_SDM)
                                 ? (unsigned)(strlen_msgbuf+1) : (unsigned)0),
                              (dword)((ar->type & MSGTYPE_SDM)
                                 ? (unsigned)(strlen_msgbuf+1) : (unsigned)0),
                              (dword)((unsigned)(strlen(ctext)+1)),
                              ctext);

            scan_ctr++;
          }

          /* And free the SEEN-BY list... */

          free(sb);
        }
      }

      free(ctext);

      (void)MsgCloseMsg(mh);
      mh=NULL;


      /* Don't kill messages or update the HWM when rescanning */

      if (!resc)
      {
        /* Delete passthru messages, if necessary */

        if ((ar->flag & AFLAG_PASSTHRU) || delete_me)
        {
          /* Save the current message */

          uid=MsgMsgnToUid(sq, hwm);


          /* Now kill it */

          (void)MsgKillMsg(sq, hwm);


          /* Reset the last message pointer */

          hwm=MsgUidToMsgn(sq, uid, UID_PREV);


          /* And grab the new high-message mark */

          hmsg=MsgHighMsg(sq);
        }
        else if (!DupesArea(ar) && !BadmsgsArea(ar))
          dwNewHWM=(dword)hwm;
      }

      if ((hwm % 5)==0 && (config.flag2 & FLAG2_QUIET)==0)
        (void)printf("\b\b\b\b\b%05lu", hwm);

      nmsg_scanned++;
    }
  }


  /* Adjust the high-water marker, if necessary */

  if (dwNewHWM != (dword)-1)
    MsgSetHighWater(sq, dwNewHWM);

  if ((config.flag2 & FLAG2_QUIET)==0)
    (void)printf("\b\b\b\b\b     \n");


  /* If we weren't provided an open handle to begin with... */
  
  if (! opensq)
    (void)MsgCloseArea(sq);
  
  if (config.max_msgs && scan_ctr >= config.max_msgs)
  {
    FILE *fp;

    if ((fp=shfopen(maxmsgs_scan, "w", O_WRONLY | O_CREAT | O_TRUNC)) != NULL)
    {
      fputs(ar->name, fp);
      fclose(fp);
    }

    erl_max=TRUE;
  }
}


int NeedToEcho(byte *mbuf)
{
  byte *p=mbuf;


  /* Skip all kludge lines: */

  while (*p=='\x01')    /* If the first is a ^A... */
  {
    while (*p && *p != '\x0d')    /* Then skip until we hit a CR */
      p++;

    /* Then skip while we still have CRs */
    while (*p && (*p=='\x0d' || *p=='\x0a' || *p==(byte)0x8d))
      p++;

    /* ...and we're finally pointing to the next line. */
  }

  if (p[0]=='N' && p[1]=='O' && p[2]=='E' && 
      p[3]=='C' && p[4]=='H' && p[5]=='O')
    return FALSE;
  else return TRUE;
}


struct _sblist *Digitize_Seenbys(struct _cfgarea *ar, byte *text, unsigned *sb_num)
{
  static struct _sblist sb, last;
  struct _sblist *fsb, *nextfsb, *nsb;

  unsigned num_sb, len;

  byte *endofline;
  byte *s, *ap, *csb;
  byte *next_sb;

  NW(ar);

  /* Set up the variables to default to OUR zone/net/node. */
  
  last=config.def;
  last.zone=last.point=0;

  begin_sb=end_sb=NULL;

  /* Find the first seen-by in the message */
  
  next_sb=GetNextToken(msgbuf, text, seen_by_str);
  

  nsb=&sb;
  num_sb=0;
  begin_sb=next_sb;

  
  /* As long as we still have a seen-by to process */
  
  while (next_sb)
  {
    nsb=&sb;
    num_sb=0;
    begin_sb=next_sb;

    /* Loop as long as there are a bunch of immediately-adjacent seenbys */

    while (eqstrn(next_sb, seen_by_str, SEEN_BY_LEN))
    {
      /* If we found a '\x0d'... */
      
      if ((endofline=strchr(next_sb, '\x0d')) != NULL)
      {
        /* Copy everything up to and including the '\x0d'. */

        len=(unsigned)(endofline-next_sb)+1;

        csb=smalloc(len+1);

        (void)strncpy(csb, next_sb, len);
        csb[len]='\0';
      }
      /* Else just copy the whole thing */
      else csb=sstrdup(next_sb);

      s=csb;
      s += 9;

      /* Blank out the current node */

      (void)memset(nsb, '\0', sizeof(struct _sblist));

      /* While we can still get node #'s out of this, do so. */

      for (ap=strtok(s, msgdelim); ap; ap=strtok(NULL, msgdelim))
      {
        (void)memset(nsb, '\0', sizeof(struct _sblist));

        /* Fill in defaults */
        
        *nsb=last;

        Parse_NetNode(ap, NULL, &nsb->net, &nsb->node, NULL);

        last=*nsb;

        /* If we got one, add to the linked list for our next node number */
        
        if (nsb->net)
        {
          nsb->next=(struct _sblist *)malloc(sizeof(struct _sblist));

          nsb=nsb->next;
          nsb->next=NULL;

          num_sb++;
        }
        else
        {
          /* needed because '*nsb=last' overwrites the ->next pointer */
          nsb->next=NULL;
        }
      }

      free(csb);
      

      if (!endofline) /* If no new line, take end of msg instead */
        endofline=next_sb+strlen(next_sb);

      /* Junp over any linefeeds and CR's */
      
      while (*endofline=='\x0d' || *endofline=='\x0a' ||
             *endofline==(byte)0x8d)
      {
        endofline++;
      }
     
      /* If all is well, this should position us at the next seen-by */

      next_sb=endofline;
    }
    
    end_sb=next_sb;

    /* We found the end of the last contiguous seen-by.  Now do a           *
     * sanity check on the rest of the data, to ensure that a message       *
     * doesn't follow.  If we find anything other than a non-kludge,        *
     * non-blank line, then we discard everything and start over.           */

    while (next_sb && *next_sb)
    {
      if (*next_sb < ' ')
      {
        /* Skip to the next '\x0d' in the message body */

        while (*next_sb != '\x0d' && *next_sb != '\0')
          next_sb++;

        if (next_sb)
          while (*next_sb=='\x0d' || *next_sb=='\x0a' ||
                 *next_sb==(byte)0x8d)
            next_sb++;
      }
      else if (*next_sb=='\x0d' || *next_sb=='\x0a' ||
               *next_sb==(byte)0x8d || *next_sb==' ')
      {
        while (*next_sb=='\x0d' || *next_sb=='\x0a' ||
               *next_sb==(byte)0x8d || *next_sb==' ')
          next_sb++;
      }
      else
      {
        /* We found some message, so get out! */
        break;
      }
    }
    
    /* If next_sb==NULL or it's equal to a NUL, we're at the end of the     *
     * message, so everything is kosher (and I'm not even Jewish! :-) )     */
       
    if (next_sb && *next_sb=='\0')
      break;
    else
    {
      char *p;

      /* Otherwise, there's some MESSAGE after it, so find the next seen-by *
       * to start at.                                                       */

      p=GetNextToken(msgbuf, next_sb, seen_by_str);

#if 0
      /* There's nuttin' after this, so let's go with what we have */

      if (p==NULL)
        break;
#endif

      /* Found some!  Set the new pointers to the beginning */

      begin_sb=next_sb=p;

      /* Free all of the old seen-by's */
      
      for (fsb=sb.next; fsb; fsb=nextfsb)
      {
        nextfsb=fsb->next;
        free(fsb);
      }
      
      (void)memset(&sb, '\0', sizeof(struct _sblist));

      nsb=&sb;
      num_sb=0;
      begin_sb=next_sb;
    }
  }


  /* Free the last one we allocated, since it isn't used */

  if (nsb && nsb != &sb)
    free(nsb);

  *sb_num=num_sb;
  return (num_sb ? &sb : NULL);  /* Didn't find anything */
}





#if 0 /* not used */
static int _stdc sbl_comp(const void *s1, const void *s2)
{
  int x;

  if ((x=((int)(struct _sblist *)s1)->zone - (int)((struct _sblist *)s2)->zone) != 0)
    return x;
  else if ((x=(int)((struct _sblist *)s1)->net - (int)((struct _sblist *)s2)->net) != 0)
    return x;
  else if ((x=(int)((struct _sblist *)s1)->node - (int)((struct _sblist *)s2)->node) != 0)
    return x;
  else return ((int)((struct _sblist *)s1)->point - (int)((struct _sblist *)s2)->point);
}
#endif

static int _stdc sbl_comp_nz(const void *s1, const void *s2)
{
  int x;

  if ((x=(int)((struct _sblist *)s1)->net - (int)((struct _sblist *)s2)->net) != 0)
    return x;
  else return ((int)((struct _sblist *)s1)->node - (int)((struct _sblist *)s2)->node);
/*
  else return ((int)((struct _sblist *)s1)->point - (int)((struct _sblist *)s2)->point);*/
}



struct _sblist *Sort_And_Flatten(struct _sblist *sb, unsigned num_sb, unsigned additional)
{
  struct _sblist *sbf;      /* Pointer to the flat list */
  struct _sblist *sbold;    /* Used to hold temp copy of last sb pointer */
  struct _sblist *sborig;   /* Value of FIRST link in the 'sb' chain */
  unsigned outoforder, i;

  sborig=sb;

  /* Allocate the memory required for the flat array */

  sbf=smalloc((num_sb+additional)*sizeof(struct _sblist));

  /* Now traverse the linked list, copy to flat array, and free() while   *
   * doing so.  This reverses the order of the seen-by list, which is     *
   * just as well, since qsort() gets abysmally slow when working on an   *
   * almost-sorted list anyhow.                                           */
 
  outoforder=FALSE;

  for (i=0; sb && i < num_sb; i++)
  {
    sbf[i]=*sb;
    sbf[i].next=NULL;
    sbold=sb;
    sb=sb->next;
    
    /* Check to see if we need to sort this element */

    if (i && sbl_comp_nz(sbf+i-1, sbf+i) > 0)
      outoforder=TRUE;

    if (sbold != sborig)
      free(sbold);
  }

  /* If necessary, sort the list */
  
  if (outoforder)
    qsort(sbf, num_sb, sizeof(struct _sblist), sbl_comp_nz);

  return sbf;
}

/* Update the specified message number */

static void near PerformMessageUpdate(HAREA sq, struct _cfgarea *ar,
                                      XMSG *msg, char *ctrl, dword msgn,
                                      int do_modify, dword *hwm, unsigned added,
                                      dword *pNewHash, dword *pNewSerial)
{
  DUPEID did;
  HMSG hmsg;
  unsigned len;
  char *txt;

  if (!do_modify)
  {
    /* If we're just killing the message, this is a simple job */

    UMSGID uid_hwm;

    S_LogMsg("*  Remote delete: %s:%ld", ar->name, msgn);

    /* Save the current scan pointer */

    if (hwm)
      uid_hwm=MsgMsgnToUid(sq, *hwm);

    if (MsgKillMsg(sq, msgn) != 0)
      S_LogMsg("!Error deleting %s:%ld", ar->name, msgn);

    /* Restore the current scan pointer */

    if (hwm)
      *hwm=MsgUidToMsgn(sq, uid_hwm, UID_PREV);

    return;
  }

  S_LogMsg("*  Remote modify: %s:%ld", ar->name, msgn);

  /* Else we have to modify the message with this new text */

  if ((hmsg=MsgOpenMsg(sq, MOPEN_CREATE, msgn))==NULL)
  {
    S_LogMsg("!Error opening %s:%ld for update!", ar->name, msgn);
    return;
  }

  msg->attr |= MSGSCANNED;

  len=strlen(txt=msgbuf+added)+1;

  MsgWriteMsg(hmsg, FALSE, msg, txt, len, len, strlen(ctrl)+1, ctrl);
  MsgCloseMsg(hmsg);

  GetDidMsgid(&did, ctrl);

  if (did.msgid_hash || did.msgid_serial)
  {
    *pNewHash=did.msgid_hash;
    *pNewSerial=did.msgid_serial;
  }

  return;
}


/* Check to see if we need to modify an existing message */

int ProcessACUpdate(HAREA sq, struct _cfgarea *ar, XMSG *msg, char *ctrl,
                    int *do_kill, UMSGID uid, dword *hwm, unsigned added,
                    int *pfSkipScan)
{
  char *ctrl_dup;
  struct _sblist *ps;
  dword msgid_hash, msgid_serial;
  static char acupdate[]="ACUPDATE";
  dword *pNewHash, *pNewSerial;
  dword msgn;
  char *kludge;
  int do_modify;

  *do_kill=FALSE;

  /* 01234567890123456789012345678901234 */
  /* ACUPDATE: MODIFY 1:249/106 2ce03312 */
            /* or */
  /* ACUPDATE: DELETE 1:249/106 2ce01623 */

  /* Return "ok" if no ACUPDATE kludge */

  if ((kludge=MsgGetCtrlToken(ctrl, acupdate))==NULL)
    return TRUE;

  /* Check to see if this is from a secure link */

  for (ps=ar->update_ok; ps; ps=ps->next)
    if (AddrMatchNS(&msg->orig, ps))
      break;

  if (!ps)
  {
    static int logged_acupdate=FALSE;

    if (!logged_acupdate)
    {
      logged_acupdate=TRUE;
      S_LogMsg("!Insecure ACUPDATE message from %s!", Address(&msg->orig));
    }

    *pfSkipScan=TRUE;
    *do_kill=TRUE;
    MsgFreeCtrlToken(kludge);
    return TRUE;
  }

  /* Don't do anything for passthru areas */

  if (ar->flag & AFLAG_PASSTHRU)
  {
    MsgFreeCtrlToken(kludge);
    return TRUE;
  }

  /* Can't process ACUPDATE messages in a *.MSG-style area */

  if (ar->type & MSGTYPE_SDM)
    return TRUE;


  ctrl_dup=sstrdup(ctrl);

  /* Remove this ACUPDATE line from the control information before rewriting */

  RemoveFromCtrl(ctrl_dup, acupdate);

  if (strncmp(kludge+10, "MODIFY", 6)==0)
    do_modify=TRUE;
  else if (strncmp(kludge+10, "DELETE", 6)==0)
    do_modify=FALSE;
  else
  {
    S_LogMsg("!Invalid ^aACUPDATE line in area %s (uid %lu)",
             ar->name, uid);
    return TRUE;
  }

  MashMsgid(kludge+17, &msgid_hash, &msgid_serial);

  /* Get the message number to modify, based on the MSGID */

  if (msgid_hash &&
      (msgn=FindUpdateMessage(sq, ar, msgid_hash, msgid_serial,
                              &pNewHash, &pNewSerial)) != 0L)
  {
    /* Update the message */

    PerformMessageUpdate(sq, ar, msg, ctrl_dup, msgn, do_modify, hwm, added,
                         pNewHash, pNewSerial);
  }

  free(ctrl_dup);

  /* Remove this update request */

  *do_kill=TRUE;

  MsgFreeCtrlToken(kludge);
  return TRUE;
}



/* Scan a message to all of the nodes on our scan list for this area */

unsigned Scan_To_All(struct _cfgarea *ar, struct _sblist *sb,
                     unsigned num_sb, XMSG *msg, byte *ctxt,
                     byte **afterkludge, UMSGID uid, int *do_kill,
                     HAREA sq, dword *hwm)
{
  struct _groute *zg;
  struct _sblist *sbnode, *ats;
  struct _statlist *sl;
  struct _sblist rescan;
  
  unsigned ctlen, scanmask, oldsmask;
  unsigned needscan=0;
  unsigned nst;

  char *ct, *scanto;
  int fSkipScan=FALSE;
  
  extern char t_area[];


  /* See if this message was supposed to modify an existing message, and    *
   * check our security to make sure that it is okay to scan out to         *
   * others.                                                                */

  if (!ProcessACUpdate(sq, ar, msg, ctxt, do_kill, uid, hwm,
                       needscan, &fSkipScan))
    return 0;


  scanto=smalloc(ar->num_scan);

  /* Now determine who we need to scan to */

  for (sbnode=ar->scan, nst=0; sbnode; sbnode=sbnode->next, nst++)
    if (Need_To_Scan(ar, sbnode, sb, num_sb, msg) && !resc)
    {
      needscan=1;
      scanto[nst]=TRUE;
    }
    else scanto[nst]=FALSE;


  if (!fSkipScan &&
      (needscan || resc))
  {
    erl_sent=TRUE;

    /* Add this message to our duplicate data buffer, but only if we're     *
     * NOT doing a one-pass toss/scan.  (If we're doing the onepass,        *
     * we'll have already checked it when doing the toss.)                  */
    
    if ((config.flag & FLAG_ONEPASS)==0)
      (void)IsADupe(ar, msg, ctxt, uid);
    
    /* Now insert all nodes we're scanning to into the binary SEEN-BYs */

    for (sbnode=ar->scan; sbnode; sbnode=sbnode->next)
      Insert_Sb_Node(sbnode, sb, &num_sb);

    
    /* Make sure that we're here, too */

    Insert_Sb_Node(&ar->primary, sb, &num_sb);


    
    /* If we're rescanning an area, insert their address too */
    
    if (resc)
      Insert_Sb_Node(NetaddrToSblist(resc, &rescan), sb, &num_sb);
    

    /* Now all any "Add_To_Seen" nodes */

    for (ats=config.ats; ats; ats=ats->next)
      Insert_Sb_Node(ats, sb, &num_sb);
    
    for (ats=ar->add; ats; ats=ats->next)
      Insert_Sb_Node(ats, sb, &num_sb);

    /* Add the 'AREA:xxx' to the front of the message */

    AddAreaTag(ctxt, ar);
    
    /* Convert control info to the <bleuch ick retch> FTSC kludge lines */
    
    ct=CvtCtrlToKludge(ctxt);

    if (ct)
    {
      ctlen=strlen(ct);

      /* Return value is 1+the number of bytes we added to begin of msgbuf */
      needscan=1+ctlen;
      
      (void)strocpy(msgbuf+ctlen, msgbuf);
      (void)memmove(msgbuf, ct, ctlen);

      MsgFreeCtrlBuf(ct);

      if (begin_sb)
        begin_sb += ctlen;

      if (end_sb)
        end_sb += ctlen;

      if (afterkludge)
        *afterkludge=msgbuf+ctlen;
    }


    Add_Tear_Line(msgbuf, ar, msg);

    
    /* Put the binary SEEN-BYs back into ASCII form, in the message itself */

    Fix_SeenBys(sb, num_sb, msgbuf, oldsmask=0, ar);

    
    /* Add our node to the PATH line(s) */

    Add_Us_To_Path(ar, msgbuf);


    /* Make sure that the message is "from" us */
    
    msg->orig.zone = ar->primary.zone;
    msg->orig.net  = ar->primary.net;
    msg->orig.node = ar->primary.node;
    msg->orig.point= ar->primary.point;


    /* Mask off offending bits in the message header */

    msg->attr &= (long)~(MSGSENT | MSGFWD | MSGORPHAN | MSGLOCAL | MSGXX2 |
                         MSGREAD);

    scanmask=0;

#ifdef DJ
    (void)fprintf(dj, "F:%0.15s T:%0.15s S:%20.20s D:%04d%02d%02d%02d%02d%02d\nS:",
                  msg->from, msg->to, msg->subj,
                  msg->date_written.date.yr+1980,
                  msg->date_written.date.mo,
                  msg->date_written.date.da,
                  msg->date_written.time.hh,
                  msg->date_written.time.mm,
                  msg->date_written.time.ss << 1);
#endif

    /* Now scan it to everyone! */

    for (sbnode=ar->scan, sl=ar->statlist, nst=0;
         sbnode;
         sbnode=sbnode->next, nst++)
    {
      if (scanto[nst])
      {
#ifdef DJ
        (void)fprintf(dj, " %d:%d/%d.%d", sbnode->zone, sbnode->net, sbnode->node,
                      sbnode->point);
#endif

        scanmask=0;
        
        if (config.pointnet && sbnode->net != config.pointnet)
          scanmask |= SCAN_NOPOINT;

        if (GetsTinySeenbys(sbnode))
          scanmask |= SCAN_TINY;

        if (scanmask != oldsmask)
          Fix_SeenBys(sb, num_sb, msgbuf, oldsmask=scanmask, ar);
        
        /* Handle ZoneGate nodes */

        for (zg=config.zgat; zg; zg=zg->next)
          if (AddrMatchS(&zg->host, sbnode))
          {
            struct _sblist *zsb, *zp, *zgl;

            /* Copy the linked list of nodes into a flat and sorted         *
             * array, as required by the fix_seenbys function               */
               
            zsb=smalloc(sizeof(struct _sblist) * zg->n_nodes);
            
            for (zp=zsb, zgl=zg->nodes; zgl; zgl=zgl->next, zp++)
            {
              *zp=*zgl;
              zp->next=NULL;
            }
            
            /* Sort it */
            
            qsort(zsb, zg->n_nodes, sizeof(struct _sblist), sbl_comp_nz);
            
            /* And now add to the SEEN-BYs */

            Fix_SeenBys(zsb, zg->n_nodes, msgbuf, 0, ar);

            free(zsb);

            scanmask=0;
            oldsmask=(unsigned)-1;
            break;
          }
        
        Scan_To(sbnode, msg, msgbuf, 0L, ar, sl);
        ar->sent++;
      }

      if (sl)
        sl=sl->next;
    }
    
#ifdef DJ
    (void)fprintf(dj, "\n");
#endif

    /* Do the rescan here! */

    if (resc)
    {
      Scan_To(&rescan, msg, msgbuf, 0L, ar, sl);
      ar->sent++;
    }


    /* Make sure that all seen-bys are placed here, for when the message    *
     * gets written to disk locally!                                        */

    if (scanmask != 0 || oldsmask==(unsigned)-1)
      Fix_SeenBys(sb, num_sb, msgbuf, oldsmask=0, ar);
    
    /* Remove the AREA: tag, since we may have reinserted it */

    RemoveFromCtrl(ctxt, t_area);
  }

  free(scanto);

  return needscan;
}




static void near Insert_Sb_Node(struct _sblist *look, struct _sblist *sb, unsigned *num_sb)
{
  struct _sblist *s, *end;
  struct _sblist find;
  
  find=*look;
  find.next=NULL;
  find.point=find.zone=0; /* Don't add zone/point info */
  
  if (bsearch(&find, sb, *num_sb, sizeof(struct _sblist), sbl_comp_nz)==NULL)
  {
    /* Find a node which is GREATER than the one we're searching for, then *
     * break out of the loop.                                              */

    for (s=sb, end=sb+*num_sb; s < end; s++)
      if (sbl_comp_nz(&find, s) < 0)
        break;

    /* If a greater node was found, then we're pointing to its entry.  If  *
     * not, then we're pointing to the end of the list.  Either way, we    *
     * should insert the new node at the current position.                 */

    /* Shift all nodes above this one up by one position */

    (void)memmove(s+1, s, (size_t)(end-s)*sizeof(struct _sblist));

    /* And insert this node... */

    *s=find;
    (*num_sb)++;
  }
}


static int near Need_To_Scan(struct _cfgarea *ar, struct _sblist *scanto, struct _sblist *sb, unsigned num_sb, XMSG *msg)
{
  NETADDR dest;
  struct _sblist sbl;
  struct _perlist *pl;


  
  /* Make sure that we don't send any messages to ourselves */

  if (DestIsHereA(SblistToNetaddr(scanto, &dest)))
    return FALSE;


  /* Now scan the area's "personal list" for this node.  If we find         *
   * an entry which indicates that node xyyzy only wants messages addressed *
   * to "Joe Sysop", don't scan unless the name compare matches.            */

  for (pl=ar->plist; pl; pl=pl->next)
    if (MatchSS(&pl->node, scanto, FALSE) && !eqstri(msg->to, pl->name))
      return FALSE;


  /* If we're scanning to a true point, always scan it (without checking    *
   * the seen bys), and send it to all of the points, unless the message    *
   * originated from that specified point.                                  */

  if (scanto->point)
  {
    /* Only scan the message to a point if we're doing a one-pass toss/scan,*
     * if the message was entered locally, or if the area is a Squish-style *
     * base (which has proper 4D zone/point info).                          */

    if ((config.flag & FLAG_ONEPASS) || (msg->attr & MSGLOCAL) ||
        (ar->type & MSGTYPE_SQUISH))
    {
      if (!MatchNS(&msg->orig, scanto, TRUE))
        return TRUE;
    }
    else return FALSE;
  }


  /* Don't scan the message to the same place it came from, assuming        *
   * that it's not a local message.                                         */

  if (msg->orig.net==scanto->net && msg->orig.node==scanto->node &&
      msg->orig.point==scanto->point && (msg->attr & MSGLOCAL)==0)
  {
    return FALSE;
  }


  /* Set the 'next' field to NULL, since it's checked by the bsearch f() */

  sbl=*scanto;
  sbl.next=NULL;

  /* And if it isn't found in the table, then return TRUE. */

  return (bsearch(&sbl, sb, num_sb,
                  sizeof(struct _sblist), sbl_comp_nz)==NULL);
}




static void near AddAreaTag(char *txt, struct _cfgarea *ar)
{
  char temp[PATHLEN];
  unsigned len;

  (void)sprintf(temp, "\x01""AREA:%s", ar->name);
  len=strlen(temp);
  
  (void)strocpy(txt+len, txt);
  (void)memmove(txt, temp, len);
}

    
 

void Scan_To(struct _sblist *scanto, XMSG *msg, char *text, dword attr, struct _cfgarea *ar, struct _statlist *sl)
{
  struct _pktprefix pp;
  dword len;
  int flavour;

  /* Size of msg.from+msg.to+msg.subj+msg.date+breathing_room */
  
  char lump[36+36+72+20+10]; 
  char *p;

  p=lump;
  
  if (*msg->__ftsc_date)
    (void)strcpy(p, msg->__ftsc_date);
  else (void)sprintf(p, "%02d %s %02d  %02d:%02d:%02d",
                     msg->date_written.date.da ? msg->date_written.date.da : 1,
                     months_ab[msg->date_written.date.mo
                                 ? msg->date_written.date.mo-1
                                 : 0],
                     (msg->date_written.date.yr+80) % 100,
                     msg->date_written.time.hh,
                     msg->date_written.time.mm,
                     msg->date_written.time.ss << 1);

  p += strlen(p)+1;

  (void)strcpy(p, msg->to);
  p += strlen(p)+1;

  (void)strcpy(p, msg->from);
  p += strlen(p)+1;

  (void)strcpy(p, msg->subj);
  p += strlen(p)+1;

  pp.ver=PKTVER;

  /* Readdress point mail */

  /*
  if (scanto->point && config.pointnet &&
      DestIsHereA(config, SblistToNetaddr(scanto, &to)))
  {
    pp.orig_net=msg->orig.net;
    pp.orig_node=msg->orig.node;
    pp.dest_net=config.pointnet;
    pp.dest_node=scanto->point;
  }
  else
  */
  {
    pp.orig_net=(sword)msg->orig.net;
    pp.orig_node=(sword)msg->orig.node;
    pp.dest_net=(sword)scanto->net;
    pp.dest_node=(sword)scanto->node;
  }

  pp.attr=(word)msg->attr;
  pp.cost=0;

  flavour=MsgAttrToFlavour(attr);

  len=(dword)strlen(text)+1L;
        
  if ((config.flag & FLAG_STATS) && sl)
  {
    sl->out_msgs++;
    sl->out_bytes += (dword)(size_t)(p-lump)+len;
    ar->flag |= AFLAG_STATDATA;
  }

  Add_Outbuf(scanto, &pp, lump, (dword)(size_t)(p-lump), text, (dword)len, flavour, ar);
}


#ifdef NEVER

/* Copy the "AREA:xxx" stuff to just BEFORE the start of 'msgbuf'.  The     *
 * msgbuf[] array is actually just a pointer about 50 bytes into            *
 * buffer[]...  This was done so that we could insert the 'AREA:xxx'        *
 * tag by copying it BEFORE the message text, so we don't have to           *
 * shift the entire message body when adding the tag, and take care         *
 * of everything in a single write operation.                               */
     
static int near Twiddle_Area_Tag(char *name, char *msgbuf)
{
  char temp[PATHLEN];
  int len;

  (void)sprintf(temp,"AREA:%s\r",name);
  len=strlen(temp);
  
  memmove(msgbuf-len, temp, len);
  return len;
}

#endif








/* Write out a new set of SEEN-BYs, and remove the old ones (if any) */

static void near Fix_SeenBys(struct _sblist *sb, unsigned num_sb, char *mbuf, unsigned smask, struct _cfgarea *ar)
{
  NETADDR here;
  byte *s, *p;

  byte *seenby;
  byte *tearpos;
  byte *endtear;

  unsigned len, cl, i;
  unsigned new_len;
  unsigned old_len;
  word last_net;


  /* Allocate a buffer big enough to hold new seen-bys */

  seenby=smalloc((num_sb+20)*MAX_ADDRLEN);

  /* Now create a hunk of text holding the SEEN-BYs to place in the text.   *
   * Process the array into the hunk, and creating a new seen-by line       *
   * every 78 columns, as necessary.                                        */
     
  for (i=0, len=0, last_net=(word)-1, p=seenby; i < num_sb; i++)
  {
    /* Add the first SEEN-BY statement for the beginning msg */
    
    if (i==0)
    {
      (void)strcpy(p,"SEEN-BY:");
      p += 8;
      len=8;
    }

    /* Strip point info, if necessary.  If this is a listing for            *
     * a pointnet, and point info wasn't requested, and we're not the       *
     * fakenet point itself), then don't add this to the sb's.              */
    
    if (sb[i].net==config.pointnet && (smask & SCAN_NOPOINT) &&
        ar->primary.net != config.pointnet)
      continue;

    /* If this seenby isn't for a system we connect with, and a tiny        *
     * seenby list was requested, then don't add this to the buffer.        *
     *                                                                      *
     * However, also check our "add to seen" and "-+" lists and leave those *
     * enabled.                                                             */

    if ((smask & SCAN_TINY) && !NodeInSlist(sb+i, ar->scan) &&
        !DestIsHereA(SblistToNetaddr(sb+i, &here)) &&
        !NodeInSlist(sb+i, ar->add) &&
        !NodeInSlist(sb+i, config.ats))
    {
      continue;
    }
    
    if (sb[i].net==last_net)
      (void)sprintf(p," %hu",sb[i].node);
    else
    {
      (void)sprintf(p," %hu/%hu", sb[i].net, sb[i].node);
      last_net=sb[i].net;
    }

    cl=strlen(p);

    if (len+cl >= 78)   /* If this line would cause a screen wrap... */
    {
      (void)sprintf(p, "\rSEEN-BY: %hu/%hu", sb[i].net, sb[i].node);

      len=strlen(p);
      p += len;
    }
    else
    {
      len += cl;  /* Increment length counter */
      p += cl;    /* And increment current string pointer */
    }
  }

  *p++='\x0d';        /* And cap the string */
  *p='\0';


  new_len=strlen(seenby);
  endtear=NULL;
  tearpos=GetTearPos(mbuf, &endtear);
  
  if (GetNextToken(mbuf, tearpos, seen_by_str)==NULL)
    tearpos=mbuf;
  

  /* Find the beginning of the seenbys */

  p=s=begin_sb;

  /* If the message already contains seen-bys */

  if (p)
  {
    do
    {
      /* Skip over any text to the first '\x0d' */

      s=strchr(s,'\x0d');

      if (!s)
        s=p+strlen(p);

      while (*s=='\x0d' || *s=='\x0a' || *s==(byte)0x8d)  /* Skip over line delims  */
        s++;

    } while (eqstrn(s,seen_by_str,8));  /* And keep going 'till we find     */
                                        /* the end of the SEEN-BYs.         */

    /* At this point, everything between 's' and 'p' is the old SEEN-BY     *
     * stuff.  Therefore, we copy everything that follows into a buffer,    *
     * stick in our own SEEN-BYs, and add the copied stuff when done.       */

    old_len=(unsigned)(s-p);


    /* Make room for the new stuff, if any */

    (void)memmove(s+(new_len-old_len), s, strlen(s)+1);
    

    /* ...and actually copy the new stuff in. */

    (void)memmove(p, seenby, new_len);


    /* Now update the seenby pointers */

    begin_sb=p;
    end_sb=p+new_len;
  }
  else   /* Tack on a new set of SEEN-BYs */
  {
    if (!endtear || (endtear[-1] != '\x0d' && endtear[-1] != '\x0a'))
    {
      unsigned mlen;

      mlen=strlen(mbuf);

      endtear=mbuf+mlen;


      /* Make sure that it ends with a CR */

      if (mbuf[mlen-1] != '\x0d')
      {
        (void)strcat(mbuf, "\x0d");
        endtear++;
      }
    }


    /* Move the old stuff (if any) out of the way... */

    (void)memmove(endtear+new_len, endtear, strlen(endtear)+1);
    

    /* And drop in the new. */

    (void)memmove(endtear,seenby,new_len);


    begin_sb=endtear;
    end_sb=endtear+new_len;
  }

  free(seenby);
}


/* Add our node to the ^aPATH line */

static void near Add_Us_To_Path(struct _cfgarea *ar, char *mbuf)
{
  unsigned len, i;
  word net=(word)-1;

  byte us[PATHLEN];
  byte pathline[PATHLEN];
  byte temp[PATHLEN];

  byte *s, *p, *s2, *pp, *end;


  /* Don't add anything to the path line if we're a point! */

  if (ar->primary.point)
    return;


  /* Convert our net/node address to string form */
  
  (void)sprintf(us, "%hu/%hu", ar->primary.net, ar->primary.node);


  /* Find the LAST path line... */

  p=NULL;
  s=mbuf;

  while ((s=GetNextToken(mbuf,s,"\x01PATH: ")) != NULL)
  {
    /* Inc 's' to make sure we don't get same path token for next           *
     * GetNextToken() call...                                               */
    
    p=s++;
  }

  /* If there WAS a path line */
  
  if (p)
  {
    /* Find the next hard return */

    for (end=p; *end != '\x0d' && *end; )
      end++;


    /* And the length of this path is... */

    len=(unsigned)(end-p+1);


    /* Make sure that we don't have a buffer overflow */

    if (len >= PATHLEN)
      len=PATHLEN-2;


    /* Copy it to a safe spot, and cap with a NUL */

    (void)strncpy(pathline, p, len);
    pathline[len]='\0';


    /* Strip off all trailing \r's and spaces. */

    i=len;

    while (i && (pathline[i-1]=='\x0d' || pathline[i-1]==' '))
      pathline[--i]='\0';



    /* Find the last slash in the path string (if any), to extract the     *
     * net number from.                                                    */

    s2=strrchr(pathline,'/');

    if (s2) /* Found it */
    {
      while (isdigit(*--s2)) /* Position pointer to the first digit */
        ;

      s2++;

      net=(word)atoi(s2); /* And place in variable */
    }

    /* If we can use the shortened form */
    
    if (s2 && net==ar->primary.net)
      (void)sprintf(temp, " %hu", ar->primary.node);
    else (void)sprintf(temp, " %s", us);



    /* Find the LAST space char on the pathline */

    pp=strrchr(pathline, ' ');



    /* And now check to make sure that our address isn't already there.     *
     * If it is, then just return without doing anything...                 */

    if (pp && (eqstri(pp,temp) || eqstri(pp+1,us)))
      return;

    if (i+strlen(temp) < 78)   /* If it'll fit, then add it... */
      (void)strcat(pathline,temp);
    else
    {
      /* Else tack it onto a NEW path string */
      (void)sprintf(temp,"\r\x01PATH: %s\r",us);
      (void)strcat(pathline,temp);
    }

    /* Out with the old... */

    (void)memmove(end+(strlen(pathline)-len)+1, end, strlen(p)+1);

    /* ...and in with the new. */

    (void)memmove(p, pathline, strlen(pathline));
  }
  else /* None found, so add to end of message buffer */
  {
    strlen_msgbuf=strlen(mbuf);

    s=mbuf;
    p=NULL;

    while ((s=GetNextToken(mbuf, s, seen_by_str)) != NULL)
      p=s++;  /* inc 's' to make sure we don't get same sb for next call */

    if (!p)
      p=mbuf+strlen_msgbuf-1;
    else
    {
      p=strchr(p, '\x0d');

      if (!p)
        p=mbuf+strlen_msgbuf-1;

      /* If there was no '\x0d' at the end of the stoopid thing. */

      if (*p=='\0') 
      {
        p[0]='\x0d';
        p[1]='\0';
        strlen_msgbuf++;
        p++;
      }
      else  /* Else skip over soft cr's and lf's. */
      {
        while (*p=='\x0a' || *p==(byte)0x8d || *p=='\x0d')
          p++;
      }
    }

    /* Now shift the path line in place */

    (void)sprintf(temp, "\x01PATH: %s\r", us);
    (void)strocpy(p+strlen(temp), p);
    (void)memmove(p, temp, strlen(temp));
  }
}



/* Add a blank tear line to message */

static void near Add_Tear_Line(char *mbuf, struct _cfgarea *ar, XMSG *msg)
{
  FILE *corig;
  NETADDR n;
  
  byte temp[PATHLEN];
  byte area_origin[PATHLEN];
  byte *addrstr;


  /* If the message contains a tear/origin line, or message didn't          *
   * originate here, then return.                                           */

  if (GetTearPos(mbuf, NULL) || (msg->attr & MSGLOCAL)==0 || begin_sb)
    return;
  
  (void)sprintf(temp,
                (ar->type & MSGTYPE_SQUISH) ? "%s.sqo" : "%s\\origin",
                ar->path);

  (void)strcpy(area_origin, config.origin);

  if ((corig=shfopen(temp, "r", O_RDONLY)) != NULL)
  {
    if (fgets(area_origin, PATHLEN, corig)==NULL)
    {
      /* If the gets didn't work, revert to the old origin */

      (void)strcpy(area_origin, config.origin);
    }

    (void)fclose(corig);
    
    /* Chop off newline */

    (void)Strip_Trailing(area_origin, '\n');
  }
  
  /* If we got this far, there must not have been a tear line, so we'll     *
   * just add one ourselves.                                                */

  /* If we're using a fakenet address, at least put in the right origiin */

  if (config.pointnet && ar->primary.net==config.pointnet &&
      config.addr->next)
  {
    addrstr=Address(SblistToNetaddr(config.addr->next, &n));
  }
  else addrstr=Address(SblistToNetaddr(&ar->primary, &n));

  (void)sprintf(temp,
#if defined(__FLAT__)
                "\r--- Squish/386 v%s\r * Origin: %s (%s)\r",
#else
                "\r--- Squish v%s\r * Origin: %s (%s)\r",
#endif
                version,
                area_origin,
                addrstr);

  (void)strcat(mbuf, temp);
}


static void near Add_Outbuf(struct _sblist *to,struct _pktprefix *pp, byte *lump, unsigned long lumplen, char *text, unsigned long textlen, int flavour, struct _cfgarea *ar)
{
/*  NETADDR na;*/

  BLIST *bl, *blnew;

  /* If this packet won't fit, then do a scan... */
  
  if ((char huge *)((char huge *)cur_ob+
           (unsigned)(sizeof(struct _pkthdr)+
                      sizeof(struct _pktprefix)+lumplen+textlen))
                                             >= (char huge *)(end_ob-10))
  {
    Flush_Outbuf();
  }

  /* Make a struct in first part of buffer */

  bl=(BLIST *)cur_ob;

  /* Fill in the destination information */
  bl->id=BL_ID;
  bl->zone=to->zone;
  bl->net=to->net;
  bl->node=to->node;
  bl->point=to->point;
  bl->done=FALSE;
  bl->flavour=flavour;
  bl->ar=ar;
  
  /*
  na.zone=to->zone;
  na.net=to->net;
  na.node=to->node;
  na.point=0;
  */
  
  /* If it's addressed to us (except for the point number)... */
  
#if 0 /* obsolete */
  if (DestIsHereA(config,&na))
  {
    /* If it's a msg to one of our points, and we're using a bink-style     *
     * outbound directory, we'll have to use the the fakenet.               */
    
    /*
    if (bl->point && config.pointnet && (config.flag & FLAG_FRODO)==0)
    {
      bl->net=config.pointnet;
      bl->node=bl->point;
      bl->point=0;
    }
    */
  }
#endif

  /* Bump pointer appropriately */

  cur_ob += sizeof(BLIST);

  /* Now copy in the packet prefix, header lump, and text itself */

  *(struct _pktprefix *)cur_ob=*pp;
  cur_ob += sizeof(struct _pktprefix);

  (void)memmove(cur_ob, lump, (unsigned)lumplen);
  cur_ob += (unsigned)lumplen;

  (void)memmove(cur_ob, text, (unsigned)textlen);
  cur_ob += (unsigned)textlen;

  bl->next=(BLIST *)cur_ob;
  bl->len=(long)((byte *)cur_ob-(byte *)bl)-sizeof(BLIST);

  /* Set the length of the next block to zero, so we know that this is      *
   * the end.                                                               */

  blnew=(BLIST *)cur_ob;
  blnew->len=0L;

  nmsg_sent++;
}



void Alloc_Outbuf(void)
{
  BLIST *bl;
  unsigned oblen=outbufmax;

  /* Don't allocate twice */
  if (outbuf)
    return;

  /* Get as much memory as possible... */

  while (oblen >= 16384 && (outbuf=malloc(oblen))==NULL)
    oblen -= 4096;

  if (oblen < 16384)
    NoMem();

  cur_ob=outbuf;

  end_ob=(byte far *)(outbuf+oblen);

  bl=(BLIST *)cur_ob;
  bl->len=0;
}

void Free_Outbuf(void)
{
  free(outbuf);
  outbuf=NULL;
}


/* Signal a disk-full error when writing a packet */

static void near ScanDiskFull(void)
{
  S_LogMsg("!Disk full error while writing outbound packets");
  cleanup_exit(ERL_ERROR);
}


void Flush_Outbuf(void)
{
/*  int pktfile;*/
  BFILE bf;

  struct _pkthdr pkthdr;

  char addr[MAX_ADDRLEN];
  char pktname[PATHLEN];

  BLIST *bl;
  BLIST *bn;

  unsigned i;

#ifdef DCORE
  printf("\n*** CORE begin=%ld\n", (long)coreleft());
#endif

  for (bl=(BLIST *)outbuf; bl->len; bl=bl->next)
  {
    if (bl->id != BL_ID)
    {
      S_LogMsg("!Internal error: garbled outbuf[].");
      cleanup_exit(ERL_ERROR);
    }

    if (! bl->done) /* If we haven't processed this one already */
    {
      bl->done=TRUE;

      (void)sprintf(addr, " (%u:%u/%u.%u)",
                    bl->zone, bl->net, bl->node, bl->point);

      if ((config.flag2 & FLAG2_QUIET)==0)
        (void)printf(addr);

#ifdef DIRECT_OB
      (void)sprintf(pktname, "%s%04hx%04hx.",
                    FixOutboundName(config, bl->zone),
                    (unsigned)bl->net,
                    (unsigned)bl->node);

      if (config.flag & FLAG_FRODO)
      {
        (void)sprintf(pktname+strlen(pktname), "%03x", bl->point);
      }
      else
      {
        (void)sprintf(pktname+strlen(pktname), "%cut",
                      bl->flavour ? bl->flavour : 'O');
      }

      BusyFileOpen(config, bl->zone, bl->net, bl->node, bl->point);
#else
      {
        struct _sblist from, to;
        
        to.zone=bl->zone; to.net=bl->net; to.node=bl->node;to.point=bl->point;
        from=bl->ar->primary;

        (void)strcpy(pktname, FixOutboundName(0xffff));
        GetFunkyPacketName(pktname+strlen(pktname), &from, &to, bl->flavour);
      }
#endif

#ifdef DCORE
      printf("\n*** CORE wbegin=%ld\n", (long)coreleft());
#endif

      if ((bf=Bopen(pktname, BO_RDWR | BO_BINARY, BSH_DENYWR, writebufmax)) != NULL)
      {
        /* If it opened okay, skip back over the last NUL */

        (void)Bseek(bf, -(long)sizeof(word), BSEEK_END);
      }
      else /* Otherwise, create a new one */
      {
#ifdef DCORE
        printf("\n--- CORE wmid=%ld\n", (long)coreleft());
#endif

        /* Add this packet to our virtual outbound area */

        HoleAddPacket(pktname, bl);

#ifdef DCORE
        printf("\n--- CORE wmix=%ld\n", (long)coreleft());
#endif

        if ((bf=Bopen(pktname, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY,
                      BSH_DENYWR, writebufmax))==NULL)
        {
          /* Try to create outbound area, if it doesn't already exist */

#ifdef DIRECT_OB
          (void)make_dir(FixOutboundName(config, bl->zone));
#else
          (void)make_dir(FixOutboundName(0xffff));
#endif

#ifdef DCORE
          printf("\n--- CORE wmi5=%ld\n", (long)coreleft());
#endif

          if ((bf=Bopen(pktname, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY,
                        BSH_DENYWR, writebufmax))==NULL)
          {
            S_LogMsg("!Can't create packet %s (%d), freemem=%ld",
                     pktname, errno, coreleft());

            if (errno==ENOMEM)
              S_LogMsg("!(Not enough memory for packet buffer.)");

            (void)printf("\nFatal error opening packet `%s'\n", pktname);

            /* Make sure that no 0-length packets are lying around */

            if (fexist(pktname))
              unlink(pktname);

            continue;
          }
        }

#ifdef DCORE
      printf("\n--- CORE wmuf=%ld\n", (long)coreleft());
#endif

        Fill_Out_Pkthdr(&pkthdr,
                        &bl->ar->primary,
                        bl->zone, bl->net, bl->node, bl->point);

        if (Bwrite(bf, &pkthdr, sizeof pkthdr) != sizeof pkthdr)
          ScanDiskFull();
      }

#ifdef DCORE
      printf("\n--- CORE wmit=%ld\n", (long)coreleft());
#endif

      /* Now write the message itself... */

      if (Bwrite(bf, ((byte *)bl)+sizeof(BLIST), (unsigned)bl->len) != bl->len)
        ScanDiskFull();

      /* Now scan for any other messages to this node */

      for (bn=bl->next; bn->len && bn->id==BL_ID; bn=bn->next)
      {
        if (!bn->done && bn->zone==bl->zone && bn->net==bl->net &&
            bn->node==bl->node && bn->point==bl->point &&
            bl->flavour==bn->flavour && 
            AddrMatchS(&bl->ar->primary, &bn->ar->primary))
        {
          /* Got one, so write it, and flag it so we don't do so again */
          
          if (Bwrite(bf, ((byte *)bn)+sizeof(BLIST), (unsigned)bn->len) != bn->len)
            ScanDiskFull();

          bn->done=TRUE;
        }
      }

      i=0;

      if ((i=Bwrite(bf, &i, sizeof(word))) != sizeof(word) ||
          Bclose(bf) != 0)
      {
        if (i != sizeof(word))
          Bclose(bf);

        ScanDiskFull();
      }

#ifdef DCORE
      printf("\n*** CORE wend=%ld\n", (long)coreleft());
#endif

#ifdef DIRECT_OB
      BusyFileClose(config, bl->zone, bl->net, bl->node, bl->point);
#endif

      if ((config.flag2 & FLAG2_QUIET)==0)
        for (i=strlen(addr); i; i--)  /* Back up over address */
          (void)printf("\b \b");
    }
  }


  /* Set pointers back to beginning again */

  cur_ob=outbuf;

  bl=(BLIST *)cur_ob;
  bl->len=0;

#ifdef DCORE
  printf("\n*** CORE end=%ld\n", (long)coreleft());
#endif
}



static int near NodeInSlist(struct _sblist *node, struct _sblist *check_list)
{
  struct _sblist *s;

  
  for (s=check_list; s; s=s->next)
    if (s->net==node->net && s->node==node->node)
      return TRUE;

  return FALSE;
}



/* Can a particular node handle tiny SEEN-BYs? */

static int near GetsTinySeenbys(struct _sblist *node)
{
  struct _sblist *sb;
  
  for (sb=config.tiny_except; sb; sb=sb->next)
    if (AddrMatchS(node, sb))
      return FALSE;

   for (sb=config.tiny; sb; sb=sb->next)
    if (AddrMatchS(node, sb))
      return TRUE;

  return FALSE;
}

 
