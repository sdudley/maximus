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

/* $Id: s_misc.c,v 1.6 2004/01/22 08:04:28 wmcbrine Exp $ */

#define NOVARS
#ifdef SWAP
#include "../swap/swap.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dos.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <time.h>
#include <ctype.h>
#include <process.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "crc.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "s_dupe.h"

#if defined(__TURBOC__) || defined(__MSC__)

void cdecl _junk_cdecl_proc(void)
{
  /* to get around incompatibility in MS-LINK */
}

#endif

void Alloc_Buffer(unsigned size)
{
  if (buffer)
    Free_Buffer();

  if ((buffer=malloc(size + 20u /* for safety */))==NULL)
    NoMem();

  orig_buffer=buffer;
}

void Free_Buffer(void)
{
  if (buffer)
    free(buffer);

  buffer=orig_buffer=NULL;
}




/* Buffered input from packet */

static unsigned chs=0;
static long read_ofs;

/* External program saying that we've modified the packet position */

void SetPktOfs(long ofs)
{
  read_ofs=ofs;
}

/* Return the current offset (taking buffering into account) of the current *
 * packet file.                                                             */

long GetPktOfs(void)
{
  return (read_ofs-(long)chs);
}

static void near Skip_To_Nul(int pktfile, char *buf, unsigned buflen)
{
  char *s;
  unsigned got;

  for (s=NULL; (got=(unsigned)fastread(pktfile, buf, buflen)) != 0; )
    if ((s=memchr(buf, '\0', got)) != NULL)
      break;

  if (s)
    (void)lseek(pktfile,-((long)got-(long)(s-buf))+1L,SEEK_CUR);
}




char *Get_To_Nul(void *msg, unsigned int *length, int pktfile, int gp)
{
  byte *b=buffer+sizeof(XMSG);
  byte *p, *s=NULL;
  unsigned got;

  if (pktfile==-1)
  {
    orig_buffer=buffer;
    chs=0;
    return NULL;
  }

  if (gp)
  {
    /* If there's not enough left... */
    if (chs < sizeof(struct _pktprefix))
    {
      /* Move what's left to start of buffer... */

      (void)memmove(b, orig_buffer, chs);


      /* And refill it! */

      got=(unsigned)fastread(pktfile, b+chs, maxmsglen-chs);

      if (got != 0 && got != (unsigned)-1)
      {
        /* Increment the file pointer */

        chs += got;
        read_ofs += (long)got;
      }

      orig_buffer=b;
    }

    got=min(sizeof(struct _pktprefix), chs);


    /* Copy the structure where we need it */

    (void)memmove(msg, orig_buffer, got);


    /* And adjust our pointers... */

    orig_buffer += got;
    chs -= got;

    /* Return number of chars */

    *length=got;
    return NULL;
  }

  /* Copy the message into the right spot, if required. We're copying      *
   * the message into this array so we can get everything with just        *
   * ONE write() call.  That also means we have to push everything up      *
   * to the front of the buffer.                                           */

  if (msg)
  {
    (void)memmove(buffer, msg, sizeof(XMSG));
    (void)memmove(b, orig_buffer, chs);
    orig_buffer=b;
  }

  if (!chs)
  {
    orig_buffer=b;
    got=(unsigned)fastread(pktfile, b, maxmsglen);

    if (got > 0)
    {
      chs += got;
      read_ofs += (long)got;
    }
  }

  for (;;)
  {
    if ((p=memchr(orig_buffer, '\0', chs)) != NULL)
    {
      /* Got it!  Adjust pointers and return */

      s=orig_buffer;

      chs -= (unsigned)((++p)-orig_buffer);
      
      orig_buffer=p;
      break;
    }
    else if (chs < maxmsglen)
    {
      (void)memmove(b, orig_buffer, chs);

      got=(unsigned)fastread(pktfile, (orig_buffer=b)+chs, maxmsglen-chs);

      if (got != 0 && got != (unsigned)-1)
      {
        chs += got;
        read_ofs += got;
      }
      else
      {
        /* eof - return everything we got anyway, and treat it as a NUL */

        s=orig_buffer;
        p=orig_buffer+chs;
        *p++='\0';    /* make sure it's capped with a NUL */
        chs=0;
        break;
      }
    }
    else
    {
      chs=0;
      orig_buffer=b;
      Skip_To_Nul(pktfile, buffer, maxmsglen);
      return NULL;
    }
  }

  if (length)
    *length=(unsigned)(p - b) + (unsigned)(msg ? sizeof(XMSG) : 0);

  if (msg)
    return buffer;
  else return s;
}



