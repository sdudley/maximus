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
static char rcs_id[]="$Id: fb.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

#define NOVER
#define NOVARS
#define MAX_INCL_VER

#include <stdio.h>
#include <limits.h>
#include <stddef.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "ffind.h"
#include "bfile.h"
#include "max.h"
#include "fb.h"
#include "fbp.h"
#include "areaapi.h"
#include "prmapi.h"

#ifdef OS_2
  #define INCL_DOSMISC
  #include "pos2.h"
#endif

#define MAX_FBBS_LINE 1024


#define MERGE_1_SIZE    16384
#define MERGE_2_SIZE    16384
#define MERGE_OUT_SIZE  32768

static struct _finf *finf;        /* Array of buffered finf structs */
static int num_finf=0;            /* Number of _finf structs buffered */
static union stamp_combo scNow;   /* Current date */


static BFILE bbs, dat, dmp, idx;

static long ctr;

static char *filesbbs_name="files.bbs";

static char *fdat_ext=".dat";
static char *fidx_ext=".idx";
static char *fdmp_ext=".dmp";

static char *fbbs_delim=" \t\r\n";

static struct _alist *ap;
static word anum;



static void near NoSpace(void)
{
  printf("\nError writing to file!  (Out of disk space?)\n");
  exit(1);
}


static void near Usage(void)
{
  printf("Usage:    FB <switches> [<area> ...]\n\n"

         "<switches> may be one or more of:\n\n"

         "    -a        Compile all file areas.\n"
         "    -f<file>  Use <file> instead of the default FAREA file database\n"
         "    -p<file>  Use PRM file <file> instead of the MAXIMUS environment variable\n"
         "    -r        Force update from filesystem for manual date/size areas\n"
         "    -s        Skip areas marked as \"Type Slow\" or \"Type CD\"\n"
         "    -u        Process the upload paths for the specified file areas\n"
         "    -x        Do not perform the final merge to MAXFILES.IDX\n\n"

         "[<area> ...] is an optional list of areas to process.  The wildcard \"*\" may\n"
         "be used at the end of a name to match any number of characters.\n\n"

         "Examples:\n\n"

         "  fb -a               - Process all file areas\n"
         "  fb -s -a            - Process all areas except those on CD-ROM\n"
         "  fb 1 2 3 4          - Process file areas 1, 2, 3 and 4.\n"
         "  fb os2.* dos.*      - Process all areas starting with \"os2.\" or \"dos.\"\n");

  exit(1);
}

