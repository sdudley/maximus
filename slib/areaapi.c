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

#include <stdlib.h>
#include <string.h>
#include "uni.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max.h"
#include "bfile.h"
#include "areaapi.h"

#define MF_BUFFER   32    /* Read 32 areas at a time */

/* Open an area file for processing */

HAF _fast AreaFileOpen(char *name, int msg_area)
{
  char fname[PATHLEN];
  HAF haf=malloc(sizeof *haf);
  dword dwId;

  if (!haf)
    return NULL;

  /* msg or file area file? */

  haf->msg_area=msg_area;

  /* Add the extension for the .dat file */

  strcpy(fname, name);
  strcat(fname, ".dat");

  if ((haf->dat=Bopen(fname, BO_RDONLY|BO_BINARY, BSH_DENYNO, sizeof(MAH)*4))==NULL)
  {
    free(haf);
    return NULL;
  }

  /* Read the area identifier and make sure that it matches the
   * specified type.
   */

  if (Bread(haf->dat, (char *)&dwId, sizeof dwId) != sizeof dwId ||
      (msg_area  && dwId != MAREA_ID) ||
      (!msg_area && dwId != FAREA_ID))
  {
    Bclose(haf->dat);
    free(haf);
    return NULL;
  }

  /* Add the extension for the .idx file */

  strcpy(fname, name);
  strcat(fname, ".idx");

  haf->idxname=strdup(fname);

  /* Don't open the index yet - we may not need it */

  haf->idx=NULL;

  return haf;
}


/* Change the search parameters in the middle of the search */

void _fast AreaFileFindChange(HAFF haff, char *newname, int newflags)
{
  if (haff)
  {
    haff->name=newname;
    haff->flags=newflags;
  }
}

/* Start a search on the message areas */

HAFF _fast AreaFileFindOpen(HAF haf, char *name, int flags)
{
  HAFF haff;

  if ((haff=malloc(sizeof *haff))==NULL)
    return NULL;

  haff->haf=haf;
  haff->name=name;
  haff->ofs=ADATA_START;
  haff->wrapped=FALSE;
  haff->start_ofs=ADATA_START;
  haff->flags=flags;
  return haff;
}

/* Reset a search and start again from the beginning */

void _fast AreaFileFindReset(HAFF haff)
{
  haff->ofs=ADATA_START;
  haff->wrapped=FALSE;
  haff->start_ofs=ADATA_START;
}


int _fast ReadFileArea(HAF haf, char *name, FAH *pfah)
{
  HAFF haff=AreaFileFindOpen(haf, name, 0);
  int rc;

  if (!haff)
    return FALSE;

  rc=AreaFileFindNext(haff, pfah, FALSE)==0;
  AreaFileFindClose(haff);

  return rc;
}


int _fast ReadMsgArea(HAF haf, char *name, MAH *pmah)
{
  HAFF haff=AreaFileFindOpen(haf, name, 0);
  int rc;

  if (!haff)
    return FALSE;

  rc=AreaFileFindNext(haff, pmah, FALSE)==0;
  AreaFileFindClose(haff);

  return rc;
}

/* Read in the area data structure from an area file */

