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
static char rcs_id[]="$Id: l_attach.c,v 1.3 2004/01/27 21:00:30 paltas Exp $";
#pragma on(unreferenced)
#endif

#define MAX_LANG_m_browse

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "dbasec.h"
#include "prog.h"
#include "l_attach.h"

int asciizcmp(void OS2FAR *a1, void OS2FAR *a2)
{
  return stricmp((char OS2FAR *)a1, (char OS2FAR *)a2);
}

#define ptr2uid(a)        *((dword *)a)
#define recncmp(a1,a2)    (int)(ptr2uid(a1) - ptr2uid(a2))

int dwordcmp(void OS2FAR *a1, void OS2FAR *a2)
{
  return recncmp(a1,a2);
}

#define atLFAID   (0)
#define atUSERTO  (sizeof(dword))
#define atAREANM  (sizeof(dword)+LFA_ASCII_NAME_SIZE)
#define atUMSGID  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN)
#define atUSERFM  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN+sizeof(UMSGID))
#define atFILENM  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN+sizeof(UMSGID)+LFA_ASCII_NAME_SIZE)
#define atATTRIB  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN+sizeof(UMSGID)+LFA_ASCII_NAME_SIZE+LFA_ASCII_FILE_SIZE)
#define atDATEAT  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN+sizeof(UMSGID)+LFA_ASCII_NAME_SIZE+LFA_ASCII_FILE_SIZE+sizeof(dword))
#define atDATERC  (sizeof(dword)+LFA_ASCII_NAME_SIZE+MAX_ALEN+sizeof(UMSGID)+LFA_ASCII_NAME_SIZE+LFA_ASCII_FILE_SIZE+sizeof(dword)+sizeof(SCOMBO))

static FIELD afLFAFields[LFA_NUM_FIELDS]=
{
  { "AttachID",     atLFAID,        sizeof(dword),  dwordcmp,  dwordcmp },
  { "UserTo",       atUSERTO, LFA_ASCII_NAME_SIZE, asciizcmp, asciizcmp },
  { "AreaName",     atAREANM,            MAX_ALEN, asciizcmp, asciizcmp },
  { "Msgid",        atUMSGID,      sizeof(UMSGID),         0,         0 },
  { "UserFrom",     atUSERFM, LFA_ASCII_NAME_SIZE,         0,         0 },
  { "FileName",     atFILENM, LFA_ASCII_FILE_SIZE,         0,         0 },
  { "Attributes",   atATTRIB,       sizeof(dword),         0,         0 },
  { "DateAttached", atDATEAT,      sizeof(SCOMBO),         0,         0 },
  { "DateReceived", atDATERC,      sizeof(SCOMBO),         0,         0 }
};


DBASE OS2FAR * LFAdbCreate(char OS2FAR *szName)
{
  return DbOpen( szName, afLFAFields, LFA_NUM_FIELDS, TRUE, 32 );
}
  
DBASE OS2FAR * LFAdbOpen(char OS2FAR *szName)
{
  return DbOpen( szName, afLFAFields, LFA_NUM_FIELDS, FALSE, 32 );
}

void LFAdbClose(DBASE OS2FAR * pdb)
{
  if (pdb && pdb!=FTSATTACHDBH)
    DbClose(pdb);
}

void LFARecInit(LFA_REC * plfa,
                char *szTo,
                char *szArea,
                UMSGID uid,
                char *szFrom,
                char *szFile)
{
  time_t timeval=time(NULL);
  struct tm *tim=localtime(&timeval);

  static dword counter = 0;

  memset(plfa, 0, sizeof(LFA_REC));
  plfa->ulAttachID  = (timeval << 3) | ++counter;
  strnncpy(plfa->szTo,   szTo,   sizeof(plfa->szTo));
  strnncpy(plfa->szArea, szArea, sizeof(plfa->szArea));
  strnncpy(plfa->szFrom, szFrom, sizeof(plfa->szFrom));
  strnncpy(plfa->szFile, szFile, sizeof(plfa->szFile));
  plfa->uid = uid;
  TmDate_to_DosDate(tim,&plfa->scDateAttached.ldate);
}


