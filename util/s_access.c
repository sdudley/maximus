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
static char rcs_id[]="$Id: s_access.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Access' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
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
#include "opusprm.h"
#include "dr.h"

#define ACSHEAPLEN  2048
#define ACSNO       32

void Add_Access(struct _clsrec * cls);

static char *acsheap;
static int acsofs;
static int clalloc;

CLH clh;
PCLH pclh=&clh;

static void near Init_Access(void);
static void near Init_Class(struct _clsrec *cls, char *name);
static word near InsertClassHeap(char *s);


int ParseAccess(FILE *ctlfile,char *name)
{
  int x;
  struct _clsrec thiscls;
  char temp[PATHLEN];         /* Scratch buffer */
  char line[MAX_LINE];        /* Contains entire line */
  char keyword[MAX_LINE];     /* First word on line */

  linenum++;

  Init_Access();
  Init_Class(&thiscls,name);

  if (! *name)
  {
    printf("\n\aInvalid access statement in line %d of CTL file!\n",linenum);
    exit(1);
  }
  
  if (!done_sys && prm.access)
  {
    printf("\n\n\aError!  All access statements must come AFTER the System,\n"
           "section!  (Maybe ACCESS.CTL was included in the wrong spot?)\n"
           "and `File Access <file>' must be specified.\n");
    exit(1);
  }

  while (fgets(line,MAX_LINE,ctlfile) != NULL)
  {
    Strip_Comment(line);

    if (! *line)
    {
      linenum++;
      continue;
    }

    priv_word=2;

    getword(line,keyword,ctl_delim,1);
    getword(line,temp,ctl_delim,2);

    if (! *keyword)
      ;
    else if (eqstri(keyword,"end"))
      break;
    else if (eqstri(keyword,"level"))
      thiscls.usLevel=(word)atol(temp);
    else if (eqstri(keyword,"desc") || eqstri(keyword,"description"))
    {
      char * p=fchar(line,ctl_delim,2);
      if (strcmp(acsheap+thiscls.zDesc,p) != 0)
        thiscls.zDesc=InsertClassHeap(p);
    }
    else if (eqstri(keyword, "alias"))
    {
      char *p = fchar(line, ctl_delim, 2);
      thiscls.zAlias = InsertClassHeap(p);
    }
    else if (eqstri(keyword,"key"))
      thiscls.usKey=toupper(*temp);
    else if (eqstri(keyword,"time"))
      thiscls.usTimeCall=(word)atol(temp);
    else if (eqstri(keyword,"cume"))
      thiscls.usTimeDay=(word)atol(temp);
    else if (eqstri(keyword,"calls"))
      thiscls.usCallsDay=(word)atol(temp);
    else if (eqstri(keyword,"logonbaud") || eqstri(keyword,"logon"))
      thiscls.usMinBaud=(word)(atol(temp)/100);
    else if (eqstri(keyword,"xferbaud") || eqstri(keyword,"xfer"))
      thiscls.usFileBaud=(word)(atol(temp)/100);
    else if (eqstri(keyword,"filelimit"))
      thiscls.ulFileLimit=(dword)atol(temp);
    else if (eqstri(keyword,"fileratio"))
      thiscls.usFileRatio=(word)atol(temp);
    else if (eqstri(keyword,"uploadreward") || eqstri(keyword,"reward"))
      thiscls.usUploadReward=(word)atol(temp);
    else if (eqstri(keyword,"ratiofree") || eqstri(keyword,"free"))
      thiscls.usFreeRatio=(word)atol(temp);
    else if (eqstri(keyword,"oldpriv"))
      thiscls.usOldPriv=(word)atol(temp);
    else if (eqstri(keyword,"loginfile") || eqstri(keyword,"file"))
    {
      if (strchr(temp,'\\')==NULL)
      {
        strcpy(line,temp);
        sprintf(temp, "%s%s", strings+prm.misc_path,line);
      }
      thiscls.zLoginFile=InsertClassHeap(temp);
    }
    else if (eqstri(keyword,"flags"))
    {
      static struct
      {
        char *kwd;
        dword flag;
      } flwords[] =
        {
          { "uploadany",    CFLAGA_ULBBSOK  },
          { "dloadhidden",  CFLAGA_FLIST    },
          { "showallfiles", CFLAGA_FHIDDEN  },
          { "showhidden",   CFLAGA_UHIDDEN  },
          { "hide",         CFLAGA_HIDDEN   },
          { "hangup",       CFLAGA_HANGUP   },
          { "nofilelimit",  CFLAGA_NOLIMIT  },
          { "notimelimit",  CFLAGA_NOTIME   },
          { "nolimits",     CFLAGA_NOLIMIT|CFLAGA_NOTIME },
          { NULL }
        };

      for (x=2; *temp; getword(line, temp, ctl_delim,++x))
      {
        int i;
        for (i=0; flwords[i].kwd ; ++i)
          if (eqstri(temp,flwords[i].kwd))
          {
            thiscls.ulAccFlags |= flwords[i].flag;
            break;
          }
        if (flwords[i].kwd==NULL)
          Unknown_Ctl(linenum,temp);
      }
    }
    else if (eqstri(keyword,"userflags") || eqstri(keyword, "user"))
    {
      int ishex=FALSE;
      char *an=temp;

      if (*an=='$' || toupper(*an)=='X')
      {
        ++an;
        ishex=TRUE;
      }
      else if (*an=='0' && toupper(an[1])=='X')
      {
        an+=2;
        ishex=TRUE;
      }
      sscanf(an,(ishex)?"%lx":"%ld", &thiscls.ulUsrFlags);
    }
    else if (eqstri(keyword,"mailflags") || eqstri(keyword,"mail"))
    {
      static struct
      {
        char *kwd;
        dword flag;
      } flwords[] =
        {
          { "showpvt",      CFLAGM_PVT      },
          { "editor",       CFLAGM_EDITOR   },
          { "localeditor",  CFLAGM_LEDITOR  },
          { "netfree",      CFLAGM_NETFREE  },
          { "msgattrany",   CFLAGM_ATTRANY  },
          { "writerdonly",  CFLAGM_RDONLYOK },
          { "norealname",   CFLAGM_NOREALNM },
          { NULL }
        };

      for (x=2; *temp; getword(line, temp, ctl_delim,++x))
      {
        int i;
        for (i=0; flwords[i].kwd; ++i)
          if (eqstri(temp,flwords[i].kwd))
          {
            thiscls.ulMailFlags |= flwords[i].flag;
            break;
          }
        if (flwords[i].kwd==NULL)
          Unknown_Ctl(linenum,temp);
      }
    }
    else if (eqstri(keyword, "app") || eqstri(keyword, "application"))
      ;
    else Unknown_Ctl(linenum,keyword);

    linenum++;
  }

  linenum++;

  Add_Access(&thiscls);

  /* We write this only on completion as the .PRM file is written */

  return 0;
}



