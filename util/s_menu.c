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
static char rcs_id[]="$Id: s_menu.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Menu' processing logic
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
#include <share.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"

int tsearch(char *key,struct _st base[],unsigned int num);
static word near InsertMenuHeap(char *s);
static void near Init_Opt(struct _opt *opt);
static int near Parse_Option(option opt,char *line,struct _opt *thisopt,int menufile);
static void near Init_Menu(void);
static void near Free_Menu(void);
static option near IsOpt(char *token);


#define MENUHEAPLEN 4096

static char *menuheap;
static int menuofs;


int Parse_Menu(FILE *ctlfile,char *name)
{
  struct _opt thisopt;
  char temp[PATHLEN];         /* Scratch buffer */
  char line[MAX_LINE];        /* Contains entire line */
  char keyword[MAX_LINE];     /* First word on line */
  char firstname[PATHLEN];


  int menufile=0, x;
  option opt;

  
  linenum++;

  menuopt=0;

  Init_Menu();
  Init_Opt(&thisopt);

  if (! *name)
  {
    printf("\n\aInvalid menu statement in line %d of CTL file!\n",linenum);
    exit(1);
  }
  
  if (!done_sys || !done_equip || !done_matrix || !done_session)
  {
    printf("\n\n\aError!  All menu statements must come AFTER the System,\n");
    printf("Equipment, Matrix, and Session sections!  (Maybe MENUS.CTL\n");
    printf("was included in the wrong spot?)\n");
    exit(1);
  }

  if (do_menus)
  {
    getword(name, line, ctl_delim, 1);
    Add_Specific_Path(line, temp, strings+prm.menupath);
    strcat(temp,".mnu");
    strcpy(firstname, temp);

    if ((menufile=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY, 
                        SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
    {
      printf("\n\aError opening '%s' for write!\n",temp);
      exit(1);
    }

    /* Write the dummy menu structure.  We'll come back to update this      *
     * later.                                                               */
       
    write(menufile, (char *)&menu, sizeof(struct _menu));
  }
  
  while (fgets(line,MAX_LINE,ctlfile) != NULL)
  {
    Strip_Comment(line);

    if (! *line)
    {
      linenum++;
      continue;
    }

    priv_word=2;

    getword(line,keyword,ctl_delim,1);

TryAgain:

    if (! *keyword)
      ;
    else if (eqstri(keyword,"end"))
      break;
    else if (!do_menus)
      ;
    else if ((opt=IsOpt(keyword)) != 0)
    {
      Parse_Option(opt,line,&thisopt,menufile);
      menu.num_options++;
      Init_Opt(&thisopt);
    }
    else if (eqstri(keyword,"menuheader") ||
             eqstri(keyword,"silentmenuheader"))
    {
      if (eqstri(keyword,"silentmenuheader"))
        menu.flag |= MFLAG_SILENT;

      getword(line,keyword,ctl_delim,2);

      if (eqstri(keyword,"none"))
        menu.header=HEADER_NONE;
      else if (eqstri(keyword,"message"))
        menu.header=HEADER_MESSAGE;
      else if (eqstri(keyword,"file"))
        menu.header=HEADER_FILE;
      else if (eqstri(keyword,"change"))
        menu.header=HEADER_CHANGE;
      else if (eqstri(keyword,"chat"))
        menu.header=HEADER_CHAT;
      else Unknown_Ctl(linenum,keyword);
    }
    else if (eqstri(keyword,"title"))
      menu.title=InsertMenuHeap(fchar(line,ctl_delim,2));
    else if (eqstri(keyword,"menufile"))
    {
      getword(line,temp,ctl_delim,2);
      Add_Filename(temp);

      menu.dspfile=InsertMenuHeap(temp);

      x=3;

      do
      {
        getword(line,temp,ctl_delim,x++);

        if (*temp)
        {
          if (eqstri(temp,"novice"))
            menu.flag |= MFLAG_MF_NOVICE;
          else if (eqstri(temp,"regular"))
            menu.flag |= MFLAG_MF_REGULAR;
          else if (eqstri(temp,"expert"))
            menu.flag |= MFLAG_MF_EXPERT;
          else if (eqstri(temp,"rip"))
            menu.flag |= MFLAG_MF_RIP;
/*        else if (eqstri(temp,"hotflash"))
            menu.flag |= MFLAG_MF_HOTFLASH;*/
          else Unknown_Ctl(linenum,temp);
        }
      }
      while (*temp);

      /* If nothing was entered, display menu for all help types */

      if (x==4)
        menu.flag |= MFLAG_MF_ALL;
    }
    else if (eqstri(keyword,"headerfile"))
    {
      getword(line,temp,ctl_delim,2);
      /*Add_Filename(temp);*/
#ifdef UNIX
      fixPathMove(temp);
#endif
      menu.headfile=InsertMenuHeap(temp);

      x=3;

      do
      {
        getword(line,temp,ctl_delim,x++);

        if (*temp)
        {
          if (eqstri(temp,"novice"))
            menu.flag |= MFLAG_HF_NOVICE;
          else if (eqstri(temp,"regular"))
            menu.flag |= MFLAG_HF_REGULAR;
          else if (eqstri(temp,"expert"))
            menu.flag |= MFLAG_HF_EXPERT;
/*        else if (eqstri(temp,"hotflash"))
            menu.flag |= MFLAG_HF_HOTFLASH;*/
          else if (eqstri(temp,"rip"))
            menu.flag |= MFLAG_HF_RIP;
          else Unknown_Ctl(linenum,keyword);
        }
      }
      while (*temp);

      /* If nothing was entered, display menu for all help types */

      if (x==4)
        menu.flag |= MFLAG_HF_ALL;
    }
    else if (eqstri(keyword,"menulength"))
      menu.menu_length=atoi(fchar(line,ctl_delim,2));
    else if (eqstri(keyword,"optionwidth"))
    {
      menu.opt_width=atoi(fchar(line,ctl_delim,2));
      if (menu.opt_width < 6)
        menu.opt_width=6;
      else if (menu.opt_width > 80)
        menu.opt_width=80;
    }
    else if (eqstri(keyword,"menucolour") || eqstri(keyword,"menucolor"))
      menu.hot_colour=atoi(fchar(line,ctl_delim,2));
    else if (eqstri(keyword,"reset"))
      menu.flag |= MFLAG_RESET;
    else if (eqstri(keyword,"app") || eqstri(keyword,"application"))
      ;
    else
    {
      static struct _flags
      {
        char *keyword;
        int flag_or_type;
        int value;
      }  flags[]={{"ctl"       , 1,  OFLAG_CTL               },
                  {"nodsp"     , 1,  OFLAG_NODSP             },
                  {"nocls"     , 1,  OFLAG_NOCLS             },
                  {"then"      , 1,  OFLAG_THEN | OFLAG_NODSP},
                  {"stay"      , 1,  OFLAG_STAY              },
                  {"else"      , 1,  OFLAG_ELSE | OFLAG_NODSP},
                  {"usrlocal"  , 1,  OFLAG_ULOCAL            },
                  {"usrremote" , 1,  OFLAG_UREMOTE           },
                  {"else"      , 1,  OFLAG_ELSE | OFLAG_NODSP},
                  {"reread"    , 1,  OFLAG_REREAD            },
                  {"rip"       , 1,  OFLAG_RIP               },
                  {"norip"     , 1,  OFLAG_NORIP             },
                  {"local"     , 0,  AREATYPE_LOCAL          },
                  {"echo"      , 0,  AREATYPE_ECHO           },
                  {"matrix"    , 0,  AREATYPE_MATRIX         },
                  {"conf"      , 0,  AREATYPE_CONF           },
                  {NULL,0,0}};

      for (x=0;flags[x].keyword;x++)
        if (eqstri(keyword,flags[x].keyword))
        {
          if (flags[x].flag_or_type==0)
          {
            if (thisopt.areatype==AREATYPE_ALL)
              thisopt.areatype=0;

            thisopt.areatype |= flags[x].value;
          }
          else thisopt.flag |= flags[x].value;

          getword(line, keyword, ctl_delim, priv_word);
          priv_word++;
          goto TryAgain;
        }

      /* If we got this far, it must have been invalid */

      Unknown_Ctl(linenum,keyword);
    }


    linenum++;
  }

  linenum++;

  if (do_menus)
  {
    char *s;
    
    lseek(menufile,0L,SEEK_SET);
    
    write(menufile, (char *)&menu, sizeof(struct _menu));
    lseek(menufile,0L,SEEK_END);
    write(menufile, menuheap, menuofs);
    close(menufile);
    
    /* Now make copies of the .MNU file, if any were requested */

    if ((s=strtok(name, ctl_delim)) != NULL)
    {
      while ((s=strtok(NULL, ctl_delim)) != NULL)
      {
        Add_Specific_Path(s, temp, strings+prm.menupath);
#ifndef UNIX
        strcat(temp, ".MNU");
#else
        strcat(temp, ".mnu");
#endif	        

        lcopy(firstname, temp);
      }
    }
  }

  Free_Menu();
  return 0;
}

/* I don't know why, but TC's bloody bsearch() function is broken!  It     *
 * wouldn't work for the `userlist' variable, and it appeared to be        *
 * skipping over it for some reason.  At any rate, this function           *
 * does approximately the same thing, except without the function          *
 * pointers and associated overhead.                                       */

int tsearch(char *key,struct _st base[],unsigned int num)
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

    for (s=key,t=base[x].token;tolower(*s)==tolower(*t);s++,t++)
      if (! *s)
        return (x);                       /* Found a match */

    if (tolower(*s) > tolower(*t))
      lastlo=x;
    else
      lasthi=x;
  }
}


