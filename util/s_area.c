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
static char rcs_id[]="$Id: s_area.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

#define SILT
#define NOVARS
#define NOINIT
#define NO_MSGH_DEF

#include <ctype.h>
#include <string.h>
#include "prog.h"
#include "bfile.h"
#include "max.h"
#include "silt.h"
#include "newarea.h"
#include "areaapi.h"
#include "areadat.h"
#include "skiplist.h"
#include "s_heap.h"
#include "s_marea.h"

#define HIDDEN  0x000b


typedef struct
{
  dword offset;
  char name[1];
} A2DATA;


void canonslash(char *orig, char *dest, int max)
{
  char buf[PATHLEN];

  strnncpy(buf, orig, min(max, PATHLEN));

  if (strlen(buf) != 3 || buf[1] != ':')
    Strip_Trailing(buf, PATH_DELIM);

  strcpy(dest, *buf ? make_fullfname(buf) : "");
}

int _stdc mystrcmp(void *v1, void *v2)
{
  A2DATA *a1=v1;
  A2DATA *a2=v2;

  return strcmp(a1->name, a2->name);
}


/* Convert the area string to uppercase */

static void near strupr2(char *area)
{
  area[0]=toupper(area[0]);
  area[1]=toupper(area[1]);
}


/* Convert an ACS string to a binary privilege level and key set */

static void near ACSToPriv(char *aname, char *acs, sword *piPriv, dword *pdKeys)
{
  NW(aname);
  linenum=0;        /* so that invalid line number is reported in err msgs */
  *piPriv=max2priv(Deduce_Priv(acs));
  *pdKeys=Deduce_Lock(acs);
}




/* Mask all of the attributes in the given area with the specified value */

static void near AddAttrib(struct _area *pa, int attrib)
{
  int i;

  for (i=0; i < 12; i++)
    pa->attrib[i] |= attrib;
}


/* Write a Max 2.x-compatible area.dat entry for a message area */

