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

/* $Id: maxuapi.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

#define INCL_DOS
#define INCL_VIO
#define INCL_REXXSAA
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "pos2.h"
#include <rexxsaa.h>
#include "max.h"
#include "maxapis.h"
#include "userapi.h"

#define MAX_HUF   128         /* Max # of concurrent RUserFileOpens */
#define MAX_HUFF  128         /* Max # of concurrent RUserFileFindOpens */

static HUF aHuf[MAX_HUF];
static HUFF aHuff[MAX_HUFF];
static char minusone[]="-1";
static char one[]="1";
static char zero[]="0";
static char usr_dot[]="usr.";

char *ptNames[]=
{
  "RUserFileOpen",
  "RUserFileSize",
  "RUserFileFind",
  "RUserFileFindOpen",
  "RUserFileFindNext",
  "RUserFileFindPrior",
  "RUserFileFindClose",
  "RUserFileUpdate",
  "RUserFileCreateRecord",
  "RUserFileClose",
  "RUserFileGetNewLastread",
  "RUserFileInitRecord",
  "RMaxUnloadFuncs",
  "RMaxTrap",
  NULL
};


#define SetRxString(r,p)  strcpy((r).strptr,p);(r).strlength=strlen(p)

/*
HUF _fast UserFileOpen(char *name, int mode);
long _fast UserFileSize(HUF huf);
int _fast UserFileFind(HUF huf, char *name, char *alias, struct _usr *pusr);
HUFF _fast UserFileFindOpen(HUF huf, char *name, char *alias);
int _fast UserFileFindNext(HUFF huff, char *name, char *alias);
int _fast UserFileFindPrior(HUFF huff, char *name, char *alias);
int _fast UserFileFindClose(HUFF huff);
int _fast UserFileUpdate(HUF huf, char *name, char *alias, struct _usr *pusr);
int _fast UserFileCreateRecord(HUF huf, struct _usr *pusr, int fCheckUnique);
int _fast UserFileRemove(HUF huf, struct _usr *pusr);
int _fast UserFileClose(HUF huf);
*/


static int AllocHuf(void)
{
  int i;

  for (i=0; i < MAX_HUF; i++)
    if (!aHuf[i])
      return i;

  return -1;
}

static void FreeHuf(int huf)
{
  if (huf >= 0 && huf < MAX_HUF)
    aHuf[huf]=0;
}


static int AllocHuff(void)
{
  int i;

  for (i=0; i < MAX_HUFF; i++)
    if (!aHuff[i])
      return i;

  return -1;
}

static void FreeHuff(int huff)
{
  if (huff >= 0 && huff < MAX_HUFF)
    aHuff[huff]=0;

}


/* Convert a REXX-string argument into an integer */

int GetIntArg(PRXSTRING prxs)
{
  char temp[PATHLEN];

  strnncpy(temp, prxs->strptr, prxs->strlength+1);
  return atoi(temp);
}


/* Get a REXX-string argument and convert to a HUF.
 * Returns TRUE if the HUF is valid.
 */

static int near GetHufArg(PRXSTRING prxs, int *piHuf)
{
  int iHuf=GetIntArg(prxs);

  if (iHuf < 0 || iHuf >= MAX_HUF ||
      aHuf[iHuf]==0)
  {
    return FALSE;
  }

  *piHuf=iHuf;
  return TRUE;
}


/* Get a REXX-string argument and convert to a HUF.
 * Returns TRUE if the HUFF is valid.
 */

static int near GetHuffArg(PRXSTRING prxs, int *piHuff)
{
  int iHuff=GetIntArg(prxs);

  if (iHuff < 0 || iHuff >= MAX_HUFF ||
      aHuff[iHuff]==0)
  {
    return FALSE;
  }

  *piHuff=iHuff;
  return TRUE;
}



/* Set a REXX return argument to a particular value */

void SetRxStringLong(PRXSTRING prxs, long l)
{
  char temp[PATHLEN];

  sprintf(temp, "%ld", l);
  SetRxString(*prxs, temp);
}


