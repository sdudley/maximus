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
static char rcs_id[]="$Id: mex_misc.c,v 1.1 2002/10/01 17:53:51 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Miscellaneous compiler routines
*/

#include <string.h>
#include "prog.h"
#include "mex.h"

/* AddrEqual() returns TRUE if the two addresses reference the same         *
 * memory location/register, or FALSE otherwise.                            */

int AddrEqual(ADDRESS *a1,ADDRESS *a2)
{
  return (a1->segment==a2->segment &&
          a1->offset==a2->offset);
}


/* Create a user-readable name for the type defined by 't' */

char * TypeName(TYPEDESC *t,char *s)
{
  *s='\0';
  
  if (!t)
    return "*";

  switch (t->form)
  {
    case FormByte:
      sprintf(s, "%schar", t->fSigned ? "" : "unsigned ");
      break;

    case FormWord:
      sprintf(s, "%sint", t->fSigned ? "" : "unsigned ");
      break;

    case FormDword:
      sprintf(s, "%slong", t->fSigned ? "" : "unsigned ");
      break;

    case FormString:
      strcpy(s, "string");
      break;

    case FormArray:
      if (t->typeinfo.array.bounds.hi==(VMADDR)-1)
      {
        sprintf(s, "array [%u..] of ", t->typeinfo.array.bounds.lo);
      }
      else
      {
        sprintf(s, "array[%u..%u] of ",
                t->typeinfo.array.bounds.lo,
                t->typeinfo.array.bounds.hi);
      }
          
      TypeName(t->typeinfo.array.el_type, s+strlen(s));
      break;

    case FormStruct:
      sprintf(s, "struct %s", t->typeinfo.struc.name);
      break;

    case FormEllipsis:
      strcpy(s, "...");
      break;

    default:
      strcpy(s, "unknown");
  }
  
  return s;
}



int IsIntegral(TYPEDESC *t)
{
  switch (t->form)
  {
    case FormByte:
    case FormWord:
    case FormDword:
      return TRUE;
  }
  
  return FALSE;
}

word PassByRef(TYPEDESC *t)
{
  return (!IsIntegral(t) && t != &StringType && t != &EllipsisType);
}


