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
static char rcs_id[]="$Id: maid.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

#define MAX_INCL_LANGUAGE
#define MAX_INCL_VER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <io.h>
#include "prog.h"
#include "language.h"
#include "max.h"
#include "prm.h"
#include "prmapi.h"

#define MAX_LTFHEAP   64000u
#define MAX_CONTENTS  2048
#define MAX_VARLEN    48

#define MAX_INCLUDES  8

#define MF_LTF      0x01
#define MF_HEADER   0x02

#define MAX_HEAP_STR   256

static char *version=VERSION;
static char *heap;
static char *hptr;

#define MAX_DEFN 256

static struct _defn
{
  char *name;
  char *xlt;
} defn[MAX_DEFN];

static word hn_ofs=0;
static int ndef=0;
static int n_symbol=0;
static int n_include=0;
static int in_user_heap=0;
static unsigned int file_ptrs=0;
static unsigned int user_ptrs=0;
static unsigned int alt_ptrs=0;
static char *heapnames;
static int do_h=FALSE;
static int do_lth=FALSE;

static char szLangName[32];
static char out[MAX_CONTENTS];
static char varname[MAX_VARLEN];
static char heapname[MAX_VARLEN]="";
static char *efile_format= "#define %s%s s_ret(0x%x)\n";
static char *efile_nformat="#define n_%s%s 0x%x\n";
static char *mfile_format= "#define str_%s lstr(%u)\n";
static char *mfile_uformat="#define str_%s hstr(lh_%s, %u)\n";
static char hname[PATHLEN];
static char lname[PATHLEN];
static char ename[PATHLEN];
static char mname[PATHLEN];
static char prmfile[PATHLEN];
static char err=FALSE;
static char firststring=TRUE;
static unsigned longest=0;

static char iiname[PATHLEN];
static char *iname[MAX_INCLUDES];
static int  linenum[MAX_INCLUDES];

struct _heap
{
  char *name;
  char *heapstart;
  char *s[MAX_HEAP_STR];      /* Strings array */
  char *a[MAX_HEAP_STR];      /* Alternate strings array */
  HOFS hofs[MAX_HEAP_STR*2];  /* only used when writing out file */
  word nofs;                  /* offset of name in heapnames heap */
  int heapsym;
  int ns;                     /* Number of strings */
  int as;                     /* Maximum number of alternate strings */
  int is_user_heap;
};

static struct _heap h[MAX_HEAP];
static int nh=0;

static FILE *ifile[MAX_INCLUDES];   /* The *.MAD input file(s) */
static FILE *hfile;                 /* The *.H output file */
static FILE *lfile;                 /* The *.LTF output file */
static FILE *efile;                 /* The *.LTH enumeration file */
static FILE *mfile;                 /* The *.MH MEX enumeration file */

static int flags=0;
static int this_num=FALSE;
static int this_badrle=FALSE;

static void Make_Name(char *in,char *out,char *ext);

#define idchar(c) (c && (isalnum(c) || c=='_' || c=='-' || c=='\''))

static void ErrOpen(char *name)
{
  printf("Fatal error opening '%s'!  Aborting...\n",name);
  exit(1);
}

static void Error(char *msg, char *args, int fatal)
{
  printf("\rError %s %d: ********* %s%c '%s'\n", iname[n_include],
           linenum[n_include], msg, (args) ? ':' : 0, (args) ? args : "");
  if (fatal)
    exit(1);
  err=TRUE;
}

static void Process_Define(char *line)
{
  char temp[PATHLEN];
  char *s;

  getword(line,temp," \t\n#",2);
  s=firstchar(line," \t\n#",3);

  if (!s)
    s="";

  if ((defn[ndef].name=strdup(temp))==NULL ||
      (defn[ndef++].xlt=strdup(s))==NULL)
  {
    Error("Out of memory defining variable",temp,TRUE);
  }
}