static int near _ReadArea(HAFF haff, int wrap)
{
  haff->start_ofs=haff->ofs;
  Bseek(haff->haf->dat, haff->ofs, BSEEK_SET);

  if (haff->haf->msg_area)
  {
    if (Bread(haff->haf->dat, (char *)&haff->ah.mah.ma, sizeof(MAREA)) != sizeof(MAREA))
    {
      if (!wrap || haff->wrapped)
        return -1;

      haff->wrapped=TRUE;

      Bseek(haff->haf->dat, haff->ofs=haff->start_ofs=ADATA_START, BSEEK_SET);

      if (Bread(haff->haf->dat, (char *)&haff->ah.mah.ma, sizeof(MAREA)) != sizeof(MAREA))
        return -1;
    }

    /* If the area file struct has grown, compensate for it */

    if (haff->ah.mah.ma.cbArea > sizeof(MAREA))
      Bseek(haff->haf->dat, haff->ah.mah.ma.cbArea - sizeof(MAREA), BSEEK_CUR);

    /* Increment our offset counter */

    haff->ofs += (haff->ah.mah.ma.cbArea > 0)
                    ? haff->ah.mah.ma.cbArea : sizeof(MAREA);

    /* Record the size of the heap and the number of overrides */

    haff->heap_size=haff->ah.mah.ma.cbHeap;
    haff->num_override=haff->ah.mah.ma.num_override;
  }
  else  /* repeat the same deal for file areas */
  {
    if (Bread(haff->haf->dat, (char *)&haff->ah.fah.fa, sizeof(FAREA)) != sizeof(FAREA))
    {
      if (!wrap || haff->wrapped)
        return -1;

      haff->wrapped=TRUE;

      Bseek(haff->haf->dat, haff->ofs=haff->start_ofs=ADATA_START, BSEEK_SET);

      if (Bread(haff->haf->dat, (char *)&haff->ah.fah.fa, sizeof(FAREA)) != sizeof(FAREA))
        return -1;
    }

    /* If the area file struct has grown, compensate for it */

    if (haff->ah.fah.fa.cbArea > sizeof(FAREA))
      Bseek(haff->haf->dat, haff->ah.fah.fa.cbArea - sizeof(FAREA), BSEEK_CUR);

    /* Increment our offset counter */

    haff->ofs += (haff->ah.fah.fa.cbArea > 0)
                    ? haff->ah.fah.fa.cbArea : sizeof(FAREA);

    /* Record the size of the heap and the number of overrides */

    haff->heap_size=haff->ah.fah.fa.cbHeap;
    haff->num_override=haff->ah.fah.fa.num_override;
  }

  return 0;
}


/* Read the overrides into memory */

static int near _ReadOverrides(HAFF haff)
{
  unsigned size;
  OVERRIDE *pov;


  /* Size to read all at once */

  size=sizeof(OVERRIDE) * haff->num_override;


  /* If no overrides, skip this part */

  if (!size)
    return 0;


  /* If not enough memory to allocate override list */

  if ((pov=malloc(size))==NULL)
    return -1;


  /* Real all of the info */

  if (Bread(haff->haf->dat, (char *)pov, size) != (signed)size)
    return -1;


  /* Increment our position counter */

  haff->ofs += size;


  /* Assign the pointer to the overrides for the caller's use */

  if (haff->haf->msg_area)
    haff->ah.mah.pov=pov;
  else haff->ah.fah.pov=pov;

  return 0;
}



/* Read the heap for this area into memory */

static int near _ReadHeap(HAFF haff)
{
  char *heap;

  /* We must ALWAYS allocate PATHLEN bytes more than necessary so that      *
   * we can add a temporary upload/download file to the current             *
   * file area.                                                             */

  if ((heap=malloc(haff->heap_size + PATHLEN*2))==NULL)
    return -1;

  if (haff->heap_size)
    if (Bread(haff->haf->dat, (char *)heap, haff->heap_size) !=
                                                      (signed)haff->heap_size)
      return -1;

  haff->ofs += haff->heap_size;

  if (haff->haf->msg_area)
    haff->ah.mah.heap=heap;
  else
    haff->ah.fah.heap=heap;

  return 0;
}


/* Returns TRUE if the specified area name in haff->name matches the        *
 * name of the msg/file area that we just read in.                          */

static int near _AreaMatch(HAFF haff)
{
  char *name=haff->haf->msg_area ? MAS(haff->ah.mah, name)
                                 : FAS(haff->ah.fah, name);

  if (haff->haf->msg_area &&
      (haff->ah.mah.ma.attribs & (MA_DIVBEGIN|MA_DIVEND)) &&
      (haff->flags & AFFO_DIV)==0)
    return FALSE;

  if (!haff->haf->msg_area &&
      (haff->ah.fah.fa.attribs & (FA_DIVBEGIN|FA_DIVEND)) &&
      (haff->flags & AFFO_DIV)==0)
    return FALSE;

  /* If there was no search name, return TRUE for all areas */

  if (!haff->name || !*haff->name)
    return TRUE;

  /* If the first char is a space (substring match only)... */

  return eqstri(haff->name, name);
}

static int _fast openIndex(HAF haf)
{
  if (haf->idx==NULL &&
      (haf->idx=Bopen(haf->idxname,BO_RDONLY | BO_BINARY, BSH_DENYNO,
                      sizeof(struct _mfidx)*MF_BUFFER))==NULL)
    return FALSE;

  return TRUE;
}


/* Seek to a specified area number in the area file */

