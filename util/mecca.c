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
static char rcs_id[]="$Id: mecca.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=MECCA -- the Maximus Embedded Command Compiler (Advanced)
    name=
    name=This is the source module for the Maximus Embedded Command
    name=Compiler (Advanced), a superset of OECC.  MECCA is 100%
    name=backwards-compatible with OECC, right down to the actual
    name=bytes output.  MECCA is a one-pass compiler which uses
    name=backpatching to handle forward references.
*/

#define MECCA
#define SILT
#define SILT_INIT
#define MECCA_INIT
#define MAX_INCL_VER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <dos.h>
#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "max.h"
#include "mecca.h"

struct _symbol *symbol[MAX_SYM]={NULL};
unsigned char tokenbuf[MAX_TOKENLEN+1];

int o_type=0; /* Output type: default ext = .RBS and RLE not done */
int lcol, scol, n_scol;
int max_len, define, errs=0;

int last_goto;
char *err_rsvd="\a    Error!  Reserved word used as a token: `%s'!\n";

static char *ixts[2]={ ".mec", ".mer" };
static char *exts[2]={ ".bbs", ".rbs" };

int _stdc main(int argc,char *argv[])
{
  Hello("MECCA", "Maximus Embedded Command Compiler (Advanced)",
        VERSION, "1990, " THIS_YEAR);

  if (argc < 2)
  {
    printf("Usage: MECCA <infile> [outfile] [-t] [-r]\n\n");

    printf("<infile> uses a default extension of .MEC.  <infile> can also specify\n"
           "a wildcard such as \"*.MEC\".\n\n");

    printf("If <outfile> is not specified, then MECCA will default to the\n");
    printf("input filename, with an extension of .BBS.  If <infile> is specified using\n"
           "wildcards, then <outfile> should specify the PATH in which to place\n"
           "the compiled output files.  If no <outfile> is specified when using\n"
           "wildcards, then the output files will be placed in the same directory\n"
           "as the input files.\n\n");

    printf("The \"-t\" parameter instructs MECCA to compile only those .MEC files which\n"
           "are newer than their .BBS counterparts.\n");

    printf("The \"-r\" parameter instructs MECCA to produce RIP compatible output and\n"
           "changes the default output extension to .RBS.\n");
    return 1;
  }

  Init_Table();

  Parse_Args(argc,argv);

  if (errs)
    printf("\n*** %d errors in compile ***\n",errs);
  else printf("\nDone!\n");
  return 0;
}


