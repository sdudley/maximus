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

#pragma off(unreferenced)
static char rcs_id[]="$Id: init.c,v 1.1.1.1 2002/10/01 17:57:23 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Initialization code for MECCA and ACCEM
*/

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "mecca.h"


void Init_Table(void)
{
  int x,y;

  static int *set[NUM_NUMS+1]={NULL,&num_gt,&num_lt,&num_eq,&num_ne,&num_ge,
                               &num_le,&num_above,&num_below,&num_equal,
                               &num_unequal,&num_notequal,&num_ae,&num_be};

  num_gt=num_lt=num_eq=num_ne=num_ge=num_le=num_above=num_below=num_equal=
    num_unequal=num_notequal=num_ae=num_be=-2;

  for (x=0;x < verb_table_size;x++)
  {
    y=verbs[x].verbno;

    if (y >= 1 && y <= NUM_NUMS)
      *set[y]=x;
  }
}

