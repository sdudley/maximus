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

/* $Id: m_browse.c,v 1.3 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=Message Section: B)rowse command
*/

#define MAX_LANG_m_browse

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <mem.h>
#include "dr.h"
#include "prog.h"
#include "max_msg.h"
#include "m_browse.h"

extern int last_title;
extern int idling;

/* name for browse help file */

static char browse_fname[]="%sbrowse";

static int near Browse_Get_Options(BROWSE *b);
static void near Deinit_Search(SEARCH *s);


int Msg_Browse(word newb, SEARCH *newfirst, char *menuname)
{
  SEARCH olds;
  BROWSE b;
  int ret;
  char nonstop=FALSE;
 
  in_mcheck=TRUE;
  b.bflag=newb;
  ret=0;
  idling=0;
  last_title=FALSE;
  display_line=display_col=1;

  if (newfirst)
    b.first=newfirst;
  else Init_Search(b.first=&olds);

  b.nonstop=&nonstop;
  b.matched=0;
  b.menuname=menuname;
  b.wildcard=NULL;
  b.fSilent=FALSE;
  
  if (Browse_Get_Options(&b)==-1)
    ret=-1;
  else
  {
    /* Save and restore the current message area */

    if (!PushMsgArea(usr.msg, 0))
      ret=-1;
    else
    {
      ret=Browse_Scan_Areas(&b);

      /* Make sure that our current area is still valid */

      PopMsgArea();
    }


    /* Make sure that something didn't lock our msg area while we were gone */

    ForceGetMsgArea();
  }

  if (b.wildcard)
    free(b.wildcard);

  Deinit_Search(b.first);
  in_mcheck=FALSE;

  if ((b.bflag & BROWSE_LIST) && !nonstop && MenuNeedMore())
  {
    Puts(CYAN);
    MoreYnns();
  }

  return ret;
}




/* Ask the user to specify an area name to be scanned */

static int near Browse_Get_Area(BROWSE *b)
{
  byte ch;
  char *bkeys=br_areak;

  if ((b->bflag & BROWSE_AREA)==0)
  {
    RipClear();

    do
    {
      WhiteN();

      if (usr.help==NOVICE && ! *linebuf)
      {
        Puts(br_area_verbose);
        ch=(byte)toupper(KeyGetRNP(browse_select_s));
      }
      else ch=(byte)toupper(KeyGetRNP(br_area));

      if (ch==bkeys[0])
        b->bflag |= BROWSE_ACUR;
      else if (ch==bkeys[1] || ch=='\r' || ch=='\0')
        b->bflag |= BROWSE_ATAG;
      else if (ch==bkeys[2])
        b->bflag |= BROWSE_AWLD;
      else if (ch==bkeys[3])
        b->bflag |= BROWSE_AGRP;
      else if (ch==bkeys[4])
        b->bflag |= BROWSE_AALL;
      else if (ch==bkeys[5])
        b->bflag |= BROWSE_ALNT;
      else if (ch==bkeys[6])
      {
        Puts(browse_select_c);
        return -1;
      }
      else if (ch==bkeys[7])
        Display_File(0, NULL, browse_fname, PRM(misc_path));
      else Printf(dontunderstand, ch);
    }
    while ((b->bflag & BROWSE_AREA)==0);

    /* If we need to get an area name for searching */

    while ((b->bflag & (BROWSE_AWLD | BROWSE_AGRP)) && ! b->wildcard)
    {
      char temp[PATHLEN];

      if (b->bflag & BROWSE_AGRP)
        b->wildcard=strdup(MessageSection(usr.msg, temp));
      else
      {
        char wc[PATHLEN];

        WhiteN();
        InputGetsL(wc, PATHLEN-1, br_which_areas);
        Puts(browse_which_c);

        if (!*wc)
          return -1;

        b->wildcard=strdup(wc);
      }
    }

    if (! *linebuf)
      Puts(browse_select_c);

  }

  return 0;
}



/* Display the list of topics for which we are searching */