void Parse_Args(int argc,char *argv[])
{
  FFIND *ff;

  static char inname[PATHLEN];
  static char outname[PATHLEN];
  static char av1[PATHLEN];
  static char av2[PATHLEN];
  static char temp[PATHLEN];
  char *p;

  int found_extension;
  int time_comp;
  int ret;
  int x;

  *av1='\0';
  *av2='\0';
  time_comp=FALSE;

  for (x=1; x < argc; x++)
  {
    if (*argv[x]=='-')
    {
      switch(tolower(argv[x][1]))
      {
        case 'r':
          o_type=!o_type;
          break;
        default:  /* for backwards compatibility */
        case 't':
          time_comp=TRUE;
          break;
      }
    }
    else if (*av1)
      strcpy(av2, argv[x]);
    else strcpy(av1, argv[x]);
  }

  if ((ff=FindOpen(av1, 0))==NULL)
  {
    strcpy(temp, av1);
    strcat(temp, ".mec");

    if ((ff=FindOpen(temp, 0))==NULL)
    {
      strcpy(temp, av1);
      strcat(temp, ".mer");

      if ((ff=FindOpen(temp, 0))==NULL)
      {
        errs++;
        printf("File not found: `%s'\n",av1);
        return;
      }
    }
  }

  for (ret=0; ret==0; ret=FindNext(ff))
  {
    int wastype=o_type;

    strcpy(inname, av1);
    max_len=12;

    if (
	(p=strrchr(inname, '/')) != NULL || 
	(p=strrchr(inname, '\\')) != NULL ||
        (p=strrchr(inname, ':')) != NULL)
    {
      *(p+1)='\0';
      max_len=strlen(inname)+12;
      strcpy(p+1, ff->szName);
    }
    else strcpy(inname, ff->szName);

    if ((p=strchr(av1, '.')) != NULL && *(p+1)=='\0')
      strcat(inname, ".");

    if (strchr(av1,'*') || strchr(av1,'?'))
    {
      if (*av2)
      {
        strcpy(outname,av2);

#ifndef UNIX
        if (outname[strlen(outname)-1] != '\\' && outname[strlen(outname)-1] != ':')  
#else
        if (outname[strlen(outname)-1] != PATH_DELIM)
#endif
          strcat(outname, PATH_DELIMS);
      }
      else strcpy(outname,inname);

      if (
	  (p=strrchr(outname, '\\')) != NULL || 
	  (p=strrchr(outname, '/')) != NULL ||
          (p=strrchr(outname, ':')) != NULL
	 )
      {
        *(p+1)='\0';
        strcat(outname, ff->szName);

        if ((p=strrchr(outname, '.')) != NULL)
          *p='\0';
      }
      else *outname='\0';
    }
    else if (argc >= 3) /* No wildcards */
    {
      strcpy(outname,av2);

#ifndef UNIX
      if (outname[strlen(outname)-1]=='\\' || outname[strlen(outname)-1]==':')
#else
      if (outname[strlen(outname)-1]==PATH_DELIM)
#endif
      {
        strcat(outname, ff->szName);

        if ((p=strrchr(outname,'.')) != NULL)
          *p='\0';
      }
    }
    else *outname='\0';

    found_extension=FALSE;
    p=inname;

    if (strpbrk(p, PATHDELIM) != NULL) /* Find if there's a filename extension */
    {
      while ((p=strpbrk(p, PATHDELIM)) != NULL)
      {
        if (*p=='\\' || *p=='/')
          found_extension=FALSE;

        if (*p=='.')
        {
          found_extension=TRUE;
          if (eqstri(p+1,"mer"))
            o_type=1;
        }

        p++;
      }
    }

    strcpy(temp,inname);

    if (found_extension)
    {
      if (! *outname)
      {
        (*strrchr(temp,'.'))='\0';   /* Chop off extension */
        strcpy(outname,temp);
      }
    }
    else
    {
      if (! *outname)
        strcpy(outname,inname);

      strcat(inname,ixts[o_type]);
    }

    found_extension=FALSE;
    p=outname;

    if (strpbrk(p, PATHDELIM) != NULL) /* Find if there's a filename extension */
    {
      while ((p=strpbrk(p, PATHDELIM)) != NULL)
      {
        if (*p=='\\' || *p=='/')
          found_extension=FALSE;

        if (*p=='.')
          found_extension=TRUE;

        p++;
      }
    }

    if (! found_extension)
      strcat(outname, exts[o_type]);

    lcol=7;
    n_scol=0;

    Compile(inname, outname, 0, time_comp);
    o_type=wastype;
  }

  FindClose(ff);
}


/* mode==0: only used on first call.  mode==1: used on second and
   consecutive calls, means we open in semi-APPEND mode, and don't
   update the label table after this input file.                    */