static void Include_File(char *fname, char st)
{
  if (++n_include==MAX_INCLUDES)
  {
    --n_include;
    Error("Too many nested include files", NULL, FALSE);
  }
  else
  {
    char temp[PATHLEN];

    ifile[n_include]=NULL;

    /* Try relative to current directory first */

    if (st=='"')
    {
      strcpy(temp,fname);
      ifile[n_include]=fopen(temp,"r");
    }

    /* Now try relative to the directory containing the original */

    if (ifile[n_include]==NULL)
    {

      /* Test for absolute path */

      if (*fname=='\\' || *fname=='/' || fname[1]==':')
        strcpy(temp,fname);
      else
      {
        char *p=iname[n_include-1];
        char *s=strrchr(p,'\\');
        char *q=strrchr(p,'/');

        /* Get path (if any) from previous level include */

        if (s==NULL || q > s)
          s=q;
        if (s==NULL)
          s=temp;
        else
        {
          int len=s-p+1;
          memcpy(temp,p,len);
          s=temp+len;
        }

        /* Append the included filename */

        strcpy(s,fname);
      }
      ifile[n_include]=fopen(temp,"r");
    }

    if (ifile[n_include]==NULL)
      ErrOpen(fname);

    else
    {
      if ((iname[n_include]=strdup(temp))==NULL)
      {
        fclose(ifile[n_include]);
        --n_include;
        Error("Out of memory processing include file",fname,TRUE);
      }
      linenum[n_include]=0;
    }

  }
}

static void Process_Include(char *line)
{
  char temp[PATHLEN];
  char q, *s;

  getword(line,temp," \t\n#",2);
  q=0;
  if (*(s=temp)=='"')
    q='"';
  else if (*s=='<')
    q='>';

  if (q!=0)
  {
    char *p=strchr(++s,q);
    if (p!=NULL)
    {
      *p='\0';
      if (strchr(s,'.')==NULL)  /* Default extension is .MI */
        strcat(temp,".mi");
      Include_File(s, q);
      return;
    }
  }
  Error("Invalid #include filename",temp,FALSE);
}


static char *Add_Definition(char *s,char *outp)
{
  char temp[PATHLEN];
  char *p;
  int x;

  /* Copy the definition into 'temp' */

  for (p=temp;idchar(*s);s++)
    *p++=*s;

  *p='\0';

  for (x=0; x < ndef; x++)
    if (eqstri(temp, defn[x].name))
    {
      strcpy(outp, defn[x].xlt);
      outp += strlen(outp);
      break;
    }

  if (x==ndef && *temp)
  {
    Error("Undefined variable", temp,FALSE);
  }

  *outp='\0';
  return s;
}

/* Process the 'string' portion of the macro */

static void Get_Var_Contents(char *s)
{
  char *outp;

  *(outp=out)='\0';

  if (*s != '\"')
    goto InQuote;

  for (s++;*s;)
  {
    if (*s=='\\')   /* quoting backslash */
    {
      *outp++='\\';

      /* Copy the next character... */
      *outp++=*++s;
      s++;
    }
    else if (*s=='\"')  /* End of string... */
    {
      for (;;)
      {

        ++s;

InQuote:

        /* Skip any whitespace */
        while (*s==' ' || *s=='\t')
          s++;

        if (*s==';')      /* End of string */
        {
          *outp='\0';
          return;
        }
        else if (isascii(*s))  /* A definition */
        {
          s=Add_Definition(s,outp);
          outp += strlen(outp);
        }
        else
        {
          *outp='\0';
          return;
        }

        if (*s==' ' || *s=='\t')
        {
          while (*s==' ' || *s=='\t')
            s++;
        }
        else if (! (*s=='\"' || *s==';' || *s=='\0'))
          s++;

        if (*s=='\"' || *s==';' || *s=='\0')
          break;
        else
          s--;
      }

      s++;
    }
    else
      *outp++=*s++;      /* Normal, so copy the character */
  }

  *outp='\0';
}



static char * Get_Var_Name(char *line)
{
  char *s,*d;

  /* Copy the name into the varname[] array */

  for (s=line,d=varname; idchar(*s) || *s=='$' || *s=='#';)
    *d++=*s++;

  *d='\0';

  if (d[-1]=='$')
  {
    d[-1]='\0';
    this_num=TRUE;
  }
  else
    this_num=FALSE;

  if (d[-1]=='#')
  {
    d[-1]='\0';
    this_badrle=TRUE;
  }
  else this_badrle=FALSE;

  while (isspace(*s))
    s++;

  /* Make sure that there was an EQUAL SIGN after everything */
  if (*s != '=')
  {
    Error("Invalid definition of symbol",varname,FALSE);
    return NULL;
  }

  while ((isspace(*++s) || *s=='=') && *s && *s != '\"')
    ;

  return s;
}

