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

/* $Id: shell.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

#pragma library("../msgapi.lib");

#define TEST_VER
#define INITSQUISH
#define ERL_ERROR 1

#define MSGAPI_HANDLERS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "msgapi.h"
#include "api_Sq.h"

#define RDLEN 8192


int chase(HAREA ha, int sfd, int ifd, FOFS begin_frame, FOFS last_frame, FOFS end_frame, int num_msg)
{
  FOFS fo, foLast;
  SQHDR sqh;
  SQIDX sqi;
  int msgnum=0;

  fo=begin_frame;
  foLast=NULL_FRAME;

  printf("scanning the %s list\n", ifd==-1 ? "free" : "message");

  while (fo)
  {
    lseek(sfd, fo, SEEK_SET);
    printf("frame %d:\n", msgnum+1);

    if (fo >= end_frame)
      printf("\a%d: frame too large (%#lx >= %#lx)\n", msgnum+1,
             fo, end_frame);

    if (read(sfd, &sqh, sizeof sqh) != sizeof sqh)
    {
      printf("\aread err %d\n", msgnum+1);
      exit(1);
    }

    if (sqh.prev_frame != foLast)
    {
      printf("\aframe %d: mismatch prev_frame (struct=%#lx, real=%#lx)\n",
             msgnum+1, sqh.prev_frame, foLast);
    }

    lseek(ifd, (long)msgnum * (long)sizeof(SQIDX), SEEK_SET);

    if (ifd != -1)
    {
      /* Validate the current in-memory frame pointers */

      if (msgnum+1==ha->cur_msg)
      {
        printf("Validating current links:\n");

        if (Sqd->foPrev != sqh.prev_frame)
          printf("\aCurrent prev_frame is wrong! (is=%#lx, sb=%#lx)\n",
                 Sqd->foPrev, sqh.prev_frame);

        if (Sqd->foCur != fo)
          printf("\aCUrrent this_frame is wrong! (is=%#lx, sb=%#lx)\n",
                 Sqd->foCur, fo);

        if (Sqd->foNext != sqh.next_frame)
          printf("\aCurrent next_frame is wrong! (is=%#lx, sb=%#lx)\n",
                 Sqd->foNext, sqh.next_frame);
      }

      if (sqh.frame_type==FRAME_FREE)
        printf("\a%d: free frame in normal list!\n", msgnum+1);

      if (read(ifd, &sqi, sizeof sqi) != sizeof sqi)
        printf("\aidx read err %d\n", msgnum+1);

      if (sqi.ofs != fo)
      {
        printf("\aidx mismatch %d: idx=%#lx, real=%#lx\n",
               msgnum+1, sqi.ofs, fo);
      }
    }
    else
    {
      if (sqh.frame_type != FRAME_FREE)
        printf("\a%d: normal frame in free list!\n", msgnum+1);
    }

    foLast=fo;
    fo=sqh.next_frame;
    msgnum++;
  }

  if (foLast != last_frame)
  {
    printf("\agot end frame at %d (%#lx), but real end frame should be "
           "at %#lx\n",
           msgnum, foLast, last_frame);
  }

  if (num_msg != -1 && msgnum != num_msg)
  {
    printf("\aexpecting %d msgs in chain but found only %d\n",
           num_msg, msgnum);
  }

  return 0;
}

