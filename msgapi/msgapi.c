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

/* $Id: msgapi.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

#define MSGAPI_INIT

#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include "alc.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "apidebug.h"



static byte *intl="INTL";
static byte *fmpt="FMPT";
static byte *topt="TOPT";
static byte *area_colon="AREA:";

#ifdef OS_2
  word _stdc far msgapierr=0;   /* Global error value for MsgAPI routines */
#else
  word _stdc msgapierr=0;       /* Global error value for MsgAPI routines */
#endif

#if defined(__MSDOS__)
struct _minf _stdc mi={0,1,0};  /* DOS defaults - v0, zone 1, no share */
#else
struct _minf _stdc mi={0,1,1};  /* OS/2 defaults - v0, zone 1, with share */
#endif

unsigned _SquishCloseOpenAreas(void);

#ifdef __WATCOMC__  /* handle DLL startup requirements for WC 8.5 */
  #if defined(OS_2) && __WCVER__ >= 900

    #define INCL_SUB
    #define INCL_DOS
    #include "pos2.h"

    void APIENTRY __DLLend(OS2UINT usTermCode)
    {
      /*char temp[100];*/
      (void)usTermCode;

      _SquishCloseOpenAreas();

      /*
      sprintf(temp, "MsgAPI 2.0 Termination, usTermCode=%d\r\n",
              usTermCode);

      VioWrtTTY(temp, 38, 0);
      */

      DosExitList(EXLST_EXIT, 0);
    }

    int __dll_initialize(void)
    {
      extern char startup_txt[];
      grow_handles(40);

      /*VioWrtTTY(startup_txt, strlen(startup_txt), 0);*/
      /*DosBeep(200, 20);*/

      DosExitList(EXLST_ADD, __DLLend);
      return 1;
    }

  #endif
#endif

/* Default allocation thunks */

static void OS2FAR * MAPIENTRY _palloc(size_t size)
{ return ((void OS2FAR *)malloc(size)); }

static void MAPIENTRY _pfree(void OS2FAR *ptr)
{ free(ptr); }

static void OS2FAR * MAPIENTRY _repalloc(void OS2FAR *ptr, size_t size)
{ return ((void OS2FAR *)realloc(ptr, size)); }

static void far * MAPIENTRY _farpalloc(size_t size)
{ return ((void far *)farmalloc(size)); }

static void MAPIENTRY _farpfree(void far *ptr)
{ farfree(ptr); }

static void far * MAPIENTRY _farrepalloc(void far *ptr, size_t size)
{ return ((void far *)farrealloc(ptr, size)); }


void OS2FAR * (MAPIENTRY *palloc)(size_t size) = _palloc;
void (MAPIENTRY *pfree)(void OS2FAR *ptr) = _pfree;
void OS2FAR * (MAPIENTRY *repalloc)(void OS2FAR *ptr, size_t size) = _repalloc;

void far * (MAPIENTRY *farpalloc)(size_t size) = _farpalloc;
void (MAPIENTRY *farpfree)(void far *ptr) = _farpfree;
void far * (MAPIENTRY *farrepalloc)(void far *ptr, size_t size) = _farrepalloc;

sword MAPIENTRY MsgOpenApi(struct _minf OS2FAR *minf)
{
  (void)memset(&mi, '\0', sizeof mi);
  mi=*minf;
  mi.haveshare=minf->haveshare=shareloaded();

  /* If the caller wants to set the malloc/free hooks, do so here */

  if (mi.req_version >= 1)
  {
    if (mi.palloc)
      palloc=mi.palloc;

    if (mi.pfree)
      pfree=mi.pfree;

    if (mi.repalloc)
      repalloc=mi.repalloc;

    if (mi.farpalloc)
      farpalloc=mi.farpalloc;

    if (mi.farpfree)
      farpfree=mi.farpfree;

    if (mi.farrepalloc)
      farrepalloc=mi.farrepalloc;
  }

  return 0;
}

sword MAPIENTRY MsgCloseApi(void)
{
  return _SquishCloseOpenAreas() ? 0 : -1;
}

