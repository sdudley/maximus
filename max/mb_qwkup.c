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
static char rcs_id[]="$Id: mb_qwkup.c,v 1.9 2004/01/12 18:54:33 wmcbrine Exp $";
#pragma on(unreferenced)

/*# QWK uploads, for processing .REP packets
*/

#define MAX_LANG_f_area
#define MAX_LANG_m_browse
#define MAX_LANG_max_chat
#define MAX_LANG_max_main

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <ctype.h>
#include <unistd.h>
#include "prog.h"
#include "max_msg.h"
#include "max_file.h"
#include "qwk.h"
#include "mb_qwkup.h"
#include "arcmatch.h"
#include "trackc.h"
#include "trackm.h"



/* Information from the mb_qwk.c package */

extern struct _akh akh;
extern struct _akd *akd;
extern char *qwk_path;

static char *msg_name;


void QWK_Upload(void)
{
  UMSGID uid;
  char fname[PATHLEN];
  int okay;

  logit(log_qwk_upload);

  if ((qwk_path=malloc(PATHLEN))==NULL)
  {
    logit(mem_none);
    return;
  }
  
  if (Make_QWK_Directory()==-1)
  {
    free(qwk_path);
    return;
  }
  
  save_tag_list(NULL);

  uid=MsgMsgnToUid(sq, last_msg);
  
  if ((msg_name=malloc(PATHLEN))==NULL)
    logit(mem_none);
  else
  {
    memset(msg_name, 0, PATHLEN);
    strcat(msg_name, PRM(olr_name));
    strcat(msg_name, dot_msg);


    if (Read_Kludge_File(&akh, &akd) != -1)
    {
      if (Receive_REP(fname)==-1)
        Printf(err_receive_rep, PRM(olr_name));
      else
      {
        /* Default: it's okay to kill packet after toss. */

        okay=TRUE;

        if (Decompress_REP(fname)==-1)
        {
          Printf(err_decompr_rep, PRM(olr_name));
          okay=FALSE;
        }
        else
        {
          if (Toss_QWK_Packet(msg_name)==-1)
          {
            Printf(err_toss_qwk, msg_name);
            okay=FALSE;
          }

          unlink(msg_name);
        }

        /* Delete the received .REP file.  Always delete it if it was     *
         * uploaded.  If it was a local packet, only delete it if it      *
         * was tossed okay.                                               */

        if (!local || okay)
          unlink(fname);
      }


      Write_Kludge_File(&akh, &akd);
      free(akd);
    }

    free(msg_name);

  }

  Clean_QWK_Directory(TRUE);
  free(qwk_path);
  
  Free_Filenames_Buffer(0);
  restore_tag_list(NULL,FALSE);
}


static int near Receive_REP(char *name)
{
  char temp[PATHLEN];
  struct _css css;
  sword protocol;
  long ul_start_time;
  sword fn;

  
  if (!local && File_Get_Protocol(&protocol, FALSE, TRUE)==-1)
    return -1;

  ChatSaveStatus(&css);
  ChatSetStatus(FALSE, cs_file_xfer);

  /* Name of the file to receive */

  sprintf(temp, ss, PRM(olr_name), dot_rep);

#ifndef UNIX
  upper_fn(temp);
#endif

  /* Save the current time */

  ul_start_time=time(NULL);


  /* Perform the actual upload */
  
  if (local)
  {
    WhiteN();
    InputGets(temp, qwk_local_rep);
    
    if (*temp)
    {
      strcpy(name, temp);
      AddFileEntry(temp, FFLAG_GOT, -1L);
    }
    else Free_Filenames_Buffer(0);
  }
  else
  {
    word sf2;
    FENTRY fent;
    
    Printf(rep_to_send, temp);
    
    /* Temporarily disable the dupe upload checking, so that an upload      *
     * of SMURF.REP doesn't collide with SMURF.ZIP (or whatever) in a       *
     * normal file area.                                                    */

    sf2=prm.flags2;
    prm.flags2 &= ~(FLAG2_CHECKDUPE|FLAG2_CHECKEXT);
    
    Free_Filenames_Buffer(0);

    if (!IsBatch(protocol))
      AddFileEntry(temp, 0, 0);

    File_Get_Files(protocol, NULL, qwk_path);
    
    prm.flags2=sf2;

    for (fn=1; GetFileEntry(fn, &fent); fn++)
    {
      sprintf(name, ss, qwk_path, fent.szName);
      unlink(name);

#ifdef TEST_VER
      logit("@Deleting %s", name);
#endif
    }

    if (GetFileEntry(0, &fent))
    {
      strcpy(name, qwk_path);
      strcat(name, fent.szName);
    }
    else *name='\0';

  }

  

  /* Compensate the user for the time spent uploading */

  Add_To_Time(time(NULL)-ul_start_time);
  
  ChatRestoreStatus(&css);


  if (!FileEntries())
    Puts(xferaborted);
  else
  {
#ifdef UNIX
    adaptcase(name);
#endif    
    if (fexist(name))
      return 0;
    else
    {
      logit(cantfind, name);
      return -1;
    }
  }
  
  return -1;
}



