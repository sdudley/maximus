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

/* $Id: expimp.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

#define INCL_DOS
#define INCL_REXXSAA

#include <string.h>
#include <ctype.h>
#include "pos2.h"
#include <rexxsaa.h>
#include "max.h"
#include "maxapis.h"


void ExportUser(char *p, struct _usr *pusr)
{
  static char yes[]="Y";
  static char no[]="N";
  #define PN(x) SetPrefixName(p, #x, pusr->x)
  #define PI(x) SetPrefixLong(p, #x, pusr->x)
  #define PBIT(field, name, mask) SetPrefixName(p, name, (pusr->field & mask) ? yes : no)

  /* Export all of the standard fields */

  PN(name);
  PN(city);
  PN(alias);
  PN(phone);
  SetPrefixLong(p, "num", pusr->lastread_ptr);
  PI(lastread_ptr);
  PI(timeremaining);

  /* If the user's password is not encrypted, export it as a normal
   * string.
   */

  if (pusr->bits & BITS_ENCRYPT)
    SetPrefixBinary(p, "pwd", pusr->pwd, sizeof pusr->pwd);
  else
    PN(pwd);

  PI(times);
  PI(call);
  SetPrefixName(p, "help", Help_Level(pusr->help));
  SetPrefixName(p, "video", Video_Mode(pusr->video));
  PI(nulls);
  PBIT(bits, "hotkeys", BITS_HOTKEYS);
  PBIT(bits, "notavail", BITS_NOTAVAIL);
  PBIT(bits, "fsr", BITS_FSR);
  PBIT(bits, "nerd", BITS_NERD);
  PBIT(bits, "noulist", BITS_NOULIST);
  PBIT(bits, "tabs", BITS_TABS);
  PBIT(bits, "encrypt", BITS_ENCRYPT);
  PBIT(bits, "rip", BITS_RIP);
  PBIT(bits2, "badlogon", BITS2_BADLOGON);
  PBIT(bits2, "ibmchars", BITS2_IBMCHARS);
  PBIT(bits2, "bored", BITS2_BORED);
  PBIT(bits2, "more", BITS2_MORE);
  PBIT(bits2, "configured", BITS2_CONFIGURED);
  PBIT(bits2, "cls", BITS2_CLS);
  PI(priv);
/*SetPrefixName(p, "priv", Priv_Level(pusr->priv));*/
  PN(dataphone);
  PI(time);
  PI(dob_year);
  PBIT(delflag, "deleted", UFLAG_DEL);
  PBIT(delflag, "permanent", UFLAG_PERM);
  PI(msgs_posted);
  PI(msgs_read);
  PI(width);
  PI(len);
  PI(credit);
  PI(debit);
/*SetPrefixName(p, "xp_priv", Priv_Level(pusr->xp_priv));*/
  PI(xp_priv);
  SetPrefixDate(p, "xp_date", pusr->xp_date);
  PI(xp_mins);
  PBIT(xp_flag, "expdate", XFLAG_EXPDATE);
  PBIT(xp_flag, "expmins", XFLAG_EXPMINS);
  PBIT(xp_flag, "expdemote", XFLAG_DEMOTE);
  PBIT(xp_flag, "expaxe", XFLAG_AXE);
  PI(sex);
  SetPrefixDate(p, "ludate", pusr->ludate);
  SetPrefixName(p, "xkeys", Keys(pusr->xkeys));
  PI(lang);
  PI(def_proto);
  PI(up);
  PI(down);
  PI(downtoday);
  PI(compress);
  PI(df_save);
  SetPrefixDate(p, "date_1stcall", pusr->date_1stcall);
  PI(extra);
  SetPrefixDate(p, "date_pwd_chg", pusr->date_pwd_chg);
  PI(nup);
  PI(ndown);
  PI(ndowntoday);
  PI(time_added);
  PN(msg);
  PN(files);
  PI(dob_day);
  PI(dob_month);
  PI(point_credit);
  PI(point_debit);
  SetPrefixDate(p, "date_newfile", pusr->date_newfile);
}


/* Import a user's help level based on an ASCII string */

void GetPrefixHelp(char *prefix, char *name, byte *phelp)
{
  char varvalue[128];

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

/*  if (stricmp(varvalue, "hotflash")==0)
    *phelp=HOTFLASH;
  else*/ if (stricmp(varvalue, "regular")==0)
    *phelp=REGULAR;
  else if (stricmp(varvalue, "expert")==0)
    *phelp=EXPERT;
  else *phelp=NOVICE;
}

/* Import the user's video settings */

void GetPrefixVideo(char *prefix, char *name, byte *pvideo)
{
  char varvalue[128];

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

  if (stricmp(varvalue, "tty")==0)
    *pvideo=GRAPH_TTY;
  else if (stricmp(varvalue, "avatar")==0)
    *pvideo=GRAPH_AVATAR;
  else *pvideo=GRAPH_ANSI;
}

/* Import a user's priv level */
#if defined( OBSOLETE )
void GetPrefixPriv(char *prefix, char *name, sword *ppriv)
{
  char varvalue[128];
  extern struct __priv _stdc _privs[];
  struct __priv *ppv;

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

  for (ppv=_privs; ppv->name; ppv++)
    if (stricmp(ppv->name, varvalue)==0)
    {
      *ppriv=ppv->priv;
      return;
    }

  ppv->priv=TWIT;
}
#endif


