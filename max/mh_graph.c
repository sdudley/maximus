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
static char rcs_id[]="$Id: mh_graph.c,v 1.1.1.1 2002/10/01 17:52:32 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Grab msghdr (graphics mode version)
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
#include "m_full.h"
#include "mh_graph.h"
#include "node.h"
#include "display.h"

#define GotoLocAttr()       Puts(reader_pos_attr)
#define GotoLocFrom()       Puts(reader_pos_from)
#define GotoAddrFrom()      Printf(reader_pos_from_addr)
#define GotoLocTo()         Puts(reader_pos_to)
#define GotoAddrTo()        Printf(reader_pos_to_addr)
#define GotoLocSubj()       Puts(reader_pos_subj)
#define GotoLocSubjField()  Puts(reader_pos_subj_field)

static int item=0;
static int inc=FALSE;
static int goitem=0;
static long this_msgnum=0L;
static long high_msg=0L;

int GetGraphicsHeader(XMSG *msg, PMAH pmah, char *mname, long mn, long highmsg)
{
  int ret=0;
  
  static int (near *MsgEntryFtab[GRAPHICS_ENTRY_FUNCS])(PMAH pmah, XMSG *)=
  {
    GetAttributes,
    GetFrom,
    GetFromAddr,
    GetToField,
    GetToAddr,
    GetSubject,
  };

  NW(mname);

  this_msgnum=mn;
  high_msg=(highmsg==-1L) ? MsgGetHighMsg(sq) : highmsg;

  RedrawAll(pmah, msg, TRUE);

  /* If we can't enter private messages, then move the cursor down to the   *
   * "To:" line.                                                            */
  
  inc=TRUE;

  goitem = (pmah->ma.attribs & MA_ANON) ? ITEM_FROM : ITEM_TO;

  if (*msg->to)
  {
    item=(goitem==ITEM_FROM ? ITEM_FROM : ITEM_SUBJ);
  }
  else if ((pmah->ma.attribs & (MA_PVT | MA_PUB)) != (MA_PVT | MA_PUB) &&
           (pmah->ma.attribs & MA_ATTACH)==0)
  {
    item=goitem;
  }
  else
  {
    item=ITEM_ATTR;
  }
  
  /* Loop around all of the functions until item==-1 (that sez we're done) */
  
  while (item >= 0 && item < GRAPHICS_ENTRY_FUNCS)
  {
    if ((item==ITEM_ORIG || item==ITEM_DEST) &&
        !(pmah->ma.attribs & MA_NET))
    {
      if (inc)
        item++;
      else item--;
    }
    
    ret=(*MsgEntryFtab[item])(pmah, msg);
  }

  /* Redisplay this to eliminate any junk underneath if FSR is not in use */

  if (ret!=-1 && (usr.bits2 & BITS2_CLS) && !(usr.bits & BITS_FSR))
    RedrawAll(pmah, msg, FALSE);

  /* Clear the last line */

  GotoLocText();
  Puts(CLEOL);
  
  return ret;
}


