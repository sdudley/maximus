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
static char rcs_id[]="$Id: s_misc.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: Miscellaneous routines
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"

void Strip_Comment(char *l)
{
  char *sp=l;
  int x;

  while (*sp)
  {
    if (isalpha(*sp))
      break;
    else if (ispunct(*sp))
    {
      *sp='\0';
      break;
    }

    sp++;
  }

  if (l[x=strlen(l)-1]=='\n')
    l[x]='\0';
}

void Add_Path(char *s,int warn)
{
  int x=0;

  char temp[MAX_LINE];
#if 0
       temp2[MAX_LINE];
#endif
  if (!s)
    return;

   while (*s && (s[x=strlen(s)-1]==' ' || s[x]=='\t'))
    s[x]='\0';

  if (*s && s[x] != PATH_DELIM)
  {
    s[++x]=PATH_DELIM;
    s[++x]='\0';
  }

#if 0
  if (prm.sys_path)
  {
    if (! (s[1]==':' || *s=='\\' || *s=='/'))
    {
      strcpy(temp,strings+prm.sys_path);
      strcat(temp,s);
    }
    else strcpy(temp,s);

    if (temp[1] != ':')
    {
      temp2[0]=*(strings+prm.sys_path);
      temp2[1]=':';
      temp2[2]='\0';
      strcat(temp2,temp);
      strcpy(temp,temp2);
    }
  }
  else
#endif
  strcpy(temp,s);

  /* If no path, default to the system path */

  if (! *temp)
    strcpy(temp, strings+prm.sys_path);

  if (warn && !direxist(temp))
  {
    printf("\nWarning!  Path `%s' does not exist!\n",fancy_fn(temp));
    Compiling(-1,NULL,NULL);
  }

  strcpy(s,temp);
}


void Add_Filename(char *s)
{
  int x;

  char temp[MAX_LINE];
#if 0
       temp2[MAX_LINE];
#endif

  if (!s)
    return;

   while (*s && (s[x=strlen(s)-1]==' ' || s[x]=='\t'))
    s[x]='\0';

#if 0
  if (prm.sys_path)
  {
    if (! (s[1]==':' || *s=='\\' || *s=='/'))
    {
      strcpy(temp,strings+prm.sys_path);
      strcat(temp,s);
    }
    else strcpy(temp,s);

    if (temp[1] != ':')
    {
      temp2[0]=*(strings+prm.sys_path);
      temp2[1]=':';
      temp2[2]='\0';
      strcat(temp2,temp);
      strcpy(temp,temp2);
    }
  }
  else
#endif
  strcpy(temp,s);
#ifdef UNIX
  fixPathMove(temp);
#endif
  strcpy(s,temp);
}


/* Front end to firstchar() routine -- If word isn't found, return a       *
 * pointer to a NUL.                                                       */

char *fchar(char *str,char *delim,int wordno)
{
  char *s;

  if ((s=firstchar(str,delim,wordno))==NULL)
    return str+strlen(str);
  else return s;
}

#if 0
byte MaxPrivToOpus(int maxpriv)
{
  switch(maxpriv)
  {
    case SYSOP:
      return _SYSOP;

    case ASSTSYSOP:
      return _ASSTSYSOP;

    case CLERK:
      return _CLERK;

    case EXTRA:
      return _EXTRA;

    case FAVORED:
      return _FAVORED;

    case PRIVIL:
      return _PRIVEL;

    case WORTHY:
      return _WORTHY;

    case NORMAL:
      return _NORMAL;

    case LIMITED:
      return _LIMITED;

    case DISGRACE:
      return _DISGRACE;

    case TWIT:
      return _TWIT;
  }

  return _HIDDEN;
}
#endif

void assert_dir(char *path)
{
  if (!direxist(path))
    makedir(path);
}

int makedir(char *d)
{
  char temp[PATHLEN];
  int x;

  for (*temp='\0';;)
  {
    if (!do_unattended)
    {
      printf("\rCreate dir `%s' [Y,n]? ",d);

      fgets(temp,PATHLEN,stdin);

      Strip_Trailing(temp, '\n');

      *temp=(char)toupper(*temp);

      Compiling(-1, NULL, NULL);
    }

    if (do_unattended || *temp=='Y' || *temp=='\n' || *temp=='\0')
    {
      char *fn;

      printf("\rCreating directory `%s'\n", d);
      Compiling(-1, NULL, NULL);

      fn=sstrdup(d);

      Strip_Trailing(fn, PATH_DELIM);

      x=make_dir(fn);
      free(fn);
      return x;
    }
    else if (*temp=='N')
      return 0;
  }
}

