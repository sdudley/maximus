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
#include <stddef.h>
#include <limits.h>
#include <dos.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "nopen.h"
#include "bfile.h"


/* Translate a BFILE-type mode into one accepted by standard system calls */

static int near TranslateBMode(int fMode)
{
  int fNopenMode=0;

  if ((fMode & BO_RDWR)==BO_RDWR)
    fNopenMode |= O_RDWR;
  else if (fMode & BO_RDONLY)
    fNopenMode |= O_RDONLY;
  else if (fMode & BO_WRONLY)
    fNopenMode |= O_WRONLY;

  if (fMode & BO_APPEND)
    fNopenMode |= O_APPEND;

  if (fMode & BO_CREAT)
    fNopenMode |= O_CREAT;

  if (fMode & BO_TRUNC)
    fNopenMode |= O_TRUNC;

  if (fMode & BO_EXCL)
    fNopenMode |= O_EXCL;

/*  if (fMode & BO_BINARY) */
/* Bopen always handles binary files */
  fNopenMode |= O_BINARY;

  return fNopenMode;
}

/* Translate a BFILE-type sharing mode into one acceptable to nopen() */

static int near TranslateBShare(int fShare)
{
  if (fShare & BSH_COMPAT)
    return SH_COMPAT;
  else if (fShare & BSH_DENYNO)
    return SH_DENYNO;
  else if (fShare & BSH_DENYRW)
    return SH_DENYRW;
  else if (fShare & BSH_DENYWR)
    return SH_DENYWR;
  else if (fShare & BSH_DENYRD)
    return SH_DENYRD;
  else
    return SH_DENYNO;
}



/* Flush any pending writes to disk */

static int near Bflush(BFILE b)
{
  size_t size;
  
  /* If nothing has changed, just ignore everything */

  if (b->lDeltaLo==-1)
    return 0;
  
/*  printf("flush %ld..%ld\n", b->lPos+(long)b->iDeltaLo,
         b->lPos+(long)b->iDeltaHi);*/

  lseek(b->fd, b->lPos + b->lDeltaLo, SEEK_SET);
  
  size=(size_t)(b->lDeltaHi - b->lDeltaLo);

  /* Handle writes which are between INT_MAX and UINT_MAX */

#if 0
  if (size < INT_MAX)
  {
#endif
    if (nwrite(b->fd, b->pcBuf + (size_t)b->lDeltaLo, size) != (int)size)
      return -1;
#if 0
  }
  else
  {
    size_t try=size-(size_t)INT_MAX;

    if (nwrite(b->fd, b->pcBuf + (size_t)b->lDeltaLo, INT_MAX) != (int)INT_MAX)
      return -1;

    if (nwrite(b->fd, b->pcBuf + (size_t)b->lDeltaLo + (size_t)INT_MAX,
               try) != (int)try)
    {
      return -1;
    }
  }
#endif

  b->lDeltaLo=b->lDeltaHi=-1;

  return 0;
}


/* Refill the file buffer from disk to satisfy a read request */

static int near Brefill(BFILE b)
{
  long lNewPos;
  unsigned uiSaved;
  int iGot;

  /* Empty any writes to disk, if necessary */

  if ((b->fMode & BO_WRONLY) && Bflush(b)==-1)
    return -1;


  /* Move everything from b->pcBufCur..b->pcBufEnd to the elements          *
   * 0..(b->pcBufEnd-b->pcBufCur).                                          */

  uiSaved = (unsigned)(b->pcBufEnd - b->pcBufCur);

  if (b->pcBuf != b->pcBufCur)
    memmove(b->pcBuf, b->pcBufCur, uiSaved);

  /* Adjust the location of the beginning of the buffer */

  b->lPos += b->pcBufCur - b->pcBuf;

/*  printf("read %ld\n", b->lPos);*/

  /* Adjust memory pointers */

  b->pcBufCur = b->pcBuf;
  b->pcBufEnd = b->pcBuf + uiSaved;

  /* Seek to the appropriate position */

  lNewPos=b->lPos + (b->pcBufEnd - b->pcBuf);

  if (lseek(b->fd, lNewPos, SEEK_SET) != lNewPos)
    return -1;


  /* Fill the buffer up to the end */

  iGot=nread(b->fd,
             b->pcBufEnd,
             b->stBufSize - (size_t)(b->pcBufEnd - b->pcBuf));

  if (iGot <= 0)
    return -1;

  /* Increment the end pointer appropriately */

  b->pcBufEnd += iGot;
  return 0;
}



/* Open a buffered file */

