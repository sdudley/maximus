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
static char rcs_id[]="$Id: m_header.c,v 1.2 2003/06/04 23:51:24 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Grab message header from user
*/

#define MAX_LANG_m_area

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <ctype.h>
#include <mem.h>
#include "prog.h"
#include "keys.h"
#include "mm.h"
#include "max_msg.h"
#include "m_full.h"
#include "node.h"
#include "userapi.h"

static int near SendWarnings(PMAH pmah);
static int near IsHeaderOkay(XMSG *msg, PMAH pmah);
static int near Parse_Alias_File(XMSG *msg, char *netnode);


/* This function is the main routine that is called by all of the other     *
 * message-entry functions -- It prompts the user for the To/From/Subject,  *
 * and handles things such as netmail credit and others.                    */

int GetMsgAttr(XMSG *msg, PMAH pmah, char *mname, long mn, long highmsg)
{
  int ret;

  /* If users are supposed to put their real names in this conference,      *
   * then do so.                                                            */
     
  if (pmah->ma.attribs & MA_REAL)
  {
    memset(msg->from, 0, sizeof msg->from);
    strcpy(msg->from, usr.name);
  }
  else if ((pmah->ma.attribs & MA_ALIAS) && *usr.alias)
  {
    memset(msg->from, 0, sizeof msg->from);
    strcpy(msg->from, usr.alias);
  }

  /* Clear the keyboard buffer */
  
  Clear_KBuffer();

  /* Warn user about lack of disk space, no time left, and if area is       *
   * read-only.                                                             */
  
  if (SendWarnings(pmah)==-1)
    return -1;
  
  ret=0;

  /* We can accept high bits in the message header */

  in_msghibit++;

  /* Only fix the private flag if we're NOT doing a C)hange message */

  if (mn==0)
    FixPrivateStatus(msg, pmah);

  Puts(enter_header_init);

  /* Now get the main portion of the message header */

  ret=(usr.video ? GetGraphicsHeader : GetTTYHeader)(msg, pmah, mname,
                                                     mn, highmsg);

  in_msghibit--;

  return (ret==-1 ? -1 : IsHeaderOkay(msg, pmah));
}


static int near SendWarnings(PMAH pmah)
{
  /* Make sure that there's (roughly) enough space to save the message */
  
  if ((pmah->ma.type & MSGTYPE_SDM) && zfree(PMAS(pmah, path)) < 10000L)
  {
    Puts(warn_splow);
    Press_ENTER();
  }
  
  /* Warm the user if s/he doesn't have much time left */

  if (timeleft() <= 5)
  {
    Printf(warn_mleft, timeleft());

    if (GetyNAnswer(strt_any, 0)==NO)
      return -1;
  }

  if (AreaIsReadOnly(pmah))
    return -1;

  return 0;
}



static int near IsHeaderOkay(XMSG *msg, PMAH pmah)
{
  char *temp, *st, *p;

  /* The message is invalid if:
   *
   * 1) the "To:" field is blank, or
   * 2) the subject field is blank and we are not attaching a file
   *    using the local file attach mechanism.
   */

  if (! *msg->to)
  {
    return -1;
  }

  if (!*msg->subj &&
      ((pmah->ma.attribs & (MA_NET|MA_ATTACH)) != (MA_NET|MA_ATTACH) ||
       (msg->attr & MSGFILE)==0))
  {
    return -1;
  }

  /* If the user didn't enter a name in the From: field, touch it up        *
   * slightly                                                               */
  
  if ((pmah->ma.attribs & MA_ANON) && isblstr(msg->from))
    strcpy(msg->from, (pmah->ma.attribs & MA_REAL) ? (char *)usr.name :
           ((pmah->ma.attribs & MA_ALIAS) && *usr.alias) ? (char *)usr.alias : (char *)usrname);

  if (strpbrk(msg->to,"@!")==NULL || strchr(msg->to,' '))
    fancier_str(msg->to);

  if (strpbrk(msg->from,"@!")==NULL || strchr(msg->from,' '))
    fancier_str(msg->from);

  /* Process the "#" and "^" characters (trunc/sent, kill/sent) on
   * the subject line.
   */

  if ((pmah->ma.attribs & MA_ATTACH)==0 && (msg->attr & (MSGFILE | MSGURQ)))
  {
    temp=strdup(msg->subj);

    for (st=strtok(temp, cmd_delim); st; st=strtok(NULL, cmd_delim))
    {
      p=st;

      if (*p=='#' || *p=='^')
        p++;

      if (!fexist(p))
      {
        Printf(a_noex, p);
        Press_ENTER();
      }
    }
  }
  
  if (pmah->ma.attribs & MA_NET)
    return (CheckCredit(&msg->dest, pmah));
  else return 0;
}