dword crcstr(dword crc,byte *s)
{
  byte *e;

  /* Only count non-control characters and non-spaces. */

  for (e=s+strlen(s);s < e;s++)
    if (*s > ' ' && *s < 127)
      crc=xcrc32(*s,crc);

  return crc;
}

void DiskFull(struct _cfgarea *ar)
{
  S_LogMsg("!Error writing msg to %s; no disk space!", ar->name);

  bad_packet=TRUE;

  /* Flush duplicate buffer */

  if (ar)
    DupeFlushBuffer();
}


/* Display an error message when we run out of memory */

void _fast NoMem(void)
{
  S_LogMsg("!Ran out of memory.  Aborting abnormally. (mem=%ld)",
            coreleft());

  (void)printf("\nRan out of memory!\n");

  S_LogClose();
  exit(ERL_ERROR);
}


/* Display an error message when we cannot open a file */

void ErrOpening(char *name,char *filename)
{
  (void)printf("Fatal error opening %s file `%s'!\n", name, fancy_fn(filename));
  exit(ERL_ERROR);
}


/* Find the next occurance of a token, as the FIRST item on a line */

char *GetNextToken(byte *start, byte *buf, byte *token)
{
  byte *p, *s;

  if (buf==NULL)
    return NULL;

  p=buf;

  while (p && (p=strstr(p, token)) != NULL)
  {
    s=p;    /* Save current pointer */

    /* If the prior character is a CR, this may be it.  Scan back just      *
     * to make sure that we can find a hard CR (0x0d).                      */

    if (p[-1]=='\x0d' || p[-1]=='\x0a' || (byte)p[-1]==(byte)0x8d)
      while (p-- > start) /* Scan back to find the first non-soft-CR */
        if (p > start && *p != '\x0a' && (byte)*p != (byte)0x8d)
          break;

    if (*p=='\x0d') /* If the first thing before the soft CR's was a */
      return s;     /* a hard CR, then we've found it.               */
    else p=strchr(s+1, '\x0d');
  }

  return NULL;
}

/* Return a pointer to the tear/origin line in message buffer, or NULL      *
 * if not found.                                                            */

byte *GetTearPos(byte *msgbuf, byte **endtear)
{
  byte *found, *estart;
  byte *sv;
  byte *s;
  
  found=msgbuf;
  
  while ((found=GetNextToken(msgbuf, found, "---")) != NULL)
  {
    s=found;

    s=strchr(s, '\x0d'); /* Skip to next '\x0d' */

    if (!s) /*It shoulda been there, but still... */
      s=found+strlen(found);

    /* Skip over end of line */

    while (*s=='\x0d' || *s=='\x0a' || *s==(byte)0x8d)  
      s++;

    /* If we have an origin line */

    if (s[0]==' ' && s[2]==' ' && s[3]=='O' && s[4]=='r' && s[5]=='i' &&
        s[6]=='g' && s[7]=='i' && s[8]=='n' && s[9]==':')
    {
      extern char *seen_by_str;

      /* Find end of tearline */

      sv=s;
      s=strchr(s, '\x0d');

      if (!s)
        s=sv+strlen(sv);

      /* Skip over junk */

      while (/**s=='\x0d' ||*/ *s=='\x0a' || *s==(byte)0x8d)
        s++;
        
      /* Skip over one \x0d at most */

      if (*s=='\x0d')
        s++;

      estart=s;

      /* Now find end of the message, after tearline */

      while (*s && (eqstrn(s, seen_by_str, 8) || *s <= ' '))
      {
        while (*s && *s != '\x0d')
          s++;
        
        if (*s=='\x0d')
          s++;
        
        while (*s && (*s=='\x0a' || *s==(byte)0x8d))
          s++;
      }
      
      /* If there was junk after the tear line, it must not've been it. */

      if (*s != '\0')
        return NULL;
      
      if (endtear)
        *endtear=/* s */ estart;
      
      return found;
    }

    found++;  /* So that GetNextToken() doesn't return the current token */
  }
 
  if (endtear)
    *endtear=NULL;
  
  /* If we got this far, then it wasn't found */
  return NULL;
}