static void near _stdc Browse_Show_List(void *cowabunga,...)
{
  va_list varg;
  int first;

  char *t_name;
  char *n_name;

  int t_true;
  int n_true;

  NW(cowabunga);  /* teecee reports "cowabunga not used" w/o this */

  va_start(varg, cowabunga);

  first=TRUE;

  t_name=va_arg(varg, char *);
  t_true=va_arg(varg, int);

  while (t_name)
  {
    do
    {
      n_name=va_arg(varg,char *);

      if (n_name)
        n_true=va_arg(varg, int);
      else n_true=FALSE;
    }
    while (n_name && !n_true);

    if (t_true)
    {
      if (!first)
      {
        Puts(br_comma);

        if (!n_name)
          Puts(br_then_or);
      }

      Puts(t_name);
      first=FALSE;
    }

    t_true=n_true;
    t_name=n_name;
  }

  va_end(varg);
}




static void near Browse_Display_Search_Criteria(SEARCH *first)
{
  SEARCH *s;

  /*  Maximus will display all messages containing:                         *
   *                                                                        *
   *        `Slartibartfast' in the to, from or subject fields,             *
   *    AND `German' in the message body,                                   *
   *    AND `Bratwurst' in the subject field,                               *
   *                                                                        *
   *                     OR                                                 *
   *                                                                        *
   *        `Maximus' in the message body,                                  */

  Puts(br_max_will);

  for (s=first;s;s=s->next)
  {
    if (! s->txt)
      continue;

    if (s != first)
    {
      if (s->flag & SF_OR)
        Puts(br_or);
      else Puts(br_and);
    }

    Printf(br_in_the_xxx_fields,s->txt);

    Browse_Show_List(NULL,
                     br_field_to, (int)(s->where & WHERE_TO),
                     br_field_from, (int)(s->where & WHERE_FROM),
                     br_field_subj, (int)(s->where & WHERE_SUBJ),
                     br_field_body, (int)(s->where & WHERE_BODY),
                     NULL);

    if ((s->where & WHERE_BODY)==0)
      Puts(br_field_maybe_plural);

    Putc('\n');
  }
}



static int near Browse_Get_Search_Where(SEARCH *s,BROWSE *b)
{
  char temp[PATHLEN];
  byte *p;

  while ((s->where & WHERE_ALL)==0)
  {
    AlwaysWhiteN();

    if (usr.help==NOVICE && ! *linebuf)
    {
      Puts(br_s_where_verbose);
      InputGets(temp,browse_select_s);
    }
    else InputGets(temp, br_s_where);

    if (! *linebuf)
      Puts(browse_select_c);

    if (! *temp)
      return -1;

    strupr(temp);

    for (p=temp;*p;p++)
    {
      if (*p==br_sk[0])
        s->where |= WHERE_TO;
      else if (*p==br_sk[1])
        s->where |= WHERE_FROM;
      else if (*p==br_sk[2])
        s->where |= WHERE_SUBJ;
      else if (*p==br_sk[3])
      {
        s->where |= WHERE_BODY;
        b->bflag |= BROWSE_GETTXT;
      }
      else if (*p==br_sk[4])
        return -1;
      else if (*p==br_sk[5])
        Display_File(0, NULL, browse_fname, PRM(misc_path));
      else Printf(dontunderstand,*p);
    }
  }

  return 0;
}



static int near Browse_Get_Search_Next(SEARCH *s)
{
  byte ch;

  if (! *linebuf)
  {
    Putc('\n');
    
    Puts(br_s_opt_lim+1);
    Puts(br_s_opt_exp+1);
    Puts(br_s_opt_go+1);
    Puts(br_s_opt_quit+1);

    Putc('\n');
  }

  do
  {
    ch=(byte)KeyGetRNP(browse_select_s);

    if (ch==*br_s_opt_lim)
    {
      s->next=malloc(sizeof(SEARCH));

      if (s->next)
      {
        Init_Search(s->next);
        s->next->flag=SF_AND;
      }
    }
    else if (ch==*br_s_opt_exp)
    {
      s->next=malloc(sizeof(SEARCH));

      if (s->next)
      {
        Init_Search(s->next);
        s->next->flag=SF_OR;
      }
    }
    else if (ch==*br_s_opt_go)
      s->next=NULL;
    else if (ch=='?')
      Display_File(0, NULL, browse_fname, PRM(misc_path));
    else if (ch==*br_s_opt_quit || ch=='\0' || ch=='\r')
    {
      Puts(browse_select_c);
      return -1;
    }
    else Printf(dontunderstand,ch);
  }
  while (s->next==NULL && ch != *br_s_opt_go);
  
  if (! *linebuf)
    Puts(browse_select_c);

  return 0;
}

