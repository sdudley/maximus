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
static char rcs_id[]="$Id: tagapi.c,v 1.1.1.1 2002/10/01 17:53:09 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

#ifdef OS_2
#define INCL_DOS
#include "pos2.h"
#endif

#include "max_msg.h"
#include "tagapip.h"

#include "ffind.h"

static char szTagSuffix[]="\x01%s\x01";
static char szEmptyList[]="\x01";

/****************************************************************************/
/********************* ENTRYPOINT: TagReadTagFile ***************************/
/****************************************************************************/

/* Read in a tag file.  This is used to read in the data records            *
 * from the mtag.* files to obtain a list of the selected areas.            */

int TagReadTagFile(struct _mtagmem *pmtm)
{
  struct _mtagidx mti;

  memset(pmtm, 0, sizeof *pmtm);

  /* Open the index file */

  if (!_TagReadIdx(&mti))
  {
    _TagNewPmtm(pmtm);
    return FALSE;
  }

  if (SquishHash(usr.name) != mti.dwUserHash)
    _TagReuseIdx(&mti, usr.lastread_ptr);

  return _TagGetMtagMem(&mti, pmtm);
}


/****************************************************************************/
/********************* ENTRYPOINT: TagWriteTagFile **************************/
/****************************************************************************/


/* Write out a tag file.  This function updates the on-disk MTAG.* files    *
 * with the information contained in pmtm.                                  */

int TagWriteTagFile(struct _mtagmem *pmtm)
{
  char fnamei[PATHLEN];
  char fnamed[PATHLEN];
  FILE *fpi, *fpd;
  struct _mtagidx mti;
  long ofs;

  sprintf(fnamei, mtag_idx, PRM(sys_path));
  sprintf(fnamed, mtag_dat, PRM(sys_path));

  /* Read the existing index */

  if (!_TagReadIdx(&mti))
  {
    mti.dwOffset=0L;
    mti.dwLen=0L;
    mti.dwUsed=0L;
  }

  /* If the length of the in-memory index entry is greater than the on-disk *
   * entry, we need to get a new index record.                              */

  if (pmtm->dwLen > mti.dwLen)
  {
    _TagReuseIdx(&mti, usr.lastread_ptr);

    /* Set the seek position to the end of the data file */

    mti.dwOffset=fsize(fnamed);

    /* If the file doesn't exist, start at the beginning */

    if (mti.dwOffset==-1)
      mti.dwOffset=0;
  }

  if ((fpi=fopen(fnamei, fexist(fnamei) ? fopen_readpb : fopen_writepb))==NULL)
  {
    cant_open(fnamei);
    return FALSE;
  }

  if ((fpd=fopen(fnamed, fexist(fnamed) ? fopen_readpb : fopen_writepb))==NULL)
  {
    cant_open(fnamed);
    fclose(fpi);
    return FALSE;
  }


  /* Now fill in the rest of the structure from memory */

  mti.dwUserHash=SquishHash(usr.name);
  mti.dwLen=pmtm->dwLen;
  mti.dwUsed=pmtm->dwUsed;

  fseek(fpi, 0L, SEEK_END);

  ofs=usr.lastread_ptr * (long)sizeof mti;

  /* Fill the file up to the appropriate offset */

  while (ftell(fpi) < ofs)
  {
    struct _mtagidx mti_new;

    memset(&mti_new, 0, sizeof mti_new);

    if (fwrite(&mti_new, sizeof mti_new, 1, fpi) != 1)
      break;
  }

  fseek(fpi, ofs, SEEK_SET);

  if (fwrite(&mti, sizeof mti, 1, fpi) != 1)
    logit(cantwrite, fnamei);


  fseek(fpd, mti.dwOffset, SEEK_SET);

  if (pmtm->pbAreas)
  {
    /* Set the rest of the record to zeroes */

    memset(pmtm->pbAreas+pmtm->dwUsed, 0, pmtm->dwLen-pmtm->dwUsed);

    if (fwrite(pmtm->pbAreas, mti.dwLen, 1, fpd) != 1)
      logit(cantwrite, fnamed);

    free(pmtm->pbAreas);
  }

  pmtm->pbAreas=NULL;

  fclose(fpd);
  fclose(fpi);
  return TRUE;
}


