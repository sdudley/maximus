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
#include "prog.h"
#include "mm.h"
#include "max_oldu.h"

/* Convert new priv levels to an old */

int max2priv(word usLevel)
{
  if (usLevel==(word)-1)  /* Automatic */
    return HIDDEN;
  else
  {
    int idx=ClassLevelIndex(usLevel);
    CLSREC *pcr=ClassRec(idx);

    static sword pequivs[] =
    {
      TWIT, DISGRACE, LIMITED, NORMAL, WORTHY, PRIVIL,
      FAVORED, EXTRA, CLERK, ASSTSYSOP, SYSOP
    };

    /* First, let's assume that the user has given us equivalents */

    if (pcr->usOldPriv != (word)-1)
      return pcr->usOldPriv;

    /* No luc, so let's try some deduction from class properties first */

    if (pcr->ulAccFlags & CFLAGA_HANGUP)
      return HIDDEN;
    if ((pcr->ulAccFlags & CFLAGA_NOLIMIT) &&
        (pcr->ulMailFlags & CFLAGM_PVT) &&
        (pcr->ulMailFlags & CFLAGM_ATTRANY))
      return SYSOP;
    if ((pcr->ulMailFlags & (CFLAGM_LEDITOR|CFLAGM_RDONLYOK|CFLAGM_NOREALNM)) ||
        (pcr->ulAccFlags & (CFLAGA_ULBBSOK|CFLAGA_FLIST|CFLAGA_UHIDDEN)))
      return ASSTSYSOP;

    /* None of that worked, so we just take a mathematical approach */

    return pequivs[(idx * 12)/pclh->usn];
  }
}

