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

/*# name=MECCA include file
*/

#define DLE 16
#define PATHDELIM ".\\/"
#define TOKENDELIM ",; \t\n"    /* Position of , at start is significant */
#define MAX_TOKENLEN 2000
#define MAX_OFFSETS 150
#define MAX_SYMBOLLEN 64
#define INOUT_BUFFER 8192
#define MAX_LLEN 100

#define LESS -1
#define EQUAL 0
#define MORE 1
#define MAX_SYM 1000

#define Compress_Sequence()                                   \
    {                                                         \
      if (lastch != -1)                                       \
      {                                                       \
        if (lastcount <= 2)                                   \
        {                                                     \
          for (x=0;x <= lastcount;x++)                        \
            putc(lastch,outfile);                             \
        }                                                     \
        else if (lastcount+1==26 || lastcount+1==10 || lastcount+1==13) \
          fprintf(outfile,"%c\x19%c%c",lastch,lastch,lastcount); \
        else fprintf(outfile,"\x19%c%c",lastch,lastcount+1);  \
                                                              \
        lastcount=0;                                          \
        lastch=-1;                                            \
      }                                                       \
                                                              \
      if (lastch != -1)                                       \
        putc(lastch,outfile);                                 \
                                                              \
      lastch=ch;                                              \
    }


/* Structure to hold symbol locations, and real offsets.  Internal to
   Compile()...                                                       */

struct _symbol
{
  char name[MAX_SYMBOLLEN];
  long location;
  long offset[MAX_OFFSETS];
  int offsetnum;
};


/* Structure for verb table, below */
struct _inf;

struct _table
{
  char *verb;
  int verbno;
  int colour;
  char *translation;
  void (*fptr)(struct _inf *inf);
};

struct _inf
{
  char **p;
  char *outname;
  char *inname;
     
  int verbnum;
     
  FILE **outfile;
  FILE *infile;
};


void NoMem(void);
void Parse_Args(int argc,char *argv[]);
void Compile(char *inname,char *outname,int mode,int time_comp);
word sssearch(char *key,struct _table base[],unsigned int num);
void Init_Table(void);
int GEdate(union stamp_combo *s1,union stamp_combo *s2);
int Colour_Num(char *name);
void Put_Colour(FILE *outfile,int col);
int isdigitstr(char *s);
void Invert_Logical(word *verbnum);
int IsValidTimeVerb(word verbnum);
word Determine_Type(char *s);
void Output_Code(word verbnum,FILE *outfile,int *lcol);
void Process_Label(char *p,int define,FILE *outfile);
void Parse_Token(char *outname,FILE **outfile, char *inname);

void P_Comment (struct _inf *inf);
void P_Copy    (struct _inf *inf);
void P_Include (struct _inf *inf);
void P_On      (struct _inf *inf);
void P_Save    (struct _inf *inf);
void P_Load    (struct _inf *inf);
void P_Fg      (struct _inf *inf);
void P_Bg      (struct _inf *inf);
void P_Steady  (struct _inf *inf);
void P_Dim     (struct _inf *inf);
void P_Bright  (struct _inf *inf);
void P_Priv    (struct _inf *inf);
void P_Label   (struct _inf *inf);
void P_Restore (struct _inf *inf);
void P_Setpriv (struct _inf *inf);
void P_Setattr (struct _inf *inf);
void P_Iftime  (struct _inf *inf);
void P_Repeat  (struct _inf *inf);
void P_Repeatseq(struct _inf *inf);
void P_Goto    (struct _inf *inf);
void P_Locate  (struct _inf *inf);
void P_Menu_Cmd(struct _inf *inf);
void P_Textsize(struct _inf *inf);
void P_Acs     (struct _inf *inf);
void P_Acsfile (struct _inf *inf);
 
#define NUM_NUMS 13   /* Number of numbered items */

#ifdef MECCA_INIT

struct _table verbs[]=
{

  /* Sorted table of MECCA verbs.  After editing to add a new verb, just   *
   * run QSORT.EXE over the table, and recompile.                          */

  #include "mecca_vb.h"
  
  {"ZZZZZZZZZZ",       0,   -1, "",                 NULL}
};

int verb_table_size=(sizeof(verbs)/sizeof(verbs[0]));

int num_gt,num_lt,num_eq,num_ne,num_ge,num_le,num_above,num_below,num_equal,
    num_unequal,num_notequal,num_ae,num_be;

#else /* !MECCA_INIT */

extern struct _table verbs[];

extern int verb_table_size;

extern int num_gt,num_lt,num_eq,num_ne,num_ge,num_le,num_above,num_below,
           num_equal,num_unequal,num_notequal,num_ae,num_be;

#endif

