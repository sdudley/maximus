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
static char rcs_id[]="$Id: s_config.c,v 1.2 2003/06/05 03:13:40 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"
#include "squish.h"
#include "s_dupe.h"


#define ARGLEN  64
#define MAXARGS 128

static char *cfgdelim=" \t\n\b";

static void near ReportErr(char *s)
{
  (void)printf("\aError!  %s\n", s);
  exit(ERL_ERROR);
}

static void near BeforeCompress(char *place)
{
  S_LogMsg("!Definition of COMPRESS.CFG must come before %s", place);
}

static void near InvalStatement(char *where)
{
  (void)printf("Invalid %s statement in config file!\n",where);
}


#ifdef OS_2


/* Add a DLL-oriented feature */

static void near V_Feature(char *line, char *ag[])
{
  struct _feature *pf;
  struct _feat_init fi;
  char szFailName[PATHLEN];
  PFN pfnTest;
  int rc;

  NW(line);

  pf=smalloc(sizeof(struct _feature));
  pf->pszDLLName=sstrdup(strupr(ag[1]));
  pf->pfNext=config.feat;
  config.feat=pf;

  /* Try to load the module */

  if ((rc=DosLoadModule(szFailName, PATHLEN, pf->pszDLLName, &pf->hmod)) != 0)
  {
    (void)printf("Could not load dynamic link library '%s'! (rc=%d)\n",
                 szFailName, rc);
    exit(ERL_ERROR);
  }

#ifdef __FLAT__
  if (DosQueryProcAddr(pf->hmod, 0, "FEATUREINIT", (PFN*)&pf->pfnInit) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURECONFIG", (PFN*)&pf->pfnConfig) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURENETMSG", (PFN*)&pf->pfnNetMsg) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURETOSSMSG", (PFN*)&pf->pfnTossMsg) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURESCANMSG", (PFN*)&pf->pfnScanMsg) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURETERM", (PFN*)&pf->pfnTerm) != 0 ||
      DosQueryProcAddr(pf->hmod, 0, "FEATURE32BIT", (PFN *)&pfnTest) != 0)
#else
  if (DosGetProcAddr(pf->hmod, "FEATUREINIT", (PPFN)(void far *)&pf->pfnInit) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURECONFIG", (PPFN)(void far *)&pf->pfnConfig) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURENETMSG", (PPFN)(void far *)&pf->pfnNetMsg) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURETOSSMSG", (PPFN)(void far *)&pf->pfnTossMsg) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURESCANMSG", (PPFN)(void far *)&pf->pfnScanMsg) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURETERM", (PPFN)(void far *)&pf->pfnTerm) != 0 ||
      DosGetProcAddr(pf->hmod, "FEATURE16BIT", (PFN *)&pfnTest) != 0)
#endif
  {
    (void)printf("Incorrect format for dynamic link library '%s'!\n",
                 pf->pszDLLName);
    exit(ERL_ERROR);
  }


  /* Initialize the feature info structure */

  (void)memset(&fi, 0, sizeof fi);

  fi.struct_len=sizeof fi;
  fi.pfnLogMsg=S_LogLine;


  /* Init the feature handler */

  if ((*pf->pfnInit)(&fi))
    exit(ERL_ERROR);

  pf->pszConfigName=sstrdup(fi.szConfigName);
  pf->ulFlag=fi.ulFlag;
}

#endif /* OS_2 */


static void near V_DupeCheck(char *line, char *ag[])
{
  int i;
  NW(line);

  config.flag2 &= ~(FLAG2_DHEADER|FLAG2_DMSGID);

  for (i=1; *ag[i]; i++)
  {
    if (eqstri(ag[i], "msgid"))
      config.flag2 |= FLAG2_DMSGID;
    else if (eqstri(ag[i], "header"))
      config.flag2 |= FLAG2_DHEADER;
    else
      printf("Unknown DupeCheck method: %s\n", ag[i]);
  }

  if ((config.flag2 & (FLAG2_DMSGID|FLAG2_DHEADER))==0)
    config.flag2 |= FLAG2_DMSGID | FLAG2_DHEADER;
}