static void near Process_RLE(char *str)
{
  unsigned char *p, *end;
  unsigned char ch;
  
  for (p=str; *p;)
  {
    /* If there are at least four identical chars here */

    if (p[0]==p[1] && p[0]==p[2] && p[0]==p[3] && p[0] >= ' ')
    {
      for (end=p+4; *end==*p; end++)
        ;
      
      /* Save a copy of the character to RLE */
      
      ch=*p;
      

      /* RLE escape code */

      p[0]='\x19';
      p[1]=ch;
      p[2]=(char)(end-p);
      
      /* Now get rid of all the stuff we compressed */

      strocpy(p+3, end);
      p += 3;
    }
    else
      p++;
  }
}


static void Process_Backslashes(char *str)
{
  int tint;
  char *out;
  char *s,*d;

  if ((out=malloc(strlen(str)+1))==NULL)
  {
    Error("No mem for backslash split",NULL,TRUE);
  }

  for (s=str,d=out;*s;)
  {
    if (*s != '\\')
      *d++=*s++;
    else
    {
      ++s;

      if (*s=='x' || *s=='X')
      {
        if (sscanf(++s,"%02x",&tint)==1)
          *d++=(char)tint;

        s += 2;
      }
      else if (*s=='r')
      {
        *d++='\r';
        s++;
      }
      else if (*s=='n')
      {
        *d++='\n';
        s++;
      }
      else if (*s=='a')
      {
        *d++='\a';
        s++;
      }
      else *d++=*s++;
    }
  }

  *d='\0';
  strcpy(str,out);
  free(out);
}


/* Write a string to the .h file, with translations */

static void near WriteHfile(unsigned char *s)
{
  fprintf(hfile, "extrn char *%s IS(\"", varname);
 
  while (*s)
  {
    if (*s=='"' || *s=='\\')
      fprintf(hfile, "\\%c", *s);
    else if (*s >= ' ' && *s <= 127)
      fputc(*s, hfile);
    else
      fprintf(hfile, "\" \"\\x%02x\" \"", (unsigned char)*s);

    s++;
  }
    
  fprintf(hfile, "\");\n");
}

#define LF_ALT  0x01  /* Alternate string */
#define LF_MEX  0x02  /* Export to .MH */

static void Process_String(char *line, word flags)
{
  char *s;
  unsigned len;

  if ((s=Get_Var_Name(line))==NULL)
    return;

  Get_Var_Contents(s);

  if (! *heapname)
    return;

  if (flags & LF_ALT)
    alt_ptrs++;
  else
  {
    if (efile && !in_user_heap)
    {
      fprintf(efile, efile_format, "", varname, n_symbol);

      if (this_num)
        fprintf(efile, efile_nformat, "", varname, n_symbol);
    }

    if (mfile && (in_user_heap || (flags & LF_MEX)))
    {
      if (firststring)
      {
        firststring=FALSE;

        /* Bracket this section in #ifdef/#endif */

        fprintf(mfile, "\n// Language heap: %s\n\n#ifdef INCL_%s\n\n",
                       heapname, heapname);

        /* Add heap variable for MEX user heaps and a heap init function */

        if (in_user_heap)
          fprintf(mfile, "string: lh_%s;\n"
                         "void init_lang_%s()\n{\n    lh_%s := \"%s\";\n}\n\n",
                         heapname, heapname, heapname, heapname);
      }

      if (in_user_heap)
        fprintf(mfile, mfile_uformat, varname, heapname, n_symbol%MAX_HEAP_STR);
      else
        fprintf(mfile, mfile_format, varname, n_symbol);
    }

    n_symbol++;

    if (in_user_heap)
      user_ptrs++;
    else
      file_ptrs++;
  }


  Process_Backslashes(out);

  if (!(flags & LF_ALT))
  {
    if (!this_badrle)
      Process_RLE(out);
    if (hfile)
      WriteHfile(out);

    h[nh].s[h[nh].ns]=hptr;
    h[nh].ns++;
  }
  else
  {
    /* Backfill any so far unused alternate slots */

    while (h[nh].as < (h[nh].ns-1))
    {
      /* copy ptrs in the 's' array into 'a' */
      h[nh].a[h[nh].as]=h[nh].s[h[nh].as];
      h[nh].as++;
    }

    /* And place this into the alternate heap */

    h[nh].a[h[nh].as]=hptr;
    h[nh].as++;
  }

  len=strlen(out)+1;

  if (((hptr-heap)+len) > MAX_LTFHEAP)
    Error("Strings exceed maximum heap size",NULL,TRUE);

  memcpy(hptr,out, len);
  hptr += len;
  if (len > longest)
    longest=len;
}