void Compile(char *inname,char *outname,int mode,int time_comp)
{
  SCOMBO in_st, out_st;

  char *amode;

  FILE *infile, *outfile;

  FFIND *inf, *outf;

  int ch;
  int tokenidx;
  int lastch;
  int lastcount;
  int x, y;

  lastch=-1;
  lastcount=0;

  if (!mode)
  {
    last_goto=FALSE;
    
    if (eqstri(inname,outname))
    {
      errs++;
      printf("Can't compile %s to itself!  Skipping.\n",inname);
      return;
    }

    if (time_comp)
    {
      if ((inf=FindInfo(inname)) != NULL)
      {
        if ((outf=FindInfo(outname)) != NULL)
        {
          in_st=inf->scWdate;
          in_st=inf->scWdate;

          out_st=outf->scWdate;
          out_st=outf->scWdate;

          FindClose(outf);

          /* Don't compile if it's not new. */

          if (! GEdate(&in_st, &out_st))
            return;
        }

        FindClose(inf);
      }
    }

    printf("Compiling %-*s to %-*s: ",
           max_len, /*strupr*/fancy_fn(inname), max_len, /*strupr*/fancy_fn(outname));
  }

  fixPathMove(inname);
  if ((infile=fopen(inname,"r"))==NULL)
  {
    if (mode==1)
      printf("    ");

    printf("Error opening `%s' for input!\n",inname);
    errs++;

    return;
  }

  if (mode==0)
    amode="w+b";
  else amode="r+b";

  fixPathMove(outname);
  if ((outfile=fopen(outname, amode))==NULL)
  {
    fclose(infile);

    printf("Error opening `%s' for output!\n", outname);
    errs++;

    return;
  }

  if (mode==0)
    printf("\n");

  if (mode==1)
    fseek(outfile, 0L, SEEK_END);

#if 0
  if (setvbuf(infile, NULL, _IOFBF, INOUT_BUFFER))
    NoMem();

  if (setvbuf(outfile, NULL, _IOFBF, INOUT_BUFFER))
    NoMem();
#endif

  while ((ch=getc(infile)) != EOF)
  {
#ifdef UNIX
    /* Wes -- The UNIX version was putting out \r\r\n,
     * instead of \r\n in the *.BBS file. I suspect
     * this is because the input eats \r\n into \n when 
     * opened from a non-UNIX platform in !O_BINARY
     * mode. So I'm going to eat the \r myself, and
     * maintain dos-compatible output.
     */
     if (ch == '\r')
       continue;
#endif

    if (!o_type && ch==lastch && ch != '[')
      lastcount++;
    else Compress_Sequence();

    if (ch=='\n')
    {
      putc('\r',outfile);
      putc('\n',outfile);
      lastcount=0;
      lastch=-1;
    }
    else if (ch=='[')
    {
      ch=-1;
      tokenidx=0;
      tokenbuf[0]='\0';

      /* Read in the token */

      while (ch != ']' && tokenbuf[0] != '[' && tokenidx < MAX_TOKENLEN)
      {
        ch=getc(infile);
#ifdef UNIX
        /* wes - see \r comments above */
        if (ch == '\r')
          continue;
#endif
        tokenbuf[tokenidx++]=(unsigned char)ch;
      }

      if (tokenbuf[0] != '[')
      {
        lastch=-1;
        lastcount=0;

        tokenbuf[tokenidx-1]='\0';

        if (tokenidx==MAX_TOKENLEN)
        {
          printf("    Token too long: `%s'\n", tokenbuf);
          errs++;
        }

/*        tokenbuf[tokenidx]='\0';*/

        strlwr(tokenbuf);

        Parse_Token(outname, &outfile, inname);
      }
    }
  }

  Compress_Sequence();

  if (mode==0)
  {
    for (x=0; symbol[x] && x < MAX_SYM; x++)
    {
      if (symbol[x]->location==-1)
      {
        printf("    Undefined symbol: `%s'\n",symbol[x]->name);
        errs++;
      }
      else
      {
        for (y=0; y < symbol[x]->offsetnum; y++)
        {
          fseek(outfile, symbol[x]->offset[y], SEEK_SET);
          fprintf(outfile, "%05ld", symbol[x]->location);
        }
      }

      free(symbol[x]);
      symbol[x]=NULL;
    }
  }

  fclose(outfile);
  fclose(infile);

  return;
}




