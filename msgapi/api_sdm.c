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
static char rcs_id[]="$Id: api_sdm.c,v 1.2 2003/06/05 22:54:50 wesgarland Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "dr.h"
#include "alc.h"
#include "max.h"
#include "old_msg.h"
#include "msgapi.h"
#include "api_sdm.h"
#include "api_sdmp.h"
#include "apidebug.h"

#define SDM_BLOCK 256
#define Mhd ((struct _sdmdata *)(mh->apidata))
#define MsghMhd ((struct _sdmdata *)(((HMSG)msgh)->ha->apidata))


static byte *hwm_from="-=|ÿSquishMailÿ|=-";


//extern void far pascal DosSleep(dword);

HAREA MSGAPI SdmOpenArea(byte OS2FAR *name, word mode, word type)
{
  HAREA mh;
#ifdef __TURBOC__
  NW(_junksqd); /* to shut up tc */
#endif
  if ((mh=palloc(sizeof(*mh)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    goto ErrOpen;
  }

  (void)memset(mh, '\0', sizeof(*mh));

  mh->id=MSGAPI_ID;

  if (type & MSGTYPE_ECHO)
    mh->isecho=TRUE;

  if ((mh->api=(struct _apifuncs *)palloc(sizeof(struct _apifuncs)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    goto ErrOpen;
  }

  (void)memset(mh->api, '\0', sizeof(struct _apifuncs));

  if ((mh->apidata=(void *)palloc(sizeof(struct _sdmdata)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    goto ErrOpen;
  }

  (void)memset((byte *)mh->apidata, '\0', sizeof(struct _sdmdata));
  
  (void)strcpy(Mhd->base,name);
  (void)Add_Trailing(Mhd->base, '\\');
  Mhd->hwm=(dword)-1L;
  
  mh->len=sizeof(*mh);
  mh->num_msg=0;
  mh->high_msg=0;
  mh->high_water=(dword)-1L;


  if (! direxist(name) && (mode==MSGAREA_NORMAL || mkdir(name)==-1))
  {
    msgapierr=MERR_NOENT;
    goto ErrOpen;
  }

  if (! _SdmRescanArea(mh))
    goto ErrOpen;

  mh->type &= ~MSGTYPE_ECHO;

  *mh->api=sdm_funcs;
  mh->sz_xmsg=XMSG_SIZE;

  msgapierr=0;
  return mh;

ErrOpen:

  if (mh)
  {
    if (mh->api)
    {
      if (mh->apidata)
        pfree((char *)mh->apidata);

      pfree(mh->api);
    }

    pfree(mh);
  }

  return NULL;
}



static sword MAPIENTRY SdmCloseArea(HAREA mh)
{
  static byte *msgbody="NOECHO\r\rPlease ignore.  This message is only used "
                       "by the SquishMail system to store\r"
                       "the high water mark for each conference area.\r\r"
                       "\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r\r"
                       "(Elvis was here!)\r\r\r";
  XMSG msg;
  HMSG msgh;
  

  
  if (InvalidMh(mh))
    return -1;
  
  if (Mhd->hwm_chgd)
    if ((msgh=SdmOpenMsg(mh, MOPEN_CREATE, 1L)) != NULL)
    {
      Init_Xmsg(&msg);

      (void)Get_Dos_Date((union stamp_combo *)&msg.date_arrived);
      (void)Get_Dos_Date((union stamp_combo *)&msg.date_written);
      
      /* Use high-bit chars in the to/from field, so that (l)users          *
       * can't log on as this userid and delete the HWM.                    */

      (void)strcpy(msg.from, hwm_from);
      (void)strcpy(msg.to, msg.from);
      (void)strcpy(msg.subj, "High wadda' mark");

      /* To prevent "intl 0:0/0 0:0/0" kludges */
      msg.orig.zone=msg.dest.zone=mi.def_zone;

      msg.replyto=mh->high_water;
      msg.attr=MSGPRIVATE | MSGREAD | MSGLOCAL | MSGSENT;
      
      (void)SdmWriteMsg(msgh, FALSE, &msg, msgbody, strlen(msgbody)+1,
                        strlen(msgbody)+1, 0L, NULL);
                    
      (void)SdmCloseMsg(msgh);
    }

  if (Mhd->msgs_open)
  {
    msgapierr=MERR_EOPEN;
    return -1;
  }

  if (Mhd->msgnum)
    pfree(Mhd->msgnum);

  pfree((char *)mh->apidata);
  pfree(mh->api);

  mh->id=0L;
  pfree(mh);

  msgapierr=MERR_NONE;
  return 0;
}


static UMSGID MAPIENTRY SdmGetNextUid(HAREA ha)
{
  if (InvalidMh(ha))
    return 0L;

  if (!ha->locked)
  {
    msgapierr=MERR_NOLOCK;
    return 0L;
  }

  return ha->high_msg+1;
}


static HMSG  MAPIENTRY SdmOpenMsg(HAREA mh, word mode, dword msgnum)
{
  extern char _stdc _nopen_cheat;
  byte msgname[PATHLEN];

  int handle;
  int filemode;
  unsigned mn, owrite=FALSE;

  HMSG msgh;


  if (InvalidMh(mh))
    return NULL;
  
  if (msgnum==MSGNUM_CUR)
    msgnum=mh->cur_msg;
  else if (msgnum==MSGNUM_PREV)
  {
    for (mn=(unsigned)mh->num_msg-1; (int)mn < (int)mh->high_msg; mn--)
      if (Mhd->msgnum[mn] < (unsigned)mh->cur_msg)
      {
        msgnum=mh->cur_msg=(dword)Mhd->msgnum[mn];
        break;
      }

    /* If mn==-1, no message to go to */

    if (mn==(unsigned)-1)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }
  else if (msgnum==MSGNUM_NEXT)
  {
    for (mn=0; mn < (unsigned)mh->num_msg; mn++)
      if (Mhd->msgnum[mn] > (unsigned)mh->cur_msg)
      {
        msgnum=mh->cur_msg=(dword)Mhd->msgnum[mn];
        break;
      }

    /* If mn==Mhd->msgnum_len, we can't go to any message */

    if (mn==(unsigned)mh->num_msg)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }
  else if (mode != MOPEN_CREATE)
  {
    /* If we're not creating, make sure that the specified msg# can         *
     * be found.                                                            */

    for (mn=0; mn < (unsigned)mh->num_msg; mn++)
      if (msgnum==Mhd->msgnum[mn])
        break;

    if (mn==(unsigned)mh->num_msg)
    {
      msgapierr=MERR_NOENT;
      return NULL;
    }
  }


  if (mode==MOPEN_CREATE)
  {
    /* If we're creating a new message... */

    if (msgnum==0L)
    {
      /* If the base isn't locked, make sure that we avoid conflicts... */

      if (! mh->locked)
      {
        /* Check to see if the msg we're writing already exists */

        (void)sprintf(msgname, sd_msg, Mhd->base, (int)mh->high_msg+1);

        if (fexist(msgname))
        {
          /* If so, rescan the base, to find out which msg# it is. */

          if (Mhd->msgnum && Mhd->msgnum_len)
            pfree(Mhd->msgnum);

          if (!_SdmRescanArea(mh))
            return NULL;
        }
      }

      msgnum=++mh->high_msg;
      
      /* Make sure that we don't overwrite the high-water mark, unless      *
       * we call with msgnum != 0L (a specific number).                     */
         
      if (mh->isecho && msgnum==1)
        msgnum=mh->high_msg=2;
    }
    else
    {
      /* Otherwise, we're overwriting an existing msg */

      owrite=TRUE;
    }

    filemode=O_CREAT | O_TRUNC | O_RDWR;
  }
  else if (mode==MOPEN_READ)
    filemode=O_RDONLY;
  else if (mode==MOPEN_WRITE)
    filemode=O_WRONLY;
  else filemode=O_RDWR;

  (void)sprintf(msgname, sd_msg, Mhd->base, (unsigned)msgnum);

  _nopen_cheat=TRUE;  /* Use fast "cheat" mode for creating msgs */

  if ((handle=sopen(msgname, filemode | O_BINARY | O_NOINHERIT, SH_DENYNONE,
                    S_IREAD | S_IWRITE))==-1)
  {
    if (filemode & O_CREAT)
      msgapierr=MERR_BADF;
    else msgapierr=MERR_NOENT;
    
    _nopen_cheat=FALSE;
    return NULL;
  }

  _nopen_cheat=FALSE;

  mh->cur_msg=msgnum;

  if ((msgh=palloc(sizeof(*msgh)))==NULL)
  {
    (void)close(handle);
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  (void)memset(msgh, '\0', sizeof(*msgh));
  msgh->fd=handle;

  if (mode==MOPEN_CREATE)
  {
    if (mh->num_msg+1 >= Mhd->msgnum_len)
    {
      Mhd->msgnum=repalloc(Mhd->msgnum,
                           (Mhd->msgnum_len += SDM_BLOCK)*sizeof(unsigned));

      if (!Mhd->msgnum)
      {
        pfree(msgh);
        (void)close(handle);
        msgapierr=MERR_NOMEM;
        return NULL;
      }
    }

    /* If we're writing a new msg, this is easy -- just add to end of list */

    if (!owrite /*msgnum==mh->high_msg || mh->num_msg==0*/)
      Mhd->msgnum[(size_t)(mh->num_msg++)]=(unsigned)msgnum;
    else
    {
      for (mn=0; mn < (unsigned)mh->num_msg; mn++)
        if (Mhd->msgnum[mn] >= (unsigned)msgnum)
          break;

      /* If this message is already in the list then do nothing -- simply   *
       * overwrite it, keeping the same message number, so no action is     *
       * required.                                                          */

      if (Mhd->msgnum[mn]==(unsigned)msgnum)
        ;
      else
      {
        /* Otherwise, we have to shift everything up by one since we're     *
         * adding this new message inbetween two others.                    */

        (void)memmove(Mhd->msgnum+mn+1,
                      Mhd->msgnum+mn,
                      (size_t)((int)mh->num_msg-(int)mn) * sizeof(unsigned));
              
        Mhd->msgnum[mn]=(unsigned)msgnum;
        mh->num_msg++;
      }
    }
  }
  
  msgh->cur_pos=0L;
  
  if (mode==MOPEN_CREATE)
    msgh->msg_len=0;
  else msgh->msg_len=(dword)-1;
  
  msgh->ha=mh;
  msgh->id=MSGH_ID;
  msgh->ctrl=NULL;
  msgh->clen=-1;
  msgh->zplen=0;

  msgapierr=MERR_NONE;
  
  /* Keep track of how many messages were opened for this area */

  MsghMhd->msgs_open++;
  
  return msgh;
}





static sword MAPIENTRY SdmCloseMsg(HMSG msgh)
{
  if (InvalidMsgh(msgh))
    return -1;
  
  MsghMhd->msgs_open--;
  
  if (msgh->ctrl)
  {
    pfree(msgh->ctrl);
    msgh->ctrl=NULL;
  }
  
  (void)close(msgh->fd);
  
  msgh->id=0L;
  pfree(msgh);
  
  msgapierr=MERR_NONE;
  return 0;
}




static dword MAPIENTRY SdmReadMsg(HMSG msgh, PXMSG msg, dword offset, dword bytes, byte *text, dword clen, byte *ctxt)
{
  NETADDR *orig,
          *dest;

  byte *fake_msgbuf=NULL;
  byte *newtext;

  unsigned len;
  struct _omsg fmsg;
  dword realbytes;
  unsigned need_ctrl;
  int got;
  
  if (InvalidMsgh(msgh))
    return (dword)-1L;
  
  if (! (clen && ctxt))
  {
    clen=0L;
    ctxt=NULL;
  }
  
  if (! (text && bytes))
  {
    bytes=0L;
    text=NULL;
  }

  orig=&msg->orig;
  dest=&msg->dest;

  if (msg)
  {
    (void)lseek(msgh->fd, 0L, SEEK_SET);

    if (farread(msgh->fd, (char *)&fmsg,
             sizeof(struct _omsg)) != (int)sizeof(struct _omsg))
    {
      msgapierr=MERR_BADF;
      return (dword)-1L;
    }

    fmsg.to[sizeof(fmsg.to)-1]='\0';
    fmsg.from[sizeof(fmsg.from)-1]='\0';
    fmsg.subj[sizeof(fmsg.subj)-1]='\0';
    fmsg.date[sizeof(fmsg.date)-1]='\0';

    Convert_Fmsg_To_Xmsg(&fmsg, msg, mi.def_zone);

    (void)StripNasties(msg->from);
    (void)StripNasties(msg->to);
    (void)StripNasties(msg->subj);
  }


  /* If we weren't instructed to read some message text (ie. only the     *
   * header, read a block anyway.  We need to scan for kludge lines,      *
   * to pick out the appropriate zone/point info.)                        */

  if (msgh->ctrl==NULL && ((msg || ctxt || text) || !(msg || ctxt || text)))
    need_ctrl=TRUE;
  else need_ctrl=FALSE;
  
  realbytes=bytes;
  NW(realbytes);


  /* If we need to read the control information, and the user hasn't      *
   * requested a read operation, we'll need to do one anyway.             */
     
  if (need_ctrl && (text==NULL || bytes < MAX_SDM_CLEN))
  {
    if ((text=fake_msgbuf=palloc(MAX_SDM_CLEN+1))==NULL)
    {
      msgapierr=MERR_NOMEM;
      return (dword)-1;
    }

    text[MAX_SDM_CLEN]='\0';
    bytes=MAX_SDM_CLEN;
  }



  /* If we need to read in some text... */
  
  if (text)
  {
    /* Seek is superfluous if we just read msg header */

    if (!msg || msgh->msgtxt_start != 0)
    {
      (void)lseek(msgh->fd, (long)sizeof(struct _omsg) +
                    (long)msgh->msgtxt_start + (long)offset,
                  SEEK_SET);

      msgh->cur_pos=offset;
    }

    got=farread(msgh->fd, text, (unsigned)bytes);
    
    /* Update counter only if we got some text, and only if we're doing     *
     * a read requested by the user (as opposed to reading ahead to find    *
     * kludge lines).                                                       */

    if (got > 0 && !fake_msgbuf)
      msgh->cur_pos += (unsigned)got;
  }
  else got=0;


  /* Convert the kludges into 'ctxt' format */
  
  if (need_ctrl && got > 0 && offset==0L)
  {
    len=(unsigned)got;
    
    if ((msgh->ctrl=CopyToControlBuf(text, &newtext, &len)) != NULL)
    {
      msgh->clen=(sdword)strlen(msgh->ctrl)+1;
      msgh->msgtxt_start=(dword)(newtext-text);

      /* Shift back the text buffer to counter absence of ^a strings */

      (void)memmove(text,newtext, (size_t)(bytes-(dword)(newtext-text)));

      got -= (int)(msgh->clen-1u);
    }
  }
  

  /* Scan the ctxt ourselves to find zone/point info */

  if (msg && msgh->ctrl)
    ConvertControlInfo(msgh->ctrl, orig, dest);
  

  /* And if the app requested ctrlinfo, put it in its place. */
      
  if (ctxt && msgh->ctrl)
    (void)memmove(ctxt, msgh->ctrl, min(strlen(msgh->ctrl)+1, (size_t)clen));

  if (fake_msgbuf)
  {
    pfree(fake_msgbuf);
    got=0;
  }

  msgapierr=MERR_NONE;
  return (dword)got;
}




static sword MAPIENTRY SdmWriteMsg(HMSG msgh, word append, PXMSG msg, byte *text, dword textlen, dword totlen, dword clen, byte *ctxt)
{
  struct _omsg fmsg;
  byte *s;
  
  NW(totlen);

  if (clen==0L || ctxt==NULL)
  {
    ctxt=NULL;
    clen=0L;
  }
  
  if (InvalidMsgh(msgh))
    return -1;

  (void)lseek(msgh->fd, 0L, SEEK_SET);

  if (msg)
  {
    Convert_Xmsg_To_Fmsg(msg, &fmsg);
    
    if (farwrite(msgh->fd, (char *)&fmsg,
              sizeof(struct _omsg)) != (int)sizeof(struct _omsg))
    {
      msgapierr=MERR_NODS;
      return -1;
    }

    if (!append && msgh->clen <= 0 && msgh->zplen==0)
    {
      statfd=msgh->fd;
      msgh->zplen=(word)WriteZPInfo(msg, WriteToFd, NULL);
    }
  }
  else if (!append || ctxt) /* Skip over old message header */
  {
    (void)lseek(msgh->fd, (long)sizeof(struct _omsg) + (long)msgh->zplen,
                SEEK_SET);
  }

  /* Now write the control info / kludges */

  if (clen && ctxt)
  {

    if (!msg)
      (void)lseek(msgh->fd,
                  (long)sizeof(struct _omsg) + (long)msgh->zplen,
                  SEEK_SET);

    s=CvtCtrlToKludge(ctxt);

    if (s)
    {
      unsigned sl_s=(unsigned)strlen(s);
      int ret;

      ret=farwrite(msgh->fd, s, sl_s);
      MsgFreeCtrlToken(s);

      if (ret != (int)sl_s)
      {
        msgapierr=MERR_NODS;
        return -1;
      }
    }
  }

  if (append)
    (void)lseek(msgh->fd, 0L, SEEK_END);

  if (text)
    if (farwrite(msgh->fd, text, (unsigned)textlen) != (int)textlen)
    {
      msgapierr=MERR_NODS;
      return -1;
    }

  msgapierr=MERR_NONE;
  return 0;
}




static sword MAPIENTRY SdmKillMsg(HAREA mh, dword msgnum)
{
  dword hwm;
  byte temp[PATHLEN];
  unsigned mn;
  
  if (InvalidMh(mh))
    return -1;


  /* Remove the message number from our private index */

  for (mn=0; mn < (unsigned)mh->num_msg; mn++)
    if (Mhd->msgnum[mn]==(unsigned)msgnum)
    {
      (void)memmove(Mhd->msgnum+mn,
                    Mhd->msgnum+mn+1,
                    (size_t)((unsigned)mh->num_msg - mn - 1) * sizeof(unsigned));
      break;
    }

  /* If we couldn't find it, return an error message */

  if (mn==(unsigned)mh->num_msg)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  (void)sprintf(temp, sd_msg, Mhd->base, (unsigned)msgnum);

  if (unlink(temp)==-1)
  {
    msgapierr=MERR_NOENT;
    return -1;
  }

  mh->num_msg--;


  /* Adjust the high message number */

  if (msgnum==mh->high_msg)
  {
    if (mh->num_msg)
      mh->high_msg=(dword)Mhd->msgnum[(unsigned)mh->num_msg - 1];
    else mh->high_msg=0;
  }


  /* Now adjust the high-water mark, if necessary */

  hwm=SdmGetHighWater(mh);

  if (hwm != (dword)-1 && hwm > 0 && hwm == msgnum) /*SJD Fri  01-24-1992  21:38:37 */
    (void)SdmSetHighWater(mh, msgnum-1);

  msgapierr=MERR_NONE;
  return 0;
}



static sword MAPIENTRY SdmLock(HAREA mh)
{
  if (InvalidMh(mh))
    return -1;

  msgapierr=MERR_NONE;
  return 0;
}

static sword MAPIENTRY SdmUnlock(HAREA mh)
{
  if (InvalidMh(mh))
    return -1;

  msgapierr=MERR_NONE;
  return 0;
}




sword MSGAPI SdmValidate(byte OS2FAR *name)
{
  msgapierr=MERR_NONE;
  return (direxist(name) != FALSE);
}




static sword MAPIENTRY SdmSetCurPos(HMSG msgh, dword pos)
{
  if (InvalidMsgh(msgh))
    return 0;

  (void)lseek(msgh->fd, (long)(msgh->cur_pos=pos), SEEK_SET);
  msgapierr=MERR_NONE;
  return 0;
}



static dword MAPIENTRY SdmGetCurPos(HMSG msgh)
{
  if (InvalidMsgh(msgh))
    return (dword)-1L;

  msgapierr=MERR_NONE;
  return msgh->cur_pos;
}




static UMSGID MAPIENTRY SdmMsgnToUid(HAREA mh, dword msgnum)
{
  if (InvalidMh(mh))
    return (UMSGID)-1;

  msgapierr=MERR_NONE;
  return (UMSGID)msgnum;
}



static dword MAPIENTRY SdmUidToMsgn(HAREA mh, UMSGID umsgid, word type)
{
  unsigned wmsgid, mn;
  
  if (InvalidMh(mh))
    return (dword)-1L;

  msgapierr=MERR_NONE;
  wmsgid=(unsigned)umsgid;
  
  for (mn=0; mn < (unsigned)mh->num_msg; mn++)
    if (Mhd->msgnum[mn]==wmsgid ||
        (type==UID_NEXT && Mhd->msgnum[mn] >= wmsgid) ||
        (type==UID_PREV && Mhd->msgnum[mn] <= wmsgid &&
        ((mn+1) >= (unsigned)mh->num_msg || Mhd->msgnum[mn+1] > wmsgid)))
      return ((dword)Mhd->msgnum[mn]);

  msgapierr=MERR_NOENT;
  return 0L;
}


static dword MAPIENTRY SdmGetHighWater(HAREA mh)
{
  HMSG msgh;
  XMSG msg;
  
  if (InvalidMh(mh))
    return (dword)-1;
  
  /* If we've already fetched the highwater mark... */
  
  if (mh->high_water != (dword)-1L)
    return (mh->high_water);
  
  if ((msgh=SdmOpenMsg(mh, MOPEN_READ, 1L))==NULL)
    return 0L;
  
  if (SdmReadMsg(msgh, &msg, 0L, 0L, NULL, 0L, NULL)==(dword)-1 ||
      !eqstr(msg.from, hwm_from))
  {
    mh->high_water=0L;
  }
  else mh->high_water=(dword)msg.replyto;
  
  (void)SdmCloseMsg(msgh);
  
  return (mh->high_water);
}


static sword MAPIENTRY SdmSetHighWater(HAREA mh,dword hwm)
{
  if (InvalidMh(mh))
    return -1;
  
  /* Only write it to memory for now.  We'll do a complete update of        *
   * the real HWM in 1.MSG only when doing a MsgCloseArea(), to save        *
   * time.                                                                  */

  if (hwm != mh->high_water)
    Mhd->hwm_chgd=TRUE;
  
  mh->high_water=hwm;
  return 0;
}



static dword MAPIENTRY SdmGetTextLen(HMSG msgh)
{
  dword pos;
  dword end;
  
  /* Figure out the physical length of the message */

  if (msgh->msg_len==(dword)-1)
  {
    pos=(dword)tell(msgh->fd);
    end=(dword)lseek(msgh->fd, 0L, SEEK_END);

    msgh->msg_len=(end < sizeof(struct _omsg))
                    ? 0L : (unsigned long)(end-(dword)sizeof(struct _omsg));

    (void)lseek(msgh->fd, (long)pos, SEEK_SET);
  }
  
  /* If we've already figured out the length of the control info */
  
  if (msgh->clen == (sdword)-1 && _Grab_Clen(msgh)==-1)
    return 0L;
  else if ((dword)msgh->msg_len > (dword)msgh->msgtxt_start)
    return (dword)(msgh->msg_len - msgh->msgtxt_start);
  else
    return 0L;
}



static dword MAPIENTRY SdmGetCtrlLen(HMSG msgh)
{
  /* If we've already figured out the length of the control info */
  
  if (msgh->clen==-1 && _Grab_Clen(msgh)==-1)
    return 0;
  else return (dword)msgh->clen;
}


















static sword near _Grab_Clen(HMSG msgh)
{
  return ((sdword)SdmReadMsg(msgh,NULL,0L,0L,NULL,0L,NULL) < (sdword)0
                        ? -1 : 0);
}



/* Rescan *.MSG area to find the number of messages contained therein */

static sword near _SdmRescanArea(HAREA mh)
{
  FFIND *ff;
  byte temp[PATHLEN];
  unsigned mn, thismsg;

  mh->num_msg=0;

  if ((Mhd->msgnum=palloc(SDM_BLOCK * sizeof(unsigned)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  Mhd->msgnum_len=SDM_BLOCK;

  (void)sprintf(temp, "%s*.msg", Mhd->base);

  if ((ff=FindOpen(temp, 0)) != 0)
  {
    mn=0;

    do
    {
      /* Don't count zero-length or invalid messages */

      if (ff->ulSize < sizeof(struct _omsg))
        continue;

      if (mn >= Mhd->msgnum_len)
      {
        Mhd->msgnum=repalloc(Mhd->msgnum,
                             (Mhd->msgnum_len += SDM_BLOCK) * sizeof(unsigned));

        if (!Mhd->msgnum)
        {
          msgapierr=MERR_NOMEM;
          return FALSE;
        }
      }

      if ((thismsg=(unsigned)atoi(ff->szName)) != 0)
      {
        Mhd->msgnum[mn++]=thismsg;

        if ((dword)thismsg > mh->high_msg)
          mh->high_msg=(dword)thismsg;
        
        mh->num_msg=(dword)mn;
      }

      #ifdef OS_2
      if((mn % 128)==127)
        DosSleep(1L);
      #endif
    }
    while (FindNext(ff)==0);

    FindClose(ff);

    /* Now sort the list of messages */

    qksort((int *)Mhd->msgnum, (unsigned)mh->num_msg);
  }
  
  return TRUE;
}





static void MSGAPI Init_Xmsg(PXMSG msg)
{
  (void)memset(msg, '\0', XMSG_SIZE);
}

static void MSGAPI Convert_Fmsg_To_Xmsg(struct _omsg *fmsg, PXMSG msg, word def_zone)
{
  NETADDR *orig, *dest;

  Init_Xmsg(msg);

  orig=&msg->orig;
  dest=&msg->dest;

  fmsg->to[sizeof(fmsg->to)-1]='\0';
  fmsg->from[sizeof(fmsg->from)-1]='\0';
  fmsg->subj[sizeof(fmsg->subj)-1]='\0';
  fmsg->date[sizeof(fmsg->date)-1]='\0';

  (void)strcpy(msg->from, fmsg->from);
  (void)strcpy(msg->to  , fmsg->to  );
  (void)strcpy(msg->subj, fmsg->subj);

  orig->zone=dest->zone=def_zone;
  orig->point=dest->point=0;

  orig->net=(word)fmsg->orig_net;
  orig->node=(word)fmsg->orig;

  dest->net=(word)fmsg->dest_net;
  dest->node=(word)fmsg->dest;

  Get_Binary_Date(&msg->date_written, &fmsg->date_written, fmsg->date);
  Get_Binary_Date(&msg->date_arrived, &fmsg->date_arrived, fmsg->date);

  (void)strcpy(msg->__ftsc_date, fmsg->date);

  msg->utc_ofs=0;

  msg->replyto=fmsg->reply;
  msg->replies[0]=fmsg->up;
  msg->attr=(dword)fmsg->attr;

  /*
  sprintf(orig->ascii,sqaddr_fmt,orig->zone,orig->net,orig->node,orig->point);
  sprintf(dest->ascii,sqaddr_fmt,dest->zone,dest->net,dest->node,dest->point);
  */
  
  /* Convert 4d pointnets */

  if (fmsg->times==~fmsg->cost && fmsg->times)
    msg->orig.point=fmsg->times;
}

static void MSGAPI Convert_Xmsg_To_Fmsg(PXMSG msg,struct _omsg *fmsg)
{
  NETADDR *orig, *dest;
        
  (void)memset(fmsg, '\0', sizeof(struct _omsg));

  orig=&msg->orig;
  dest=&msg->dest;

  (void)strncpy(fmsg->from, msg->from, sizeof(fmsg->from));
  (void)strncpy(fmsg->to  , msg->to  , sizeof(fmsg->to));
  (void)strncpy(fmsg->subj, msg->subj, sizeof(fmsg->subj));

  fmsg->from[sizeof(fmsg->from)-1]='\0';
  fmsg->to  [sizeof(fmsg->to  )-1]='\0';
  fmsg->subj[sizeof(fmsg->subj)-1]='\0';

  fmsg->orig_net=(sword)orig->net;
  fmsg->orig=(sword)orig->node;

  fmsg->dest_net=(sword)dest->net;
  fmsg->dest=(sword)dest->node;
  
  if (*msg->__ftsc_date)
  {
    (void)strncpy(fmsg->date, msg->__ftsc_date, sizeof(fmsg->date));
    fmsg->date[sizeof(fmsg->date)-1]='\0';
  }
  else (void)sprintf(fmsg->date, "%02d %s %02d  %02d:%02d:%02d",
                     msg->date_written.date.da ? msg->date_written.date.da : 1,
                     months_ab[msg->date_written.date.mo
                                 ? msg->date_written.date.mo-1
                                 : 0],
                     (msg->date_written.date.yr+80) % 100,
                     msg->date_written.time.hh,
                     msg->date_written.time.mm,
                     msg->date_written.time.ss << 1);
             
  fmsg->date_written=msg->date_written;
  fmsg->date_arrived=msg->date_arrived;

  fmsg->reply=(word)msg->replyto;
  fmsg->up=(word)msg->replies[0];
  fmsg->attr=(word)(msg->attr & 0xffffL);


  /* Non-standard point kludge to ensure that 4D pointmail works correctly */

  if (orig->point)
  {
    fmsg->times=orig->point;
    fmsg->cost=~fmsg->times;
  }
}


int MAPIENTRY WriteZPInfo(PXMSG msg, void (MAPIENTRY *wfunc)(byte OS2FAR *str), byte OS2FAR *kludges)
{
  byte temp[PATHLEN];
  byte *null="";
  int bytes=0;

  if (!kludges)
    kludges=null;

  if ((msg->dest.zone != mi.def_zone || msg->orig.zone != mi.def_zone) &&
      !stristr(kludges, "\x01INTL"))
  {
    (void)sprintf(temp, "\x01INTL %u:%u/%u %u:%u/%u\r",
                  (unsigned)msg->dest.zone, (unsigned)msg->dest.net,
                  (unsigned)msg->dest.node, (unsigned)msg->orig.zone,
                  (unsigned)msg->orig.net, (unsigned)msg->orig.node);

    (*wfunc)(temp);
    bytes += (int)strlen(temp);
  }

  if (msg->orig.point && !strstr(kludges, "\x01""FMPT"))
  {
    (void)sprintf(temp, "\x01""FMPT %u\r", (unsigned)msg->orig.point);
    (*wfunc)(temp);
    bytes += (int)strlen(temp);
  }

  if (msg->dest.point && !strstr(kludges, "\x01""TOPT"))
  {
    (void)sprintf(temp, "\x01""TOPT %u\r", (unsigned)msg->dest.point);
    (*wfunc)(temp);
    bytes += (int)strlen(temp);
  }

  return bytes;
}


static void MAPIENTRY WriteToFd(byte OS2FAR *str)
{
  (void)farwrite(statfd, str, strlen(str));
}

  
static void near Get_Binary_Date(struct _stamp *todate, struct _stamp *fromdate, byte *asciidate)
{
#ifdef HCC
  /* If compiling for the HCC, use the ASCII message date only, not         *
   * the binary dates.  This breaks MR, but Max doesn't mind.               */

  ASCII_Date_To_Binary(asciidate, (union stamp_combo *)todate);
#else
  if (fromdate->date.da==0 ||
      fromdate->date.da > 31 ||
      fromdate->date.yr < 7  ||
      fromdate->time.hh > 23 ||
      fromdate->time.mm > 59 ||
      /*fromdate->time.ss > 59 ||*/ /* lint: constant out of range */
      ((union stamp_combo *)&fromdate)->ldate==0)
  {
    ASCII_Date_To_Binary(asciidate,(union stamp_combo *)todate);
  }
  else *todate=*fromdate;
#endif
}