static int Process_a_Line(void)
{
  static char line[MAX_CONTENTS];
  char temp[PATHLEN];
  word flags=in_user_heap ? LF_MEX : 0;
  int nws;

  while (fgets(line,MAX_CONTENTS,ifile[n_include])==NULL)
  {
    fclose(ifile[n_include]);
    if (n_include--==0)
      return FALSE;
    free(iname[n_include+1]);
  }

  linenum[n_include]++;
    
  Strip_Trailing(line,'\n');

  /* Strip any leading spaces */

  nws=strspn(line," \t");
  if (nws > 0)
    memmove(line,line+nws,strlen(line+nws)+1);

  if (! *line || *line==';')
    return TRUE;

  while (*line=='@')
  {
#if defined(OS_2)
    if (eqstrin(line+1, "DOS", 3) || eqstrin(line+1, "NT", 2))
      return TRUE;
#elif defined(__MSDOS__)
    if (eqstrin(line+1, "OS2", 3) || eqstrin(line+1, "NT", 2))
      return TRUE;
#elif defined(NT)
    if (eqstrin(line+1, "DOS", 3) || eqstrin(line+1, "OS2", 3))
      return TRUE;
#elif defined(UNIX)
    if (eqstrin(line+1, "DOS", 3) || eqstrin(line+1, "OS2", 3) || eqstrin(line + 1, "NT", 2))
      return TRUE;
#else
    #error Invalid OS!
#endif

    /* RIP or ALT - use as alternate */

    if (eqstrin(line+1, "RIP", 3) || eqstrin(line+1, "ALT", 3))
      flags |= LF_ALT;

    /* MEX - export to MEX .MH (assumed in 'user' heaps) */

    if (eqstrin(line+1, "MEX", 3))
      flags |= LF_MEX;

    if (eqstrin(line+1, "NT", 2)) /* NT is only two letters long */
      strocpy(line, line+4);
    else
      strocpy(line, line+5);
  }

  while (line[strlen(line)-1] != ';' && strlen(line) < MAX_CONTENTS-1 &&
         *line != '#' && *line != ':' && *line != '=')
  {
    strcat(line," ");

    if (fgets(line+strlen(line), MAX_CONTENTS-strlen(line), ifile[n_include])==NULL)
      Error("premature end of file",NULL,TRUE);

    linenum[n_include]++;

    Strip_Trailing(line,'\n');
  }

  /* Strip any leading spaces */

  nws=strspn(line," \t");
  if (nws > 0)
    memmove(line,line+nws,strlen(line+nws)+1);

  if (*line==':' || *line=='=')
  {

    if (*heapname=='\0')
      h[nh=0].name=strdup(line+1);
    else
    {
      nh++;

      if (nh >= MAX_HEAP)
      {
        Error("Too many string heaps defined",NULL,TRUE);
      }

      h[nh].name=strdup(line+1);
    }

    if (nh != 0)
    {
      if (efile && !in_user_heap)
        fprintf(efile, "\n#endif /* %s */\n\n", heapname);
      if (mfile && !firststring)
        fprintf(mfile, "\n#endif // %s\n\n", heapname);
    }

    if (nh && n_symbol >= nh*MAX_HEAP_STR)
    {
      Error("Too many symbols in heap",heapname,TRUE);
    }
    else
      n_symbol=nh*MAX_HEAP_STR;

    /* 'user' heap definitions start with a '=' */

    in_user_heap=(*line=='=');

    strcpy(heapname,line+1);

    printf("Heap: %-32.32s\r",heapname);
    fflush(stdout);

    if (efile && !in_user_heap)
    {
      fputs("\n\n\n",efile);
      fprintf(efile, efile_format,  "hunk_", heapname, n_symbol);
      fprintf(efile, efile_nformat, "hunk_", heapname, n_symbol);

      fprintf(efile, "\n#ifdef MAX_LANG_%s\n\n", heapname);
    }

    firststring=TRUE;

    h[nh].heapstart=hptr;
    h[nh].heapsym=n_symbol;
    h[nh].ns=0;
    h[nh].as=0;
    h[nh].is_user_heap=in_user_heap;
  }
  else if (*line=='#')
  {
    getword(line,temp," \t\n#",1);

    if (eqstri(temp,"define"))
      Process_Define(line);
    else if (eqstri(temp,"include"))
      Process_Include(line);
    else
    {
      Error("Unknown directive",temp,FALSE);
    }
  }
  else if (isalpha(*line))
    Process_String(line,flags);

  return TRUE;
}
  

