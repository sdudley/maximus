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
static char rcs_id[]="$Id: s_dupe.c,v 1.1 2002/10/01 17:56:22 sdudley Exp $";
#pragma on(unreferenced)

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include <ctype.h>
#include "dr.h"
#include "prog.h"
#include "squish.h"
#include "crc.h"
#include "s_dupe.h"

static int dfile=-1;
static DUPEHEAD dh={0};

/* Pointer to start of dupe file data */
static void far *dupebuf=NULL;

/* Last dupeID written to the dupe file */
static int have_last_dupe=FALSE;
static DUPEID lastdupe;

static char dupefile_sdm[]="%s\\dupefile.dat";
static char dupefile_sq[] ="%s.sqb";
static char msgid_str[]="MSGID";

/* Undo the last duplicate message written to buffer */

void UndoLastDupe(void)
{
  DUPEID far *dupelist;

  if (!have_last_dupe || !config.dupe_msgs)
    return;

  if (dh.high_dupe==dh.num_dupe)
    dh.num_dupe--;
    
  if (dh.high_dupe==0)
    dh.high_dupe=config.dupe_msgs-1;
  else
    dh.high_dupe--;
  
  dupelist=(DUPEID far *)((DUPEHEAD far *)dupebuf+1);

  if (config.dupe_msgs)
    dupelist[dh.high_dupe]=lastdupe;

  have_last_dupe=FALSE;
}

/* This function verfies that the MSGID in the message 'msgn' is actually   *
 * the same as the one indicated in the dupe file index.                    */

static int near VerifyMsgid(HAREA sq, dword msgn, dword msgid_hash, dword msgid_serial)
{
  dword ctlen;
  char *ctrl;
  HMSG hmsg;
  int rc=FALSE;

  if ((hmsg=MsgOpenMsg(sq, MOPEN_READ, msgn))==NULL)
    return FALSE;

  ctlen=MsgGetCtrlLen(hmsg);

  if ((ctrl=malloc((size_t)ctlen+5)) != NULL)
  {
    char *kludge;

    MsgReadMsg(hmsg, NULL, 0L, 0L, NULL, ctlen, ctrl);

    if ((kludge=MsgGetCtrlToken(ctrl, msgid_str)) != NULL)
    {
      dword msg_hash, msg_serial;

      MashMsgid(kludge+7, &msg_hash, &msg_serial);
      MsgFreeCtrlToken(kludge);

      rc=(msg_hash==msgid_hash && msg_serial==msgid_serial);
    }

    free(ctrl);
  }

  MsgCloseMsg(hmsg);
  return rc;
}


/* Search the dupe index file; we want to correlate a specific ^aMSGID      *
 * with a UMSGID.  To do this, simply perform a linear search on            *
 * the current dupe file.                                                   */

dword FindUpdateMessage(HAREA sq, struct _cfgarea *ar, dword msgid_hash, dword msgid_serial, dword **ppmsgid_hash, dword **ppmsgid_serial)
{
  DUPEID far *dptr, far *dend;
  DUPEID far *dupelist;

  if (config.has_dlist != ar || !config.dupe_msgs)
  {
    S_LogMsg("!Internal error - config.has_dlist != ar!");
    return 0L;
  }

  dupelist=(DUPEID far *)((DUPEHEAD far *)dupebuf+1);

  for (dptr=dupelist, dend=dptr+dh.num_dupe; dptr < dend; dptr++)
  {
    if (msgid_hash==dptr->msgid_hash &&
        msgid_serial==dptr->msgid_serial &&
        dptr->umsgid)
    {
      dword msgn=MsgUidToMsgn(sq, dptr->umsgid, UID_EXACT);

      if (VerifyMsgid(sq, msgn, msgid_hash, msgid_serial))
      {
        *ppmsgid_hash=&dptr->msgid_hash;
        *ppmsgid_serial=&dptr->msgid_serial;
        return msgn;
      }
    }
  }

  return 0L;
}



/* CRC the first two words of a field, not counting hibits, ctrl chars      *
 * or spaces.                                                               */

