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


  /* Return terminal or screen width */

  word EXPENTRY intrin_term_width(void)
  {
    regs_2[0]=TermWidth();
    return 0;
  }

  /* Return terminal or screen length */

  word EXPENTRY intrin_term_length(void)
  {
    regs_2[0]=TermLength();;
    return 0;
  }

  /* Return screen length */

  word EXPENTRY intrin_screen_length(void)
  {
    regs_2[0]=VidNumRows();
    return 0;
  }

  /* Return screen width */

  word EXPENTRY intrin_screen_width(void)
  {
    regs_2[0]=VidNumCols();
    return 0;
  }

  word EXPENTRY intrin_set_textsize(void)
  {
    MA ma;
    int cols, rows;

    MexArgBegin(&ma);
    cols=MexArgGetWord(&ma);
    rows=MexArgGetWord(&ma);
    SetTermSize(cols, rows);

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_rip_send(void)
  {
    MA ma;
    char *pszFile;
    int fDisplay;

    MexArgBegin(&ma);
    pszFile = MexArgGetString(&ma, FALSE);
    fDisplay=MexArgGetWord(&ma);
    if (!pszFile)
      regs_2[0]=0;
    else
    {
      regs_2[0]=RIP_SendFile(pszFile, fDisplay);
      free(pszFile);
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_rip_hasfile(void)
  {
    MA ma;
    char *pszFile;
    long *lfilesize;

    MexArgBegin(&ma);
    pszFile = MexArgGetString(&ma, FALSE);
    lfilesize=MexArgGetRef(&ma);
    if (!pszFile)
      regs_2[0]=0;
    else
    {
      regs_2[0]=RIP_HasFile(pszFile, lfilesize);
      free(pszFile);
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_ansi_detect(void)
  {
    regs_2[0]=autodetect_ansi();
    return 0;
  }

  word EXPENTRY intrin_rip_detect(void)
  {
    regs_2[0]=autodetect_rip();
    return 0;
  }

#endif /* MEX */