static void Write_Heap(void)
{
  static struct _gheapinf ginf;
  static struct _heapdata hd;

  long hpos, apos;
  sword x, y, j;
  char *p, langfile[sizeof(ginf.language)];
  word l;

  if (err)
    return;

  /* Increment over last heap */
  nh++;

  /* Space over last status message */
  printf("\r%-32.32s\r","");

  /* Make heapnames heap */

  if ((heapnames=malloc(MAX_HEAPNAMES))==NULL)
  {
    printf("No mem for heapnames\n");
    exit(1);
  }

  hn_ofs=0;
  for (x=0 ; x < nh ; x++)
  {
    word i=(word)strlen(h[x].name)+1;
    if ((hn_ofs+i) > MAX_HEAPNAMES)
    {
      printf("Size of all heapnames exceeds maximum (%d)\n", MAX_HEAPNAMES);
      exit(1);
    }
    memcpy(heapnames+hn_ofs,h[x].name,i);
    h[x].nofs=hn_ofs;
    hn_ofs+=i;
  }

  memset(&ginf, '\0', sizeof(struct _gheapinf));

  p=strrchr(iiname, PATH_DELIM);

  if (!p)
    p=strrchr(iiname, ':');

  if (p)
    p++;
  else
    p=iiname;

  strncpy(langfile, p, sizeof(langfile)-1);
  langfile[sizeof(langfile)-1]=0;

  if ((p=strchr(langfile, '.')) != NULL)
      *p=0;

  if (*szLangName)
    strcpy(ginf.language, szLangName);
  else
  {
    /* If no language name specified on the command line, get
     * it from the language file itself.
     */

    strcpy(ginf.language, langfile);
  }

  ginf.n_heap=(byte)nh;
  ginf.max_gptrs_len=ginf.max_gheap_len=0;
  ginf.file_ptrs=file_ptrs;
  ginf.user_ptrs=user_ptrs;
  ginf.hn_len=hn_ofs;

  /* Don't do the heap size check on heaps 0 and 1, since they're the       *
   * global and sysop heaps (respectively), and aren't figured into the     *
   * total calculation.                                                     */

  for (x=2;x < nh;x++)
  {
    l=(h[x].ns+h[x].as)*sizeof(HOFS);
    
    if (l > ginf.max_gptrs_len)
      ginf.max_gptrs_len=l;
    
    if (x==nh-1)
      l=hptr-h[x].heapstart;
    else
      l=h[x+1].heapstart-h[x].heapstart;
    
    if (l > ginf.max_gheap_len)
      ginf.max_gheap_len=l;
  }

  printf("Strings: primary=%u user=%u alt=%u longest=%ub\n",
            file_ptrs,user_ptrs,alt_ptrs,longest);
  printf("   Heap: total=%ub max_heap_size=%ub max_ptr_size=%ub\n",
            hptr-heap,ginf.max_gheap_len,ginf.max_gptrs_len);

  fwrite((char *)&ginf, sizeof(struct _gheapinf), 1, lfile);
  fwrite((char *)heapnames, hn_ofs, 1, lfile);
  apos=ftell(lfile);
  
  if (hfile || efile)
    printf("\n");

  for (x=0;x < nh;x++)
  {
    hd.start_ofs=-1L;
    hd.start_num=h[x].heapsym;
    hd.ndefs=h[x].ns;
    hd.adefs=h[x].as;
    hd.heapname=h[x].nofs;

    if (x==nh-1)
      hd.hlen=hptr-h[x].heapstart;
    else
      hd.hlen=h[x+1].heapstart-h[x].heapstart;

    if (hfile || efile)
      printf("Heap %c%02x, Size=%05d  Pointers=%-3d Alternates=%-3d (%s)\n",
              (h[x].is_user_heap)?'*':' ',x,hd.hlen,hd.ndefs,hd.adefs,h[x].name);
    
    fwrite((char *)&hd, sizeof(struct _heapdata), 1, lfile);
  }
  
  for (x=0;x < nh;x++)
  {
    /* Now, reduce each pointer to the offset-from-start-of-heap form */
    
    for (y=0;y < h[x].ns;y++)
      h[x].hofs[y] = h[x].s[y] - h[x].heapstart;

    for (j=0;j < h[x].as;j++,y++)
      h[x].hofs[y] = h[x].a[j] - h[x].heapstart;
    
    hpos=ftell(lfile);
    fwrite((char *)h[x].hofs, sizeof(HOFS)*y, 1, lfile);

    if (x==nh-1)
      y=hptr-h[x].heapstart;
    else
      y=h[x+1].heapstart-h[x].heapstart;
    
    fwrite(h[x].heapstart, y, 1, lfile);
    
    fseek(lfile, apos + x*sizeof(struct _heapdata), SEEK_SET);
    fread((char *)&hd, sizeof(struct _heapdata), 1, lfile);
    hd.start_ofs=hpos;
    fseek(lfile, apos + x*sizeof(struct _heapdata), SEEK_SET);
    fwrite((char *)&hd, sizeof(struct _heapdata), 1, lfile);
    
    fseek(lfile, 0L, SEEK_END);
  }

  if (*prmfile)
  {
    int fd;
    long end;

    /* We have to close this first so that the file dates are correct */

    fclose(lfile);
    lfile=NULL;

    end=timerset(200);

    /* Update the .PRM file if requested */

    if ((fd=open(prmfile, O_RDWR | O_BINARY)) != -1)
    {
      struct m_pointers prm;

      /* Read in the PRM file header */

      if (read(fd, (char *)&prm, sizeof(prm)) != sizeof(prm) ||
          prm.id != 'M' || prm.version != CTL_VER ||
          prm.heap_offset != sizeof(struct m_pointers))
      {
        printf("\nNote: PRM file '%s' has not been updated.\n"
               "(This is normal for an upgrade install.)\n",
               prmfile);
      }
      else
      {

        /* Now read in the heap */

        lseek(fd, prm.heap_offset, SEEK_SET);
        read(fd, heap, MAX_LTFHEAP);

        /* Check the max ptr/heap lengths */

        if (ginf.max_gptrs_len > prm.max_ptrs)
          prm.max_ptrs = ginf.max_gptrs_len;
        if (ginf.max_gheap_len > prm.max_heap)
          prm.max_heap = ginf.max_gheap_len;

        /* Now check the max ptr/heap lengths for the global heaps */

        l=(h[0].ns+h[0].as)*sizeof(HOFS);
        if (l > prm.max_glh_ptrs)
          prm.max_glh_ptrs=l;

        l=(h[1].heapstart-h[0].heapstart);
        if (l > prm.max_glh_len)
          prm.max_glh_len=l;

        /* Now check the max ptr/heap lengths for the sysop heap */
        /* .. if this IS the sysop heap, of course :-) */

        if (eqstri(heap+prm.lang_file[0],langfile))
        {
          prm.max_syh_ptrs=(h[1].ns+h[1].as)*sizeof(HOFS);
          prm.max_syh_len=h[2].heapstart-h[1].heapstart;
        }

        while (!timeup(end))
          ;

        /* Now write it out regardless whether we changed anything */

        lseek(fd, 0L, SEEK_SET);
        write(fd, (char *)&prm, sizeof(prm));

        printf("\n'%s' updated.\n", prmfile);
      }
      close(fd);
    }
  }  

  printf("\nDone.\n");
}