static void near Init_Access(void)
{
  if (!acsheap)
  {
    if ((acsheap=malloc(ACSHEAPLEN))==NULL)
      NoMem();

    clh.usn=0;
    clh.ussize=sizeof(struct _clsrec);
    if ((clh.pcInfo=malloc(clh.ussize*ACSNO))==NULL)
      NoMem();
    clalloc=ACSNO;
    clh.pHeap=acsheap;
    pclh=&clh;

    acsheap[0]='\0';
    acsofs=1;
  }
}



static void near Init_Class(struct _clsrec *cls, char *name)
{
  memset(cls,'\0',sizeof(struct _clsrec));

  /* Init with some reasonable defaults */

  cls->usLevel=10;
  cls->usKey=toupper(*name);
  cls->zAbbrev=cls->zDesc=InsertClassHeap(name);
  cls->usTimeDay=60;
  cls->usTimeCall=45;
  cls->usCallsDay=(word)-1;
  cls->usMinBaud=3;
  cls->usFileBaud=3;
  cls->ulFileLimit=1000;
  cls->usOldPriv=(word)-1;
}


static word near InsertClassHeap(char *s)
{
  int len=strlen(s);
  word oldofs;
  
  if (acsofs+len >= ACSHEAPLEN)
  {
    printf("Error!  Access file too big (`%s')\n",s);
    exit(1);
  }
  
  strcpy(acsheap+acsofs, s);
  oldofs=acsofs;
  acsofs += len+1;
  return oldofs;
}

static int cmpclass(void const *a, void const *b)
{
  struct _clsrec const *A = a;
  struct _clsrec const *B = b;
  return (A->usLevel < B->usLevel) ? -1 : (A->usLevel > B->usLevel) ? 1 : 0;
}

  /* Add a new access level to the array */