static dword near crc2word(char *str, dword crc)
{
  unsigned spaces;
  char *p;

  for (p=str, spaces=0; *p; p++)
  {
    if (*p==' ' && ++spaces==2)
      break;
    else if (((byte)*p & (byte)0x80u) || (byte)*p <= (byte)' ')
      break;
    else crc=xcrc32(tolower(*p), crc);
  }

  return crc;
}


/* Get name of duplicate message file */

static void near MakeDupeFileName(char *fname, struct _cfgarea *ar)
{
  (void)sprintf(fname,
                (ar->type & MSGTYPE_SDM) ? dupefile_sdm : dupefile_sq,
                ar->path);
}


/* Write the current list of dupe messages to disk, then close the file */

static void near WriteDupeList(void)
{
  char fname[PATHLEN];
  struct _cfgarea *pa;
  int size;

  /* Area which has the dupe list is? */

  pa=config.has_dlist;

  if (!pa || !dupebuf)
    return;

  lseek(dfile, 0L, SEEK_SET);

  /* Add the dupe file header */

  dh.sig=DUPEHEAD_SIG;
  *(DUPEHEAD *)dupebuf=dh;

  size=config.dupe_msgs * sizeof(DUPEID) + sizeof(DUPEHEAD);

  MakeDupeFileName(fname, pa);

  if (dfile==-1 &&
      (dfile=shopen(fname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY))==-1)
  {
    /* If a *.MSG directory does not exist, create it so that the
     * dupe file can be stored there.
     */

    if ((pa->type & MSGTYPE_SDM) && !direxist(pa->path))
      mkdir(pa->path);

    if ((dfile=shopen(fname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY))==-1)
      S_LogMsg("!Error creating dupe file for %s! (%d)", pa->path, errno);
  }

  if (dfile != -1)
  {
    if (farwrite(dfile, (char far *)dupebuf, size) != (int)size)
      S_LogMsg("!Error writing to dupe file for %s!", pa->path);

    (void)close(dfile);
  }

  dfile=-1;

  farfree(dupebuf);
  dupebuf=NULL;
  config.has_dlist=NULL;
  have_last_dupe=FALSE;
}


/* Read the dupe list for 'ar' into memory */

static int near ReadDupeList(struct _cfgarea *ar)
{
  char fname[PATHLEN];

  unsigned int size=(unsigned int)config.dupe_msgs*(unsigned int)sizeof(DUPEID)
                    + sizeof(DUPEHEAD);

  /* Re-alloc and fill, if necessary */

  if (dupebuf==NULL)
  {
    /* Allocate enough memory for max size of dupe list */

    if ((dupebuf=farcalloc(size, 1))==NULL)
      return FALSE;
  }

  /* Get the name of the dupe file */

  MakeDupeFileName(fname, ar);

  config.has_dlist=ar;

  if ((dfile=shopen(fname, O_RDWR | O_BINARY | O_NOINHERIT))==-1)
  {
    (void)memset(dupebuf, '\0', size);
    return TRUE;
  }

  if (farread(dfile, (char far *)dupebuf, size) != (int)size)
  {
    /* Only report error if it was a b/4 format dupe file */

    if (((DUPEHEAD far *)dupebuf)->sig==DUPEHEAD_SIG)
      S_LogMsg("!Error reading from dupe file %s", fname);

    (void)memset(dupebuf, '\0', size);
  }

  dh=*(DUPEHEAD far *)dupebuf;

  if (dh.sig != DUPEHEAD_SIG)
    (void)memset(dupebuf, 0, size);

  if (dh.num_dupe >= config.dupe_msgs)
    dh.num_dupe=config.dupe_msgs-1;

  if (dh.high_dupe >= config.dupe_msgs)
    dh.high_dupe=config.dupe_msgs-1;

  return TRUE;
}


/* Flush our buffer of duplicate messages */

void DupeFlushBuffer(void)
{
  if (!config.dupe_msgs)
    return;

  WriteDupeList();
  dupebuf=NULL;
}


/* Fill in the part of the dupe id that pertains to the message header */

