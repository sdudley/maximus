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

/* $Id: s_marea.c,v 1.3 2004/01/22 08:04:28 wmcbrine Exp $ */

/*# name=SILT: 'Section Area' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT
#define NO_MSGH_DEF

#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "s_heap.h"
#include "s_marea.h"

static long cbLast=0;
static word division=0;
static char prefix[PATHLEN]="";

#ifdef MAX_TRACKER

#include "trackc.h"
#include "trackp.h"

static TRK_OWNER toNewOwner;

/* Add a new owner for this area */

static void near FiltOwner(void *v, char *words[], char *line)
{
  NW(v); NW(words);

  strncpy(toNewOwner, line, sizeof(toNewOwner)-1);
  toNewOwner[sizeof(toNewOwner)-1]=0;

  if (strlen(line) >= sizeof(toNewOwner))
  {
    printf("\n\aError!  Owner \"%s\" is too long (must be <= %d chars)\n",
           line, sizeof(toNewOwner)-1);
    Compiling(-1, NULL, NULL);
  }
}

#endif


static void near FiltBarricade(void *v, char *words[], char *line)
{
  NW(line);

  if (! *words[1] || !*words[2])
  {
    printf("\n\aError!  Two arguments required for Barricade keyword on line %d!\n", linenum);
    Compiling(-1,NULL,NULL);
  }

  HeapAdd(&PMI(v)->h, &PMI(v)->ma.barricademenu, words[1]);
  HeapAdd(&PMI(v)->h, &PMI(v)->ma.barricade, words[2]);
}




/* Truncate origin to 61 chars */

static void near FiltOrigin(void *v, char *words[], char *line)
{
  char *txt=fchar(line, ctl_delim, 3);

  if (*words[1] != '.')
  {
    if (! isdigit(*words[1]))
    {
      printf("\a\nWarning!  First word after 'Origin' on line %d must be a network address.\n",
             linenum);
      Compiling(-1, NULL, NULL);
    }

    ParseNNN(words[1], &PMI(v)->ma.primary, FALSE);
  }

  if (*words[2] != '.')
  {
    if (! isdigit(*words[2]))
    {
      printf("\a\nWarning!  Second word after 'Origin' on line %d must be a network address.\n",
             linenum);
      Compiling(-1, NULL, NULL);
    }

    ParseNNN(words[2], &PMI(v)->ma.seenby, FALSE);
  }

  strocpy(line, txt);
}



/* Message area style information (read-only, etc.) */

static void near FiltStyle(void *v, char *words[], char *line)
{
  int w;  /* Word number */

  static struct
  {
    char *name;
    word mask1;
    word mask2;
  } *pst, style_tab[]=
    {
      {"Pvt",           MA_PVT,     0},
      {"Private",       MA_PVT,     0},
      {"Pub",           MA_PUB,     0},
      {"Public",        MA_PUB,     0},
      {"ReadOnly",      MA_READONLY,0},
      {"HiBit",         MA_HIBIT,   0},
      {"HighBit",       MA_HIBIT,   0},
      {"Net",           MA_NET,     0},
      {"Matrix",        MA_NET,     0},
      {"Echo",          MA_ECHO,    0},
      {"EchoMail",      MA_ECHO,    0},
      {"Conf",          MA_CONF,    0},
      {"Conference",    MA_CONF,    0},
      {"Loc",           0,          0},
      {"Local",         0,          0},
      {"Anon",          MA_ANON,    0},
      {"Anonymous",     MA_ANON,    0},
      {"NoNameKludge",  MA_NORNK,   0},
      {"RealName",      MA_REAL,    0},
      {"Alias",         MA_ALIAS,   0},
      {"Audit",         MA_AUDIT,   0},
      {"Hidden",        MA_HIDDN,   0},
      {"Attach",        MA_ATTACH,  0},
      {"NoMailCheck",   0,          MA2_NOMCHK},
      {NULL, 0}
    };

  NW(line);

  /* Parse all of the known keywords */

  for (w=1; w < MAX_PARSE_WORDS && *words[w]; w++)
  {
    for (pst=style_tab; pst->name; pst++)
      if (eqstri(words[w], pst->name))
      {
        PMI(v)->ma.attribs |= pst->mask1;
        PMI(v)->ma.attribs_2 |= pst->mask2;
        break;
      }

    if (!pst->name)
    {
      /* Handle exceptions */

      if (eqstri(words[w], "squish"))
        PMI(v)->ma.type=MSGTYPE_SQUISH;
      else if (eqstri(words[w], "*.msg") || eqstri(words[w], "sdm"))
        PMI(v)->ma.type=MSGTYPE_SDM;
      else
        Unknown_Ctl(linenum, words[w]);
    }
  }
}