static int near Decompress_REP(char *rep_name)
{
  char cmd[PATHLEN];
  char temp[PATHLEN];
  struct _arcinfo *ai;
  int ret, fd, ctr;
  static char qwk_busy[]="qwk_busy.$$$";
  Load_Archivers();


  /* Now autodetect the archive type */

  if ((fd=open(rep_name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return -1;

  for (ai=ari; ai; ai=ai->next)
    if (MatchArcType(fd, ai))
      break;
    
  close(fd);

  if (!ai)
    ai=ari;

  if (!ai)
  {
    Puts(unknown_compr);
    ret=-1;
  }
  else
  {
    Form_Archiver_Cmd(rep_name, msg_name, cmd, ai->extract);

    /* Check for QWK_BUSY.$$$ semaphore to prevent overwrites */

    for (ctr=0; ctr < 10; ctr++)
      if (fexist(qwk_busy))
        Delay(100);
      else
        break;

    if (ctr==10)
    {
      Puts(qwk_busy_msg);
      logit(log_qwk_busy, qwk_busy);
      ret=-123;
    }
    else
    {
      /* Create semaphore */

      if ((fd=sopen(qwk_busy, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                    SH_DENYNO, S_IREAD | S_IWRITE)) != -1)
        close(fd);


      ret=Outside(NULL, NULL, OUTSIDE_RUN, cmd, FALSE, CTL_NONE, 0, NULL);

      sprintf(temp, ss, qwk_path, msg_name);

      /* Try to move file out of the way.  If it succeeds, use it from there */

      if (move_file(msg_name, temp)==0)
        strcpy(msg_name, temp);

      unlink(qwk_busy);
    }
  }

  #ifdef UNIX
  adaptcase(msg_name);
  #endif

  if (ret != 0 || !fexist(msg_name))
  {
    logit(log_err_compr, ret);
    unlink(msg_name);
    ret=-1;
  }
  
  return ret;
}




/* Toss the given xxxxxxxx.MSG packet into the appropriate message bases */

static int near Toss_QWK_Packet(char *name)
{
  struct _qmhdr *qh;
  XMSG msg;

  char *block;

  word msgn;
  int qfd, ret;

  ret=0;
#ifndef UNIX
  upper_fn(name);
#else
  adaptcase(name);
#endif

  Printf(tossing_rep_packet, No_Path(name));
  
  /* Allocate a block of memory to hold the QWK header */
  
  if ((block=malloc(QWK_RECSIZE*3))==NULL)
    return -1;

  /* Open the .MSG file itself */
  
  if ((qfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    free(block);
    return -1;
  }
  
  qh=(struct _qmhdr *)block;

  /* Now try to snatch the header for the .REP file itself */
    
  if (QWK_Get_Rep_Header(qfd, block) != -1)
  {
    /* Now, simply read in all of the messages in the packets, convert      *
     * them to Fido format, and write them to the message base              *
     * as appropriate.                                                      */

    for (msgn=1; read(qfd, (char *)block, QWK_RECSIZE)==QWK_RECSIZE; msgn++)
    {
      QWK_To_Xmsg(qh=(struct _qmhdr *)block, &msg, msgn);

      if (!Toss_QWK_Message(qh, &msg, qfd, block))
        lseek(qfd, (atoi(qh->len)-1)*QWK_RECSIZE, SEEK_CUR);
    }
  }
  
  close(qfd);
  free(block);

  return ret;
}


/* Read the first header in a QMail .MSG (reply packet) file */

static int near QWK_Get_Rep_Header(int qfd, char *block)
{
  char *p, *end;
  
  if (read(qfd, block, QWK_RECSIZE) != QWK_RECSIZE)
    return -1;

  block[15]='\0';
  
  for (p=block, end=block+QWK_RECSIZE; p < end; p++)
    if (*p==' ')
    {
      *p='\0';
      break;
    }
    
  if (!eqstri(block, PRM(olr_name)))
  {
    Printf(wrong_rep, block, PRM(olr_name));
    return -1;
  }
  
  return 0;
}


/* Convert the PCBored-style message header to a Max/FTSC one */

static void near QWK_To_Xmsg(struct _qmhdr *qh, XMSG *msg, word msgn)
{
  char date[40];
  SCOMBO now, *d;
  int year;
  
  Blank_Msg(msg);

  /* Copy in the 'To:' field, and undo all of the BASIC crap */

  memmove(msg->to, qh->to, sizeof(qh->to));
  msg->to[sizeof(qh->to)-1]='\0';

  if (all_caps(fix_basic_crap(msg->to)))
    cfancy_str(msg->to);


  /* Copy in the 'Subj:' field */

  memmove(msg->subj, qh->subj, sizeof(qh->subj));
  msg->subj[sizeof(qh->subj)-1]='\0';

  if (all_caps(fix_basic_crap(msg->subj)))
    cfancy_str(msg->subj);

  if (isdigit(*qh->replyto))
    msg->replyto=atol(qh->replyto);

  /* Now convert the PCBored-format message date */

  Get_Dos_Date((SCOMBO *)&msg->date_written);

  strncpy(date, qh->date, 8);
  date[8]='\0';
  
  year = atoi(date+6);

  if (year >= 80)
    msg->date_written.date.yr = year-80;
  else
    msg->date_written.date.yr = year + 20;

  msg->date_written.date.mo = atoi(date);
  msg->date_written.date.da = atoi(date+3);

  strncpy(date, qh->time, 5);
  date[5]='\0';

  msg->date_written.time.hh = atoi(date);
  msg->date_written.time.mm = atoi(date+3);
  msg->date_written.time.ss = 0;

  Get_Dos_Date(&now);

  d=(SCOMBO *)&msg->date_written;

  if (/*GEdate(d, &now) ||*/
      d->msg_st.date.mo > 12 ||
      d->msg_st.date.da > 31 ||
      d->msg_st.time.ss > 59 ||
      d->msg_st.time.mm > 59 ||
      d->msg_st.time.hh > 23)
  {
    char *lm=log_qwk_bad_date;

    /* If we found an invalid date, zero out the time field.  This will     *
     * give the messages a valid time.  This is done instead of using       *
     * the current time, since adding in the current h/m/s would prevent    *
     * duplicate detection from catching these messages on a remote         *
     * system.                                                              */

    logit(*lm ? lm : "!Bad date in QWK msg"); /**/

    d->dos_st.date=now.dos_st.date;
    d->msg_st.time.hh=0;
    d->msg_st.time.mm=0;
    d->msg_st.time.ss=0;
  }

  /* Increment the seconds counter by 2 for every msg we toss.  This is     *
   * so that legit messages within the same packet won't be counted         *
   * as dupes by the remote system.  This won't cause problems              *
   * with users duping messages by re-uploading, since it's based on        *
   * the message's number within the packet, which remains constant.        */

  msg->date_written.time.ss = ((msgn-1) % 30);


  /* Store today's date */

  Get_Dos_Date((SCOMBO *)&msg->date_arrived);


  /* Make sure to handle private messages appropriately */

  if (qh->status=='*' || qh->status=='+')
    msg->attr |= MSGPRIVATE;
}


/* Get the ^aMSGID from the original msg, if possible */

static void near QWKGetMsgid(HAREA sq, UMSGID replyto, char *reply_kludge)
{
  char *ctrl_buf=NULL;
  char *rep;
  unsigned ctrl_len;
  XMSG xmsg;
  HMSG hmsg;
  dword rep_mn=MsgUidToMsgn(sq, replyto, UID_EXACT);

  /* Couldn't find a reply kludge in the original message */

  *reply_kludge=0;

  /* Can't do anything with msg links for a *.MSG-style area */

  if (mah.ma.type & MSGTYPE_SDM)
    return;

  /* Original message does not exist? */

  if (!rep_mn || (hmsg=MsgOpenMsg(sq, MOPEN_RW, rep_mn))==NULL)
    return;


  /* Allocate enough memory to hold the message control information */

  if ((ctrl_len=MsgGetCtrlLen(hmsg)) > 0 &&
      (ctrl_buf=malloc(ctrl_len)) != NULL)
  {
    /* Read the original message header */

    MsgReadMsg(hmsg, &xmsg, 0, 0, NULL, ctrl_len, ctrl_buf);


    /* Try to pick a ^aMSGID kludge out of the original message */

    if ((rep=MsgGetCtrlToken(ctrl_buf, "MSGID:")) != NULL)
    {
      /* Take the MSGID kludge and convert it into a reply kludge */

      strnncpy(reply_kludge, rep, PATHLEN);
      memmove(reply_kludge, "REPLY", 5);
      MsgFreeCtrlToken(rep);
    }

    free(ctrl_buf);
  }

  MsgCloseMsg(hmsg);
}

/* Adjust the reply links based on the QWK message header */

static void near QWKFixLinks(HAREA sq, dword mn, UMSGID replyto)
{
  XMSG xmsg;
  HMSG hmsg;
  dword rep_mn=MsgUidToMsgn(sq, replyto, UID_EXACT);
  int i;

  /* Can't do anything with msg links for a *.MSG-style area */

  if (mah.ma.type & MSGTYPE_SDM)
    return;


  /* Original message does not exist? */

  if (!rep_mn || (hmsg=MsgOpenMsg(sq, MOPEN_RW, rep_mn))==NULL)
    return;


  /* Read the original message header */

  MsgReadMsg(hmsg, &xmsg, 0, 0, NULL, 0L, NULL);

  /* Scan for an unused slot */

  for (i=0; i < MAX_REPLY; i++)
    if (!xmsg.replies[i])
    {
      xmsg.replies[i]=MsgMsgnToUid(sq, mn);
      break;
    }

  /* Update the message header */

  if (i < MAX_REPLY)
    MsgWriteMsg(hmsg, FALSE, &xmsg, NULL, 0, 0, 0, NULL);

  MsgCloseMsg(hmsg);
}


/* Get the area name from this message.  Returns TRUE if ok, FALSE if deleted */

static int near QWKRetrieveAreaFromPkt(PXMSG msg, struct _qmhdr *qh, char *aname, word *pwTossTo)
{
  word tossto;    /* Area (cardinal #) in which to place msg */

  /* Skip any deleted messages, or those messages which are addressed     *
   * to the Maximus "control program".                                    */
  
  if (qh->msgstat==QWK_KILLED || eqstri(msg->to, cprog_name))
    return FALSE;

  /* New method -- prefer ASCII version */

  tossto=atoi(qh->msgn);
  
  /* Otherwise, use the low byte of the binary conference number (omitting
   * the high byte for the benefit of EZ-Reader, per old method's comments;
   * check this) */

  if (!tossto)
    tossto=qh->confLSB;

  /* Now state what we're doing */

  Printf(qwk_msg_stats, tossto, msg->to, msg->subj);

  /* Adjust to internal offset */

  tossto--;

  /* Get area name based on offset.
   * If the area number is too high, get a new area from the user. */

  strcpy(aname, (tossto >= akh.num_areas) ? qmark : (char *)(akd[tossto].name));

  *pwTossTo=tossto;
  return TRUE;
}



/* Ensure that the current area is valid -- if not, get another one from    *
 * the user.                                                                */

static int near QWKGetValidArea(PXMSG msg, char *aname, word tossto)
{
  BARINFO bi;
  MAH ma;
  int rc=TRUE;

  memset(&ma, 0, sizeof ma);

  while (eqstri(aname, qmark) ||
         !ReadMsgArea(ham, aname, &ma) ||
         !ValidMsgArea(NULL, &ma, VA_VAL | VA_PWD, &bi) ||
         !PopPushMsgArea(aname, &bi) ||
	 #ifndef UNIX
         ((mah.ma.attribs & MA_READONLY) && !mailflag(CFLAGM_RDONLYOK)) ||
         !CanAccessMsgCommand(&mah, msg_upload, 0))
	 #else
         ((mah.ma.attribs & MA_READONLY) && !mailflag(CFLAGM_RDONLYOK)))
	 #endif	 
  {
    Printf(qwk_invalid_area, msg->to, msg->subj);

    InputGets(aname, which_area);

    if (*aname=='\0')          /* <enter> means skip message */
    {
      rc=FALSE;
      break;
    }
    else if (eqstri(aname, qmark))
      ListMsgAreas(NULL, FALSE, FALSE);

    /* Set 'tossto' to zero so that we can recycle properly */
  }


  /* Make sure that this message area is in the translation index */

  InsertAkh(aname, (int)tossto);


  DisposeMah(&ma);
  return rc;
}





/* Adjust the fields in the QWK message header appropriately */

static void near QWKFixHeader(PXMSG msg, struct _qmhdr *qh)
{
  /* Now copy in the user's name for the "From:" field, now that we have    *
   * the current area number.  Skip the 'From:' field in the packet         *
   * itself, since we know which user it's from anyway, UNLESS it's         *
   * an anonymous area.                                                     */

  if (mah.ma.attribs & MA_REAL)
    strcpy(msg->from, usr.name);
  else if ((mah.ma.attribs & MA_ALIAS) && *usr.alias)
    strcpy(msg->from, usr.alias);
  else strcpy(msg->from, usrname);

  if ((mah.ma.attribs & MA_ANON) && *qh->from != ' ')
  {
    memmove(msg->from, qh->from, sizeof(qh->from));
    msg->from[sizeof(qh->from)-1]='\0';

    if (all_caps(fix_basic_crap(msg->from)))
      cfancy_str(msg->from);
  }

  msg->orig=mah.ma.primary;

  /* Fix the private bit, according to the areas' settings */

  FixPrivateStatus(msg, &mah);
 
  /* Display the name of the real area to toss it to */

  Printf(qwk_max_area, MAS(mah, name));
  vbuf_flush();
}



/* Write the message to the squish area */

static int near QWKTossMsgBody(PXMSG msg, struct _qmhdr *qh, int msg_blocks, int qfd, char *block, char **pkludge, int *pfUpdateStatus)
{
#ifdef MAX_TRACKER
  QWKTRACKINFO qti;                       /* Tracking info from current msg */
  char *actinfo;                          /* Current ^!>ACTRACK pos'n */
#endif
  HMSG msgh;                              /* Handle for msg being written */
  char reply_kludge[PATHLEN];             /* ^aREPLY kludge */
  char orig[MAX_OTEAR_LEN];               /* Origin line for msg */
  byte *kludge, *tag;                     /* Kludge and tagline ptrs */
  int rc=TRUE;                            /* Return code for parent */
  long tlen;                              /* Total length of msg to write */
  char *endbuf = block + QWK_RECSIZE*2;   /* End of fixed-sized buffer */
  char *end1buf;                          /* End of first block in buffer */
  char *endblock = block;                 /* Variable end of currently-read text */
  byte *p=block;                          /* For processing text in the buf */
  unsigned bytes_remain = (msg_blocks-1)*QWK_RECSIZE; /* bytes needing to be read */
  unsigned block_pos = 0;                 /* File pos'n of block[0] */
  int got;                                /* Bytes read from file */

  *pfUpdateStatus=FALSE;  /* We were not just updating status */
  *pkludge=NULL;
#ifdef MAX_TRACKER
  memset(&qti, 0, sizeof qti);
#endif

  /* Fix the fields in the message header */

  QWKFixHeader(msg, qh);

  /* Generate an origin line to stick on the end */
  
  if (mah.ma.attribs & MA_ECHO)
    GenerateOriginLine(orig, &mah);
  else *orig='\0';


  /* The total msg length is the number of blocks in the file, plus         *
   * some 'fudge' room.                                                     */

  tlen=(msg_blocks+1) * QWK_RECSIZE + strlen(orig) + 1;
  

  /* Now create a new message */
  
  if ((msgh=MsgOpenMsg(sq, MOPEN_CREATE, 0L))==NULL)
    return FALSE;

  /* Create the kludge lines for this message, but blank out the origmsgid  *
   * field first, to make sure that no erroneous ^aREPLY kludges are        *
   * generated.                                                             */

  QWKGetMsgid(sq, msg->replyto, reply_kludge);
  *orig_msgid='\0';
  kludge=GenerateMessageKludges(msg, &mah, *reply_kludge ? reply_kludge : NULL);


  /* Repeat while we have bytes left */

  do
  {
    /* Try to fill up buffer while bytes still remain*/

    if (bytes_remain)
    {
      if ((got=read(qfd, (char *)endblock, min(endbuf-endblock, bytes_remain))) > 0)
      {
        endblock += got;
        bytes_remain -= got;
      }
    }

    end1buf = block + QWK_RECSIZE;

    /* While we have bytes to process */

    if (endblock > block)
    {
      int len_p;    /* Length of text being written to qwk file */
      char *end;    /* End of .qwk write buffer */

      /* Strip imbedded NULs out of the uploaded message block */

      for (p=(byte *)block; p < (byte *)endblock; p++)
        if (*p=='\0')
          *p=' ';

      /* If it's the last block, strip all of the !@#$ padding spaces at      *
       * the end of the message.                                              */

      if (!bytes_remain)
        for (p=(byte *)endblock-1; p >= (byte *)block && *p==' '; p--)
          *p='\0';

      /* Make sure that it's nul-terminated. */

      *endblock=0;


      /* Now convert everything to non-hi-bit, and convert PCB CR's to 0x0d. */

      for (p=(byte *)block; *p && p < (byte *)endblock; p++)
      {
        if (*p==0x0d)
          continue;
        else if (*p==QWK_EOL)
          *p=0x0d;
        else if (*p < ' ')
          *p=' ';
        else
        {
          if ((mah.ma.attribs & MA_HIBIT)==0)
            *p=nohibit[*p];
        }
      }



      /* Point 'p' to the beginning of the block, since that's where we'll    *
       * be writing at.                                                       */

      p=block;


      /* If it's the first block in the message, write the _xmsg header */

      if (block_pos==0)
      {
        /* But first, if it's a netmail area, process the destination         *
         * address.                                                           */

        if ((mah.ma.attribs & MA_NET) &&
            tolower(p[0])=='t' && tolower(p[1])=='o' && tolower(p[2])==':')
        {
          /* Jump over the "To:" header */

          p += 3;

          /* Skip past any spaces at the beginning */

          while (p < (byte *)(block+QWK_RECSIZE) && *p==' ')
            p++;

          if (isdigit(*p))
            MaxParseNN(p, &msg->dest);

          while (p < (byte *)(block+QWK_RECSIZE) && *p != '\r')
            p++;

          /* Now skip over any blank lines */

          while (*p=='\r')
            p++;
        }


        /* Now write the initial message header */

        if (MsgWriteMsg(msgh, FALSE, msg, NULL, 0L, tlen,
                        strlen(kludge)+1, kludge) < 0)
        {
          rc=FALSE;
          break;
        }
      }

      /* Search-and-nuke any .QWK tearlines */

      if ((tag=strstr(p, qwk_nuke_tear)) != NULL)
        memmove(tag+1, qwk_replace_tear, 3);

      /* Figure out how long our text is, and figure out where it ends */

      len_p=strlen(p);
      end=p+len_p;

      /* However, write no more than one block of text at a time */

      if (end > end1buf)
      {
        end=end1buf;
        len_p=(byte *)end1buf-p;
      }

#ifdef MAX_TRACKER
      actinfo=p;

      /* Process any actinfo strings in the first buffer */

      while ((actinfo=strstr(actinfo, "^!> ")) != NULL && actinfo < end1buf)
      {
        char *lastr, *nextr;

        /* If we found the "discard" keyword, delete the message after it   *
         * it has been tossed.                                              */

        TrackProcessACInfo(actinfo, &qti);

        *actinfo=0;

        /* Find the beginning of the last line */

        if ((lastr=strrchr(block, '\r')) != NULL)
          lastr++;
        else if (actinfo < block+20 && block_pos==0)
          lastr=block;
        else lastr=actinfo;

        /* Now find the end of this line */

        if ((nextr=strchr(actinfo+1, '\r')) != NULL)
        {
          int len;

          /* Got it.  Now, overlap the entire line containing the ACINFO    *
           * stuff.                                                         */

          len=++nextr-lastr;
          memmove(lastr, nextr, endblock-nextr+1);

          endblock -= len;
          end -= len;
          len_p -= len;
          end1buf -= len;
        }
      }
#endif

      /* Now write the converted hunk of message to the Max msgbase */

      if (MsgWriteMsg(msgh, TRUE, NULL, p, len_p, tlen, 0L, NULL) < 0)
      {
        rc=FALSE;
        break;
      }

      block_pos += end-block;
      memmove(block, end, endblock-end);
      endblock = block + (endblock-end);
    }
  }
  while (*block && (bytes_remain || endblock > block));

  /* Now write the origin line and the terminating NUL */

  MsgWriteMsg(msgh, TRUE, NULL, orig, strlen(orig)+1, tlen, 0L, NULL);
  MsgCloseMsg(msgh);

  /* Fix the reply links for the message to which this is a reply, and      *
   * generate an extra ^aREPLY kludge for our own message, if               *
   * necessary.                                                             */

  QWKFixLinks(sq, MsgGetHighMsg(sq), msg->replyto);


#ifdef MAX_TRACKER
  /* If the message was not to be tossed, delete it, and pretend that       *
   * it was never tossed in the first place, after updating the tracking    *
   * information.                                                           */

  if (TrackQWKUpdateTracking(&qti))
  {
    MsgKillMsg(sq, MsgGetHighMsg(sq));
    *pfUpdateStatus=TRUE;
  }

  vbuf_flush();

#endif

  *pkludge=kludge;

  return rc;
}



/* Toss one single message */

static int near Toss_QWK_Message(struct _qmhdr *qh, XMSG *msg, int qfd, char *block)
{
  char aname[PATHLEN];      /* Name of this area */
  word msg_blocks;          /* Number of QWK blocks written to msg */
  char *kludges=NULL;       /* Kludges written in msg header */
  int fUpdateStatus=FALSE;  /* Only update status (not write reply itself) */
  word tossto;              /* Logical area # into which we should place msg */

  msg_blocks=atoi(qh->len);

  /* Don't toss zero-length msgs */

  if (!msg_blocks)
    return TRUE;


  /* Skip over the current message if it was deleted */

  if (!QWKRetrieveAreaFromPkt(msg, qh, aname, &tossto))
    return FALSE;


  /* Push the current menu area on the stack, so that we can poppush        *
   * stuff later and always be assured that we only need to pop one         *
   * area off the stack to return to initial conditions.                    */

  if (!PushMsgArea(usr.msg, 0))
  {
    AreaError(msgapierr);
    Puts(inval_cur_msg);
    return FALSE;
  }

  if (!QWKGetValidArea(msg, aname, tossto) ||
      !QWKTossMsgBody(msg, qh, msg_blocks, qfd, block, &kludges, &fUpdateStatus))
  {
    if (kludges)
      free(kludges);

    /* Skip over the current message */

    Puts(qwk_msg_skipped);
    PopMsgArea();
    return FALSE;
  }

  /* If we were not just updating the tracking status of an existing msg... */

  if (!fUpdateStatus)
  {
    logit(log_qwk_msg_to, msg->to, MAS(mah, name), (long)MsgGetHighMsg(sq));

    /* If we couldn't write the message (not enough credit), kill it! */

    if (! WroteMessage(&mah, msg, kludges, sq, FALSE))
      MsgKillMsg(sq, MsgGetHighMsg(sq));
  }

  if (kludges)
    free(kludges);

  /* Return back to the original message area */

  PopMsgArea();
  return TRUE;
}


/* This function strips off all trailing spaces from the asciiz string    *
 * 'str'.                                                                 */

static char * near fix_basic_crap(char *str)
{
  char *s=str+strlen(str)-1;
  
  while (s >= str && *s==' ')
    *s--='\0';
  
  return str;
}


static int near all_caps(char *s)
{
  char *p;

  for (p=s; *p; )
    if (islower(*p++))
      return FALSE;

  return TRUE;
}

