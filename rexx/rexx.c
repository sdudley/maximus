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
static char rcs_id[]="$Id: rexx.c,v 1.1.1.1 2002/10/01 17:54:49 sdudley Exp $";
#pragma on(unreferenced)

#define INCL_DOS
#define INCL_REXXSAA

#include <string.h>
#include "pos2.h"
#include <rexxsaa.h>
#include "max.h"

#define MakeRxString(r,p)  (r).strptr=(PCH)p;(r).strlength=strlen(p)

void MakeRxVal(PRXSTRING prxs, char *txt, int len)
{
  int rc;

#ifdef __FLAT__
  PVOID pv;

  if ((rc=DosAllocMem(&pv, len, PAG_COMMIT | PAG_READ | PAG_WRITE)) != 0)
#else
  SEL sel;

  if ((rc=DosAllocSeg(len, &sel, SEG_NONSHARED)) != 0)
#endif
  {
    printf("SYS%04d: DosAllocSeg(%d)\n", rc, len);
    exit(1);
  }

#ifdef __FLAT__
  prxs->strptr = pv;
#else
  prxs->strptr=MAKEP(sel, 0);
#endif

  prxs->strlength=len;

  memcpy(prxs->strptr, txt, len);
}

void SetRexxVarBinary(char *name, char *val, int length)
{
  SHVBLOCK svb;
  int rc;

  svb.shvnext=NULL;

  MakeRxString(svb.shvname, name);

  svb.shvvalue.strptr = val;
  svb.shvvalue.strlength = length;

  svb.shvnamelen = strlen(name);
  svb.shvvaluelen = length;
  svb.shvcode = RXSHV_SYSET;
  svb.shvret = RXSHV_OK;

#ifdef __FLAT__
  rc = RexxVariablePool(&svb);
#else
  rc = RxVar(&svb);
#endif

  if (rc==RXSHV_NOAVL)
    printf("REX%04d: Set - RxVarRet(\"%s\", \"%s\")\n", rc, name, val);
  else if (svb.shvret != RXSHV_OK && svb.shvret != RXSHV_NEWV)
    printf("REX%04d: Set - RxVar(\"%s\", \"%s\")\n", svb.shvret, name, val);
}

void SetRexxVar(char *name, char *val)
{
  #define MakeRxString(r,p)  (r).strptr=(PCH)p;(r).strlength=strlen(p)
  SetRexxVarBinary(name, val, strlen(val));
}

int GetRexxVar(char *name, char *val, int sizeofvalue)
{
  SHVBLOCK svb;
  int rc;

  svb.shvnext=NULL;

  MakeRxString(svb.shvname, name);

  svb.shvvalue.strptr=val;
  svb.shvvalue.strlength=sizeofvalue;

  svb.shvnamelen=strlen(name);
  svb.shvvaluelen=sizeofvalue;
  svb.shvcode=RXSHV_SYFET;
  svb.shvret=RXSHV_OK;

#ifdef __FLAT__
  rc = RexxVariablePool(&svb);
#else
  rc = RxVar(&svb);
#endif

  if (rc==RXSHV_NOAVL)
    printf("REX%04d: Get - RxVarRet(\"%s\", \"%s\")\n", rc, name, val);
  else if (svb.shvret != RXSHV_OK && svb.shvret != RXSHV_NEWV)
  {
    val[sizeofvalue]=0;
    printf("REX%04d: Get - RxVar(\"%s\", \"%s\")\n", svb.shvret, name, val);
  }

  return svb.shvvaluelen;
}


void SetPrefixBinary(char *prefix, char *name, char *value, int length)
{
  char varname[128];

  strcpy(varname, prefix);
  strcat(varname, name);

  SetRexxVarBinary(varname, value, length);
}

void SetPrefixName(char *prefix, char *name, char *value)
{
  char varname[128];

  strcpy(varname, prefix);
  strcat(varname, name);

  SetRexxVar(varname, value);
}

void SetPrefixLong(char *prefix, char *name, long value)
{
  char varname[128];
  char varvalue[128];

  strcpy(varname, prefix);
  strcat(varname, name);

  sprintf(varvalue, "%ld", value);

  SetRexxVar(varname, varvalue);
}

void SetPrefixDate(char *prefix, char *name, union stamp_combo sc)
{
  char varname[128];
  char varvalue[128];

  strcpy(varname, prefix);
  strcat(varname, name);

  sc_time(&sc, varvalue);

  SetRexxVar(varname, varvalue);
}




void GetPrefixInt(char *prefix, char *name, void *pvalue, int sizeofvalue)
{
  char varname[128];
  char varvalue[128];
  int rc;

  strcpy(varname, prefix);
  strcat(varname, name);

  rc = GetRexxVar(varname, varvalue, 16);

  if (rc <= 15)
    varvalue[rc] = 0;

  if (sizeofvalue==1)
    *(unsigned char *)pvalue=atoi(varvalue);
  else if (sizeofvalue==2)
    *(unsigned short *)pvalue=atoi(varvalue);
  else if (sizeofvalue==4)
    *(long *)pvalue=atol(varvalue);
}

void GetPrefixBinary(char *prefix, char *name, char *value, int sizeofvalue)
{
  char varname[128];

  strcpy(varname, prefix);
  strcat(varname, name);

  GetRexxVar(varname, value, sizeofvalue);
}

void GetPrefixName(char *prefix, char *name, char *value, int sizeofvalue)
{
  char varname[128];
  int rc;

  memset(value, 0, sizeofvalue);

  strcpy(varname, prefix);
  strcat(varname, name);

  rc = GetRexxVar(varname, value, sizeofvalue-1);

  if (rc <= sizeofvalue-1)
    value[rc] = 0;
}