/* Returns TRUE if the specified user is in the user list */

int IsInUserList(char *name, int show_us)
{
  struct _usr user;
  HUF huf;
  int rc;

  /* Try to open the user file */

  if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
  {
    cant_open(PRM(user_file));
    return FALSE;
  }

  rc=UserFileFind(huf, name, NULL, &user) ||
     UserFileFind(huf, NULL, name, &user);

  if (!show_us && (eqstri(name, usr.name) || eqstri(name, usr.alias)))
    rc=FALSE;

  UserFileClose(huf);

  return rc;
}


int CheckCredit(NETADDR *dest, PMAH pmah)
{
  NFIND *nf;
  int cost;

  if (! (pmah->ma.attribs & MA_NET))
    return 0;
  
  if ((nf=NodeFindOpen(dest))==NULL)
  {
    if (! GEPriv(usr.priv, prm.unlisted_priv))
    {
      Puts(unlisteddest);
      Press_ENTER();
      return -1;
    }

    cost=prm.unlisted_cost;
  }
  else
  {
    cost=nf->found.cost;
    NodeFindClose(nf);
  }

  if (mailflag(CFLAGM_NETFREE))
    cost=0;

  if (usr.debit+cost > usr.credit)
  {
    Puts(ncredit);
    Press_ENTER();
    return -1;
  }

  return 0;
}


int Get_FidoList_Name(XMSG *msg, char *netnode, char *fidouser)
{
  NETADDR *d;
  FILE *ulfile;

  char line[120];
  char name[60], *p;

  long lo, hi, last, tryit;
  int linelen, comp;

  /* If no "To:" field, always return FALSE */

  if (isblstr(msg->to))
    return FALSE;

  if (Parse_Alias_File(msg, netnode))
    return TRUE;
  
  /* Handle version7 name lookups */

  if (prm.nlver==NLVER_7)
  {
    struct _newnode node;
    NETADDR dest;
    
    if (V7FindName(msg->to, &dest, &node, PRM(net_info)))
    {
      strcpy(netnode, Address(&dest));
      return TRUE;
    }
  }
  

#if 1 /* Last, First Middle */
  for (p=(char *)(msg->to+strlen(msg->to)-1); p >= (char *)msg->to && *p != ' '; p--)
    ;

#else /* Middle Last, First */
  for (p=msg->to; *p && *p != ' '; p++)
    ;
  
#endif

  /* Try to find it in the FroDo nodelist */

  if (prm.nlver==NLVER_FD)
  {
    NETADDR dest;
    
    /* Convert the name to JoHo format */
 
#if !defined(__GNUC__)
    sprintf(name, "%s %-0.*s", p+1, p-msg->to, msg->to);
#else
    /* What the HELL did scott mean by THAT? -- wes */
    sprintf(name, "%s %s", p+1, (char *)msg->to);
#endif

    name[15]='\0';
    
    while(strlen(name) < 15)
      strcat(name, " ");
 
    if (FDFindName(name, &dest, PRM(net_info)))
    {
      strcpy(netnode, Address(&dest));
      return TRUE;
    }
  }
  
  if (! *fidouser)
    return FALSE;

#if !defined(__GNUC__)
  sprintf(name,
          "%s, %-0.*s",
          p+1,
          p-msg->to,
          msg->to);
  /* WTF? */
#else
  sprintf(name, "%s %s", p+1, msg->to);
#endif
 
  if ((ulfile=shfopen(fidouser, fopen_readb, O_RDONLY | O_BINARY | O_NOINHERIT))==NULL)
    return FALSE;

  /* Find out the length of the first line */

  if (fgets(line, sizeof(line), ulfile) != NULL)
  {
    linelen=strlen(line);

    /* Now find out where the end of the file is */

    fseek(ulfile,0L,SEEK_END);

    if (linelen)
    {
      hi=ftell(ulfile)/linelen;

      /* Now use binary search logic to find the user's name */

      lo=0L;
      last=-1L;

      for (;;)
      {
        tryit=((hi-lo) >> 1)+lo;

        if (last==tryit)
          break;

        last=tryit;

        fseek(ulfile,(long)linelen*tryit,SEEK_SET);

        if (fgets(line,sizeof(line),ulfile)==NULL)
          break;

        comp=strnicmp(name,line,strlen(name));

        if (comp==0)
        {
          /* Got it! */

          p=line+strlen(name);

          /* Skip over the name, until we find a digit (start of z:n/n.p) */

          while (*p && !isdigit(*p))
            p++;

          /* Got a digit, so copy the address in */

          if (isdigit(*p))
          {
            d=&msg->dest;
            MaxParseNN(p, d);
            strcpy(netnode, Address(d));
          }

          break;
        }
        else if (comp < 0)
          hi=tryit;
        else lo=tryit;
      }
    }
  }

  fclose(ulfile);

  return (*netnode != '\0');
}