/****************************************************************************/
/********************* ENTRYPOINT: TagQueryTagList **************************/
/****************************************************************************/

/* This function returns TRUE if the area name specified by                 *
 * pszArea is found in the tag list.                                        */

int TagQueryTagList(struct _mtagmem *pmtm, char *pszArea)
{
  char szAreaName[PATHLEN];

  if (!pmtm->pbAreas)
    return FALSE;

  sprintf(szAreaName, szTagSuffix, pszArea);

  return stristr(pmtm->pbAreas, szAreaName) != NULL;
}


/****************************************************************************/
/********************* ENTRYPOINT: TagAddTagList ****************************/
/****************************************************************************/

int TagAddTagList(struct _mtagmem *pmtm, char *pszArea)
{
  char szAreaName[PATHLEN];

  if (!pmtm->pbAreas)
  {
    /* If there is no area list, try to allocate one */

    if ((pmtm->pbAreas=(char *)malloc(MTAG_PAD_SIZE))==NULL)
      return FALSE;

    strcpy(pmtm->pbAreas, szEmptyList);

    pmtm->dwUsed=1;
    pmtm->dwLen=MTAG_PAD_SIZE;
  }

  sprintf(szAreaName, szTagSuffix, pszArea);

  /* Don't allow the user to tag the same area twice! */

  if (stristr(pmtm->pbAreas, szAreaName))
    return FALSE;

  /* If this would be too much, try to reallocate the buffer used for       *
   * storing the tag information.                                           */

  if (pmtm->dwUsed + strlen(szAreaName)+1 >= pmtm->dwLen)
  {
    if ((pmtm->pbAreas=(char *)realloc(pmtm->pbAreas, pmtm->dwLen+MTAG_PAD_SIZE))==NULL)
      return FALSE;

    /* Increase the padding for this entry */

    pmtm->dwLen += MTAG_PAD_SIZE;
  }

  strcpy(pmtm->pbAreas + (*pmtm->pbAreas ? strlen(pmtm->pbAreas)-1 : 0),
         szAreaName);

  pmtm->dwUsed=strlen(pmtm->pbAreas);

  return TRUE;
}


/****************************************************************************/
/********************* ENTRYPOINT: TagDeleteTagList ************************/
/****************************************************************************/

int TagDeleteTagList(struct _mtagmem *pmtm, char *pszArea)
{
  char szAreaName[PATHLEN];
  char *p, *e;

  if (!pmtm->pbAreas)
    return FALSE;

  sprintf(szAreaName, szTagSuffix, pszArea);

  /* Find this area within the list of names */

  if ((p=stristr(pmtm->pbAreas, szAreaName))==NULL)
    return FALSE;

  /* Find the next terminating ^a and move the text over. */

  e=p+strlen(szAreaName)-1;

  memmove(p, e, strlen(e)+1);

  /* Decrement the number of used bytes in this list */

  pmtm->dwUsed=strlen(pmtm->pbAreas);

  return TRUE;
}



/* This function reads the data record specified by pmti and loads it       *
 * into memory, placing the results in the pmtm structure.                  */

static int _TagGetMtagMem(struct _mtagidx *pmti, struct _mtagmem *pmtm)
{
  char fname[PATHLEN];
  FILE *fp;

  sprintf(fname, mtag_dat, PRM(sys_path));

  /* If the data file was too short, just create a new record */

  if ((fp=fopen(fname, fopen_readb))==NULL)
  {
    _TagNewPmtm(pmtm);
    return TRUE;
  }

  fseek(fp, pmti->dwOffset, SEEK_SET);

  if (!pmti->dwLen)
    _TagNewPmtm(pmtm);
  else if ((pmtm->pbAreas=(char *)malloc(pmti->dwLen))==NULL)
  {
    logit(mem_none);
    _TagNewPmtm(pmtm);
  }
  else
  {
    if (fread(pmtm->pbAreas, pmti->dwUsed, 1, fp) != 1)
    {
      fseek(fp, 0L, SEEK_END);

      _TagNewPmtm(pmtm);
    }
    else
    {
      pmtm->pbAreas[pmti->dwUsed]=0;
      pmtm->dwUsed=pmti->dwUsed;
      pmtm->dwLen=pmti->dwLen;
    }
  }

  fclose(fp);
  return TRUE;
}