static void near V_Statistics(char *line, char *ag[])
{
  NW(line);

  /* Turn on statistics */

  config.flag ^= FLAG_STATS;

  /* Use the new statistics filename, if necessary */

  if (*ag[1])
  {
    if (config.statfile)
      free(config.statfile);

    config.statfile=sstrdup(ag[1]);
  }

  if (SkipFirst(config.area))
    ReportErr("The Statistics keyword must precede all area definitions!");
}

static void near V_Outbound(char *line, char *ag[])
{
  struct _outb *pob;

  NW(line);


  pob=smalloc(sizeof(struct _outb));

  /* Get default zone and dir info */

  pob->zone=*ag[2] ? (word)atoi(ag[2]) : 0;
  pob->dir=sstrdup(ag[1]);

  /* Make sure that it doesn't end with a backslash */

  if (strlen(pob->dir) > 3)
  {  
    (void)Strip_Trailing(pob->dir, '\\');
#ifdef UNIX
    (void)Strip_Trailing(pob->dir, '/');
#endif
  }

  /* Add to linked list */

  pob->next=config.outb;
  config.outb=pob;
}


static void near V_Duplicates(char *line, char *ag[])
{
  NW(line);

  config.dupe_msgs=(word)atoi(ag[1]);

#ifndef __FLAT__
  if (((long)config.dupe_msgs * (long)sizeof(DUPEID) +
       sizeof(DUPEHEAD)) > 65300L)
  {
    S_LogMsg("!`Duplicates' keyword set too high - Max: %u",
             config.dupe_msgs=((65300u - sizeof(DUPEHEAD))/sizeof(DUPEID)-1));
  }
#endif
}

static void near V_LogLevel(char *line, char *ag[])
{
  NW(line);
  config.loglevel=(byte)atoi(ag[1]);
}


#ifndef __FLAT__
static void near CheckBufSize(unsigned size)
{
  if (size >= 64)
    ReportErr("Buffers statement: each value must be less than 64K");
}
#endif

static void near V_Buffers(char *line,char *ag[])
{
  NW(config);
  NW(line);
  
  if (eqstri(ag[1], "small"))
  {
    writebufmax=WRITEBUF_SMALL;
    outbufmax=OUTBUF_SMALL;
    maxmsglen=MAXMSGLEN_SMA;
  }
  else if (eqstri(ag[1], "medium"))
  {
    writebufmax=WRITEBUF_MEDIUM;
    outbufmax=OUTBUF_MEDIUM;
    maxmsglen=MAXMSGLEN_MED;
  }
  else if (eqstri(ag[1], "large"))
  {
    writebufmax=WRITEBUF_LARGE;
    outbufmax=OUTBUF_LARGE;
    maxmsglen=MAXMSGLEN_LAR;
  }
  else if (isdigit(*ag[1]))
  {
    writebufmax=(unsigned)atoi(ag[1]);
    outbufmax=(unsigned)atoi(ag[2]);
    maxmsglen=(unsigned)atoi(ag[3]);

#ifndef __FLAT__
    CheckBufSize(writebufmax);
    CheckBufSize(outbufmax);
    CheckBufSize(maxmsglen);
#endif

    /* Now multiply by 1024 to get size in kbytes */

    writebufmax *= 1024;
    outbufmax *= 1024;
    maxmsglen *= 1024;

    if (!writebufmax || !outbufmax || !maxmsglen)
      ReportErr("The three parameters for `Buffers' must all be non-zero!");

    if (writebufmax < WRITEBUF_SMALL)
      writebufmax=WRITEBUF_SMALL;

    if (outbufmax < OUTBUF_SMALL)
      outbufmax=OUTBUF_SMALL;

    if (maxmsglen < 8000)
      maxmsglen=8000;
  }
  else InvalStatement("Buffers");
}

static void near V_Swap(char *line,char *ag[])
{
  NW(line);
  NW(ag);

#ifdef SWAP
  config.flag2 |= FLAG2_SWAP;
#else
  NW(config);
#endif
}


#ifdef NEVER

/* Specify more than the default number of file handles */

static void near V_Handles(struct _config *config,char *line,char *ag[])
{
  int handles;

  NW(line);
  
  handles=atoi(ag[1]);
  
  if (handles < 20 || handles > 250)
  {
    printf("Error!  Invalid number of file handles specified!");
    return;
  }
  
  config.max_handles=handles;
  config.max_talist=(handles-12)/2;
}

#endif



