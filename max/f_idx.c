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
static char rcs_id[]="$Id: f_idx.c,v 1.1 2002/10/01 17:51:04 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Routines for accessing the compiled file database
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max_file.h"
#include "f_idx.h"
#include "fb.h"


static void near fext(char *name, PFAH pfah, char *ext)
{
  char *dot;

  if (*FAS(*pfah, filesbbs))
    strcpy(name, FAS(*pfah, filesbbs));
  else
  {
    strcpy(name, FAS(*pfah, downpath));
    strcat(name, filesdat_name);
  }

  if ((dot=strrchr(name, '.')) != NULL)
    *dot='\0';

  strcat(name, ext);
}
    
 
IDXF * IndexOpen(char *pat)
{
  IDXF *ix;
  char idxname[PATHLEN];
  char *star, *quest;

  if ((ix=malloc(sizeof(IDXF)))==NULL)
    return NULL;

  memset(ix, '\0', sizeof(IDXF));

  strcpy(idxname, PRM(sys_path));
  strcat(idxname, master_idx);
  
  if ((ix->ifd=shopen(idxname, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    free(ix);
    return NULL;
  }
  
  ix->hi=lseek(ix->ifd, 0L, SEEK_END);
  
  if (ix->hi==-1 || (ix->hi /= sizeof(FIDX)) <= 0)
  {
    close(ix->ifd);
    free(ix);
    return NULL;
  }
  
  ix->lo=0L;
  ix->last=-1L;
  ix->next_found_ofs=-1L;


  /* If the filename contains wildcards... */

  star=strchr(pat, '*');
  quest=strchr(pat, '?');

  if (star || quest)
  {
    /* Find the length of the pattern, up to the FIRST wildcard character.  *
     * This ensures that we don't try to match any of the wildcards with a  *
     * real filename character.                                             */

    if (star && (quest==NULL || quest > star))
      ix->pat_len=(star-pat);
    else ix->pat_len=(quest-pat);
  }
  else
  {
    /* No wildcard patch, so try to compare the whole filename */
    ix->pat_len=MAX_FN_LEN;
  }

  if ((ix->pat=strdup(pat))==NULL || IndexNext(ix) != 0)
  {
    IndexClose(ix);
    return NULL;
  }

  return ix;
}


int IndexNext(IDXF *ix)
{
  char tempfn[MAX_FN_LEN+1];
  long seekto, tryit;
  int result;

  upper_fn(ix->pat);

  /* If we're resuming a prior search, seek to where we last left off. */
  
  while (! ix->found)
  {
    /* Binary search - start in the middle of the high and the low */
    
    tryit=((ix->hi-ix->lo) >> 1) + ix->lo;
    
    /* Don't search the same entry swice */
    
    if (ix->last==tryit)
      break;
    
    ix->last=tryit;
    
    /* Seek to the 'try' record number */
    
    seekto=tryit*sizeof(FIDX);
    
    if (lseek(ix->ifd,seekto,SEEK_SET) != seekto)
      break;
    
    if (read(ix->ifd,(char *)&ix->fidx,sizeof(FIDX))
                                             != sizeof(FIDX))
      break;


    /* See if the pattern is the same as the name */
    
    result=strncmp(ix->pat, ix->fidx.name, ix->pat_len);
    
    /* If we found a match... */

    if (result==0)
    {
      /* If it wasn't an exact match for the FULL pattern length, scan      *
       * back until we find the beginning of these matches...               */

      if (!eqstrin(ix->fidx.name, ix->pat, MAX_FN_LEN))
      {
        if (tell(ix->ifd)-(long)(sizeof(FIDX)*2) >= 0L)
        {
          lseek(ix->ifd, -(long)(sizeof(FIDX)*2), SEEK_CUR);

          while (tell(ix->ifd) > 0L)
          {
            if (read(ix->ifd, (char *)&ix->fidx, sizeof(FIDX)) 
                                                    != sizeof(FIDX))
            {
              break;
            }

            /* If the pattern does NOT match, then we just found the        *
             * match in front of the pattern, and hence the start of        *
             * the wildcard list.  We've got it now, so seek back to the    *
             * start of the list and begin processing...                    */

            if (!eqstrin(ix->fidx.name, ix->pat, ix->pat_len))
            {
              /* Read in the right index entry */
              read(ix->ifd, (char *)&ix->fidx, sizeof(FIDX));
              break;
            }

            /* Seek back two records */

            if (tell(ix->ifd)-(long)(sizeof(FIDX)*2) < 0L)
              break;

            lseek(ix->ifd,-(long)(sizeof(FIDX)*2),SEEK_CUR);
          }
        }
      }
      
      /* If we got this far, then we found a probable match, so get out     *
       * of loop and perform a linear search.                               */
      
      ix->found=TRUE;
      ix->next_found_ofs = tell(ix->ifd) - (long)sizeof(FIDX);

      if (ix->next_found_ofs < 0L)
        ix->next_found_ofs=0L;
    }
    else if (result < 0)
      ix->hi=tryit;
    else ix->lo=tryit;
  }

  if (ix->found && ix->next_found_ofs != -1L)
  {
    /* Seek back to the beginning of this one */

    lseek(ix->ifd, ix->next_found_ofs, SEEK_SET);


    /* Scan through all of the file names AS LONG AS THE FIRST pat_len    *
     * bytes match.  Since the wildcard stuff will only be matched to     *
     * a 'pat_len' length, this stuff is ensured to work.                 */

    while (read(ix->ifd, (char *)&ix->fidx, sizeof(FIDX))
                                               ==sizeof(FIDX) &&
           (!ix->pat_len || strnicmp(ix->pat, ix->fidx.name, ix->pat_len)==0))
    {
      /* Copy string to a temp buffer to nul-terminate it */

      memmove(tempfn, ix->fidx.name, MAX_FN_LEN);
      tempfn[MAX_FN_LEN]='\0';

      if (MatchWC(tempfn, ix->pat))
      {
        ix->found=TRUE;
        ix->next_found_ofs=tell(ix->ifd);
        return 0;
      }
    }
  }

  /* Not found, so set the flag appropriately */

  ix->next_found_ofs=-1L;
  ix->found=TRUE;
  return -1;
}


void IndexClose(IDXF *ix)
{
  if (ix)
  {
    close(ix->ifd);
    
    if (ix->pat)
    {
      free(ix->pat);
      ix->pat=NULL;
    }
    
    free(ix);
  }
}





/* Given a _fidx structure, find the appropriate area it references,        *
 * with priv-level checking et al, find the record in FILES.DAT,            *
 * and return TRUE if everything was okay.  (The _fdat record is            *
 * then placed in the struct pointed to by fdat.)                           */

word FidxIsOkay(FIDX *fidx, FDAT *fdat, char *name, char *path, word check_cur,
                word check_priv, PFAH pfahIn)
{
  BARINFO bi;
  FAH fa;
  HAFF haff;
  PFAH pfah;
  char temp[PATHLEN];
  int ret, dfd;
  long ofs;

  memset(&fa, 0, sizeof fa);
  pfah=pfahIn ? pfahIn : &fa;
  
  /* Attempt to find the specified file area number */

  if ((haff=AreaFileFindOpen(haf, NULL, 0))==NULL)
    return FALSE;

  ret=AreaFileFindSeek(haff, pfah, (unsigned)fidx->anum);
  AreaFileFindClose(haff);

  if (ret != 0)
    return FALSE;

  /* If the area is invalid, the barricade is wrong, or if                  *
   * it's the current area, don't tag this one.  (If it's the               *
   * current area, we should have already picked this one up when           *
   * searching files.dat or files.bbs.)                                     */

  if (!ValidFileArea(NULL, pfah, VA_VAL | VA_PWD | VA_EXTONLY, &bi) ||
      (eqstri(FAS(fah, name), PFAS(pfah, name)) && !check_cur) ||
      !CanAccessFileCommand(pfah, file_tag, 0, &bi))
  {
    /* Only check the priv level if necessary */

    if (check_priv)
    {
      if (!pfahIn)
        DisposeFah(&fa);

      return FALSE;
    }
  }

  /* Figure out the nae of the FILES.DAT file to open */

  fext(temp, pfah, dot_dat);

  /* Copy the name of the area into 'name' */

  if (name)
    strcpy(name, PFAS(pfah, name));
  
  if (path)
    strcpy(path, PFAS(pfah, downpath));
  

  if ((dfd=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    if (!pfahIn)
      DisposeFah(&fa);

    return FALSE;
  }

  ofs=fidx->fpos*sizeof(FDAT);
  
  ret=(lseek(dfd, ofs, SEEK_SET)==ofs &&
       read(dfd, (char *)fdat, sizeof(FDAT))==sizeof(FDAT));
     
  if (!eqstrni(fdat->name, fidx->name, MAX_FN_LEN))
  {
    ret=FALSE;
    logit(log_maxfiles_sync);
  }


  /* Don't search for deleted files or for comments */

  if (fdat->flag & (FF_DELETED|FF_COMMENT))
    ret=FALSE;

  close(dfd);

  /* If there's a path for this file, read it from the heap */

  if (((path && fdat->path) || check_priv) && ret)
  {
    word len;

    /* Open the .DMP file */

    fext(temp, pfah, dot_dmp);
    
    if ((dfd=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
      ret=FALSE;
    else
    {
      /* Check the priv level req'd to access file against user's priv */

      if (check_priv)
      {
        char *acs;

        /* Read ACS into memory */

        lseek(dfd, fdat->acs, SEEK_SET);

        if (read(dfd, (char *)&len, sizeof(word)) != sizeof(word))
          ret=FALSE;

        if ((acs=malloc(len))==NULL)
        {
          logit(mem_none);
          ret=FALSE;
        }
        else
        {
          /* Compare with user's access level */
          if (read(dfd, acs, len) != (signed)len ||
              !PrivOK(acs, FALSE))
          {
            ret=FALSE;
          }

          free(acs);
        }
      }

      if (path && fdat->path)
      {
        lseek(dfd, fdat->path, SEEK_SET);

        /* Read the length of the path */

        if (read(dfd, (char *)&len, sizeof(word)) != sizeof(word))
          ret=FALSE;

        /* Now read in the path of the file */

        if (len >= PATHLEN || read(dfd, path, len) != (signed)len)
          ret=FALSE;
      }

      /* Close the dump file */

      close(dfd);
    }
  }
  
  if (!pfahIn)
    DisposeFah(&fa);

  return ret;
}


/* Remove the entry for a particular file from the files database */

void RemoveFilesDat(PFAH pfah, char *fname)
{
  #define NUM_FDAT 16

  FDAT *fdbuf, *fdat, *fdend;
  char temp[PATHLEN];
  int got, fd, out=FALSE;
  
  fext(temp, pfah, dot_dat);
  
  if ((fd=shopen(temp, O_RDWR | O_BINARY | O_NOINHERIT))==-1)
    return;
  
  if ((fdbuf=malloc(sizeof(FDAT) * NUM_FDAT))==NULL)
  {
    close(fd);
    return;
  }

  /* Read in a chunk of file entries */
  
  while (!out && (got=read(fd, (char *)fdbuf, sizeof(FDAT) * NUM_FDAT)) > 0)
  {
    /* Now loop through all of the ones that we got */
    
    for (fdat=fdbuf, fdend=fdat+got/sizeof(FDAT); fdat < fdend; fdat++)
      if (eqstri(fdat->name, fname))
      {
        /* If we found a match, zero out the filename and rewrite the entry */
        
        *fdat->name='\0';
        fdat->flag |= FF_DELETED;

        lseek(fd, -(long)(fdend-fdat)*sizeof(FDAT), SEEK_CUR);
        write(fd, (char *)fdat, sizeof(FDAT));
        out=TRUE;
        break;
      }
  }
  
  free(fdbuf);
  close(fd);
}