int _stdc main(int argc, char *argv[])
{
  FAH fah={0};                /* Current area being processed */
  char adat_name[PATHLEN];    /* Default file area data file to process */
  char master_idx[PATHLEN];   /* Path to maxfiles.idx */
  char *pszMaximus;           /* Name of default .prm file */
  char **ppszAreas;           /* Array of area names to process */
  int cAreas = 0;             /* Count of areas to process */
  HAF haf;                    /* Handle to a single file area */
  HAFF haff;                  /* Handle to file area database */
  struct _alist *last, *p;    /* Used for walking the list of areas */
  int ffsUpdate = FALSE;      /* Force manual date/size update from filesystem */
  int fUp = FALSE;            /* TRUE if we are to process the upload paths */
  int fDoSlow = TRUE;         /* TRUE if we are to skip slow areas */
  int fSkipMerge = FALSE;     /* TRUE if we are to skip the maxfiles merge */
  int cAreasProcessed = 0;    /* Number of areas processed so far */
  HPRM hp;                    /* Handle to .prm file */
  int i;                      /* Loop index */

#ifdef __FLAT__
  long lSize=0L;
#endif

  NW(argc);
  NW(argv);

  #ifdef DMALLOC
  dmalloc_on(0);
  #endif

  /* Disable "drive not found" errors */

  #ifdef OS_2
    #ifdef __FLAT__
      DosError(FERR_DISABLEHARDERR);
    #else
      DosError(HARDERROR_DISABLE);
    #endif
  #endif

  Hello("FB", "File Database Build Utility", VERSION, "1991, " THIS_YEAR);

  if ((finf=malloc(sizeof(struct _finf)*MAX_FINF))==NULL)
    NoMem();

  Get_Dos_Date(&scNow);

  install_24();
  atexit(uninstall_24);

  if ((ppszAreas = malloc(argc * sizeof(char *)))==NULL)
    NoMem();

  if (argc < 2)
    Usage();

  pszMaximus = GetMaximus(argc, argv, 1);

  hp = PrmFileOpen(pszMaximus, 1);
  strcpy(adat_name, PrmFileString(hp, farea_name));

  /* maxfiles.idx is always in the system path */

  strcpy(master_idx, PrmFileString(hp, sys_path));
  strcat(master_idx, "maxfiles.idx");

  /* Handle a different name for AREA.DAT */

  for (i=1; i < argc; i++)
  {
    if (*argv[i]=='?')
      Usage();
    else if (*argv[i] != '-' && *argv[i] != '/')
      ppszAreas[cAreas++] = argv[i];
    else
    {
      switch (tolower(argv[i][1]))
      {
        case 'a':   /* no action - default is to process all file areas */
          cAreas = 0;
          break;

        case 'f':
          strcpy(adat_name, argv[i]+2);
          break;

        case 'r':
          ffsUpdate = TRUE;
          break;

        case 's':
          fDoSlow = FALSE;
          break;

        case 'u':
          fUp = TRUE;
          break;

        case 'x':
          fSkipMerge = TRUE;
          break;

        case 'p': /* handled by GetMaximus */
          break;

        case '?':
          printf("Unknown command-line option '%s'\n", argv[i]);
          /* fall-through */

        default:
          Usage();
      }
    }
  }

  ppszAreas[cAreas] = (char *)0;

  if ((haf=AreaFileOpen(adat_name, FALSE))==NULL)
  {
    cant_open(adat_name);
    return EXIT_FAILURE;
  }

  if ((haff=AreaFileFindOpen(haf, NULL, AFFO_DIV))==NULL)
  {
    cant_open(adat_name);
    return EXIT_FAILURE;
  }

  ap=NULL;
  last=NULL;
  cAreasProcessed=0;

  /* Collect all of the file areas to be processed */

  for (anum=0; AreaFileFindNext(haff, &fah, FALSE)==0; anum++)
  {
    char szFilesBbs[PATHLEN];
    char szFilePath[PATHLEN];

    if (! *FAS(fah, downpath) || (fah.fa.attribs & (FA_DIVBEGIN|FA_DIVEND|FA_NOINDEX)))
      continue;

    if (fah.fa.filesbbs)
      PrmRelativeString(hp, FAS(fah, filesbbs), szFilesBbs);
    else
    {
      PrmRelativeString(hp, FAS(fah, downpath), szFilesBbs);
      strcat(szFilesBbs, filesbbs_name);
    }
        
    if (cAreas==0 || do_this_area(&fah, ppszAreas, fDoSlow))
    {
      char temp[PATHLEN];

      if (fDoSlow || (fah.fa.attribs & FA_SLOW)==0)
      {
        if (fUp)
        {
          PrmRelativeString(hp, FAS(fah, uppath), temp);
          strcat(temp, filesbbs_name);
        }
        else
          strcpy(temp, szFilesBbs);

        PrmRelativeString(hp,
                          fUp ? FAS(fah, uppath) : FAS(fah, downpath),
                          szFilePath);

        process_files_bbs(&fah,
                          temp,
                          szFilePath,
                          ffsUpdate);
      }
    }

    if (!fSkipMerge)
    {
      /* Now make a copy of the index filename */

      make_ext(szFilesBbs, fidx_ext);

      if ((p=malloc(sizeof(struct _alist)))==NULL)
        NoMem();

      #ifdef __FLAT__ /* For flat model, keep track of all index sizes */
      {
        long lFileSize=fsize(szFilesBbs);

        if (lFileSize > 0)
          lSize += lFileSize;
      }
      #endif

      p->path=strdup(szFilesBbs);
      p->anum=anum;
      p->size=-1L;

      last=p;
      p->next=ap;
      ap=p;
      cAreasProcessed++;
    }
  }

#ifdef __FLAT__
  /* For flat model, just qsort() everything in memory */

  if (cAreasProcessed)
    sort_index(master_idx, ap, lSize);
#else
  /* If we got any areas to do a global index on */

  if (last)
  {
    unlink(master_idx);
    
    /* Keep merging until we end up with only one list */

    if (ap && ap->next)
    {
      while (ap && ap->next)
        merge_lists(--cAreasProcessed);

      unlink(master_idx);

      if (*ap->path=='$')
        rename(ap->path, master_idx);
      else
        lcopy(ap->path, master_idx);
    }
    else
    {
      /* If there's only one file left, simply copy it (instead of          *
       * renaming, which would remove the original.                         */

      lcopy(ap->path, master_idx);
    }

    printf("\r                       \rDone!\n");
  }
#endif
  
  for (p=ap; p; last=p->next, free(p), p=last)
    if (p->path)
      free(p->path);

  AreaFileFindClose(haff);
  AreaFileClose(haf);

  free(ppszAreas);

  PrmFileClose(hp);
  free(finf);

  return EXIT_SUCCESS;
}



/* do_this_area:
 *
 * Returns TRUE if we are supposed to process the specified file
 * area.
 */