int near GetAttributes(PMAH pmah, XMSG *msg)
{
  dword bit;
  int ret=0, i;
  byte ch;

  /* If we can't accept anything other than the default in this area,     *
   * skip this field IF and ONLY IF we can't enter any other attributes   *
   * in a matrix-type area.                                               */

  if ((pmah->ma.attribs & (MA_PUB|MA_PVT)) != (MA_PUB|MA_PVT))
  {
    for (i=0; i < 16; i++)
      if (GEPriv(usr.priv, prm.msg_ask[i]) || mailflag(CFLAGM_ATTRANY))
        break;

    if ((((pmah->ma.attribs & MA_NET)==0 && !mailflag(CFLAGM_ATTRANY)) || i==16) &&
        !AllowAttribute(pmah, MSGKEY_LATTACH))
    {
      item=goitem;
      inc=TRUE;
      ret=0;
      goto DoRet;
    }
  }

  if (usr.help != EXPERT)
  {
    GotoLocText();

    /* If it's pub or pvt... */

    if ((pmah->ma.attribs & (MA_PVT|MA_PUB))==(MA_PVT|MA_PUB))
    {
      if ((pmah->ma.attribs & MA_ATTACH) && AllowAttribute(pmah, MSGKEY_LATTACH))
        Puts(pvt_or_attach_or_help);
      else
        Puts(p_for_pvt_or_q_for_help);
    }
    else
    {
      if ((pmah->ma.attribs & MA_ATTACH) && AllowAttribute(pmah, MSGKEY_LATTACH))
        Puts(enter_msg_attrs_attach);
      else
        Puts(enter_msg_attrs);
    }
  }
  
  GotoLocAttr();
  vbuf_flush();

  while ((ch=(byte)toupper(Mdm_getcw())) != '\r')
  {
    switch (ch)
    {
      case 0:
        switch(Mdm_getcw())
        {
          case K_DOWN:
          item++;
          inc=TRUE;
          ret=0;
          goto DoRet;
        }
        break;

      case '\x1b':
        switch (Mdm_getcw())
        {
          case '\x1b':
            item=ret=-1;
            goto DoRet;

          case '[':
          case 'O':
            switch(Mdm_getcw())
            {
              case 'B':
                item++;
                inc=TRUE;
                ret=0;
                goto DoRet;
            }
            break;
        }
        break;

      case K_CTRLX:
      case K_TAB:
        item++;
        inc=TRUE;
        ret=0;
        goto DoRet;

      case '?':
        Puts(CLS);
        Display_File(DISPLAY_NONE, NULL, "%sattrib", PRM(misc_path));
        RedrawAll(pmah, msg, FALSE);
        GotoLocAttr();
        vbuf_flush();
        break;

      case ' ': /* Assume pvt */
        ch=(byte)msgattr_keys[0];

      default:

        /* Special status for Pvt flag - this relies on area attribute */

        prm.msg_ask[0]=(pmah->ma.attribs & (MA_PVT|MA_PUB)) != (MA_PVT|MA_PUB)
                        ? (word)-1
                        : usr.priv;

        for (i=0; i < 16; i++)
        {
          bit=(1L << (dword)i);
          
          if (((byte)msgattr_keys[i]==ch) && AllowAttribute(pmah, i))
          {
            if (i==0 || (pmah->ma.attribs & MA_NET))
            {
              msg->attr ^= bit;
              DisplayMessageAttributes(msg, pmah);
              
              /* If we are uploading a file using the local file attach
               * mechanism, ensure that the subject line is blank.
               */

              if ((pmah->ma.attribs & MA_ATTACH) && (msg->attr & MSGFILE))
                *msg->subj = 0;

              /* Redraw the Subj/File line if modifying the file bits */

              if (bit & (MSGFILE | MSGFRQ | MSGURQ))
                DisplayMessageSubj(msg, pmah);

              GotoLocAttr();
              vbuf_flush();
            }
            break;
          }
        }

        /* Attach flag is a special case in local attach areas */

        if ((pmah->ma.attribs & MA_NET)==0 &&
            (ch==(byte)msgattr_keys[4]) &&
            AllowAttribute(pmah, MSGKEY_LATTACH))
        {
          msg->attr ^= MSGFILE;
          DisplayMessageAttributes(msg, pmah);
          GotoLocAttr();
          vbuf_flush();
        }
        break;
    }
  }

  item=goitem;
  inc=TRUE;
  
DoRet:

  DisplayMessageAttributes(msg, pmah);
  GotoLocText();
  Puts(fsr_msginfo_col);

  return ret;
}


