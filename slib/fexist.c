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

/*# name=File-exist and directory-searching routines
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
# include <sys/types.h>
# include <sys/stat.h>
# include <unistd.h>
#endif

#if (defined(__MSDOS__) || defined(OS_2) || defined(NT)) && !defined(__IBMC__)
#include <dos.h>
#endif

#include "ffind.h"
#include "prog.h"


/*
main()
{
  printf("asdfe=%d\n",direxist("c:\\asdfe"));
  printf("blank=%d\n",direxist("c:\\blank"));
  printf("tc=%d\n",direxist("c:\\tc"));
  printf("c:\\=%d\n",direxist("c:\\"));
  printf("d:\\=%d\n",direxist("d:\\"));
  printf("e:\\=%d\n",direxist("e:\\"));
  printf("f:\\=%d\n",direxist("f:\\"));
}
*/


int _fast fexist(char *filename)
{
#ifdef UNIX
  if (strchr(filename, '?') || strchr(filename, '*'))
  {
#endif
    FFIND *ff;

    ff=FindOpen(filename,0);

    if (ff)
    {
      FindClose(ff);
      return TRUE;
    }
    else return FALSE;
#ifdef UNIX
  }
  else
  {
    struct stat	sb;
    char 	*fndup = fixPathDup(filename);
    int 	i;

    i = stat(fndup, &sb);
    fixPathDupFree(filename, fndup);

    if (i)
      return FALSE;

    return TRUE;
  }
#endif
}

long _fast fsize(char *filename)
{
#ifdef UNIX
  if (strchr(filename, '?') || strchr(filename, '*'))
  {
#endif
    FFIND *ff;
    long ret=-1L;

    ff=FindOpen(filename, 0);

    if (ff)
    {
      ret=ff->ulSize;
      FindClose(ff);
    }

    return ret;
#ifdef UNIX
  }
  else
  {
    struct stat	sb;
    char 	*fndup = fixPathDup(filename);
    int 	i;

    i = stat(fndup, &sb);
    fixPathDupFree(filename, fndup);

    if (i)
      return -1;

    return sb.st_size;
  }
#endif
}

#if defined(__MSDOS__)
  int _fast direxist(char *directory)
  {
    FFIND *ff;
    char *tempstr;
    int ret;

    if ((tempstr=(char *)malloc(strlen(directory)+5))==NULL)
      return FALSE;

    strcpy(tempstr, directory);

    Add_Trailing(tempstr,'\\');

    /* Root directory of any drive always exists! */

    if ((isalpha(tempstr[0]) && tempstr[1]==':' &&
        ((tempstr[2]=='\0') ||
         (tempstr[2]=='\\' || tempstr[2]=='/') && tempstr[3]=='\0')) ||
        eqstri(tempstr, "\\"))
    {
      ret=TRUE;
    }
    else
    {
      Strip_Trailing(tempstr, '\\');

      ff=FindOpen(tempstr, ATTR_SUBDIR | ATTR_HIDDEN | ATTR_READONLY);

      ret=(ff != NULL && (ff->usAttr & ATTR_SUBDIR));

      if (ff)
        FindClose(ff);
    }

    free(tempstr);
    return ret;

  }
#else
  #include "uni.h"

  int _fast direxist(char *directory)
  {
    int ret;
    char *tempstr;
    size_t l;

#ifndef UNIX
    if (NULL == (tempstr=(char *)strdup(directory)))
      return FALSE;
#else
    if (!directory)
      return FALSE;

    tempstr = fixPathDup(directory);
    if ((tempstr == directory) || (tempstr == (directory + 2)))
      tempstr = strdup(directory);
#endif

    /* Root directory of any drive always exists! */

#ifdef UNIX
    if (eqstr(tempstr, "/"))
#else
    if ((isalpha(tempstr[0]) && tempstr[1]==':' &&
        (tempstr[2]=='\\' || tempstr[2]=='/') && !tempstr[3]) ||
        eqstr(tempstr, "\\"))
#endif
    {
      free(tempstr);
      return TRUE;
    }

    l = strlen(tempstr);
    if( tempstr[l-1] == '\\' || tempstr[l-1] == '/')
      tempstr[l-1] = 0;           /* remove trailing backslash */

    ret = !access(tempstr, 0);

    free(tempstr);
    return ret;
  }
#endif

