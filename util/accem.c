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
static char rcs_id[]="$Id: accem.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=ACCEM -- decompiles .BBS files into .MEC files
*/

#define ACCEM
#define SILT
#define SILT_INIT
#define MAX_INCL_VER
#define MECCA_INIT

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "prog.h"
#include "max.h"
#include "mecca.h"

#define MAX_COMPILED  10

void Process_Colour_Code(FILE *bbsfile,FILE *mecfile);
int sstsearch(char *key,struct _table base[],unsigned int num);
void Get_Number_String(FILE *bbsfile,char *string,int max_len);
int _stdc trcmp(void *, void *);

long offsets[MAX_OFFSETS],
     fpos;
   
int num_offsets,
    llen;
    
int _stdc main(int argc,char *argv[])
{
  struct _inf inf;
  
  FILE *bbsfile,
       *mecfile;
     
  char string[MAX_COMPILED+5],
       temp[PATHLEN],
       inname[PATHLEN],
       outname[PATHLEN],
       *s, *p;
  
  long save_fp,
       tl;

  int verbnum,
      split,
      iter,
      ch,
      x;



  Hello("ACCEM", "The Maximus Embedded Command Decompiler", VERSION,
        "1990, " THIS_YEAR);
  
  if (argc < 2)
  {
    printf("Usage: ACCEM <infile> [outfile] [-s]\n\n");
    
    printf("If no extension is specified for <infile>, then ACCEM will default to\n");
    printf("an extension of .BBS.  Likewise, if no extension is specified for\n");
    printf("[outfile], then ACCEM will default to an extension of .MEC.  If no\n");
    printf("[outfile] is specified, then ACCEM will use the root of the input\n");
    printf("filename, with an extension of .MEC.\n\n");

    printf("The `-s' command-line parameter is optional: if used, it will cause\n");
    printf("ACCEM to split any lines which are over 100 characters in length, by\n");
    printf("placing an empty brace at the end of the line.  MECCA will ignore this\n");
    printf("brace when compiling, and act as if the two lines were one.  However,\n");
    printf("this switch may be necessary if your editor cannot handle lines over a\n");
    printf("certain length.\n");

    return 1;
  }

  split=FALSE;

  strcpy(inname,argv[1]);

  if (argc==3 && (eqstri(argv[2],"-s") || eqstri(argv[2],"/s")))
  {
    split=TRUE;
    argc--;
  }

  if (argc >= 3)
  {
    strcpy(outname,argv[2]);

    if (argc >= 4 && (eqstri(argv[3],"-s") || eqstri(argv[3],"/s")))
      split=TRUE;
  }
  else
  {
    strcpy(outname,inname);

    if ((s=strchr(outname,'.')) != NULL)
      *s='\0';
  }

  p=strchr(inname,'.');
  if (!p)
  {
    p=inname+strlen(inname);
    strcpy(p, ".bbs");

    if (!fexist(inname))
    {
      strcpy(p, ".rbs");

      if (!fexist(inname))
        strcpy(p, ".bbs");
    }
  }

  /* strupr(inname); */
  fancy_fn(inname);

  if (! strchr(outname,'.'))
    strcat(outname, (p[1]=='R') ? ".mer" : ".mec");

  /* strupr(outname); */
  fancy_fn(outname);
    
  iqsort((char *)verbs, verb_table_size, sizeof(verbs[0]), trcmp);
  Init_Table();

  fixPathMove(inname);
  if ((bbsfile=fopen(inname,"rb"))==NULL)
  {
    printf("Error opening `%s' for read!\n",argv[1]);
    exit(1);
  }
  
  fixPathMove(outname);
  if ((mecfile=fopen(outname,"wb"))==NULL)
  {
    printf("Error opening `%s' for write!\n",argv[2]);
    exit(1);
  }

  num_offsets=llen=0;

  printf("Compiling `%s' to `%s':\n\n",inname,outname);

  printf("Pass 1: Building jump table\n");
  
  for (ch=0;ch != EOF;)
  {
    if ((ch=fgetc(bbsfile))=='\x0f' && (ch=fgetc(bbsfile))=='V')
    {
      Get_Number_String(bbsfile,string,5);
      tl=atol(string);

      for (x=0;x < num_offsets;x++)
        if (offsets[x]==tl)
          break;

      if (x==num_offsets) /* If's a new label offset (not found in table) */
        offsets[num_offsets++]=tl;
    }
  }

  printf("Pass 2: Decompiling\n");
  
  fseek(bbsfile,0L,SEEK_SET);
  fpos=0L;
  
  for (;;)
  {
    save_fp=fpos;

    for (x=0;x < num_offsets;x++)
      if (fpos==offsets[x])
      {
        sprintf(temp,"[/L%d]",x);
        llen += strlen(temp);
        fputs(temp,mecfile);
        break;
      }

    if ((ch=fgetc(bbsfile))==EOF)
      break;

    if (llen >= MAX_LLEN)
    {
      if (split)
        fprintf(mecfile,"[\n]");

      llen=1;
    }

    fpos++;
    
    if (ch=='[')
    {
      fputs("[[",mecfile);
      llen += 2;
    }
    else if (ch >= 32 || ch=='\t' || ch=='\r' || ch=='\n')
    {
      fputc(ch,mecfile);
      llen++;

      if (ch=='\r' || ch=='\n')
        llen=0;
    }
    else
    {
      *string='\0';

      switch (ch)
      {
        case 22:
          fpos++;
          
          if ((ch=fgetc(bbsfile))=='\x01') /* Colour code */
          {
            Process_Colour_Code(bbsfile,mecfile);
            break;
          }
          else strcpy(string,"\x16");
          /* else fall-through, a normal AVATAR command */
        
        default:
          for (iter=0;iter < MAX_COMPILED;iter++)
          {
            string[x=strlen(string)]=(char)ch;
            string[x+1]='\0';

            if ((verbnum=sstsearch(string,verbs,verb_table_size)) != -1)
            {
              if (verbs[verbnum].fptr)
              {
                inf.p=NULL;
                inf.outfile=&mecfile;
                inf.infile=bbsfile;
                inf.outname=NULL;
                inf.verbnum=verbnum;
                
                (*verbs[verbnum].fptr)(&inf);
              }
              else
              {
                fprintf(mecfile,"[%s]",verbs[verbnum].verb);
                llen += 2+strlen(verbs[verbnum].verb);
              }
              break;
            }

            fpos++;
            
            if ((ch=fgetc(bbsfile))==EOF)
              break;
          }

          if (iter != MAX_COMPILED)
            break;

          /* Now skip over the bad char to try again */
          fseek(bbsfile,fpos=save_fp+1,SEEK_SET);

          /* Special handling of priv levels */

          if (*string==0x10 && (ch=fgetc(bbsfile))!=EOF)
          {
            char *p;

            /* ?below, ?equal, ?file, ?line, ?xclude  */

            static char privch[]="BQFLX";

            if ((p=strchr(privch,ch))!=NULL)
            {

              static char *privtok[] =
              {
                "[%cbelow]", "[%cequal]", "[%cfile]", "[%cline]", "[%cxclude]"
              };

              fpos += 2;
              llen += fprintf(mecfile, privtok[p-privch], tolower(fgetc(bbsfile)));
              continue;
            }
            /* Old style ?file ? */
#ifdef OLD_PRIVFILE
            fpos++;
            llen += fprintf(mecfile, privtok[2], toupper(ch));
            continue;
#ese
            ungetc(ch,bbsfile);
#endif
          }

          llen += fprintf(mecfile, "[%d]", (unsigned char)*string);
          continue;
      }
    }
  }
  
  printf("\nDone!\n");
  
  fclose(mecfile);
  fclose(bbsfile);

  return 0;
}