#if 0
int Deduce_Priv(char *p)
{
  if (strnicmp(p,"hidden",6)==0)
    return HIDDEN;
  else if (strnicmp(p,"sysop",5)==0)
    return SYSOP;
  else if (strnicmp(p,"asstsysop",9)==0)
    return ASSTSYSOP;
  else if (strnicmp(p,"clerk",5)==0)
    return CLERK;
  else if (strnicmp(p,"extra",5)==0)
    return EXTRA;
  else if (strnicmp(p,"favored",5)==0)
    return FAVORED;
  else if (strnicmp(p,"privel",6)==0 || strnicmp(p,"privil",6)==0)
    return PRIVIL;
  else if (strnicmp(p,"worthy",6)==0)
    return WORTHY;
  else if (strnicmp(p,"normal",6)==0)
    return NORMAL;
  else if (strnicmp(p,"limited",7)==0)
    return LIMITED;
  else if (strnicmp(p,"disgrace",8)==0)
    return DISGRACE;
  else if (strnicmp(p,"twit",4)==0)
    return TWIT;
  else
  {
    printf("\a\nInvalid priv level `%s' on line %d of CTL file!\n",p,linenum);
    exit(1);
  }
  return 0;
}
#endif


int Deduce_Attribute(char *a)
{
  if (eqstri(a,"private"))
    return 0;
  else if (eqstri(a,"crash"))
    return 1;
  else if (eqstri(a,"fileattach"))
    return 4;
  else if (eqstri(a,"killsent"))
    return 7;
  else if (eqstri(a,"hold"))
    return 9;
  else if (eqstri(a,"filerequest"))
    return 11;
  else if (eqstri(a,"updaterequest"))
    return 15;
  else if (eqstri(a,"fromfile"))
    return -22;
  else if (eqstri(a,"localattach"))
    return -20;
  else
  {
    printf("\a\nInvalid attribute `%s' on line %d of CTL file!\n",a,linenum);
    exit(1);
  }
  return 0;
}


#if 0
int Deduce_Class(int priv)
{
  int x;

  for (x=0;x < num_class;x++)
    if (prm.cls[x].priv==priv)
      break;

  prm.cls[x].priv=priv;

  if (x >= num_class)
    num_class++;

  return x;
}
#endif

void Unknown_Ctl(int linenum,char *p)
{
  printf("\n\aUnknown statement in line %d of `%s':  `%s'?\n",linenum,
         ctl_name,p);
  Compiling(-1,NULL,NULL);
}


int Compiling(char type,char *string,char *name)
{
  static char last_string[80],
              last_name[80];

  static int last_length=0;
  int x;

  if (type==-1)
    printf(last_string,last_name);
  else
  {
    if (last != type)   /* A NEW section */
    {
      putchar('\n');
      printf(string,name);
    }
    else
    {
      for (x=0;x < last_length;x++)
        printf("\x08 \x08");

      printf("%s",name);
    }

    last=type;
    last_length=strlen(name);
    strcpy(last_string,string);
    strcpy(last_name,name);
  }

  return 0;
}



int Add_Backslash(char *s)
{
  int x;

  if (*s && s[x=strlen(s)-1] != PATH_DELIM)
  {
    s[++x]=PATH_DELIM;
    s[++x]='\0';
  }

  return 0;
}



int Remove_Backslash(char *s)
{
  int x;

  if (*s && s[x=strlen(s)-1]==PATH_DELIM && x != 2)
    s[x]='\0';

  return 0;
}


int Parse_Weekday(char *s)
{
  int x;

  for (x=0;x < 7;x++)
    if (eqstri(s,weekday[x]))
      return x;

  if (eqstri(s,"all"))
    return 7;

  printf("\n\aInvalid day-of-week `%s' on line %d of CTL file!\n",s,linenum);
  exit(1);
  return 0;
}



dword Deduce_Lock(char *p)
{
  char *orig = p;
  int ch;
  dword lock;

  lock=0;

  /* Look for the slash in the string... */

  p=strchr(p,'/');

  /* If there *was* a slash, there must be some locks here... */

  if (p)
  {
    while (*++p)
    {
      ch=toupper(*p);
      
      if (ch >= '1' && ch <= '8')
        lock |= (1L << (long)(ch-'1'));
      else if (ch >= 'A' && ch <= 'X')
        lock |= (1L << (long)((ch-'A')+8));
      else if (ch=='!')
      {
        printf("\nWarning for ACS '%s' - the 'not key' flag, '!', is not supported\n"
               "when writing Max 2.x-compatible privilege level information.\n",
               orig);
        p++;
      }
      else if (ch != ' ' && ch != '\t')
      {
        printf("\n\aInvalid lock `%c' in \"%s\" on line %d of CTL file!\n",
               *p,
               orig,
               linenum);
        exit(1);
      }
    }
  }

  return lock;
}