static void near V_MaxMsgs(char *line, char *ag[])
{
  NW(line);
  
  config.max_msgs=(word)atoi(ag[1]);
}


static void near V_MaxArchive(char *line, char *ag[])
{
  NW(line);
  
  config.max_archive=(word)atoi(ag[1]);
}




static void near V_Area(char *line,char *ag[])
{
  /* 0    1       2           3 ...
   * Area NETMAIL g:\msg\net -p123/456 -0 -$ -f -s
   */

  struct _cfgarea *ar, *testar;
  char *p=firstchar(line, cfgdelim, 4);

  if (!p)
    p="";

  ar=Declare_Area(ag[2], ag[1], p, 0, 0);

  if (!ar)
    return;
  
  if (eqstri(ag[0], "badarea"))
  {
    ar->flag |= AFLAG_BAD;

    /* See if this area is already in the bad_msgs list */

    for (testar=config.badmsgs; testar; testar=testar->next_type)
      if (testar==ar)
        break;

    /* If not, add it */

    if (!testar)
    {
      ar->next_type=config.badmsgs;
      config.badmsgs=ar;
    }
  }
  else if (eqstri(ag[0], "dupearea"))
  {
    ar->flag |= AFLAG_DUPES;

    /* Make sure that dupe area is not already in list */

    for (testar=config.dupes; testar; testar=testar->next_type)
      if (testar==ar)
        break;

    /* If not, add it */

    if (!testar)
    {
      ar->next_type=config.dupes;
      config.dupes=ar;
    }
  }
  else if (eqstri(ag[0], "netarea"))
  {
    struct _cfgarea *last_ar, *net_ar;

    ar->flag |= AFLAG_NET;

    for (last_ar=NULL, net_ar=config.netmail;
         net_ar && ar != net_ar;
         last_ar=net_ar, net_ar=net_ar->next_type)
      ;

    /* Make sure that we don't make a circular list */

    if (ar != net_ar)
    {
      if (last_ar)
        last_ar->next_type=ar;
      else config.netmail=ar;
    }
  }
  else /* an echo area */
  {
    ar->type |= MSGTYPE_ECHO;
  }
}



static void near V_Netfile(char *line,char *ag[])
{
  struct _tosspath *tp;
  int x;
  
  NW(line);
  
  tp=smalloc(sizeof(struct _tosspath));
  tp->flag=0;
  tp->next=config.tpath;
  config.tpath=tp;

  for (x=1; x < MAXARGS; x++)
  {
    if (eqstri(ag[x],"nopkt"))
      tp->flag |= TFLAG_NOPKT;
    else if (eqstri(ag[x],"noarc"))
      tp->flag |= TFLAG_NOARC;
    else if (eqstri(ag[x],"noecho"))
      tp->flag |= TFLAG_NOECHO;
    else
    {
      (void)Strip_Trailing(ag[x],'\\');
#ifdef UNIX
      (void)Strip_Trailing(ag[x],'/');
#endif
  
      tp->path=sstrdup(ag[x]);
      break;
    }
  }
  
  return;
}







static void near HandleGZRoute(char *ag[], struct _groute **begin, word gateroute)
{
  struct _groute *gr;
  struct _sblist *sb, last;
  word except;
  word nn;


  gr=smalloc(sizeof(struct _groute));


  /* Add to our linked list of gateroute nodes */
  
  gr->next=*begin;
  *begin=gr;


  /* Start parsing just after the keyword name */
  
  nn=1;

  if (gateroute)
  {
    struct _flotype *f;
    
    for (f=flo_str; f->name; f++)
      if (eqstri(ag[1], f->name))
        break;

    if (f->name)
      gr->flavour=f->flavour;
    else
    {
      S_LogMsg("!Invalid GateRoute flavour");
      gr->flavour='F';
    }
    
    /* Start parsing at the next argument */
    
    nn++;
  }


  /* Default to our address for the route-to host */

  gr->host=config.def;

  ParseNN(ag[nn++], &gr->host.zone, &gr->host.net,
          &gr->host.node, &gr->host.point, FALSE);

  /* Default zone/net/node for nodes, too*/

  last=config.def;

  for (except=FALSE; nn < MAXARGS && *ag[nn]; nn++)
  {
    if (eqstri(ag[nn], "except"))
    {
      except=TRUE;
      continue;
    }
    
    sb=smalloc(sizeof(struct _sblist));

    /* Default to last address */

    *sb=last;
    

    ParseNN(ag[nn], &sb->zone, &sb->net, &sb->node, &sb->point, TRUE);
    
    last=*sb;
    last.point=0;


    /* Add to our linked list of nodes to gateroute for */
    
    if (except)
    {
      sb->next=gr->except;
      gr->except=sb;
    }
    else
    {
      sb->next=gr->nodes;
      gr->nodes=sb;
      gr->n_nodes++;
    }
  }
}





