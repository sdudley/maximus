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

#ifndef INTERNAL_LANGUAGES

  /* Return a string from the language heap */

  word EXPENTRY intrin_lang_string(void)
  {
    MA ma;
    int stringnum;

    MexArgBegin(&ma);
    stringnum=MexArgGetWord(&ma);

    MexReturnString(s_ret(stringnum));

    return MexArgEnd(&ma);
  }

  /* Return a string from a named language heap */

  word EXPENTRY intrin_lang_heap_string(void)
  {
    MA ma;
    char *psz;
    int stringnum;

    MexArgBegin(&ma);
    psz=MexArgGetString(&ma, TRUE);
    stringnum=MexArgGetWord(&ma);
    if (!psz)
      MexReturnString("");
    else
    {
      MexReturnString(s_reth(psz,stringnum));
      free(psz);
    }

    return MexArgEnd(&ma);
  }

#endif /* !INTERNAL_LANGUAGES */

#endif /* MEX */
