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
static char rcs_id[]="$Id: node.c,v 1.1.1.1 2002/10/01 17:52:53 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "mm.h"
#include "node.h"
#include "fdnode.h"


int namecomp(void *n1, void *v2);
int addrcomp(void *v1, void *v2);
int phcomp(void *s1, void *r2);

static int near V56FindOpen(NFIND *nf);
static NFIND * near V7FindOpen(NFIND *nf);
static NFIND * near FDFindOpen(NFIND *nf);
static word near FDCostOf(int fdafd, word rec_num);
static word near FDLookUpCost(struct _johofile *jf, struct _johonode *jn);





NFIND * NodeFindOpen(NETADDR *find)
{
  NFIND *nf;
  
  if ((nf=malloc(sizeof(NFIND)))==NULL)
    return NULL;
  
  memset(nf, 0, sizeof *nf);

  nf->find=*find;

  /* Version 5 or version 6 nodelist */

  if (prm.nlver==NLVER_5 || prm.nlver==NLVER_6)
  {
    if (!V56FindOpen(nf))
    {
      free(nf);
      return NULL;
    }
  
    if (NodeFindNext(nf)==0)
      return nf;

    NodeFindClose(nf);
    return NULL;
  }
  else if (prm.nlver==NLVER_7)
    return (V7FindOpen(nf));
  else return (FDFindOpen(nf));
}