int _fast AreaFileFindSeek(HAFF haff, void *v, unsigned uiNum)
{
  struct _mfidx mfi;

  if (!openIndex(haff->haf))
    return -1;

  Bseek(haff->haf->idx, uiNum * (long)sizeof mfi, BSEEK_SET);

  if (Bread(haff->haf->idx, (char *)&mfi, sizeof mfi) != sizeof mfi)
    return -1;

  haff->ofs=mfi.ofs;

  return AreaFileFindNext(haff, v, FALSE);
}


/* We have a specific area name, so try to find it in the index */

static void near _TryIndexSearch(HAFF haff)
{
  MFIDX mfi;
  dword hash;

  if (!openIndex(haff->haf))
    return;

  hash=SquishHash(haff->name);

  /* Scan index records */

  Bseek(haff->haf->idx, 0L, BSEEK_SET);
  while (Bread(haff->haf->idx, &mfi, sizeof(mfi)) != sizeof(mfi))
  {
    /* Check the index record for a match */

    if (mfi.name_hash==hash && strnicmp(mfi.name, haff->name, sizeof(mfi.name)-1)==0)
    {
      haff->ofs=mfi.ofs;
      break;
    }
  }
}



/* Find the next area specified by the handle-area-file-find struct */

static int near _AreaFileFind(HAFF haff, void *v, unsigned wrap, int forward)
{
  if (haff->haf->msg_area)
  {
    memset(&haff->ah.mah, 0, sizeof haff->ah.mah);
    haff->ah.mah.ma.cbPrior=((MAH *)v)->ma.cbPrior;
    DisposeMah(v);
  }
  else
  {
    memset(&haff->ah.fah, 0, sizeof haff->ah.fah);
    haff->ah.fah.fa.cbPrior=((FAH *)v)->fa.cbPrior;
    DisposeFah(v);
  }

  /* Try to do an index search to speed things up */

  if (forward && haff->name && haff->ofs==ADATA_START)
    _TryIndexSearch(haff);

  do
  {
    /* If we're doing a backwards seek... */

    if (!forward)
    {
      if (haff->start_ofs==ADATA_START)
      {
        /* Don't wrap more than once */

        if (!wrap || haff->wrapped)
          return -1;

        haff->wrapped=TRUE;
      }

      /* Note that cbPrior will be the negative offset of the last              *
       * area, for the very first area in the area file.                        */

      if (haff->haf->msg_area)
        haff->ofs = haff->start_ofs - haff->ah.mah.ma.cbPrior;
      else
        haff->ofs = haff->start_ofs - haff->ah.fah.fa.cbPrior;
    }

    if (haff->haf->msg_area)
      DisposeMah(&haff->ah.mah);
    else DisposeFah(&haff->ah.fah);

    if (_ReadArea(haff, wrap) != 0 ||
        _ReadOverrides(haff) != 0 ||
        _ReadHeap(haff) != 0)
    {
      return -1;
    }
  }
  while (!_AreaMatch(haff));

  /* If we found the area, copy this data to the caller */

  if (haff->haf->msg_area)
  {
    *(MAH *)v=haff->ah.mah;
    ((MAH *)v)->heap_size=haff->heap_size;
    memset(&haff->ah.mah, 0, sizeof haff->ah.mah);
  }
  else
  {
    *(FAH *)v=haff->ah.fah;
    ((FAH *)v)->heap_size=haff->heap_size;
    memset(&haff->ah.fah, 0, sizeof haff->ah.fah);
  }

  return 0;
}



/* Seek to the prior area in the area file */

int _fast AreaFileFindPrior(HAFF haff, void *v, unsigned wrap)
{
  return _AreaFileFind(haff, v, wrap, FALSE);
}


int _fast AreaFileFindNext(HAFF haff, void *v, unsigned wrap)
{
  return _AreaFileFind(haff, v, wrap, TRUE);
}



/* End an area-find */

int _fast AreaFileFindClose(HAFF haff)
{
  if (haff)
    free(haff);

  return 0;
}


/* Close the area file */

int _fast AreaFileClose(HAF haf)
{
  if (haf)
  {
    if (haf->idx != NULL)
      Bclose(haf->idx);

    if (haf->dat != NULL)
      Bclose(haf->dat);

    if (haf->idxname)
      free(haf->idxname);

    haf->idxname=NULL;
    haf->idx=NULL;
    haf->dat=NULL;

    free(haf);
  }

  return 0;
}


