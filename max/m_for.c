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
static char rcs_id[]="$Id: m_for.c,v 1.1.1.1 2002/10/01 17:52:41 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: F)orward command
*/

#define MAX_LANG_max_main

#include <stdio.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "m_for.h"



static int near Fwd_Get_Parms(struct _fwdp *fp);
static int near Read_Bomb_File(XMSG *msg);
static int _stdc near mprintf(HMSG sq,char *fmt,...);
static int near mputs(HMSG sq,char *txt,int len);




static int near Read_Bomb_File(XMSG *msg)
{
  char temp[PATHLEN];
  char word[PATHLEN];
  int wn;

  static FILE *bombfile=NULL;

  if (bombfile==NULL)
  {
    Putc('\n');

    InputGets(temp, get_route_file);

    if (! *temp)
      return FALSE;

    if ((bombfile=shfopen(temp, fopen_read, O_RDONLY))==NULL)
    {
      Printf(iseenoxhere,temp);
      return FALSE;
    }
  }

  while (fgets(temp, PATHLEN-1, bombfile))
  {
    Trim_Line(temp);

    if (! *temp)
      continue;
    
    *netnode='\0';
    *msg->to='\0';
    
    getword(temp, word, cmd_delim, wn=1);

    while (*word)
    {
      if (! *msg->to)
      {
        strcpy(msg->to, fancier_str(Strip_Underscore(word)));

        if (mah.ma.attribs & MA_NET)
          Get_FidoList_Name(msg, netnode, PRM(fidouser));
      }
      else if (isdigit(*word))
      {
        strcpy(netnode, word);
        
        MaxParseNN(netnode, &msg->dest);

        strcpy(netnode, Address(&msg->dest));
      }
      else if (*word=='-')
      {
        word[1]=(char)toupper(word[1]);
        
        switch (word[1])
        {
          case 'C':   msg->attr |= MSGCRASH;            break;
          case 'D':   msg->attr |= (MSGCRASH|MSGHOLD);  break;
          case 'H':   msg->attr |= MSGHOLD;             break;
          case 'N':                                     break;
          default:    Printf(huh, word);                break;
        }
      }
      else Printf(huh, word);

      getword(temp, word, cmd_delim, ++wn);
    }
    

    /* Got it! */
    
    if (*msg->to)
      return TRUE;
  }

  fclose(bombfile);
  bombfile=NULL;
  return FALSE;
}



static int _stdc near mprintf(HMSG sq,char *fmt,...)
{
  char buf[256];
  va_list var_args;
  
  va_start(var_args,fmt);
  vsprintf(buf,fmt,var_args);
  va_end(var_args);

  return (mputs(sq, buf, strlen(buf)));
}



static int near mputs(HMSG sq,char *txt,int len)
{
  return (MsgWriteMsg(sq, TRUE, NULL, txt, len, 0L, 0L, NULL));
}



static int near Fwd_MsgIsOkay(struct _fwdp *f)
{
  if ((f->fh=MsgOpenMsg(sq, MOPEN_READ, f->msgnum))==NULL ||
      MsgReadMsg(f->fh, &f->fmsg, 0L, 0L, NULL, 0L, NULL)==-1 ||
      !CanSeeMsg(&f->fmsg))
  {
    if (f->fh)
      MsgCloseMsg(f->fh);
    
    return -1;
  }
  
  return 0;
}

static int near Fwd_Get_Parms(struct _fwdp *f)
{
  char temp[PATHLEN];
  char *p;
  
  f->bomb=f->kill=FALSE;
  *temp='\0';
  
  RipClear();

  do
  {
    WhiteN();
    
    if (! *temp)
      InputGets(temp, fwd_which);

    if (*temp=='\0')
      return -1;

    
    p=temp;
    
    while (isalpha(*p))
    {
      *p=(char)toupper(*p);
      
      if (*p=='B')
        f->bomb=TRUE;
      else if (*p=='K')
        f->kill=TRUE;
      
      p++;
    }

    strocpy(temp, p);
  }
  while (! *p);
  

  /* Only local users, or remote users with a priv greater than            *
   * msg_fromfile can do bombing runs, since it gives them access to       *
   * our HD.                                                               */

  if (f->bomb && !(GEPriv(realpriv(), prm.msg_fromfile) && local))
  {
    Puts(you_dont_have_access);
    return -1;
  }
 
  if (eqstri(p, eq))
    f->msgnum=last_msg;
  else
  {
    f->msgnum=atol(temp);

    /* If we need to convert between UMSGIDs and message numbers */

    if (prm.flags2 & FLAG2_UMSGID)
      f->msgnum=MsgUidToMsgn(sq, f->msgnum, UID_EXACT);
  }


  
  /* Now store the umsgid of the original message */

  f->original=MsgMsgnToUid(sq, f->msgnum);

  if (f->msgnum==0)
    return -1;

  return 0;
}