static void near inval_route(char *s)
{
  (void)printf("Error!  Invalid %s statement: a host and at least one node\n"
               "must be specified!\n\n", s);
}





static void near V_GateRoute(char *line,char *ag[])
{
  NW(line);
  
  if (*ag[1]=='\0' || *ag[2]=='\0')
  {
    inval_route("GateRoute");
    return;
  }
  
  HandleGZRoute(ag, &config.gate, TRUE);
}




static void near V_ZoneGate(char *line, char *ag[])
{
  NW(line);
  
  if (*ag[1]=='\0' || *ag[2]=='\0')
  {
    inval_route("ZoneGate");
    return;
  }
  
  HandleGZRoute(ag, &config.zgat, FALSE);
}





static void near V_Tiny(char *line,char *ag[])
{
  unsigned except;
  struct _sblist *tiny, last;
  unsigned i;

  NW(line);
  
  last=config.def;
  
  for (i=1, except=FALSE; i < MAXARGS && *ag[i]; i++)
  {
    if (eqstri(ag[i], "except"))
    {
      except=TRUE;
      continue;
    }

    tiny=smalloc(sizeof(struct _sblist));

    *tiny=last;

    ParseNN(ag[i], &tiny->zone, &tiny->net, &tiny->node, &tiny->point, TRUE);

    last=*tiny;
    last.point=0;
    
    /* Append to linked list of tiny seenby nodes */
    
    if (except)
    {
      tiny->next=config.tiny_except;
      config.tiny_except=tiny;
    }
    else
    {
      tiny->next=config.tiny;
      config.tiny=tiny;
    }
  }
}


static void near V_StripAttributes(char *line, char *ag[])
{
  unsigned except;
  struct _sblist *sa, last;
  unsigned i;

  NW(line);
  
  /* If no argument was specified, assume "world" */

  if (*ag[1]=='\0')
  {
    strcpy(ag[1], "world");
    *ag[2]='\0';
  }

  last=config.def;
  
  for (i=1, except=FALSE; i < MAXARGS && *ag[i]; i++)
  {
    if (eqstri(ag[i], "except"))
    {
      except=TRUE;
      continue;
    }

    sa=smalloc(sizeof(struct _sblist));

    *sa=last;

    ParseNN(ag[i], &sa->zone, &sa->net, &sa->node, &sa->point, TRUE);

    last=*sa;
    last.point=0;
    
    /* Append to linked list of tiny seenby nodes */
    
    if (except)
    {
      sa->next=config.stripattr_except;
      config.stripattr_except=sa;
    }
    else
    {
      sa->next=config.stripattr;
      config.stripattr=sa;
    }
  }
}


static void near V_Remap(char *line,char *ag[])
{
  struct _remap *rem, *last;
  char *p;

  NW(line);
  
  if (! *ag[2])
  {
    InvalStatement("Remap");
    return;
  }

  rem=smalloc(sizeof(struct _remap));

  rem->node=config.def;

  ParseNN(ag[1], &rem->node.zone, &rem->node.net,
          &rem->node.node, &rem->node.point, FALSE);

  p=firstchar(line, cfgdelim, 3);

  if (! p)
   p="";

  rem->name=sstrdup(p);
  rem->next=NULL;

  /* Append to END linked list - this is necessary so that they're          *
   * processed in order of appearance.                                      */
  
  for (last=config.remap; last && last->next; last=last->next)
    ;
  
  if (last)
    last->next=rem;
  else config.remap=rem;
}


static void near V_Compress(char *line,char *ag[])
{
  struct _arcinfo *ai;

  NW(line);
  NW(ag);

  if ((config.arc=Parse_Arc_Control_File(config.compress_cfg))==NULL)
    ErrOpening("archiver config", config.compress_cfg);
  
  /* Walk to the end of the linked list */

  for (ai=config.arc; ai && ai->next; ai=ai->next)
    ;

  if (ai)
    config.def_arc=ai;
}