void Add_Access(struct _clsrec * cls)
{
  static int prm_class = 0;       /* First 12 classes stored in prm file */
  int i;
  char *nm=acsheap+cls->zAbbrev;

  /* Check for duplicates */

  for (i=0; i < clh.usn; ++i)
  {
    char *p=acsheap+clh.pcInfo[i].zAbbrev;

    if (eqstri(nm,p))
    {
      printf("\nError! Class `%s' has already been defined\n",nm);
      Compiling(-1,NULL,NULL);
      return;
    }

    if (cls->usLevel == clh.pcInfo[i].usLevel)
    {
      printf("\nWarning! Class `%s' has the same priv level as `%s' (%d)\n",
              nm, p, cls->usLevel);
      Compiling(-1,NULL,NULL);
    }

    if (cls->usKey && cls->usKey == clh.pcInfo[i].usKey)
    {
      printf("\nWarning! Class `%s' has the same priv key as `%s' (%c)\n",
              nm, p, cls->usKey);
      Compiling(-1,NULL,NULL);
    }
  }

  if (i == clalloc) /* Need to realloc */
  {
    clalloc+=ACSNO;
    if ((clh.pcInfo=realloc(clh.pcInfo,clalloc*sizeof(struct _clsrec)))==NULL)
      NoMem();
  }

  clh.pcInfo[i]=*cls;
  clh.usn++;

  /* We want to keep these records sorted by priv level for efficiency,
     so if there is more than one record, we should qsort() them */

  if (clh.usn > 1)
    qsort(clh.pcInfo, clh.usn, sizeof(*cls), cmpclass);


  /* Copy this to the .prm file for compatibility reasons, but omit
   * the HIDDEN (11) and classes with undeclared OldPriv levels.
   */

  if (prm_class < MAXCLASS &&
      cls->usOldPriv != 11 &&
      cls->usOldPriv != (word)-1)
  {
    prm.cls[prm_class].priv          = cls->usOldPriv;
    prm.cls[prm_class].max_time      = cls->usTimeDay;
    prm.cls[prm_class].max_call      = cls->usTimeCall;
    prm.cls[prm_class].max_dl        = cls->ulFileLimit;
    prm.cls[prm_class].ratio         = cls->usFileRatio;
    prm.cls[prm_class].min_baud      = cls->usMinBaud * 100;
    prm.cls[prm_class].min_file_baud = cls->usFileBaud * 100;
    prm_class++;
  }
}

  /* Write the new access file */

void Write_Access()
{
  int acsfile;
  char temp[PATHLEN];
  struct _clshdr  chdr;

  memset(&chdr, 0, sizeof chdr);

  chdr.ulclhid=CLS_ID;
  chdr.usclfirst=sizeof chdr;
  chdr.usn=clh.usn;
  chdr.ussize=sizeof(struct _clsrec);
  chdr.usstr=acsofs+1;

  sprintf(temp, "%s.dat", strings+prm.access);

  if ((acsfile=open(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                 S_IREAD | S_IWRITE))==-1)
  {
    printf("\n\aError opening `%s' for write!\n",temp);
    exit(1);
  }
  write(acsfile, (char*)&chdr, sizeof chdr);
  write(acsfile, (char*)clh.pcInfo, sizeof(struct _clsrec)*clh.usn);
  write(acsfile, acsheap, acsofs+1);
  close(acsfile);
}

CLSREC *ClassRec(int idx)
{
  if (idx < 0 || idx > clh.usn)
  {
    printf("\a\nInvalid priv index `%d' on line %d of CTL file!\n",idx,linenum);
    exit(1);
  }
  return clh.pcInfo + idx;
}

int Deduce_Priv(char *p)
{
  int   i, n;

    /* Assume it is a numeric privilege */

  if (isdigit(*p) || *p=='-')
    return (word)atol(p);

  n=strcspn(p, " \t/,-=+<>()&|");
  for (i=0; i < clh.usn; ++i)
  {
    CLSREC *pcr = ClassRec(i);
    char *cName = clh.pHeap + pcr->zAbbrev;
    char *cAlias = clh.pHeap + pcr->zAlias;

    if ((strnicmp(cName,p,n)==0 && cName[n]=='\0') ||
        (strnicmp(cAlias,p,n)==0 && cAlias[n]=='\0'))
    {
      return pcr->usLevel;
    }
  }

  if (stricmp(p,"noaccess")==0)
    return (int)(unsigned int)(word)-1;

  printf("\a\nInvalid priv `%s' on line %d of CTL file!\n",p,linenum);
  exit(1);
  return -1;  /*NOTREACHED*/
}

int Deduce_Class(int priv)
{
  int     i, j=-1;

  for (i=0; i<pclh->usn; j=i++)
  {
    CLSREC *pcr=ClassRec(i);
    if (pcr->usLevel > (word)priv)
      return j;
    if (pcr->usLevel == (word)priv)
      return i;
  }
  printf("\a\nInvalid priv level `%u' on line %d of CTL file!\n",priv,linenum);
  exit(1);
  return -1;  /*NOTREACHED*/
}

int ClassLevelIndex(word usLevel)
{
  return Deduce_Class((int)usLevel);
}

