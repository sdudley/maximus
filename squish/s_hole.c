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

/**
 * @file	s_hole.c
 * @author	Scott J. Dudley
 * @version	$Id: s_hole.c,v 1.3 2003/06/18 01:58:26 wesgarland Exp $
 *
 * $Log: s_hole.c,v $
 * Revision 1.3  2003/06/18 01:58:26  wesgarland
 * Based on changes submitted by Bo Simonsen; modified to have lowercase extensions
 * for the ?ut filenames, where the ? is the mail flavour (FLO)
 *
 */

#ifndef __GNUC__
#pragma off(unreferenced)
#endif
static __attribute__((unused)) char rcs_id[]="$Id: s_hole.c,v 1.3 2003/06/18 01:58:26 wesgarland Exp $";
#ifndef __GNUC__
#pragma on(unreferenced)
#endif

#define NOVARS
/*#define DEBUG_HOLE*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ctype.h>
#include <share.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_hole.h"

static char *msgfrom="ARCmail";
char bsy_pkt_queued[]=":%s busy - packet queued";
static HAREA netmail=NULL;

static struct _netinf *netmsg=NULL;
static word n_netmsg=0;

struct _hpkt *hpl=NULL;
static word n_hpkt=0;

static void near TooManyPkts(void)
{
  S_LogMsg("!|Too many packets in .SQ hold area!");
  S_LogMsg("!|Remaining packets queued for next run");
}

static void near TooManyNetmsgs(void)
{
  S_LogMsg("!|Too many netmail attaches!");
  S_LogMsg("!|Increase `MaxAttach' in SQUISH.CFG!");
}

/* Set the name of a packet in our hpkt list */

static void near SetHpktName(char *hpname, char *setname)
{
  char temp[PATHLEN];
  char *from;

  strcpy(temp, FixOutboundName(0xffff));

  from=setname + strlen(temp);

  /* Make sure that we can get a substring out of it */

  if (strncmp(setname, temp, strlen(temp)) != 0)
  {
    S_LogMsg("!Internal error in path compare!");
    S_LogMsg("!Internal error - have '%s', need substring '%s'!\n",
           setname, temp);

    exit(ERL_ERROR);
  }

  if (strlen(from) >= MAX_HPKT_NAME-1)
  {
    S_LogMsg("!Internal error - from pkt '%s' is more than %d",
             MAX_HPKT_NAME);
    exit(ERL_ERROR);
  }

  strcpy(hpname, from);

  /* Convert the filename to uppercase */

  upper_fn(hpname);
}


/* Return the name of a packet in the holding area */

char *GetHpktName(char *hpname)
{
  static char pktname[PATHLEN];

  strcpy(pktname, FixOutboundName(0xffff));
  strcat(pktname, hpname);
  return pktname;
}

/* Add a packet to our linked list of packets */

void HoleAddPacket(char *name,BLIST far *bl)
{
  struct _hpkt *hpkt;

  NW(config);
  
  if (n_hpkt >= config.maxpkt)
  {
    TooManyPkts();
    return;
  }
  
  hpkt=hpl+n_hpkt++;
  
  hpkt->to.zone =bl->zone;
  hpkt->to.net  =bl->net;
  hpkt->to.node =bl->node;
  hpkt->to.point=bl->point;
  
  hpkt->from=bl->ar->primary;

  SetHpktName(hpkt->name, name);
}

