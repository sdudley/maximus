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

/*# name=Portable file-searching hooks
    credit=Thanks go to Peter Fitzsimmons for these routines.
*/

#ifdef UNIX
# ifdef SOLARIS
#  define __EXTENSIONS__
# endif
# include <sys/types.h>
# include <time.h>
# include <sys/stat.h>
# include <unistd.h>
# include <ctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef UNIX
# include <sys/stat.h>
# include <unistd.h>
# include <ctype.h>
#endif
#include "compiler.h"

#ifndef __IBMC__
#include <dos.h>
#endif

#ifdef __TURBOC__
#include <dir.h>
#endif

#include "ffind.h"

#if defined(OS_2)
  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  #ifdef __FLAT__
    static void near CopyFBUF2FF(FFIND *ff, FILEFINDBUF3 *findbuf)
  #else
    static void near CopyFBUF2FF(FFIND *ff, FILEFINDBUF *findbuf)
  #endif
  {
    ff->usAttr=findbuf->attrFile;
    ff->ulSize=findbuf->cbFile;

    ff->scWdate.dos_st.time=*((USHORT *)&findbuf->ftimeLastWrite);
    ff->scWdate.dos_st.date=*((USHORT *)&findbuf->fdateLastWrite);

    ff->scCdate.dos_st.time=*((USHORT *)&findbuf->ftimeCreation);
    ff->scCdate.dos_st.date=*((USHORT *)&findbuf->fdateCreation);

    ff->scAdate.dos_st.time=*((USHORT *)&findbuf->ftimeLastAccess);
    ff->scAdate.dos_st.date=*((USHORT *)&findbuf->fdateLastAccess);

    if (!ff->scCdate.ldate)
      ff->scCdate.ldate=ff->scWdate.ldate;

    if (!ff->scAdate.ldate)
      ff->scAdate.ldate=ff->scWdate.ldate;

    strncpy(ff->szName, findbuf->achName, sizeof(ff->szName));
  }

  /* Find first file in list */

  FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
  {
    FFIND *ff;

    ff=malloc(sizeof(FFIND));

    if (ff)
    {
      #ifdef __FLAT__ /* OS/2 2.0 */
        ULONG usSearchCount=1;
        FILEFINDBUF3 findbuf;
      #else
        USHORT usSearchCount = 1;
        FILEFINDBUF findbuf;
      #endif


      ff->hdir=HDIR_CREATE;

      #ifdef __FLAT__
        if (DosFindFirst(filespec, &ff->hdir, attribute, &findbuf,
                         sizeof findbuf, &usSearchCount, FIL_STANDARD)==0)
      #else
        if (DosFindFirst(filespec, &ff->hdir, attribute, &findbuf,
                         sizeof findbuf, &usSearchCount, 0L)==0)
      #endif
        {
          CopyFBUF2FF(ff, &findbuf);
        }
        else
        {
          free(ff);
          ff=NULL;
        }
    }

    return ff;
  }

  int _fast FindNext(FFIND *ff)
  {
    int rc=-1;

    if (ff)
    {
      #ifdef __FLAT__
        ULONG usSearchCount=1;
        FILEFINDBUF3 findbuf;
      #else
        USHORT usSearchCount=1;
        FILEFINDBUF findbuf;
      #endif

      if (ff->hdir &&
          DosFindNext(ff->hdir, &findbuf, sizeof findbuf, &usSearchCount)==0)
      {
        CopyFBUF2FF(ff, &findbuf);
        rc=0;
      }
    }

    return rc;
  }

  void _fast FindClose(FFIND *ff)
  {
    if (ff)
    {
      if (ff->hdir)
        DosFindClose(ff->hdir);

      free(ff);
    }
  }

  /* This function was added because it is SIGNIFICANTLY faster under OS/2 to
   * call DosQPathInfo() rather than DosFindFirst(),   if all you are
   * intested in is getting a specific file's date/time/size.
   *
   *PLF Thu  10-17-1991  18:12:37
   */

  FFIND * _fast FindInfo(char *filespec)
  {
    FFIND *ff;
    FILESTATUS fs;
    char *f;

    ff = malloc(sizeof(*ff));
    if(!ff)
        return NULL;

    memset(ff, 0, sizeof(*ff));

  #ifdef __FLAT__
    if (DosQueryPathInfo(filespec, FIL_STANDARD, (PBYTE)&fs, sizeof(fs))==0)
  #else
    if (DosQPathInfo(filespec, FIL_STANDARD, (PBYTE)&fs, sizeof(fs), 0L)==0)
  #endif
    {
      ff->usAttr=fs.attrFile;
      ff->ulSize=fs.cbFile;

      ff->scWdate.dos_st.time=*((USHORT *)&fs.ftimeLastWrite);
      ff->scWdate.dos_st.date=*((USHORT *)&fs.fdateLastWrite);

      ff->scCdate.dos_st.time=*((USHORT *)&fs.ftimeCreation);
      ff->scCdate.dos_st.date=*((USHORT *)&fs.fdateCreation);

      ff->scAdate.dos_st.time=*((USHORT *)&fs.ftimeLastAccess);
      ff->scAdate.dos_st.date=*((USHORT *)&fs.fdateLastAccess);



      if ((f=strrchr(filespec, '\\'))==NULL)
        f=filespec;
      else f++;

      strncpy(ff->szName, f, sizeof(ff->szName));
    }
    else
    {
      free(ff);
      return NULL;
    }

    return ff;
  }

