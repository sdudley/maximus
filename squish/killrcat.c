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
static char rcs_id[]="$Id: killrcat.c,v 1.1 2002/10/01 17:56:04 sdudley Exp $";
#pragma on(unreferenced)

#include <string.h>

#define INCL_VIO
#define INCL_DOS

#include <os2.h>
#include "msgapi.h"
#include "sqfeat.h"


#define FLAG_BODY   0x01
#define FLAG_TO     0x02
#define FLAG_FROM   0x04
#define FLAG_SUBJ   0x08
#define FLAG_CASE   0x10
#define FLAG_EXCT   0x20

struct _kksearch
{
  int flag;
  char *txt;
  char *area;
  struct _kksearch *next;
};

static struct _kksearch *gpSearch;


/* Code to handle chinese characters */

#define ISLEFT(c) ((c) > (byte)0x80 && (c) < (byte)0xff)
#define ISRIGHT(c) (((c) >= (byte)0x40 && (c) <= (byte)0x7e) || \
                    ((c) >= (byte)0xa1 && (c) <= (byte)0xfe))

word ischin(byte *buf)
{
  return (ISLEFT(buf[0]) && ISRIGHT(buf[1]));
}

char * stristr(char *string,char *search)
{
  /* "register" keyword used to fix the brain-dead MSC (opti)mizer */

  unsigned last_found=0;
  unsigned strlen_search=strlen(search);
  byte l1, l2;
  unsigned i;

  if (string)
  {
    while (*string)
    {
      /**** start chinese modifications *****/
      l1=(byte)(ischin(string) ? 2 : 1);
      l2=(byte)(ischin(search+last_found) ? 2 : 1);

      if (l1==l2)
        i=(l1==1) ? memicmp(string, search+last_found, l1) : 1;
      else i=1;
      
      if (!i)
        last_found += l1;
      /**** end chinese modifications *****/
      /* old code: if ((tolower(*string))==(tolower(search[last_found])))
                     last_found++;
      */
      else
      {
        if (last_found != 0)
        {
          string -= last_found-1;
          last_found=0;
          continue;
        }
      }

      string += l1;

      if (last_found==strlen_search) return(string-last_found);
    }
  }

  return(NULL);
}



int tolower(int c)
{
  if (c >= 'A' && c <= 'Z')
    c='a'+(c-'A');

  return c;
}

char * firstchar(char *strng,char *delim,int findword)
{
  int x, isw, wordno=0;
  unsigned sl_d, sl_s;

  char *string,
       *oldstring;

  /* We can't do *anything* if the string is blank... */

  if (! *strng)
    return NULL;

  string=oldstring=strng;

  sl_d=strlen(delim);

  for (string=strng;*string;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++)
      if (*string==delim[x])
        isw=1;

    if (isw==0)
    {
      oldstring=string;
      break;
    }
  }

  sl_s=strlen(string);

  for (wordno=0;(string-oldstring) < sl_s;string++)
  {
    for (x=0,isw=0;x <= sl_d;x++)
      if (*string==delim[x])
      {
        isw=1;
        break;
      }

    if (!isw && string==oldstring)
      wordno++;

    if (isw && (string != oldstring))
    {
      for (x=0,isw=0;x <= sl_d;x++) if (*(string+1)==delim[x])
      {
        isw=1;
        break;
      }

      if (isw==0)
        wordno++;
    }

    if (wordno==findword)
      return((string==oldstring || string==oldstring+sl_s) ? string : string+1);
  }

  return NULL;
}



void VioPuts(char *txt)
{
  (void)VioWrtTTY(txt, (USHORT)strlen(txt), 0);
}

void VioPutc(char ch)
{
  char s[2];

  *s=ch;
  s[1]=0;
  VioPuts(s);
}

/****************************************************************************
 * FeatureInit:                                                             *
 ****************************************************************************
 *                                                                          *
 * This is called at Squish startup, when the "Features" line in SQUISH.CFG *
 * is processed.  This routine can be used to perform feature-specific      *
 * initialization needs.                                                    *
 *                                                                          *
 * The only action that this function must perform is to fill out the       *
 * 'feature information' structure that is passed to it by Squish.          *
 * At present, the only field in this structure is the 'Config Name'        *
 * option.  This can be used to register keywords for which the             *
 * FeatureConfig function will be called.  This can be used to              *
 * implement feature-specific keywords in the configuration file.           *
 *                                                                          *
 * NOTE:  External features should NOT display anything on the screen       *
 * in this function.                                                        *
 *                                                                          *
 * A return value of 0 indicates that the feature initialized successfully. *
 * A non-zero return value indicates failure and instructs Squish to        *
 * terminate.                                                               *
 *                                                                          *
 ****************************************************************************/

word FEATENTRY _export FeatureInit(struct _feat_init far *pfi)
{
  /* The szConfigName field should contain a list of the keywords           *
   * for which FeatureConfig should be called.  These tokens are only       *
   * matched if they are the first word on a given line.  More than one     *
   * token can be given, as long as a "\r" is used to separate adjacent     *
   * tokens, such as "Killrcat\rKillrdog\rKillrbird".                       */

  strcpy(pfi->szConfigName, "Killrcat");
  return 0;
}


