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

#include <stdlib.h>
#include <string.h>

#define INCL_VIO
#define INCL_DOS
#define INCL_DOSERRORS

#include <pos2.h>
#include "compiler.h"

#ifdef __WATCOMC__  /* handle DLL startup requirements for WC 8.5 */
  #if defined(OS_2) && !defined(__FLAT__) && __WCVER__ <= 850
    int main(void)
    {
      return 1;
    }
  #endif

  #if defined(OS_2) && !defined(__FLAT__) && __WCVER__ >= 900
    int __DLLstart(void)
    {
      return 0;
    }
  #endif
#endif


static HSYSSEM hssm;
static char *gpszSemName=NULL;

/* Clean-up processing for the serialization semaphore */

static void far pascal BbsSemTerminate(USHORT usTermCode)
{
  NW(usTermCode);

  DosSemClear(hssm);
  DosCloseSem(hssm);
  DosExitList(EXLST_EXIT, 0);
}

static void near BbsPuts(char *txt)
{
  VioWrtTTY(txt, strlen(txt), 0);
}

static void near BbsSemError(char *sem, int rc)
{
  char temp[5];

  BbsPuts("SYS");

  itoa(rc, temp, 10);
  BbsPuts(temp);

  BbsPuts(": Error in DosOpenSem(\"");
  BbsPuts(sem);
  BbsPuts("\")\r\n");
}


void EXPENTRY BbsSemSerialize(char *pszSemName, char *pszSemEnv)
{
  USHORT rc;
  char *pszOldSemName;

  /* Get the sem name from the environment, if possible */

  if (pszSemEnv)
  {
    pszOldSemName=pszSemName;

    /* Scan for it.  If not found, restore the sem name to the default */

    if (DosScanEnv(pszSemEnv, &pszSemName) != 0)
      pszSemName=pszOldSemName;
  }

  if (!pszSemName)
    pszSemName="/sem/squish/default";

  gpszSemName=pszSemName;

  if ((rc=DosCreateSem(CSEM_PUBLIC, &hssm, pszSemName))==0)
    DosSemRequest(hssm, SEM_INDEFINITE_WAIT);
  else if (rc != ERROR_ALREADY_EXISTS)
    BbsSemError(pszSemName, rc);
  else
  {
    if ((rc=DosOpenSem(&hssm, pszSemName)) != 0)
      BbsSemError(pszSemName, rc);
    else
    {
      BbsPuts("Serializing on semaphore ");
      BbsPuts(pszSemName);
      BbsPuts("\r\n");

      if ((rc=DosSemRequest(hssm, SEM_INDEFINITE_WAIT))==ERROR_SEM_OWNER_DIED)
        rc=0;
    }
  }

  /* Add program to exit list if successful; otherwise, terminate */

  if (rc==0)
    DosExitList(EXLST_ADD, (PFN)BbsSemTerminate);
  else DosExit(EXIT_PROCESS, 255);
}