static void near V_Routing(char *line,char *ag[])
{
  NW(line);
  NW(ag);

  if (!fexist(config.routing))
  {
    (void)printf("\aCan't find routing control file `%s'\n", config.routing);
    exit(ERL_ERROR);
  }
}



static void near V_AddToSeen(char *line,char *ag[])
{
  struct _sblist *ats, last;
  int y;
  
  NW(line);

  last=config.def;

  for (y=1; y < MAXARGS && *ag[y]; y++)
  {
    ats=smalloc(sizeof(struct _sblist));

    /* Copy in the default zone/net/node/pt info */
    
    *ats=last;
    
    ParseNN(ag[y], &ats->zone, &ats->net, &ats->node, &ats->point, FALSE);
    
    last=*ats;
    last.point=0;

    if (!ats->zone)
    {
      ReportErr("Zone number is missing from \"Address\" statement, or\n"
                "a zone of 0 was supplied.\n");
    }

    /* Add this to the add-to-seen linked list */
    
    ats->next=config.ats;
    config.ats=ats;

    config.num_ats++;
  }
}


/* A default archiver */

static void near V_DefaultPack(char *line,char *ag[])
{
  struct _arcinfo *ai;
  
  NW(line);
  
  if (config.arc==NULL)
  {
    BeforeCompress("DefaultPacker");
    return;
  }
  
  for (ai=config.arc; ai; ai=ai->next)
    if (eqstri(ai->arcname, ag[1]))
    {
      config.def_arc=ai;
      return;
    }

  (void)printf("Unknown archiving method: `%s'\n", ag[1]);
}



static void near V_Pack(char *line,char *ag[])
{
  struct _arcinfo *ai;
  struct _sblist *node, last;
  int y;
  
  NW(line);
  
  if (config.arc==NULL)
  {
    BeforeCompress("Pack statement");
    return;
  }

  for (ai=config.arc; ai; ai=ai->next)
    if (eqstri(ai->arcname, ag[1]))
      break;

  if (! ai)
  {
    (void)printf("Unknown archiving method: `%s'\n", ag[1]);
    return;
  }
  
  last=config.def;

  for (y=2; y < MAXARGS && *ag[y]; y++)
  {
    node=smalloc(sizeof(struct _sblist));
    *node=last;
    
    ParseNN(ag[y], &node->zone, &node->net, &node->node, &node->point, TRUE);
    
    last=*node;
    last.point=0;

    node->next=ai->nodes;
    ai->nodes=node;
  }
}



static void near V_Password(char *line,char *ag[])
{
  struct _nodepwd *pwd;
  
  NW(line);

  pwd=smalloc(sizeof(struct _nodepwd));

  pwd->addr=config.def;

  ParseNN(ag[1], &pwd->addr.zone, &pwd->addr.net, 
          &pwd->addr.node, &pwd->addr.point, FALSE);

  (void)strcpy(pwd->pwd, ag[2]);

  /* Add to linked list */

  pwd->next=config.pwd;
  config.pwd=pwd;
}




static void near V_Address(char *line,char *ag[])
{
  struct _sblist *addr, *ad, *adlast, last;
  int x;

  NW(line);
  
  (void)memset(&last, '\0', sizeof(struct _sblist));

  last=config.def;
  
  for (x=1; x < MAXARGS && *ag[x]; x++)
  {
    addr=smalloc(sizeof(struct _sblist));
    
    *addr=last;

    ParseNN(ag[x], &addr->zone, &addr->net, &addr->node, &addr->point, FALSE);
    
    last=*addr;
    last.point=0;
 

    /* Unlike everything else, add the address to the END of the linked     *
     * list, instead of the front (like we do in all other cases).          */
       
    for (ad=config.addr, adlast=NULL; ad; adlast=ad, ad=ad->next)
      ;
    
    if (adlast)
      adlast->next=addr;
    else
    {
      config.addr=addr;
      config.def=*addr;
      config.def.point=0;
    }
  }
}


