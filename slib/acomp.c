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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "prog.h"

int _stdc AreaNameComp(byte *a1,byte *a2)
{
  int id1=isdigit(*a1), id2=isdigit(*a2);
  long at1, at2;

  if (id1 || id2)
  {
    /* Numbers always come before letters */

    if (id1 && !id2)
      return -1;

    if (id2 && !id1)
      return 1;

    /* Handle a numeric comparison */

    at1=atol(a1);
    at2=atol(a2);

    if (at1 != at2)
      return (int)(at1-at2);
/*    else if (strlen(a1) < strlen(a2))
      return -1;
    else if (strlen(a1) > strlen(a2))
      return 1;*/
  }

  return (stricmp(a1, a2));
}