void Add_Specific_Path(char *frompath,char *topath,char *add_path)
{
  if (!prm.menupath || strchr(frompath,':') || strchr(frompath,'\\') ||
      strchr(frompath,'/'))

  {
    strcpy(topath,frompath);
    return;
  }

  strcpy(topath,add_path);
  strcat(topath,frompath);
}

int Make_Strng(char *value,int type)
{
  int x;

  char temp[MAX_LINE],
       *s;

  if (value==NULL)
    s="";
  else s=value;

  while (*s && (s[x=strlen(s)-1]==' ' || s[x]=='\t'))
    s[x]='\0';

  if (type)
  {
    if (!prm.sys_path)
    {
      printf("\n\aError in line %d of control file!  The `Path System' variable MUST be\nspecified before any other filenames or paths!\n",linenum);
      exit(1);
    }

    x=(*s == ':') ? 2 : 1;    /* Allow for MEX scripts */
    if (! (s[x]==':' || s[x-1]=='\\' || s[x-1]=='/'))
    {
      if (x==2)
        strcpy(temp,":");
      else *temp='\0';
      strcat(temp,strings+prm.sys_path);
      strcat(temp,s+x-1);
    }
    else strcpy(temp,s);
  }
  else strcpy(temp,s);

  return (Add_To_Heap(temp,FALSE));
}



int Make_Pth(char *value)
{
  int x=0;

  char temp[MAX_LINE],
       *s;

  if (value==NULL)
    s="";
  else s=value;

  while (*s && (s[x=strlen(s)-1]==' ' || s[x]=='\t'))
    s[x]='\0';

  if (*s && s[x] != PATH_DELIM)
  {
    s[++x]=PATH_DELIM;
    s[++x]='\0';
  }

  strcpy(temp,s);
  Add_Path(temp,1);

  return (Add_To_Heap(temp,TRUE));
}


int Add_To_Heap(char *s,int fancy)
{
  int old_ofs;
  char *o;

  /* If string is already in table, no need to add! */

  if ((o=memstr(strings,s,offset,strlen(s)+1)) != NULL)
    old_ofs=o-strings;
  else
  {
    strcpy(strings+offset,s);
    old_ofs=offset;
    offset += strlen(s)+1;

    if (fancy)
      fancy_fn(strings+old_ofs);
  }

  if (offset >= HEAP_SIZE)
  {
    printf("\n\aRan out of memory while adding `%s' to .PRM heap.\n",s);
    exit(1);
  }

  return old_ofs;
}


#if 0 /* obsolete */

void Attrib_Or(int clnum,int attr,struct _area *area)
{
  int x;

  if (clnum==-1)
  {
    for (x=0;x < MAXCLASS;x++)
      area->attrib[x] |= attr;
  }
  else area->attrib[clnum] |= attr;
}

void Attrib_And(int clnum,int attr,struct _area *area)
{
  int x;

  if (clnum==-1)
  {
    for (x=0;x < MAXCLASS;x++)
      area->attrib[x] &= attr;
  }
  else area->attrib[clnum] &= attr;
}


int Blank_Sys(struct _sys *sys,int mode)
{
  if (mode==0)
  {
    sys->ls_caller=0;
    sys->quote_pos=0L;
  }

  sys->priv=HIDDEN;
  sys->attrib=0;
  sys->lock=0;

  memset(sys->msgpath, '\0', sizeof(sys->msgpath));
  memset(sys->uppath, '\0', sizeof(sys->uppath));
  memset(sys->filepath, '\0', sizeof(sys->filepath));
  memset(sys->bbspath, '\0', sizeof(sys->bbspath));
  memset(sys->hlppath, '\0', sizeof(sys->hlppath));

  strcpy(sys->bbspath, strings+prm.menupath);
  strcpy(sys->hlppath, strings+prm.sys_path);
  strcat(sys->hlppath, "Hlp" PATH_DELIMS);

  return 0;
}


void Blank_Area(struct _area *area)
{
  memset(area,'\0',sizeof(struct _area));

  area->id=AREA_id;
  area->struct_len=sizeof(struct _area);
  area->type=MSGTYPE_SDM;

  /*strcpy(area->origin,strings+prm.system_name);*/

  area->msgpriv=area->filepriv=HIDDEN;
  area->origin_aka=-1;
}

#endif



void NoMem(void)
{
  printf("\aRan out of memory!\n");
  exit(1);
}


void ErrWrite(void)
{
  printf("Error writing to area data file!\n");
  exit(1);
}


/* Take the given line from the config file and call appropriate handler. */
/* Returns true if to break out of loop.                                  */