#elif defined(NT)

  static void near CopyWFD2FF(FFIND *ff, WIN32_FIND_DATA *pwfd)
  {
    ff->usAttr=pwfd->dwFileAttributes;
    ff->ulSize=pwfd->nFileSizeLow;

    FileTimeToDosDateTime(&pwfd->ftLastWriteTime,
                          &ff->scWdate.dos_st.date,
                          &ff->scWdate.dos_st.time);

    FileTimeToDosDateTime(&pwfd->ftCreationTime,
                          &ff->scCdate.dos_st.date,
                          &ff->scCdate.dos_st.time);

    FileTimeToDosDateTime(&pwfd->ftLastAccessTime,
                          &ff->scAdate.dos_st.date,
                          &ff->scAdate.dos_st.time);

    strncpy(ff->szName, pwfd->cFileName, sizeof(ff->szName));
  }


  int attrmatch(unsigned int required, unsigned int actual)
  {
    if ((actual & ATTR_SYSTEM) && (required & ATTR_SYSTEM)==0)
      return FALSE;

    if ((actual & ATTR_HIDDEN) && (required & ATTR_HIDDEN)==0)
      return FALSE;

    if ((actual & ATTR_SUBDIR) && (required & ATTR_SUBDIR)==0)
      return FALSE;

    return TRUE;
  }



  FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
  {
    FFIND *ff;

    NW(attribute);

    ff=malloc(sizeof(FFIND));

    if (ff)
    {
      WIN32_FIND_DATA wfd;

      ff->uiAttrSearch = attribute;

      if ((ff->hdir=FindFirstFile(filespec, &wfd)) != INVALID_HANDLE_VALUE)
      {
        CopyWFD2FF(ff, &wfd);

        /* If the file attribute did NOT match... */

        if (!attrmatch(ff->uiAttrSearch, ff->usAttr))
        {
          int rc;

          do
          {
            rc = FindNextFile(ff->hdir, &wfd);
            CopyWFD2FF(ff, &wfd);
          }
          while (rc && !attrmatch(ff->uiAttrSearch, ff->usAttr));

          if (!rc)
          {
            free(ff);
            ff=NULL;
          }
        }
      }
      else
      {
        free(ff);
        ff=NULL;
      }
    }

    return ff;
  }

  int _fast FindNext(FFIND *ff)
  {
    int rc=-1;

    if (ff)
    {
      WIN32_FIND_DATA wfd;

      if (ff->hdir != INVALID_HANDLE_VALUE)
      {
        int findrc;

        findrc = FindNextFile(ff->hdir, &wfd);
        CopyWFD2FF(ff, &wfd);

        while (findrc && !attrmatch(ff->uiAttrSearch, ff->usAttr))
        {
          findrc = FindNextFile(ff->hdir, &wfd);
          CopyWFD2FF(ff, &wfd);
        }

        if (findrc)
          rc=0;
      }
    }

    return rc;
  }

  void _fast FndClose(FFIND *ff)
  {
    if (ff)
    {
      #undef FindClose

      if (ff->hdir != INVALID_HANDLE_VALUE)
        FindClose(ff->hdir);

      free(ff);
    }
  }

  FFIND * _fast FindInfo(char *filespec)
  {
    return FindOpen(filespec, 0);
  }

