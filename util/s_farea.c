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
static char rcs_id[]="$Id: s_farea.c,v 1.1.1.1 2002/10/01 17:57:41 sdudley Exp $";
#pragma on(unreferenced)

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
#include "s_farea.h"

static unsigned cbLast=0;
static word division=0;
static char prefix[PATHLEN]="";


static void near FiltBarricade(void *v, char *words[], char *line)
{
  NW(line);

  if (! *words[1] || !*words[2])
  {
    printf("\n\aError!  Two args needed for Barricade keyword on line %d!\n", linenum);
    Compiling(-1,NULL,NULL);
  }

  HeapAdd(&PFI(v)->h, &PFI(v)->fa.barricademenu, words[1]);
  HeapAdd(&PFI(v)->h, &PFI(v)->fa.barricade, words[2]);
}

static void near FileAreaWrite(FAINFO *pfi, int closeit)
{
  static int fa_fd=-1;
  static int fai_fd=-1;
  unsigned size;
  OVRLIST ol;
  struct _mfidx mfi;

  if (!do_farea)
    return;

  if (!pfi && fa_fd != -1 && closeit)
  {
    FAREA fa;
    long pos=lseek(fa_fd, 0L, SEEK_END);

    /* Find out the position of the last area in the file, so that we       *
     * can fix the wraparound pointer in the first record to point          *
     * to it.                                                               */

    pos -= cbLast + ADATA_START;

    lseek(fa_fd, ADATA_START, SEEK_SET);

    if (read(fa_fd, (char *)&fa, sizeof fa) != sizeof fa)
    {
      printf("Error reading final file area data!\n");
      exit(1);
    }

    lseek(fa_fd, ADATA_START, SEEK_SET);

    fa.cbPrior=-pos;

    if (write(fa_fd, (char *)&fa, sizeof fa) != sizeof fa)
    {
      printf("Error writing final file area data!\n");
      exit(1);
    }

    close(fa_fd);
    close(fai_fd);
    fa_fd=-1;
    return;
  }

  if (fa_fd==-1 && !closeit)
  {
    char fname[PATHLEN];
    dword dwId=FAREA_ID;

    if (strings[prm.farea_name]==0)
    {
      printf("Error!  FILEAREA.CTL cannot be SILTed separately.  Use\n"
             "\"SILT MAX\" and ensure that you have a \"Files farea\"\n"
             "keyword in the Session Section.\n");
      exit(1);
    }

    strcpy(fname, strings + prm.farea_name);
    strcat(fname, ".dat");

    if ((fa_fd=sopen(fname, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                     SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    {
      printf("Can't open file area data file %s!\n", fname);
      exit(1);
    }

    strcpy(fname, strings + prm.farea_name);
    strcat(fname, ".idx");

    if (write(fa_fd, (char *)&dwId, sizeof dwId) != sizeof dwId)
    {
      printf("\aError writing key to file data file %s\n", fname);
      exit(1);
    }

    if ((fai_fd=sopen(fname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                      SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    {
      printf("Can't open file area index file %s!\n", fname);
      exit(1);
    }
  }

  if (pfi)
  {
    /* Fill out the structure length information */

    pfi->fa.cbArea=sizeof(pfi->fa);
    pfi->fa.cbHeap=pfi->h.end - pfi->h.heap;
    pfi->fa.cbPrior=cbLast;

    cbLast=pfi->fa.cbArea + pfi->fa.cbHeap +
           pfi->fa.num_override * sizeof(OVERRIDE);

    /* Make sure that directory exists, unless "slow" flag is set */

    if ((pfi->fa.attribs & (FA_DIVBEGIN|FA_DIVEND))==0)
      if (pfi->fa.downpath && (pfi->fa.attribs & FA_SLOW)==0)
        assert_dir(pfi->h.heap + pfi->fa.downpath);

    strnncpy(mfi.name, pfi->h.heap + pfi->fa.name, sizeof(mfi.name));
    mfi.name_hash=SquishHash(pfi->h.heap + pfi->fa.name);
    mfi.ofs=tell(fa_fd);

    if (write(fai_fd, (char *)&mfi, sizeof mfi) != sizeof mfi)
      ErrWrite();

    /* Write the main area data structure */

    if (write(fa_fd, (char *)&pfi->fa, sizeof pfi->fa) != sizeof pfi->fa)
      ErrWrite();

    /* Write all of the message area overrides */

    for (ol=pfi->ol; ol; ol=ol->next)
      if (write(fa_fd, (char *)&ol->or, sizeof ol->or) != sizeof ol->or)
        ErrWrite();

    /* Figure out the size of the zstr heap and write it, too */

    size=pfi->h.end-pfi->h.heap;

    if (write(fa_fd, (char *)pfi->h.heap, size) != (signed)size)
      ErrWrite();
  }
}




/* File area type information */

static void near FiltType(void *v, char *words[], char *line)
{
  int w;  /* Word number */

  static struct
  {
    char *name;
    word mask;
  } *pst, style_tab[]=
    {
      {"Slow",      FA_SLOW},
      {"Staged",    FA_STAGED},
      {"NoNew",     FA_NONEW},
      {"CD",        FA_CDROM},
      {"Hidden",    FA_HIDDN},
      {"DateAuto",  FA_AUTODATE},
      {"DateManual",FA_MANDATE},
      {"DateList",  FA_LISTDATE},
      {"FreeTime",  FA_FREETIME},
      {"FreeSize",  FA_FREESIZE},
      {"FreeBytes", FA_FREESIZE},
      {"Free",      FA_FREEALL},
      {"NoIndex",   FA_NOINDEX},
      {NULL, 0}
    };

  NW(line);

  /* Parse all of the known keywords */

  for (w=1; w < MAX_PARSE_WORDS && *words[w]; w++)
  {
    for (pst=style_tab; pst->name; pst++)
      if (eqstri(words[w], pst->name))
      {
        PFI(v)->fa.attribs |= pst->mask;
        break;
      }

    if (!pst->name)
      Unknown_Ctl(linenum, words[w]);
  }
}





void FileAreaClose(void)
{
  FileAreaWrite(NULL, TRUE);
}




int ParseFileArea(FILE *ctlfile, char *name)
{
  char line[PATHLEN];
  char fullname[PATHLEN];
  static FAINFO fi;
  OVRLIST ol;

  struct _vbtab verbs[]=
  {
    {0,           "acs",          &fi.fa.acs},
    {FiltPath,    "download",     &fi.fa.downpath},
    {FiltPath,    "upload",       &fi.fa.uppath},
    {FiltBarricade,"barricade",   NULL},
    {0,           "filelist",     &fi.fa.filesbbs},
    {0,           "desc",         &fi.fa.descript},
    {0,           "description",  &fi.fa.descript},
    {FiltType,    "type",         NULL},
    {FiltMenuname,"menuname",     NULL},
    {FiltOverride,"override",     NULL},
    {0,           "app",          NULL},
    {0,           "application",  NULL},
    {0,           NULL,           NULL}
  };


  if (strchr(name, '.'))
    BadDivisionName();

  /* Make sure that area file is open */

  FileAreaWrite(NULL, FALSE);

  if (do_farea)
  {
    memset(&fi, 0, sizeof fi);
    fi.marea=FALSE;
    fi.fa.division=division;

    HeapNew(&fi.h, MAX_FILE_HEAP);

    strcpy(fullname, prefix);
    strcat(fullname, name);

    HeapAdd(&fi.h, &fi.fa.name, fullname);
  }


  while (fgets(line, PATHLEN, ctlfile))
    if (VerbParse(&fi, do_farea ? verbs : NULL, line))
      break;

  if (do_farea)
  {
    FileAreaWrite(&fi, FALSE);

    for (ol=fi.ol; ol; ol=ol->next)
      free(ol);

    HeapDelete(&fi.h);
  }

  return 0;
}


void ParseFileDivisionBegin(char *name, char *acs, char *displayfile, char *descript)
{
  FAINFO fi;
  char fullname[PATHLEN];

  if (!do_farea)
    return;

  if (strchr(name, '.'))
    BadDivisionName();

  strcpy(fullname, prefix);
  strcat(fullname, name);

  strcat(prefix, name);
  strcat(prefix, ".");

  FileAreaWrite(NULL, FALSE);

  memset(&fi, 0, sizeof fi);
  fi.marea=FALSE;

  HeapNew(&fi.h, MAX_MSG_HEAP);

  HeapAdd(&fi.h, &fi.fa.name, fullname);
  HeapAdd(&fi.h, &fi.fa.descript, descript);
  HeapAdd(&fi.h, &fi.fa.acs, acs);
  HeapAdd(&fi.h, &fi.fa.filesbbs, displayfile);

  fi.fa.attribs = FA_DIVBEGIN;
  fi.fa.division = division++;

  FileAreaWrite(&fi, FALSE);
  HeapDelete(&fi.h);
}

void ParseFileDivisionEnd(void)
{
  FAINFO fi;
  char *p;


  if (!do_farea)
    return;

  if (! *prefix)
  {
    printf("\n\aError!  FileDivisionEnd on line %d has no correspondig FileDivisionBegin!\n", linenum);
    Compiling(-1, NULL, NULL);
    return;
  }

  /* Strip off the trailing dot */

  prefix[strlen(prefix)-1]=0;

  /* Now set the prefix back to the prior area */

  if ((p=strrchr(prefix, '.')) != NULL)
    p[1]=0;
  else *prefix=0;

  FileAreaWrite(NULL, FALSE);

  memset(&fi, 0, sizeof fi);
  fi.marea=FALSE;

  HeapNew(&fi.h, MAX_MSG_HEAP);

  fi.fa.attribs = FA_DIVEND;
  fi.fa.division = --division;
  HeapAdd(&fi.h, &fi.fa.acs, "");

  FileAreaWrite(&fi, FALSE);
  HeapDelete(&fi.h);
}