void Parse_Token(char *outname,FILE **outfile, char *inname)
{
  struct _inf inf;
  char *p;
  word verbnum;

  /* These cover the ?line/?file/?above type tokens */

  static struct
  {
    char *token;    /* Partial token */
    char *pfseq;    /* Printf string */
  } ptokens[] =
    {
      { "below",  "\x10""B%c"   },
      { "equal",  "\x10""Q%c"   },
      { "file",   "\x10""F%c"   },
      { "line",   "\x10""L%c"   },
      { "xclude", "\x10""X%c"   },
      { NULL,     NULL          }
    };


  p=strtok(tokenbuf, TOKENDELIM);

  while (p)
  {
    if (*p=='/')
    {
      p++;
      define=TRUE;
    }
    else define=FALSE;

    if ((verbnum=sssearch(p, verbs, verb_table_size))==0xffffu)
    {
      /* Test for exceptions */
      if (isdigitstr(p))            /* A "[65]"-type ASCII code */
        fputc(atoi(p),*outfile);
      else
      {
        int i;

        for ( i=0 ; ptokens[i].token ; ++i )
          if (stricmp(p+1, ptokens[i].token)==0)
          {
            if (define)
              printf(err_rsvd,p);

            fprintf(*outfile, ptokens[i].pfseq, toupper(*p));
            break;
          }
        if (!ptokens[i].token)    /* Else it's undefined, maybe a label? */
          Process_Label(p,define,*outfile);
      }
    }
    else
    {
      if (define)
        printf(err_rsvd,p);

      last_goto=FALSE;

      if (verbs[verbnum].fptr)
      {
        /* Copy any pertinent info into struct */

        inf.p=&p;
        inf.infile=NULL;
        inf.outfile=outfile;
        inf.outname=outname;
        inf.inname=inname;
        inf.verbnum=verbnum;

        /* And call the handling routine */

        (*verbs[verbnum].fptr)(&inf);
      }
      else Output_Code(verbnum,*outfile,&lcol);
    }

    if (p != NULL)
      p=strtok(NULL,TOKENDELIM);
  }
}



void P_On(struct _inf *inf)
{
  FILE *outfile;
  char *p;
  
  int ch,
      x;
    
  outfile=*inf->outfile;
    
  if (*inf->p && ftell(outfile) != 0)  /* Get the next colour token */
  {
    p=strtok(NULL,TOKENDELIM);

    fseek(outfile,-1L,SEEK_CUR);
#ifdef UNIX
    /* wes - see \r comments elsewhere */
    while ((ch = getc(outfile)) == '\r');
#else
    ch=getc(outfile);
#endif
    fseek(outfile,-1L,SEEK_CUR);

    x=Colour_Num(p);

    if (x >= 8)
    {
      printf("    Invalid background colour: %s\n", p);
      x -= 8;
    }

    ch += x*16;

    lcol=ch;

    if (ch >= 160)
    {
      fseek(outfile,-1L,SEEK_CUR);
      ch -= 128;
    }

    putc(ch,outfile);
  }
}

void P_Comment(struct _inf *inf)
{
  *inf->p=NULL;
}

void P_Include(struct _inf *inf)
{
  char *p;
  
  p=strtok(NULL,TOKENDELIM);

  if (p)
  {
    char *szName=p;

    fclose(*inf->outfile);

    /* If no absolute path specified, assume that included file
       is relative to the same directory as original
    */

    if (*p != '\\' && *p != '/' && p[1] != ':')
    {
      char *s = strrchr(inf->inname, PATH_DELIM);
      if (s!=NULL)
      {
        int i = s - inf->inname;
        if ((szName=malloc(strlen(p)+i+2))==NULL)
          NoMem();
        strncpy(szName,inf->inname,i);
        szName[i++]= PATH_DELIM;
        strcpy(szName+i,p);
      }
    }

    Compile(szName,inf->outname,1,FALSE);

    if (szName!=p)
      free(szName);

    if ((*inf->outfile=fopen(inf->outname,"r+b"))==NULL)
    {
      printf("\a    Error reopening `%s' for output!\n", inf->outname);
      errs++;
      return;
    }

#if 0
    if (setvbuf(*inf->outfile,NULL,_IOFBF,INOUT_BUFFER))
      NoMem();
#endif

    fseek(*inf->outfile,0L,SEEK_END);
  }
}

void P_Copy(struct _inf *inf)
{
  FILE *copyfile,
       *outfile;
     
  char *p;
  int ch;
  
  p=strtok(NULL,TOKENDELIM);
  outfile=*inf->outfile;

  fixPathMove(p);
  if (p && (copyfile=fopen(p,"rb")) != NULL)
  {
    while ((ch=getc(copyfile)) != EOF)
    {
#ifdef UNIX
      /* wes - see \r comments elsewhere */
      if (ch == '\r')
        continue;
#endif
      putc(ch,outfile);
    }

    fclose(copyfile);
  }
  else
  {
    printf("    Error opening `%s'!\n",p);
    errs++;
  }
}
      
 
void P_Setpriv(struct _inf *inf)
{
  char *p;
  
  p=strtok(NULL,TOKENDELIM);

  if (p)
    fprintf(*inf->outfile,"\x17""s%c",toupper(*p));
}