/****************************************************************************
 * FeatureConfig                                                            *
 ****************************************************************************
 *                                                                          *
 * This function is called when Squish detects one of the specified         *
 * feature configuration keywords in SQUISH.CFG.  The feature should        *
 * the information on this line as required, and then return to Squish.     *
 *                                                                          *
 * A return value of 0 indicates success.  A non-zero return value          *
 * instructs Squish to abort.                                               *
 *                                                                          *
 ****************************************************************************/

word FEATENTRY _export FeatureConfig(struct _feat_config far *pfc)
{
  struct _kksearch *ps;
  char *p;

  NW(pfc);

  if ((ps=malloc(sizeof(struct _kksearch)))==NULL)
    return 0;

  ps->flag=0;

  for (p=pfc->ppszArgs[1]; p && *p; p++)
    switch (tolower(*p))
    {
      case 'b':   ps->flag |= FLAG_BODY;  break;
      case 's':   ps->flag |= FLAG_SUBJ;  break;
      case 't':   ps->flag |= FLAG_TO;    break;
      case 'f':   ps->flag |= FLAG_FROM;  break;
      case 'c':   ps->flag |= FLAG_CASE;  break;
      case 'e':   ps->flag |= FLAG_EXCT;  break;

      default:
        VioPuts("Invalid KillrCat flag: `");
        VioPutc(*p);
        VioPuts("\r\n");
        break;
    }

  if (!pfc->ppszArgs[1] || !pfc->ppszArgs[2] || !pfc->ppszArgs[3])
  {
    VioPuts("Invalid configuration line: `");
    VioPuts(pfc->szConfigLine);
    VioPuts("\r\n");
    return 0;
  }

  ps->area=strdup(pfc->ppszArgs[2]);
  ps->txt=strdup(firstchar(pfc->szConfigLine, " \t\n", 4));

  ps->next=gpSearch;
  gpSearch=ps;

  return 0;
}



/****************************************************************************
 * FeatureNetMsg                                                            *
 ****************************************************************************
 *                                                                          *
 * This function is called just before Squish packs a mesage from a         *
 * netmail area.  Squish will call this function for each netmail message,  *
 * regardless of the status of the MSGSENT bit, unless otherwise defined    *
 * in the feature initialization structure (see FeatureInit).               *
 *                                                                          *
 * Information in the feat_netmsg structure describes the current message   *
 * being processed, in addition to pointers to the message header, body     *
 * text, and control information.                                           *
 *                                                                          *
 * If any special actions are necessary, the feature should fill out the    *
 * ulAction field in the structure before this function terminates.         *
 *                                                                          *
 * A return value of 0 indicates success.  A non-zero return value          *
 * instructs Squish to terminate execution.                                 *
 *                                                                          *
 ****************************************************************************/


word FEATENTRY _export FeatureNetMsg(struct _feat_netmsg far *pfn)
{
  NW(pfn);
  return 0;
}


static int near exactcomp(char *t1, char *t2)
{
  return (strcmp(t1, t2)==0);
}

static int near incomp(char *t1, char *t2)
{
  int x;

  x=!!stristr(t2, t1);
/*  printf("compare %s and %s:\nComp=%d\n", t1, t2, x);*/
  return x;
}

word FEATENTRY _export FeatureTossMsg(struct _feat_toss far *pft)
{
  struct _kksearch *ps;
  int (near *comp)(char *t1, char *t2);
  int kill;

  for (ps=gpSearch; ps; ps=ps->next)
  {
    if (stricmp(ps->area, pft->szArea) != 0 && stricmp(ps->area, "all") != 0)
      continue;

    comp=(ps->flag & FLAG_EXCT) ? exactcomp : incomp;
    kill=FALSE;

    if (ps->flag & FLAG_TO)
      kill |= (*comp)(ps->txt, pft->pMsg->to);

    if (ps->flag & FLAG_FROM)
      kill |= (*comp)(ps->txt, pft->pMsg->from);

    if (ps->flag & FLAG_SUBJ)
      kill |= (*comp)(ps->txt, pft->pMsg->subj);

    if (ps->flag & FLAG_BODY)
    {
      kill |= (*comp)(ps->txt, pft->pszCtrl);
      kill |= (*comp)(ps->txt, pft->pszMsgTxt);
    }

    if (kill)
    {
      VioPuts("\r\nKillrCat: Kill msg to `");
      VioPuts(pft->pMsg->to);
      VioPuts("' from `");
      VioPuts(pft->pMsg->from);
      VioPuts("' subj `");
      VioPuts(pft->pMsg->subj);
      VioPuts("'\r\n");

      strcpy(pft->szArea, "bad_msgs");
      pft->ulTossAction=FTACT_AREA;
      break;
    }
  }

  return 0;
}


word FEATENTRY _export FeatureScanMsg(struct _feat_scan far *pfs)
{
  NW(pfs);
  return 0;
}

word FEATENTRY _export FeatureTerm(struct _feat_term far *pft)
{
  NW(pft);
  return 0;
}


#ifdef __FLAT__
void FEATENTRY _export Feature32Bit(void)
#else
void FEATENTRY _export Feature16Bit(void)
#endif
{
}