/* Message area renumbering information */

static void near FiltRenum(void *v, char *words[], char *line)
{
  int atoi2;

  NW(line);

  atoi2=atoi(words[2]);

  if (eqstri(words[1], "max"))
    PMI(v)->ma.killbynum=atoi2;
  else if (eqstri(words[1], "days"))
    PMI(v)->ma.killbyage=atoi2;
  else if (eqstri(words[1], ""))
    PMI(v)->ma.killskip=atoi2;
  else
    Unknown_Ctl(linenum, words[2]);
}


/* Make sure that area exists and update parameters in base header */

static void near assert_msgarea(char *path, word type, word killbyage,
                                word killbynum, word killskip)
{
  HAREA ha;
  char szPath[PATHLEN];
  char szUp[PATHLEN];

  dword dwKillByNum = killbynum ? (dword)killbynum : (dword)-1L;
  dword dwKillSkip = killskip ? (dword)killskip : (dword)-1L;
  dword dwKillByAge = killbyage ? (dword)killbyage : (dword)-1L;


  strcpy(szPath, path);

  /* Make sure that path does not contain a trailing backslash */

  if (strlen(szPath) > 3)
    Strip_Trailing(szPath, PATH_DELIM);

  if ((ha=MsgOpenArea(szPath, MSGAREA_CRIFNEC, type))==NULL)
  {
    char *p;

    strcpy(szUp, szPath);

    if ((p=strrstr(szUp, ":\\/")) != NULL)
      *p=0;

    if (*szUp && !direxist(szUp))
      makedir(szUp);

    if ((ha=MsgOpenArea(szPath, MSGAREA_CRIFNEC, type))==NULL)
    {
      printf("\a\nError creating area %s!\n", path);
      Compiling(-1,NULL,NULL);
      return;
    }
  }

  /* Set the parameters for a Squish area */

  if (type & MSGTYPE_SQUISH)
    SquishSetMaxMsg(ha, dwKillByNum, dwKillSkip, dwKillByAge);

  MsgCloseArea(ha);
}


