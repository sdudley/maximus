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
static char rcs_id[]="$Id: s_toss.c,v 1.4 2003/09/10 22:15:48 paltas Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <time.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_toss.h"
#include "s_dupe.h"
#include "arcmatch.h"
#ifdef UNIX
# include <errno.h>
#endif

#ifndef __TURBOC__
#include <stddef.h>
#endif

static char msgs_name[]="max_msgs.dat";
char t_area[]="AREA";

static char comment[80]="";
static char last_comment[80]="";

static struct _partial part, resume;
static dword toss_cnt;
static long nmsg_skipped=0L;

#ifdef DJ
extern FILE *dj;
#endif

/* List of packets to toss */

struct _plist
{
  char *name;
  union stamp_combo ldate;
  struct _plist *next;
};



void Toss_Messages(char *echotoss, word last_max, time_t start)
{
  struct _tosspath *tp;
  time_t secs;

  talist=smalloc(sizeof(struct _talist) * config.max_talist);
  talist_init=FALSE;
  
  toss_cnt=0L;
  *last_area='\0';
  *last_comment='\0';
  msgb=NULL;
  sq=NULL;
  last_sq=NULL;

  config.has_dlist=NULL;
  Alloc_Buffer(maxmsglen+sizeof(XMSG));

  if (config.flag & FLAG_ONEPASS)
    Alloc_Outbuf();

  /* Only toss from bad_msgs if we're NOT retossing an old packet from      *
   * max_msgs.                                                              */

  if (!last_max && (config.flag & FLAG_TOSSBAD))
    TossBadMsgs(config.badmsgs);


  /* Read the MAX_MSGS.DAT file to pick up old packet information */
  
  ReadMaxMsgs(echotoss);

  /* Process the resumed packet, if any */

  if (*resume.pktname)
  {
    Zero_Statistics();
    S_LogMsg("#Tossing messages from resumed packet");
    Toss_Pkt(resume.pktname, resume.tflag);
  }


  /* Toss compressed mail */

  for (tp=config.tpath; tp && !erl_max; tp=tp->next)
  {
    if (tp->flag & TFLAG_NOARC)
      continue;

    TossArchives(tp);
  }


  /* Now toss bare packets from the netfile directories */

  for (tp=config.tpath; tp && !erl_max; tp=tp->next)
    if ((tp->flag & TFLAG_NOPKT)==0)
      Toss_Packet_Sequence(tp->path, tp->flag);


  /* Now toss everything from the current directory, including any          *
   * bare .pkt files.                                                       */

  if (!erl_max)
    Toss_Packet_Sequence(".", 0);


  /* Close any previously-opened message areas */

  Close_Area();

  /* Flush duplicate buffer */
  
  DupeFlushBuffer();

  secs=time(NULL)-start;

  if (secs==0)
    secs=1;

  if (!erl_max)
    ReportSpeed(secs);

  if (config.flag & FLAG_ONEPASS)
  {
    Flush_Outbuf();
    Free_Outbuf();
  }

  (void)printf("\n");

  Free_Buffer();
  Write_Echotoss(echotoss);
  free(talist);
}


/* Compare two packet timestamps */

static int _stdc plcomp(const void *i1, const void *i2)
{
  struct _plist *p1=(struct _plist *)i1;
  struct _plist *p2=(struct _plist *)i2;

  if (p1->ldate.ldate==p2->ldate.ldate)
    return 0;
  else if (GEdate(&p1->ldate, &p2->ldate))
    return 1;
  else return -1;
}




/* Toss compressed mail from the various netfile areas */

static void near TossArchives(struct _tosspath *tp)
{
  struct _plist *plist, *pl, *plar, *plnext, *plp;
  extern char *arcm_exts[];
  char fname[PATHLEN];
  FFIND *ff;
  char **p;
  int n_pl;

  /* Toss any existing packets from current dir and netfile path */

  n_pl=0;
  plist=NULL;


  /* Now create a list of ARCmail in this directory */

  for (p=arcm_exts; *p; p++)
  {
    (void)sprintf(fname, "%s" PATH_DELIMS "*%s?", tp->path, *p);

    if ((ff=FindOpen(fname, 0)) != NULL)
    {
      do
      {
        pl=smalloc(sizeof(*pl));
        pl->name=sstrdup(ff->szName);
        pl->ldate=ff->scWdate;
        pl->next=plist;
        plist=pl;
        n_pl++;
      }
      while (FindNext(ff)==0);

      FindClose(ff);
    }
  }

  /* Make sure that we have some mail to toss */

  if (!n_pl)
    return;

  /* Copy to array */

  plar=smalloc(sizeof(struct _plist) * n_pl);

  for (plp=plar, pl=plist; pl; plnext=pl->next, free(pl), pl=plnext)
    *plp++=*pl;

  qsort(plar, n_pl, sizeof(struct _plist), plcomp);

  for (pl=plar; n_pl-- && !erl_max; pl++)
  {
    sprintf(fname, "%s" PATH_DELIMS "%s", tp->path, pl->name);

    if (Decompress_Archive(fname, "*.pkt") != 0)
    {
      char ren[PATHLEN];
      char final[PATHLEN];

      /* Rename 0000fffc.su? to 0000fffc.bu? etc. */

      (void)strcpy(ren, fname);

      if (strlen(ren) > 3)
        ren[strlen(ren)-3]='b';

      (void)uniqrename(fname, ren, final, 0);

      S_LogMsg("!Bad archive:  renamed to %s", final);
    }
    else
    {
      /* else the decompress was OK, so return TRUE */
      (void)unlink(fname);

      if ((config.flag & FLAG_BATCHXA)==0)
        Toss_Packet_Sequence(".", tp->flag);
    }
  }

  free(plar);
}



/* Report the tossing speed of the current run */

static void near ReportSpeed(time_t secs)
{
  unsigned long processed;

  if (!secs)
    return;

  (void)printf("\nTossed %lu messages in %lu seconds (%lu.%lu msgs/sec).\n",
               nmsg_tossed,
               secs,
               (unsigned long)nmsg_tossed/secs,
               (unsigned long)(nmsg_tossed*10Lu/secs) % 10Lu);

  if (nmsg_skipped)
    (void)printf("** SKIPPED %lu MESSAGES THAT WERE TOO LONG TO TOSS.\n",
                 nmsg_skipped);

  if (config.flag & FLAG_ONEPASS)
  {
    (void)printf("Sent %lu messages in %lu seconds (%lu.%lu msgs/sec).\n",
                 nmsg_sent,
                 secs,
                 (unsigned long)nmsg_sent/secs,
                 (unsigned long)(nmsg_sent*10Lu/secs) % 10Lu);

    processed=nmsg_tossed+nmsg_sent;

    (void)printf("Processed %lu messages in %lu seconds (%lu.%lu msgs/sec).\n",
                 (unsigned long)processed,
                 (unsigned long)secs,
                 (unsigned long)processed/secs,
                 (unsigned long)(processed*10L/secs) % 10L);

  }
}



/* Read the max_msgs file that we left around last time */