/* Import a date string to a binary bitmap */

void GetPrefixDate(char *prefix, char *name, union stamp_combo *psc)
{
  char varvalue[128];

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

  if (*varvalue)
    ASCII_Date_To_Binary(varvalue, psc);
  else
    psc->ldate = 0L;
}


/* Set the user's keys */

void GetPrefixKeys(char *prefix, char *name, dword *xkeys)
{
  char varvalue[128];
  char *p;

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

  *xkeys=0;

  for (p=strupr(varvalue); *p; p++)
    if (*p >= '1' && *p <= '8')
      *xkeys |= (1L << (*p-'1'));
    else if (*p >= 'A' && *p <= 'X')
      *xkeys |= (1L << ((*p-'A')+8));
}



/* Set a bitflag based on a Y/N setting */

void GetPrefixBit(char *prefix, char *name, void *ptr, int mask, int size)
{
  char varvalue[128];

  GetPrefixName(prefix, name, varvalue, sizeof varvalue);

  switch (toupper(*varvalue))
  {
    case 'Y':
      if (size==1)
        *(unsigned char *)ptr |= mask;
      else
        *(unsigned short *)ptr |= mask;

      break;

    case 'N':
      if (size==1)
        *(unsigned char *)ptr &= ~mask;
      else
        *(unsigned short *)ptr &= ~mask;

      break;

    default:
      printf("REXX MaxAPI Error!  Invalid value '%s' for field %s%s!\n",
             varvalue, prefix, name);
  }
}


void ImportUser(char *p, struct _usr *pusr)
{
  #define GN(x) GetPrefixName(p, #x, pusr->x, sizeof pusr->x)
  #define GI(x) GetPrefixInt(p, #x, &pusr->x, sizeof pusr->x)
  #define GBIT(field, name, mask) GetPrefixBit(p, name, &pusr->field, mask, sizeof pusr->field)

  memset(pusr, 0, sizeof *pusr);

  pusr->struct_len = sizeof(*pusr) / 20;

  /* Export all of the standard fields */

  GN(name);
  GN(city);
  GN(alias);
  GN(phone);
  GI(lastread_ptr);
  GI(timeremaining);

  GBIT(bits, "encrypt", BITS_ENCRYPT);
  GBIT(bits, "rip", BITS_RIP);

  if (pusr->bits & BITS_ENCRYPT)
    GetPrefixBinary(p, "pwd", pusr->pwd, sizeof pusr->pwd);
  else
    GN(pwd);

  GI(times);
  GI(call);
  GetPrefixHelp(p, "help", &pusr->help);
  GetPrefixVideo(p, "video", &pusr->video);
  GI(nulls);
  GBIT(bits, "hotkeys", BITS_HOTKEYS);
  GBIT(bits, "notavail", BITS_NOTAVAIL);
  GBIT(bits, "fsr", BITS_FSR);
  GBIT(bits, "nerd", BITS_NERD);
  GBIT(bits, "noulist", BITS_NOULIST);
  GBIT(bits, "tabs", BITS_TABS);
  /* The GBIT for encrypt is done above */
  GBIT(bits2, "badlogon", BITS2_BADLOGON);
  GBIT(bits2, "ibmchars", BITS2_IBMCHARS);
  GBIT(bits2, "bored", BITS2_BORED);
  GBIT(bits2, "more", BITS2_MORE);
  GBIT(bits2, "configured", BITS2_CONFIGURED);
  GBIT(bits2, "cls", BITS2_CLS);
  GI(priv);
/*GetPrefixPriv(p, "priv", &pusr->priv);*/
  GN(dataphone);
  GI(time);
  GI(dob_year);
  GBIT(delflag, "deleted", UFLAG_DEL);
  GBIT(delflag, "permanent", UFLAG_PERM);
  GI(msgs_posted);
  GI(msgs_read);
  GI(width);
  GI(len);
  GI(credit);
  GI(debit);
/*GetPrefixPriv(p, "xp_priv", &pusr->xp_priv);*/
  GI(xp_priv);
  GetPrefixDate(p, "xp_date", &pusr->xp_date);
  GI(xp_mins);
  GBIT(xp_flag, "expdate", XFLAG_EXPDATE);
  GBIT(xp_flag, "expmins", XFLAG_EXPMINS);
  GBIT(xp_flag, "expdemote", XFLAG_DEMOTE);
  GBIT(xp_flag, "expaxe", XFLAG_AXE);
  GI(sex);
  GetPrefixDate(p, "ludate", &pusr->ludate);
  GetPrefixKeys(p, "xkeys", &pusr->xkeys);
  GI(lang);
  GI(def_proto);
  GI(up);
  GI(down);
  GI(downtoday);
  GI(compress);
  GI(df_save);
  GetPrefixDate(p, "date_1stcall", &pusr->date_1stcall);
  GI(extra);
  GetPrefixDate(p, "date_pwd_chg", &pusr->date_pwd_chg);
  GI(nup);
  GI(ndown);
  GI(ndowntoday);
  GI(time_added);
  GN(msg);
  GN(files);
  GI(dob_day);
  GI(dob_month);
  GI(point_credit);
  GI(point_debit);
  GetPrefixDate(p, "date_newfile", &pusr->date_newfile);
}