int near GetFrom(PMAH pmah, XMSG *msg)
{
  if (!(pmah->ma.attribs & MA_ANON) && !mailflag(CFLAGM_ATTRANY))
  {
    if (inc)
      item++;
    else item--;
    
    return 0;
  }

  Puts(fsr_msginfo_col);
  GotoLocFrom();
  if (GetItemString(msg->from, 29))
    return -1;

  if (strpbrk(msg->from, "@!")==NULL || strchr(msg->from, ' '))
    fancier_str(msg->from);

  DisplayShowName(rbox_sho_fname, msg->from);

  return 0;
}

        
int near GetFromAddr(PMAH pmah, XMSG *msg)
{
  char tempaddr[30];
  NETADDR *o;

  if (!(pmah->ma.attribs & MA_ANON) && !mailflag(CFLAGM_ATTRANY))
  {
    if (inc)
      item++;
    else item--;
    
    return 0;
  }

  o=&msg->orig;

  GotoAddrFrom();
  Puts(fsr_addr_col);
  Puts(CLEOL);

  strcpy(tempaddr, Address(o));
  
  if (GetItemString(tempaddr, 24))
    return -1;

  if (*tempaddr=='\0')
    strcpy(tempaddr, Address(&prm.address[0]));

  MaxParseNN(tempaddr, o);
  DisplayShowAddress(rbox_sho_faddr, o, pmah);
  return 0;
}



int near GetToField(PMAH pmah, XMSG *msg)
{
  int ret=0;
  char first_name[PATHLEN];
  char last_name[PATHLEN];
  NETADDR *d;

  d=&msg->dest;

  if (usr.help != EXPERT && !(prm.flags & FLAG_no_ulist) &&
      (pmah->ma.attribs & (MA_NET | MA_SHARED))==0)
  {
    GotoLocText();
    Puts(gmsg_l2p5);
  }
  
  Puts(fsr_msginfo_col);

  strcpy(first_name, msg->to);

  GotoLocTo();
  if (GetItemString(msg->to, 29))
  {
    ret=-1;
    goto DoRet;
  }

  /* To stop those complaints about internet addresses :-) */

  if (strpbrk(msg->to,"@!")==NULL || strchr(msg->to, ' '))
    fancier_str(msg->to);

  strcpy(last_name, msg->to);

  if ((*netnode=='\0' || !eqstri(first_name, msg->to)) &&
      (pmah->ma.attribs & MA_NET))
  {
    if (Get_FidoList_Name(msg, netnode, PRM(fidouser)))
      MaxParseNN(netnode, d);

    /* In case NAMES.MAX changed the 'to' field */

    if (!eqstri(last_name, msg->to))
      DisplayShowName(rbox_sho_tname, msg->to);
  }

  if (eqstri(msg->to, sysop_txt) && (pmah->ma.attribs & MA_NET)==0)
  {
    strcpy(msg->to, PRM(sysop));
    DisplayShowName(rbox_sho_tname, msg->to);
  }
  else
  {

    if (eqstr(msg->to, qmark))
    {
      char temp[PATHLEN];

      /* Try 'misc\userlist' first, since it might simply contain
         help, menu, other information or even [menu_cmd userlist].. */

      sprintf(temp, "%suserlist", PRM(misc_path));
      if (Display_File(DISPLAY_NONE, NULL, temp)!=0 && !(prm.flags & FLAG_no_ulist))
      {

        /* No userlist file is found, so if we are allowed to,
           we can jump right into the userlist */

        Puts(CLS);

        /* If user didn't abort userlist, then pause for an ENTER, *
         * so the screen clear doesn't wipe everything.            */

        if (UserList() != -1)
        {
          Puts(WHITE);
          Press_ENTER();
        }

      }

      *msg->to='\0';

      RedrawAll(pmah, msg, FALSE);
      item=ITEM_TO;
      ret=0;
      goto DoRet;
    }

    /* Make sure user exists if a pvt. message in a local area! */
    if (*msg->to &&
        (pmah->ma.attribs & (MA_NET | MA_SHARED))==0 &&
        (msg->attr & MSGPRIVATE) &&
        !IsInUserList(msg->to, TRUE))
    {
      *msg->to='\0';

      GotoLocText();
      Puts(userdoesntexist);
      Press_ENTER();

      RedrawAll(pmah, msg, FALSE);
      item=ITEM_TO;
    }
  }
  
DoRet:

  DisplayShowName(rbox_sho_tname, msg->to);
  GotoLocText();
  Puts(fsr_msginfo_col);

  return ret;
}



