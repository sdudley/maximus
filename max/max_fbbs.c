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
static char rcs_id[]="$Id: max_fbbs.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=FILES.BBS-specific routine for displaying file entry
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <io.h>
#include <fcntl.h>
#include "prog.h"
#include "ffind.h"
#include "max_file.h"
#include "display.h"


  /* Get file size from description */

static char * near descsize(char *psz, long * plSize)
{
  long lsize=0L;

  while (*psz==',' || isdigit(*psz))
  {
    if (*psz != ',')    /* Which lamebrain ever thought of *this* idea?? */
      lsize=(lsize*10L)+(*psz - '0');         /* Assumes ASCII of course */
    psz++;
  }

  while (*psz==' ')                                       /* Skip spaces */
    psz++;

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
        if (year < 80)  /* > 2000 A.D. */
          year += 100;
        else if (year > 1900)
          year -= 1900; /* Four digit year */

        stamp->msg_st.date.da=day;
        stamp->msg_st.date.mo=mon;
        stamp->msg_st.date.yr=(year-80);

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


int Process_Files_Entry(DSTK *d, char *orig_entry)
{
  SCOMBO scFile;        /* true file date */
  SCOMBO scUl;          /* update date */

  FFIND *ff;

  byte *filename, *line, *string, *filespec;
  byte *thisdesc, *s, *p;

  word slsf;            /* slsf==strlen(searchfor).                         */
  word doit;            /* If we can display entry, based on newness,       *
                         * keywords, etc.                                   */
  word begl;            /* Temporary holding place for NEW beginline value! */
  word isoffline;       /* If the current file IS off-line                  */
  word ret;             /* Return value for this function                   */
  word y;               /* Scratch loop variable                            */
  int fSkipDisp=FALSE;  /* Skip the date check for this file?               */
  int wild, l;


  /* Allocate memory to hold some temporary strings */

  filename=malloc(MAX_FBBS_ENTRY+1);
  line=malloc(MAX_FBBS_ENTRY+1);
  string=malloc(MAX_FBBS_ENTRY+1);
  filespec=malloc(MAX_FBBS_ENTRY+1);
    
  if (filename==NULL || line==NULL || string==NULL || filespec==NULL)
  {
    if (string)
      free(string);
    
    if (line)
      free(line);
    
    if (filename)
      free(filename);
    
    return DRET_NOMEM;
  }
  
  /* Save the old copy of beginline */
  
  begl=d->beginline;


  /* Room to hold string, plus ^v^a^X codes when doing a substring search */

  if (strlen(orig_entry) >= MAX_FBBS_ENTRY-13)
    orig_entry[MAX_FBBS_ENTRY-13]='\0';


  /* Get rid of RLE codes */
  
  if (Trim_Line(orig_entry))
    begl=TRUE;

  
  /* Get the name of the file we're trying to find */
  
  strcpy(line, orig_entry);
  getword(line, filename, " \t\n", 1);


  /* Fix the RLE codes */
  
  if (strchr(filename,'\x19'))
    Fix_RLE(filename);
  
  upper_fn(filename);

  
  /* Now add the path for the current directory */
  
  if (strchr(filename, PATH_DELIM))
    strcpy(filespec, filename);
  else
    sprintf(filespec, ss, FAS(fah, downpath), filename);

  wild = (strpbrk(filespec, "*?") != NULL);


  /* Optimization: If we're doing a search, and no wildcards are
   * involved, we can skip the date check for this file if the text
   * pattern is not in the original text.  This is especially
   * helpful for CD-ROM file areas.
   */

  if ((d->type & DISPLAY_SEARCH) && !wild && d->beginline)
  {
    strcpy(string, line);
    cstrlwr(string);

    fSkipDisp = (stristr_nochin(string, searchfor)==NULL);
  }

  /* Set a default return code */

  ret=DRET_OK;

  /* If we haven't decided to skip displaying and/or dating this entry */

  if (!fSkipDisp)
  {
    if ((d->type & DISPLAY_NEWFILES) || autodate(fah))
    {
      ff=wild ? FindOpen(filespec, 0) : FindInfo(filespec);

      isoffline=(ff==NULL);
    }
    else
    {
      isoffline=FALSE;
      ff=NULL;
    }

    /* Repeat for each line in the description */

    for (;;)
    {
      if (isoffline)
        scFile.ldate = scUl.ldate = 0L;
      else
      {
        if ((d->type & DISPLAY_NEWFILES) || autodate(fah))
        {
          char temp_buf[MAX_FN_LEN+10];

          strnncpy(temp_buf, ff->szName, MAX_FN_LEN+1);
          strcpy(filename, Strip_Ansi(temp_buf, NULL, 0L));

          /* So no AVATAR codes won't be duplicated on 2nd pass */

          strcpy(line, orig_entry);

          scFile = ff->scWdate;
          scUl = ff->scCdate;
        }
      }

      /* Strip off the path specification */

      if ((p=strrstr(filename, "\\/")) != NULL)
        strocpy(filename, p+1);

      /* Space-pad to 12 characters */

      for (s=filename+strlen(filename), p=filename+MAX_FN_LEN; s < p; s++)
        *s=' ';

      filename[MAX_FN_LEN]='\0';
      upper_fn(filename);

      if (d->type & DISPLAY_NEWFILES)
      {
        if (!d->beginline || isoffline)
          doit=FALSE;
        else if ((doit=GEdate(&scUl, &new_date)) != FALSE)
          matches++;
      }
      else if (d->type & DISPLAY_SEARCH)
      {
        if (!d->beginline)
          doit=FALSE;
        else
        {
          /* If we can find it in either the filename or description... */

          if ((p=stristr_nochin(filename,searchfor)) != NULL ||
              (p=stristr_nochin(line,searchfor)) != NULL)
          {
            slsf=strlen(searchfor);

            if ((prm.charset & CHARSET_CHINESE)==0)
            {
              cstrlwr(filename);
              cstrlwr(line);
            }

            /* Move everything AFTER the string we found, to the right
             * three places, so we can insert the cyan colour code here
             */

            l=strlen(file_desc_col);
            strocpy(p+slsf+l, p+slsf);
            memcpy(p+slsf, file_desc_col, l);

            if ((prm.charset & CHARSET_CHINESE)==0)
              for (y=0; y < slsf; y++)
                p[y]=(byte)toupper(p[y]);

            /* Now move everything after the FRONT of the string we found,
             * to the right three places, so we can insert the light-yellow
             * colour code.
             */

            l=strlen(file_found_col);
            strocpy(p+l, p);
            memcpy(p,file_found_col,l);

            l=strlen(file_desc_col);
            strocpy(filename+l, filename);
            memcpy(filename, file_desc_col, l);

            matches++;  /* Found a match on search pattern */
            doit=TRUE;
          }
          else
          {
            doit=FALSE;
          }
        }
      }
      else
      {
        doit=TRUE;
      }

      if (doit)
      {
        dword ulSize=ff ? ff->ulSize : 0L;

        /* Strip spaces before the description, if it's less than 14 chars */

        p=firstchar(line," \t\n",2);

        /* DLN: Don't do this if listdate applies in this area.
           CDROM files listings sometimes have column format and
           this prevents us from correctly parsing the size & date
           from the file itself in ShowFileEntry() */

        if (!listdate(fah) && p > line+14 && isspace(*(line+14)))
          p=line+14;

        if (!p)
          p=blank_str;


        /* Skip over any '/f' or '/b's */

        while (*p=='/')
        {
          while (*p && *p != ' ')
            p++;

          while (*p==' ')
            p++;
        }

        if ((thisdesc=malloc(MAX_FBBS_ENTRY+1))==NULL)
        {
          ret=DRET_NOMEM;
          break;
        }

        /* Display the file entry itself */

        ret=ShowFileEntry(&d->type, d->nonstop, isoffline, &scFile, &scUl,
                          filename, ulSize, d->ck_abort, p,
                          thisdesc, &fah);

        free(thisdesc);

        if (ret != DRET_OK)
          break;
      }

      /* Don't display any more matches if the file is off-line, doesn't    *
       * contain any wildcards, or if no more matches exist.                */

      if (isoffline || !wild ||
          (!autodate(fah) &&
           (d->type & DISPLAY_NEWFILES)==0) ||
          FindNext(ff) != 0)
      {
        break;
      }
    }

    if ((d->type & DISPLAY_NEWFILES) || autodate(fah))
      FindClose(ff);
  }

  /* Restore the beginning line counter */

  d->beginline=begl;

  free(filespec);
  free(string);
  free(line);
  free(filename);
  
  return ret;
}





sword ShowFileEntry(word *type,
                    char *nstop,
                    word isoffline,
                    SCOMBO *pscFile,
                    SCOMBO *pscUl,
                    byte *filename,
                    long size,
                    int ck_abort,
                    byte *desc,
                    byte *descwork,
                    PFAH pfah)
{
  union stamp_combo *scptr;
  int fullformat;

  byte tmpdate[35];
  byte *dw;


  if (*type & DISPLAY_AREANAME)
  {
    Puts(space_over);

    Putc('\n');

    if (DispMoreYnBreak(nstop, NULL, *type))
      return DRET_BREAK;

    Printf(file_ar_name, PFAS(pfah, name), PFAS(pfah, descript));

    Putc('\n');

    if (DispMoreYnBreak(nstop, NULL, *type))
      return DRET_BREAK;

    Putc('\n');

    if (DispMoreYnBreak(nstop, NULL, *type))
      return DRET_BREAK;

    /* Only display area's name once */

    *type &= ~DISPLAY_AREANAME;
  }

  if (DispMoreYnBreak(nstop,NULL,*type))
    return DRET_BREAK;

  Printf("%s%-*s ", ((*type & DISPLAY_SEARCH) ? file_desc_col
         : file_name_col), MAX_FN_LEN, filename);

  /* Assume nicely coloured format for 'autodate' areas */

  fullformat=autodate(*pfah);

  /* Parse size if we have it (CDROM listings etc) */

  if (!fullformat && listdate(*pfah))
  {
    if ((*type & DISPLAY_SEARCH) && (size > 0))
      fullformat=TRUE;
    else if (isdigit(*desc))
    {
      char *p = desc + strspn(desc,"0123456789");

      /* Handle "size date" order */

      if (*p != '-' && *p != '/' && *p != '.')  /* It isn't a date, at least */
      {
        desc=descsize(desc, &size);
        desc=descdate(desc, pscFile);
        *pscUl = *pscFile;
        if (pscFile->ldate)
          fullformat=TRUE;
      }

      /* If we don't have autodate for this area, we may have "date size" */

      else
      {
        desc=descdate(desc, pscFile);
        *pscUl = *pscFile;
        if (pscFile->ldate)     /* Apparently valid */
        {
          fullformat=TRUE;
          desc=descsize(desc,&size);
        }
      }
    }
  }

  if (!fullformat)
    Printf(file_desc_col);
  else
  {
    if (isoffline)
    {
      Printf(file_offline, file_offline_col,
             prm.date_style==3 ? '\x07' : '\x09');
    }
    else
    {
      Printf("%s%7ld %s", file_size_col, size, file_date_col);

      Puts(FileDateFormat(pscFile, tmpdate));
    }

    if (ck_abort && halt())
      return DRET_BREAK;

    /* Display a flashing "*" if the file is new */

    if (*type & DISPLAY_NEWFILES)
      scptr=&new_date;
    else
      scptr=&usr.ludate;

    if (!isoffline && GEdate(pscUl, scptr) && scptr->ldate)
      Printf("%s*%s ", file_new_col, file_desc_col);
    else
      Printf("  %s", file_desc_col);
  }


  /* Now go through the description, copying out each word for          *
   * wordwrapping.                                                      */

  while (*desc)
  {
    dw=descwork;

    while (*desc && *desc != ' ' && *desc != '-')
      *dw++=*desc++;

    /* Copy the delimiter to the end of the string, and cap it. */

    if (*desc)
      *dw++=*desc++;

    *dw='\0';
    dw=descwork;

    /* If word would cause a wordwrap */

    if ((word)(current_col+stravtlen(descwork)) > (word)TermWidth())
    {
      /* Jump to next line, and indent */

      Putc('\n');

      if (DispMoreYnBreak(nstop, NULL, *type))
        return DRET_BREAK;

      Printf(file_desc_col);

      if (prm.fbbs_margin)
        Printf("\x19 %c", prm.fbbs_margin);
      else Printf("\x19 %c",  (prm.date_style==3 ? '\x1d' :
                              (prm.date_style==-1 ? '\x0e' : '\x1f'))-
                              (manualdate(*pfah) ? 1 : 0));


      /* Strip off any beginning whitespace */

      while (*dw==' ')
        dw++;

      /* done wordwrap */
    }

    /* Output the word */

    Puts(dw);
  }

  /* Done the description */

  Putc('\n');
  
  if (DispMoreYnBreak(nstop,NULL,*type))
    return DRET_BREAK;

  return DRET_OK;
}




int Parse_Priv(FILE *bbsfile)  /* 1==EOF 2==EOL */
{
  int ch;

  switch (ch=fgetc(bbsfile))
  {
    case 'L':     /* Show rest of line to <ch> and above */
    case 'B':
    case 'Q':
    case 'X':
      return (Priv_Code(fgetc(bbsfile),ch));

    case 'F':
      ch=fgetc(bbsfile);
      /* fall-through */

    default:
      if (Priv_Code(ch,'L')==SKIP_LINE)
        return SKIP_FILE;
      else return SKIP_NONE;
  }
}