int sstsearch(char *key,struct _table base[],unsigned int num)
{
  int x,
      lastx=-1,
      lasthi,
      lastlo;

  char *s,*t;

  lasthi=num;
  lastlo=0;

  for (;;)
  {
    x=((lasthi-lastlo) >> 1)+lastlo;

    if (lastx==x)
      return -1;

    lastx=x;

    for (s=key,t=base[x].translation;*s==*t;s++,t++)
      if (! *s)
        return (x);                       /* Found a match */

    if (*s > *t)
      lastlo=x;
    else lasthi=x;
  }
}


void Process_Colour_Code(FILE *bbsfile,FILE *mecfile)
{
  int ch;
  char temp[PATHLEN];
  
  /* Unescape any control codes with their high bit set. */
  
  fpos++;
  
  if ((ch=fgetc(bbsfile))==DLE)
  {
    ch=fgetc(bbsfile) & 0x7f;
    fpos++;
  }

  colour_to_string(ch,temp);
  fprintf(mecfile,"[%s]",temp);
  llen += 2+strlen(temp);
}



int _stdc trcmp(void *v1, void *v2)
{
  return (strcmp(((struct _table *)v1)->translation,
                 ((struct _table *)v2)->translation));
}

      
void Get_Number_String(FILE *bbsfile,char *string,int max_len)
{
  int ch,
      x;
  
  *string='\0';
  
  while ((ch=fgetc(bbsfile)) >= '0' && ch <= '9')
  {
    if ((x=strlen(string))==max_len)
      break;

    string[x]=(char)ch;
    string[x+1]='\0';

    fpos++;
  }

  ungetc(ch,bbsfile);
}


void P_Comment (struct _inf *inf) {NW(inf);}
void P_Copy    (struct _inf *inf) {NW(inf);}
void P_Include (struct _inf *inf) {NW(inf);}
void P_On      (struct _inf *inf) {NW(inf);}
void P_Save    (struct _inf *inf) {NW(inf);}
void P_Load    (struct _inf *inf) {NW(inf);}
void P_Fg      (struct _inf *inf) {NW(inf);}
void P_Bg      (struct _inf *inf) {NW(inf);}
void P_Steady  (struct _inf *inf) {NW(inf);}
void P_Dim     (struct _inf *inf) {NW(inf);}
void P_Bright  (struct _inf *inf) {NW(inf);}
void P_Priv    (struct _inf *inf) {NW(inf);}
void P_Label   (struct _inf *inf) {NW(inf);}
void P_Restore (struct _inf *inf) {NW(inf);}

