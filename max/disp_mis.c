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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: disp_mis.c,v 1.3 2004/01/27 21:00:26 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=.BBS-file miscellaneous support functions
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
#include "mm.h"
#include "max_area.h"
#include "max_menu.h"
#include "display.h"


char * Ordinal(long number)
{
  number %= 100L;

  if (number > 10L && number < 20L)
    return ordinal_th;

  switch(number % 10L)
  {
    case 1L:
      return ordinal_st;

    case 2L:
      return ordinal_nd;

    case 3L:
      return ordinal_rd;

    default:
      return ordinal_th;
  }
}



#ifdef NEVER /* obsolete */

/* Keep getting from a file until the next blank/extended ASCII */

void Get_To_Blank(char *s,FILE *f)
{
  char *orig;
  int c, s1, s2;

  orig=s;

  while (((c=fgetc(f)) > 32) || c==25)
  {
    if (c==25)  /* Expand any RLE sequences */
    {
      s1=fgetc(f);
      s2=fgetc(f);

      while (s2--)  /* Expand the character 's1', a total of 's2' times */
        *s++=(char)s1;

      if (! (s1 > 32))  /* Break out of loop if RLE'd char is a space */
        break;
    }
    else *s++=(char)c;
  }

  /* Get rid of the LF after the CR */
  if (c==13)
    fgetc(f);

  *s='\0';

  /* Strip all trailing blanks */
  for (s--;s >= orig && *s <= 32;s--)
    *s='\0';

}
#endif


void Fix_RLE(char *s)
{
  char *temp;
  char *p;
  char *orig=s;

  int s1, s2;

  if ((temp=malloc(PATHLEN*3))==NULL)
    return;

  p=temp;

  if (strlen(s) >= PATHLEN)
    s[PATHLEN-1]='\0';

  while (*s)
  {
    if (*s==25)  /* Expand any RLE sequences */
    {
      s1=*++s;

      if (s1)
      {
        s2=*++s;

        if (s2)
        {
          while (s2--)  /* Expand the character 's1', a total of 's2' times */
            *p++=(char)s1;
        
          s++;
        }
      }
    }
    else *p++=*s++;
  }

  *p='\0';

  strcpy(orig,temp);
  free(temp);
}



#ifdef NEVER /* obsolete */
char * Get_BBS_String(char *string,FILE *bbsfile,int max)
{
  fgets(string,max,bbsfile);
  Fix_RLE(string);
  Trim_Line(string);
  
  return string;
}
#endif



void Add_Full_Path(char *src,char *dest)
{
  char *temp;
  
  if ((temp=malloc(PATHLEN))==NULL)
    return;

  if (src[1]==':' || *src=='\\' || *src=='/' || *src=='.')
    strcpy(dest,src);
  else sprintf(dest,ss,original_path,src);

  if (dest[1] != ':')
  {
    temp[0]=*original_path;
    temp[1]=':';
    temp[2]='\0';
    strcat(temp,dest);
    strcpy(dest,temp);
  }
  
  free(temp);
}



word Priv_Code(int ch,int action)
{
  word priv;

  if (ch >= '1' && ch <= '8')
  {
    if (action=='Q')
    {
      if (! UserHasKey(ch-'1'))
        return SKIP_LINE;
      else return SKIP_NONE;
    }
    else if (action=='X')
    {
      if (UserHasKey(ch-'1'))
        return SKIP_LINE;
      else return SKIP_NONE;
    }
    else return SKIP_LINE;
  }

  priv=ClassKeyLevel(toupper(ch));

  switch (action)
  {
    case 'Q':                   /* Equal to priv <x> */
      return (usr.priv==priv) ? SKIP_NONE : SKIP_LINE;

    case 'X':                   /* NOT equal to priv <x> */
      return (usr.priv != priv) ? SKIP_NONE : SKIP_LINE;

    case 'B':                   /* Below priv <x> */
      return (LEPriv(usr.priv,priv)) ? SKIP_NONE : SKIP_LINE;

    case 'L':                   /* Greater/equal to priv <x> */
      return (GEPriv(usr.priv,priv)) ? SKIP_NONE : SKIP_LINE;

  }

  /* ? */

  return SKIP_NONE;
}





#ifndef ORACLE

/* Run a menu option from a .mec file (with [menu_opt]) or the
 * menu_cmd function in MEX.
 */

int BbsRunOpt(option opt_type, char *arg)
{
  struct _amenu *menu;
  struct _opt *opt;
  unsigned uiFlag;
  char *p;

  /* Allocate memory for the menu structure itself */
  
  if ((menu=malloc(sizeof(struct _amenu)))==NULL)
    return -1;
  
  memset(menu,'\0',sizeof(struct _amenu));

  /* Allocate memory for the first (and only) menu option */
  
  if ((opt=menu->opt=malloc(sizeof(struct _opt)))==NULL)
  {
    free(menu);
    return -1;
  }

  /* Allocate memory for the "heap", to hold our single string */
  
  if ((menu->menuheap=malloc(strlen(arg)+PATHLEN))==NULL)
  {
    free(opt);
    free(menu);
    return -1;
  }


  /* Initialize menu option structure */
  
  memset(opt, '\0', sizeof(struct _opt));


  /* Now fix up the menu for this virtual option */

  menu->m.header=HEADER_NONE;
  menu->m.num_options=1;
  menu->m.hot_colour=-1;


  /* First byte of menu is always zero.  */

  p=menu->menuheap;
  *p++='\0';


  /* Add the argument for this option */

  opt->arg = p - menu->menuheap;
  strcpy(p, arg);
  p += strlen(p)+1;

  /* Add in the priv level for this option */

  strcpy(p, ClassDesc(cls));

  opt->type=opt_type;
  opt->priv=p - menu->menuheap;
  opt->areatype=AREATYPE_ALL;
  
  RunOption(menu, opt, 0, NULL, &uiFlag, "");
  
  free(menu->menuheap);
  free(menu->opt);
  free(menu);

  return 0;
}

#endif