static void near ReadMaxMsgs(char *tosslog)
{
  int fd;
  FILE *tl;
  char temp[PATHLEN];
   
  /* If we're to resume a max_msgs toss/scan */

  if (!fexist(msgs_name))
  {
    /* Not there, so nothing to resume */
    
    (void)memset(&resume, '\0', sizeof(resume));
    return;
  }

  if ((fd=sopen(msgs_name, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    S_LogMsg(cantopen, msgs_name);
  else
  {
    if (read(fd, (char *)&resume, sizeof(part)) != (int)sizeof(part))
      S_LogMsg(cantread, msgs_name);

    (void)close(fd);
    (void)unlink(msgs_name);
  }

  /* Now read in the echotoss.log, set the "tossed to" flag for all of the  *
   * areas that it specifies, and then delete the tosslog.  (This will get  *
   * rewritten on the last pass of tossing.)                                */

  if ((tl=shfopen(tosslog, "r", O_RDONLY))==NULL)
    return;

  while (fgets(temp, PATHLEN, tl))
  {
    struct _cfgarea search, *found;
    
    (void)Strip_Trailing(temp, '\n');
    search.name=temp;
    
    if ((found=SkipSearchList(config.area, &search)) != NULL)
      found->flag |= AFLAG_TOSSEDTO;
  }
  
  (void)fclose(tl);
  (void)unlink(tosslog);
}




/* Write the max_msgs file */

static void near WriteMaxMsgs(void)
{
  int fd;

  /* Save the status of our 'long message encountered' flag */

  part.long_packet=resume.long_packet;
    
  if ((fd=sopen(msgs_name, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
               SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    S_LogMsg(cantopen, msgs_name);
  else
  {
    (void)write(fd, (char *)&part, sizeof(part));
    (void)close(fd);
  }
}




/* Write our echotoss.log file for this pass */

static void near Write_Echotoss(char *echotoss)
{
  struct _cfgarea *ar;
  FILE *etf;


  if (*echotoss)
  {
    if ((etf=shfopen(echotoss, "a", O_CREAT | O_WRONLY | O_APPEND))==NULL)
    {
      S_LogMsg("!Error opening echotoss filename `%s'!",echotoss);
      return;
    }

    /* Write everything except the netmail area to the tosslog */

    for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
    {
      if ((ar->flag & AFLAG_TOSSEDTO) && *ar->name && !NetmailArea(ar) &&
          !BadmsgsArea(ar) && !DupesArea(ar))
      {
        (void)fprintf(etf, "%s\n", ar->name);
      }
    }

    (void)fclose(etf);
    
    if (fsize(echotoss)==0)
      (void)unlink(echotoss);
  }
}




/* Decompress an incoming archive */

static int near Decompress_Archive(char *arcname,char *get)
{
  struct _arcinfo *ai;
  char cmd[PATHLEN];
  int arcret, err, fd;

  #ifdef DMALLOC
    /*dptab(FALSE);*/
  #endif

  if ((fd=sopen(arcname, O_RDONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    return -1;

  (void)printf("\nDecompressing ");

  for (ai=config.arc; ai; ai=ai->next)
    if (MatchArcType(fd, ai))
    {
      (void)printf("%s ", ai->arcname);
      break;
    }

  if (ai==NULL)
  {
    (void)printf("unknown ");
    ai=config.arc;
  }
  
  (void)printf("bundle %s...\n\n", arcname);

  (void)close(fd);

  S_LogMsg("*Un%sing %s", ai->arcname, arcname);

#ifdef DJ
  fprintf(dj, "**** Decompressing %s\n", arcname);
#endif
 
  Form_Archiver_Cmd(arcname, get, cmd, ai->extract);

  arcret=CallExtern(cmd, TRUE);
  
  if (arcret==0 && !fexist("*.pkt"))
  {
    S_LogMsg("!No packets found after calling archiver!");
    arcret=-1;
    errno=0;
  }

  if (arcret==0)
    err=0;
  else
  {
    HandleArcRet(arcret, cmd);
    err=-1;
  }

  return err;
}



/* Toss a bunch of packets from a specific path, sorted by date */

static void near Toss_Packet_Sequence(char *path, word tflag)
{
  FFIND *ff;
  struct _plist *plist, *pl, *plnext, *plar, *plp;
  char pktname[PATHLEN];
  char temp[PATHLEN];
  word n_pl;
  int ret;

  (void)sprintf(temp, "%s" PATH_DELIMS "*.pkt", path);

  ff=FindOpen(temp,0);

  if (! ff)
    return;

  S_LogMsg("#Tossing%s msgs from %s%s",
           (config.flag & FLAG_ONEPASS) ? "/scanning" : "",
           path,
           (config.flag & FLAG_SECURE) ? " (secure)" : "");

  /* Build a linked list out of the packet names in this area */
         
  for (ret=0, n_pl=0, plist=NULL; ret==0; ret=FindNext(ff))
  {
    pl=smalloc(sizeof(struct _plist));
    
    /* Add packet name to list, including path */

    (void)sprintf(pktname, "%s" PATH_DELIMS "%s", path, ff->szName);
    pl->name=sstrdup(pktname);

    /* Add packet time */

    pl->ldate=ff->scWdate;

    /* Add to linked list */

    pl->next=plist;
    plist=pl;
    
    n_pl++;
  }
  
  /* Allocate an array to hold the silly thiing */
  
  plar=smalloc(sizeof(struct _plist)*n_pl);

  /* Now copy the linked list to an array */
  
  for (plp=plar, pl=plist; pl; plnext=pl->next, free(pl), pl=plnext)
    *plp++=*pl;

  /* Quicksort the list of packets into order of ascending age */
  
  qsort(plar, n_pl, sizeof(struct _plist), plcomp);

  /* Now toss from this sorted packet list */
  
  for (pl=plar; n_pl--; pl++)
  {
    if (!erl_max)
    {
      Zero_Statistics();
      Toss_Pkt(pl->name, tflag);
    }
    
    free(pl->name);
  }

  free(plar);

  FindClose(ff);
  
  return;
}





/* Rename a packet to a particular extension */

static void near RenamePkt(char *pktname, char *ext, char *out)
{
  char *s;

  /* If it wasn't processed, rename it to *.BAD */

  (void)strcpy(out, pktname);

  if ((s=strrchr(out, '.'))==NULL)
    (void)strcat(out, ext);
  else
    (void)strcpy(++s, ext+1);

  (void)uniqrename(pktname, out, NULL, 0);
}



/* Log a bad packet */

static void near BadPacket(char *pktname)
{
  char szFinal[PATHLEN];

#ifndef UNIX
  RenamePkt(pktname, ".BAD", szFinal);
#else
  RenamePkt(pktname, ".bad", szFinal);
#endif

  S_LogMsg("!Bad packet:  renamed to %s", szFinal);
}



/* Log a packet with a long message */

static void near LongPacket(char *pktname)
{
  char szFinal[PATHLEN];

#ifndef UNIX
  RenamePkt(pktname, ".LNG", szFinal);
#else
  RenamePkt(pktname, ".lng", szFinal);
#endif

  S_LogMsg("!Packet contains long msg:  renamed to %s", szFinal);
}




/* Indicate that we are tossing a particular packet */

static void near Tossing_It(char *name)
{
  char *p;
  
  if ((p=strrchr(name, '/')) != NULL)
    name=p+1;
  
  if ((p=strrchr(name, '\\')) != NULL)
    name=p+1;
  
  if ((p=strrchr(name, ':')) != NULL)
    name=p+1;
  
  (void)printf("Toss %12.12s: ", name);
}





/* Say where the packet originated from */

static void near Tossing_From(word zone, word net, word node, word point)
{
  char temp[50];

  /* Add the system's name info the buffer */

  (void)sprintf(temp, "(%hu:%hd/%hd", zone, net, node);

  if (point)
    (void)sprintf(temp+strlen(temp), ".%hu", point);

  (void)strcat(temp, ")");

  (void)printf("%-22s", temp);
}




#ifndef UNIX 

static char *products[]=
{ 
  "Fido", "ConfMail", "SEAdog", NULL, NULL, "Opus", "Dutchie", NULL,
  "Tabby", NULL, "Wolf-68k/CPM-68k", "QMM", "FrontDoor", NULL, NULL,
  NULL, NULL, NULL, NULL, "GS-Point", NULL, NULL, NULL, NULL, NULL,
  NULL, "D'Bridge", "Bink", NULL, NULL, "Daisy", NULL, NULL,
  NULL, "TMail", "TCOMMail", NULL, "RBBSMail", NULL, "Chameleon", NULL,
  "QMail", NULL, NULL, NULL, NULL, "AMAX", NULL, NULL, NULL, "Paragon",
  NULL, "StarNet", NULL, "QEcho", NULL, NULL, "TrapDoor", "Welmat",
  NULL, NULL, NULL, NULL, "TosScan", NULL, NULL, "TIMS", "Isis", NULL,
  "XRS", NULL, NULL, NULL, NULL, NULL, "IMAIL", NULL, NULL, NULL, NULL,
  "InterMail", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
  "Raid", NULL, NULL, NULL, NULL, NULL, NULL, "GEcho", NULL, NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, "Mosaic", "TPBEcho", NULL,
  NULL, NULL, NULL, NULL, NULL, NULL, "Ping", NULL, NULL, NULL, NULL,
  NULL, "Squish", NULL, NULL, NULL, NULL, NULL, "MsgTrack", "FMail",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL, "FMAIL", NULL, "Tick",
  NULL, NULL, NULL, NULL, NULL, NULL, NULL
};

#endif

/* Print out product code for a known product */

#ifdef UNIX
extern struct ftscprod_ products[];
#endif

#ifndef UNIX
#define NUMPROD (sizeof(products)/sizeof(products[0]))
#endif

static char * near ProdCode(byte prod)
{
  static char pnum[3];
#ifndef UNIX
  if (prod < NUMPROD && products[prod])
    return products[prod];
  else
  {
    (void)sprintf(pnum, "%02hx", (word)prod);
    return pnum;
  }
#else
  if (products[prod].name)
    return products[prod].name;
  else
  {
    (void)sprintf(pnum, "%02hx", (word)prod);
    return pnum;
  }
#endif
}




/* Indicate the origin of a packet */
static void near WhoFrom(char *name, struct _inmsg *in)
{
  S_LogMsg("* %s, %02d/%02d/%02d, %02d:%02d:%02d, by %s %u.%02d",
           fancy_fn(name),
           in->pkt.month+1, in->pkt.day, (in->pkt.year % 100),
           in->pkt.hour, in->pkt.minute, in->pkt.second,
           ProdCode(in->pkt.product),
           (in->pkt.rev_maj > 10) ? (in->pkt.rev_maj/10) : (in->pkt.rev_maj),
           in->pkt.rev_min);

  S_LogMsg("- Orig=%hu:%hu/%hu.%hu, Dest=%hu:%hu/%hu.%hu, Type=%s",
           in->pkt.orig_zone,
           in->pkt.orig_net,
           in->pkt.orig_node,
           in->pkt.orig_point,
           in->pkt.dest_zone,
           in->pkt.dest_net,
           in->pkt.dest_node,
           in->pkt.dest_point,
           (in->pkt.cw & cREV2P) ? "2+" : "StoneAge");
}

/* Returns TRUE if two packet names are equal, not couting the path */

static word near PktNameEqual(char *p1, char *p2)
{
  char *n1, *n2;
  
  n1=strrstr(p1, "/\\:");
  
  if (n1)
    n1++;
  else n1=p1;
    
  n2=strrstr(p2, "/\\:");

  if (n2)
    n2++;
  else n2=p2;
  
  return (eqstri(n1, n2));
}




/* Toss many messages from a single .pkt */

static void near Toss_Pkt(char *pktname, word tflag)
{
  char *s;
  struct _inmsg in;
  int tossed=FALSE;
  unsigned long size;

  bad_packet=FALSE;
  resume.long_packet=FALSE;
  *last_area='\0';
  *last_comment='\0';
  (void)strcpy(in.pktname, pktname);

#ifdef DJ
  (void)fprintf(dj, "**** Tossing from %s\n", in.pktname);
#endif

  if ((in.pktfile=sopen(in.pktname, O_RDONLY | O_BINARY | O_NOINHERIT,
                        SH_DENYWR, S_IREAD | S_IWRITE))==-1)
  {
    (void)printf("Error opening packet `%s'!\n", fancy_fn(in.pktname));
    return;
  }
  
  /* Save the name of this packet, in case we're doing a partial            *
   * toss/scan.                                                             */

  (void)strcpy(part.pktname, in.pktname);
  part.tflag=tflag;
  
  if ((size=(unsigned long)lseek(in.pktfile, 0L, SEEK_END))==0L)
    (void)printf("Deleting null packet %s", in.pktname);
  else
  {
    (void)lseek(in.pktfile, 0L, SEEK_SET);

    Tossing_It(fancy_fn(in.pktname));

    if (fastread(in.pktfile, (char *)&in.pkt, (unsigned)sizeof(struct _pkthdr)) !=
           (int)sizeof(struct _pkthdr) || in.pkt.ver != PKTVER)
    {
      S_LogMsg("!Invalid packet: %s",in.pktname);
      bad_packet=TRUE;
    }

   
    s=strrchr(in.pktname, PATH_DELIM);

    if (s)
      s++;
    else s=in.pktname;

    if (!bad_packet)
    {
      (void)Adjust_Pkt_Header(&in.pkt);

/*
: 06 May 11:56:56 SQSH Tossing msgs from . (secure)
# 06 May 11:56:56 SQSH  25212309.Pkt 05/06/91 01:23:45 Product=Squish
# 06 May 11:56:56 SQSH  Orig=1:249/106.0, Dest=1:249/122.0, Type=StoneAge
*/

      WhoFrom(s, &in);

      if (! PktNameEqual(in.pktname, resume.pktname))
        SetPktOfs((long)sizeof(struct _pkthdr));
      else
      {
        /* We ARE tossing the resumed packet, so set flags appropriately */

        S_LogMsg("*   Resuming toss at offset %ld", resume.offset);

        (void)lseek(in.pktfile, resume.offset, SEEK_SET);
        SetPktOfs(resume.offset);

        (void)memset(&resume, '\0', sizeof(resume));
      }

      Tossing_From(in.pkt.orig_zone,
                   (word)in.pkt.orig_net,
                   (word)in.pkt.orig_node,
                   in.pkt.orig_point);

      /* Write statistics information for this packet */

      if (config.flag & FLAG_STATS)
      {
        struct _tpkt tp;
        char *p;

        tp.orig.zone= (word)in.pkt.orig_zone; /* Copy orig address */
        tp.orig.net=  (word)in.pkt.orig_net;
        tp.orig.node= (word)in.pkt.orig_node;
        tp.orig.point=(word)in.pkt.orig_point;

        /* Strip path off packet name */

        if ((p=strrstr(in.pktname, "/\\")) != NULL)
          p++;
        else p=in.pktname;

        (void)strncpy(tp.pktname, p, 13);
        tp.pktname[13]='\0';

        tp.size=size;
        (void)Get_Dos_Date(&tp.proc_time);

        /* Write to stats file */

        StatsWriteBlock(TYPE_PKT, sizeof tp, 0, &tp);
      }


      if ((config.flag & FLAG_SECURE) && !PacketSecure(&in))
      {
        S_LogMsg("!Insecure packet from %hu:%hd/%hd.%hu",
                 in.pkt.orig_zone,
                 in.pkt.orig_net,
                 in.pkt.orig_node,
                 in.pkt.orig_point);

        bad_packet=TRUE;
      }
      else
      {
        int got;

        (void)Get_To_Nul(NULL, 0, -1, 0);

        while ((got=TossReadMsgFromPkt(&in)) != -1)
        {
          tossed=TRUE;

          /* Only toss a message if got==0.  If got==1, it means that       *
           * a msg was too long to process.  Therefore, we don't want       *
           * to toss THIS msg, but we want to continue with the rest        *
           * of the packet.                                                 */

          if (got==0 && !TossOneMsg(&in, FALSE, tflag))
          {
            bad_packet=TRUE;
            break;
          }

          if (config.max_msgs && toss_cnt >= config.max_msgs)
            break;
        }
        
        if (config.max_msgs && toss_cnt >= config.max_msgs)
          erl_max=TRUE;
      }
    }
  }

  (void)close(in.pktfile);

  NW(tossed);
     
  (void)printf("\n");

  if (bad_packet)
    BadPacket(in.pktname);
  else if (erl_max)
  {
    /* If we hit the max # of msgs to toss, create a temp file to save      *
     * our location.                                                        */
    
    WriteMaxMsgs();
  }
  else if (resume.long_packet)
    LongPacket(in.pktname);
  else
  {
    if (unlink(in.pktname) != 0)
      S_LogMsg("!Can't delete %s", in.pktname);
  }

  StatsWriteAreas();
  Report_Statistics();
}



/* Adjust the packet header to handle 2+ packets */

word Adjust_Pkt_Header(struct _pkthdr *pkt)
{
  word type=cREV2;

  /* If the capability word doesn't match, then it's bogus */

  if ( ((pkt->cw >> 8) | (pkt->cw << 8)) != pkt->cw_validation)
    pkt->cw=cREV2;

  if (pkt->cw & cREV2P)
  {
    /* Fix the kludge of pkt->orig_net==-1 for point compatibility with    *
     * older systems.                                                     */

    /* not mandated by official 2+ spec */

    /*
    if (pkt->orig_point)
      pkt->orig_net=pkt->aux_net;
    */
    
    pkt->cw=type=cREV2P;
  }
  else /* If 2+ isn't supported, then zero out fields */
  {
    /* Convert QM-style zone info */

    pkt->orig_zone=pkt->qm_orig_zone;
    pkt->dest_zone=pkt->qm_dest_zone;
    pkt->orig_point=0;
    pkt->dest_point=0;

    pkt->cw=type=cREV2;
    pkt->rev_min=0;
  }

  if (pkt->orig_zone==0)
    pkt->orig_zone=config.addr->zone;

  if (pkt->dest_zone==0)
    pkt->dest_zone=config.addr->zone;
  
  return type;
}



/* Read a single message from a packet stream */

static int near TossReadMsgFromPkt(struct _inmsg *in)
{
  unsigned int ux;

  (void)Get_To_Nul(&in->pktprefix, &ux, in->pktfile, 1);

  /* Check for end of packet */
  
  if (ux <= 3 /* 2 - damn d'bridge! */ ||
      *(word *)(char *)(&in->pktprefix)==0 ||
      ux==0)
  {
    return -1;
  }

  if (ux < sizeof(struct _pktprefix) || in->pktprefix.ver != PKTVER)
  {
    S_LogMsg("!Grunged: %s, offset=%lx hex",
             in->pktname, GetPktOfs()-(long)ux);

    bad_packet=TRUE;
    return -1;
  }

  Get_TFS(in);

  if ((in->textptr=Get_To_Nul(&in->msg, &in->length, in->pktfile, 0))==NULL)
  {
    S_LogMsg("!Msg too long to toss:  skipping to next msg.");
    nmsg_skipped=TRUE;
    resume.long_packet=TRUE;
    return 1;
  }

  in->ctrl=MsgCreateCtrlBuf(in->textptr+sizeof(XMSG),
                            &in->txt, &in->length);

  /* if out of memory... */

  if (! in->ctrl)
    return -1;

  Copy_To_Header(in);

  /* Save the offset of the next message to process */
  
  part.offset=GetPktOfs();
  
  return 0;
}


/* Get the to/from/subject fields into the message header */

static void near Get_TFS(struct _inmsg *in)
{
  (void)memset(&in->msg, '\0', sizeof(XMSG));

  (void)strncpy(in->msg.__ftsc_date,
                Get_To_Nul(NULL, NULL, in->pktfile, 0),
                sizeof(in->msg.__ftsc_date));

  (void)strncpy(in->msg.to,
                Get_To_Nul(NULL, NULL, in->pktfile, 0),
                sizeof(in->msg.to));

  (void)strncpy(in->msg.from,
                Get_To_Nul(NULL, NULL, in->pktfile, 0),
                sizeof(in->msg.from));

  (void)strncpy(in->msg.subj,
                Get_To_Nul(NULL,NULL,in->pktfile,0),
                sizeof(in->msg.subj));

  /* Now make sure that they're NUL-terminated */

  in->msg.to[sizeof(in->msg.to)-1]='\0';
  in->msg.from[sizeof(in->msg.from)-1]='\0';
  in->msg.subj[sizeof(in->msg.subj)-1]='\0';
  in->msg.__ftsc_date[sizeof(in->msg.__ftsc_date)-1]='\0';
}



/* Copy other packet information to the message header */

static void near Copy_To_Header(struct _inmsg *in)
{
  NETADDR *orig,
          *dest;


  /* Zero out the message */

  orig=&in->msg.orig;
  dest=&in->msg.dest;

  /* If the tossed packet includes a zone number, use that.  Otherwise,     *
   * use the default zone from the config.                                  */

  orig->zone=(in->pkt.orig_zone ? in->pkt.orig_zone : config.addr->zone);
  dest->zone=(in->pkt.dest_zone ? in->pkt.dest_zone : config.addr->zone);

  /* Copy in the net information */

  dest->net=(word)in->pktprefix.dest_net;
  orig->net=(word)in->pktprefix.orig_net;

  dest->node=(word)in->pktprefix.dest_node;
  orig->node=(word)in->pktprefix.orig_node;

  orig->point=(word)in->pkt.orig_point;
  dest->point=(word)in->pkt.dest_point;


  /* Only scan kludge lines if this is NOT an echomail message */

  if (! strstr(in->ctrl, "\x01""AREA"))
    ConvertControlInfo(in->ctrl, orig, dest);



  /* Only allow the following bits to pass through */
  in->msg.attr=in->pktprefix.attr & (MSGPRIVATE | MSGCRASH | MSGREAD |
                                     MSGHOLD | MSGXX2 | MSGRRQ |
                                     MSGCPT | MSGARQ | MSGFILE);

  ASCII_Date_To_Binary(in->msg.__ftsc_date,
                       (union stamp_combo *)&in->msg.date_written);
                     
  (void)Get_Dos_Date((union stamp_combo *)&in->msg.date_arrived);
}





/* Allocate buffer for tossing messages */

static char * near AllocBigBuf(byte *txtp, byte *ctrl, unsigned num_sb)
{
  if (! msgb)
  {
    int size=sizeof(XMSG) + strlen(ctrl) + NumKludges(ctrl) +
                     strlen(txtp) + (num_sb*25) + 60 + MAX_TAGLEN;

    if ((msgb=malloc(size))==NULL)
    {
      S_LogMsg("!Not enough mem to toss/scan message!");

      /*S_LogMsg("@%d %d %d %d %d %d\n",
               (int)strlen(ctrl), (int)NumKludges(ctrl),
               (int)strlen(txtp), (int)(num_sb*25), 60, MAX_TAGLEN);*/

      (void)printf("Ran out of memory!\n");
      return NULL;
    }
  }

  msgbuf=msgb+MAX_TAGLEN;
  (void)memmove(msgbuf, txtp, strlen(txtp)+1);


  if (begin_sb)
    begin_sb=msgbuf+(unsigned)(ptrdiff_t)(begin_sb-txtp);

  if (end_sb)
    end_sb=msgbuf+(unsigned)(end_sb-txtp);

  return msgb;
}




/* Free the big buffer for tossing messages */

static void near DeallocBigBuf(void)
{
  if (msgb)
  {
    free(msgb);
    msgb=msgbuf=NULL;
  }
}




#ifdef OS_2

/* Handle DLL-specific features when tossing messages */

static void near InvokeTossFeatures(struct _inmsg *in, struct _cfgarea **ar, int *dokill, int *noscan, char *txtp)
{
  char tag[MAX_TAGLEN];
  struct _feat_toss ft;
  struct _feature *pf;

  /* Initialize the 'toss msg' structure and fill out defaults */
  (void)memset(&ft, 0, sizeof ft);

  ft.struct_len=sizeof ft;

  (void)strcpy(ft.szArea, (*ar)->name);
  (void)strcpy(tag, ft.szArea);   /* save copy of tag for comparison */
  (void)strcpy(ft.szPktName, in->pktname);

  ft.pMsg=&in->msg;
  ft.pszCtrl=in->ctrl;
  ft.pszMsgTxt=txtp;


  /* Loop through all of the features and call the 'toss message' function */

  for (pf=config.feat; pf; pf=pf->pfNext)
    if (pf->pfnTossMsg)
    {
      ft.ulTossAction=0;  /* initialize action to zero */

      /* Call the feature for this message */

      if ((*pf->pfnTossMsg)(&ft))
        exit(ERL_ERROR);

      if (ft.ulTossAction & FTACT_AREA)
        (void)strcpy(tag, ft.szArea);

      if (ft.ulTossAction & FTACT_KILL)
        *dokill=TRUE;

      if (ft.ulTossAction & FTACT_NSCN)
        *noscan=TRUE;

      if (ft.ulTossAction & FTACT_HIDE)
        break;
    }


  /* If the feature changed the area tag, toss it to that area instead */

  if (!eqstri((*ar)->name, tag))
  {
    struct _cfgarea arname, *newar;

    /* Search for this area's name */
      
    arname.name=ft.szArea;

    /* Search the skip list for it, and if found, toss to this area         *
     * instead.                                                             */

    if ((newar=SkipSearchList(config.area, &arname)) != NULL)
      *ar=newar;
  }
}

#endif




/* Find the appropriate netmail area for tossing this message */

static struct _cfgarea *GetNetArea(PXMSG px)
{
  struct _cfgarea *pn, *best;
  int found_zone=-1;

  for (pn=best=config.netmail; pn; pn=pn->next_type)
    if (pn->primary.zone==px->orig.zone && pn->primary.zone != found_zone)
    {
      best=pn;
      found_zone=pn->primary.zone;
    }

  return best;
}



/* Check all inbound netfile directories to make sure that an attached      *
 * file does not exist.                                                     */

static int near AttachedFileExists(char *pszFile)
{
  struct _tosspath *ptp;
  char path[PATHLEN];
  char *p;
  int i;

  /* Scan all of the possible toss-paths */

  for (ptp=config.tpath; ptp; ptp=ptp->next)
  {
    strcpy(path, ptp->path);

    /* Make sure that path ends with a "/" */

    if (path[i=strlen(path)-1] != '\\' && path[i] != '/')
      strcat(path, PATH_DELIMS);

    /* Also strip off any directory information from the attached file */

    if ((p=strrstr(pszFile, "\\/:")) != NULL)
      p++;
    else
      p=pszFile;

    strcat(path, pszFile);

    /* If the file exists, return TRUE */

    if (fexist(path))
      return TRUE;
  }

  return FALSE;
}



/* Determine whether or not the current message is secure */

static int near CheckInsecure(struct _cfgarea *ar, struct _inmsg *in)
{
  int insecure=FALSE;
  struct _sblist *sb;
 
  if (ar)
  {
    /* Check to see if we're not allowed to receive msg from this node */

    for (sb=ar->norecv; sb; sb=sb->next)      /* lint beware -------------v */
      if (MatchNS(&in->msg.orig, sb, ((config.flag & FLAG_CHECKZ)==0) ? 1 : 0))
      {
        insecure=TRUE;
        break;
      }

    if ((config.flag & FLAG_SECURE) && !NetmailArea(ar) &&
         !BadmsgsArea(ar) && !DupesArea(ar))
    {
      /* Check for secure message */

      for (sb=ar->scan; sb; sb=sb->next)    /* lint beware -------------v */
        if (MatchNS(&in->msg.orig, sb, ((config.flag & FLAG_CHECKZ)==0) ? 1 : 0))
          break;

      /* Node wasn't found in scan list, so it must be insecure */

      if (sb==NULL)
        insecure=TRUE;
    }

    if (insecure)
      (void)sprintf(comment, "Insec %s", ar ? ar->name : (byte *)"");
  }

  return insecure;
}



/* Toss a single message to the appropriate message area */

static int near TossOneMsg(struct _inmsg *in, int badmsg, word tflag)
{
  struct _sblist *sb;
  struct _cfgarea *ar;

  char temp[PATHLEN];
  byte *realstart;
  byte *savemb;
  byte *tearpos;
  byte *endtear;
  byte *txtp;

  dword msgn;

  unsigned num_sb;
  word dupe, insecure;

  int dokill=FALSE;
#ifdef OS_2
  int noscan=0;
#endif

  int ret;
  
#ifdef DCORE
  printf("\nToss mem=%ld\n", (long)coreleft());
#endif

  /* Make sure that the copy area for the msg is free */

  DeallocBigBuf();

  txtp=in->txt;

  /* If it's not addressed to us, drop it in netmail, without           *
   * bothering to check whether or not it's an EchoMail msg.            */

  *area_tag='\0';
  *comment='\0';

  ar=GetNetArea(&in->msg);

  if (! DestIsHere(&in->pktprefix))
  {
    if (! Process_Transient_Mail(in))
    {
      ret=FALSE;
      goto Done;
    }

    /* If the transient mail function copied it somewhere else,             */
    /* then make sure that we've accomodated for that.                      */
    
    if (msgb)   
      txtp=msgb+MAX_TAGLEN;
  }
  else
  {
    char *p;
    
    /* Now parse the AREA: line out of the kludges */

    if ((p=GetCtrlToken(in->ctrl, "AREA"))==NULL)
    {
      if (badmsg) /* don't toss "netmail" which is in bad_msgs */
      {
        ret=FALSE;
        goto Done;
      }
    }
    else
    {
      ar=Get_Area_Tag(in, p);
      MsgFreeCtrlToken(p);
    }
  }


  /* Check for a blank message */

  if (ar && NetmailArea(ar) && (config.flag & FLAG_KILLZ))
  {
    byte *p;

    /* Check for anything other than CR's or LF's. */

    for (p=txtp; *p=='\n' || *p=='\r' || *p==(byte)0x8d; )
      p++;

    if (*p==0)
    {
      /* The message seems to be blank.  However, we'll have to make sure   *
       * that there wasn't a valid file attached before we delete it.       */

      if ((in->pktprefix.attr & MSGFILE)==0 ||
           !AttachedFileExists(in->msg.subj) ||
           DestIsHereA(&in->msg.dest))
      {
        ret=TRUE;
        goto Done;
      }
    }
  }


  /* Check the security of this message */

  insecure=CheckInsecure(ar, in);

  if (insecure)
    ar=GetBadmsgsArea(in);

  /* Check for duplicate msgs */

  dupe=FALSE;
  

  /* If it's an echomail message... */

  if (ar && !BadmsgsArea(ar) && !NetmailArea(ar) && !DupesArea(ar))
  {
    /* Does this violate our TFLAG_NOECHO? */

    if (tflag & TFLAG_NOECHO)
    {
      S_LogMsg("!EchoMail message found in a NoEcho NetFile");
      NewArea("*NoEcho*");
      bad_packet=TRUE;
      ret=!badmsg;
      goto Done;
    }

    /* Make sure that the area is open */

    if ((config.flag & FLAG_ONEPASS)==0 || (ar->flag & AFLAG_PASSTHRU)==0)
      Open_Area(ar);

    /* Is this echo message a duplicate? */

    if (IsADupe(ar, &in->msg, in->ctrl, sq ? MsgGetNextUid(sq) : 0L))
    {
      Handle_Dupe(ar);

      dupe=TRUE;

      /* If we're not supposed to kill dupes, AND we're not already tossing *
       * from bad_msgs...                                                   */

      if ((config.flag & FLAG_KILLDUPE)==0 && !badmsg)
        ar=GetDupesArea(in);
      else
      {
        /* Display msg to indicate that we're killing dupes (unless we      *
         * toss from bad_msgs).                                             */

        if (!badmsg)
          NewArea("*KILL*");

        /* Return TRUE (tossed ok) if NOT tossing from bad msgs, or         *
         * if we are tossing from bad_msgs and killdupe is enabled.         */

        ret=!badmsg || (config.flag & FLAG_KILLDUPE);
        goto Done;
      }
    }
  }

  if (ar==NULL)
  {
    ret=TRUE;
    goto Done;
  }


  /* Strip private flag from message, if requested */

  if (ar->flag & AFLAG_STRIPPVT)
    in->msg.attr &= ~MSGPRIVATE;
  
  if ((badmsg && BadmsgsArea(ar)))
  {
    ret=FALSE;
    goto Done;
  }


#ifdef OS_2
  InvokeTossFeatures(in, &ar, &dokill, &noscan, txtp);
#endif

  /* Only open the area if we're NOT doing a one-pass toss/scan on a        *
   * passthru area.                                                         */
     
  if ((config.flag & FLAG_ONEPASS)==0 || (ar->flag & AFLAG_PASSTHRU)==0)
  {
    if (NetmailArea(ar))
      erl_net=TRUE;
    else erl_echo=TRUE;

    if (! Open_Area(ar))
    {
      UndoLastDupe();
      ret=FALSE;
      goto Done;
    }
  }
  else NewArea(ar->name);

  msglen=(dword)strlen(txtp)+1;
  
  /* Calculate statistics on messages that we've received */

  if (config.flag & FLAG_STATS)
  {
    ar->statarea.in_msgs++;
    ar->statarea.in_bytes += (dword)strlen(in->msg.from)+
                             (dword)strlen(in->msg.to)+
                             (dword)strlen(in->msg.subj)+
                             (dword)4+ /* for nul bytes */
                             (dword)strlen(in->ctrl)+
                             msglen;

    ar->flag |= AFLAG_STATDATA;
  }

  /* If we need to send this turkey */


  if ((config.flag & FLAG_ONEPASS) &&
#ifdef OS_2
      !noscan &&
#endif
      msglen > 0 &&
      NeedToEcho(txtp) &&
      ar->scan &&
      !NetmailArea(ar) &&
      !BadmsgsArea(ar) &&
      !DupesArea(ar))
  {
    /* Don't attempt to rescan (or set HWM for) a passthru area */

    if ((ar->flag & AFLAG_PASSTHRU)==0)
    {
      if (MsgGetHighWater(sq) != MsgHighMsg(sq))
      {
        if ((config.flag2 & FLAG2_QUIET)==0)
          (void)printf("\n");

        savemb=msgbuf;

        realstart=msgbuf=smalloc(maxmsglen+(unsigned)sizeof(XMSG));

        /* Space to insert the 'AREA:' line */
        msgbuf += MAX_TAGLEN;

        Scan_Area(ar, sq);

        free(realstart);
        msgbuf=savemb;

        if ((config.flag2 & FLAG2_QUIET)==0)
        {
          Tossing_It(in->pktname);

          Tossing_From((word)in->pkt.orig_zone, (word)in->pkt.orig_net,
                       (word)in->pkt.orig_node, (word)in->pkt.orig_point);

          (void)printf("\b\b\b\b\b\b\b%s: -----", ar->name);
        }
      }
    }


    /* If there are >= 2 nodes in areas.bbs, or if the first node listed  *
     * in areas.bbs wasn't the one we received it from, try to scan it.   */

    if ((ar->scan && ar->scan->next) ||
        !AddrMatchNS(&in->msg.orig, ar->scan))
    {
      /* Convert seen-bys to a binary linked list */

      sb=Digitize_Seenbys(ar, txtp, &num_sb);

      /* If there weren't any seenbys, try to turf this bugger... */

      if (sb==NULL || num_sb==0)
      {
        ar=config.badmsgs;

        (void)strcpy(comment, "No SEEN-BYs");

        /* Make sure that this doesn't get written to the duplicate buffer  *
         * of the area we tried to toss to.                                 */
           
        UndoLastDupe();

        insecure=TRUE;

        if (badmsg)
        {
          ret=FALSE;
          goto Done;
        }

        if (ar==NULL || !Open_Area(ar))
        {
          ret=TRUE;
          goto Done;
        }
      }
      else
      {
        /* Sort the list, and convert linked list into a flat array */

        sb=Sort_And_Flatten(sb, num_sb, ar->num_scan+config.num_ats+1);


        if (! AllocBigBuf(txtp, in->ctrl, num_sb))
        {
          free(sb);
          ret=FALSE;
          goto Done;
        }

        txtp=msgbuf;

        if (Scan_To_All(ar, sb, num_sb, &in->msg, in->ctrl, &txtp, 0L, &dokill, sq, NULL))
        {
          in->msg.attr |= MSGSCANNED;

          /* Sent a one-pass msg */

          toss_cnt++;
        }


        /* And free the SEEN-BY list... */

        free(sb);

        /* Recalculate message length */

        msglen=(dword)strlen(txtp)+1;
      }
    }
    else
    {
      int fSkipScan=FALSE;

      if (AllocBigBuf(txtp, in->ctrl, 0))
        ProcessACUpdate(sq, ar, &in->msg, in->ctrl, &dokill,
                        0L, NULL, 0, &fSkipScan);
    }
  }




  /* Chop off seen-bys, if required */

  if ((config.flag & FLAG_ONEPASS) && 
      (config.flag & FLAG_SAVECTRL)==0 &&
      (ar->type & MSGTYPE_SQUISH) &&
      !NetmailArea(ar) &&
      !BadmsgsArea(ar) &&
      !DupesArea(ar))
  {
    tearpos=GetTearPos(txtp,&endtear);

    if (tearpos && endtear)
    {
      msglen=(unsigned)(endtear-txtp);
      txtp[(int)msglen++]='\0';
    }
  }


  if (((config.flag & FLAG_ONEPASS) && (ar->flag & AFLAG_PASSTHRU)) || dokill)
    msgn=0L;
  else
  {
    HMSG msgh;

    ar->flag |= AFLAG_TOSSEDTO;

    if ((msgh=MsgOpenMsg(sq, MOPEN_CREATE, 0L))==NULL)
    {
      if (msgapierr==MERR_NODS)
      {
        UndoLastDupe();
        DiskFull(ar);
      }
      else
      {
        S_LogMsg("!Error creating message in area %s! (%d)",
                 ar->name, msgapierr);

        bad_packet=TRUE;

        if (ar)
          DupeFlushBuffer();
      }

      ret=FALSE;
      goto Done;
    }


    /* Add the private bit, if instructed to do so */

    if (ar->flag & AFLAG_HIDEMSG)
      in->msg.attr |= MSGPRIVATE;


    /* Kill in-transit messages */

    if (NetmailArea(ar) && (config.flag & FLAG_KILLFWD) &&
        (config.flag & FLAG_FRODO))
    {
      in->msg.attr |= MSGKILL;
    }

    /* Write the header, kludge lines and body.  Re-add the area tag, if    *
     * this message is a duplicate.                                         */

    if (dupe || insecure)
    {
      unsigned len;
      
      (void)sprintf(temp, "\x01""AREA:%s", area_tag);
      len=strlen(temp);

      /* Shove the tag in at the beginning */
      
      (void)strocpy(in->ctrl+len, in->ctrl);
      (void)memmove(in->ctrl, temp, strlen(temp));
    }
    
    if (MsgWriteMsg(msgh, FALSE, &in->msg, txtp, (dword)msglen, (dword)msglen,
                    (dword)strlen(in->ctrl)+1, in->ctrl)==-1)
    {
      UndoLastDupe();

      if (msgapierr==MERR_NODS)
      {
        (void)MsgCloseMsg(msgh);
        DiskFull(ar);
        ret=FALSE;
        goto Done;
      }

      S_LogMsg("!Error writing msg to %s (%d)", ar->name, msgapierr);

      if (msgapierr==MERR_BADF || msgapierr==MERR_BADMSG)
        S_LogMsg("!(Possibly-grunged area.)");
      else if (msgapierr==MERR_NOMEM)
        S_LogMsg("!(Not enough memory.)");
      else if (msgapierr==MERR_SHARE || msgapierr==MERR_EACCES)
        S_LogMsg("!(Sharing violation.)");

      bad_packet=TRUE;
    }

    (void)MsgCloseMsg(msgh);

    msgn=MsgHighMsg(sq);
  }

  if ((config.flag2 & FLAG2_QUIET)==0 &&
      ((ar->flag & AFLAG_PASSTHRU)==0 || (config.flag & FLAG_ONEPASS)==0))
    (void)printf("\b\b\b\b\b%05lu", msgn);

  #if defined(__WATCOMC__) && !defined(NT)
  fflush(stdout);
  #endif

  nmsg_tossed++;
  ar->tossed++;

  if ((config.flag & FLAG_ONEPASS) && !NetmailArea(ar) && !BadmsgsArea(ar) &&
      !DupesArea(ar) && (ar->flag & AFLAG_PASSTHRU)==0)
  {
    (void)MsgSetHighWater(sq,MsgHighMsg(sq));
  }

  ret=TRUE;

Done:

  MsgFreeCtrlBuf(in->ctrl);
  DeallocBigBuf();

  return ret;
}


static int near Process_Transient_Mail(struct _inmsg *in)
{
/*char temp[PATHLEN];
  int x;*/

  NW(config);

  in->msg.attr |= MSGFWD;

#if 1

  (void)sprintf(comment, "Fwd %hd/%hd", in->pktprefix.dest_net,
                in->pktprefix.dest_node);

#else

  (void)sprintf(temp,"%s (Fwd %hd/%hd)",
                *last_area ? "" : "       ",
                in->pktprefix.dest_net,in->pktprefix.dest_node);

  if ((config.flag2 & FLAG2_QUIET)==0)
  {
    (void)printf(temp);

    for (x=strlen(temp); x--; )
      (void)printf("\b");
  }
#endif

#ifdef NEVER
  /* Copy everything to a static buffer, so we don't disturb the            *
   * bytes just read in from the .PKT file.  This is so we can modify the   *
   * message by adding our ^aVia line, below.                               */

  if (! AllocBigBuf(in->txt, 1))
  {
    S_LogMsg("#Warning: Not enough mem to tag");
    return FALSE;
  }
#endif

  return TRUE;
}




/* Get the name of the area to which we are tossing */

static struct _cfgarea * near Get_Area_Tag(struct _inmsg *in, char *txt)
{
  struct _cfgarea *ar, arname;
  char *p, *o;

  ar=NULL;

  /* Skip spaces which are placed after the "AREA:" by the        *
   * brain-tead QECHO, and copy everything else into the buffer.  */

  for (p=txt+5, o=area_tag; *p && o < area_tag+MAX_TAGLEN-1; p++)
    if (*p != ' ')
      *o++=*p;

  *o='\0';


  /* Search for this area's name */
      
  arname.name=area_tag;


  /* Now check the skip list for it.  If it's not found, or if it's         *
   * not an echomail area...                                                */

  if ((ar=SkipSearchList(config.area, &arname))==NULL ||
      (ar->flag & (AFLAG_BAD|AFLAG_DUPES|AFLAG_NET)))
  {
    if ((config.flag2 & FLAG2_QUIET)==0)
    {
      (void)sprintf(comment, "Unknown %s", area_tag);
    }

    return config.badmsgs;
  }

  /* Now remove the AREA: thingy from the kludge junk */

  RemoveFromCtrl(in->ctrl, t_area);

  return ar;
}



/* Handle the area name if we get a dupe msg */

static void near Handle_Dupe(struct _cfgarea *ar)
{
  ar->dupes++;
  
  if (ar->name)
    (void)sprintf(comment, "Dupe %s", ar->name);
  else
    (void)strcpy(comment, "Dupe");
}



/* Switch to a new area and print an appropriate message */

static void near NewArea(char *name)
{
  unsigned len;

  /* If we're onto a new area */

  if (! eqstri(last_area, name) || !eqstri(last_comment, comment))
  {
    len=strlen(last_area)+strlen(last_comment)+(*last_comment ? 3 : 0);

    /* Back up over message number */
    
    if ((config.flag2 & FLAG2_QUIET)==0)
    {
      (void)printf("\b\b\b\b\b\b\b       \b\b\b\b\b\b\b");

      while (len--)       /* Back up over area name */
        (void)printf("\b \b");

      (void)strcpy(last_area, name);
      (void)strcpy(last_comment, comment);

      if (*comment)
        (void)printf("%s (%s): -----", name, comment);
      else (void)printf("%s: -----", name);
    }
  }
}


/* Open a new message area */

static int near Open_Area(struct _cfgarea *ar)
{
  struct _talist savetl={0};
  word an;
  
  if (!ar)
    return FALSE;

  /* If this is the first time around, initialize the list of areas */
  
  if (!talist_init)
  {
    talist_init=TRUE;

    for (an=0; an < config.max_talist; an++)
    {
      talist[an].sq=NULL;
      talist[an].ar=NULL;
    }
  }

  /* Now, check to see if we need to change areas */
  
  if (last_sq != ar)
  {
    /* If so, then scan the list of areas in 'talist' to see if this area   *
     * is already open.  We're keeping all of the most-recently-used        *
     * area handles, with talist[0] being the most recently used, and       *
     * talist[MAX_TALIST-1] being the least-recently used.  What we're      *
     * going to do is to shift a segment of the talist[] buffer up by one   *
     * structure, to insert the new most-recent area in talist[0].  First,  *
     * we search through the talist[] to see if the current area is already *
     * there.  If it's found at element 'x', we move everything from        *
     * talist[0..x-1] to talist[1..x], and then copy 'savetl' into          *
     * talist[0], which is the desired effect.                              */
       
    for (an=0; an < config.max_talist; an++)
      if (ar==talist[an].ar)
      {
        savetl=talist[an];
        break;
      }

    /* If the area was found in our LRU buffer, shift it to the beginning */
      
    if (an != config.max_talist)
    {
      (void)memmove(talist+1, talist, an*sizeof(struct _talist));
      talist[0]=savetl;
      sq=talist[0].sq;
    }
    else /* otherwise, we'll have to open a new area */
    {
      /* Close the old area we just shoved off */
      
      savetl=talist[config.max_talist-1];

      /* Shift the array apropriately */

      (void)memmove(talist+1, talist,
                    (config.max_talist-1) * sizeof(struct _talist));

      /* Blank out the first record, "just in case" */

      talist[0].sq=NULL;
      talist[0].ar=NULL;
      
      /* Now physically close the area */

      if (savetl.sq)
      {
        (void)MsgUnlock(savetl.sq);
        (void)MsgCloseArea(savetl.sq);
        
        savetl.sq=NULL;
        savetl.ar=NULL;
      }
      
      /* Attempt to open the new area */
      
      if ((sq=MsgOpenArea(ar->path, MSGAREA_CRIFNEC, ar->type))==NULL)
      {
        S_LogMsg("!Can't create area %s", ar->path);
        last_sq=NULL;
        return FALSE;
      }
      
      if ((ar->sq_max_msgs || ar->sq_save_msgs || ar->sq_keep_days) &&
          (ar->type & MSGTYPE_SQUISH))
      {
        dword max, skip, days;
        
        max= ar->sq_max_msgs  ? ar->sq_max_msgs  : (dword)-1;
        skip=ar->sq_save_msgs ? ar->sq_save_msgs : (dword)-1;
        days=ar->sq_keep_days ? ar->sq_keep_days : (dword)-1;
        
        
        SquishSetMaxMsg(sq, max, skip, days);
      }

      (void)MsgLock(sq);

      talist[0].sq=sq;
      talist[0].ar=ar;
    }

    last_sq=talist[0].ar;
  }
  
  NewArea(ar->name);

  return TRUE;
}



/* Close an existing area */

static void near Close_Area(void)
{
  word an;

  for (an=0; an < config.max_talist; an++)
    if (talist[an].sq)
      (void)MsgCloseArea(talist[an].sq);

  sq=NULL;
}



/* Zero out the statistics count for this pass */

void Zero_Statistics(void)
{
  struct _cfgarea *ar;
  
  for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
  {
    ar->dupes=0;
    ar->tossed=0;
    ar->sent=0;
  }
}


/* Log the statistics for this pass */

void Report_Statistics(void)
{
  struct _cfgarea *ar;
  char logfmt[PATHLEN];

  for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
    if (ar->dupes || ar->tossed || ar->sent)
    {
      (void)strcpy(logfmt,":   %-14s (Toss=%04hu");

      if (ar->dupes || ar->sent)
        (void)strcat(logfmt,"  Sent=%04hu");

      if (ar->dupes)
        (void)strcat(logfmt,"  Dupe=%04hu");

      (void)strcat(logfmt,")");

      S_LogMsg(logfmt, ar->name, ar->tossed, ar->sent, ar->dupes);
    }
}



/* Determine whether or not the current packet is secure */

static int near PacketSecure(struct _inmsg *in)
{
  char pktpwd[20];
  struct _sblist fr;
  struct _nodepwd *np;


  fr.zone=(word)in->pkt.orig_zone;
  fr.net=(word)in->pkt.orig_net;
  fr.node=(word)in->pkt.orig_node;
  fr.point=(word)in->pkt.orig_point;

  for (np=config.pwd; np; np=np->next)  /* lint beware ----------v */
    if (MatchSS(&fr, &np->addr, ((config.flag & FLAG_CHECKZ)==0) ? 1 : 0))
    {
      (void)memmove(pktpwd, in->pkt.password, 8);
      pktpwd[8]='\0';

      if (! eqstri(pktpwd, np->pwd))
      {
        S_LogMsg("!Bad pwd in packet: got=`%s', expected=`%s'",
                 pktpwd, np->pwd);

        return FALSE;
      }

      break;
    }

  return TRUE;
}



/* Get the first bad messages area */

static struct _cfgarea *GetBadmsgsArea(struct _inmsg *in)
{
  NW(in);
  return config.badmsgs;
}



/* Get the first dupes area */

static struct _cfgarea *GetDupesArea(struct _inmsg *in)
{
  NW(in);
  return config.dupes;
}



/* Toss messages from all bad_msgs areas */

static void near TossBadMsgs(struct _cfgarea *ar)
{
  struct _inmsg in;
  XMSG *msg;
  HAREA harea;
  HMSG hmsg;
  UMSGID uid;
  byte *ctrl;
  dword mn;
  int tossed;

  msg=(XMSG *)buffer;

  for (; ar; ar=ar->next_type)
  {
    if ((harea=MsgOpenArea(ar->path, MSGAREA_NORMAL, ar->type))==NULL)
      continue;
    
    (void)MsgLock(harea);

    if (MsgGetNumMsg(harea))
    {
      S_LogMsg("#Tossing from %s (%ld msgs)", ar->path, MsgGetNumMsg(harea));

      Zero_Statistics();

      /* Indicate that we're tossing BAD_MSGS */

      StatsWriteBlock(TYPE_BTOSS, 0, 0, NULL);


      /* Now loop through all of the msgs in this area */

      for (mn=1L; mn <= MsgHighMsg(harea); mn++)
      {
        if ((hmsg=MsgOpenMsg(harea, MOPEN_READ, mn))==NULL)
        {
          if (msgapierr != MERR_NOENT)
            S_LogMsg("!Can't open msg %s:%lu", ar->name, mn);

          continue;
        }

        if (MsgGetTextLen(hmsg) + MsgGetCtrlLen(hmsg) + 80 > maxmsglen ||
            (ctrl=malloc((size_t)MsgGetCtrlLen(hmsg)))==NULL)
        {
          S_LogMsg("!Message %s:%lu too long - skipped", ar->name, mn);
        }
        else
        {
          if (MsgReadMsg(hmsg, msg, 0L, MsgGetTextLen(hmsg)+1,
                         buffer+sizeof(XMSG),
                         MsgGetCtrlLen(hmsg), ctrl) > 0)
          {
            (void)sprintf(in.pktname, "%s:%lu", ar->name, mn);

            /* If it's in bad_msgs, we have no reliable way to determine    *
             * zone or point information.                                   */

#if 0 /* apparently, people liked this "misfeature" */
            if (ar->type & MSGTYPE_SDM)
            {
              msg->orig.zone=config.def.zone;
              msg->orig.point=0;
            }
#endif

            *last_area='\0';
            *last_comment='\0';
            Tossing_It(in.pktname);

            Tossing_From(msg->orig.zone, msg->orig.net,
                         msg->orig.node, msg->orig.point);

            in.pktfile=-1;
            in.msg=*msg;

            in.pkt.orig_zone= msg->orig.zone;
            in.pkt.orig_net=  (sword)msg->orig.net;
            in.pkt.orig_node= (sword)msg->orig.node;
            in.pkt.orig_point=msg->orig.point;

            in.pktprefix.orig_net=(sword)msg->orig.net;
            in.pktprefix.orig_node=(sword)msg->orig.node;

            in.pkt.dest_zone= msg->dest.zone;
            in.pkt.dest_net=  (sword)msg->dest.net;
            in.pkt.dest_node= (sword)msg->dest.node;
            in.pkt.dest_point=msg->dest.point;

            in.pktprefix.dest_net=(sword)msg->dest.net;
            in.pktprefix.dest_node=(sword)msg->dest.node;

            in.pktprefix.ver=PKTVER;
            in.pktprefix.attr=(word)msg->attr;
            in.pktprefix.cost=0;

            in.textptr=buffer;
            in.txt=buffer+sizeof(XMSG);
            in.ctrl=ctrl;
            in.length=(word)(MsgGetTextLen(hmsg)+sizeof(XMSG));

            tossed=TossOneMsg(&in, TRUE, 0);

            if (*comment && !tossed)
              (void)printf(" (%s)", comment);

            (void)printf("\n");

            /* If it was tossed okay, delete this message from bad_msgs */

            if (tossed)
            {
              (void)MsgCloseMsg(hmsg);
              hmsg=NULL;

              uid=MsgMsgnToUid(harea, mn);
              (void)MsgKillMsg(harea, mn);
              mn=MsgUidToMsgn(harea, uid, UID_PREV);
            }
          }
          else
          {
            /* ctrl is freed by tossonemsg() */
            free(ctrl);
          }
        }

        if (hmsg)
          (void)MsgCloseMsg(hmsg);
      }

      StatsWriteAreas();
      Report_Statistics();
    }
    
    (void)MsgUnlock(harea);
    (void)MsgCloseArea(harea);
  }
}


/*
void widget()
{
  coreleft();
}
*/