int near GetToAddr(PMAH pmah, XMSG *msg)
{
  NETADDR *d;
  int ret=0;

  NW(pmah);
  
  d=&msg->dest;
  
  if (usr.help != EXPERT)
  {
    GotoLocText();
    Puts(edmsg);
  }
  
  if (*netnode)
    strcpy(netnode,Address(d));

  Puts(fsr_addr_col);
  GotoAddrTo();
  if (GetItemString(netnode, 24))
    return -1;

  if (! *netnode)
    strcpy(netnode, Address(&prm.address[0]));

  MaxParseNN(netnode, d);

  if (eqstri(netnode, "/"))
  {
    Puts(CLS);

    if (NetList())
      Press_ENTER();

    *netnode='\0';

    RedrawAll(pmah, msg, FALSE);
    item=ITEM_DEST;
    ret=0;
  }
  else if (*netnode && (netnode[strlen(netnode)-1]=='/' &&
           (*netnode+1)) || netnode[strlen(netnode)-1]=='#')
  {
    Puts(CLS);

    if (! eqstri(netnode, "#"))
      MaxParseNN(netnode, d);

    if (NodeList(d->zone, d->net))
      Press_ENTER();

    *netnode='\0';

    RedrawAll(pmah, msg, FALSE);
    item=ITEM_DEST;
    ret=0;
  }
  else
  {
    DisplayShowAddress(rbox_sho_taddr, d, pmah);
    DisplayShowDest(d);
  }

  /*
  GotoLocText();
  Puts(CLEOL);
  Puts(fsr_msginfo_col);
  */

  return ret;
}

static void near DisplayShowDest(NETADDR *d)
{
  NFIND *nf;

  if (! *netnode)
    return;

  if ((nf=NodeFindOpen(d)) != NULL)
  {
    Printf(fsr_dest, nf->found.name, nf->found.city);
    NodeFindClose(nf);
  }
  else
  {
    Puts(fsr_unlisted_node);
  }
}




int near GetSubject(PMAH pmah, XMSG *msg)
{
  NW(pmah);

  if ((pmah->ma.attribs & (MA_NET|MA_ATTACH))==(MA_NET|MA_ATTACH) &&
      (msg->attr & MSGFILE) &&
      AllowAttribute(pmah, MSGKEY_LATTACH))
  {
    if (inc)
      item++;
    else
      item--;
    return 0;
  }

  Puts(fsr_msginfo_col);
  
  GotoLocSubj();
  if (GetItemString(msg->subj, sizeof(msg->subj)-2))
    return -1;

  DisplayMessageSubj(msg, pmah);
  return 0;
}




int near GetItemString(char *str, int max)
{
  switch (mdm_getspnc(str, max))
  {
    case -1:
      item=-1;
      return TRUE;

    case MSGENTER_UP:
      item--;
      inc=FALSE;
      break;

    case MSGENTER_DOWN:
      item++;
      inc=TRUE;
  }

  return FALSE;
}


static void near RedrawAll(PMAH pmah, XMSG *msg, int showhelp)
{
  Puts(enter_msg_init);
  DrawReaderScreen(pmah, FALSE);

  if (showhelp && (usr.help != EXPERT) && prm.hlp_hdrentry)
  {
    Puts(GRAY "\n");
    Display_File(DISPLAY_NONE, NULL, PRM(hlp_hdrentry));
  }

  DisplayMessageNumber(msg, this_msgnum, high_msg);
  DisplayMessageAttributes(msg, pmah);

  DisplayShowName(rbox_sho_fname, msg->from);
  DisplayShowDate(rbox_sho_fdate, (union stamp_combo *)&msg->date_written);
  
  if (*msg->from && (pmah->ma.attribs & MA_NET))
    DisplayShowAddress(rbox_sho_faddr, &msg->orig, pmah);

  DisplayShowName(rbox_sho_tname, msg->to);
  DisplayShowDate(rbox_sho_tdate, (union stamp_combo *)&msg->date_arrived);

  if (*netnode && (pmah->ma.attribs & MA_NET))
    DisplayShowAddress(rbox_sho_taddr, &msg->dest, pmah);

  DisplayMessageSubj(msg, pmah);
}