static void near Fwd(char *line, char *ag[], int ff)
{
  struct _fwdlist *fwd;
  struct _sblist last;
  word fil;
  int i;

  NW(line);

  fil=FALSE;

  last=config.def;

  for (i=1; i < MAXARGS && *ag[i]; i++)
  {
    if (eqstri(ag[i], "file"))
    {
      fil=TRUE;
      continue;
    }
    
    fwd=smalloc(sizeof(struct _fwdlist));
    fwd->file=fil;

    fwd->node=last;

    ParseNN(ag[i], &fwd->node.zone, &fwd->node.net, &fwd->node.node,
            &fwd->node.point, TRUE);
    
    last=fwd->node;
    last.point=0;

    /* Append to either the fwdfrom or fwdto linked list */
    
    if (ff)
    {
      fwd->next=config.fwdfrom;
      config.fwdfrom=fwd;
    }
    else
    {
      fwd->next=config.fwdto;
      config.fwdto=fwd;
    }
  }
}


static void near V_FwdFrom(char *line,char *ag[])
{
  Fwd(line, ag, TRUE);
}

static void near V_FwdTo(char *line,char *ag[])
{
  Fwd(line, ag, FALSE);
}



static void near ValidateCfg(void)
{
  if (SkipFirst(config.area)==NULL)
    ReportErr("At least one area must be defined!");

  if (config.arc==NULL)
    ReportErr("At least one archiver must be defined!");
  
  if (config.tpath==NULL)
    ReportErr("At least one netfile path must be defined!");
  
  if (config.netmail==NULL)
    ReportErr("No netmail area defined!");

  if (config.badmsgs==NULL)
    ReportErr("No bad messages area defined!");
  
  if (config.addr==NULL)
    ReportErr("At least one address must be defined!");
  
  if (config.origin==NULL)
    ReportErr("A default origin line must be defined!");

  if (config.outb==NULL)
    ReportErr("At least one outbound directory must be defined!");
}



struct _cfgtable
{
  char *verb;
  void (near *handler)(char *line,char *ag[]);
  unsigned op;
  void *flag;
  word mask;
};

#define VB_FUNC 0
#define VB_BYTE 1
#define VB_WORD 2
#define VB_FILE 3
#define VB_DIR  4
#define VB_FLAG 5
#define VB_STR  6
#define VB_FLG2 7
#define VB_FLGT 8