static word near do_this_area(PFAH pfah, char **ppszAreaList, int fDoSlow)
{
  int iLenStr;
  char **p;

  if (!fDoSlow && (pfah->fa.attribs & FA_SLOW))
    return FALSE;

  for (p=ppszAreaList; *p; p++)
  {
    iLenStr = strlen(*p);

    /* We should process the area if the names are equal, or if the wildcard
     * at the end of the string matches properly.
     */

    if (eqstri(PFAS(pfah, name), *p) ||
        ((*p)[iLenStr - 1] == '*' &&
         strnicmp(*p, PFAS(pfah, name), iLenStr-1)==0))
    {
      return TRUE;
    }
  }
    
  return FALSE;
}



/* Used for calling qsort to sort these files */

static int _stdc finf_comp(const void *v1, const void *v2)
{
  return (strncmp(((struct _finf *)v1)->name, 
                  ((struct _finf *)v2)->name, 12));
}



/* Gather all of the files in this area into an array */

static void near make_finf(char *path)
{
  FFIND *ff;
  struct _finf *fi, *e;
  
  if ((ff=FindOpen(path, 0))==NULL)
  {
    num_finf=0;
    return;
  }

  num_finf=0;
  fi=finf;
  e=finf+MAX_FINF;
  
  do
  {
    memset(fi->name, '\0', sizeof(fi->name));
    strncpy(fi->name, ff->szName, sizeof(fi->name));
    /* strupr(fi->name); */
    fancy_fn(fi->name);

    fi->cdate=ff->scCdate;
    fi->wdate=ff->scWdate;
    fi->size=ff->ulSize;

    num_finf++;
    fi++;
  }
  while (fi < e && FindNext(ff)==0);
  
  if (fi==e)      /* Couldn't fit all of them, so do it the slow way */
    num_finf=-1;
  else
    qsort(finf, num_finf, sizeof(struct _finf), finf_comp);

  FindClose(ff);
}

static char fbbs_line[MAX_FBBS_LINE];
static int push_line=0;

static char * nextfbbsline(char * buf)
{
  char *line;

  if (bbs==NULL)
    return NULL;
  line = (push_line) ? fbbs_line : Bgets(fbbs_line, MAX_FBBS_LINE, bbs);
  push_line=0;
  if (line && buf)
  {
    strcpy(buf, line);
    line=buf;
  }
  return line;
}


/* Process a files.bbs file for one area */

static int near process_files_bbs(PFAH pfah, char *fbbs, char *path, int ffsUpdate)
{
  char *line;
  char temp[PATHLEN];
  char fbbsbuf[MAX_FBBS_LINE];
  char ch;
  word n;
  
  printf("Processing %-40s     ",path);
  fflush(stdout);

  ctr=0L;

  if ((bbs=Bopen(fbbs, BO_RDONLY | BO_BINARY, BSH_DENYNO, 8192))==NULL)
  {
    printf("(no file list)\n");
    return -1;
  }
  
  if (!ffsUpdate && (pfah->fa.attribs & FA_LISTDATE))
    num_finf=-1;  /* Force no search if area has manual date/size */
  else
  {
    sprintf(temp, "%s" WILDCARD_ALL, path);
    make_finf(temp);
  }
  
  strcpy(temp, fbbs);
  make_ext(temp, fdat_ext);
  
  if ((dat=Bopen(temp, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY,
                 BSH_DENYNO, 4096))==NULL)
  {
    cant_open(temp);
    return -1;
  }

  strcpy(temp, fbbs);
  make_ext(temp, fdmp_ext);
  
  if ((dmp=Bopen(temp, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY, BSH_DENYNO, 4096))==NULL)
  {
    cant_open(temp);
    return -1;
  }

  ch=0;

  /* Write three NUL bytes to the beginning of the file */

  for (n=3; n--; )
    cBwrite(dmp, (char *)&ch, 1);

  strcpy(temp, fbbs);
  make_ext(temp, fidx_ext);
  
  if ((idx=Bopen(temp, BO_CREAT | BO_TRUNC | BO_RDWR | BO_BINARY,
                 BSH_DENYNO, 2048))==NULL)
  {
    cant_open(temp);
    return -1;
  }


  while ((line=nextfbbsline(fbbsbuf)) != NULL)
    if (process_line(pfah, line, path) < 0)
      break;

  printf("\b\b\b\b%04ld", ctr);
  fflush(stdout);

  sort_idx(path);

  Bclose(idx);
  Bclose(dmp);
  Bclose(dat);
  Bclose(bbs);

  return 0;
}



  /* Get file size from description */

static char * near descsize(char *psz, dword * plSize)
{
  dword lsize=0L;

  while (*psz==',' || isdigit(*psz))
  {
    if (*psz != ',')    /* Which lamebrain ever thought of *this* idea?? */
      lsize=(lsize*10L)+(*psz - '0');         /* Assumes ASCII of course */
    psz++;
  }

  while (*psz==' ')                                       /* Skip spaces */
    psz++;

  if (plSize)
    *plSize=lsize;
  return psz;
}

  /* Try to parse a (assume US format) date mm-dd-yy from description */

