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

#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"
#include "userapi.h"
#include "uni.h"
#include "compiler.h"

#ifdef OS_2
  #define INCL_DOS
  #include "pos2.h"
#endif

dword _fast UserHash(byte *f)
{
  dword hash=0, g;
  signed char *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + (dword)tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }
  
  return (hash & 0x7fffffffLu);
}




/* This function will rebuild the user file index if it determines that
 * there is a record count mismatch between the main user file and the
 * index file.
 */

static void near _RebuildIndex(HUF huf)
{
  long idxsize=lseek(huf->fdndx, 0L, SEEK_END) / (long)sizeof(USRNDX);
  long size=UserFileSize(huf);
  struct _usr user;
  USRNDX usrndx;

  /* If the index and user files are the same number of records, there
   * is no need to rebuild.
   */

  if (size==idxsize)
    return;

  lseek(huf->fdbbs, 0L, SEEK_SET);
  lseek(huf->fdndx, 0L, SEEK_SET);

  /* Truncate index file to zero bytes long */

  setfsize(huf->fdndx, 0L);

  lseek(huf->fdbbs, 0L, SEEK_SET);

  while (read(huf->fdbbs, (char *)&user, sizeof user)==sizeof user)
  {
    usrndx.hash_name=UserHash(user.name);
    usrndx.hash_alias=UserHash(user.alias);

    if (write(huf->fdndx, (char *)&usrndx, sizeof usrndx) != sizeof usrndx)
      return;
  }
}


/* Returns TRUE if the usr.name or usr.alias fields match those given
 * in the user structure.
 */

static int near _UserMatch(HUF huf, long ofs, char *name, char *alias,
                           struct _usr *pusr)
{
  long pos=(long)sizeof(struct _usr) * ofs;
  int fNameMatch;
  int fAliasMatch;

  if (lseek(huf->fdbbs, pos, SEEK_SET) != pos)
    return FALSE;

  if (read(huf->fdbbs, (char *)pusr, sizeof *pusr) != sizeof *pusr)
    return FALSE;

  fNameMatch=(name && eqstri(pusr->name, name));
  fAliasMatch=(alias && eqstri(pusr->alias, alias));

  return ((fNameMatch && !alias) ||
          (fAliasMatch && !name) ||
          (fNameMatch && fAliasMatch) ||
          (!name && !alias));
}




/* Open the user file for access.  To create a new user file, specify
 * O_CREAT | O_TRUNC for the mode parameter.
 */

HUF _fast UserFileOpen(char *name, int mode)
{
  char filename[PATHLEN];
  int tries;
  HUF huf;

  if ((huf=malloc(sizeof(*huf)))==NULL)
    return NULL;

  huf->id_huf=ID_HUF;

  strcpy(filename, name);
  strcat(filename, ".bbs");

  for (tries=10; tries--; )
  {
    if ((huf->fdbbs=sopen(filename, O_RDWR | O_BINARY | mode, SH_DENYNO,
                          S_IREAD | S_IWRITE)) != -1)
      break;

#ifdef OS_2
    if (fexist(name))
      DosSleep(1L);
#endif
  }

  if (huf->fdbbs==-1)
  {
    free(huf);
    return NULL;
  }

  strcpy(filename, name);
  strcat(filename, ".idx");

  if ((huf->fdndx=sopen(filename, O_RDWR | O_BINARY | mode, SH_DENYNO,
                        S_IREAD | S_IWRITE))==-1)
  {
    /* If the index file was damaged, recreate it */

    if (huf->fdndx==-1)
    {
      huf->fdndx=sopen(filename, O_CREAT | O_TRUNC | O_RDWR | O_BINARY,
                       SH_DENYNO, S_IREAD | S_IWRITE);

      if (huf->fdndx != -1)
        _RebuildIndex(huf);
    }

    if (huf->fdndx==-1)
    {
      close(huf->fdbbs);
      free(huf);
      return NULL;
    }
  }

  return huf;
}



/* Return the number of users in the user file */

long _fast UserFileSize(HUF huf)
{
  long len;

  if (!huf || huf->id_huf != ID_HUF)
    return -1L;

  len=lseek(huf->fdbbs, 0L, SEEK_END);
  len /= (long)sizeof(struct _usr);

  return len;
}

int _fast UserFileSeek(HUF huf, long rec, struct _usr *pusr, int sz)
{
  long len;

  if ((len=UserFileSize(huf))==-1L)
    return FALSE;

  if (rec==-1)
    rec=len-1;

  if (rec < 0 || rec >= len)
    return FALSE;

  lseek(huf->fdbbs, rec*sizeof(struct _usr), SEEK_SET);
  return read(huf->fdbbs, (char *)pusr, sz) == sz;
}


/* Find the index of a user within the user file.  Returns the offset
 * if found, or -1L if the user/alias could not be located.  Either
 * or both of 'name' or 'alias' can be NULL.
 *
 * This is an internal function only!  (It has a plOfs option!)
 * Use the external UserFileFind as an external entrypoint.
 */