HAREA MAPIENTRY MsgOpenArea(byte OS2FAR *name, word mode, word type)
{
  if (type & MSGTYPE_SQUISH)
    return (SquishOpenArea(name,mode,type));
  else return (SdmOpenArea(name,mode,type));
}

sword MAPIENTRY MsgValidate(word type, byte OS2FAR *name)
{
  if (type & MSGTYPE_SQUISH)
    return (SquishValidate(name));
  else /*if (type==MSGTYPE_SDM)*/
    return (SdmValidate(name));
/*  else return FALSE;*/
}

/* Check to see if a message handle is valid.  This function should work    *
 * for ALL handlers tied into MsgAPI.  This also checks to make sure that   *
 * the area which the message is from is also valid.  (ie. The message      *
 * handle isn't valid, unless the area handle of that message is also       *
 * valid.)                                                                  */

sword MAPIENTRY InvalidMsgh(HMSG msgh)
{
  if (msgh==NULL || msgh->id != MSGH_ID || MsgInvalidHarea(msgh->ha))
  {
    msgapierr=MERR_BADH;
    return TRUE;
  }

  return FALSE;
}

/* Check to ensure that a message area handle is valid.                     */

sword MAPIENTRY InvalidMh(HAREA mh)
{
  if (mh==NULL || mh->id != MSGAPI_ID)
  {
    msgapierr=MERR_BADH;
    return TRUE;
  }

  return FALSE;
}


byte * MAPIENTRY StripNasties(byte *str)
{
  byte *s;

  for (s=str; *s; s++)
    if (*s < ' ')
      *s=' ';

  return str;
}




/* Copy the text itself to a buffer, or count its length if out==NULL */

static word near _CopyToBuf(byte *p, byte *out, byte **end, unsigned remaining)
{
  word len=0;

  if (out)
    *out++='\x01';

  len++;


  for (; remaining > 0 && (*p=='\x0d' || *p=='\x0a' || *p==(byte)0x8d); )
  {
    p++;
    remaining--;
  }

  while (remaining > 0 && *p=='\x01' || strncmp(p, area_colon, 5)==0)
  {
    /* Skip over the first ^a */

    if (*p=='\x01')
    {
      p++;
      remaining--;
    }

    while (remaining > 0 && *p && *p != '\x0d' && *p != '\x0a' &&
           *p != (byte)0x8d)
    {
      if (out)
        *out++=*p;
      
      p++;
      remaining--;
      len++;
    }

    if (out)
      *out++='\x01';

    len++;

    while (remaining > 0 && *p=='\x0d' || *p=='\x0a' || *p==(byte)0x8d)
    {
      p++;
      remaining--;
    }
  }

  /* Cap the string */

  if (out)
    *out='\0';

  len++;
  

  /* Make sure to leave no trailing x01's. */

  if (out && out[-1]=='\x01')
    out[-1]='\0';
  

  /* Now store the new end location of the kludge lines */

  if (end)
    *end=p;
  
  return len;
}


/* Used to free returned ptr from MsgCvtCtrlToKludge, MsgCreateCtrlBuf */

void MAPIENTRY MsgFreeCtrlBuf(char *cbuf)
{
  pfree(cbuf);
}

/* Used to free returned ptr from MsgGetCtrlToken */

void MAPIENTRY MsgFreeCtrlToken(char *cbuf)
{
  pfree(cbuf);
}



byte OS2FAR * MAPIENTRY CopyToControlBuf(byte OS2FAR *txt, byte OS2FAR * OS2FAR *newtext, unsigned OS2FAR *length)
{
  byte *cbuf, *end;

  word clen;


  /* Figure out how long the control info is */

  clen=_CopyToBuf(txt, NULL, NULL, *length);

  /* Allocate memory for it */
  
  #define SAFE_CLEN 20

  if ((cbuf=palloc(clen+SAFE_CLEN))==NULL)
    return NULL;

  (void)memset(cbuf, '\0', clen+SAFE_CLEN);

  /* Now copy the text itself */

  clen=_CopyToBuf(txt, cbuf, &end, *length);

  *length -= (size_t)(end-txt);

  if (newtext)
    *newtext=end;

  return cbuf;
}