static void Compile_Language(HPRM hp)
{
  iname[0]=iiname;

  ifile[0]=hfile=lfile=efile=NULL;

  /* If it could not be opened, so try to compile it relative to the
   * language path.
   */

  if ((ifile[0]=fopen(iiname,"r"))==NULL)
  {
    char temp[PATHLEN];

    /* If invalid .prm file, abort */

    if (!hp)
      ErrOpen(iiname);

    PrmRelativeString(hp, PrmFileString(hp, lang_path), temp);
    strcat(temp, iiname);

    if ((ifile[0]=fopen(temp,"r"))==NULL)
      ErrOpen(iiname);

    strcpy(iiname, temp);
  }

  if (do_h)
    Make_Name(iiname,hname,".h");
  else
    *hname='\0';
  
  Make_Name(iiname,lname,".ltf");
  Make_Name(iiname,mname,".mh");
  
  if (do_lth)
    Make_Name(iiname,ename,".lth");
  else
    *ename='\0';

  if (*hname && (hfile=fopen(hname,"w"))==NULL)
    ErrOpen(hname);

  if ((lfile=fopen(lname,"w+b"))==NULL)
    ErrOpen(lname);

  if (*ename && (efile=fopen(ename,"w"))==NULL)
    ErrOpen(ename);

  if (*mname && (mfile=fopen(mname,"w"))==NULL)
    ErrOpen(mname);

  linenum[0]=0;

  if (hfile)
    fprintf(hfile,"/* %s: static language include file, generated by MAID v%s */\n\n",
            /*strupr*/fancy_fn(hname),version);

  if (efile)
    fprintf(efile,"/* %s: dynamic language definition file, generated by MAID v%s */\n\n",
            /*strupr*/fancy_fn(ename),version);

  if (mfile)
    fprintf(mfile,"// %s: dynamic language definition file, generated by MAID v%s\n\n"
                  "string lstr(int: index);\n"
                  "string hstr(ref string: heapname, int: index);\n\n",
            /*strupr*/fancy_fn(mname),version);

  while (Process_a_Line())
    ;

  Write_Heap();

  if (efile)
  {
    fprintf(efile, "#define n_lang_ptrs %u\n", file_ptrs);
    fprintf(efile, "#endif\n");
    fclose(efile);
  }

  if (lfile )
    fclose(lfile);
  
  if (mfile)
    fclose(mfile);
  
  if (hfile)
    fclose(hfile);

}