static int _fast _UserFileFind(HUF huf, char *name, char *alias,
                               struct _usr *pusr, long *plOfs,
                               long lStartOfs, int fForward)
{
  dword hash_name=name ? UserHash(name) : -1L;
  dword hash_alias=alias ? UserHash(alias) : -1L;
  USRNDX *pun, *pu;
  long ofs;
  int got;

  /* Rebuild the index, if necessary */

  _RebuildIndex(huf);

  /* Allocate memory for a block of user index records */

  if ((pun=malloc(sizeof(USRNDX) * UNDX_BLOCK))==NULL)
    return FALSE;

  /* Seek to the beginning of the index file */

  ofs=lStartOfs;

  for (;;)
  {
    long pos;

    if (fForward)
      pos=ofs;
    else
      pos=ofs > (long)UNDX_BLOCK ? ofs-(long)UNDX_BLOCK : 0L;

    lseek(huf->fdndx, pos * (long)sizeof(USRNDX), SEEK_SET);

    if ((got=read(huf->fdndx, (char *)pun,
                  UNDX_BLOCK * sizeof(USRNDX))) < sizeof(USRNDX))
      break;

    if (!fForward)
      ofs=pos+(long)got-1L;

    /* Divide by the length of the structure */

    got /= (int)sizeof(USRNDX);

    for (pu=fForward ? pun : pun+got-1; got--; fForward ? pu++ : pu--)
    {
      if (name && hash_name==pu->hash_name && !alias ||
          alias && hash_alias==pu->hash_alias && !name ||
          hash_name==pu->hash_name && hash_alias==pu->hash_alias ||
          !name && !alias)
      {
        /* Now check the hash against the user name itself */

        if (_UserMatch(huf, ofs, name, alias, pusr))
        {
          free(pun);
          *plOfs=ofs;
          return TRUE;
        }
      }

      if (fForward)
        ofs++;
      else
        ofs--;
    }
  }

  free(pun);
  return FALSE;
}



/* External entrypoint for UserFileFind */

int _fast UserFileFind(HUF huf, char *name, char *alias, struct _usr *pusr)
{
  long ofs;

  if (!huf || huf->id_huf != ID_HUF)
    return FALSE;

  return _UserFileFind(huf, name, alias, pusr, &ofs, 0L, TRUE);
}


/* UserFileFindOpen
 *
 * This function opens a multi-user find session.  The name and
 * alias parameters are treated the same way as they are in the
 * UserFileFind function.
 *
 * If the user is found, the function returns a HUFF handle and
 * the huff->usr structure contains the record of the found user.
 * UserFileFindNext/Prior must be called (with the appropriate
 * arguments) to find the remaining users.
 *
 * If successful, this function returns a non-NULL handle.  If
 * the user was not found, this function returns NULL.
 */

HUFF _fast UserFileFindOpen(HUF huf, char *name, char *alias)
{
  HUFF huff;

  if (!huf || huf->id_huf != ID_HUF)
    return NULL;

  if ((huff=malloc(sizeof *huff))==NULL)
    return NULL;

  huff->id_huff=ID_HUFF;
  huff->huf=huf;
  huff->lLastUser=-1L;
  huff->ulStartNum=0L;
  huff->cUsers=0;

  /* Allocate memory for the arary of users */

  if ((huff->pusr=malloc(UBBS_BLOCK * sizeof(struct _usr)))==NULL)
  {
    free(huff);
    return NULL;
  }

  if (!UserFileFindNext(huff, name, alias))
  {
    free(huff->pusr);
    free(huff);
    return NULL;
  }

  return huff;
}


/* Find the next user record in sequence */

int _fast UserFileFindNext(HUFF huff, char *name, char *alias)
{
  HUF huf;
  dword dwSize;
  long ofs;

  if (!huff || huff->id_huff != ID_HUFF)
    return FALSE;

  huf=huff->huf;
  dwSize=UserFileSize(huf);

  /* If we're looking for the next instance of a specific name, use the
   * index to do the search.
   */

  if (name || alias)
  {
    if (_UserFileFind(huf, name, alias, &huff->usr,
                      &ofs, huff->lLastUser+1, TRUE))
    {
      huff->lLastUser=ofs;
      return TRUE;
    }

    return FALSE;
  }


  /* Loop through all of the offsets until we find what we're looking for */

  for (ofs=(unsigned long)(huff->lLastUser+1); ofs < dwSize; ofs++)
  {
    /* If the offset is out of bounds... */

    if (ofs < huff->ulStartNum || ofs >= huff->ulStartNum+huff->cUsers)
    {
      int size=UBBS_BLOCK * sizeof(struct _usr);
      int got;

      lseek(huf->fdbbs, ofs * (long)sizeof(struct _usr), SEEK_SET);
      got=read(huf->fdbbs, (char *)huff->pusr, size);

      /* Update buffer pointers, if appropriate */

      if (got >= 0)
        got /= sizeof(struct _usr);

      if (got >= 0)
      {
        huff->ulStartNum=ofs;
        huff->cUsers = got;
      }

      /* If we reached EOF, just ignore it */

      if (got < 0 ||
          ofs < huff->ulStartNum ||
          ofs >= huff->ulStartNum+huff->cUsers)
      {
        huff->lLastUser=UserFileSize(huf);
        return FALSE;
      }
    }

    huff->usr=huff->pusr[ofs - huff->ulStartNum];
    huff->lLastUser=ofs;
    return TRUE;
  }

  huff->lLastUser=UserFileSize(huf);
  return FALSE;
}