void P_Setattr(struct _inf *inf)
{
  char *p;
  
  p=strtok(NULL,TOKENDELIM);

  if (p)
    fprintf(*inf->outfile,"\x17""\x0d""B%c",toupper(*p));
}

void P_Menu_Cmd(struct _inf *inf)
{
  char temp[PATHLEN];
  char *p;
  
  word opt;
  int x;

  p=strtok(NULL,TOKENDELIM);

  if (p && !isdigit(*p))
  {
    for (x=0;x < silt_table_size;x++)
      if (eqstri(silt_table[x].token,p))
        break;

    if (x==silt_table_size)
    {
      printf("\a    Invalid menu command: `%s'!\n",p);
      x=0;
    }
    else sprintf(p=temp,"%d",(int)silt_table[x].opt);
  }

  if (p)
  {
    opt=atoi(p);

    fprintf(*inf->outfile,"\x17""r%c%c",
            (byte)(opt >> 8),
            (byte)(opt & 0xff));
  }
}

void P_Access(struct _inf *inf, char type)
{
  char *p;

  if ((p=strtok(NULL,TOKENDELIM+1))!=NULL)
    fprintf(*inf->outfile,"\x10%c%s ", type, p);
}

void P_Acsfile(struct _inf *inf)
{
  P_Access(inf, 'f');
}

void P_Acs(struct _inf *inf)
{
  P_Access(inf, 'a');
}


void P_Save(struct _inf *inf)
{
  NW(inf);
  scol=lcol;
}

void P_Load(struct _inf *inf)
{
  if (n_scol)
    n_scol--;

  if (scol != -1)
    Put_Colour(*inf->outfile,scol);

  lcol=scol;
}

void P_Fg(struct _inf *inf)
{
  char *p;
  int x;
  
  p=strtok(NULL,TOKENDELIM);

  if (p)
  {
    x=Colour_Num(p);
    lcol=(lcol & 0xf0)+x;
    Put_Colour(*inf->outfile,lcol);
  }
}

void P_Bg(struct _inf *inf)
{
  char *p;
  int x;
  
  p=strtok(NULL,TOKENDELIM);

  if (p)
  {
    x=Colour_Num(p);
    lcol=(lcol & 0x8f)+(x << 4);
    Put_Colour(*inf->outfile,lcol);
  }
}

void P_Steady(struct _inf *inf)
{
  Put_Colour(*inf->outfile,lcol);
}

void P_Dim(struct _inf *inf)
{
  Put_Colour(*inf->outfile,lcol &= ~0x08);
}

void P_Bright(struct _inf *inf)
{
  Put_Colour(*inf->outfile,lcol |= 0x08);
}

void P_Label(struct _inf *inf)
{
  char *p;

  p=strtok(NULL,TOKENDELIM);

  if (p)
    Process_Label(p,TRUE,*inf->outfile);
}

void P_Restore(struct _inf *inf)
{
  NW(inf);
  lcol=scol;
}