static struct _cfgtable vt[]=
{
  {"echoarea",        V_Area,       VB_FUNC,NULL,             0},
  {"netarea",         V_Area,       VB_FUNC,NULL,             0},
  {"badarea",         V_Area,       VB_FUNC,NULL,             0},
  {"dupearea",        V_Area,       VB_FUNC,NULL,             0},
  {"dupecheck",       V_DupeCheck,  VB_FUNC,NULL,             0},
  {"loglevel",        V_LogLevel,   VB_FUNC,NULL,             0},
  {"application",     NULL,         VB_FUNC,NULL,             0},
  {"app",             NULL,         VB_FUNC,NULL,             0},
  {"origin",          NULL,         VB_STR, &config.origin,   0},
  {"maxmsgs",         V_MaxMsgs,    VB_FUNC,NULL,             0},
  {"maxarchive",      V_MaxArchive, VB_FUNC,NULL,             0},
  {"zonegate",        V_ZoneGate,   VB_FUNC,NULL,             0},
  {"localarea",       NULL,         VB_FUNC,NULL,             0},
  {"address",         V_Address,    VB_FUNC,NULL,             0},
  {"forwardto",       V_FwdTo,      VB_FUNC,NULL,             0},
  {"forwardfrom",     V_FwdFrom,    VB_FUNC,NULL,             0},
  {"addtoseen",       V_AddToSeen,  VB_FUNC,NULL,             0},
  {"gateroute",       V_GateRoute,  VB_FUNC,NULL,             0},
  {"buffers",         V_Buffers,    VB_FUNC,NULL,             0},
  {"pack",            V_Pack,       VB_FUNC,NULL,             0},
  {"defaultpacker",   V_DefaultPack,VB_FUNC,NULL,             0},
  {"password",        V_Password,   VB_FUNC,NULL,             0},
  {"remap",           V_Remap,      VB_FUNC,NULL,             0},
  {"tinyseenbys",     V_Tiny,       VB_FUNC,NULL,             0},
  {"maxpkt",          NULL,         VB_WORD,&config.maxpkt,   0},
  {"maxattach",       NULL,         VB_WORD,&config.maxattach,0},
  {"pointnet",        NULL,         VB_WORD,&config.pointnet, 0},
  {"duplicates",      V_Duplicates, VB_FUNC,NULL,             0},
  {"addmode",         NULL,         VB_FLAG,NULL,             FLAG_ADDMODE},
/*  {"stripattributes", NULL,         VB_FLAG,NULL,             FLAG_STRPATTR},*/
  {"stripattributes", V_StripAttributes, VB_FUNC, NULL,       0},
  {"statistics",      V_Statistics, VB_FUNC,NULL,             0},
  {"batchunarc",      NULL,         VB_FLAG,NULL,             FLAG_BATCHXA},
  {"tossbadmsgs",     NULL,         VB_FLGT,NULL,             FLAG_TOSSBAD},
  {"checkzones",      NULL,         VB_FLAG,NULL,             FLAG_CHECKZ},
  {"arcmailattach",   NULL,         VB_FLAG,NULL,             FLAG_FRODO},
  {"busyflags",       NULL,         VB_FLAG,NULL,             FLAG_BSY},
  {"killblank",       NULL,         VB_FLAG,NULL,             FLAG_KILLZ},
  {"killintransit",   NULL,         VB_FLAG,NULL,             FLAG_KILLFWD},
  {"killdupes",       NULL,         VB_FLAG,NULL,             FLAG_KILLDUPE},
  {"savecontrolinfo", NULL,         VB_FLAG,NULL,             FLAG_SAVECTRL},
  {"oldarcmailexts",  NULL,         VB_FLAG,NULL,             FLAG_OLDARCM},
  {"quietarc",        NULL,         VB_FLAG,NULL,             FLAG_QUIETARC},
  {"secure",          NULL,         VB_FLGT,NULL,             FLAG_SECURE},
  {"binkpoint",       NULL,         VB_FLG2,NULL,             FLAG2_BINKPT},
  {"nuke",            NULL,         VB_FLG2,NULL,             FLAG2_NUKE},
  {"nosoundex",       NULL,         VB_FLG2,NULL,             FLAG2_NOSNDX},
  {"nostomp",         NULL,         VB_FLG2,NULL,             FLAG2_NOSTOMP},
  {"killintransitfile",NULL,        VB_FLG2,NULL,             FLAG2_KFFILE},
  {"linkmsgid",       NULL,         VB_FLG2,NULL,             FLAG2_LMSGID},
  {"dupelongheader",  NULL,         VB_FLG2,NULL,             FLAG2_LONGHDR},
  {"netfile",         V_Netfile,    VB_FUNC,NULL,             0},
  {"outbound",        V_Outbound,   VB_FUNC,NULL,             0},
  {"swap",            V_Swap,       VB_STR, &config.swappath, 0},
  {"compress",        V_Compress,   VB_FILE,&config.compress_cfg,0},
  {"areasbbs",        NULL,         VB_FILE,&config.areasbbs, 0},
  {"routing",         V_Routing,    VB_FILE,&config.routing,  0},
  {"logfile",         NULL,         VB_FILE,&config.logfile,  0},
#ifdef OS_2
  #ifdef __FLAT__
    {"feature",         NULL,         VB_FUNC,NULL,             0},
    {"feature32",       V_Feature,    VB_FUNC,NULL,             0},
  #else
    {"feature",         V_Feature,    VB_FUNC,NULL,             0},
    {"feature32",       NULL,         VB_FUNC,NULL,             0},
  #endif
#endif
  {"track",           NULL,         VB_FILE,&config.tracklog, 0}
};

#define vtlen (unsigned)(sizeof(vt)/sizeof(vt[0]))


#pragma on(check_stack) /* Stack checking for our config file parser */
         