/* Create a new in-memory record of tagged areas */

static void near _TagNewPmtm(struct _mtagmem *pmtm)
{
  pmtm->pbAreas=(char *)malloc(MTAG_PAD_SIZE);

  if (pmtm->pbAreas)
    strcpy(pmtm->pbAreas, szEmptyList);

  pmtm->dwLen=MTAG_PAD_SIZE;
  pmtm->dwUsed=1;
}


#if 0 /* ?not currently used */

/* This function physically copies the tag data from one file to another,   *
 * removing any dead space and leaving enough room for future expansion.    */

static int near _TagPackMessages(FILE *in_i, FILE *in_d, FILE *out_i, FILE *out_d, int recnum)
{
  struct _mtagidx mti_in, mti_out;
  byte *pbBuf;
  int processed=0;

  logit(log_repacking_mtag);

  if ((pbBuf=(byte *)malloc(PACK_BUF_SIZE))==NULL)
  {
    logit(mem_none);
    return FALSE;
  }

  /* Read each record in the original index */

  while (fread(&mti_in, sizeof mti_in, 1, in_i)==1)
  {
    mti_out=mti_in;

    /* Skip over the record to be deleted, if necessary */

    if (processed++ != recnum && mti_out.dwLen)
    {
      mti_out.dwOffset=ftell(out_d);

      /* Round up to the nearest multiple of MTAG_PAD_SIZE bytes */

      mti_out.dwLen=(mti_in.dwUsed + MTAG_PAD_SIZE) / MTAG_PAD_SIZE * MTAG_PAD_SIZE;

      /* Seek to record location in original index file */

      fseek(in_d, mti_in.dwOffset, SEEK_SET);

      /* Loop around and copy data as appropriate */

      while (mti_in.dwLen > 0)
      {
        int len=min(PACK_BUF_SIZE, mti_in.dwLen);

        if (fread(pbBuf, len, 1, in_d) != 1)
        {
          logit(log_error_pack, 2);
          break;
        }
        else
        {
          if (fwrite(pbBuf, len, 1, out_d) != 1)
            logit(log_error_pack, 3);

          mti_in.dwLen -= len;
        }
      }
    }

    if (fwrite(&mti_out, sizeof mti_out, 1, out_i) != 0)
    {
      logit(log_error_pack, 1);
      break;
    }
  }

  free(pbBuf);

  return TRUE;
}
#endif

#if 0 /* not currently used? */

/* This function repacks the message tag file by eliminating dead space     *
 * in mtag.dat, removing free frames from mtag.fre, and generally cleaning  *
 * up the entire works.                                                     */