static char * near descdate(char *psz, union stamp_combo *stamp)
{
  int mon, day, year, temp;

  if ((mon=atoi(psz))!=0)
  {
    char *p =psz;

    while (*p && isdigit(*p++))
      ;

    if ((day=atoi(p))!=0)
    {

      while (*p && isdigit(*p++))
        ;

      year=atoi(p);

      while (*p && isdigit(*p++))
        ;

      /* Now, let's try to make sense of all this ... */

      if (day > 31 && year && year <= 31)   /* Probably yy-mm-dd */
        temp=year, year=day, day=temp;

      if (day && day < 32 && mon < 13)
      {
        if (stamp)
        {
          if (year < 80)  /* > 2000 A.D. */
            year += 100;
          else if (year > 1900)
            year -= 1900; /* Four digit year */

          stamp->msg_st.date.da=day;
          stamp->msg_st.date.mo=mon;
          stamp->msg_st.date.yr=(year-80);
        }

        while (*p==' ')
          p++;

        return p; /* Return ptr past the date */
      }
    }
  }

  /* No success */

  stamp->ldate=0; /* Zero the date */
  return psz;     /* Return unmodified ptr */
}


/* Process one line of FILES.BBS */

static int near process_line(PFAH pfah, byte *line, char *path)
{
  FDAT fdat;
  struct _finf *f, *e;
  int nofind;

  char buf[PATHLEN];
  char fname[PATHLEN];
  char *desc;
  char *filepath;
  char *nline;

  int haspath, found;

  
  /* Only process lines which contin file entries */

  if (*line <= ' ' || *line=='-' || *line >= 127)
    return 1;
  
  /* Remove trailing newline */

  Strip_Trailing(line, '\n');

  /* Examine following lines for any continution */

  while ((nline=nextfbbsline(NULL)) != NULL)
  {
    if (isspace(*nline) || *nline == '+')
    {
      /* Skip any leading spaces */

      char *p=nline + strspn(nline, " \t");

      if (*p == '|' || *p == '+')   /* Looks like a continuation line */
      {
        int llen=strlen(line);
        int nlen=strlen(p);

        if ((llen+nlen) < MAX_FBBS_LINE)    /* See if it will fit */
        {
          Strip_Trailing(p, '\n');

          if (!isspace(*++p))
            line[llen++]=' ';

          strcpy(line+llen, p);
          continue;
        }

        /* Just discard anything that won't fit */

      }
    }

    /* Push this back so the next fetch will get it */

    push_line=1;
    break;
  }

  /* Get the filename and description */

  getword(line, fname, fbbs_delim, 1);
  desc=firstchar(line, fbbs_delim, 2);
  
  /* Fill out the files.dat struct */

  memset(&fdat, '\0', sizeof(FDAT));
  fdat.struct_len=sizeof(FDAT);

  /* Find out if there's a path specification */

  filepath=strrstr(fname, "/\\");

  if (filepath)
  {
    /* If we have a path, copy just the raw filename to the fdat struct */

    strncpy(fdat.name, filepath+1, MAX_FN_LEN);
    fdat.name[MAX_FN_LEN]='\0';
    /* strupr(fdat.name); */
    fancy_fn(fdat.name);

    /* Strip off the filename, then dupe it onto 'path' */

    filepath[1]='\0';
    filepath=strdup(fname);
    haspath=TRUE;

    /* Set the real filename to include the full path and filename */

    strcpy(fname, filepath);
    strcat(fname, fdat.name);
  }
  else
  {
    strncpy(fdat.name, fname, MAX_FN_LEN);
    fdat.name[MAX_FN_LEN]='\0';
    strupr(fdat.name);

    haspath=FALSE;

    if (num_finf==-1)
      strcpy(fname, path);
    else
      *fname='\0';

    strcat(fname, fdat.name);
    /* strupr(fname); */
    fancy_fn(fname);
  }

/*fdat.priv=area.filepriv;
  fdat.lock=area.filelock;*/
  
  fdat.flag=FF_FILE;

  /* Make free bytes/time if area-wide */

  if (pfah->fa.attribs & FA_FREETIME)
    fdat.flag |= FF_NOTIME;
  if (pfah->fa.attribs & FA_FREESIZE)
    fdat.flag |= FF_NOBYTES;

  /* Handle /t or /b */

  if (desc && *desc=='/')
  {
    for (desc++; tolower(*desc)=='t' || tolower(*desc)=='b'; desc++)
      if (tolower(*desc)=='t')
        fdat.flag |= FF_NOTIME;
      else
        fdat.flag |= FF_NOBYTES;
    
    /* Now skip any spaces */

    while (*desc==' ')
      desc++;
  }

  /* If we have manual size & date overrides for this area, we can save on
   * disturbing the (possible) CDROM and extract these from the file, and
   * at the same time strip the size and date `garbage' from the description
   * This only applies to filenames that don't contain wildcards, of course.
   */

  nofind=0;
  if (!filepath &&
      (pfah->fa.attribs & FA_LISTDATE) &&
      strpbrk(fname, "*?")==NULL)
  {
    dword * plSize = (num_finf==-1) ? &fdat.fsize : NULL;
    union stamp_combo * puCombo = (num_finf==-1) ? &fdat.udate : NULL;
    char *p = desc;
    
    while (isspace(*p))
      p++;

    /* Decide if some CDROM manufacturer in their great 
       wisdom have swapped the standard size/date order. */

    p += strspn(desc,"0123456789");

    /* Handle standard "size date" order */

    if (*p != '-' && *p != '/' && *p != '.')  /* It isn't a date, at least */
    {
      desc=descsize(desc,plSize);
      desc=descdate(desc,puCombo);
    }
    else /* Handle "date size" order */
    {
      desc=descdate(desc,puCombo);
      desc=descsize(desc,plSize);
    }

    if (num_finf==-1 && fdat.udate.ldate)
      nofind=1;
  }

  if (nofind)
  {

    /* This is safe since CDROMs are typically pseudo-FAT */

    memcpy(&fdat.fdate, &fdat.udate, sizeof(fdat.udate));
    write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));

  }
  else
  {

    /* If the filename has a path, or we don't have it in our finf buffer */

    if (haspath || num_finf==-1)
    {
      FFIND *ff;

      if ((ff=FindOpen(fname, 0))==NULL)
      {
        fdat.flag |= FF_OFFLINE;
        write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));
        free(filepath);
        return 0;
      }

      do
      {
        /* Copy the filename into the fdat buffer */

        strnncpy(fdat.name, ff->szName, sizeof(fdat.name));

        /* Copy the upload date from the creation date, and the file
         * date from the write date.
         * These will already be filled if we've skipped the find.
         */

        fdat.udate=ff->scCdate;
        fdat.fdate=ff->scWdate;
        fdat.fsize=ff->ulSize;

        /* strupr(fdat.name); */
	fancy_fn(fdat.name);
        write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));
      }
      while (!nofind && FindNext(ff)==0);

      FindClose(ff);
    }
    else
    {
      found=FALSE;

      /* No path, so do it the fast way.  The file is presumably already      *
       * in our local buffer.                                                 */

      if (strchr(fname, '*') || strchr(fname, '?'))
      {
        /* It contains wildcards, so do a sequential search */

        for (f=finf, e=f+num_finf; f < e; f++)
        {
          strncpy(buf, f->name, 12);
          buf[12]='\0';

          if (MatchWC(fname, buf))
          {
            strcpy(fdat.name, buf);

            fdat.udate=f->cdate;
            fdat.fdate=f->wdate;
            fdat.fsize=f->size;

            write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));

            found=TRUE;
          }
        }
      }
      else
      {
        /* It's a single file, so try a binary search */

        int iLo=0;
        int iHi=num_finf-1;
        int iTry, iComp;

        while (iLo <= iHi)
        {
          iTry=(iLo+iHi)/2;

          f=finf + iTry;

          strncpy(buf, f->name, 12);
          buf[12]=0;

          iComp=strcmp(fname, buf);

          if (iComp < 0)
            iHi=iTry-1;
          else if (iComp > 0)
            iLo=iTry+1;
          else
          {
            /* We found it, so it's okay to write the record */
            strcpy(fdat.name, buf);

            fdat.udate=f->cdate;
            fdat.fdate=f->wdate;
            fdat.fsize=f->size;

            write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));
            found=TRUE;
            break;
          }
        }
      }
    
      /* If not found */
    
      if (!found)
      {
        fdat.flag |= FF_OFFLINE;
        write_entry(&fdat, desc, NULL, filepath, PFAS(pfah, acs));
      }
    }
  }

  if (filepath)
    free(filepath);

  return 0;
}