static void near verify(char *base, HAREA ha)
{
  static char temp[120];
  static SQBASE sqb;

  int sfd;
  int ifd;

  sprintf(temp, "%s.sqd", base);

  if ((sfd=sopen(temp, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
  {
    printf("err 1\n");
    exit(1);
  }

  sprintf(temp, "%s.sqi", base);

  if ((ifd=sopen(temp, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
  {
    printf("err 2\n");
    exit(1);
  }

  read(sfd, (char *)&sqb, sizeof sqb);
  printf("num_msg=%ld\n", sqb.num_msg);
  printf("high_msg=%ld\n", sqb.high_msg);
  printf("skip_msg=%ld\n", sqb.skip_msg);
  printf("high_water=%ld\n", sqb.high_water);
  printf("uid=%ld\n", sqb.uid);
  printf("begin_frame=%#lx\n", sqb.begin_frame);
  printf("last_frame=%#lx\n", sqb.last_frame);
  printf("free_frame=%#lx\n", sqb.free_frame);
  printf("last_free_frame=%#lx\n", sqb.last_free_frame);
  printf("end_frame=%#lx\n", sqb.end_frame);



  chase(ha, sfd, ifd, sqb.begin_frame, sqb.last_frame, sqb.end_frame, sqb.num_msg);
  chase(ha, sfd, -1, sqb.free_frame, sqb.last_free_frame, sqb.end_frame, -1);

  close(ifd);
  close(sfd);
}


int main(int argc, char *argv[])
{
  MSGH *msgh;
  long tl;

  char ctxt[]="\x01""MSGID: 1:249/106 12345678\x01""REPLY: 1:249/106.1 87654321\x01";
  char lastfrom[36]="";
  static char temp[100];
  static char ctext[1000];
  char *p;

  MSG *sq;

  XMSG msg,
       msg2,
       rmsg;

  char *rd;
  char *name=argv[1];
  long num;

  if (argc < 2)
  {
    printf("usage: shell <base>\n");
    return ERL_ERROR;
  }

  if ((rd=malloc(RDLEN))==NULL)
  {
    printf("no memory.\n");
    exit(ERL_ERROR);
  }

  memset(&msg,'\0',sizeof(XMSG));
  memset(&msg2,'\0',sizeof(XMSG));

  Get_Dos_Date((union stamp_combo *)&msg.date_arrived);
  Get_Dos_Date((union stamp_combo *)&msg.date_written);

  strcpy(msg.from,"Scott Dudley");
  strcpy(msg.to,"Test");
  strcpy(msg.subj,"This is a test!");

  strcpy(msg2.from,"Maximus-CBCS");
  strcpy(msg2.to,"Fido");
  strcpy(msg2.subj,"Bad dog!");

  msg.attr=MSGLOCAL | MSGPRIVATE;
  msg2.attr=MSGLOCAL | MSGPRIVATE;

  if ((sq=MsgOpenArea(*name=='!' ? name+1 : name,
                      MSGAREA_NORMAL,
                      *name=='!' ? MSGTYPE_SDM : MSGTYPE_SQUISH))==NULL)
  {
    printf("\aError opening SquishFile!\n");

    printf("Create? ");

    fgets(temp, 100, stdin);

    if (*temp=='Y' || *temp=='y')
    {
      if ((sq=MsgOpenArea(*name=='!' ? name+1 : name,
                      MSGAREA_CRIFNEC,
                      *name=='!' ? MSGTYPE_SDM : MSGTYPE_SQUISH))==NULL)
      {
        printf("\aError opening SquishFile!\n");
        exit(ERL_ERROR);
      }
    }
    else exit(ERL_ERROR);
  }

  for (;;)
  {
    printf("SQ> ");

    fgets(temp,100,stdin);

    switch (tolower(*temp))
    {
      case '?':
        printf("re - reply to msg\n"
               "rn - read next msg\n"
               "rp - read previous msg\n"
               "## - read msg number ##\n"
               "w  - write msg\n"
               "x  - write msg, but trap in middle\n"
               "t  - trap\n"
               "n  - no private bit\n"
               "z# - set high water mark to #\n"
               "v  - verify squish base integrity\n"
               "s  - statistics\n"
               "k# - kill msg number #\n"
               "b+ - lock area\n"
               "b- - unlock area\n"
               "q  - quit\n"
               "l  - list msgs\n");
        break;

      case 'b':
        if (temp[1]=='+')
          printf("Area locked %ssuccessfully\n",
                 MsgLock(sq)==0 ? "" : "un");
        else
          printf("Area unlocked %ssuccessfully\n",
                 MsgUnlock(sq)==0 ? "" : "un");
        break;

      case 'r':
        if (temp[1]=='e')
          goto ReplyMsg;
        else if (temp[1]=='n')
          tl=MSGNUM_next;
        else tl=MSGNUM_previous;

        if ((msgh=MsgOpenMsg(sq,MOPEN_READ,tl))==NULL)
        {
          printf("Not found.\n");
          break;
        }

ReadText:
        if (MsgReadMsg(msgh,&rmsg,0L,RDLEN,rd,999L,ctext)==-1)
          printf("Error reading message.\n");
        else
        {
          printf("From: %-20s (%u:%d/%d.%u)  Date: %s Private: %s\n"
                 "  To: %-20s (%u:%d/%d.%u)\n"
                 "Subj: %s\n"
                 "This: %ld (uid=%ld idx, %ld hdr), Down: %ld (uid=%ld), Up: %ld (uid=%ld)\n\n",
                 rmsg.from,
                 rmsg.orig.zone,
                 rmsg.orig.net,
                 rmsg.orig.node,
                 rmsg.orig.point,
                 sc_time((union stamp_combo *)&rmsg.date_written,temp),
                 (rmsg.attr & MSGPRIVATE) ? "Yes" : "No",
                 rmsg.to,
                 rmsg.dest.zone,
                 rmsg.dest.net,
                 rmsg.dest.node,
                 rmsg.dest.point,
                 rmsg.subj,
                 MsgCurMsg(sq),
                 MsgMsgnToUid(sq,MsgCurMsg(sq)),
                 msgh->uidUs,
                 MsgUidToMsgn(sq,rmsg.replyto,UID_EXACT),
                 rmsg.replyto,
                 MsgUidToMsgn(sq,rmsg.replies[0],UID_EXACT),
                 rmsg.replies[0]);

          if (msgh->uidUs != MsgMsgnToUid(sq, MsgCurMsg(sq)))
            printf("\auidUs is not same as idx!\n");

          strcpy(lastfrom,rmsg.from);
          p=rd;

          /* Remove all \n's and soft CR's */
          while ((p=strpbrk(p,"\n\x8d")) != NULL)
            memmove(p,p+1,strlen(p+1)+1);

          p=rd;

          /* Replace all returns with CR/LFs. */
          while ((p=strchr(p,'\r')) != NULL)
          {
            memmove(p+2,p+1,strlen(p+1)+1);
            p[1]='\n';

            p += 2;
          }

          fprintf(stdout, "%s", rd);

          printf("\n");
          
          printf("ctext:\n---%s---\n",ctext);
        }

        MsgCloseMsg(msgh);
        break;

ReplyMsg:
        msg.replyto=MsgMsgnToUid(sq,MsgCurMsg(sq));
        strcpy(msg.to,lastfrom);
        /* fall-thru */

      case 'w':
        num=atol(temp+1);
        printf("\nTEXT> ");

        fgets(temp,499,stdin);

        if ((msgh=MsgOpenMsg(sq, MOPEN_CREATE, num))==NULL)
          printf("Can't open message!\n");
        else
        {
          if (MsgWriteMsg(msgh,FALSE,&msg,temp,strlen(temp)+1,
                          strlen(temp)+1, strlen(ctxt)+1, ctxt)==-1)
            printf("Can't write message!\n");
          else printf("Wrote msg#%d\n",MsgHighMsg(sq));

          MsgCloseMsg(msgh);
        }
       break;

      case 'x': /* write msg, but trap in middle */

        num=atol(temp+1);
        printf("\nTEXT> ");

        fgets(temp,499,stdin);

        if ((msgh=MsgOpenMsg(sq, MOPEN_CREATE, num))==NULL)
          printf("Can't open message!\n");
        else
        {
          *(char far *)0=0;

          if (MsgWriteMsg(msgh,FALSE,&msg,temp,strlen(temp)+1,
                          strlen(temp)+1, strlen(ctxt)+1, ctxt)==-1)
            printf("Can't write message!\n");
          else printf("Wrote msg#%d\n",MsgHighMsg(sq));

          MsgCloseMsg(msgh);
        }
       break;


      case 't': /* trap */
        *(char far *)0=0;
        break;

      case 'n':  /* no private bit */

        if ((msgh=MsgOpenMsg(sq,MOPEN_RW,MsgCurMsg(sq)))==NULL)
          printf("Can't open message!\n");
        else
        {
           if (MsgReadMsg(msgh,&rmsg,0L,0L,NULL,0L,NULL) != -1)
           {
             rmsg.attr &= ~(MSGPRIVATE|MSGREAD);

             if (MsgWriteMsg(msgh,FALSE,&rmsg,NULL,0L,0L,0L,NULL)==-1)
               printf("Can't write message!\n");
             else printf("Done.\n",MsgHighMsg(sq));
           }

          MsgCloseMsg(msgh);
        }
       break;

      case 'z':
        MsgSetHighWater(sq, atol(temp+1));
        printf("new hwm is %ld\n", MsgGetHighWater(sq));
        break;

      case 'v':
        verify(argv[1], sq);
        break;

      case 's':
        printf("Curmsg=%ld; nummsg=%ld; hwm=%ld\n", MsgGetCurMsg(sq),
               MsgGetNumMsg(sq), MsgGetHighWater(sq));
        break;

      case 'k':
        printf("Message #%d was %skilled okay.\n",atoi(temp+1),
               MsgKillMsg(sq,atoi(temp+1))==0 ? "" : "NOT ");
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if ((msgh=MsgOpenMsg(sq,MOPEN_READ,atol(temp)))==NULL)
        {
          printf("Goto was unsuccessful.\n");
          break;
        }
        goto ReadText;

      case 'q':
        goto done;

      case 'l':
        while ((msgh=MsgOpenMsg(sq,MOPEN_READ,MSGNUM_next)) != NULL)
        {
          if (MsgReadMsg(msgh,&msg,0L,0L,NULL,0L,NULL)==-1)
            printf("Error reading message.\n");
          else printf("#%-3ld Fm: %-15.15s  To: %-15.15s  Subj: %-25.25s\r\n",
                      MsgCurMsg(sq),msg.from,msg.to,msg.subj);

          MsgCloseMsg(msgh);
        }
        break;
    }
  }

done:

  MsgCloseArea(sq);
  free(rd);
  return 0;
}