int VerbParse(void *pfi, struct _vbtab *verbs, char *line)
{
  char *words[MAX_PARSE_WORDS], *p;
  char szFirstWord[PATHLEN];
  struct _vbtab *pvt;
  int w, rc;

  rc=FALSE;

  Strip_Trailing(line, '\n');

  if ((p=strchr(line, '%')) != NULL)
    *p='\0';

  if ((p=strchr(line, ';')) != NULL)
    *p='\0';

  /* Speed up parsing if we're only looking for the first word */

  if (!verbs)
  {
    words[0]=szFirstWord;
    getword(line, words[0], ctl_delim, 1);
  }
  else
  {
    for (w=0; w < MAX_PARSE_WORDS; w++)
    {
      if ((words[w]=malloc(PATHLEN))==NULL)
        NoMem();

      if (w==0 || *words[w-1])
        getword(line, words[w], ctl_delim, w+1);
      else
        *words[w]=0;
    }
  }

  /* Find this in verb table */

  if (verbs)
    for (pvt=verbs; pvt->verb && !eqstri(words[0], pvt->verb); pvt++)
      ;

  /* If we didn't get a match */

  if (!verbs || !pvt->verb)
  {
    if (eqstri(words[0], "end"))
      rc=TRUE;
    else if (*words[0] && verbs)
      Unknown_Ctl(linenum, words[0]);
  }
  else
  {
    /* If we have a filter function, call it with a ptr to the second word */

    if (pvt->f)
      (*pvt->f)(pfi, words, fchar(line, ctl_delim, 2));

    /* Add this to a zstr heap, if necessary */

    if (pvt->pzstr)
      HeapAdd(&PFI(pfi)->h, pvt->pzstr, fchar(line, ctl_delim, 2));
  }

  /* Deallocate the space used for these words */

  if (verbs)
    for (w=0; w < MAX_PARSE_WORDS; w++)
      free(words[w]);

  linenum++;

  return rc;
}


/* Process paths -- make sure that they end with a trailing backslash */

void near FiltPath(void *v, char *words[], char *line)
{
  int len = strlen(line);

  NW(words);
  NW(v);

  while (len && line[len-1]==' ')
    --len;

  line[len] = 0;

/*  strcpy(line, make_fullfname(line));*/
  Add_Trailing(line, PATH_DELIM);
}


/* Process message command overrides */

void near FiltOverride(void *v, char *words[], char *line)
{
  OVRLIST ol=malloc(sizeof(*ol));
  int idx;

  NW(line);

  if (!ol)
    NoMem();

/* 0        1       2         3          4 */
/* override message msg_reply sysop/1234 R */

  if (!*words[1])
  {
    printf("Error!  No menu name specified for override on line %d!\n", linenum);
    return;
  }

  if (!*words[3])
  {
    printf("Error!  No access control string specified for override on line %d!\n", linenum);
    return;
  }

  if ((idx=tsearch(words[2], silt_table, silt_table_size)) != -1)
    ol->or.opt=silt_table[idx].opt;
  else Unknown_Ctl(linenum, words[2]);

  if (*words[4])
    ol->or.name=*words[4];
  else ol->or.name=0;

  if (PFI(v)->marea)
  {
    HeapAdd(&PMI(v)->h, &ol->or.menuname, words[1]);
    HeapAdd(&PMI(v)->h, &ol->or.acs, words[3]);

    /* Add this to the linked list of override options */

    ol->next=PMI(v)->ol;
    PMI(v)->ol=ol;

    PMI(v)->ma.num_override++;
  }
  else
  {
    HeapAdd(&PFI(v)->h, &ol->or.menuname, words[1]);
    HeapAdd(&PFI(v)->h, &ol->or.acs, words[3]);

    /* Add this to the linked list of override options */

    ol->next=PFI(v)->ol;
    PFI(v)->ol=ol;

    PFI(v)->fa.num_override++;
  }
}

void near FiltMenuname(void *v, char *words[], char *line)
{
  NW(line);

  if (! *words[1] || !*words[2])
    printf("\n\aError!  Two arguments must be specified for the MenuName keyword!\n");

  if (PFI(v)->marea)
  {
    HeapAdd(&PMI(v)->h, &PMI(v)->ma.menureplace, words[1]);
    HeapAdd(&PMI(v)->h, &PMI(v)->ma.menuname, words[2]);
  }
  else
  {
    HeapAdd(&PFI(v)->h, &PFI(v)->fa.menureplace, words[1]);
    HeapAdd(&PFI(v)->h, &PFI(v)->fa.menuname, words[2]);
  }
}

void BadDivisionName(void)
{
  printf("\n\aError!  Invalid character ('.') in area/division name on line %d!\n", linenum);
  Compiling(-1, NULL, NULL);
}