int DestIsHere(struct _pktprefix *prefix)
{
  NETADDR paddr;
  
  paddr.zone=0;
  paddr.net=(word)prefix->dest_net;
  paddr.node=(word)prefix->dest_node;
  paddr.point=POINT_ALL;
  
  return (DestIsHereA(&paddr));
}

int DestIsHereA(NETADDR *dest)
{
  struct _sblist *addr;

  
  for (addr=config.addr; addr; addr=addr->next)
    if (AddrMatchNS(dest, addr))
      return TRUE;

  return FALSE;
}

char *FixOutboundName(word zone)
{
  struct _outb *pob;
  static char outname[PATHLEN];
  word len;

  /* Make sure that zone number is within bounds */

  if (zone > 0xfff && zone != 0xffff)
    zone=config.addr->zone;

  /* Set a default outbound directory */

  (void)strcpy(outname, config.outb->dir);

  for (pob=config.outb; pob; pob=pob->next)
  {
    if (pob->zone==zone || pob->zone==0)
    {
      (void)strcpy(outname, pob->dir);

      /* Strip trailing backslashes off dir name */

      if ((len=(word)strlen(outname)) != 3)
        while (outname[len]=='\\' || outname[len]=='/')
          outname[len--]='\0';

      /* If it's not our default zone, or if we're in FD mode, use OUT.SQ */

      if (pob->zone != config.addr->zone || (config.flag & FLAG_FRODO))
      {
        if (zone==0xffff || (config.flag & FLAG_FRODO))
          (void)strcat(outname, ".sq");
        else if (zone != config.addr->zone)
          (void)sprintf(outname+strlen(outname), ".%03hx", zone);
      }

      break;
    }
  }

  (void)Add_Trailing(outname, PATH_DELIM);
  
  if (!direxist(outname))
    (void)make_dir(outname);

  return outname;
}

void Fill_Out_Pkthdr(struct _pkthdr *ph,
                     struct _sblist *us,
                     word them_zone, word them_net, word them_node, word them_point)
{
  struct _nodepwd *np;
  struct _sblist sb;
  time_t gmt;
  struct tm *lt;

  /* Get current time */

  gmt=time(NULL);
  lt=localtime(&gmt);

  /* Zero out entire packet header */

  (void)memset(ph, '\0', sizeof(struct _pkthdr));
  
  ph->orig_node=(sword)us->node;         /* Fill in addresses */
  ph->dest_node=(sword)them_node;
  ph->year=(word)lt->tm_year+1900;
  ph->month=(word)lt->tm_mon;
  ph->day=(word)lt->tm_mday;
  ph->hour=(word)lt->tm_hour;
  ph->minute=(word)lt->tm_min;
  ph->second=(word)lt->tm_sec;
  ph->baud=0;
  ph->ver=PKTVER;
  ph->orig_net=(sword)us->net;
  ph->dest_net=(sword)them_net;
  ph->product=pMax;
  ph->rev_maj=SQUISH_REV_MAJ;
  ph->rev_min=SQUISH_REV_MIN;
  ph->password[0]='\0';
  ph->qm_orig_zone=ph->orig_zone=us->zone;
  ph->qm_dest_zone=ph->dest_zone=them_zone;
  ph->aux_net=us->net;

  ph->cw=cREV2P;
  
  /* Swap the byte order of the capability words */
  
  ph->cw_validation=(word)((word)ph->cw >> 8) | (word)((word)ph->cw << 8);

  /* Fill in other 4D info */

  ph->orig_point=us->point;
  ph->dest_point=them_point;
  ph->product_hi=0;

/*if (us->point)*/          /* Kludge using the pkt.aux_net field     */
/*  ph->orig_net=-1;*/      /* for compatibility with other<tm> progs */

  ph->prod_data=0x544b5058L; /* "XPKT" */

  /* Now check to see if we need to add a password for this node,     *
   * and if found, add it accordingly.                                */

  sb.zone=them_zone;
  sb.net=them_net;
  sb.node=them_node;
  sb.point=them_point;

  for (np=config.pwd; np; np=np->next)
  {
    if (AddrMatchS(&sb, &np->addr))
    {
      (void)strncpy(ph->password, np->pwd, 8);
      break;
    }
  }
}




/* Compare two configuration areas by name */

static int _stdc CfgareaComp(void *a1, void *a2)
{
  return (stricmp(((struct _cfgarea *)a1)->name,
                  ((struct _cfgarea *)a2)->name));
}