void AddMsgArea(BFILE bDat, BFILE bNdx, BFILE bIdx, PMAH pmah, SLIST *sl)
{
  struct _area a;
  struct _aidx aidx;
  struct _102aidx aidx102;
  char *name;
  A2DATA *pa2, *pa2Found;
  int fUpdate=FALSE;
  int i;

  if ((name=strdup(PMAS(pmah, name)))==NULL)
    NoMem();

  /* Munge the name if user requested it */

  if (do_ul2areas)
  {
    char *p;

    /* Replace all '.' with  '_' */

    while ((p=strchr(name,'.')))
      *p='_';
  }
  else if (do_short2areas)
  {
    char *p=strrchr(name,'.');

    /* Remove group(s) from name */

    if (p)
      strocpy(name,p+1);
  }

  /* See if the area is already in our list of areas */

  if ((pa2=malloc(sizeof(A2DATA) + strlen(name)))==NULL)
    NoMem();

  strcpy(pa2->name, name);
  strlwr(pa2->name);

  /* If we found the area already in the data file, just update it */

  if ((pa2Found=SkipSearchList(sl, pa2)) != NULL)
  {
    Bseek(bDat, pa2Found->offset, BSEEK_SET);
    fUpdate=TRUE;

    if (Bread(bDat, &a, sizeof a) != sizeof a)
    {
      printf("Error reading existing file area from offset %" INT32_FORMAT " (area %s)\n",
             pa2Found->offset, name);
      exit(1);
    }

    Bseek(bDat, pa2Found->offset, BSEEK_SET);
  }
  else
  {
    memset(&a, 0, sizeof a);
    a.id=AREA_ID;
    a.struct_len=sizeof a;
    a.type=MSGTYPE_SDM;
    a.msgpriv=a.filepriv=HIDDEN;
    a.origin_aka=-1;
  }

  free(pa2);
  pa2=NULL;


  strncpy((char *)&a.areano, name, 2);
  strnncpy(a.name, name, 39);
  strupr2((char *)&a.areano);
  a.type=pmah->ma.type;

  canonslash(PMAS(pmah, path), a.msgpath, 80);

  if (a.type & MSGTYPE_SDM)
    Add_Trailing(a.msgpath, PATH_DELIM);

  strnncpy(a.msgname, PMAS(pmah, echo_tag), 40);
  strnncpy(a.msginfo, PMAS(pmah, descript), 80);
  canonslash(PMAS(pmah, barricade), a.msgbar, 80);
  strnncpy(a.origin, PMAS(pmah, origin), 62);
  ACSToPriv(name, PMAS(pmah, acs), &a.msgpriv, &a.msglock);
  a.origin_aka=0;

  /* Find a match for the origin address, if possible */

  for (i=0; i < ALIAS_CNT && prm.address[i].zone; i++)
  {
    if (prm.address[i].zone == pmah->ma.primary.zone &&
        prm.address[i].net  == pmah->ma.primary.net &&
        prm.address[i].node == pmah->ma.primary.node &&
        prm.address[i].point== pmah->ma.primary.point)
    {
      a.origin_aka = i;
      break;
    }
  }

  strnncpy(a.msgmenuname, PMAS(pmah, menuname), 13);
  a.killbyage=pmah->ma.killbyage;
  a.killbynum=pmah->ma.killbynum;

  /* Convert the attribute field */

  if (pmah->ma.attribs & MA_READONLY)
    AddAttrib(&a, NOPUBLIC | NOPRIVATE);
  else if ((pmah->ma.attribs & (MA_PVT|MA_PUB))==(MA_PVT|MA_PUB))
    ;
  else if (pmah->ma.attribs & MA_PVT)
    AddAttrib(&a, NOPUBLIC);
  else if (pmah->ma.attribs & MA_PUB)
    AddAttrib(&a, NOPRIVATE);

  if (pmah->ma.attribs & MA_NET)
    AddAttrib(&a, SYSMAIL);

  if (pmah->ma.attribs & MA_ECHO)
    AddAttrib(&a, ECHO);

  if (pmah->ma.attribs & MA_CONF)
    AddAttrib(&a, CONF);

  if (pmah->ma.attribs & MA_ANON)
    AddAttrib(&a, ANON_OK);

  if (pmah->ma.attribs & MA_NORNK)
    AddAttrib(&a, NREALNAME);

  if (pmah->ma.attribs & MA_REAL)
    AddAttrib(&a, UREALNAME);

  if (pmah->ma.attribs & MA_ALIAS)
    AddAttrib(&a, UALIAS);

  /* If this is a division beginning or end, hide it */

  if (pmah->ma.attribs & (MA_DIVBEGIN | MA_DIVBEGIN))
    a.msgpriv=HIDDEN;

  /* Create an AREA.NDX entry */

  memset(&aidx, 0, sizeof aidx);
  aidx.offset=Btell(bDat);
  strncpy(aidx.name, name, sizeof aidx.name);

  /* Create an AREA.IDX entry */

  memset(&aidx102, 0, sizeof aidx102);
  memmove(&aidx102.area, aidx.name, 2);
  strupr2((char *)&aidx102.area);
  aidx102.offset=aidx.offset;

  if (Bwrite(bDat, &a, sizeof a) != sizeof a)
      printf("Error writing to Max 2.x-compatible area data file!\n");

  if (fUpdate)
    Bseek(bDat, 0L, BSEEK_END);
  else if (Bwrite(bNdx, &aidx, sizeof aidx) != sizeof aidx ||
           Bwrite(bIdx, &aidx102, sizeof aidx102) != sizeof aidx102)
  {
    printf("Error writing to Max 2.x-compatible area index files!\n");
    exit(1);
  }

  free(name);
}


/* Write a Max 2.x-compatible area.dat entry for a file area */

void AddFileArea(BFILE bDat, BFILE bNdx, BFILE bIdx, PFAH pfah, SLIST *sl)
{
  struct _area a;
  struct _aidx aidx;
  struct _102aidx aidx102;
  word wExists;
  A2DATA *pa2;
  char *name;

  if ((name=strdup(PFAS(pfah, name)))==NULL)
    NoMem();

  /* Munge the name if user requested it */

  if (do_ul2areas)
  {
    /* Replace all '.' with  '_' */
    char *p;
    while ((p=strchr(name,'.')))
      *p='_';
  }
  else if (do_short2areas)
  {
    /* Remove group(s) from name */
    char *p=strrchr(name,'.');
    if (p)
      strocpy(name,p+1);
  }

  memset(&a, 0, sizeof a);

  a.id=AREA_ID;
  a.struct_len=sizeof a;
  a.type=MSGTYPE_SDM;
  a.msgpriv=a.filepriv=HIDDEN;
  a.origin_aka=-1;

  strncpy((char *)&a.areano, name, 2);
  strnncpy(a.name, name, 39);
  strupr2((char *)&a.areano);

  canonslash(PFAS(pfah, downpath), a.filepath, 80);
  Add_Trailing(a.filepath, PATH_DELIM);

  canonslash(PFAS(pfah, uppath), a.uppath, 80);
  Add_Trailing(a.uppath, PATH_DELIM);

  canonslash(PFAS(pfah, barricade), a.filebar, 80);

  canonslash(PFAS(pfah, filesbbs), a.filesbbs, 80);

  strnncpy(a.fileinfo, PFAS(pfah, descript), 80);
  ACSToPriv(name, PFAS(pfah, acs), &a.filepriv, &a.filelock);
  strnncpy(a.filemenuname, PFAS(pfah, menuname), 13);

  /* If this is a division beginning or end, hide it */

  if (pfah->fa.attribs & (FA_DIVBEGIN | FA_DIVBEGIN))
    a.filepriv=HIDDEN;

  memset(&aidx, 0, sizeof aidx);
  aidx.offset=Btell(bDat);
  strncpy(aidx.name, name, sizeof aidx.name);

  /* Create an AREA.IDX entry */

  memset(&aidx102, 0, sizeof aidx102);
  memmove(&aidx102.area, aidx.name, 2);
  strupr2((char *)&aidx102.area);
  aidx102.offset=aidx.offset;

  if ((pa2=malloc(sizeof(A2DATA) + strlen(name)))==NULL)
    NoMem();

  pa2->offset=aidx.offset;
  strcpy(pa2->name, name);
  strlwr(pa2->name);

  pa2->offset=aidx.offset;

  if (!SkipAddNode(sl, pa2, &wExists))
  {
    printf("Error adding area %s to skip list\n", name);
    exit(1);
  }

  if (Bwrite(bDat, &a, sizeof a) != sizeof a)
  {
    printf("Error writing to Max 2.x-compatible area data file!\n");
    exit(1);
  }

  if (Bwrite(bNdx, &aidx, sizeof aidx) != sizeof aidx ||
      Bwrite(bIdx, &aidx102, sizeof aidx102) != sizeof aidx102)
  {
    printf("Error writing to Max 2.x-compatible area index file!\n");
    exit(1);
  }

  free(name);
}