void P_Priv(struct _inf *inf)
{
  FILE *outfile;
  
  word verbnum, type;

  char keystr[PATHLEN], *kp;
  char priv_letter;
  
  char *p, *p2, *s;
     
  p=strtok(NULL,TOKENDELIM);
  p2=strtok(NULL,TOKENDELIM);

  outfile=*inf->outfile;
  verbnum=inf->verbnum;
  
  if (p)
  {
    kp=keystr;

    if ((s=strchr(p,'/')) != NULL)
      while (*++s)
      {
        *s=(char)toupper(*s);
        
        if ((*s >= '1' && *s <= '8') || (*s >= 'A' && *s <= 'Z'))
          *kp++=(char)toupper(*s);
      }

    *kp='\0';
    
    priv_letter=(char)toupper(*p);

    type=Determine_Type(p2);

    if (type==1)
      Invert_Logical(&verbnum);

    if (priv_letter != '/')
    {
      if (verbnum==num_gt || verbnum==num_above)
        fprintf(outfile,"\x10""L%c\x10""X%c",priv_letter,priv_letter);
      else if (verbnum==num_lt || verbnum==num_below)
      {
        if (type==2)
          fprintf(outfile,"\x10""F%c",priv_letter);
        else fprintf(outfile,"\x10""B%c\x10""X%c",priv_letter,priv_letter);
      }
      else if (verbnum==num_eq || verbnum==num_equal)
        fprintf(outfile,"\x10""Q%c",priv_letter);
      else if (verbnum==num_ne || verbnum==num_unequal ||
               verbnum==num_notequal)
        fprintf(outfile,"\x10""X%c",priv_letter);
      else if (verbnum==num_ge || verbnum==num_ae)
        fprintf(outfile,"\x10""L%c",priv_letter);
      else if (verbnum==num_le || verbnum==num_be)
        fprintf(outfile,"\x10""B%c",priv_letter);
    }

    /* Now handle any keys */

    if (*keystr)
      fprintf(outfile, "\x17""k%c%s ",
              ((verbnum==num_eq || (verbnum==num_ge) || (verbnum==num_gt))
              ? 'I' : 'N'), keystr);

    if (type==2 && verbnum != num_lt && verbnum != num_below)
      fprintf(outfile,"\x0f""Q\r\n");
  }
}

void P_Iftime(struct _inf *inf)
{
  int hh,mm;
  
  char temp[PATHLEN],
       *p,
       *p2;

  word vn;
         
  p=strtok(NULL,TOKENDELIM);
  p2=strtok(NULL,TOKENDELIM);

  if ((vn=sssearch(p,verbs,verb_table_size))==0xffffu ||
      !IsValidTimeVerb(vn))
  {
    printf("\a    Invalid time token: `%s'!\n",p);
    errs++;
  }
  else
  {
    if (sscanf(p2,"%d:%d",&hh,&mm) != 2)
    {
      printf("\a    Invalid time format: `%s'!\n",p2);
      errs++;
    }
    else
    {
      strcpy(temp,"\x17t");

      if (vn==num_gt || vn==num_above)
        strcat(temp,"\x01");
      else if (vn==num_lt || vn==num_below)
        strcat(temp,"\x02");
      else if (vn==num_eq || vn==num_equal)
        strcat(temp,"\x03");
      else if (vn==num_ne || vn==num_unequal || vn==num_notequal)
        strcat(temp,"\x04");
      else if (vn==num_ge || vn==num_ae)
        strcat(temp,"\x05");
      else if (vn==num_le || vn==num_be)
        strcat(temp,"\x06");

      strcat(temp,"%c%c");
      fprintf(*inf->outfile,temp,hh+1,mm+1);
    }
  }
}


void Process_Label(char *p,int define,FILE *outfile)
{
  word vn;

  if (sssearch(p,verbs,verb_table_size) != 0xffffu)
  {
    printf(err_rsvd,p);
    errs++;
  }

  for (vn=0; symbol[vn] && vn < MAX_SYM; vn++)
    if (eqstr(symbol[vn]->name, p))
      break;

  if (vn==MAX_SYM)
  {
    printf("\a    Error!  Too many symbols defined!\n");
    errs++;
  }
  else
  {
    if (symbol[vn]==NULL) /* Allocate new token */
    {
      if ((symbol[vn]=(struct _symbol *)malloc(sizeof(struct _symbol)))==NULL)
        NoMem();

      strncpy(symbol[vn]->name, p, MAX_SYMBOLLEN);
      symbol[vn]->name[MAX_SYMBOLLEN-1]='\0';
      symbol[vn]->offsetnum=0;
      symbol[vn]->location=-1;
    }

    if (define)
      symbol[vn]->location=ftell(outfile);
    else
    {
      if (!last_goto)
        printf("    Warning: Label reference `%s' is not preceded by a `goto'.\n",p);
      if (symbol[vn]->location==-1)
        symbol[vn]->offset[symbol[vn]->offsetnum++]=ftell(outfile);

      if (symbol[vn]->offsetnum==MAX_OFFSETS)
      {
        printf("\a    Too many references, label `%s'\n",p);
        errs++;
      }
      else if (!define)
      {
        if (symbol[vn]->location != -1)
          fprintf(outfile, "%05ld", symbol[vn]->location);
        else fputs("?????", outfile);
      }
    }
  }
}

