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
static char rcs_id[]="$Id: mh_tty.c,v 1.1.1.1 2002/10/01 17:52:33 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Grab msghdr (TTY version)
*/

#define MAX_LANG_max_bor

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <mem.h>
#include "prog.h"
#include "keys.h"
#include "max_msg.h"
#include "mh_tty.h"
#include "node.h"
#include "display.h"

static char *msgname;


int GetTTYHeader(XMSG *msg, PMAH pmah, char *mname, long mn, long highmsg)
{
  int ret, i;

#define TTYQS 6

  static int (near *TTYMsgEntryPtr[TTYQS])(PMAH pmah, XMSG *msg)=
  {
    #define NUM_TTY_HEADFUNCS 5
      
    TTYGetPrivate,
    TTYGetNetmailAddress,
    TTYGetToField,
    TTYGetSubject,
    TTYGetFrom,
    TTYGetAttach
  };
  
  NW(mn);
  NW(highmsg);
  
  msgname=mname;

  DisplayEmptyHeader(pmah);

  /* Display the name of this area */
  
  Printf(WHITE "%s" CYAN "\n\n", MAS(*pmah, descript));

  if ((usr.help != EXPERT) && prm.hlp_hdrentry)
  {
    Puts(GRAY "\n");
    Display_File(DISPLAY_NONE, NULL, PRM(hlp_hdrentry));
    Puts("\n");
  }

  /* And then get all of the information we need */

  for (i=0, ret=0; i < TTYQS && ret==0; i++)
    ret=(*TTYMsgEntryPtr[i])(pmah, msg);
  
  return ret;
}




int near TTYGetPrivate(PMAH pmah, XMSG *msg)
{
  char temp[PATHLEN];
  byte ch;;
  
  sprintf(temp, "%sWHY_PVT", PRM(misc_path));

  if ((pmah->ma.attribs & (MA_PUB|MA_PVT))==(MA_PUB|MA_PVT))
  {
    if (msg->attr & MSGPRIVATE)
      ch=GetYnhAnswer(temp, apvt, 0);
    else ch=GetyNhAnswer(temp, apvt, 0);

    if (ch==YES)
      msg->attr |= MSGPRIVATE;
    else msg->attr &= ~MSGPRIVATE;

    Putc('\n');
  }

  Clear_KBuffer();
  
  return 0;
}




int near TTYGetNetmailAddress(PMAH pmah, XMSG *msg)
{
  NETADDR *d;
  NFIND *nf;
  int len;
  
  d=&msg->dest;

  if (! (pmah->ma.attribs & MA_NET))
    return 0;
  
  /* Ask the user to enter the message attributes */
  
  AskMessageAttrs(msg);

  do
  {
    int fUsedDefault;

    /* If we don't have a destination address, or we're NOT doing a reply,  *
     * then ask for one.  (If we DO have a dest address, but aren't doing   *
     * a reply, it means that we're doing a C)hange, so we should ask       *
     * again, just in case the user wants to modify the destination addr.   */
    
    if (! *netnode || !isareply)
    {
      WhiteN();
      
      if (! *linebuf)          /* "Enter destination addres..." */
      {
        Puts(edmsg);
        Putc('\n');
      }

      InputGets(netnode, naddr); /* Get the address into 'netnode' */
    }

    
    
    
    /* If the user entered a "/" or "#" somewhere in the string, to get a   *
     * listing of network addresses...                                      */
       
    if (*netnode && (netnode[len=strlen(netnode)-1]=='/' ||
        netnode[len]=='#'))
    {
      /* If the user entered something besides the single-character         *
       * "#" and "/" strings...                                             */
         
      if (len > 1)
      {
        /* If the user gave "123/", convert it to "123/#" */
        
        if (netnode[len]=='/')
        {
          strcat(netnode,"#");
          len++;
        }
      
        MaxParseNN(netnode, d);
      }

      Putc('\n');
        
      if (netnode[len]=='/')
        NetList();
      else NodeList(d->zone, d->net);

      Putc('\n');
      
      *netnode='\0';
      continue;
    }

    
    fUsedDefault = FALSE;

    /* If no net/node address was entered, then stick in our own */
    
    if (! *netnode)
    {
      strcpy(netnode, Address(&msg->dest));
      fUsedDefault = TRUE;
    }

            
    /* Convert that address into binary */

    MaxParseNN(netnode, d);

    
    /* And convert it back for display, to make sure that the user knows    *
     * what s/he is getting.                                                */

    strcpy(netnode, Address(d));

    /* Display the address... */
    
    Printf("%s, " GREEN,netnode);

    if ((nf=NodeFindOpen(&msg->dest)) != NULL)
    {
      /* And if found in nodelist, display the name and city */
      
      Printf("%0.34s, %0.20s\n\n", nf->found.name, nf->found.city);
      NodeFindClose(nf);

      break;
    }
    else
    {
      /* Otherwise, say that it was an unlisted node */
      
      Puts(unlisted_node);
      Puts(n_n);

      /* ... and if the user doesn't have a high enough priv to send an     *
       * unlisted message, recycle.                                         */
      
      if (GEPriv(usr.priv, prm.unlisted_priv))
        break;
      else
      {
        /* If we used the default address, but it didn't exist in the
         * nodelist, abort the message so that we don't get stuck in
         * an endless loop.
         */

        if (fUsedDefault)
          return -1;

        *netnode='\0';
      }
    }
  }
  while (*netnode=='\0');  /* Keep going 'till we get a net address */
  
  return 0;
}