static void near GetDidHeader(DUPEID *pid, PXMSG msg)
{
  char temp[PATHLEN];
  int subjsize;
  byte *p;

  pid->crc=(dword)0xffffffffLu;

  /* CRC the first two words of the to/from fields */
  
  pid->crc=crc2word(msg->from, pid->crc);
  pid->crc=crc2word(msg->to, pid->crc);

  /* Figure out how much of the subject line needs to be checked */

  subjsize = (config.flag2 & FLAG2_LONGHDR) ? XMSG_SUBJ_SIZE : 23;

  (void)strncpy(temp, msg->subj, subjsize);
  temp[subjsize]=0;
  (void)strlwr(temp);
  
  /* Remove any 're:" prefixes */
  
  while (temp[0]=='r' && temp[1]=='e' && temp[2]==':' && temp[3]==' ')
    (void)memmove(temp, temp+4, strlen(temp+4)+1);

  for (p=temp; *p; p++)
    if (*p != ' ')
      pid->crc=xcrc32(*p, pid->crc);

  /* Now copy in the message's date.  If it's a valid date (year != 0),     *
   * then simply make a copy of the 4-byte stamp.  Otherwise, hash the      *
   * ASCII date.                                                            */
     
  if (msg->date_written.date.yr == 0)
    pid->date=crcstr((dword)0xffffffffLu, msg->__ftsc_date);
  else
  {
    char *date=msg->__ftsc_date;
    int ch;

    pid->date=*(dword *)(char *)&msg->date_written;

    /* Handle messages with a one-second granularity */

    ch=date[strlen(date)-1];

    if (ch >= '0' && ch <= '9')
      if (((ch-'0') & 1) != 0)
        pid->date=~pid->date;
  }
}




/* Fill in the part of the message header that pertains to the              *
 * MSGID kludge.                                                            */

void GetDidMsgid(DUPEID *pid, char *ctrl)
{
  char *msgid=MsgGetCtrlToken(ctrl, msgid_str);

  /* If there was no MSGID kludge, zero out the msgid lines and do nothing */

  if (!msgid)
  {
    pid->msgid_hash=0L;
    pid->msgid_serial=0L;
    return;
  }

  MashMsgid(msgid+7, &pid->msgid_hash, &pid->msgid_serial);
  MsgFreeCtrlToken(msgid);
}


/* Is the specified message a duplicate? */

int IsADupe(struct _cfgarea *ar, XMSG *msg, char *ctrl, dword uid)
{
  DUPEID far *dupelist;
  DUPEID did, far *dptr, far *dend;
  int fCheckHeader;
  int fCheckMsgid;

  if (!ar || !config.dupe_msgs)
    return FALSE;

  /* If the current dupelist wasn't for this area */

  if (config.has_dlist != ar)
  {
    WriteDupeList();

    if (!ReadDupeList(ar))
      return FALSE;
  }

  /* Get the duplicate ID for this message */

  GetDidHeader(&did, msg);
  GetDidMsgid(&did, ctrl);

  /* Convert this message numbre to a UMSGID */

  did.umsgid=uid;

  /* ... wrap around dupe pointer if necessary... */

  if (dh.high_dupe==config.dupe_msgs)
    dh.high_dupe=0;

  /* Now check the dupelist for a match on BOTH crc and date. */

  dupelist=(DUPEID far *)((DUPEHEAD far *)dupebuf+1);

  fCheckHeader=!!(config.flag2 & FLAG2_DHEADER);
  fCheckMsgid=!!(config.flag2 & FLAG2_DMSGID);

  for (dptr=dupelist, dend=dptr+dh.num_dupe; dptr < dend; dptr++)
  {
    /* Return TRUE if either the CRC/date or the msgid/serial               *
     * match up.                                                            */

    if ((fCheckHeader && did.crc==dptr->crc && did.date==dptr->date) ||
        (fCheckMsgid && did.msgid_hash && did.msgid_hash==dptr->msgid_hash &&
        did.msgid_serial==dptr->msgid_serial))
    {
      return TRUE;
    }
  }

  lastdupe=dupelist[dh.high_dupe];
  have_last_dupe=TRUE;

  dupelist[dh.high_dupe]=did;

  if (++dh.high_dupe > dh.num_dupe)
    dh.num_dupe=dh.high_dupe;

  return FALSE;
}