void P_Goto(struct _inf *inf)
{
  fputs("\x0fV", *inf->outfile);
  last_goto=TRUE;
}

void P_Textsize(struct _inf *inf)
{
  int rows, cols;
  char *p;

  cols=rows=-1;
  if ((p = strtok(NULL,TOKENDELIM))!=NULL)
  {
    cols=atoi(p);
    if ((p=strtok(NULL,TOKENDELIM))!=NULL)
      rows=atoi(p);
  }
  fputs(verbs[inf->verbnum].translation, *inf->outfile);
  fputc(cols, *inf->outfile);
  fputc(rows, *inf->outfile);
}

void P_Repeat(struct _inf *inf)
{
  Output_Code(inf->verbnum, *inf->outfile, &lcol);
}

void P_Repeatseq(struct _inf *inf)
{
  Output_Code(inf->verbnum, *inf->outfile, &lcol);
}


void P_Locate(struct _inf *inf)
{
  Output_Code(inf->verbnum, *inf->outfile, &lcol);
}


void NoMem(void)
{
  printf("    Ran out of memory!\n");
  exit(1);
}


word sssearch(char *key, struct _table base[], unsigned int num)
{
  word try, lasthi, lastlo;
  word lastx=0xffffu;

  char *s, *t;

  lasthi=num;
  lastlo=0;

  for (;;)
  {
    try=((lasthi-lastlo) >> 1)+lastlo;

    if (lastx==try)
      return 0xffffu;

    lastx=try;

    for (s=key, t=base[try].verb; *s==*t; s++, t++)
      if (! *s)
        return (try);                       /* Found a match */

    if (*s > *t)
      lastlo=try;
    else
      lasthi=try;
  }
}



void Put_Colour(FILE *outfile,int col)
{
  if (col >= 160)
    fprintf(outfile, "\x16\x01%c", col-128);
  else fprintf(outfile, "\x16\x01\x10%c", col | 0x80);
}


int Colour_Num(char *name)
{
  word vn;
  
  if ((vn=sssearch(name, verbs, verb_table_size))==0xffffu)
  {
    printf("    Invalid colour: `%s'\n",name);
    errs++;

    return 7;
  }

  return (verbs[vn].colour);
}

int isdigitstr(char *s)
{
  while (*s)
    if (! isdigit(*s++))
      return FALSE;

  return TRUE;
}




void Invert_Logical(word *verbnum)
{
  if (*verbnum==num_gt || *verbnum==num_above)
    *verbnum=num_le;
  else if (*verbnum==num_lt || *verbnum==num_below)
    *verbnum=num_ge;
  else if (*verbnum==num_eq || *verbnum==num_equal)
    *verbnum=num_ne;
  else if (*verbnum==num_ne || *verbnum==num_unequal ||
           *verbnum==num_notequal)
    *verbnum=num_eq;
  else if (*verbnum==num_ge || *verbnum==num_ae)
    *verbnum=num_lt;
  else if (*verbnum==num_le || *verbnum==num_be)
    *verbnum=num_gt;
}




int IsValidTimeVerb(word verbnum)
{
  if (verbs[verbnum].fptr==P_Priv)
    return TRUE;
  else return FALSE;
}


word Determine_Type(char *s)
{
  if (!s || eqstri(s,"see") ||  eqstri(s,"show") || eqstri(s,"line"))
    return 0;
  else if (eqstri(s,"skip") || eqstri(s,"noshow"))
    return 1;
  else if (eqstri(s,"quit") || eqstri(s,"exit"))
    return 2;

  printf("\a    Invalid action: `%s'\n",s);

  return 0;
}



/* Output a translation code to the *.BBS file, and set the colour token    *
 * (if required.)                                                           */

void Output_Code(word verbnum,FILE *outfile,int *lcol)
{
  if (verbnum < verb_table_size)
  {
    fputs(verbs[verbnum].translation, outfile);

    if (verbs[verbnum].colour != -1)
      *lcol=verbs[verbnum].colour;
  }
}