byte OS2FAR * MAPIENTRY GetCtrlToken(byte OS2FAR *where, byte OS2FAR *what)
{
  byte *end, *found, *out;

  if (where && (found=strstr(where, what)) != NULL && found[-1]=='\x01')
  {
    end=strchr(found,'\x01');

    if (!end)
      end=found+strlen(found);

    if ((out=palloc((size_t)(end-found)+1))==NULL)
      return NULL;

    (void)memmove(out, found, (size_t)(end-found));
    out[(size_t)(end-found)]='\0';
    return out;
  }

  return NULL;
}


void MAPIENTRY ConvertControlInfo(byte OS2FAR *ctrl, NETADDR OS2FAR *orig, NETADDR OS2FAR *dest)
{
  byte *p, *s;
  
  if ((p=s=GetCtrlToken(ctrl, intl)) != NULL)
  {
    NETADDR norig, ndest;

    /* Copy the defaults from the original address */

    norig=*orig;
    ndest=*dest;

    /* Parse the destination part of the kludge */

    s += 5;
    Parse_NetNode(s, &ndest.zone, &ndest.net, &ndest.node, &ndest.point);
    
    while (*s != ' ' && *s)
      s++;

    if (*s)
      s++;

    Parse_NetNode(s, &norig.zone, &norig.net, &norig.node, &norig.point);

    pfree(p);

    /* Only use this as the "real" zonegate address if the net/node         *
     * addresses in the INTL line match those in the message                *
     * body.  Otherwise, it's probably a gaterouted message!                */

    if (ndest.net==dest->net && ndest.node==dest->node &&
        norig.net==orig->net && norig.node==orig->node)
    {
      *dest=ndest;
      *orig=norig;

      /* Only remove the INTL line if it's not gaterouted, which is         *
       * why we do it here.                                                 */
      
      RemoveFromCtrl(ctrl,intl);
    }
  }


  /* Handle the FMPT kludge */
  
  if ((s=GetCtrlToken(ctrl,fmpt)) != NULL)
  {
    orig->point=(word)atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,fmpt);
  }

  
  /* Handle TOPT too */
  
  if ((s=GetCtrlToken(ctrl,topt)) != NULL)
  {
    dest->point=(word)atoi(s+5);
    pfree(s);

    RemoveFromCtrl(ctrl,topt);
  }
}

  
byte OS2FAR * MAPIENTRY CvtCtrlToKludge(byte OS2FAR *ctrl)
{
  byte *from;
  byte *buf;
  byte *to;
  size_t clen;
  
  clen=strlen(ctrl) + NumKludges(ctrl) + 20;
  
  if ((buf=palloc(clen))==NULL)
    return NULL;
  
  to=buf;

  /* Convert ^aKLUDGE^aKLUDGE... into ^aKLUDGE\r^aKLUDGE\r... */
  
  for (from=ctrl; *from=='\x01' && from[1];)
  {
    /* Only copy out the ^a if it's NOT the area: line */

    if (!eqstrn(from+1, area_colon, 5))
      *to++=*from;

    from++;
    
    while (*from && *from != '\x01')
      *to++=*from++;

    *to++='\r';
  }
  
  *to='\0';

  return buf;
}



    
void MAPIENTRY RemoveFromCtrl(byte OS2FAR *ctrl, byte OS2FAR *what)
{
  byte *search;
  byte *p, *s;
  
  if ((search=palloc(strlen(what)+2))==NULL)
    return;
  
  (void)strcpy(search, "\x01");
  (void)strcat(search, what);
  
  /* Now search for this token in the control buffer, and remove it. */
    
  while ((p=strstr(ctrl, search)) != NULL)
  {
    for (s=p+1; *s && *s != '\x01'; s++)
      ;
      
    (void)strocpy(p, s);
  }
  
  pfree(search);
}


word MAPIENTRY NumKludges(char OS2FAR *txt)
{
  word nk=0;
  char *p;
  
  for (p=txt; (p=strchr(p, '\x01')) != NULL; p++)
    nk++;
    
  return nk;
}

#ifdef OS_2

int far pascal farread(int handle, byte far *buf, unsigned len)
{
  return read(handle, buf, len);
}

int far pascal farwrite(int handle, byte far *buf, unsigned len)
{
  return write(handle, buf, len);
}

#endif