static int near Browse_Get_Search_Text(SEARCH *s)
{
  char temp[PATHLEN];

  WhiteN();
  InputGetsLL(temp, PATHLEN-1, br_text_to_search);

  if (! *linebuf)
    Puts(browse_select_c);

  if (! *temp)
    return -1;
  
  s->txt=strdup(temp);
  
  return 0;
}


static int near Browse_Get_Search(BROWSE *b)
{
  SEARCH *s;
  int crit;

  s=b->first;
  crit=0;

  do
  {
    if (Browse_Get_Search_Where(s,b)==-1)
      return -1;

    if (Browse_Get_Search_Text(s)==-1)
      return -1;

    Browse_Display_Search_Criteria(b->first);

    if (Browse_Get_Search_Next(s)==-1)
      return -1;

    s=s->next;

  } while (s && ++crit <= 16);

  b->bflag |= BROWSE_SEARCH;
  
  return 0;
}




static int near Browse_Get_Type(BROWSE *b)
{
  SEARCH *s;
  char temp[PATHLEN];
  byte ch;

  if ((b->bflag & BROWSE_TYPE)==0 && b->first->txt==NULL)
  {
    do
    {
      WhiteN();

      if (usr.help==NOVICE && ! *linebuf)
      {
        Puts(br_type_verbose);
        ch=(byte)toupper(KeyGetRNP(browse_select_s));
      }
      else ch=(byte)toupper(KeyGetRNP(br_type));

      if (ch==br_typek[0])
        b->bflag |= BROWSE_ALL;
      else if (ch==br_typek[1] || ch=='\r' || ch=='\0')
        b->bflag |= BROWSE_NEW;
      else if (ch==br_typek[2]) /* Y)our */
      {
        b->first->txt=strdup(usr.name);
        b->first->attr |= MSGREAD;
        b->first->flag=SF_NOT_ATTR | SF_OR;
        b->first->where=WHERE_TO;
        b->first->next=NULL;

        if (*usr.alias && (b->first->next=s=malloc(sizeof(SEARCH))) != NULL)
        {
          Init_Search(s);

          s->txt=strdup(usr.alias);
          s->attr |= MSGREAD;
          s->flag=SF_NOT_ATTR | SF_OR;
          s->where=WHERE_TO;
        }

        b->bflag |= (BROWSE_NEW | BROWSE_EXACT);
      }
      else if (ch==br_typek[3])
        b->bflag |= BROWSE_FROM;
      else if (ch==br_typek[4])
        b->bflag |= BROWSE_SEARCH;
      else if (ch==br_typek[5])
        Display_File(0, NULL, browse_fname, PRM(misc_path));
      else if (ch==br_typek[6])
      {
        Puts(browse_select_c);
        return -1;
      }
      else Printf(dontunderstand,ch);
    }
    while ( (b->bflag & BROWSE_TYPE)==0 );
  }

  if (b->bflag & BROWSE_SEARCH)
  {
    if (Browse_Get_Search(b)==-1)
      return -1;
  }

  if (b->bflag & BROWSE_FROM)
  {
    WhiteN();

    InputGets(temp, br_ty_from);
    Puts(browse_which_c);
    
    if (*temp=='=')
    {
      b->bflag &= ~BROWSE_FROM;
      b->bflag |= BROWSE_NEW;
    }
    else
    {
      b->bdata=atol(temp);

      if (prm.flags2 & FLAG2_UMSGID)
        b->bdata=MsgUidToMsgn(sq, b->bdata, UID_NEXT);
    
      if (! b->bdata)
      {
        Puts(browse_select_c);
        return -1;
      }
    }
  }

  if (! *linebuf)
    Puts(browse_select_c);


  return 0;
}