void Initialize_Variables(void)
{
  (void)memset(&config, '\0', sizeof(struct _config));

  config.areasbbs=sstrdup("");
  config.routing=sstrdup("");
  config.logfile=sstrdup("");
  config.tracklog=sstrdup("");
  config.swappath=sstrdup("");
  config.origin=sstrdup("None");
  config.compress_cfg=sstrdup("compress.cfg");
  config.statfile=sstrdup("squish.stt");
  config.max_talist=OS_MAX_TALIST;
  config.max_handles=OS_MAX_HANDLES;
  config.dupe_msgs=1000;
  config.maxpkt=config.maxattach=128;
  config.loglevel=6;

  config.flag2 |= FLAG2_DMSGID | FLAG2_DHEADER;

  /* Create a skip list to hold the area configuration */

  if ((config.area=SkipCreateList(16, 8, CfgareaComp))==NULL)
    NoMem();
}




void Cleanup(void)
{
  struct _cfgarea *ar;
  struct _sblist *sb, *nextsb;
  struct _arcinfo *ap, *nextap;
  struct _remap *rem, *nextrem;
  struct _nodepwd *pwd, *nextpwd;
  struct _fwdlist *fwd, *nextfwd;
  struct _statlist *sl, *nextsl;
  struct _groute *gr, *nextgr;
  struct _tosspath *tp, *nexttp;
  struct _outb *pob, *nextpob;

#ifdef OS_2 /* Free the list of DLL features */
  struct _feature *feat, *nextfeat;

  for (feat=config.feat; feat; nextfeat=feat->pfNext, free(feat), feat=nextfeat)
  {
    struct _feat_term ft;

    /* Prepare the termination structure */

    ft.struct_len=sizeof(ft);

    /* Tell the DLL to finish up */

    (void)(*feat->pfnTerm)(&ft);

    /* Free memory used by llist */

    if (feat->pszDLLName)
      free(feat->pszDLLName);

    if (feat->pszConfigName)
      free(feat->pszConfigName);

    /* Free the dynamic link library itself */

    (void)DosFreeModule(feat->hmod);
  }

#endif
  
  for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
  {
    /* Free all of the nodes we're scanning to */
    
    for (sb=ar->scan; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;

    for (sb=ar->update_ok; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;

    for (sl=ar->statlist; sl; nextsl=sl->next, free(sl), sl=nextsl)
      ;

    free(ar->path);
    free(ar->name);
    free(ar);
  }
  
  (void)SkipDestroyList(config.area);

  for (ap=config.arc; ap; nextap=ap->next, (free)(ap), ap=nextap)
  {
    if (ap->arcname)
      (free)(ap->arcname);

    if (ap->extension)
      (free)(ap->extension);
    
    if (ap->add)
      (free)(ap->add);
    
    if (ap->extract)
      (free)(ap->extract);
    
    if (ap->view)
      (free)(ap->view);

    if (ap->id)
      (free)(ap->id);
    
    /* Free all of the nodes to use for this archiver */
    
    for (sb=ap->nodes; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;
  }


  for (rem=config.remap; rem; nextrem=rem->next, free(rem), rem=nextrem)
    free(rem->name);

  for (gr=config.gate; gr; nextgr=gr->next, free(gr), gr=nextgr)
  {
    for (sb=gr->nodes; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;
    
    for (sb=gr->except; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;
  }

  for (gr=config.zgat; gr; nextgr=gr->next, free(gr), gr=nextgr)
    for (sb=gr->nodes; sb; nextsb=sb->next, free(sb), sb=nextsb)
      ;

  for (pwd=config.pwd; pwd; nextpwd=pwd->next, free(pwd), pwd=nextpwd)
    ;

  for (fwd=config.fwdto; fwd; nextfwd=fwd->next, free(fwd), fwd=nextfwd)
    ;

  for (fwd=config.fwdfrom; fwd; nextfwd=fwd->next, free(fwd), fwd=nextfwd)
    ;

  for (sb=config.ats; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;

  for (sb=config.tiny; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;

  for (sb=config.tiny_except; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;

  for (sb=config.addr; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;

  for (sb=config.stripattr; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;
   for (sb=config.stripattr_except; sb; nextsb=sb->next, free(sb), sb=nextsb)
    ;

  for (tp=config.tpath; tp; nexttp=tp->next, free(tp), tp=nexttp)
    if (tp->path)
      free(tp->path);

  for (pob=config.outb; pob; nextpob=pob->next, free(pob), pob=nextpob)
    if (pob->dir)
      free(pob->dir);

  free(config.areasbbs);
  free(config.routing);
  free(config.logfile);
  free(config.tracklog);
  free(config.origin);
  free(config.compress_cfg);
  free(config.swappath);
  free(config.statfile);
}




int MatchNN(NETADDR *m1, NETADDR *m2, int ign_zone)
{
  if ((m1->zone==m2->zone   || m2->zone==0          || m1->zone==0 ||
       m1->zone==ZONE_ALL   || m2->zone==ZONE_ALL   || ign_zone) &&
      (m2->net==m1->net     || m1->net==NET_ALL     || m2->net==NET_ALL) &&
      (m2->node==m1->node   || m1->node==NODE_ALL   || m2->node==NODE_ALL) &&
      (m2->point==m1->point || m1->point==POINT_ALL ||
       m2->point==POINT_ALL || ign_zone))
    return TRUE;
  else return FALSE;
}


int MatchNS(NETADDR *m1, struct _sblist *m2, int ign_zone)
{
  if ((m1->zone==m2->zone   || m2->zone==0          || m1->zone==0 || 
       m1->zone==ZONE_ALL   || m2->zone==ZONE_ALL   || ign_zone) &&
      (m2->net==m1->net     || m1->net==NET_ALL     || m2->net==NET_ALL) &&
      (m2->node==m1->node   || m1->node==NODE_ALL   || m2->node==NODE_ALL) &&
      (m2->point==m1->point || m1->point==POINT_ALL ||
       m2->point==POINT_ALL/* || ign_zone*/))
    return TRUE;
  else return FALSE;
}

int MatchSS(struct _sblist *m1, struct _sblist *m2, int ign_zone)
{
  if ((m1->zone==m2->zone   || m2->zone==0 || m1->zone==0 || m1->zone==ZONE_ALL ||
       m2->zone==ZONE_ALL || (config.flag & FLAG_CHECKZ)==0 || ign_zone) &&
      (m2->net==m1->net     || m1->net==NET_ALL || m2->net==NET_ALL) &&
      (m2->node==m1->node   || m1->node==NODE_ALL || m2->node==NODE_ALL) &&
      (m2->point==m1->point || m1->point==POINT_ALL || m2->point==POINT_ALL/* || ign_zone*/))
    return TRUE;
  else return FALSE;
}


static int near Scanned_Area(char *aname,void (*ScanCmd)(struct _cfgarea *ar,HAREA opensq))
{
  struct _cfgarea arname;
  struct _cfgarea *ar;
  
  arname.name=aname;

  if ((ar=SkipSearchList(config.area, &arname)) != NULL)
    (*ScanCmd)(ar, NULL);

  return (ar != NULL);
}



void Do_Echotoss(char *etname, void (*ScanCmd)(struct _cfgarea *ar,HAREA opensq),
                 int bad_scan_all, char *scan_after)
{
  FILE *etfile;
  struct _cfgarea *ar;
  char temp[PATHLEN];
  extern word mode;
  int after;    /* Have we reached the pointer after area 'scan_after' yet? */

  NW(bad_scan_all);

  /* If no scan_after area provided, scan all areas */

  after=!(scan_after && *scan_after);

  if (etname==NULL)
  {
    /* If we're currently doing a scan, scan the area ONLY if it wasn't   *
     * tossed to earlier in the session.  This means that we can do       *
     * a SQUISH IN OUT -fechtoss.log, and scan the areas in echotoss.log  *
     * which we didn't toss to.                                           *
     *                                                                    *
     * If the scanning command is not scan (ie. link), we want it to      *
     * only process areas that we DID toss into.                          */

    for (ar=SkipFirst(config.area); ar; ar=SkipNext(config.area))
    {
      /* Only scan areas which come after the named message area */

      if (scan_after && eqstri(ar->name, scan_after))
        after=TRUE;

      if (after &&
          ((mode & MODE_toss)==0 || (ar->flag & AFLAG_TOSSEDTO)))
      {
        (*ScanCmd)(ar, NULL);
      }
    }
  }
  else
  {
    /* If the echotoss doesn't exist, process it only if we're NOT          *
     * doing the link phase.  If no echotoss exists for the link,           *
     * we want to simply ignore the request, rather than relinking          *
     * all areas.                                                           */

    if ((etfile=shfopen(etname, "r", O_RDONLY | O_NOINHERIT))==NULL)
    {
      /* If it didn't exist, interpret etname as a tag name */

      if (! Scanned_Area(etname, ScanCmd))
      {
        /* If THAT failed, it musta been a bad name */

        (void)printf("Can't find `%s'; no areas scanned.\n", etname);
        S_LogMsg("#Can't read echotoss log `%s'; scan aborted", etname);
      }
    }
    else  /* if etfile was opened okay */
    {
      while (fgets(temp, PATHLEN, etfile))
      {
        (void)Strip_Trailing(temp, '\n');
        (void)Strip_Trailing(temp, '\r');
        (void)Strip_Trailing(temp, ' ');
        (void)Strip_Trailing(temp, '\t');
        (void)Strip_Trailing(temp, ' ');

        if (scan_after && eqstri(temp, scan_after))
          after=TRUE;

        if (after && *temp && !Scanned_Area(temp, ScanCmd))
          (void)printf("Unknown area: `%s'\n", temp);
      }

      (void)fclose(etfile);
    }
  }
}

int MsgAttrToFlavour(dword attr)
{
  if ((attr & (MSGCRASH|MSGHOLD))==(MSGCRASH|MSGHOLD))
    return 'D';
  else if (attr & MSGCRASH)
    return 'C';
  else if (attr & MSGHOLD)
    return 'H';
  else return 'O';
}

unsigned FlavourToMsgAttr(byte flavour)
{
  switch (toupper(flavour))
  {
    default:
    case 'O':
    case 'N':
    case 'F': return 0; /* nothing */
    case 'C': return MSGCRASH;
    case 'H': return MSGHOLD;
    case 'D': return MSGCRASH | MSGHOLD;
  }
}




int NetmailArea(struct _cfgarea *ar)
{
  struct _cfgarea *net;
  
  for (net=config.netmail; net; net=net->next_type)
    if (net==ar)
      return TRUE;
    
  return FALSE;
}

int BadmsgsArea(struct _cfgarea *ar)
{
  struct _cfgarea *net;
  
  for (net=config.badmsgs; net; net=net->next_type)
    if (net==ar)
      return TRUE;
    
  return FALSE;
}


int DupesArea(struct _cfgarea *ar)
{
  struct _cfgarea *net;
  
  for (net=config.dupes; net; net=net->next_type)
    if (net==ar)
      return TRUE;
    
  return FALSE;
}

NETADDR *SblistToNetaddr(struct _sblist *sb, NETADDR *n)
{
  n->zone=sb->zone;
  n->net=sb->net;
  n->node=sb->node;
  n->point=sb->point;
  return n;
}


struct _sblist *NetaddrToSblist(NETADDR *n, struct _sblist *sb)
{
  sb->zone=n->zone;
  sb->net=n->net;
  sb->node=n->node;
  sb->point=n->point;
  return sb;
}


#ifdef SWAP
static char * near MakeFullPath(char *cmd)
{
  char this[PATHLEN];
  char progname[PATHLEN];
  char *path=getenv("PATH");
  char *try;
  char *newpath;
  char *s;
  word last=TRUE;
  
  
  /* The name of the program to run is the first word of the command */
  
  getword(cmd, progname, " ;", 1);
  

  /* If no path set, use the current directory */

  newpath=path ? sstrdup(path) : sstrdup(".");


  /* Now scan all directories on the path */

#ifndef UNIX
  for (s=strtok(newpath, " ;"); s || last; s=strtok(NULL, " ;"))
#else
  for (s=strtok(newpath, ":"); s || last; s=strtok(NULL, ":"))
#endif
  {
    if (s)
    {
      (void)strcpy(this, s);
      Add_Trailing(this, PATH_DELIM);
    }
    else
    {
      (void)strcpy(this, "");
      last=FALSE;
    }
    
    (void)strcat(this, progname);
    try=sstrdup(this);
    
    if (!fexist(this))
    {
      (void)strcpy(this, try);
      (void)strcat(this, ".com");
      
      if (!fexist(this))
      {
        (void)strcpy(this, try);
        (void)strcat(this, ".exe");

        if (!fexist(this))
        {
          /* doesn't exist, so search next dir on path */
          free(try);
          *this='\0';
          continue;
        }
      }
    }


    /* Now tack on the rest of the command */
        
    s=firstchar(cmd, " ;", 2);
    
    (void)strcat(this, " ");
    (void)strcat(this, s ? s : "");
    
    free(try);
    break;
  }
  
  free(newpath);
  
  return (sstrdup(this));
}
#endif


int CallExtern(char *origcmd, word archiver)
{
  char *cmd;
  byte *args[MAX_ARC_ARGS];
  byte *s;
  
  word nargs=0, x;
  int save_stdout=0;
  int nul_file;
  int arcret;

  if ((cmd=strdup(origcmd))==NULL)
  {
    errno=ENOMEM;
    return -1;
  }

#ifndef OS_2
  if ((config.flag2 & FLAG2_SWAP)==0)
  {
#endif
    for (nargs=0, s=strtok(cmd," "); s && *s; s=strtok(NULL," "))
      args[nargs++]=sstrdup(s);

    args[nargs]=NULL;
#ifndef OS_2
  }
#endif

  if (archiver && (config.flag & FLAG_QUIETARC))
  {
    save_stdout=dup(fileno(stdout));

    /* Open the nul device using compatibility mode, NOT deny-none! */

    if ((nul_file=sopen(NULL_DEVICE, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                       SH_COMPAT, S_IREAD | S_IWRITE)) != -1)
    {
      (void)dup2(nul_file, fileno(stdout));
      (void)close(nul_file);
    }
  }

  #ifdef __WATCOMC__
    _heapshrink();
  #endif

  uninstall_24();

#ifdef OS_2
    {
      /* Hide our file handles so that children do not inherit them - saves *
       * all kinds of problems with archivers and other things.             */

      USHORT i;

      for (i=5; i < 50; i++)
        (void)DosSetFHandState(i, OPEN_FLAGS_NOINHERIT);
    }
#endif
  
#ifdef SWAP
  if (config.flag2 & FLAG2_SWAP)
  {
    char *s;
    char exec_return;
    char *_swappath;
    char *path=MakeFullPath(cmd);
    
    

    if (config.swappath && *config.swappath)
      _swappath=config.swappath;
    else _swappath="__SQUISH.~~~";
    
    s=strtok(path, " ");

    if (!s)
      s=path;
    
    errno=swap(s, s+strlen(s)+1, &exec_return, _swappath);
    
    if (errno)
    {
      /* Convert the error code to a C errno value */

      switch (errno)
      {
        case 1: errno=ENOMEM; break;
        case 2: errno=9999; break;    /* special "swap file full" condition */
        case 3: errno=EBADF; break;
      }

      arcret=-1;
    }
    else arcret=exec_return;

    free(path);

    /*
    if (config.swappath && *config.swappath)
      _swappath=config.swappath;

    arcret=xspawnvp(P_WAIT, args[0], (char **)args);
    */
  }
  else
#endif
  arcret=spawnvp(P_WAIT, args[0], (char **)args);

  install_24();

  if (archiver && (config.flag & FLAG_QUIETARC))
  {
    (void)dup2(save_stdout, fileno(stdout));
    (void)close(save_stdout);
  }

  if ((config.flag2 & FLAG2_SWAP)==0)
    for (x=0; x < nargs; x++)
      free(args[x]);
  
  free(cmd);
  return arcret;
}



/* Filter for locally-entered invalid dates.  This one's for you, Tony! */

word InvalidDate(byte *da)
{
  word mo, dn;
  
  /* Fix brain-damaged Opus date kludge */

  if (da[19]==(byte)0xffu || da[18]=='\0' || da[18]==' ')
    (void)memmove(da+10, da+9, 10);
  
  /* Y2K kludge date fix */
  if (isdigit(da[7]) && isdigit(da[8]) && isdigit(da[9]))
    (void)memmove(da+7, da+8, 12);

  da[19]='\0';


  /* 01234567890123456789 */
  /* 05 Jan 91  11:22:33  */

  if (isdigit(*da) || *da==' ')
  {
    if (*da==' ') /* fix brain-dead opus behaviour */
      *da='0';

    if (da[0] < '0' || da[0] > '3' ||
        da[1] < '0' || da[1] > '9' ||
        da[2] != ' ')
      return TRUE;

    for (mo=0; mo < 12; mo++)
      if (strncmp(da+3, months_ab[mo], 3)==0)
        break;

    if (mo==12)
      return TRUE;

    if (da[6] != ' ')
      return TRUE;

    if (da[7] < '0' || da[7] > '9' ||
        da[8] < '0' || da[8] > '9')
      return TRUE;

    if (da[9] != ' ' || da[10] != ' ')
      return TRUE;

    if (da[11]==' ')  /* fix brain-dead opus behaviour */
      da[11]='0';

    if (da[11] < '0' || da[11] > '2' ||
        da[12] < '0' || da[12] > '9')
      return TRUE;

    if (da[13] != ':')
      return TRUE;

    if (da[14] < '0' || da[14] > '5' ||
        da[15] < '0' || da[15] > '9')
      return TRUE;

    if (da[16] != ':')
      return TRUE;

    if (da[17] < '0' || da[17] > '5' ||
        da[18] < '0' || da[18] > '9')
      return TRUE;

    if (da[19] != '\0')
      return TRUE;
  }
  else
  {
    /* 01234567890123456789*/
    /* Sun 15 Jan 91 11:22 */

    for (dn=0; dn < 7; dn++)
      if (strncmp(da, weekday_ab[dn], 3)==0)
        break;
      
    if (dn==7)
      return TRUE;
    
    if (da[3] != ' ')
      return TRUE;
    
    if (((da[4] < '1' || da[4] > '3') && da[4] != ' ') ||
        da[5] < '0' || da[5] > '9')
      return TRUE;
    
    if (da[6] != ' ')
      return TRUE;
    
    for (mo=0; mo < 12; mo++)
      if (strncmp(da+7, months_ab[mo], 3)==0)
        break;
      
    if (mo==12)
      return TRUE;
    
    if (da[13] != ' ')
      return TRUE;
    
    if ((da[14] != ' ' && (da[14] < '0' || da[14] > '2')) ||
        da[15] < '0' || da[15] > '9')
      return TRUE;
    
    if (da[16] != ':')
      return TRUE;
    
    if (da[17] < '0' || da[17] > '5' ||
        da[18] < '0' || da[18] > '9')
      return TRUE;
    
    if (da[19] != '\0')
      return TRUE;
  }
  
  return FALSE;
}

void HandleArcRet(int arcret, char *cmd)
{
  if (arcret==-1)
    S_LogMsg("!Archiver error: %s (errno=%d, %s)",
             (
#ifdef __TURBOC__
              errno==ENOFILE || errno==ENOPATH ||
#endif
              errno==ENOENT) 
                            ?  "Program not found" :
              errno==ENOMEM ?  "Not enough memory"  :
              errno==EACCES ?  "Permission denied"  :
              errno==9999   ?  "Error writing to swap file" :
                               "Can't run archiver",
            errno, strerror(errno));
  else
    S_LogMsg("!Archiver returned errorlevel %d", arcret);

  S_LogMsg("!Archiver command was: \"%s\"", cmd);
}


/* Mash a ^aMSGID-type line into hash and serial number components */

void MashMsgid(char *begin, dword *msgid_hash, dword *msgid_serial)
{
  char hash_buf[PATHLEN];
  size_t maxsize;
  char *end;

  /*  012345678                                               */
  /* ^aMSGID: 1:249/106 12345678                              */
  /*                                                          */
  /* ^aMSGID: "dudleys f106 n249 z1 fidonet org" 12345678     */

  end=begin;

  /* If we got a quote, skip over until the next quote is found */

  if (*begin=='\"')
  {
    for (end=begin+1; *end; end++)
      if (*end=='"')
        if (*++end != '"')
          break;
  }
  else
  {
    /* Else just skip until the next space */

    while (*end && *end != ' ')
      end++;
  }

  maxsize=min(PATHLEN-1, (size_t)(end-begin));

  strncpy(hash_buf, begin, maxsize);
  hash_buf[(size_t)maxsize]=0;
  *msgid_hash=SquishHash(hash_buf);

  /* Skip over the spaces */

  while (*end==' ')
    end++;

  /* Make sure that the hex ID is read in correctly */

  if (sscanf(end, "%08" UINT32_XFORMAT "x", msgid_serial) != 1)
  {
    *msgid_serial=*msgid_hash=0L;
    return;
  }
#ifdef DEBUG
  S_LogMsg("!DEBUG: MSGID Hash: Begin (%s), End (%s) msgid_serial: %0.8x",
           begin, end, *msgid_serial);
#endif
}