static int near Parse_Alias_File(XMSG *msg,char *netnode)
{
  FILE *alias;
  char temp[PATHLEN];
  char *tptr;
  char *p;
  int gotit;

#ifndef UNIX
  sprintf(temp, "%sNAMES.MAX", PRM(sys_path));
#else
  sprintf(temp, "%snames.max", PRM(sys_path));
#endif

  if ((alias=shfopen(temp, fopen_read, O_RDONLY | O_NOINHERIT))==NULL)
    return FALSE;

  gotit=FALSE;

  while (fgets(temp, PATHLEN, alias) != NULL)
  {
    tptr=temp;

    Strip_Trailing(tptr, '\n');

    
    /* Skip lines which start with a comment */

    if (*tptr==';')
      continue;

    /* Lines prefixed with a "*" can only be used by sysop */

    if (*tptr=='*')
    {
      if (!mailflag(CFLAGM_ATTRANY))
        continue;

      tptr++;
    }
    

    if ((p=strtok(tptr, comma))==NULL)
      continue;

    if (eqstri(p, "end"))
      break;

    if (eqstri(msg->to,p))
    {
      if ((p=strtok(NULL, comma))==NULL)
        continue;

      strcpy(msg->to, p);

      if ((p=strtok(NULL, comma)) != NULL)
      {
        strcpy(netnode,p);

        if ((p=strtok(NULL, comma)) != NULL)
          strcpy(msg->subj, p);
      }

      gotit=TRUE;
      break;
    }
  }

  fclose(alias);

  return gotit;
}






int AreaIsReadOnly(PMAH pmah)
{
  char temp[PATHLEN];

  /* If this is a read-only area (and the user's priv is NOT greater than   *
   * assistant sysop), then say that the user is S.O.L.                     */
     
  if ((pmah->ma.attribs & MA_READONLY) && !mailflag(CFLAGM_RDONLYOK))
  {
    char *roname="%sreadonly.bbs";
    char *sroname="%s.sqx";
    char *rname=((pmah->ma.type & MSGTYPE_SDM) ? roname : sroname);
    char *path=MAS(*pmah, path);
    
    /* If READONLY.BBS exists in the message area, then display it */
    
    sprintf(temp, rname, path);

    if (fexist(temp) && *path)
      Display_File(0, NULL, rname, path);
    else
    {
      /* Otherwise display a canned message */ 

      Puts(rd_only);
      Press_ENTER();
    }

    return TRUE;
  }
  
  return FALSE;
}

void FixPrivateStatus(XMSG *msg, PMAH pmah)
{
  int i;

  /* Do nothing if the area is read-only.  Otherwise, set/reset the         *
   * private bit accordingly.                                               */

  if ((pmah->ma.attribs & (MA_PUB|MA_PVT))==(MA_PUB|MA_PVT))
    ;
  else if (pmah->ma.attribs & MA_PVT)
    msg->attr |= MSGPRIVATE;
  else if (pmah->ma.attribs & MA_PUB)
    msg->attr &= ~MSGPRIVATE;
  
  /* Now handle all of the default and assumed attributes */
  
  if (pmah->ma.attribs & MA_NET)
    for (i=1; i < 16; i++)
      if (GEPriv(usr.priv, prm.msg_assume[i]))
        msg->attr |= (1 << i);
      else msg->attr &= ~(1 << i);

  msg->attr |= MSGLOCAL;
}