static void SysError(int rc, char *pszName, char *pszData)
{
  char temp[60];

  sprintf(temp, "SYS%04d: %s - %s\r\n", rc, pszName, pszData);
  VioWrtTTY(temp, strlen(temp), 0);
}



static int near DoUnload(void)
{
  char **pt;
  int retval = 0;
  int rc;

  for (pt=ptNames; *pt; pt++)
  {
#ifdef __FLAT__
    rc = RexxDeregisterFunction(*pt+1);
#else
    rc = RxFunctionDeregister(*pt+1);
#endif

    if (rc)
    {
      SysError(rc, "RxFunctionDeregister", *pt+1);
      retval = -1;
      break;
    }
  }

  return retval;
}



OS2UINT APIENTRY RMaxLoadFuncs(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  char **pt;
  int retval = 0;
  int rc;

  (void)pszName; (void)argc; (void)prxsArgv; (void)pszQueue; (void)prxsRet;

  for (pt=ptNames; *pt; pt++)
  {
    /* Don't register a function that is already registered */

    if (RexxQueryFunction(*pt+1) == RXFUNC_OK)
      continue;

#ifdef __FLAT__
    rc = RexxRegisterFunctionDll(*pt+1, "MAXUAPI", *pt);
#else
    rc = RxFunctionRegister(*pt+1, "MAXUAPI", *pt, RXFUNC_DYNALINK);
#endif

    if (rc)
    {
      SysError(rc, "RexxRegisterFunctionDll", *pt);
      retval = -1;
      break;
    }
  }

  sprintf(prxsRet->strptr, "%d", retval);
  prxsRet->strlength=strlen(prxsRet->strptr);

  return 0;
}


OS2UINT APIENTRY RMaxUnloadFuncs(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                                 PSZ pszQueue, PRXSTRING prxsRet)
{
  int retval;

  (void)pszName; (void)argc; (void)prxsArgv; (void)pszQueue; (void)prxsRet;

  retval = DoUnload();

  sprintf(prxsRet->strptr, "%d", retval);
  prxsRet->strlength=strlen(prxsRet->strptr);

  return 0;
}


OS2UINT APIENTRY MaxTrap(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                        PSZ pszQueue, PRXSTRING prxsRet)
{
  (void)pszName; (void)argc; (void)prxsArgv; (void)pszQueue; (void)prxsRet;

  *(char far *)0=0;
  return 0;
}



/* Open the user file */

OS2UINT APIENTRY RUserFileOpen(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuf;
  int iMode;
  char szFile[PATHLEN];
  char szMode[PATHLEN];

  (void)pszName; (void)argc; (void)pszQueue;

  /* Try to allocate a HUF id.  If that fails, return 1 */

  if ((iHuf=AllocHuf())==-1)
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  strnncpy(szFile, prxsArgv[0].strptr, prxsArgv[0].strlength+1);
  strnncpy(szMode, prxsArgv[1].strptr, prxsArgv[1].strlength+1);

  if (eqstri(szMode, "create"))
    iMode=O_CREAT | O_TRUNC;
  else
    iMode=0;

  /* Now try to open the user file itself */

  if ((aHuf[iHuf]=UserFileOpen(szFile, iMode))==NULL)
  {
    FreeHuf(iHuf);
    SetRxString(*prxsRet, minusone);
    return 0;
  }


  /* Now convert the HUF id number into an integer, then return it to
   * the REXX program.
   */

  SetRxStringLong(prxsRet, iHuf);
  return 0;
}




/* Get the size of the user file */

OS2UINT APIENTRY RUserFileSize(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuf;
  long lSize;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  lSize=UserFileSize(aHuf[iHuf]);

  SetRxStringLong(prxsRet, lSize);
  return 0;
}



/* Find a single user in the user file. */

