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
static char rcs_id[]="$Id: sq_idx.c,v 1.1.1.1 2002/10/01 17:54:31 sdudley Exp $";
#pragma on(unreferenced)

#define MSGAPI_HANDLERS
#define MSGAPI_NO_OLD_TYPES

#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <assert.h>
#include <limits.h>
#include "prog.h"
#include "msgapi.h"
#include "api_sq.h"
#include "apidebug.h"

#define HixSqd            ((struct _sqdata *)(hix)->ha->apidata)


#ifdef __FLAT__
  #define MORE_SPACE       256      /* Allow for up to 256 additions */
  #define SEGMENT_SIZE    (LONG_MAX/(long)sizeof(SQIDX))
  #define SHIFT_SIZE      32768
#else
  #define MORE_SPACE        16      /* Allow for up to 16 additions */
  #define SEGMENT_SIZE    (32767L/(long)sizeof(SQIDX))
  #define SHIFT_SIZE      8192
#endif


#if defined(__FARDATA__) || defined(__FLAT__)
  #define fmemmove memmove
#else
  #define fmemmove f_memmove
#endif


/* Open the index file and read the index for this area */

HIDX _SquishOpenIndex(HAREA ha)
{
  HIDX hix;

  if ((hix=palloc(sizeof(*hix)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return NULL;
  }

  /* Store the current area handle */

  hix->id=ID_HIDX;
  hix->ha=ha;
  hix->lDeltaLo=-1;
  hix->lDeltaHi=-1;
  hix->cSeg=0;
  hix->fBuffer=0;
  hix->fHadExclusive = FALSE;

  return hix;
}


/* This function returns the size of the virtual index */

dword _SquishIndexSize(HIDX hix)
{
  dword lSize;
  int i;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
    lSize=(dword)lseek(HixSqd->ifd, 0L, SEEK_END);
  else
  {
    for (i=0, lSize=0; i < hix->cSeg; i++)
      lSize += hix->pss[i].dwUsed * (dword)sizeof(SQIDX);
  }

  return lSize;
}



/* Start buffering reads/writes to the index file */

int _SquishBeginBuffer(HIDX hix)
{
  dword dwMsgs;
  int i;

  assert(hix->id==ID_HIDX);

  /* Multiple buffers are ok, but we only need to do it once */

  if (hix->fBuffer++ != 0)
    return TRUE;

  hix->cSeg=(int)(hix->ha->num_msg / SEGMENT_SIZE) + 1;

  /* Allocate memory for the array of segments */

  if ((hix->pss=palloc(sizeof(SQIDXSEG) * (unsigned)hix->cSeg))==NULL)
  {
    msgapierr=MERR_NOMEM;
    hix->fBuffer=0;
    return FALSE;
  }

  dwMsgs=hix->ha->num_msg;                /* Read all messages into memory */

  /* Find out how many records are in the file */

  if ((hix->lAllocatedRecords=lseek(HixSqd->ifd, 0L, SEEK_END)) < 0)
  {
    msgapierr=MERR_BADF;
    hix->fBuffer=0;
    return FALSE;
  }

  /* Find out the number of records, not the number of bytes */

  hix->lAllocatedRecords /= sizeof(SQIDX);
  hix->fHadExclusive = HixSqd->fHaveExclusive;

  /* Read from head of index file */

  (void)lseek(HixSqd->ifd, 0L, SEEK_SET); 

  /* Repeat for each segment in the index file */

  for (i=0; i < hix->cSeg; i++)
  {
    dword dwSize=min(dwMsgs+MORE_SPACE, (long)SEGMENT_SIZE);
    unsigned uiSize;

    /* Try to allocate memory for this segment */

    if ((hix->pss[i].psqi=farpalloc((size_t)dwSize * (size_t)sizeof(SQIDX)))==NULL)
    {
      while (i--)
        farpfree(hix->pss[i].psqi);

      pfree(hix->pss);

      msgapierr=MERR_NOMEM;
      hix->fBuffer=0;
      return FALSE;
    }

    hix->pss[i].dwMax=dwSize;

    /* Now read in the messages for this segment */

    dwSize=min(dwMsgs, SEGMENT_SIZE);

    uiSize=(unsigned)dwSize * (unsigned)sizeof(SQIDX);

    if (farread(HixSqd->ifd, (char far *)hix->pss[i].psqi, uiSize) != (int)uiSize)
    {
      do
        farpfree(hix->pss[i].psqi);
      while (i--);

      pfree(hix->pss);

      msgapierr=MERR_BADF;
      hix->fBuffer=0;
      return FALSE;
    }

    /* Decrement the count for msgs in the next segment, if necessary */

    if (dwSize != SEGMENT_SIZE)
      dwMsgs=0;
    else
      dwMsgs -= SEGMENT_SIZE;


    hix->pss[i].dwUsed=dwSize;
  }

  /* Now we have the whole file in memory */

  return TRUE;
}


/* Return a pointer to the 'dwMsg'th message in the buffered index */

static SQIDX far *sidx(HIDX hix, dword dwMsg)
{
  dword dwStart=1L;
  int i;

  for (i=0; i < hix->cSeg; i++)
  {
    if (dwMsg >= dwStart && dwMsg < dwStart + hix->pss[i].dwUsed)
      return hix->pss[i].psqi + (size_t)(dwMsg - dwStart);

    dwStart += hix->pss[i].dwUsed;
  }

  return NULL;
}


/* Get an index from the virtual index file */

int SidxGet(HIDX hix, dword dwMsg, SQIDX *psqi)
{
  SQIDX far *psqiFound;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
  {
    (void)lseek(HixSqd->ifd, (long)(dwMsg-1) * (long)sizeof(SQIDX), SEEK_SET);

    if (farread(HixSqd->ifd, (char far *)psqi, sizeof(SQIDX)) != (int)sizeof(SQIDX))
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

    return TRUE;
  }

  psqiFound=sidx(hix, dwMsg);

  if (!psqiFound)
    return FALSE;

  *psqi=*psqiFound;
  return TRUE;
}




/* Add a new index record to the end of the array */

static int near _SquishAppendIndexRecord(HIDX hix, SQIDX *psqi)
{
  SQIDXSEG *pss;


  /* If we need to expand the index file on disk, do so now */

  if ((long)hix->ha->num_msg > hix->lAllocatedRecords)
  {
    long lSize;
    SQIDX sqi;

    /* Make room for up to 64 new records */

    hix->lAllocatedRecords=hix->ha->num_msg+64;
    lSize=(hix->lAllocatedRecords-1) * (long)sizeof(SQIDX);

    sqi.ofs=0L;
    sqi.umsgid=(UMSGID)-1L;
    sqi.hash=(UMSGID)-1L;

    /* Write a blank index entry at the appropriate location to fill        *
     * up the file.                                                         */

    if (lseek(HixSqd->ifd, lSize, SEEK_SET) != lSize ||
        write(HixSqd->ifd, (char *)&sqi, sizeof sqi) != sizeof(sqi))
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }
  }

  /* If we already have some segments... */

  if (hix->cSeg)
  {
    /* Add to an existing segment */

    pss=hix->pss + hix->cSeg-1;

    /* If the record fits within this segment, just append it. */

    if (pss->dwUsed < pss->dwMax)
    {
      pss->psqi[(size_t)pss->dwUsed]=*psqi;
      pss->dwUsed++;
      return TRUE;
    }

    /* If we can expand this segment by reallocating memory... */

    if (pss->dwMax < SEGMENT_SIZE)
    {
      SQIDX far *psqiNew;

      assert(pss->dwMax >= pss->dwUsed);

      /* Don't use realloc because we cannot afford to lose the info that we  *
       * already have!                                                        */

      if ((psqiNew=farpalloc(((size_t)pss->dwMax + MORE_SPACE) * sizeof(SQIDX)))==NULL)
      {
        msgapierr=MERR_NOMEM;
        return FALSE;
      }

      (void) fmemmove(psqiNew,
                      pss->psqi,
                      (size_t)pss->dwUsed * (size_t)sizeof(SQIDX));

      psqiNew[(size_t)pss->dwUsed]=*psqi;

      pss->dwUsed++;
      pss->dwMax += MORE_SPACE;

      farpfree(pss->psqi);
      pss->psqi=psqiNew;
      return TRUE;
    }
  }


  /* If we arrived here, we either have no segments, or all of our          *
   * existing segments are full.  To handle this, we need to reallocate     *
   * the array of pointers to segments and add a new one.                   */

  if ((pss=palloc(sizeof(SQIDXSEG) * (size_t)(hix->cSeg+1)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  (void)memmove(pss, hix->pss, (size_t)hix->cSeg * sizeof(SQIDXSEG));
  hix->pss=pss;

  /* Allocate memory for the new segment */

  if ((hix->pss[hix->cSeg].psqi=farpalloc(MORE_SPACE * sizeof(SQIDX)))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  pss=hix->pss + hix->cSeg;

  /* Add the specified record to our indices */

  pss->dwUsed=1;
  pss->dwMax=MORE_SPACE;
  *pss->psqi=*psqi;

  /* Increment the segment count */

  hix->cSeg++;
  return TRUE;
}


/* Store an index entry in the virtual index file */

int SidxPut(HIDX hix, dword dwMsg, SQIDX *psqi)
{
  SQIDX far *psqiFound;
  int rc;

  assert(hix->id==ID_HIDX);

  if (!hix->fBuffer)
  {
    (void)lseek(HixSqd->ifd, (long)(dwMsg-1) * (long)sizeof(SQIDX), SEEK_SET);

    if (farwrite(HixSqd->ifd, (char far *)psqi, sizeof(SQIDX)) != (int)sizeof(SQIDX))
    {
      msgapierr=MERR_NODS;
      return FALSE;
    }

    return TRUE;
  }

  /* If we can't find the appropriate index record */

  if ((psqiFound=sidx(hix, dwMsg))==NULL)
  {
    rc=FALSE;

    /* If the index is out of range, only create a new record if it's       *
     * to be placed at EOF.                                                 */

    if (dwMsg==hix->ha->num_msg+1)
      rc=_SquishAppendIndexRecord(hix, psqi);
  }
  else
  {
    *psqiFound=*psqi;
    rc=TRUE;
  }

  if (rc)
  {
    if (hix->lDeltaLo==-1 || hix->lDeltaLo > (long)dwMsg)
      hix->lDeltaLo=(long)dwMsg;

    if (hix->lDeltaHi==-1 || hix->lDeltaHi < (long)dwMsg)
      hix->lDeltaHi=(long)dwMsg;
  }

  return rc;
}


/* Delete an entry from the index */

unsigned _SquishRemoveIndexEntry(HIDX hix, dword dwMsg, SQIDX *psqiOut,
                                 SQHDR *psqh, int fFixPointers)
{
  SQIDX sqi;
  char *pcBuf;
  int got, i;

  assert(hix->id==ID_HIDX);

  /* Create a blank record for writing at the end */

  sqi.ofs=NULL_FRAME;
  sqi.umsgid=(UMSGID)-1L;
  sqi.hash=(dword)-1L;

  if (hix->fBuffer)
  {
    dword dwStart=1L;

    /* Find the segment containing the deleted message */

    for (i=0; i < hix->cSeg; i++)
    {
      /* If it's in this segment */

      if (dwMsg >= dwStart && dwMsg < dwStart + hix->pss[i].dwUsed)
      {
        int j=(int)(dwMsg-dwStart);
        unsigned rc=TRUE;

        /* If caller wants copy of deleted record */

        if (psqiOut)
          *psqiOut=hix->pss[i].psqi[j];

        /* Shift the rest of the text over this segment */

        (void)fmemmove(hix->pss[i].psqi+j, hix->pss[i].psqi+j+1,
                       (size_t)(hix->pss[i].dwUsed - (dword)j - (dword)1)
                          * (size_t)sizeof(SQIDX));

        hix->pss[i].dwUsed--;

        if (!_SquishAppendIndexRecord(hix, &sqi))
          rc=FALSE;

        if (hix->lDeltaLo==-1 || hix->lDeltaLo > (long)dwMsg)
          hix->lDeltaLo=(long)dwMsg;

        hix->lDeltaHi=(long)_SquishIndexSize(hix) / (long)sizeof(SQIDX);

        if (fFixPointers && rc)
          return _SquishFixMemoryPointers(hix->ha, dwMsg, psqh);
        else
          return rc;
      }

      dwStart += hix->pss[i].dwUsed;
    }

    /* Huh?  Message not in index! */

    return FALSE;
  }


  /* Else if it's not buffered: */


  (void)lseek(HixSqd->ifd, (long)dwMsg * (long)sizeof(SQIDX), SEEK_SET);


  if ((pcBuf=palloc(SHIFT_SIZE))==NULL)
  {
    msgapierr=MERR_NOMEM;
    return FALSE;
  }

  while ((got=read(HixSqd->ifd, pcBuf, SHIFT_SIZE)) > 0)
  {
    /* Skip back to one position before this index entry */

    (void)lseek(HixSqd->ifd, -(long)got - sizeof(SQIDX), SEEK_CUR);

    if (write(HixSqd->ifd, pcBuf, (unsigned)got) != got)
    {
      msgapierr=MERR_BADF;
      return FALSE;
    }

    (void)lseek(HixSqd->ifd, (long)sizeof(SQIDX), SEEK_CUR);
  }

  pfree(pcBuf);

  /* Now write the last entry to stomp over the index element that is at    *
   * the end of the file.                                                   */

  (void)lseek(HixSqd->ifd, -(long)sizeof(SQIDX), SEEK_CUR);

  if (write(HixSqd->ifd, (char *)&sqi, sizeof(SQIDX)) != (int)sizeof(SQIDX))
  {
    msgapierr=MERR_BADF;
    return FALSE;
  }

  if (fFixPointers)
    return _SquishFixMemoryPointers(hix->ha, dwMsg, psqh);
  else
    return TRUE;
}



/* Close an index file handle */

unsigned _SquishCloseIndex(HIDX hix)
{
  assert(hix->id==ID_HIDX);

  while (hix->fBuffer)
    if (!_SquishEndBuffer(hix))
      return FALSE;

  hix->id=0;

  pfree(hix);

  return TRUE;
}



/* Dump the index file buffer */

int _SquishEndBuffer(HIDX hix)
{
  int i;
  int rc=TRUE;
  long lSize;

  assert(hix->id==ID_HIDX);

  if (hix->fBuffer==0)
    return FALSE;

  if (--hix->fBuffer != 0)
    return TRUE;


  /* Reduce the index file to the size that it really should be */

  lSize=(long)hix->ha->num_msg * (long)sizeof(SQIDX);

  /* Only update the size of the index file if we know that we had
   * exclusive access to the area when reading the index.
   */

  if (hix->fHadExclusive)
    setfsize(HixSqd->ifd, lSize);


  /* If we need to rewrite the index */

  if (hix->lDeltaLo != -1 && hix->lDeltaHi != -1)
  {
    dword dwStart=1;

    (void) lseek(HixSqd->ifd,
                 (hix->lDeltaLo - 1L) * (long)sizeof(SQIDX),
                 SEEK_SET);

    for (i=0; i < hix->cSeg; i++)
    {
      unsigned uiWriteSize;

      /* If this buffer is within the "delta" range */

      if ((long)dwStart + (long)hix->pss[i].dwUsed > hix->lDeltaLo &&
          (long)dwStart <= hix->lDeltaHi)
      {
        size_t j, size;

        if ((long)dwStart > hix->lDeltaLo)
          j=0;
        else
          j=(size_t)(hix->lDeltaLo-(long)dwStart);

        if ((long)dwStart + (long)hix->pss[i].dwUsed > hix->lDeltaHi)
          size = (size_t)(hix->lDeltaHi - (long)dwStart + 1L);
        else
          size = (size_t)(hix->pss[i].dwUsed);

        size -= j;

        uiWriteSize=(size_t)size * (size_t)sizeof(SQIDX);

        if (rc)
        {
          if (farwrite(HixSqd->ifd, (char far *)(hix->pss[i].psqi+j), uiWriteSize)
                 != (int)uiWriteSize)

          {
            msgapierr=MERR_NODS;
            rc=FALSE;
          }
        }
      }

      dwStart += hix->pss[i].dwUsed;
    }
  }


  /* Free the memory used by these segments */

  for (i=0; i < hix->cSeg; i++)
    farpfree(hix->pss[i].psqi);

  pfree(hix->pss);
  hix->cSeg=0;

  return rc;
}