static int near Fwd_Get_Area(struct _fwdp *f)
{
  char temp[PATHLEN];
  BARINFO bi;
  
  do
  {
    InputGets(temp, where_to_fwd);
    
    switch(*temp)
    {
      case '?':
        ListMsgAreas(NULL, FALSE, FALSE);
        break;
        
      default:
        SetAreaName(f->toname, temp);

        if (eqstri(f->toname, usr.msg))
        {
      case '\0':
      case '=':
          CopyMsgArea(&f->toar, &mah);
          strcpy(f->toname, usr.msg);
          f->tosq=sq;
          break;
        }
        
        memset(&f->toar, 0, sizeof f->toar);

        if (!ReadMsgArea(ham, f->toname, &f->toar) ||
            !ValidMsgArea(NULL, &f->toar, VA_VAL | VA_PWD, &bi) ||
            (f->tosq=MaxOpenArea(&f->toar))==NULL)
        {
          AreaError(msgapierr);

          if (f->toar.heap)
            DisposeMah(&f->toar);

          return -1;
        }
        break;
    }
  }
  while (*temp=='?');
  
  return 0;
}



  
static void near Fwd_Header(struct _fwdp *f,HMSG th)
{
  /* Only insert ORIG:AREA line if forwarding to new area */

  if (f->tosq != sq)
    mprintf(th, org_area, *MAS(mah, echo_tag) ? MAS(mah, echo_tag)
                             : MAS(mah, name));
  
  /* Only insert ORIG:FROM line if we've changed the from line */

  if (!eqstri(f->fmsg.from, f->tmsg.from))
    mprintf(th, orig_from, f->fmsg.from, Address(&f->fmsg.orig));

  mprintf(th, orig_to, f->fmsg.to, Address(&f->fmsg.dest));

  /* Only insert ORIG:SUBJ line if subject is different */

  if (!eqstri(f->fmsg.subj, f->tmsg.subj))
    mprintf(th, orig_subj, f->fmsg.subj);

  mprintf(th, "\r");
}
  


static void near Forward_Body(struct _fwdp *f,HMSG th,char *buf)
{
  char *found;
  char *p;
  long ofs;
  word got;
  int breakout=FALSE;
  
  for (ofs=0;
       (got=(word)MsgReadMsg(f->fh, NULL, ofs, FWDBUFSIZE, buf, 0L,
                             NULL)) > 0 &&
       !breakout;
      )
  {
    if (got==FWDBUFSIZE)
      got -= FWD_OVERLAP;

    ofs += got;

    /* Make sure that we don't copy past the traling NUL */

    if (got > strlen(buf))
    {
      got=strlen(buf)+1;
      breakout=TRUE;
    }
    
    /* Strip trailing NULs from buffer */
    
    while (got && buf[got-1]=='\0')
      got--;

    /* Make sure it has at LEAST one nul. */

    buf[FWDBUFSIZE]='\0';

    if ((f->toar.ma.attribs & MA_SHARED)==0)
      mputs(th, buf, got);
    else
    {
      p=buf;
      
      for (p=buf;
           (p=stristr(p, "---")) != NULL && p < buf+FWDBUFSIZE-FWD_OVERLAP;
           p=found+1)
      {
        found=p;
        
        /* Make sure that the tear line is at the beginning of a line */
        
        if (p != buf && p[-1] != '\r' && p[-1] != '\n')
          continue;

        /* Now find the beginning of the next line */
        
        if ((p=strchr(p, '\r'))==NULL)
          continue;
        
        while (*p=='\r' || *p=='\n' || *p=='\x8d')
          p++;
        
        if (p[0]==' ' && p[2]==' ' && p[3]=='O' && p[4]=='r' && p[5]=='i' &&
            p[6]=='g' && p[7]=='i' && p[8]=='n' && p[9]==':')
        {
          mputs(th, buf, found-1-buf);
          return;
        }
        
      } /* for */

      mputs(th, buf, got);

    } /* echomail */
  }
}



static void near Forward_Add_Trailer(struct _fwdp *f,HMSG th)
{
  char temp[MAX_OTEAR_LEN];
  
  if (f->toar.ma.attribs & MA_ECHO)
  {
    GenerateOriginLine(temp, &f->toar);
    mputs(th, temp, strlen(temp));
  }
}