static void near Init_Menu(void)
{
  memset(&menu,'\0',sizeof(struct _menu));

  if ((menuheap=malloc(MENUHEAPLEN))==NULL)
    NoMem();

  menuheap[0]='\0';
  menuofs=1;

  menu.opt_width=20;
  menu.header=HEADER_NONE;
  menu.hot_colour=-1;

  return;
}

static void near Free_Menu(void)
{
  free(menuheap);
}





static int near Parse_Option(option opt,char *line,struct _opt *thisopt,int menufile)
{
  static char temp[MAX_LINE];
  static char arg[MAX_LINE];
  char *p;

  thisopt->type=opt;

  if (opt==display_menu || opt==display_file || opt==xtern_erlvl ||
      opt==xtern_run    || opt==xtern_dos    || opt==xtern_chain ||
      opt==xtern_concur || opt==link_menu    || opt==key_poke    ||
      opt==o_if         || opt==o_menupath   || opt==mex         ||
      opt==msg_reply_area)
  {
    getword(line, temp, ctl_delim, priv_word++);

    if (opt==display_file)
    {
      strcpy(arg,temp);
      Add_Filename(arg);
    }
    else if (opt==msg_reply_area && *temp=='.') /* Special case */
      *arg='\0';                     /* User selects reply area */
    else strcpy(arg,temp);

    thisopt->arg=InsertMenuHeap(arg);
  }

  getword(line,temp,ctl_delim,priv_word++);


  
  /* Parse out the priv and locks */
  
  thisopt->priv=InsertMenuHeap(temp);

  /*thisopt->priv=Deduce_Priv(temp);
  thisopt->lock=Deduce_Lock(temp);*/


  
  /* Now figure out the name of the menu option */

  p=fchar(line,ctl_delim,priv_word);

  getword(p,temp,"\"",1);

/*temp[18]='\0';*/
  thisopt->name=InsertMenuHeap(temp);


  
  /* Grab the keypoke string */
  
  getword(p,temp,"\"",3);
  
  if (*temp)
    thisopt->keypoke=InsertMenuHeap(temp);


  
  /* Write this option to the control file, if necessary */

  if (do_menus)
    write(menufile, (char *)thisopt, sizeof(struct _opt));
  
  return 0;
}


static void near Init_Opt(struct _opt *opt)
{
  memset(opt,'\0',sizeof(struct _opt));
  
  opt->type=nothing;
  opt->areatype=AREATYPE_ALL;
  opt->priv=0;
}

static word near InsertMenuHeap(char *s)
{
  int len=strlen(s);
  word oldofs;
  
  if (menuofs+len >= MENUHEAPLEN)
  {
    printf("Error!  Menu file too big (option '%s')\n",s);
    exit(1);
  }
  
  strcpy(menuheap+menuofs, s);
  oldofs=menuofs;
  menuofs += strlen(s)+1;
  return oldofs;
}

static option near IsOpt(char *token)
{
  int x;

  if ((x=tsearch(token,silt_table,silt_table_size)) != -1)
    return silt_table[x].opt;
  else return 0;
}