#elif defined(__MSDOS__)

  static void near CopyDTA2FF(FFIND *ff)
  {
    ff->usAttr=(word)ff->__dta.bAttr;

    ff->scCdate.dos_st.time=ff->__dta.usTime;
    ff->scCdate.dos_st.date=ff->__dta.usDate;

    /* Copy dates into other structure too */

    ff->scAdate=ff->scWdate=ff->scCdate;
    ff->ulSize=ff->__dta.ulSize;

    memset(ff->szName, '\0', sizeof(ff->szName));
    memmove(ff->szName, ff->__dta.achName, sizeof(ff->szName));
    strupr(ff->szName);
  }

  FFIND * _fast FindOpen(char *filespec, unsigned short attribute)
  {
    FFIND *ff;

    ff=malloc(sizeof(FFIND));

    if (ff)
    {
      if (__dfindfirst(filespec, attribute, &ff->__dta)==0)
        CopyDTA2FF(ff);
      else
      {
        free(ff);
        ff=NULL;
      }
    }

    return ff;
  }


  int _fast FindNext(FFIND *ff)
  {
    int rc=-1;

    if (ff)
    {
      if ((rc=__dfindnext(&ff->__dta))==0)
        CopyDTA2FF(ff);
    }

    return rc;
  }

  void _fast FindClose(FFIND *ff)
  {
    if (ff)
      free(ff);
  }

  FFIND * _fast FindInfo(char *filespec)
  {
    return FindOpen(filespec, 0);
  }


#elif UNIX
/* Implementations in here by Wes. Just guessing exactly what needs
 * to be exported from above, and how it works :?. Will probably need
 * to fix this later once I've read through more than just the function
 * prototypes and comments....
 *
 * Looks like this is sort of like BSD's FTS code, grabbing file
 * names through glob expansion and stat attributes while its at it.
 *
 * WARNING WARNING WARNING
 *
 * #define trickery in the header lets us use GNU libc glob when available,
 * or regular glob on non-GNU libc systems. DO NOT CHANGE THIS CODE 
 * unless you understand EXACTLY how it works.
 */

static int populateFF(FFIND *ff, const char *filename, struct stat *sb_p)
{
  struct stat sb;
  struct tm   timebuf;
  const char *basename;

  basename = strrchr(filename, '/');
  if (basename)
    basename++;
  else
    basename = filename;

  if (sb_p)
    sb = *sb_p;
  else
  {
    if (stat(filename, &sb))
      return 1; /* Failure - pretend we never saw it */
  }

  if (ff->uiAttrSearch & ATTR_READONLY)
  {
    if (sb.st_mode & 0222) /* Somebody can write it -- it's not "read only" [yuck] */
      return 1; /* Do we need more granularity? I doubt it, since it works under DOS */
  }

  /* Populate all the bits of the FF structure as best we can. */

  TmDate_to_DosDate(localtime_r(&sb.st_ctime, &timebuf), &ff->scCdate); /* C = creation? last change? */
  TmDate_to_DosDate(localtime_r(&sb.st_atime, &timebuf), &ff->scAdate); /* A = last access? */
  TmDate_to_DosDate(localtime_r(&sb.st_mtime, &timebuf), &ff->scWdate); /* W = last write? */
  ff->ulSize = sb.st_size;
  strncpy(ff->szName, basename, sizeof(ff->szName));
  ff->szName[sizeof(ff->szName) - 1] = (char)0;

  return 0;
}

/** Find the next file in a FindOpen list, and populate
 *  the structure. From usage in ffind.c, it looks like
 *  we return 0 if we return a file's info. 
 */
int FindNext(FFIND *ff)
{
#if defined(FAKE_GLOB_ONLYDIR)
  struct stat sb;
#endif
  struct stat *sb_p = NULL;

  if (!ff || !ff->globInfo.gl_pathc)
    return 1; /* Nothing to find - called after FindInfo()? */

  /* Look for a file in the array which matches the attributes */
  for (; ff->globNext < ff->globInfo.gl_pathc; ff->globNext++)
  {
#if defined(FAKE_GLOB_ONLYDIR)
    /* Fake GLOB_ONLYDIR by ignoring non-directories */
    if (ff->globFlags & GLOB_ONLYDIR)
    {
      if (stat(ff->globInfo.gl_pathv[ff->globNext], &sb) != 0)
	continue;

      if (!S_ISDIR(sb.st_mode))
	continue;

      sb_p = &sb;
    }
#endif
#if defined(FAKE_GLOB_PERIOD)
    /* Fake GLOB_PERIOD by refusing to find filenames with
     * dots in them if the wildcard itself did not have one.
     */
    if ((ff->globFlags & GLOB_PERIOD) && !strchr(ff->globExpr, '.'))
      if (strchr(ff->globInfo.gl_pathv[ff->globNext], '.'))
	continue;
#endif
    if (populateFF(ff, ff->globInfo.gl_pathv[ff->globNext++], sb_p) == 0)
      return 0; /* Got one */
  }

  return 1; /* All Done */
}