int near TTYGetToField(PMAH pmah, XMSG *msg)
{
  char oldto[sizeof(msg->to)];
  
  
  /* Loop until the 'to' field is filled with something */

  if (!isareply)
  {
    strcpy(oldto, msg->to);
    *msg->to='\0';
  }
  
  while (isblstr(msg->to))
  {
    if ((prm.flags & FLAG_no_ulist)==0 && (pmah->ma.attribs & MA_NET)==0)
      Puts(bed_lu);  /* "Type `?' by itself to list users" */

    /* If we're doing a change message */
    
    if (*oldto && !isareply)
      Printf(keep_to, oldto);

    Puts(eto);       /* "    To: " */

    /* Get the addressee's name */

    InputGetsNH(msg->to, min(34, (TermWidth()-current_col)+1), NULL);


    if (*msg->to=='\0' && *oldto && !isareply)
    {
      strcpy(msg->to, oldto);
      break;
    }
    
    /* If the user wants a listing of users, give it to 'im (as         *
     * long as we're allowed to, according to the control file.         */

    if (eqstr(msg->to, qmark))
    {
      char temp[PATHLEN];

      /* Try 'misc\userlist' first, since it might simply contain
         help, menu, other information or even [menu_cmd userlist].. */

      sprintf(temp, "%suserlist", PRM(misc_path));
      if (Display_File(DISPLAY_NONE, NULL, temp)!=0 && !(prm.flags & FLAG_no_ulist))
      {
        Putc('\n');
        UserList();
        Putc('\n');
      }
      *msg->to='\0';
      continue;
    }

    if (isblstr(msg->to))
      return -1;

    
    
    /* If this message is addressed to the sysop (and it's not in a     *
     * netmail area), then convert it to the real sysop name.           */

    if ((pmah->ma.attribs & MA_NET)==0 && eqstri(msg->to, sysop_txt))
    {
      Printf(mroute_sysop, PRM(sysop));
      strcpy(msg->to, PRM(sysop));
    }
    else
    {
      /* Otherwise, if it's a local message area, and the message is    *
       * private, make sure that the addressee exists.                  */

      if (*msg->to &&
          (pmah->ma.attribs & (MA_NET | MA_SHARED))==0 &&
          (msg->attr & MSGPRIVATE) &&
          !IsInUserList(msg->to, TRUE))
      {
        /* Clear the 'To' field, so that we come back for another     *
         * iteration.                                                 */

        *msg->to='\0';
        Puts(userdoesntexist);
        Putc('\n');
      }
    }   /* ! eqstri(msg->to,"sysop") */
  }     /* isblstr(msg->to) */
  
  return 0;
}