void HoleScanHole(void)
{
  struct _pkthdr hdr;
  struct _hpkt *hpkt;
  
  char hpath[PATHLEN];
  char fname[PATHLEN];
  
  FFIND *ff;
  
  int pfd;
  unsigned bytes;

  
  (void)strcpy(hpath, FixOutboundName(0xffff));

  (void)printf("Checking %s...\n\n", hpath);
  
  (void)strcpy(fname, hpath);
  (void)strcat(fname, "*.?ut");
  
  
  /* If there aren't any packets, then there's nothing to do */

  if ((ff=FindOpen(fname, 0)) == NULL)
    return;

  do
  {
#ifndef UNIX
    (void)strupr(ff->szName);
#endif

    (void)strcpy(fname, hpath);
    (void)strcat(fname, ff->szName);

    #ifdef DEBUG_HOLE
      (void)printf("Attempting to read %s\n", fname);
    #endif
      
    if ((pfd=sopen(fname, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
      continue;

    bytes=(unsigned)fastread(pfd, (char *)&hdr, sizeof(struct _pkthdr));
    (void)close(pfd);

    /* If there was an error reading the file, just continue */
    
    if (bytes != sizeof(struct _pkthdr))
      continue;
    
    (void)Adjust_Pkt_Header(&hdr);
    
    if (n_hpkt >= config.maxpkt)
    {
      TooManyPkts();
      continue;
    }

    hpkt=hpl+n_hpkt++;

    hpkt->from.zone=hdr.orig_zone;
    hpkt->from.net=(word)hdr.orig_net;
    hpkt->from.node=(word)hdr.orig_node;
    hpkt->from.point=hdr.orig_point;

    hpkt->to.zone=hdr.dest_zone;
    hpkt->to.net=(word)hdr.dest_net;
    hpkt->to.node=(word)hdr.dest_node;
    hpkt->to.point=hdr.dest_point;

    SetHpktName(hpkt->name, fname);

    switch (toupper(ff->szName[9]))
    {
      case 'C': hpkt->attr=MSGCRASH;  break;
      case 'H': hpkt->attr=MSGHOLD;   break;
      case 'D': hpkt->attr=MSGCRASH|MSGHOLD; break;
      default:  /* happy lint */  break;
    }

    /* Add to linked list */

    #ifdef DEBUG_HOLE
    (void)printf("%s: %hu:%hu/%hu.%hu -> %hu:%hu/%hu.%hu\n",
                 GetHpktName(hpkt->name),
                 hpkt->from.zone, hpkt->from.net, hpkt->from.node, hpkt->from.point,
                 hpkt->to.zone, hpkt->to.net, hpkt->to.node, hpkt->to.point);
    #endif
  }
  while (FindNext(ff)==0);
}



void Hole_Read_Netmail_Area(void)
{
  struct _netinf *nm;
  XMSG xmsg;
  
  HMSG msgh;
  dword mn;
  byte trunc, del;

  if ((config.flag & FLAG_FRODO)==0)
    return;

  (void)printf("\nScanning attaches in netmail area...\n");
  
  if ((netmail=MsgOpenArea(config.netmail->path,
                           MSGAREA_CRIFNEC,
                           config.netmail->type))==NULL)
  {
    (void)printf("Invalid netmail area: `%s'!\n", config.netmail->path);
    exit(ERL_ERROR);
  }

  for (mn=1; mn <= MsgHighMsg(netmail); mn++)
    if ((msgh=MsgOpenMsg(netmail,MOPEN_READ,mn)) != NULL)
    {
      dword clen;
      byte *ctrl;
      byte *flag;
      
      if ((ctrl=malloc((word)(clen=(dword)(word)MsgGetCtrlLen(msgh))))==NULL)
        clen=0L;
      else clen=(dword)(word)MsgGetCtrlLen(msgh);

      if (MsgReadMsg(msgh, &xmsg, 0L, 0L, NULL, clen, ctrl) != (dword)-1)
      {
        trunc=del=FALSE;
        
        if ((flag=GetCtrlToken(ctrl, "FLAGS")) != NULL)
        {
          if (strstr(flag, "TFS"))
            trunc=TRUE;
          
          if (strstr(flag, "KFS"))
            del=TRUE;

          if (strstr(flag, "DIR"))
            xmsg.attr |= MSGCRASH | MSGHOLD;
          
          MsgFreeCtrlToken(flag);
        }
        
        if (ctrl)
          free(ctrl);
      
        /* Send message if it's FROM "SquishMail", if it originated from    *
         * here, and if it has the F'Att bit (but not Sent) turned on.      */
        
        if ((xmsg.attr & (MSGSENT | MSGFILE))==MSGFILE)
        {
          if (n_netmsg >= config.maxattach)
          {
            TooManyNetmsgs();
            continue;
          }

          nm=netmsg+n_netmsg++;
          
          nm->attr=xmsg.attr;
          (void)NetaddrToSblist(&xmsg.orig, &nm->from);
          (void)NetaddrToSblist(&xmsg.dest, &nm->to);

          nm->name=sstrdup(xmsg.subj);

          nm->trunc=trunc;
          nm->del=del;
          
          #ifdef DEBUG_HOLE
          (void)printf("Msg #%3ld: %s from %s to ",
                       mn, nm->name, Address(&xmsg.orig));

          (void)printf("%s\n", Address(&xmsg.dest));
          #endif
        }
      }
      else
      {
        if (ctrl)
          free(ctrl);
      }
      
      
      (void)MsgCloseMsg(msgh);
    }
  

  #ifdef DEBUG_HOLE
  {
    struct _hpkt *nm, *end;

    (void)printf("To recap:\n\n");

    for (nm=netinf, end=netmsg+n_netmsg; nm < end; nm++)
      (void)printf("%hu:%hu/%hu.%hu, %s\n", nm->to.zone, nm->to.net,
                   nm->to.node, nm->to.point, nm->name);
  }
  #endif
}






void Hole_Free_Netmail_Area(void)
{
  NW(config);

  (void)MsgCloseArea(netmail);
}


word Hole_Add_To_Net(NETADDR *to,char *txt,int flavour)
{
  HMSG msgh;
  long msgattr;
  int attr_dir;

  XMSG msg;
  struct _netinf *nm, *end;

  char nul;
  char ctxt[120];

  int delete_sent;
  int trunc_sent;

  if ((config.flag & FLAG_FRODO)==0)
    return 0;

  delete_sent=trunc_sent=attr_dir=FALSE;

  if (*txt=='^')
  {
    txt++;
    delete_sent=TRUE;
  }

  if (*txt=='#')
  {
    txt++;
    trunc_sent=TRUE;
  }

  msgattr=0L;

  switch(flavour)
  {
    case 'C':
      msgattr |= MSGCRASH;
      break;

    case 'H':
      msgattr |= MSGHOLD;
      break;

    case 'D':
      msgattr &= ~(MSGCRASH|MSGHOLD);
      attr_dir=TRUE;
      break;

    default:
      /* happy lint */
      break;
  }

  #ifdef DEBUG_HOLE
  (void)printf("Searching for match for %s\n",txt);
  #endif


  /* Now scan the linked list, to see if we can find this file in there */

  for (nm=netmsg, end=netmsg+n_netmsg; nm < end; nm++)
  {
    if (eqstri(txt, nm->name) &&
        AddrMatchNS(to, &nm->to))
    {
      #ifdef DEBUG_HOLE
      (void)printf("%s matched %s - no action necessary.\n", txt, nm->name);
      #endif

      return 0;
    }
  }

  /* It we got this far, it must not have been found.  So, prepare to       *
   * write the message.                                                     */

  (void)memset(&msg,'\0',sizeof(XMSG));

  (void)strcpy(msg.from,msgfrom);
  (void)strcpy(msg.to,"SysOp");
  (void)strcpy(msg.subj,txt);

  msg.attr=MSGLOCAL | MSGPRIVATE | MSGKILL | MSGFILE | msgattr;

  (void)SblistToNetaddr(config.addr, &msg.orig);
  
  msg.dest=*to;

  (void)Get_Dos_Date((union stamp_combo *)&msg.date_arrived);
  (void)Get_Dos_Date((union stamp_combo *)&msg.date_written);
 
  if ((msgh=MsgOpenMsg(netmail, MOPEN_CREATE, 0L))==NULL)
  {
    S_LogMsg("!Err creating attach for %s",Address(to));
    return 0;
  }

  nul='\0';
  *ctxt='\0';

  if (delete_sent || trunc_sent || attr_dir)
  {
    (void)strcpy(ctxt,"\x01""FLAGS");

    if (delete_sent)
      (void)strcat(ctxt," KFS");

    if (trunc_sent)
      (void)strcat(ctxt," TFS");

    if (attr_dir)
      (void)strcat(ctxt," DIR");

    (void)strcat(ctxt,"\x01");
  }

  if (MsgWriteMsg(msgh, FALSE, &msg, &nul, (dword)1, (dword)1,
                  (dword)strlen(ctxt)+(dword)1, ctxt) != 0)
  {
    S_LogMsg("!Err writing attach msg for %s", Address(to));
  }

  (void)MsgCloseMsg(msgh);

  #ifdef DEBUG_HOLE
  (void)printf("Generated attach to %s of file %s\n", Address(to), txt);
  #endif

  /* Add to list */

  if (n_netmsg > config.maxattach)
  {
    TooManyNetmsgs();
    return 0;
  }
  
  nm=netmsg+n_netmsg++;

  nm->name=sstrdup(txt);

  (void)NetaddrToSblist(to, &nm->to);
  nm->attr=msg.attr;

  return 1;
}



MATCHOUT * HoleMatchOutOpen(NETADDR *who,int type,byte flavour)
{
  MATCHOUT *mo;

  if ((mo=(MATCHOUT *)malloc(sizeof(MATCHOUT)))==NULL)
    return NULL;

  mo->ff=NULL;
  mo->who=*who;
  mo->flavour=flavour;
  mo->type=(sword)type;
  mo->config=&config;

  mo->hpkt=hpl;

  if (HoleMatchOutNext(mo))
    return mo;
  
  free(mo);
  return NULL;
}

int HoleMatchOutNext(MATCHOUT *mo)
{
  byte *fn;
  struct _hpkt *end;

  mo->got_type=MATCH_OUT;

  for (end=hpl+n_hpkt; mo->hpkt < end; mo->hpkt++)
  {
    fn=strrchr(GetHpktName(mo->hpkt->name), '.');

    if (fn &&
        AddrMatchNS(&mo->who, &mo->hpkt->to) &&
        fexist(GetHpktName(mo->hpkt->name)) &&
        (mo->flavour==0 ||
         mo->flavour==(byte)toupper(fn[1]) ||
         (mo->flavour=='F' && fn[1]=='O') ||
         (mo->flavour=='O' && fn[1]=='F') ||
         (mo->flavour=='L' && fn[1]=='N') ||
         (mo->flavour=='U' && fn[1]!='N')))
    {
      (void)SblistToNetaddr(&mo->hpkt->to, &mo->found);
      (void)strcpy(mo->name, GetHpktName(mo->hpkt->name));
      mo->fFromHole=TRUE;
      return 1;
    }
  }

  return 0;
}


void HoleMatchOutClose(MATCHOUT *mo)
{
  if (mo)
    free(mo);
}



void GetFunkyPacketName(char *name, struct _sblist *from, struct _sblist *to, int flavour)
{
  struct _hpkt *hp, *end;
  char *last;

 
  for (hp=hpl, end=hpl+n_hpkt; hp < end; hp++)
    if (AddrMatchS(&hp->from, from) && AddrMatchS(&hp->to, to) &&
       (MsgAttrToFlavour(hp->attr)==flavour /*|| (config.flag & FLAG_ADDMODE)*/))
    {
      char hpname[PATHLEN];

      strcpy(hpname, GetHpktName(hp->name));

      last=strrchr(hpname, '\\');

      if (last==NULL)
        last=strrchr(hpname, '/');

      if (last==NULL)
      {
        S_LogMsg("!Internal error - last=null, hpname=%s", hpname);
        exit(ERL_ERROR);
      }

      /* Copy name of packet, but not path. */

      (void)strcpy(name, last ? last+1 : name);

       return;
    }
    
  (void)sprintf(name, "%08lx.%cut", get_unique_number(), tolower((int)flavour));
}


#ifdef NEVER

void HoleOpenList(void)
{
  char temp[PATHLEN];
  
  if ((config.flag & FLAG_FRODO)==0)
    return;

  (void)sprintf(temp, holename, FixOutboundName(1u));

  if ((holefile=sopen(temp, O_CREAT | O_WRONLY | O_BINARY, SH_DENYNO,
                     S_IREAD | S_IWRITE))==-1)
  {
    S_LogMsg("!Error creating HOLELIST.DAT!");
    exit(ERL_ERROR);
  }

  lseek(holefile,0L,SEEK_END);
}

void HoleCloseList(void)
{
  if ((config.flag & FLAG_FRODO)==0)
    return;

  if (holefile != -1)
  {
    close(holefile);
    holefile=-1;
  }
}

#endif


void HoleMoveOut(void)
{
  NETADDR addr;
  struct _hpkt *hp, *end;
  char newname[PATHLEN];
  char *dot;
  char flavour;
  
  if (config.flag & FLAG_FRODO)
    return;

  for (hp=hpl, end=hpl+n_hpkt; hp < end; hp++)
  {
    dot=strrchr(GetHpktName(hp->name), '.');
    
    if (dot==NULL || dot < strrchr(GetHpktName(hp->name), PATH_DELIM))
      flavour='O';
    else flavour=dot[1];

    if (fexist(GetHpktName(hp->name)))
    {
      MakeOutboundName(SblistToNetaddr(&hp->to, &addr), newname);
      (void)sprintf(newname+strlen(newname), "%cut", tolower((int)flavour));

      if (BusyFileExist(&addr))
      {
        S_LogMsg(bsy_pkt_queued,
                 Address(SblistToNetaddr(&hp->to, &addr)));
      }
      else if (!eqstri(GetHpktName(hp->name), newname))
        (void)Merge_Pkts(GetHpktName(hp->name), newname);
    }

/*    free(hp->name);
    hp->name=NULL;*/
    *hp->name=0;
  }
}



void HoleRemoveFromList(char *name)
{
  struct _hpkt *hp;
  
  for (hp=hpl; hp < hpl+n_hpkt; hp++)
  {
    /* Not found, so skip to next entry */

    if (!eqstri(GetHpktName(hp->name), name))
      continue;
    
/*    free(hp->name);*/
    
    (void)memmove(hp, hp+1, (n_hpkt-(word)(hp-hpl)-1)*sizeof(struct _hpkt));
    n_hpkt--;
  }
}



/* Handle the renaming of a packet in the OUT.SQ area.  This is used        *
 * when the CHANGE routing verb changes the flavour of a message.           */
   
void HoleRename(char *from, char *to)
{
  struct _hpkt *hp;
  
  for (hp=hpl; hp < hpl+n_hpkt; hp++)
  {
    if (!eqstri(GetHpktName(hp->name), from))
      continue;
    
    SetHpktName(hp->name, to);
/*    free(hp->name);
    hp->name=strdup(to);*/
    break;
  }
}



void HoleInitHole(void)
{
  if (!hpl)
  {
    hpl=smalloc(config.maxpkt * sizeof(struct _hpkt));
    n_hpkt=0;
  }
  
  if (!netmsg)
  {
    if (config.flag & FLAG_FRODO)
      netmsg=smalloc(config.maxattach * sizeof(struct _hpkt));
  
    n_netmsg=0;
  }
}




void HoleDeinitHole(void)
{
  word n;
  
  if (hpl)
  {
/*    for (n=0; n < n_hpkt; n++)
      if (hpl[n].name)
        free(hpl[n].name);*/

    free(hpl);
    hpl=NULL;
  }

  if ((config.flag & FLAG_FRODO)==0)
    return;
  
  if (netmsg)
  {
    for (n=0; n < n_netmsg; n++)
      if (netmsg[n].name)
        free(netmsg[n].name);
    
    free(netmsg);

    n_netmsg=0;
    netmsg=NULL;
  }
}


/* Nuke all bundles in the outbound area which don't have an accompanying   *
 * attach message.                                                          */

void Hole_Nuke_Bundles(void)
{
  char hpath[PATHLEN];
  char fname[PATHLEN];
  char full[PATHLEN];
  FFIND *ff;
  char *ext;
  
  if ((config.flag & FLAG_FRODO)==0)
    return;

  (void)strcpy(hpath, FixOutboundName(0xffff));
  (void)strcpy(fname, hpath);
  (void)strcat(fname, "*.*");
  
  if ((ff=FindOpen(fname, 0)) != NULL)
  {
    do
    {
      struct _netinf *nm, *end;

      char *strs[]={"mo", "tu", "we", "th", "fr", "sa", "su", NULL};
      char **p;


      (void)upper_fn(ff->szName);

      /* Find the extension of the file */
      
      ext=ff->szName+strlen(ff->szName)-4;
      

      /* Make sure that it ends with a dot. */
      
      if (*ext++ != '.')
        continue;

      /* Make sure that it's an arcmail file */

      for (p=strs; *p; p++)
        if (eqstrin(ext, *p, 2))
          break;

      /* If it's not arcmail, loop to the next filename */

      if (! *p)
        continue;
      
      /* GOT IT!  OK, now search the netmail attach list.  Stop when        *
       * we find a match.                                                   */
      
      for (nm=netmsg, end=netmsg+n_netmsg; nm < end; nm++)
        if (stristr(nm->name, ff->szName))
          break;


      /* If we found it in attach list, there's no need to nuke bundle */

      if (nm < end)
        continue;
      
      (void)strcpy(full, hpath);
      (void)strcat(full, ff->szName);
      (void)fancy_fn(full);
      
      (void)unlink(full);
    }
    while (FindNext(ff)==0);
    
    FindClose(ff);
  }
}


#if 0

void FloToArc(void)
{
  MATCHOUT *mo;
  NETADDR node={ZONE_ALL, NET_ALL, NODE_ALL, POINT_ALL};
  FILE *flo;
  word flag;

  flag=config.flag;

  config.flag &= ~FLAG_FRODO;
  
  mo=MatchOutOpen(&node, MATCH_FLO, 0);
  
  if (!mo)
    return;
  
  do
  {
    char temp[PATHLEN];
    byte *fp, flav;
      
    
    if ((flo=shfopen(mo->name, "r", O_RDONLY))==NULL)
    {
      (void)printf("Error opening %s\n", mo->name);
      continue;
    }

    /* Determine the flavour of this file */

    fp=strrchr(mo->name, '.');
      
    if (fp==NULL)
      flav='F';
    else flav=(byte)toupper(fp[1]);
      
    while (fgets(temp, PATHLEN, flo))
    {
      Strip_Trailing(temp, '\n');
      
      config.flag |= FLAG_FRODO;

      if (Hole_Add_To_Net(&mo->found, temp, flav))
        (void)printf("Sent %s to %s\n", temp, Address(&mo->found));

      config.flag &= ~FLAG_FRODO;
    }
    
    (void)fclose(flo);
  }
  while (MatchOutNext(mo));

  MatchOutClose(mo);

  config.flag=flag;
}


void ArcToFlo(void)
{
  struct _netinf *nm;
  NETADDR to;
  
  if ((config.flag & FLAG_FRODO)==0)
    return;
  
  for (nm=netmsg; nm < netmsg+n_netmsg; nm++)
  {
    byte temp[PATHLEN];
    byte name[PATHLEN];
    byte flav;
    
    flav=(byte)
          ((nm->attr & MSGCRASH) ? 'C' : (nm->attr & MSGHOLD) ? 'H' : 'F');
    
    config.flag &= ~FLAG_FRODO;
    FloName(temp, SblistToNetaddr(&nm->to, &to), flav, FALSE);

    if (nm->trunc || nm->del)
    {
      name[0]=(byte)(nm->trunc ? '#' : '^');
      (void)strcpy(name+1, nm->name);
    }
    else (void)strcpy(name, nm->name);

    (void)Add_To_FloFile(name, NULL, temp);
    config.flag |= ~FLAG_FRODO;

    (void)printf("Sending %s to %s\n", nm->name, Address(&to));
  }
}

void KillArc(void)
{
  HAREA sq;
  HMSG msgh;
  XMSG msg;
  UMSGID uid;
  struct _cfgarea *ar=config.netmail;
  dword mn;
  
  if ((sq=MsgOpenArea(ar->path, MSGAREA_NORMAL, ar->type))==NULL)
  {
    (void)printf("Error opening netmail area\n");
    return;
  }
  
  for (mn=1L; mn <= MsgHighMsg(sq); mn++)
  {
    if ((msgh=MsgOpenMsg(sq, MOPEN_READ, mn))==NULL)
      continue;

    MsgReadMsg(msgh, &msg, 0L, 0L, NULL, 0L, NULL);
    MsgCloseMsg(msgh);
    
    if (!eqstr(msg.from, "ARCmail"))
      continue;

    (void)printf("Nuked #%ld, to %s, re: %s\n", mn, Address(&msg.dest),
                 msg.subj);
    
    uid=MsgMsgnToUid(sq, mn);
    MsgKillMsg(sq, mn);
    mn=MsgUidToMsgn(sq, uid, UID_PREV);
  }
  
  MsgCloseArea(sq);
}

#endif