static void Maid_Format(void)
{
  printf("Format:\n\n");
  
  printf("  maid <filename[.mad]> [-n<name>] [-d] [-s] [-p[prmname]]\n\n");

  printf("   -n<name>  Set language name to <name>\n");
  printf("   -d        Produce dynamic language include file (.lth)\n");
  printf("   -s        Produce static language include file (.h)\n");
  printf("   -p<file>  Update specified .PRM file.  (Defaults to MAXIMUS environment\n"
         "             variable.  Use '-p' with no argument to disable PRM updating.)\n");
  exit(1);
}

static void Make_Name(char *in,char *out,char *ext)
{
  char *s;

  strcpy(out,in);

  if ((s=strrchr(out,'.')) != NULL)
    *s='\0';

  strcat(out,ext);
}


static void Process_Switches(int argc, char *argv[])
{
  int x;
  char *last_slash;

  for (x=1;x < argc;x++)
  {
    if (*argv[x]=='/' || *argv[x]=='-')
    {
      switch(argv[x][1])
      {
        case 's':       /* Static include file */
          do_h=TRUE;
          break;

        case 'd':       /* Dunamic include file */
          do_lth=TRUE;
          break;

        case 'n':       /* Set language name */
          strcpy(szLangName, argv[x]+2);
          break;

        case 'p':       /* handled by GetMaximus */
          break;

        default:
          printf("Unknown argument: '%s'!\n", argv[x]);
          Maid_Format();
      }
    }
    else
      strcpy(iiname, argv[x]);
  }

  if (! *iiname)
    Maid_Format();
  
  if ((last_slash=strrstr(iiname, "\\/:"))==NULL)
    last_slash=iiname;

  if (! strchr(last_slash,'.'))
    strcat(iiname,".mad");

}




int _stdc main(int argc,char *argv[])
{
  char *pszMaximus;
  HPRM hp;

  NW(flags);

  printf("\nMAID  Maximus International Definitions Compiler, Version %s\n",
         version);
  printf("Copyright 1990, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

  pszMaximus = GetMaximus(argc, argv, 1);
  strcpy(prmfile, pszMaximus);

  Process_Switches(argc, argv);

  if ((heap=hptr=malloc(MAX_LTFHEAP))==NULL)
  {
    printf("Not enough memory to process language file.  Aborting...\n");
    exit(1);
  }

  hp = PrmFileOpen(pszMaximus, FALSE);
  Compile_Language(hp);

  if (hp)
    PrmFileClose(hp);

  free(heap);

  if (err)
  {
    unlink(lname);       /* Kill output file just in case */
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/* following code is to alleviate lib dependancies for silly things */
dword MAPIENTRY SquishHash(byte OS2FAR *f) { NW(f); return 0; }

void NoMem()
{
  fprintf(stderr, "Out of memory!");
}