static int near V56FindOpen(NFIND *nf)
{
  char name[PATHLEN];

  sprintf(name, idxnode, PRM(net_info));

  if ((nf->v56.ifd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return FALSE;

  sprintf(name, (prm.nlver==5) ? sysnode : datnode, PRM(net_info));

  if ((nf->v56.dfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    close(nf->v56.ifd);
    return FALSE;
  }

  nf->v56.recsize=(word)(lseek(nf->v56.dfd, 0L, SEEK_END)/
                         (lseek(nf->v56.ifd, 0L, SEEK_END)/
                          (long)sizeof(struct _ndi)));

  nf->v56.pos=0L;

  nf->v56.idxcnt=nf->v56.idxcur=0;
  nf->v56.zone=prm.address[0].zone;

  if ((nf->v56.idxbuf=malloc(NUM_NDI*sizeof(struct _ndi)))==NULL)
  {
    close(nf->v56.ifd);
    close(nf->v56.dfd);
    return FALSE;
  }
  
  return TRUE;
}



static NFIND * near V7FindOpen(NFIND *nf)
{
  struct _newnode node6;
  NETADDR tofind;

  /* Version7 only supports lookup (no findnext stuff), so do it all here */

  tofind=nf->find;

  if (V7FindNode(&tofind, &node6, PRM(net_info)) ||
      (tofind.point &&
       (tofind.point=0, V7FindNode(&tofind, &node6, PRM(net_info)))))
  {
    nf->found.zone=tofind.zone;
    nf->found.net=tofind.net;
    nf->found.node=tofind.node;
    nf->found.point=tofind.point;
    nf->found.cost=node6.Cost;

    strcpy(nf->found.name, node6.SystemName);
    strcpy(nf->found.phone, node6.PhoneNumber);
    strcpy(nf->found.city, node6.MiscInfo);
    nf->found.flag=node6.NodeFlags;
    return nf;
  }

  NodeFindClose(nf);
  return NULL;
}


static NFIND * near FDFindOpen(NFIND *nf)
{
  struct _johonode jn;
  struct _johofile jf;
  int good=FALSE;
  struct _fdx found;
  NETADDR find;

  find=nf->find;
  find.point=0;

  if (JoHoOpen(PRM(net_info), &jf, FALSE))
  {
    if (JoHoLookup(jf.fdfd, sizeof(struct _fdb), sizeof(struct _fdx),
                   &find, &found, addrcomp) &&
        JoHoFetch(&found, &jf, &jn))
    {
      nf->found.zone= (found.zone_hi  << 8) | found.zone_lo;
      nf->found.net=  (found.net_hi   << 8) | found.net_lo;
      nf->found.node= (found.node_hi  << 8) | found.node_lo;
      nf->found.point=(found.point_hi << 8) | found.point_lo;

      nf->found.cost=FDLookUpCost(&jf, &jn);

      strcpy(nf->found.name, jn.system);
      strcpy(nf->found.phone, jn.phone);
      strcpy(nf->found.city, jn.city);
      nf->found.flag=0;
      good=TRUE;
    }

    JoHoClose(&jf);

    if (good)
      return nf;
  }

  NodeFindClose(nf);
  return NULL;
}

      
      
/* Find the cost required to send mail to a given node.  This *should*      *
 * be easy, but I can't figure out any easier way to do it, at least        *
 * from what I can see of JoHo's file format...                             */
      
static word near FDLookUpCost(struct _johofile *jf, struct _johonode *jn)
{
  static char our_prefix[20]="";
  static int got_prefix=FALSE;
  struct _pdx p;
  word cost=0;

  char name[PATHLEN];
  char phone[60];
  char *s;

  int phfd, fdafd, got;

  sprintf(name, "%sPHONE.FDX", PRM(net_info));
  
  /* Open the phone index file */
  
  if ((phfd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
    return 0;
  
  sprintf(name, "%sPHONE.FDA", PRM(net_info));
  
  if ((fdafd=shopen(name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    close(phfd);
    return 0;
  }
  
  strncpy(phone, jn->phone, 59);
  phone[59]='\0';
  
  /* Do a search on the phone number, dropping off parts till we find it */

  while ((got=JoHoLookup(phfd, sizeof(struct _pdb), 
                         sizeof(struct _pdx), phone, &p, phcomp))==0)
  {
    Strip_Trailing(phone, '-');

    if ((s=strrchr(phone, '-')) != NULL && s[1])
      s[1]='\0';
    else break;
  }

  /* If we found it */
  
  if (got)
    cost=FDCostOf(fdafd, p.rec_num);
  else
  {
    /* Couldn't find node in cost tables.  OK, so figure out whether it's   *
     * domestic or international.                                           */

    if (!got_prefix)
    {
      struct _fdx found;
      struct _johonode our_jn;
      NETADDR prim;

      /* Find our node's phone number prefix, and compare that with the     *
       * node in queston to figure out whether or not it's international.   */

      got_prefix=TRUE;

      prim=prm.address[0];

      if (JoHoLookup(jf->fdfd, sizeof(struct _fdb), sizeof(struct _fdx),
                     &prim, &found, addrcomp) &&
          JoHoFetch(&found, jf, &our_jn))
      {
        char *s;

        strncpy(our_prefix, our_jn.phone, 19);

        /* Chop off everything after the first dash */

        if ((s=strchr(our_prefix, '-')) != NULL)
          s[1]='\0';
        else *our_prefix='\0';
      }
    }

    /* Domestic call */

    if (*our_prefix=='\0' || eqstrn(jn->phone, our_prefix, strlen(our_prefix)))
    {
       if (JoHoLookup(phfd, sizeof(struct _pdb), sizeof(struct _pdx),
                      (void *)"DOM", &p, phcomp))
         cost=FDCostOf(fdafd, p.rec_num);
    }
    else /* International call */
    {
      if (JoHoLookup(phfd, sizeof(struct _pdb), sizeof(struct _pdx),
                     "INTL", &p, phcomp))
        cost=FDCostOf(fdafd, p.rec_num);
    }
  }

  close(fdafd);
  close(phfd);

  return cost;
}



static word near FDCostOf(int fdafd, word rec_num)
{
  struct _pda pda;

  /* Seek to the indicated record and extract the node's cost */

  lseek(fdafd, rec_num*(long)sizeof(struct _pda), SEEK_SET);

  if (read(fdafd, (char *)&pda, sizeof(pda))==sizeof(pda))
    return pda.cost & 0x7fff;   /* mask off high bit */

  return 0;
}
  


/* Used for doing name lookups on the FrontDoor nodelist */

int FDFindName(char *find, NETADDR *n, char *path)
{
  struct _johofile jf;
  struct _udx f;
  int success=FALSE;
  
  cstrupr(find);

  if (!JoHoOpen(path, &jf, TRUE))
    return FALSE;

  if (JoHoLookup(jf.ufd, sizeof(struct _udb), sizeof(struct _udx),
                 find, &f, namecomp))
  {
    n->zone= (f.zone_hi  << 8) | f.zone_lo;
    n->net=  (f.net_hi   << 8) | f.net_lo;
    n->node= (f.node_hi  << 8) | f.node_lo;
    n->point=(f.point_hi << 8) | f.point_lo;

    success=TRUE;
  }
  
  JoHoClose(&jf);
  
  return success;
}



int NodeFindNext(NFIND *nf)
{
  int got;
  struct _ndi *p, *e;
  struct _v56 *v56;
  word rec;
  struct _node node5;
  struct _newnode node6;

  /* V7 findnext is not supported */

  if (prm.nlver==NLVER_7 || prm.nlver==NLVER_FD)
    return -1;
  
  v56=&nf->v56;

  for (;;)
  {
    /* Refresh the nodelist index, if necessary */

    if (v56->idxcur >= v56->idxcnt)
    {
      lseek(v56->ifd, v56->pos*(long)sizeof(struct _ndi), SEEK_SET);

      got=read(v56->ifd, (char *)v56->idxbuf, NUM_NDI*sizeof(struct _ndi));
      got /= sizeof(struct _ndi);

      /* Didn't read anything, so return 0 */

      if (got <= 0)
        return -1;

      /* Increment pointer in index file */

      v56->pos += (long)got;

      v56->idxcnt=got;
      v56->idxcur=0;
    }

    /* Now scan the index that we just read in */

    for (p=v56->idxbuf+v56->idxcur, e=v56->idxbuf+v56->idxcnt; p < e; p++)
    {
      nf->found.flag=0;
      
      if (p->node==(word)-2)    /* ZC */
      {
        v56->zone=p->net;
        v56->net=p->net;
        v56->node=0;
        v56->point=0;

        nf->found.flag |= B_zone;
      }
      else if (p->node==(word)-1 || p->node==0)   /* RC or NC */
      {
        v56->net=p->net;
        v56->node=0;
        v56->point=0;
                
        nf->found.flag |= (p->node==(word)-1) ? B_region : B_host;
      }
      else if (p->node==(word)-3)   /* Point */
      {
        v56->point=p->net;
        nf->found.flag |= B_point;
      }
      else                          /* Node */
      {
        v56->net=p->net;
        v56->node=p->node;
        v56->point=0;
      }

      if (prm.nlver==5)
        nf->find.zone=ZONE_ALL;

      v56->idxcur++;

      /* Now see if we got a match */

      if ((v56->zone==nf->find.zone || nf->find.zone==ZONE_ALL) &&
          (v56->net==nf->find.net || nf->find.net==NET_ALL) &&
          (v56->node==nf->find.node || nf->find.node==NODE_ALL))
      {
        break;
      }
    }

    /* If we found a match, get out of this loop too */

    if (p != e)
      break;
  }

  /* The only way to get here is if a node was found */

  rec=(int)v56->pos-(e-p);

  lseek(v56->dfd, rec * (long)v56->recsize, SEEK_SET);

  if (read(v56->dfd, (prm.nlver==5 ? (char *)&node5 : (char *)&node6),
           v56->recsize) != (signed)v56->recsize)
  {
    return -1;
  }

  /* Got the record okay.  Now copy it into the "MaxNode" structure */

  if (prm.nlver==5)
  {
    nf->found.zone=v56->zone;
    nf->found.net=v56->net;
    nf->found.node=v56->node;
    nf->found.point=v56->point;
    nf->found.cost=node5.cost;

    strcpy(nf->found.name, node5.name);
    strcpy(nf->found.phone, node5.phone);
    strcpy(nf->found.city, node5.city);
    /* nf->found.flag will have been filled in above */
  }
  else
  {
    nf->found.zone=v56->zone;
    nf->found.net=v56->net;
    nf->found.node=v56->node;
    nf->found.point=v56->point;
    nf->found.cost=node6.Cost;

    strcpy(nf->found.name, node6.SystemName);
    strcpy(nf->found.phone, node6.PhoneNumber);
    strcpy(nf->found.city, node6.MiscInfo);
    nf->found.flag=node6.NodeFlags;
  }

  return 0;
}


void NodeFindClose(NFIND *nf)
{
  if (!nf)
    return;
  
  if (prm.nlver==NLVER_5 || prm.nlver==NLVER_6)
  {
    if (nf->v56.idxbuf)
      free(nf->v56.idxbuf);

    close(nf->v56.ifd);
    close(nf->v56.dfd);
  }
  else if (prm.nlver==NLVER_7)
  {
    /* Version7 cleanup - none at present */
  }
  else if (prm.nlver==NLVER_FD)
  {
    /* FD cleanup - none at present */
  }

  free(nf);
}


/* Comparison function for the JoHo nodelist */

int addrcomp(void *v1, void *v2)
{
  NETADDR a1=*(NETADDR *)v1;
  NETADDR a2;
  struct _fdx *f=(struct _fdx *)v2;
  int d;

  a2.zone= (f->zone_hi  << 8) | f->zone_lo;
  a2.net=  (f->net_hi   << 8) | f->net_lo;
  a2.node= (f->node_hi  << 8) | f->node_lo;
  a2.point=(f->point_hi << 8) | f->point_lo;

  if ((d=a1.zone-a2.zone) != 0)
    return d;
  
  if ((d=a1.net-a2.net) != 0)
    return d;
  
  if ((d=a1.node-a2.node) != 0)
    return d;
  
  return (a1.point-a2.point);
}



int namecomp(void *n1, void *v2)
{
  char n2[16];

  strncpy(n2, ((struct _udx *)v2)->name, 15);
  n2[15]='\0';
  
  return (strncmp(n1, n2, 15));
}



int phcomp(void *s1, void *r2)
{
  char n1[60];
  struct _pdx *p=(struct _pdx *)r2;
  char n2[60];

  strcpy(n1, s1);
  memmove(n2, p->phone, p->phone_len);
  n2[p->phone_len]='\0';

  {
    int i=strncmp(n1, n2, strlen(n2));

    /*
    if (i==0)
      printf("Match: '%s' - '%s'\n", n1, n2);
    else printf("No match: '%s' - '%s'\n", n1, n2);
    */

    return i;
  }

}