/* Write a files.dat entry for one file */

static void near write_entry(FDAT *fdat, char *desc, char *ul, char *path, char *acs)
{
  FIDX fidx;
  char pair, match;
  char *s, *p;
  long ofs;
  word slen;

  /* Adjust file dates so that they aren't newer than the current date */

  if (GEdate(&fdat->fdate, &scNow))
  {
    fdat->fdate.ldate=0;
    fdat->fdate.msg_st.date.mo=1;
    fdat->fdate.msg_st.date.da=1;
  }

  if (GEdate(&fdat->udate, &scNow))
  {
    fdat->udate.ldate=0;
    fdat->udate.msg_st.date.mo=1;
    fdat->udate.msg_st.date.da=1;
  }

  if ((++ctr % 64L)==0)
  {
    printf("\b\b\b\b%04ld", ctr);
    fflush(stdout);
  }

  fdat->uploader=0L;

  pair=desc ? *desc : '\0';
  match=0;

  /* Check to see if we have a '(xxxx)', '[xxxx]' or '<xxxx>' download      *
   * count.                                                                 */

  if (pair=='(')
    match=')';
  else if (pair=='[')
    match=']';
  else if (pair=='<')
    match='>';

  if (match)
  {
    /* If we start with one of those chars, then do a search, starting      *
     * at the char AFTER the first open punctuator.  Keep going while       *
     * we have blanks, tabs or digits.  If all goes well, this should       *
     * put us at the very END of the field.  Also, mark the location        *
     * of the FIRST digit we see.                                           */

    for (s=desc+1, p=NULL; desc && *s==' ' || *s=='\t' || isdigit(*s); s++)
      if (!p && isdigit(*s))
        p=s;

    /* If the next char is the closing punctuator, then we know that we've  *
     * found a bona-fide download counter.                                  */

    if (desc && *s==match)
    {
      fdat->times_dl=p ? (dword)atol(p) : 0L;

      /* Now skip any spaces after the DL counter */

      while (*++s==' ')
        ;

      /* ... and adjust the description appropriately */

      strocpy(desc, s);
      
      fdat->flag |= FF_DLCTR;
    }
  }

  /* Write the description and (optionally) the name of the uploader */

  slen=(desc ? strlen(desc) : 0)+1;

  fdat->desc=Btell(dmp);
  cBwrite(dmp, (char *)&slen, sizeof(word));
  cBwrite(dmp, desc ? desc : "", slen);

  ofs = fdat->desc + sizeof(word) + slen;

  /* Write the name of the uploader, if necessary */

  if (ul)
  {
    fdat->uploader=ofs;
    slen=strlen(ul)+1;

    cBwrite(dmp, (char *)&slen, sizeof(word));
    cBwrite(dmp, ul, slen);

    ofs += sizeof(word) + slen;
  }

  /* Write the file's path, if necessary */

  if (path)
  {
    fdat->path=ofs;
    slen=strlen(path)+1;

    cBwrite(dmp, (char *)&slen, sizeof(word));
    cBwrite(dmp, path, slen);

    ofs += sizeof(word) + slen;
  }

  if (acs)
  {
    fdat->acs=ofs;
    slen=strlen(acs)+1;

    cBwrite(dmp, (char *)&slen, sizeof(word));
    cBwrite(dmp, (char *)acs, slen);

    ofs += sizeof(word) + slen;
  }
  
  memset(&fidx, '\0', sizeof(FIDX));
  memmove(fidx.name, fdat->name, MAX_FN_LEN);

  fidx.anum=anum;
  fidx.fpos=(word)(Btell(dat) / (long)sizeof(FDAT));
  
  cBwrite(dat, (char *)fdat, sizeof(FDAT));
  cBwrite(idx, (char *)&fidx, sizeof(FIDX));
}