/* Generate a Max 2.x-compatible area.dat file */

void Generate20Areas(void)
{
  static char szErrOpen[]="\nError opening '%s' for write!\n";
  char *pszDat;
  char *pszIdx;
  char pszNdx[PATHLEN];
  SLIST *sl;
  BFILE bDat, bNdx, bIdx;
  MAH mah={0};
  FAH fah={0};
  HAF haf;
  HAFF haff;
  char *p;

  printf("\nWriting Max 2.x-compatible AREA.DAT/NDX/IDX files...");

  if ((sl=SkipCreateList(16, 8, mystrcmp))==NULL)
  {
    printf("Can't allocate memory for skip list!\n");
    exit(1);
  }

  pszDat = strings + prm.adat_name;
  pszIdx = strings + prm.aidx_name;
  strcpy(pszNdx, pszIdx);

  if ((p=stristr(pszNdx, ".idx"))==NULL)
    strcat(pszNdx, ".ndx");
  else
    strcpy(p, ".ndx");

  /* Open the output AREA.* files */

  if ((bDat=Bopen(pszDat, BO_CREAT | BO_TRUNC | BO_RDWR | BO_BINARY,
                  BSH_DENYNO, 4096))==NULL)
  {
    printf(szErrOpen, pszDat);
    exit(1);
  }

  if ((bNdx=Bopen(pszNdx, BO_CREAT | BO_TRUNC | BO_RDWR | BO_BINARY,
                  BSH_DENYNO, 16384))==NULL)
  {
    printf(szErrOpen, pszNdx);
    exit(1);
  }

  if ((bIdx=Bopen(pszIdx, BO_CREAT | BO_TRUNC | BO_RDWR | BO_BINARY,
                  BSH_DENYNO, 1024))==NULL)
  {
    printf(szErrOpen, pszIdx);
    exit(1);
  }


  /* Open the file areas and add one-by-one */

  if ((haf=AreaFileOpen(strings + prm.farea_name, FALSE))==NULL)
  {
    printf("Error opening file data file %s for read!\n",
           strings + prm.marea_name);
    exit(1);
  }

  if ((haff=AreaFileFindOpen(haf, NULL, AFFO_DIV)) != NULL)
  {
    while (AreaFileFindNext(haff, &fah, FALSE)==0)
      AddFileArea(bDat, bNdx, bIdx, &fah, sl);

    AreaFileFindClose(haff);
  }

  AreaFileClose(haf);
  DisposeFah(&fah);



  /* Open the message areas and add one-by-one */

  if ((haf=AreaFileOpen(strings + prm.marea_name, TRUE))==NULL)
  {
    printf("Error opening message data file %s for read!\n",
           strings + prm.marea_name);
    exit(1);
  }

  if ((haff=AreaFileFindOpen(haf, NULL, AFFO_DIV)) != NULL)
  {
    while (AreaFileFindNext(haff, &mah, FALSE)==0)
      AddMsgArea(bDat, bNdx, bIdx, &mah, sl);

    AreaFileFindClose(haff);
  }

  AreaFileClose(haf);
  DisposeMah(&mah);


  Bclose(bIdx);
  Bclose(bNdx);
  Bclose(bDat);

  SkipDestroyList(sl);
}