/* Free memory used by a MAH structure */

void _fast DisposeMah(MAH *pmah)
{
  if (pmah->heap)
    free(pmah->heap);

  if (pmah->pov)
    free(pmah->pov);

  memset(pmah, 0, sizeof *pmah);
}


/* Free memorry used by a FAH structure */

void _fast DisposeFah(FAH *pfah)
{
  if (pfah->heap)
    free(pfah->heap);

  if (pfah->pov)
    free(pfah->pov);

  memset(pfah, 0, sizeof *pfah);
}


#ifdef TEST_HARNESS
main()
{
  MAH mah={0};
  HAF haf=AreaFileOpen("marea.dat", "marea.idx", TRUE);
  HAFF haff;

  if (!haf)
  {
    printf("Can't open marea.dat\n");
    return 1;
  }

  if ((haff=AreaFileFindOpen(haf, NULL, 0))==NULL)
  {
    printf("Can't search area file!\n");
    return 1;
  }

  while (AreaFileFindNext(haff, &mah, FALSE)==0)
    printf("Found area %s (%s)\n", MAS(mah, name), MAS(mah, descript));

  AreaFileFindClose(haff);

  printf("---\n");

  if ((haff=AreaFileFindOpen(haf, "muf", 0))==NULL)
  {
    printf("Can't search area file 2!\n");
    return 1;
  }

  if (AreaFileFindNext(haff, &mah, FALSE) != 0)
  {
    printf("can't find muf 2!\n");
    return 1;
  }

  printf("Found area %s (%s)\n", MAS(mah, name), MAS(mah, descript));

  AreaFileFindChange(haff, NULL, 0);

  while (AreaFileFindNext(haff, &mah, TRUE)==0)
    printf("Found area %s (%s)\n", MAS(mah, name), MAS(mah, descript));

  AreaFileFindClose(haff);

  printf("---\n");

  if ((haff=AreaFileFindOpen(haf, "muf", 0))==NULL)
  {
    printf("Can't search area file 2!\n");
    return 1;
  }

  if (AreaFileFindNext(haff, &mah, FALSE) != 0)
  {
    printf("can't find muf 2!\n");
    return 1;
  }

  printf("Found area %s (%s)\n", MAS(mah, name), MAS(mah, descript));

  AreaFileFindChange(haff, NULL, 0);

  while (AreaFileFindPrior(haff, &mah, TRUE)==0)
    printf("Found area %s (%s)\n", MAS(mah, name), MAS(mah, descript));

  AreaFileFindClose(haff);
  AreaFileClose(haf);


  return 0;
}
#endif

int _fast CopyMsgArea(PMAH to, PMAH from)
{
  DisposeMah(to);

  if (!from->heap)
    return TRUE;

  *to=*from;

  if ((to->heap=malloc(from->ma.cbHeap + PATHLEN*2))==NULL)
    return FALSE;

  to->pov=NULL;

  if (from->ma.num_override &&
      (to->pov=malloc(from->ma.num_override * sizeof(OVERRIDE)))==NULL)
  {
    free(to->heap);
    to->heap=NULL;
    return FALSE;
  }

  /* Now move information from the first heap to the second */

  memmove(to->heap, from->heap, from->ma.cbHeap + PATHLEN*2);

  if (to->pov)
    memmove(to->pov, from->pov, from->ma.num_override * sizeof(OVERRIDE));

  return TRUE;
}

int _fast CopyFileArea(PFAH to, PFAH from)
{
  DisposeFah(to);

  if (!from->heap)
    return TRUE;

  *to=*from;

  if ((to->heap=malloc(from->fa.cbHeap + PATHLEN*2))==NULL)
    return FALSE;

  to->pov=NULL;

  if (from->fa.num_override &&
      (to->pov=malloc(from->fa.num_override * sizeof(OVERRIDE)))==NULL)
  {
    free(to->heap);
    to->heap=NULL;
    return FALSE;
  }

  /* Now move information from the first heap to the second */

  memmove(to->heap, from->heap, from->fa.cbHeap + PATHLEN*2);

  if (to->pov)
    memmove(to->pov, from->pov, from->fa.num_override * sizeof(OVERRIDE));

  return TRUE;
}