OS2UINT APIENTRY RUserFileFind(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuf;
  char szName[PATHLEN];
  char szAlias[PATHLEN];
  struct _usr usr;
  int rc;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  strnncpy(szName,  prxsArgv[1].strptr, prxsArgv[1].strlength+1);
  strnncpy(szAlias, prxsArgv[2].strptr, prxsArgv[2].strlength+1);

  /* Now perform the actual find */

  rc=UserFileFind(aHuf[iHuf],
                  *szName ? szName : NULL,
                  *szAlias ? szAlias : NULL,
                  &usr);

  if (!rc)
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  /* Export the user structure and return TRUE */

  ExportUser(usr_dot, &usr);
  SetRxString(*prxsRet, one);
  return 0;
}


/* Start a find session with the user file */

OS2UINT APIENTRY RUserFileFindOpen(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                                 PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuf, iHuff;
  char szName[PATHLEN];
  char szAlias[PATHLEN];

  (void)pszName; (void)argc; (void)pszQueue;

  /* Get a HUF to work with */

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  /* Get the name and alias of the user to find */

  strnncpy(szName,  prxsArgv[1].strptr, prxsArgv[1].strlength+1);
  strnncpy(szAlias, prxsArgv[2].strptr, prxsArgv[2].strlength+1);

  /* Try to allocate a HUFF index for this search */

  if ((iHuff=AllocHuff())==-1)
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  if ((aHuff[iHuff]=UserFileFindOpen(aHuf[iHuf],
                                     *szName ? szName : NULL,
                                     *szAlias ? szAlias : NULL))==NULL)
  {
    FreeHuff(iHuff);
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  /* Export the found user */

  ExportUser(usr_dot, &aHuff[iHuff]->usr);

  /* Return the found HUFF number */

  SetRxStringLong(prxsRet, iHuff);
  return 0;
}


/* Find the next user that matches the name/alias criteria */

OS2UINT APIENTRY RUserFileFindNext(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuff;
  char szName[PATHLEN];
  char szAlias[PATHLEN];
  int rc;

  (void)pszName; (void)argc; (void)pszQueue;

  /* Get a HUF to work with */

  if (!GetHuffArg(prxsArgv+0, &iHuff))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  /* Get the name and alias of the user to find */

  strnncpy(szName,  prxsArgv[1].strptr, prxsArgv[1].strlength+1);
  strnncpy(szAlias, prxsArgv[2].strptr, prxsArgv[2].strlength+1);

  /* Call the actual findnext function */

  rc=UserFileFindNext(aHuff[iHuff],
                      *szName ? szName : NULL,
                      *szAlias ? szAlias : NULL);


  /* Export the new user structure if the search was successful */

  if (rc)
    ExportUser(usr_dot, &aHuff[iHuff]->usr);


  /* Return true/false accordingly */

  SetRxStringLong(prxsRet, rc);
  return 0;
}


/* Find the prior record in a named search */

OS2UINT APIENTRY RUserFileFindPrior(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuff;
  char szName[PATHLEN];
  char szAlias[PATHLEN];
  int rc;

  (void)pszName; (void)argc; (void)pszQueue;

  /* Get a HUF to work with */

  if (!GetHuffArg(prxsArgv+0, &iHuff))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  /* Get the name and alias of the user to find */

  strnncpy(szName,  prxsArgv[1].strptr, prxsArgv[1].strlength+1);
  strnncpy(szAlias, prxsArgv[2].strptr, prxsArgv[2].strlength+1);

  /* Call the actual findnext function */

  rc=UserFileFindPrior(aHuff[iHuff],
                       *szName ? szName : NULL,
                       *szAlias ? szAlias : NULL);


  /* Export the new user structure if the search was successful */

  if (rc)
    ExportUser(usr_dot, &aHuff[iHuff]->usr);


  /* Return true/false accordingly */

  SetRxStringLong(prxsRet, rc);
  return 0;
}


/* Close a UserFileFindOpen search */

OS2UINT APIENTRY RUserFileFindClose(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuff;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHuffArg(prxsArgv+0, &iHuff))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  UserFileFindClose(aHuff[iHuff]);

  FreeHuff(iHuff);

  SetRxString(*prxsRet, zero);
  return 0;
}