static int near Browse_Get_Display(BROWSE *b)
{
  byte ch;

  if ((b->bflag & BROWSE_DISPLAY)==0)
  {
    do
    {
      WhiteN();

      if (usr.help==NOVICE && ! *linebuf)
      {
        Puts(br_display_verbose);
        ch=(byte)toupper(KeyGetRNP(browse_select_s));
      }
      else ch=(byte)toupper(KeyGetRNP(br_display));

      if (ch==br_dispk[0] || ch=='\r' || ch=='\0')
        b->bflag |= BROWSE_READ;
      else if (ch==br_dispk[1])
        b->bflag |= BROWSE_LIST;
      else if (ch==br_dispk[2])
        b->bflag |= BROWSE_QWK;
      else if (ch==br_dispk[3])
      {
        Puts(browse_select_c);
        return -1;
      }
      else if (ch==br_dispk[4])
        Display_File(0, NULL, browse_fname, PRM(misc_path));
      else Printf(dontunderstand,ch);
    }
    while ( (b->bflag & BROWSE_DISPLAY)==0 );
  }

  if (! *linebuf)
    Puts(browse_select_c);
  
  if (b->bflag & BROWSE_READ)
  {
    b->Begin_Ptr=Read_Begin;
    b->Status_Ptr=List_Status;
    b->Idle_Ptr=List_Idle;
    b->Display_Ptr=Read_Display;
    b->After_Ptr=Read_After;
    b->End_Ptr=Read_End;
  }

  if (b->bflag & BROWSE_LIST)
  {
    b->Begin_Ptr=List_Begin;
    b->Status_Ptr=List_Status;
    b->Idle_Ptr=List_Idle;
    b->Display_Ptr=List_Display;
    b->After_Ptr=List_After;
    b->End_Ptr=List_End;
  }
  
  if (b->bflag & BROWSE_QWK)
  {
    b->Begin_Ptr=QWK_Begin;
    b->Status_Ptr=QWK_Status;
    b->Idle_Ptr=QWK_Idle;
    b->Display_Ptr=QWK_Display;
    b->After_Ptr=QWK_After;
    b->End_Ptr=QWK_End;
  }
  
  b->Match_Ptr=Match_All;

  return 0;
}

static int near Browse_Get_Options(BROWSE *b)
{
  if (Browse_Get_Area(b)==-1)
    return -1;

  if (Browse_Get_Type(b)==-1)
    return -1;

  if (Browse_Get_Display(b)==-1)
    return -1;

  return 0;
}




static void near Deinit_Search(SEARCH *s)
{
  SEARCH *ns;
  SEARCH *savenext;

  if (s->txt)
    free(s->txt);

  for (ns=s->next;ns;ns=savenext)
  {
    if (ns->txt)
      free(ns->txt);

    savenext=ns->next;
    free(ns);
  }
}



void Msg_Checkmail(char *menuname)
{
  SEARCH first, *s;

  Init_Search(&first);
  first.txt=strdup(usr.name);
  first.attr=MSGREAD;
  first.flag=SF_NOT_ATTR | SF_OR;
  first.where=WHERE_TO;
  first.next=NULL;

  if (*usr.alias && (first.next=s=malloc(sizeof(SEARCH))) != NULL)
  {
    Init_Search(s);

    s->txt=strdup(usr.alias);
    s->attr |= MSGREAD;
    s->flag=SF_NOT_ATTR | SF_OR;
    s->where=WHERE_TO;
  }
  
  Msg_Browse(BROWSE_AALL | BROWSE_NEW | BROWSE_EXACT | BROWSE_READ, &first, menuname);
}




void Init_Search(SEARCH *s)
{
  s->next=NULL;
  s->txt=NULL;
  s->attr=0L;
  s->flag=SF_OR;
  s->where=0;
}