static void near Forward_One(struct _fwdp *f,struct _fwdp *fp)
{
  UMSGID to_id;
  static SCOMBO last_fwd;
  SCOMBO now;
  HMSG th;
  char *buf;
  byte *kludge;
  int zero;

  Get_Dos_Date(&now);

  /* Make sure that we don't spit out msgs with identical dates */

  while (now.ldate==last_fwd.ldate ||
         now.ldate==((SCOMBO *)&f->tmsg.date_written)->ldate)
  {
    Get_Dos_Date(&now);
    Delay(25);
  }

  last_fwd=now;
  f->tmsg.date_written=*(struct _stamp *)&now;

  
  if (CheckCredit(&f->tmsg.dest, &f->toar)==-1)
    return;

  if ((th=MsgOpenMsg(f->tosq, MOPEN_CREATE, 0L))==NULL)
    return;
  
  if ((buf=malloc(FWDBUFSIZE+20))==NULL)
  {
    MsgCloseMsg(th);
    return;
  }

  buf[FWDBUFSIZE]='\0';
  
  /* Now grab the kludges to put in the message */
  
  kludge=GenerateMessageKludges(&f->tmsg, &mah, NULL);
  
  MsgWriteMsg(th, FALSE, &f->tmsg, NULL, 0L, MsgGetTextLen(f->fh)+FWD_SAFE,
              kludge ? strlen(kludge)+1 : 0L, kludge);
  
  Fwd_Header(f, th);
  Forward_Body(f, th, buf);
  Forward_Add_Trailer(f, th);

  /* Set tosslog flags et al.  Used instead of below. */

  WroteMessage(&f->toar, &f->tmsg, kludge, f->tosq, FALSE);

  if (kludge)
    free(kludge);

#ifdef NEVER
  if ((f->toar.attrib[cls] & SYSMAIL) && ! MsgToUs(&f->tmsg.dest))
  {
    if (!f->bomb && !fp)
      Putc('\n');

    Handle_Matrix_Charges(&f->tmsg.dest, f->bomb==FALSE);
  }
#endif

  /* Add the terminating NUL */

  zero=0;
  MsgWriteMsg(th, TRUE, NULL, (char *)&zero, 1L, 0L, 0L, NULL);

  free(buf);
  MsgCloseMsg(th);

  to_id=UIDnum(MsgHighMsg(f->tosq));
  logit(log_fwd, f->tmsg.to, f->toname, to_id);

  if (!f->bomb && !fp)
    Puts("\n\n");

  Printf(fwd_to, f->tmsg.to, Address(&f->tmsg.dest), to_id);

  vbuf_flush();
}


static void near Fwd_Message(struct _fwdp *f, struct _fwdp *fp)
{
  if (!fp)
  {
    Blank_Msg(&f->tmsg);
    /* Make sure we use the orig addr from the dest area, not this one! */
    f->tmsg.orig=f->toar.ma.primary;
    strcpy(f->tmsg.subj, f->fmsg.subj);
    *netnode='\0';
  }

  f->tmsg.attr &= ~(MSGSCANNED | MSGREAD | MSGSENT);
  f->tmsg.attr |= MSGLOCAL;
  
  if (f->bomb)
  {
    while (Read_Bomb_File(&f->tmsg))
    {
      if (f->fmsg.attr & MSGPRIVATE)
        f->tmsg.attr |= MSGPRIVATE;

      Forward_One(f,fp);
    }
  }
  else
  {
    if (f->fmsg.attr & MSGPRIVATE)
      f->tmsg.attr |= MSGPRIVATE;
    
    if (!fp)
      FixPrivateStatus(&f->tmsg, &f->toar);

    if (fp || GetMsgAttr(&f->tmsg,
                         &f->toar,
                         f->toname,
                         MsgGetHighMsg(f->tosq)+1,
                         MsgGetHighMsg(f->tosq)) != -1)
    {
      Forward_One(f,fp);
    }
    else Puts(CLS);
  }
}


static void near FwdCleanup(struct _fwdp *f)
{
  MsgCloseMsg(f->fh);
  
  /* If we're supposed to delete the original message, do so now. */

  if (f->kill)
    MsgKillMsg(sq, MsgUidToMsgn(sq, f->original, UID_EXACT));
}




void Msg_Forward(struct _fwdp *fp)
{
  struct _fwdp *fwdp;
  UMSGID uid;
  dword mn;
  
  /* If caller provided a fwdp structure, then use that.  Otherwise,        *
   * allocate a blank one of our own.                                       */

  if (fp)
    fwdp=fp;
  else
  {
    if ((fwdp=malloc(sizeof(struct _fwdp)))==NULL)
      return;

    memset(fwdp, 0, sizeof *fwdp);
  }
  
  /* Save the current message number */

  uid=MsgMsgnToUid(sq, last_msg);

  /* Unless we were alreaady given a fwdp structure, find out which message *
   * number to forward.  Then check to make sure that the message is okay   *
   * (has a priv level high enough to see it and so on), and get the        *
   * area name (unless we were already given a fwdp structure).             */

  if ((fp || Fwd_Get_Parms(fwdp)==0) && Fwd_MsgIsOkay(fwdp)==0 &&
      (fp || Fwd_Get_Area(fwdp)==0))
  {
    if (!AreaIsReadOnly(&fwdp->toar))
    {
      /* Finally, do the forward */

      Fwd_Message(fwdp,fp);
    }

     /* If the MSG* provided isn't the default MSG*, then close it */

    if (fwdp->tosq && fwdp->tosq != sq)
      MsgCloseArea(fwdp->tosq);

    FwdCleanup(fwdp);
  }

  DisposeMah(&fwdp->toar);

  /* If we allocated this before, then free it. */

  if (!fp)
    free(fwdp);

  if ((mn=MsgUidToMsgn(sq, uid, UID_PREV)) != 0)
    last_msg=mn;
}