/* Called if we encounter an error when opening a file */

static void near cant_open(char *name)
{
  printf("Error!  Can't open file `%s'.\n",name);
}


/* Read in an index file, sort it, and write it out */

static void near sort_idx(char *path)
{
  FIDX *ip=NULL;
  long idxsize;
  word nidx;

  Bseek(idx, 0L, BSEEK_END);
  idxsize=Btell(idx);

  nidx=(word)(idxsize/(long)sizeof(FIDX));

  if (nidx==0)
  {
    say_done();
    return;
  }

  if ((unsigned long)idxsize > UINT_MAX || (ip=malloc((unsigned int)idxsize))==NULL)
    NoMem();

  printf(" - Sorting");
  fflush(stdout);

  Bseek(idx,0L,SEEK_SET);

  if (Bread(idx, (char *)ip, (unsigned)idxsize) != idxsize)
    printf("\aError!  Can't read index for %s!\n", path);
  else
  {
    qsort(ip, (size_t)nidx, sizeof(FIDX), idxcomp);
    Bseek(idx, 0L, SEEK_SET);

    if (cBwrite(idx, (char *)ip, (int)idxsize) != 0)
      printf("\aError writing index file!\n");
    else say_done();
  }
  
  free(ip);
}



/* Compare two index entries */

static int _stdc idxcomp(const void *i1,const void *i2)
{
  return (strncmp( ((FIDX *)i1)->name, ((FIDX *)i2)->name, MAX_FN_LEN ) );
}





/* Print "done" for an area */

static void near say_done(void)
{
  printf(" - Done!\n");
  fflush(stdout);
}





/* Ran out of memory */

void _fast NoMem(void)
{
  printf("\nOut of memory!\n");
  exit(1);
}