/* Update an existing user record */

OS2UINT APIENTRY RUserFileUpdate(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  char szName[PATHLEN];
  char szAlias[PATHLEN];
  struct _usr usr;
  int iHuf;
  int rc;

  (void)pszName; (void)argc; (void)pszQueue;
  SetRxString(*prxsRet, zero);

  /* Get a HUF to work with */

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  /* Get the name an alias of the user to find */

  strnncpy(szName,  prxsArgv[1].strptr, prxsArgv[1].strlength+1);
  strnncpy(szAlias, prxsArgv[2].strptr, prxsArgv[2].strlength+1);

  ImportUser(usr_dot, &usr);

  /* Call the user record update function */

  rc=UserFileUpdate(aHuf[iHuf],
                    *szName ? szName : NULL,
                    *szAlias ? szAlias : NULL,
                    &usr);

  SetRxStringLong(prxsRet, rc);
  return 0;
}



/* Create a new user record */

OS2UINT APIENTRY RUserFileCreateRecord(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  struct _usr usr;
  int iHuf;
  int rc;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, minusone);
    return 0;
  }

  ImportUser(usr_dot, &usr);

  /* Call the creation function */

  rc=UserFileCreateRecord(aHuf[iHuf], &usr, TRUE);

  SetRxStringLong(prxsRet, rc);
  return 0;
}



/* Close a user file handle */

OS2UINT APIENTRY RUserFileClose(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  int iHuf;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  UserFileClose(aHuf[iHuf]);
  FreeHuf(iHuf);

  SetRxString(*prxsRet, one);
  return 0;
}



/* Get a new lastread pointer number */

OS2UINT APIENTRY RUserFileGetNewLastread(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                             PSZ pszQueue, PRXSTRING prxsRet)
{
  HUFF huff;
  int iHighLastread=0;
  int iHuf;

  (void)pszName; (void)argc; (void)pszQueue;

  if (!GetHufArg(prxsArgv+0, &iHuf))
  {
    SetRxString(*prxsRet, zero);
    return 0;
  }

  /* Scan the whole user file to find the highest lastread pointer */

  if ((huff=UserFileFindOpen(aHuf[iHuf], NULL, NULL))==NULL)
    iHighLastread=-1;
  else
  {
    do
    {
      if (huff->usr.lastread_ptr > iHighLastread)
        iHighLastread=huff->usr.lastread_ptr;
    }
    while (UserFileFindNext(huff, NULL, NULL));

    UserFileFindClose(huff);
  }

  SetRxStringLong(prxsRet, iHighLastread+1);
  return 0;
}



/* Create a blank user record and export it to usr.* */

OS2UINT APIENTRY RUserFileInitRecord(PUCHAR pszName, OS2UINT argc, PRXSTRING prxsArgv,
                                   PSZ pszQueue, PRXSTRING prxsRet)
{
  struct _usr usr;

  (void)pszName; (void)argc; (void)prxsArgv; (void)pszQueue; (void)prxsRet;

  /* Set the new user to be empty */

  memset(&usr, 0, sizeof usr);

  /* Initialize the user to some default settings */

  usr.help=NOVICE;
  usr.video=GRAPH_TTY;
  usr.bits=BITS_TABS;
  usr.bits2=BITS2_IBMCHARS | BITS2_BORED | BITS2_CLS | BITS2_MORE;
  usr.priv=0;
  usr.width=80;
  usr.len=24;
  strcpy(usr.msg, "1");
  strcpy(usr.files, "1");
  usr.struct_len=sizeof(struct _usr)/20;
  usr.def_proto=PROTOCOL_NONE;

  ExportUser(usr_dot, &usr);
  SetRxString(*prxsRet, one);
  return 0;
}




#if 0
void APIENTRY __DLLend(OS2UINT usTermCode)
{
  (void)usTermCode;

  DoUnload();
  DosExitList(EXLST_EXIT, 0);
}

int __dll_initialize(void)
{
  DosExitList(EXLST_ADD, __DLLend);
  return 1;
}
#endif