BFILE _fast Bopen(char *pszName, int fMode, int fShare, size_t stBufSize)
{
  BFILE b;

#ifndef __FLAT__
  if (stBufSize >= INT_MAX)
    stBufSize = 16384;
#endif

  /* Allocate memory for the BFILE object */

  if ((b=malloc(sizeof(*b)))==NULL)
  {
    errno=ENOMEM;
    return NULL;
  }

  memset(b, '\0', sizeof *b);

  /* Fill out the header */

  b->id=BUF_ID;
  b->fMode=fMode;

  /* Try to open the file */

  if ((b->fd=nsopen(pszName,
                    TranslateBMode(fMode),
                    TranslateBShare(fShare),
                    S_IREAD | S_IWRITE))==-1)
  {
    free(b);
    return NULL;
  }


  /* Allocate memory for the buffer */

  if ((b->pcBuf=malloc(stBufSize))==NULL)
  {
    errno=ENOMEM;
    nclose(b->fd);
    free(b);
    return NULL;
  }

  /* Initialize the buffer pointers */

  b->pcBufCur=b->pcBuf;
  b->pcBufEnd=b->pcBuf;
  b->stBufSize=stBufSize;
  b->lDeltaLo=b->lDeltaHi=-1;

  b->lPos=0L;

  return b;
}



/* Close an existing BFILE object */

int _fast Bclose(BFILE b)
{
  int rc;

  if (b->id != BUF_ID)
    return -1;
  
  /* Flush any pending output */

  rc=Bflush(b);

  /* Now clean up memory structures */

  b->id=0;
  
  if (b->pcBuf)
    free(b->pcBuf);

  /* Close physical disk file */

  nclose(b->fd);
  free(b);
  
  return rc==0 ? 0 : -1;
}



/* Read bytes from a BFILE object */

int _fast Bread(BFILE b, void *pvBuf, unsigned uiSize)
{
  unsigned uiGot=0;

  /* Make sure that we are allowed to read from this file */

  if ((b->fMode & BO_RDONLY)==0)
  {
    errno=EACCES;
    return -1;
  }

  do
  {
    unsigned uiPass;
    
    /* Figure out how many bytes to move */

    uiPass=(unsigned)(b->pcBufEnd - b->pcBufCur);
    uiPass=min(uiPass, uiSize);
    
    if (uiPass)
    {
      /* Transfer as many bytes as we can */

      memmove(pvBuf, b->pcBufCur, uiPass);

      /* Increment pointers */

      pvBuf=(void *)((char *)pvBuf + uiPass);
      b->pcBufCur += uiPass;
      uiGot += uiPass;
      uiSize -= uiPass;
    }
    
    /* If we need to refill the buffer, do so. */

    if (uiPass==0 || b->pcBufCur==b->pcBufEnd)
      if (Brefill(b)==-1)
        break;
  }
  while (uiSize);
  
  return uiGot;
}



/* Write bytes to a BFILE object */

int _fast Bwrite(BFILE b, void *pvBuf, unsigned uiSize)
{
  unsigned uiGot=0;

  if ((b->fMode & BO_WRONLY)==0)
  {
    errno=EACCES;
    return -1;
  }

  do
  {
    unsigned uiPass;
    
    /* Figure out how many bytes to move */

    uiPass=(unsigned)(b->stBufSize - (b->pcBufCur-b->pcBuf));
    uiPass=min(uiPass, uiSize);
    
    /* If we have anything to copy */

    if (uiPass)
    {
      long lNewLo, lNewHi;

      /* Copy the text to the buffer */

      memmove(b->pcBufCur, pvBuf, uiPass);

      /* Set the delta pointers */

      lNewLo=(long)(unsigned)(b->pcBufCur - b->pcBuf);
      lNewHi=lNewLo + (long)uiPass;

      if (b->lDeltaLo==-1 || b->lDeltaLo > lNewLo)
        b->lDeltaLo=lNewLo;

      if (b->lDeltaHi==-1 || b->lDeltaHi < lNewHi)
        b->lDeltaHi=lNewHi;

      /* Adjust memory pointers */

      b->pcBufCur += uiPass;

      if (b->pcBufCur > b->pcBufEnd)
        b->pcBufEnd = b->pcBufCur;

      pvBuf=(void *)((char *)pvBuf + uiPass);
      uiGot += uiPass;
      uiSize -= uiPass;
    }
    

    /* If we need to dump the buffer, do so. */

    if (uiPass==0 || b->pcBufCur==b->pcBuf+b->stBufSize)
    {
      if (Bflush(b)==-1)
        break;

      b->lPos += b->pcBufCur - b->pcBuf;
      b->pcBufCur=b->pcBuf;
      b->pcBufEnd=b->pcBuf;
    }
  }
  while (uiSize);
  
  return uiGot;
}


