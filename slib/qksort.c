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

/*# name=Integer quicksort routine.
    credit=Thanks to Thomas Plum and _Reliable_Data_Structure_In_C_
    credit=for this routine.
*/

#include <stdio.h>
#include "prog.h"

#define NUM sizeof(array)/sizeof(array[0])
#define SWAP(a,b,s) s=a; a=b; b=s;

static void _fast iqksort(int *p_lo,int *p_hi);

void _fast qksort(int a[],size_t n)
{
  if (n > 1)
    iqksort(a,&a[n-1]);
}



static void _fast iqksort(int *p_lo,int *p_hi)
{
  int  *p_mid=p_lo+(((int)(p_hi-p_lo))/2),
       *p_i,
       *p_lastlo,
       tmp;

  SWAP(*p_lo,*p_mid,tmp);

  p_lastlo=p_lo;

  for (p_i=p_lo+1;p_i <= p_hi;++p_i)
  {
    if (*p_lo > *p_i)
    {
      ++p_lastlo;
      SWAP(*p_lastlo,*p_i,tmp);
    }
  }

  SWAP(*p_lo,*p_lastlo,tmp);

  if (p_lo < p_lastlo && p_lo < p_lastlo-1)
    iqksort(p_lo,p_lastlo-1);

  if (p_lastlo+1 < p_hi)
    iqksort(p_lastlo+1,p_hi);
}