static void near Parse1Config(char *cfgname, char *args[MAXARGS],
                              char *in, char *line)
{
  FILE *cfgfile;
  char *p, *t;
  char **pparg;
  unsigned i, op;

  if ((cfgfile=shfopen(cfgname, "r", O_RDONLY))==NULL)
    ErrOpening("config", cfgname);

  while (fgets(in, CFGLEN, cfgfile))
  {
    if ((p=strpbrk(in,";\n")) != NULL)
      *p='\0';

    (void)strcpy(line, in);
      
    p=strtok(line, cfgdelim);

    for (i=0; p && i < MAXARGS; i++)
    {
      (void)strcpy(args[i], p);
      p=strtok(NULL, cfgdelim);
    }

    args[i][0]='\0';
    
    if (! *args[0])
      continue;
    
    for (i=0; i < vtlen; i++)
      if (eqstri(args[0], vt[i].verb))
      {
        op=vt[i].op;

        switch(op)
        {
          case VB_BYTE:
            *(char *)vt[i].flag=TRUE;
            break;
            
          case VB_WORD:
            *(word *)vt[i].flag=(word)atoi(args[1]);
            break;
            
          case VB_FILE:
          case VB_DIR:
          case VB_STR:
            pparg=(char **)vt[i].flag;

            if (*pparg)
              free(*pparg);

            if (op==VB_STR)
            {
              /* A string, so snatch the whole line */

              p=firstchar(in, cfgdelim, 2);

              if (!p)
                p="";
            }
            else /* either a VB_DIR or a VB_FILE, so use only one word */
            {
              p=args[1];
            }
            
            *pparg=sstrdup(p);

            t=*pparg;

            if (op==VB_DIR && strlen(t) > 3)
              (void)Strip_Trailing(t,'\\');
            break;
            
          case VB_FLAG:
            config.flag |= vt[i].mask;
            break;

          case VB_FLGT:
            config.flag ^= vt[i].mask;
            break;

          case VB_FLG2:
            config.flag2 |= vt[i].mask;
            break;

          default:
            break;
        }


        if (vt[i].handler)
          (*vt[i].handler)(in, args);

        break;
      }

#ifdef OS_2 /* Feature-specific config lines */
    {
      struct _feature *pf;
      struct _feat_config fc;

      fc.struct_len=sizeof fc;
      (void)strcpy(fc.szConfigLine, in);
      fc.ppszArgs=args;

      /* Search through all of the features */

      for (pf=config.feat; pf; pf=pf->pfNext)
      {
        char *s;

        /* Search through the config name */

        for (p=pf->pszConfigName; p; p=strchr(p, '\r'))
        {
          /* Skip over the \r, if necessary */

          if (*p=='\r')
            p++;


          /* The next \r or end of string delimits the length of keyword */

          if ((s=strchr(p, '\r'))==NULL)
            s=p+strlen(p);


          /* Compare it to our keyword */

          if (eqstrni(args[0], p, (unsigned)(s-p)))
          {
            if ((*pf->pfnConfig)(&fc))
              exit(ERL_ERROR);

            i=0;    /* So we don't trigger "invalid command" */
            /*break;*/  /* No need to search more than one */
          }
        }
      }
    }
#endif  /* OS_2 */

    /* If we reached the end of the verb table */

    if (i==vtlen)
    {
      if (!eqstri(args[0], "include"))
        (void)printf("Invalid command in %s: `%s'\n", cfgname, args[0]);
      else
      {
        char fname[PATHLEN];

        /* Close the current file handle */

        long pos=ftell(cfgfile);
        (void)fclose(cfgfile);

        /* Store the name of the config file on our local stack */

        (void)strcpy(fname, args[1]);

        /* Parse the included config file */

        Parse1Config(fname, args, in, line);

        /* Reopen the current config file */

        if ((cfgfile=shfopen(cfgname, "r", O_RDONLY))==NULL)
          ErrOpening("config", cfgname);

        (void)fseek(cfgfile, pos, SEEK_SET);
      }
    }
  }
  
  (void)fclose(cfgfile);
}

#pragma off(check_stack) /* Stack checking for our config file parser */


/* External entrypoint for config file parser */

void Parse_Config(char *cfgname)
{
  char *args[MAXARGS]={NULL};
  char *in, *line;
  unsigned i;

  /* Allocate memory for the config file parsing routines */

  in=smalloc(CFGLEN);
  line=smalloc(CFGLEN);

  for (i=0; i < MAXARGS; i++)
    args[i]=smalloc(ARGLEN);

  /* Parse the config file */

  Parse1Config(cfgname, args, in, line);

  /* Free memory */

  for (i=0; i < MAXARGS; i++)
    free(args[i]);

  free(line);
  free(in);

  /* Now ensure that the config file is correct */

  ValidateCfg();
}




