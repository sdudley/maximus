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
static char rcs_id[]="$id";
#pragma on(unreferenced)

/*# name=File area routines: upload/download fileslist management
*/

#include <stdio.h>
#include <mem.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max_file.h"
#include "strbuf.h"

/*#if defined( OS_2 ) || defined( __FLAT__ )*/
#if 0
#define EXPAND_BUF  1
#define MAXEXPAND   16
#define CAN_DO_EXPAND
#else
#define EXPAND_BUF  0
#define MAXEXPAND   32
#endif

static  word maxnames=MAXEXPAND;                  /* Current buffer size */
static  word fnames;                              /* Number of filenames */

#ifdef CAN_DO_EXPAND
static  char canexpand=EXPAND_BUF;
#endif

static  FENTRY *filebuf;
static  strbuf *filestr;

void Init_File_Buffer(void)
{
  int x;

  x=maxnames*sizeof(FENTRY);
  filebuf=malloc(x);
  filestr=sb_new(maxnames*64);
  if (!filebuf || !filestr)
  {
    logit(mem_none);
    quit(ERROR_CRITICAL);
  }
  memset(filebuf, 0, x);
}

void Free_File_Buffer(void)
{
  free(filebuf);
  /* code to free filestr goes here */
}


int relocnm(char *fold, char *fnew)
{
  int i;
  FENTRY *f=filebuf;

  for ( i=0; i++ < fnames; ++i, ++f)
  {
    if (fold==f->szName)
      f->szName=fnew;
    else if (fold==f->szDesc)
      f->szDesc=fnew;
    else continue;
    return TRUE;
  }
  return FALSE;
}

static char * allocstr(char *s)
{
  char *p=sb_alloc(filestr,s);

  if (p==NULL)
  {
    strbuf *b=sb_realloc(filestr, sbsize(filestr)+1024, relocnm);
    if (b==NULL || (p=sb_alloc(b,s))==NULL)
    {
#ifdef CAN_DO_EXPAND
      canexpand=0;
#endif
      logit(mem_none);
      return NULL;
    }
    filestr=b;
  }
  return p;
}


static void freestr(char *s)
{
  if (s)
    sb_free(filestr,s);
}

void Free_Filenames_Buffer(word usLeave)
{

  if (usLeave==0)
  {
    /* Do this the fast way */

    fnames=0;
    sb_reset(filestr);
    memset(filebuf, 0, maxnames*sizeof(FENTRY));
  }
  else
  {
    /* Do it the slow way */

    word n;
    FENTRY *f=filebuf;

    for (n=usLeave; n < fnames; n++, f++)
    {
      freestr(f->szName);
      freestr(f->szDesc);
      memset(f, 0, sizeof(FENTRY));
    }

    f=filebuf;
    for (n=0; n < fnames && f->szName; n++, ++f)
      ;
    fnames=n;
  }
}

word FileEntries(void)
{
  return fnames;
}


int RemoveFileEntry(word n)
{
  word k;

  if (n < fnames)
  {
    FENTRY *f = filebuf + n;

    /* Remove filename from list */

    if (f->szName)
      freestr(f->szName);
    if (f->szDesc)
      freestr(f->szDesc);

    fnames--;
    k=fnames-n;

    if (k)
      memmove(f, f+1, k * sizeof(FENTRY));

    memset(f+k, 0, sizeof(FENTRY));

    return TRUE;
  }
  return FALSE;
}

int CanAddFileEntry(void)
{
  return (fnames < maxnames)
#ifdef CAN_DO_EXPAND
  || canexpand
#endif
  ;
}


int AddFileEntry(char *fname, word flags, long size)
{
  if (fnames == maxnames)
  {
    /* Try to expand it */
#ifdef CAN_DO_EXPAND
    if (canexpand)
    {
      word newmax=maxnames+16;
      FENTRY *newfilebuf=realloc(filebuf,newmax * sizeof(FENTRY));
      if (!newfilebuf)
        canexpand=0;
      else
      {
        filebuf=newfilebuf;
        memset(filebuf+maxnames, 0, 16 * sizeof(FENTRY));
        maxnames=newmax;
      }
    }
#endif
  }
  if (CanAddFileEntry())
  {
    FENTRY *f = filebuf+fnames;

    if ((f->szName=allocstr(fname))!=NULL)
    {
      f->fFlags=flags;
      f->szDesc=NULL;
      f->ulSize=(size >= 0L) ? size : fsize(fname);
      if (f->ulSize==(unsigned long)-1L)
        f->fFlags |= FFLAG_NOENT;
      return fnames++;
    }
  }
  return -1;
}

int GetFileEntry(word n, FENTRY *fent)
{
  if (n < fnames)
  {
    *fent=filebuf[n];
    return TRUE;
  }
  return FALSE;
}

int UpdFileEntry(word n, FENTRY *fent)
{
  int rc=TRUE;

  if (n < fnames)
  {
    FENTRY *f=filebuf+n;

    if (fent->szName != f->szName && strcmp(fent->szName, f->szName) != 0)
    {
      char *p=allocstr(fent->szName);

      if (!p)
        return FALSE;
      freestr(f->szName);
      f->szName=p;
    }

    if (fent->szDesc != f->szDesc &&
        !(fent->szDesc && f->szDesc && strcmp(fent->szDesc, f->szDesc)!=0))
    {
      freestr(f->szDesc);
      if (fent->szDesc)
        f->szDesc=allocstr(fent->szDesc);
      else f->szDesc=NULL;
    }
    f->ulSize=fent->ulSize;
    f->fFlags=fent->fFlags;
  }
  else if (AddFileEntry(fent->szName, fent->fFlags, fent->ulSize)!=-1)
  {
    if (fent->szDesc)
      filebuf[fnames-1].szDesc=allocstr(fent->szDesc);
  }
  else rc=FALSE;

  return rc;
}