#ifdef __FLAT__

  static unsigned long ReadFiles(FIDX *pfi, long lMaxSize,
                                 struct _alist *al, int *piGot,
                                 int cnt)
  {
    long lFileSize;
    int iGotThis;
    int iFile;
    BFILE bf;

    if ((cnt%4)==0)
    {
      printf("\rReading (%d)", cnt);
      fflush(stdout);
    }

    /* Open the index file */

    if ((bf=Bopen(al->path, BO_RDONLY | BO_BINARY, BSH_DENYNO,
                  MERGE_1_SIZE))==NULL)
    {
      /* ignore */
      return lMaxSize;
    }


    /* Get size of file, then go to beginning */

    lFileSize=Bseek(bf, 0L, BSEEK_END);
    Bseek(bf, 0L, SEEK_SET);

    /* Make sure we don't go over end of buffer */

/*    printf("filesize=%ld, maxsize=%ld\n", lFileSize, lMaxSize);*/
    lFileSize=min(lFileSize, lMaxSize);

    /* Read the whole file in */

    if (Bread(bf, pfi, lFileSize) != lFileSize)
    {
      printf("Fatal error reading %ld bytes from index file %s!\n",
             (long)lFileSize, al->path);
      exit(1);
    }

    Bclose(bf);

    /* Increment the count of stuff that was read */

    iGotThis = lFileSize / sizeof(FIDX);
    *piGot += iGotThis;

    /* Now fix the entries in this area so that they have the correct
     * file area number, just in case an area was inserted.
     */

    for (iFile = iGotThis; --iFile >= 0; )
      pfi++->anum = al->anum;

    return lMaxSize - (iGotThis * sizeof(FIDX));
  }


  static void near sort_index(char *master_idx, struct _alist *al, long size)
  {
    FIDX *pfi;
    BFILE bf;
    int got=0;
    int cnt=0;

    /* Allocate memory for the indices */

    if ((pfi=malloc((size_t)size+1))==NULL)
    {
      printf("Error!  Can't allocate %ld bytes to sort indices\n", size);
      exit(1);
    }

    unlink(master_idx);

    /* Read in all index files */

    while (al)
    {
      size=ReadFiles(pfi+got, size, al, &got, cnt++);
      al=al->next;
    }

    printf("\r                         \rSorting...");
    fflush(stdout);

    qsort(pfi, got, sizeof(FIDX), idxcomp);

    printf("\rWriting...");
    fflush(stdout);

    if ((bf=Bopen(master_idx, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY,
                  BSH_DENYNO, MERGE_OUT_SIZE))==NULL)
    {
      cant_open(master_idx);
      exit(1);
    }

    if (Bwrite(bf, pfi, got * sizeof(FIDX)) != got*sizeof(FIDX))
      NoSpace();

    Bclose(bf);

    printf("\rDone!       \n");

    free(pfi);
  }

