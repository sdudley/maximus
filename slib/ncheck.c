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

/*# name=Check for NULL pointer assignments
    credit=Thanks go to Peter Fitzsimmons for this module.
*/


#include "prog.h"

/* don't do this in OS/2 with FAR data. OS/2 itself will find null pointers
 * for us.
 */
#if !defined(OS_2) && !defined(NT) && !defined(M_I86CM) && !defined(M_I86LM) && !defined(M_I86HM) && !defined(__FLAT__) && !defined(__TURBOC__)

/*PLF
 *
 * This version will work on any MSDOS compiler, in any model. It calculates
 * the null pointer checksum the first time it is called, then uses that
 * value as a sentinel thereafter.
 */

#define CHECK_SIZE 20    /* check-sum this many bytes */

int _fast nullptr(void)
{
  static int checksum=0,
             FirstTime=TRUE; /* the valid chksum could be zero, so we need this */

  int oursum=0;

  char *null=NULL;

  int i;

  if (FirstTime)
  {
    FirstTime=FALSE;

    for(i=0;i < CHECK_SIZE;i++)
      checksum += null[i];

    return FALSE;
  }
  else
  {
    for (i=0;i < CHECK_SIZE;i++)
      oursum += null[i];

    return (oursum != checksum);
  }
}

#else
int _fast nullptr(void)
{
    return(FALSE);
}
#endif