FFIND *FindOpen(char *filespec, unsigned short attribute)
{
  /* I think this function is basically "init" - e.g. handle
   * constructor. So we set stuff up for FindInfo... big
   * questions include what the heck to do with attribute.
   * I think I'll use unix perms for ATTR_READONLY, ignore
   * ATTR_HIDDEN and ATTR_SYSTEM -- maybe we could use
   * "owned by root" for ATTR_SYSTEM someday? 
   * Usage in fexist.c suggests ATTR_SUBDIR means "only
   * find directories", so we'll set that up with the
   * glob flags. We'll always traverse directories.
   */

  FFIND *ff;
  char	unix_filespec[1024];
  char  *d /*dos*/, *u /*unix*/;

  ff = calloc(sizeof(*ff), 1);
  if (!ff)
    return NULL;

  ff->globFlags = GLOB_NOSORT | GLOB_NOESCAPE | GLOB_PERIOD; /* Get as close to DOS wildcarding as possible */
  if (attribute & ATTR_SUBDIR)
    ff->globFlags |= GLOB_ONLYDIR;

  /* Need to add case-insensitivity, etc, to make glob work the same way DOS does */
  d = filespec;
  u = unix_filespec;

  if ((d[0] == '.') && !strchr(d, '/') && (strlen(d) < 5))
    *u++ = '*';

  if (d[0] == '\0')
    *u++ = '*';

  do
  {
    switch(*d)
    {
      case '\\':
        *u++ = '/';
        break;
      case 'a'...'z':
      case 'A'...'Z':
        *u++ = '[';
        *u++ = tolower((int)*d);
        *u++ = toupper((int)*d);
        *u++ = ']';
        break;
      default:
        *u++ = *d;      
    }
  }
  while (*d++ && ((sizeof(unix_filespec) - (u - unix_filespec)) > 10));
  *u = (char)0;

  if (u > unix_filespec)
  {
    if (u[-1] == '.')
      u[-1] = (char)0; /* trailing dot actually means "no extension" in DOS */
  }  

#if defined(FAKE_GLOB_PERIOD)
  strncpy(ff->globExpr, unix_filespec, sizeof(ff->globExpr));
  ff->globExpr[sizeof(ff->globExpr) - 1] = (char)0;
#endif

  if (glob(unix_filespec, ff->globFlags & GLOB_FLAGS_MASK, NULL, &(ff->globInfo)))
  {
    free(ff);
    return NULL;
  }

  /* Usage suggest we also return one file's info
   * with this puppy, so quickly run FindNext to
   * fudge this before returning..
   */

  if (FindNext(ff) == 0)
    return ff;      

  free(ff);
  return NULL;
}

FFIND *FindInfo(char *filespec)
{
  /* This appears to be a non-globbing version of find open,
   * which (obviously) only finds one file.
   */

  FFIND *ff;

  ff = calloc(sizeof(*ff), 1);
  if (!ff)
    return NULL;

  if (populateFF(ff, filespec, NULL))
  {
    free(ff);
    return NULL;
  }

  return ff;
}

void FindClose(FFIND *ff)
{
  /* Clean up after FindOpen or FindInfo */

  if (!ff)
    return;

  if (ff->globInfo.gl_pathv)
    globfree(&ff->globInfo);
  
  free(ff);
}

#else
  #error Unknown OS
#endif




#ifdef TEST_SHELL

#define TRUE 1
#define FALSE 0

int walk(char *path);

void main(int argc, char **argv)
{
    walk(PATH_DELIMS);     /* start at root*/
}

/* this simple function assumes the path ALWAYS has an ending backslash */
walk(char *path)
{
  /* quicks hacks by wes. moving hard-coded strings
   * to match bits out of prog.h. Probably doesn't
   * work, but should break under NT or OS/2.. I guess
   * we'll know once its time to test.
   */

    FFIND *ff;
    int done = FALSE;
    char full[66];

    strcpy(full, path);
    strcat(full, WILDCARD_ALL);


    if( ff = FindOpen(full, ATTR_SUBDIR) ){

        for(done = FALSE; !done; done = FindNext(ff)){
            if( (ff->usAttr & ATTR_SUBDIR) && (ff->szName[0] != '.') ){
                strcpy(full, path);
                strcat(full, ff->szName);
                puts(full);

                {
                  char temp[120];
                  FFIND *f;

                  strcpy(temp, full);
                  strcat(temp, PATH_DELIMS WILDCARD_ALL);


                  if ((f=FindOpen(temp, 0)) != NULL)
                  {
                    do
                    {
                      printf("\t%s\n", f->szName);
                    }
                    while (FindNext(f)==0);

                    FindClose(f);
                  }

                }

                strcat(full, PATH_DELIMS);
                if( !walk(full) )
                    return(FALSE);
            }
        }
        FindClose(ff);
        return(TRUE);
    }
    else{
        puts("FindOpen() failed");
    }
    return(FALSE);
}

#endif