static int near _TagRepackMtag(int recnum)
{
  char ifnamei[PATHLEN];
  char ifnamed[PATHLEN];
  char ofnamei[PATHLEN];
  char ofnamed[PATHLEN];

  FILE *in_i, *in_d;
  FILE *out_i, *out_d;

  int rc;

  sprintf(ifnamei, mtag_idx, PRM(sys_path));
  sprintf(ifnamed, mtag_dat, PRM(sys_path));

  sprintf(ofnamed, "%smtagdat.$$$", PRM(sys_path));
  sprintf(ofnamei, "%smtagidx.$$$", PRM(sys_path));

  /* Open all of the input files necessary to do the pack */

  if ((in_i=fopen(ifnamei, fopen_readb))==NULL)
  {
    cant_open(ifnamei);
    return FALSE;
  }

  if ((in_d=fopen(ifnamed, fopen_readb))==NULL)
  {
    fclose(in_i);
    cant_open(ifnamed);
    return FALSE;
  }

  /* Open all of the output files necessary to do the pack */

  if ((out_d=fopen(ofnamed, fopen_writeb))==NULL)
  {
    fclose(in_i);
    fclose(in_d);
    cant_open(ofnamed);
    return FALSE;
  }

  if ((out_i=fopen(ofnamei, fopen_writeb))==NULL)
  {
    fclose(out_d);
    fclose(in_i);
    fclose(in_d);
    cant_open(ofnamed);
    return FALSE;
  }


  /* Perform the actual packing operation */

  rc=_TagPackMessages(in_i, in_d, out_i, out_d, recnum);

  fclose(out_i);
  fclose(out_d);
  fclose(in_d);
  fclose(in_i);

  /* Unlink the original files and move the others back to the right pos'n */

  unlink(ifnamed);
  rename(ofnamed, ifnamed);

  unlink(ifnamei);
  rename(ofnamei, ifnamei);

  return rc;
}
#endif


/* Add the given index to the list of free message frames in MTAG.FRE */

static int near _TagAddToFree(struct _mtagidx *pmti, int recnum)
{
  struct _mtagfre mtf;
  char fname[PATHLEN];
  long len;
  int fd;

  NW(recnum);

  /* Nothing to add if it is a zero-length block */

  if (!pmti->dwLen)
    return TRUE;

  sprintf(fname, mtag_fre, PRM(sys_path));

  if ((fd=shopen(fname, O_WRONLY | O_BINARY))==-1)
    fd=sopen(fname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, SH_DENYNO,
             S_IREAD | S_IWRITE);

  if (fd==-1)
    return FALSE;

  lseek(fd, 0L, SEEK_END);

  /* Copy the length and offset information from the user record */

  mtf.dwOffset=pmti->dwOffset;
  mtf.dwLen=pmti->dwLen;

  if (write(fd, (char *)&mtf, sizeof mtf) != sizeof mtf)
    logit(cantwrite, fname);

  len=tell(fd);
  close(fd);

  /* If we have created too many free blocks within the file, repack the    *
   * main database.                                                         */

  if (len > MAX_MTAG_FRE * (long)sizeof mtf)
  {
    /*_TagRepackMtag(recnum);*/
    unlink(fname);
  }

  return TRUE;
}


/* The index pointed to by pmti was in use in the past, but we need to      *
 * reuse this offset for other things.  We must simply return this to       *
 * the free list (and perform compaction, if necessary).                    */

static int near _TagReuseIdx(struct _mtagidx *pmti, int recnum)
{
  /* Get rid of the old block */

  _TagAddToFree(pmti, recnum);
  pmti->dwUserHash=SquishHash(usr.name);
  pmti->dwUsed=0L;

  return TRUE;
}


/* This function reads the index entry for the user (specified by           *
 * usr.lastread_ptr) and places the data into pmti.                         */

static int near _TagReadIdx(struct _mtagidx *pmti)
{
  char fname[PATHLEN];
  long lOfs;
  int fd;

  sprintf(fname, mtag_idx, PRM(sys_path));

  if ((fd=shopen(fname, O_RDONLY | O_BINARY))==-1)
    fd=sopen(fname, O_WRONLY | O_BINARY, SH_DENYNO, S_IREAD | S_IWRITE);

  if (fd==-1)
    return FALSE;

  /* Read in the index record */

  lseek(fd, lOfs=usr.lastread_ptr * (long)sizeof *pmti, SEEK_SET);

  if (read(fd, (char *)pmti, sizeof *pmti) != sizeof *pmti)
  {
    /* Expand the file to the appropriate length */

    memset(pmti, 0, sizeof *pmti);

    while (lseek(fd, 0L, SEEK_END) < lOfs)
      if (write(fd, (char *)pmti, sizeof *pmti) != sizeof *pmti)
      {
        close(fd);
        return FALSE;
      }
  }

  close(fd);

  return TRUE;
}