/* Find the prior user record in sequence */

int _fast UserFileFindPrior(HUFF huff, char *name, char *alias)
{
  HUF huf;
  long ofs;

  if (!huff || huff->id_huff != ID_HUFF)
    return FALSE;

  huf=huff->huf;

  /* If we're looking for the next instance of a specific name, use the
   * index to do the search.
   */

  if (name || alias)
  {
    if (_UserFileFind(huf, name, alias, &huff->usr,
                      &ofs, huff->lLastUser-1, FALSE))
    {
      huff->lLastUser=ofs;
      return TRUE;
    }

    return FALSE;
  }


  /* Loop through all of the offsets until we find what we're looking for */

  for (ofs=huff->lLastUser-1; ofs >= 0; ofs--)
  {
    /* If the offset is out of bounds... */

    if (ofs < huff->ulStartNum || ofs >= huff->ulStartNum+huff->cUsers)
    {
      int size=UBBS_BLOCK * sizeof(struct _usr);
      int got;

      lseek(huf->fdbbs, ofs * (long)sizeof(struct _usr), SEEK_SET);
      got=read(huf->fdbbs, (char *)huff->pusr, size);

      if (got >= 0)
        got /= sizeof(struct _usr);

      if (got >= 0)
      {
        huff->ulStartNum=ofs;
        huff->cUsers = got;
      }

      /* If we reached EOF, just ignore it */

      if (got < 0 ||
          ofs < huff->ulStartNum ||
          ofs >= huff->ulStartNum+huff->cUsers)
      {
        huff->lLastUser=0L;
        return FALSE;
      }
    }

    huff->usr=huff->pusr[ofs - huff->ulStartNum];
    huff->lLastUser=ofs;
    return TRUE;
  }

  huff->lLastUser=0L;
  return FALSE;
}



/* Close a user file finding session */

int _fast UserFileFindClose(HUFF huff)
{
  if (!huff || huff->id_huff != ID_HUFF)
    return FALSE;

  free(huff->pusr);
  free(huff);

  return TRUE;
}



/* Update an existing user record */

int _fast UserFileUpdate(HUF huf, char *name, char *alias, struct _usr *pusr)
{
  USRNDX usrndx;
  struct _usr usr;
  long ofs;

  /* Try to find the old user */

  if (!_UserFileFind(huf, name, alias, &usr, &ofs, 0L, TRUE))
    return FALSE;


  /* Seek to appropriate offset */

  lseek(huf->fdbbs, ofs * (long)sizeof(struct _usr), SEEK_SET);
  lseek(huf->fdndx, ofs * (long)sizeof(USRNDX), SEEK_SET);


  /* Construct the new index entry for this user */

  usrndx.hash_name=UserHash(pusr->name);
  usrndx.hash_alias=UserHash(pusr->alias);


  /* Return TRUE only if both writes succeed */

  return write(huf->fdbbs, (char *)pusr, sizeof *pusr) == sizeof *pusr &&
         write(huf->fdndx, (char *)&usrndx, sizeof(USRNDX)) == sizeof(USRNDX);
}

/* Write a new user record to the user filee */

int _fast UserFileCreateRecord(HUF huf, struct _usr *pusr, int fCheckUnique)
{
  USRNDX usrndx;
  long ofs=UserFileSize(huf);
  struct _usr junkusr;

  /* Make sure that the user doesn't already exist */

  if (fCheckUnique && UserFileFind(huf, pusr->name, NULL, &junkusr))
    return FALSE;

  /* Append to end */

  lseek(huf->fdbbs, ofs * (long)sizeof(struct _usr), SEEK_SET);
  lseek(huf->fdndx, ofs * (long)sizeof(USRNDX), SEEK_SET);

  /* Construct the new index entry for this user */

  usrndx.hash_name=UserHash(pusr->name);
  usrndx.hash_alias=UserHash(pusr->alias);

  /* Return TRUE only if both writes succeed */

  return write(huf->fdbbs, (char *)pusr, sizeof *pusr) == sizeof *pusr &&
         write(huf->fdndx, (char *)&usrndx, sizeof(USRNDX)) == sizeof(USRNDX);
}

/* Not supported - user file must be packed to delete user */

int _fast UserFileRemove(HUF huf, struct _usr *pusr)
{
  NW(huf);
  NW(pusr);

  return FALSE;
}


/* Close the user file */

int _fast UserFileClose(HUF huf)
{
  if (!huf || huf->id_huf != ID_HUF)
    return FALSE;

  if (huf->fdbbs != -1)
    close(huf->fdbbs);

  if (huf->fdndx != -1)
    close(huf->fdndx);

  memset(huf, 0, sizeof huf);
  free(huf);
  return TRUE;
}