int near TTYGetSubject(PMAH pmah, XMSG *msg)
{
  char *rdbox, temp[PATHLEN];

  /* Skip the subject field if it is for a local attach */

  if ((pmah->ma.attribs & MA_NET) &&
      (msg->attr & MSGFILE) &&
      AllowAttribute(pmah, MSGKEY_LATTACH) && !local)
  {
    return 0;
  }

  NW(pmah);
  
  rdbox = (pmah->ma.attribs & MA_NET && msg->attr & (MSGFILE | MSGFRQ | MSGURQ))
          ? files_colon
          : subj_colon;

  Printf(CYAN "%s " YELLOW, rdbox);

  if (! *msg->subj)
    InputGetsNH(msg->subj, min(70, (TermWidth()-current_col) + 1), NULL);
  else
  {
    Printf("%s\n" CYAN, msg->subj);

    Puts(keep_subj);
    Puts(new_subj);

    InputGetsNH(temp, min(70, (TermWidth()-current_col) + 1), NULL);

    if (*temp)
      strcpy(msg->subj, temp);
  }
  
  return 0;
}



int near TTYGetFrom(PMAH pmah, XMSG *msg)
{
  char szOldName[36];

  if (! (pmah->ma.attribs & MA_ANON))
    return 0;
  
  strcpy(szOldName, msg->from);

  Puts(pefon);
  Puts(n_from);
  InputGetsNH(msg->from, min(34, (TermWidth()-current_col) + 1), NULL);

  if (! *msg->from)
    strcpy(msg->from, szOldName);
  
  return 0;
}





static void near AskMessageAttrs(XMSG *msg)
{
  int i;

  /* x != 0, since we don't want to mess with MSGPRIVATE! */
  
  for (i=1; i < 16; i++)
  {
    if (GEPriv(usr.priv, prm.msg_assume[i]) ||
        (GEPriv(usr.priv, prm.msg_ask[i]) &&
         GetyNAnswer(s_ret(n_msg_attr0+i), 0)==YES))
    {
      msg->attr |= (1 << i);
    }
    else msg->attr &= ~(1 << i);
  }

  msg->attr |= MSGLOCAL;
}

static void near DisplayEmptyHeader(PMAH pmah)
{
  /* If the user doesn't has graphics, then clear the screen appropriately, *
   * and put the cursor in the right place.                                 */
  
  if (usr.bits2 & BITS2_CLS)
    Puts(CLS);
  else if (usr.video)
  {
    Puts(avt_home);

    Printf("\x16\x19\x03"   "\n" CLEOL   "%c",
           TermLength() - !!(prm.flags & FLAG_statusline));
  }
  
  Puts("\x16\x08\x03\x01" CYAN);

  /* "This will be a..." */
  
  Puts(this_be);

  if (pmah->ma.attribs & MA_PUB)
    Puts(sp_public);  /* "...public..." */
  else if (pmah->ma.attribs & MA_PVT)
    Puts(sp_private); /* "...private..." */
  else if (pmah->ma.attribs & MA_ECHO)
    Puts(pl_n); /* "This will be an... */

  if (pmah->ma.attribs & MA_ECHO)
    Puts(th_e); /* "... echomail message" */

  /* ...in area #xx. */
  Printf(this_area, msgname);
}

int near TTYGetAttach(PMAH pmah, XMSG *msg)
{
  if (!(msg->attr & MSGFILE) &&
      AllowAttribute(pmah, MSGKEY_LATTACH) &&
      (pmah->ma.attribs & MA_ATTACH) &&
      GetyNAnswer(msg_with_attach,0)==YES)
  {
    msg->attr |= MSGFILE;
  }

  return 0;
}