/* Get one character from a BFILE */

int _fast _Bgetc(BFILE b)
{
  if (b->pcBufCur < b->pcBufEnd)
    return *b->pcBufCur++;

  Brefill(b);

  if (b->pcBufCur < b->pcBufEnd)
    return *b->pcBufCur++;
  else
    return EOF;
}


/* Seek to a specific location in the file */

long _fast Bseek(BFILE b, long lRelPos, int fWhere)
{
  long lNewPos;
  int rc;

  switch (fWhere)
  {
    default:

    case BSEEK_SET: lNewPos=lRelPos; break;
    case BSEEK_CUR: lNewPos=b->lPos + (b->pcBufCur - b->pcBuf) + lRelPos; break;
    case BSEEK_END:
      /* Find the physical end of file */

      lNewPos=lseek(b->fd, 0L, SEEK_END);

      /* Make sure that we seek to the virtual end of file, if necessary */

      if (lNewPos < b->lPos + b->lDeltaHi)
        lNewPos=b->lPos + b->lDeltaHi;

      lNewPos += lRelPos;
      break;
  }

  if (lNewPos < 0L)
    lNewPos=0L;

  if (lNewPos >= b->lPos && lNewPos < b->lPos + (long)(b->pcBufEnd - b->pcBuf))
  {
    b->pcBufCur=b->pcBuf + (size_t)(lNewPos-b->lPos);
    return lNewPos;
  }

  rc=Bflush(b);

  b->lPos=lNewPos;
  b->pcBufCur=b->pcBufEnd=b->pcBuf;

  return rc==0 ? lNewPos : -1;
}



/* Returns file number for a buffered file */

int _fast Bfileno(BFILE b)
{
  return b->fd;
}



/* Return the current file pointer position of the BFILE object */

long _fast Btell(BFILE b)
{
  return b->lPos + (b->pcBufCur - b->pcBuf);
}


/* Read a string from the BFILE object */

char * _fast Bgets(char *pszOut, unsigned uiMaxLen, BFILE b)
{
  int ch=0;
  char *s, *end;
  
  for (s=pszOut, end=s+uiMaxLen; s < end; )
  {
    if ((ch=Bgetc(b))=='\n' || ch==EOF)
      break;

    *s++=(char)ch;
  }

  *s++='\0';

  if (ch==EOF && s==pszOut+1)
    return NULL;

  Strip_Trailing(pszOut, '\r');

  return pszOut;
}



/* Write a string to the BFILE object */

int _fast Bputs(BFILE b, char *pszOut)
{
  unsigned uiSize=strlen(pszOut);

  return Bwrite(b, pszOut, uiSize)==uiSize ? 0 : -1;
}


#ifdef TEST_HARNESS
main()
{
  BFILE b;
  char temp[PATHLEN];
  char *p;
  long l;
  int i;

  if ((b=Bopen("test", BO_RDWR | BO_CREAT | BO_TRUNC, BSH_DENYNO, 32))==NULL)
  {
    printf("Error opening file \"test\"\n");
    return 1;
  }

  Bseek(b, 0L, BSEEK_SET);

  for (i=0; i < 100; i++)
  {
    sprintf(temp, "BFile list #%d - pos=%ld!\r\n", i, Btell(b));
    printf("%s", temp);

    if (Bputs(b, temp) != 0)
    {
      printf("Error writing to BFILE! %d\n", i);
      exit(1);
    }
  }

  printf("Now pos=%ld\n", Btell(b));
  printf("Seek back 8, pos=%ld\n", l=Bseek(b, -8L, BSEEK_CUR));

  p=Bgets(temp, PATHLEN, b);
  printf("Read line='%s', p=%p\n", temp, p);

  printf("Seek 0=%l\n", Bseek(b, 0L, SEEK_SET));
  printf("Seek to end again, pos=%ld\n", Bseek(b, l, BSEEK_SET));
  p=Bgets(temp, PATHLEN, b);
  printf("Read line='%s', p=%p\n", temp, p);


  printf("Seek to beginning=%ld\n", Bseek(b, 0L, BSEEK_SET));
  p=Bgets(temp, PATHLEN, b);

  printf("First line='%s', p=%p\n", temp, p);

  printf("Seek to 5 = %ld\n", Bseek(b, 5, BSEEK_SET));

  i=Bwrite(b, "Foobar", 6);

  printf("Write 6 bytes at 5 = %d\n", i);

  Bseek(b, 0L, BSEEK_SET);

  p=Bgets(temp, PATHLEN, b);

  printf("First line is now '%s'\n", temp);

  Bclose(b);

  return 0;
}
#endif


