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

#include <stddef.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include "prog.h"
#include "max.h"
#include "prmapi.h"

static void near PrmAbort(char *pszFile, int iReason, int iErrorlevel)
{
  printf("Error!  The Maximus parameter file, '%s', cannot be accessed\n"
         "because %s.\n",
         pszFile,
         iReason==0 ? "there is not enough memory" :
         iReason==1 ? "the file cannot be opened" :
         iReason==2 ? "the file cannot be read" :
         iReason==3 ? "the file is of an invalid type" :
                      "of an unknown error.");

  exit(iErrorlevel);

}

HPRM PrmFileOpen(char *pszFile, int iAbortOnError)
{
  char pszName[PATHLEN];
  int fd;           /* descriptor for .prm file */
  long lSize;       /* file size */
  size_t stHeap;    /* size of variable-length heap */
  HPRM hp;          /* handle to be returned to caller */


  if ((hp = malloc(sizeof *hp))==NULL)
  {
    if (iAbortOnError)
      PrmAbort(pszFile, 0, iAbortOnError);
    else
      return NULL;
  }

  if ((fd=shopen(pszFile, O_RDONLY | O_BINARY))==-1)
  {
    /* Try adding a default .prm extension */

    strcpy(pszName, pszFile);
    strcat(pszName, ".prm");

    if ((fd=shopen(pszName, O_RDONLY | O_BINARY))==-1)
    {
      if (iAbortOnError)
        PrmAbort(pszFile, 1, iAbortOnError);
      else
      {
        free(hp);
        return NULL;
      }
    }
  }

  if (read(fd, (char *)&hp->mp, sizeof hp->mp) != sizeof hp->mp)
  {
    if (iAbortOnError)
      PrmAbort(pszFile, 2, iAbortOnError);
    else
    {
      close(fd);
      free(hp);
      return NULL;
    }
  }

  if (hp->mp.id != 'M' || hp->mp.version != CTL_VER ||
      hp->mp.heap_offset != sizeof(struct m_pointers))
  {
    if (iAbortOnError)
      PrmAbort(pszFile, 3, iAbortOnError);
    else
    {
      close(fd);
      free(hp);
      return NULL;
    }
  }

  /* Figure out the size of the variable-length heap */

  lSize = lseek(fd, 0L, SEEK_END);
  stHeap = (size_t)(lSize - (long)sizeof(struct m_pointers));
  lseek(fd, (long)sizeof(struct m_pointers), SEEK_SET);

  if ((hp->pszHeap = malloc(stHeap))==NULL)
  {
    if (iAbortOnError)
      PrmAbort(pszFile, 0, iAbortOnError);
    else
    {
      close(fd);
      free(hp);
      return NULL;
    }
  }

  /* Read in the variable-length heap */

  if (read(fd, hp->pszHeap, stHeap) != stHeap)
  {
    if (iAbortOnError)
      PrmAbort(pszFile, 2, iAbortOnError);
    else
    {
      close(fd);
      free(hp);
      return NULL;
    }
  }

  close(fd);

  return hp;
}


void PrmFileClose(HPRM hp)
{
  free(hp->pszHeap);
  free(hp);
}


/* Make a string relative to the PRM file's system path */

char * PrmRelativeString(HPRM hp, char *pszIn, char *pszOut)
{
  char *pszSysPath = PrmFileString(hp, sys_path);

  /* The language path may be relative, so we have to resolve it
   * relative to the system path.  On the flip side, Max is always
   * guaranteed to be in the \Max directory, so it doesn't have
   * this problem.
   */

  if (pszIn[1] != ':' && pszIn[0] != '\\' && pszIn[0] != '/')
  {
    strcpy(pszOut, pszSysPath);
    strcat(pszOut, pszIn);
  }
  else
  {
    strcpy(pszOut, pszIn);
  }

  return pszOut;
}

