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

#include "mexall.h"

#ifdef MEX

  /* Support for executing Maximus menu commands from within a MEX
   * script.
   */

  word EXPENTRY intrin_menu_cmd(void)
  {
    MA ma;
    int iOpt;
    char *szArgs;


    MexArgBegin(&ma);

    /* Get the menu option number */

    iOpt=MexArgGetWord(&ma);


    /* Now get the arguments for the command, if any */

    if ((szArgs=MexArgGetString(&ma, FALSE)) != NULL)
    {
      MexImportData(pmisThis);
      BbsRunOpt(iOpt, szArgs);
      MexExportData(pmisThis);

      free(szArgs);
    }

    return MexArgEnd(&ma);
  }


  /* Function to display a *.BBS-style file */

  word EXPENTRY intrin_display_file(void)
  {
    MA ma;
    char *pcNonStop;
    char *psz;
    word rc=(word)-1;

    MexArgBegin(&ma);

    psz=MexArgGetString(&ma, FALSE);
    pcNonStop=MexArgGetRef(&ma);

    if (psz)
    {
      MexImportData(pmisThis);
      rc=Display_File(0, pcNonStop, percent_s, psz);
      MexExportData(pmisThis);
      free(psz);
    }

    regs_2[0]=rc;

    return MexArgEnd(&ma);
  }



  /* Shell to an external program */

  word EXPENTRY intrin_shell(void)
  {
    MA ma;
    IADDR where;
    char *s, *cap;
    int method, out_method;
    word wLen;

    MexArgBegin(&ma);
    method=MexArgGetWord(&ma);
    s=MexArgGetNonRefString(&ma, &where, &wLen);

    /* Convert our argument into one acceptable to Outside() */

    out_method=0;

    if (method & IOUTSIDE_RUN)
      out_method=OUTSIDE_RUN;
    else
      out_method=OUTSIDE_DOS;

    if (method & IOUTSIDE_REREAD)
      out_method |= OUTSIDE_REREAD;

    regs_2[0]=-1;

    /* Create a nul-terminated string of our argument */

    if ((cap=malloc(wLen+1)) != NULL)
    {
      memmove(cap, s, wLen);
      cap[wLen]=0;

      MexImportUser(pmisThis->pmu, &usr);

      /* Call the external program */

      regs_2[0]=Outside(NULL, NULL, out_method, cap, FALSE, CTL_NONE,
                        RESTART_MENU, NULL);

      MexExportUser(pmisThis->pmu, &usr);

      free(cap);
    }

    MexKillString(&where);

    return MexArgEnd(&ma);
  }

#endif /* MEX */

