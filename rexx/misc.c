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
static char rcs_id[]="$Id: misc.c,v 1.1.1.1 2002/10/01 17:54:49 sdudley Exp $";
#pragma on(unreferenced)

#include <string.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"

char months_ab[][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul",
                     "Aug","Sep","Oct","Nov","Dec"};

#if defined( OBSOLETE )
struct __priv _stdc _privs[]={{"Twit",     TWIT},
                              {"Disgrace", DISGRACE},
                              {"Limited",  LIMITED},
                              {"Normal",   NORMAL},
                              {"Worthy",   WORTHY},
                              {"Privil",   PRIVIL},
                              {"Favored",  FAVORED},
                              {"Extra",    EXTRA},
                              {"Clerk",    CLERK},
                              {"AsstSysOp",ASSTSYSOP},
                              {"SysOp",    SYSOP},
                              {"Hidden",   HIDDEN},
                              {NULL,       -999}};
#endif

char * _fast stristr(char *string,char *search)
{
  /* "register" keyword used to fix the brain-dead MSC (opti)mizer */

  word last_found=0;
  word strlen_search=strlen(search);

  if (string)
  {
    while (*string)
    {
      if ((tolower(*string))==(tolower(search[last_found])))
        last_found++;
      else
      {
        if (last_found != 0)
        {
          string -= last_found-1;
          last_found=0;
          continue;
        }
      }

      string++;

      if (last_found==strlen_search)
        return string-last_found;
    }
  }

  return NULL;
}

char * _fast sc_time(union stamp_combo *sc,char *string)
{
  if (sc->msg_st.date.yr==0)
    *string='\0';
  else
  {
    sprintf(string,
           "%02d %s %02d %02d:%02d:%02d",
           sc->msg_st.date.da,
           months_ab[sc->msg_st.date.mo-1],
           (80+sc->msg_st.date.yr) % 100,
           sc->msg_st.time.hh,
           sc->msg_st.time.mm,
           sc->msg_st.time.ss << 1);
  }

  return string;
}


char * Keys(long key)
{
  static char keys[MAX_KEYS+1];
  char *s;
  int x;

  s=keys;

  for (x=0;x < MAX_KEYS;x++)
    if (key & (1L << x))
    {
      if (x <= 7)
        *s++=(char)('1'+x);
      else *s++=(char)('A'+(x-8));
    }

  *s='\0';

  return keys;
}


#if defined( OBSOLETE )
char * _fast Priv_Level(int priv)
{
  int x;

  for (x=0;x < _PRIVS_NUM;x++)
    if (priv==_privs[x].priv)
      return _privs[x].name;

  return "";
}
#endif


char * Help_Level(int help)
{
/*if (help==HOTFLASH)
    return "Hotflash";
  else*/ if (help==EXPERT)
    return "Expert";
  else if (help==REGULAR)
    return "Regular";
  else return "Novice";
}

char *Video_Mode(int video)
{
  switch (video)
  {
    case GRAPH_ANSI:    return "ANSI";
    case GRAPH_AVATAR:  return "AVATAR";
    default:            return "TTY";
  }
}

static void near StandardDate(union stamp_combo *d_written);

void _fast ASCII_Date_To_Binary(char *msgdate,union stamp_combo *d_written)
{
  char temp[80];

  int dd,yy,mo,
      hh,mm,ss,
      x;

  time_t timeval;
  struct tm *tim;

  timeval=time(NULL);
  tim=localtime(&timeval);

  if (*msgdate=='\0') /* If no date... */
  {
    /* Insert today's date */
    strftime(msgdate,19,"%d %b %y %H:%M:%S",tim);

    StandardDate(d_written);
    return;
  }

  if (sscanf(msgdate,"%d %s %d %d:%d:%d",&dd,temp,&yy,&hh,&mm,&ss)==6)
    x=1;
  else if (sscanf(msgdate,"%d %s %d %d:%d",&dd,temp,&yy,&hh,&mm)==5)
  {
    ss=0;
    x=1;
  }
  else if (sscanf(msgdate, "%*s %d %s %d %d:%d",&dd,temp,&yy,&hh,&mm)==5)
    x=2;
  else if (sscanf(msgdate,"%d/%d/%d %d:%d:%d",&mo,&dd,&yy,&hh,&mm,&ss)==6)
    x=3;
  else x=0;

  if (x==0)
  {
    StandardDate(d_written);
    return;
  }

  if (x==1 || x==2) /* Formats one and two have ASCII date, so compare to list */
  {
    for (x=0;x < 12;x++)
    {
      if (eqstri(temp,months_ab[x]))
      {
        d_written->msg_st.date.mo=x+1;
        break;
      }
    }

    if (x==12)    /* Invalid month, use January instead. */
      d_written->msg_st.date.mo=1;
  }
  else d_written->msg_st.date.mo=mo; /* Format 3 don't need no ASCII month */

  if (yy >= 1900)
    d_written->msg_st.date.yr = yy - 1980;
  else if (yy >= 80)
    d_written->msg_st.date.yr = yy - 80;
  else
    d_written->msg_st.date.yr = yy + 20;

  d_written->msg_st.date.da=dd;

  d_written->msg_st.time.hh=hh;
  d_written->msg_st.time.mm=mm;
  d_written->msg_st.time.ss=ss >> 1;
}



/* Date couldn't be determined, so set it to Jan 1st, 1980 */

static void near StandardDate(union stamp_combo *d_written)
{
  d_written->msg_st.date.yr=0;
  d_written->msg_st.date.mo=1;
  d_written->msg_st.date.da=1;

  d_written->msg_st.time.hh=0;
  d_written->msg_st.time.mm=0;
  d_written->msg_st.time.ss=0;
}