static void near MsgAreaWrite(MAINFO *pmi, int closeit)
{
  static int mai_fd=-1;
  static int ma_fd=-1;
  unsigned size;
  OVRLIST ol;
  struct _mfidx mfi;

  if (!do_marea)
    return;

  if (!pmi && ma_fd != -1 && closeit)
  {
    MAREA ma;
    long pos=lseek(ma_fd, 0L, SEEK_END);

    /* Find out the position of the last area in the file, so that we       *
     * can fix the wraparound pointer in the first record to point          *
     * to it.                                                               */

    pos -= cbLast + ADATA_START;

    lseek(ma_fd, ADATA_START, SEEK_SET);

    if (read(ma_fd, (char *)&ma, sizeof ma) != sizeof ma)
    {
      printf("Error reading final msg area data!\n");
      exit(1);
    }

    lseek(ma_fd, ADATA_START, SEEK_SET);

    ma.cbPrior=-pos;

    if (write(ma_fd, (char *)&ma, sizeof ma) != sizeof ma)
    {
      printf("Error writing final msg area data!\n");
      exit(1);
    }

    close(ma_fd);
    close(mai_fd);
    ma_fd=-1;
    return;
  }

  if (ma_fd==-1 && !closeit)
  {
    char fname[PATHLEN];
    dword dwId=MAREA_ID;

    if (strings[prm.marea_name]==0)
    {
      printf("Error!  MSGAREA.CTL cannot be SILTed separately.  Use\n"
             "\"SILT MAX\" and ensure that you have a \"Messages marea\"\n"
             "keyword in the Session Section.\n");
      exit(1);
    }

    strcpy(fname, strings + prm.marea_name);
    strcat(fname, ".dat");

    if ((ma_fd=sopen(fname, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                     SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    {
      printf("\nCan't open msg area data file %s!\n", fname);
      exit(1);
    }

    if (write(ma_fd, (char *)&dwId, sizeof dwId) != sizeof dwId)
    {
      printf("\aError writing key to msg data file %s\n", fname);
      exit(1);
    }

    strcpy(fname, strings + prm.marea_name);
    strcat(fname, ".idx");

    if ((mai_fd=sopen(fname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                      SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    {
      printf("\nCan't open msg area index file %s!\n", fname);
      exit(1);
    }
  }

  if (pmi)
  {
    /* Fill out the structure length information */

    pmi->ma.cbArea=sizeof(pmi->ma);
    pmi->ma.cbHeap=pmi->h.end - pmi->h.heap;
    pmi->ma.cbPrior=cbLast;

    /* Record the size of this area */

    cbLast=pmi->ma.cbArea + pmi->ma.cbHeap +
           pmi->ma.num_override * sizeof(OVERRIDE);

    /* Default to Squish format */

    if (!pmi->ma.type)
      pmi->ma.type=MSGTYPE_SQUISH;

    if ((pmi->ma.attribs & (MA_PUB|MA_PVT))==0)
      pmi->ma.attribs |= MA_PUB;

    /* Strip the trailing backslash for Squish-style msg areas */

    if (pmi->ma.type & MSGTYPE_SQUISH)
    {
      char *p=pmi->h.heap + pmi->ma.path;
      int len;

      if (p[len=strlen(p)-1]==PATH_DELIM)
        p[len]=0;
    }

    /* Write the location of this area to the index file */

    strnncpy(mfi.name, pmi->h.heap + pmi->ma.name, sizeof(mfi.name));
    mfi.name_hash=SquishHash(pmi->h.heap + pmi->ma.name);
    mfi.ofs=tell(ma_fd);


    /* Touch the area's header, if necessary */

    if ((pmi->ma.attribs & (MA_DIVBEGIN|MA_DIVEND))==0)
      assert_msgarea(pmi->h.heap + pmi->ma.path, pmi->ma.type,
                     pmi->ma.killbyage, pmi->ma.killbynum,
                     pmi->ma.killskip);

    /* Do a little bit of error checking */

    if (pmi->ma.attribs & (MA_AUDIT) &&
        pmi->ma.type != MSGTYPE_SQUISH)
    {
      printf("\n\aError!  Style Audit can only be used in Squish areas! (area ending on line %d)\n",
             linenum);
      Compiling(-1, NULL, NULL);

      pmi->ma.attribs &= ~MA_AUDIT;
    }

    if (pmi->ma.attribs & (MA_ATTACH) &&
        pmi->ma.type != MSGTYPE_SQUISH)
    {
      printf("\n\aError!  Style Attach can only be used in Squish areas! (area ending on line %d)\n",
             linenum);
      Compiling(-1, NULL, NULL);

      pmi->ma.attribs &= ~MA_ATTACH;
    }

    /* Write the header to disk */

    if (write(mai_fd, (char *)&mfi, sizeof mfi) != sizeof mfi)
      ErrWrite();

    /* Write the main area data structure */

    if (write(ma_fd, (char *)&pmi->ma, sizeof pmi->ma) != sizeof pmi->ma)
      ErrWrite();

    /* Write all of the message area overrides */

    for (ol=pmi->ol; ol; ol=ol->next)
      if (write(ma_fd, (char *)&ol->or, sizeof ol->or) != sizeof ol->or)
        ErrWrite();

    /* Figure out the size of the zstr heap and write it, too */

    size=pmi->h.end-pmi->h.heap;

    if (write(ma_fd, (char *)pmi->h.heap, size) != (signed)size)
      ErrWrite();
  }

#ifdef MAX_TRACKER
  if (*toNewOwner)
  {
    TRK t;

    if ((t=TrkOpen("TRK", TRUE)) != NULL)
    {
      TrkSetDefaultOwner(t, pmi->h.heap + pmi->ma.name, toNewOwner);
      TrkClose(t);
    }

    *toNewOwner=0;
  }
#endif
}

void MsgAreaClose(void)
{
  MsgAreaWrite(NULL, TRUE);
}

int ParseMsgArea(FILE *ctlfile, char *name)
{
  char line[PATHLEN];
  char fullname[PATHLEN];
  static MAINFO mi;
  OVRLIST ol;

  struct _vbtab verbs[]=
  {
    {0,           "acs",          &mi.ma.acs},
    {FiltPath,    "path",         &mi.ma.path},
    {0,           "tag",          &mi.ma.echo_tag},
    {0,           "desc",         &mi.ma.descript},
    {0,           "description",  &mi.ma.descript},
    {FiltOrigin,  "origin",       &mi.ma.origin},
#ifdef MAX_TRACKER
    {FiltOwner,   "owner",        NULL},
#endif
    {FiltMenuname,"menuname",     NULL},
    {FiltOverride,"override",     NULL},
    {FiltStyle,   "style",        NULL},
    {FiltRenum,   "renum",        NULL},
    {FiltBarricade,"barricade",   NULL},
    {0,           "app",          NULL},
    {0,           "application",  NULL},
    {0,           "attachpath",   &mi.ma.attachpath},
    {0,           NULL,           NULL}
  };


  if (strchr(name, '.'))
    BadDivisionName();


  if (do_marea)
  {
    /* Make sure that area file is open */

    MsgAreaWrite(NULL, FALSE);

    memset(&mi, 0, sizeof mi);
    mi.marea=TRUE;
    mi.ma.division=division;

    mi.ma.primary=prm.address[0];
    mi.ma.seenby=prm.address[!!prm.address[0].point && prm.address[1].zone];

    HeapNew(&mi.h, MAX_MSG_HEAP);

    strcpy(fullname, prefix);
    strcat(fullname, name);

    HeapAdd(&mi.h, &mi.ma.name, fullname);

#ifdef MAX_TRACKER
    *toNewOwner=0;
#endif
  }

  while (fgets(line, PATHLEN, ctlfile))
    if (VerbParse(&mi, do_marea ? verbs : NULL, line))
      break;

  if (do_marea)
  {
    MsgAreaWrite(&mi, FALSE);

    for (ol=mi.ol; ol; ol=ol->next)
      free(ol);

    HeapDelete(&mi.h);
  }

  return 0;
}


void ParseMsgDivisionBegin(char *name, char *acs, char *displayfile,
                           char *descript)
{
  MAINFO mi;
  char fullname[PATHLEN];

  if (!do_marea)
    return;

  if (strchr(name, '.'))
    BadDivisionName();

  strcpy(fullname, prefix);
  strcat(fullname, name);

  strcat(prefix, name);
  strcat(prefix, ".");

  MsgAreaWrite(NULL, FALSE);

  memset(&mi, 0, sizeof mi);
  mi.marea=TRUE;

  HeapNew(&mi.h, MAX_MSG_HEAP);

  HeapAdd(&mi.h, &mi.ma.name, fullname);
  HeapAdd(&mi.h, &mi.ma.descript, descript);
  HeapAdd(&mi.h, &mi.ma.acs, acs);
  HeapAdd(&mi.h, &mi.ma.path, displayfile);

  mi.ma.attribs = MA_DIVBEGIN;
  mi.ma.division = division++;

  MsgAreaWrite(&mi, FALSE);
  HeapDelete(&mi.h);
}

void ParseMsgDivisionEnd(void)
{
  MAINFO mi;
  char *p;

  if (!do_marea)
    return;

  if (! *prefix)
  {
    printf("\n\aError!  MsgDivisionEnd on line %d has no correspondig MsgDivisionBegin!\n", linenum);
    Compiling(-1, NULL, NULL);
    return;
  }

  /* Strip off the trailing dot */

  prefix[strlen(prefix)-1]=0;

  /* Now set the prefix back to the prior area */

  if ((p=strrchr(prefix, '.')) != NULL)
    p[1]=0;
  else *prefix=0;

  MsgAreaWrite(NULL, FALSE);

  memset(&mi, 0, sizeof mi);
  mi.marea=TRUE;

  HeapNew(&mi.h, MAX_MSG_HEAP);

  mi.ma.attribs = MA_DIVEND;
  mi.ma.division = --division;
  HeapAdd(&mi.h, &mi.ma.acs, "");

  MsgAreaWrite(&mi, FALSE);
  HeapDelete(&mi.h);
}