void P_Locate(struct _inf *inf)
{
  int ch;
  char temp[PATHLEN];
  
  fpos += 2;
  
  ch=fgetc(inf->infile);
  
  sprintf(temp,"[locate %2d %2d]",ch,fgetc(inf->infile));
  fputs(temp,*inf->outfile);
  llen += strlen(temp);
}

void P_Menu_Cmd(struct _inf *inf)
{
  int ch;
  char temp[PATHLEN];
  int i;
  
  fpos += 2;
  
  ch=fgetc(inf->infile) << 8;
  ch |= fgetc(inf->infile);
  
  for (i=0; i < silt_table_size; i++)
    if (silt_table[i].opt==ch)
    {
      strcpy(temp, silt_table[i].token);
      break;
    }
    
  if (i==silt_table_size)
    sprintf(temp, "%u", ch);
      
  fprintf(*inf->outfile, "[menu_cmd %s]", temp);
  llen += strlen(temp)+11;
}

void P_Access(struct _inf *inf, char *tok)
{
  char temp[PATHLEN];
  int i;
  
  for (i=0; i < (PATHLEN-1) && (temp[i]=fgetc(inf->infile)) > 32 ; ++i)
    ++fpos;
  ++fpos;
  temp[i]='\0';
  fprintf(*inf->outfile, "[%s %s]", tok, temp);
  llen += strlen(temp)+strlen(tok)+3;
}

void P_Acsfile(struct _inf *inf)
{
  P_Access(inf, "acsfile");
}

void P_Acs(struct _inf *inf)
{
  P_Access(inf, "acs");
}


void P_Setpriv (struct _inf *inf)
{
  fpos++;

  fprintf(*inf->outfile,"[setpriv %c]",(char)toupper(fgetc(inf->infile)));
  llen += 11;
}

void P_Setattr (struct _inf *inf)
{
  fpos++;
    
  fprintf(*inf->outfile,"[msg_attr %c]",fgetc(inf->infile));
  llen += 12;
}

void P_Iftime  (struct _inf *inf)
{
  static char *time_verbs[]={"GT","LT","EQ","NE","GE","LE",NULL};

  char temp[PATHLEN],
       *s;
  
  int ch,
      x;
  
  fpos++;
  ch=fgetc(inf->infile)-1;
  
  for (x=0,s=NULL;time_verbs[x];x++)
    if (toupper(ch)==x)
    {
      s=time_verbs[x];
      break;
    }
    
  if (! s)
    s=time_verbs[0];
  
  fpos++;
  ch=fgetc(inf->infile);
  
  fpos++;
  
  sprintf(temp,"[iftime %s %02d:%02d]",s,ch-1,fgetc(inf->infile)-1);
  fputs(temp,*inf->outfile);
  llen += strlen(temp);
}


void P_Repeat(struct _inf *inf)
{
  int ch,
      x,n;

  fpos += 2;
  ch=fgetc(inf->infile);
  n=fgetc(inf->infile);

  for (x=0;x < n;x++)
  {
    if (ch=='[')  /* Double up any left brackets */
    {
      fputc('[',*inf->outfile);
      llen++;
    }

    fputc(ch,*inf->outfile);
    llen++;
  }
}

void P_Repeatseq(struct _inf *inf)
{
  char temp[PATHLEN];

  int ch;

  sprintf(temp,"[repeatseq %d",ch=fgetc(inf->infile));

  /* Increase fpos by number of chars, plus length, plus repeat times */
  fpos += ch+2;

  /* Increase ch by one, to grab the #-of-repeats byte */
  ch++;

  while (ch--)
    sprintf(&temp[strlen(temp)]," %d",fgetc(inf->infile));

  strcat(temp,"]");

  llen += strlen(temp);

  fputs(temp,*inf->outfile);
}




void P_Goto(struct _inf *inf)
{
  char string[6],
       temp[PATHLEN];
  long tl;
  int x;

  Get_Number_String(inf->infile,string,5);
  tl=atol(string);

  for (x=0;x < num_offsets;x++)
    if (tl==offsets[x])
    {
      sprintf(temp,"[goto L%d]",x);
      fputs(temp,*inf->outfile);
      llen += strlen(temp);
      break;
    }

  if (x==num_offsets)
  {
    fprintf(*inf->outfile,"[goto Unknown]");
    llen += 14;
  }
}


void P_Textsize(struct _inf *inf)
{
  int ch;
  char temp[PATHLEN];
  
  fpos += 2;
  
  ch=fgetc(inf->infile);
  
  sprintf(temp,"[textwin %2d %2d]",ch,fgetc(inf->infile));
  fputs(temp,*inf->outfile);
  llen += strlen(temp);
}