#else /* !__FLAT__ */



  /* Remove a node from the area list */

  static int near unlist(struct _alist *rem)
  {
    struct _alist *p, *last;

    for (p=ap, last=NULL; p; last=p, p=p->next)
      if (p==rem)
      {
        free(p->path);

        if (last)
          last->next=p->next;
        else ap=p->next;

        free(p);
        return TRUE;
      }

    return FALSE;
  }




  /* Add a node to the area list */

  static void near addlist(struct _alist *p)
  {
    struct _alist *oldap;

    oldap=ap;
    ap=p;
    ap->next=oldap;
  }



  /* Merge all of the sorted index files into one large sorted index */

  static void near merge_lists(int len)
  {
    char temp[PATHLEN];
    static int filectr=0;
    struct _alist *s1, *s2, *p;
    word new_anum;
    int x;

    /* Find the two smallest indices in our linked list */

    get_smallest(&s1, &s2);

    printf("Merging (%d)  \r", len);
    fflush(stdout);


    if (s1->size > 0L && s2->size > 0L)
    {
      if (!s2)
      {
        printf("Already sorted!\n");
        return;
      }

      sprintf(temp, "$MRG%04x.$$$", filectr++);

      x=merge_these(s1, s2, temp);

      if (x==1)
      {
        unlist(s1);
        return;
      }
      else if (x >= 2)
      {
        if (x==3)
          unlist(s1);

        unlist(s2);
        return;
      }

      new_anum=0xffffu;
    }
    else
    {
      /* One of the two was a zero length file, so just create a dummy        *
       * entry.                                                               */

      if (s1->size > 0)
      {
        strcpy(temp, s1->path);
        new_anum=s1->anum;
      }
      else
      {
        strcpy(temp, s2->path);
        new_anum=s2->anum;
      }
    }

    /* Delete the temporary files */

    if (*s1->path=='$')
      unlink(s1->path);

    if (*s2->path=='$')
      unlink(s2->path);

    unlist(s1);
    unlist(s2);

    if ((p=malloc(sizeof(struct _alist)))==NULL)
      NoMem();

    p->path=strdup(temp);
    p->size=fsize(temp);
    p->anum=new_anum;

    addlist(p);
  }




  /* Get the smallest index entry in a list */

  static void near get_smallest(struct _alist **smaller,struct _alist **bigger)
  {
    struct _alist *p;

    if (!ap)
    {
      *smaller=*bigger=NULL;
      return;
    }

    if (ap->size==-1L)
      ap->size=fsize(ap->path);

    if (ap->next->size==-1L)
      ap->next->size=fsize(ap->next->path);

    if (ap->size < ap->next->size)
    {
      *smaller=ap;
      *bigger=ap->next;
    }
    else
    {
      *smaller=ap->next;
      *bigger=ap;
    }

    for (p=ap->next->next; p; p=p->next)
    {
      if (p->size==-1L)
        p->size=fsize(p->path);

      if (p->size < (*smaller)->size)
      {
        *bigger=*smaller;
        *smaller=p;
      }
      else if (p->size < (*bigger)->size)
        *bigger=p;
    }
  }



  /* Read one file entry in from the BFILE file */

  int read_b(BFILE bf, FIDX *pb, struct _alist *al)
  {
    int rc=(Bread(bf, pb, sizeof *pb)==sizeof *pb);

    if (rc && al->anum != 0xffffu)
      pb->anum=al->anum;

    return rc;
  }


  /* Merge files together, given the file handles */

  static int near merge_these_bfiles(BFILE bf1, BFILE bf2, BFILE bfo,
                                     struct _alist *a1, struct _alist *a2)
  {
    FIDX b1, b2;
    int haveb1, haveb2;

    haveb1=read_b(bf1, &b1, a1);
    haveb2=read_b(bf2, &b2, a2);

    while (haveb1 && haveb2)
    {
      /* Find the 'lowest' of the two filenames */

      if (strncmp(b1.name, b2.name, MAX_FN_LEN) <= 0)
      {
        if (Bwrite(bfo, &b1, sizeof b1) != sizeof b1)
          NoSpace();

        haveb1=read_b(bf1, &b1, a1);
      }
      else
      {
        if (Bwrite(bfo, &b2, sizeof b2) != sizeof b2)
          NoSpace();

        haveb2=read_b(bf2, &b2, a2);
      }
    }

    if (haveb1)
    {
      do
      {
        if (Bwrite(bfo, &b1, sizeof b1) != sizeof b1)
          NoSpace();
      }
      while (read_b(bf1, &b1, a1));
    }

    if (haveb2)
    {
      do
      {
        if (Bwrite(bfo, &b2, sizeof b2) != sizeof b2)
          NoSpace();
      }
      while (read_b(bf2, &b2, a2));
    }

    return 0;
  }






  /* Use mergesort on the named files */

  static int near merge_these(struct _alist *a1, struct _alist *a2, char *into)
  {
    BFILE bf1, bf2, bfo;
    int rc=-1;

    if ((bf1=Bopen(a1->path, BO_RDONLY | BO_BINARY, BSH_DENYNO,
                   MERGE_1_SIZE))==NULL)
    {
      cant_open(a1->path);
      exit(1);
    }
    else
    {
      if ((bf2=Bopen(a2->path, BO_RDONLY | BO_BINARY, BSH_DENYNO,
                     MERGE_2_SIZE))==NULL)
      {
        cant_open(a2->path);
        exit(1);
      }
      else
      {
        if ((bfo=Bopen(into, BO_CREAT | BO_TRUNC | BO_WRONLY | BO_BINARY,
                       BSH_DENYNO, MERGE_OUT_SIZE))==NULL)
        {
          cant_open(into);
          exit(1);
        }
        else
        {
          rc=merge_these_bfiles(bf1, bf2, bfo, a1, a2);
          Bclose(bfo);
        }

        Bclose(bf2);
      }

      Bclose(bf1);
    }

    return rc;
  }
#endif



/* Add the specified file extension to the given filename */

static void near make_ext(char *name, char *ext)
{
  char *bs, *dot;
  
  bs=strrstr(name, "\\/");
  dot=strrchr(name, '.');
  
  if (!bs)
    bs=name;
  
  if (dot && dot > bs)
    *dot='\0';
  
  strcat(name, ext);
}


static sword _fast near cBwrite(BFILE b, void *buf, word size)
{
  if (Bwrite(b, buf, size) != size)
    NoSpace();
  
  return 0;
}


/* See if two filenames match, taking wildcards into account */

static word near MatchWC(char *pat,char *fn)
{
  while (*pat && *fn)
  {
    if (*pat=='*' || *fn=='*')
    {
/*    if (*fn != '.')
        fn++; */

      while (*fn && *fn != '.')
        fn++;
      
      if (*pat != '.')
        pat++;
      
       while (*pat && *pat != '.')
        pat++;
    }
    else if (*pat=='?' || *fn=='?')
    {
      if (*pat != '.')
        pat++;
      
      if (*fn != '.')
        fn++;
    }
    else if (toupper(*pat++) != toupper(*fn++))
    {
      return FALSE;
    }
  }
  
  return (*pat=='\0' && *fn=='\0');
}


