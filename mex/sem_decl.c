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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: sem_decl.c,v 1.3 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Variable declaration semantic routines
*/

#include <string.h>
#include "prog.h"
#include "mex.h"

/* This routine takes an identifier name, enters it into the symbol table   *
 * as a typeless object, and places it in a linked list with the object     *
 * 'next'.  This routine also checks for redeclarations and conflicts,      *
 * even though the variables themselves aren't officially "declared"        *
 * until the entire list of variables is collected, and then has            *
 * all of the information filled out in the symbol table.                   */

ATTRIBUTES * var_list(IDTYPE *id,ATTRIBUTES *next)
{
  ATTRIBUTES *attr, *a, *last;
  byte present;
  
  attr=st_enter(symtab, id, &present);
  
  if (present)
  {
    error(MEXERR_REDECLARATIONOF, id);
    return next;
  }
  
  for (a=next, last=NULL; a; last=a, a=a->next_decl)
    ;

  if (last)
  {
    last->next_decl=attr;
    attr->next_decl=NULL;
    return next;
  }
  else
  {
    attr->next_decl=next;
    return attr;
  }

  /*
  attr->next_decl=next;
  return attr;
  */
}




/* This routine walks through the linked list of variables pointed to       *
 * by 'attr', declaring them all of be of type 'typedesc', and              *
 * officially entering them in the symbol table.  This function returns the *
 * number of bytes of storage used in the AR.                               */

VMADDR declare_vars(TYPEDESC *typedesc,ATTRIBUTES *attr)
{
  ATTRIBUTES *ap;
  
  /* Error in declaration */

  if (!typedesc)
    return offset;

  if (typedesc==&VoidType)
  {
    error(MEXERR_TYPEMUSTNOTBEVOID);
    return offset;
  }

  if (typedesc && typedesc->form==FormArray &&
      typedesc->typeinfo.array.bounds.hi==(VMADDR)-1)
  {
    error(MEXERR_ARRAYMUSTHAVEBOUNDS);
    return offset;
  }

  for (ap=attr; ap; ap=ap->next_decl)
  {
    /* Indicate that it's a variable of the specified class */

    ap->class=ClassVariable;
    ap->typedesc=typedesc;

    /* Make its address into a location into the function's AR, unless      *
     * it's declared outside of a function, in which case it's global.      */
    
    ap->c.addr.segment=(byte)(scope==0 ? SEG_GLOBAL : SEG_AR);

    if (scope==0 || iDeclaringStruct)
      ap->c.addr.offset=offset;     /* Offset from start of DS or structure */
    else
      ap->c.addr.offset=-(offset+typedesc->size);  /* Offset from AR */

    ap->c.addr.indirect=FALSE;    /* Variable is NOT indirectly accessed    */
    ap->c.addr.typedesc=typedesc; /* Value of type being pointed to         */
    
    #ifdef DECLDEBUG
    debug("Allocating '%s' at %soffset %ld (size %ld)",
          ap->name,
          scope==0 ? "GLOBAL " : "",
          scope==0 ? (long)glob_offset : (long)offset,
          (long)typedesc->size);
    #endif

    if (scope==0)
    {
      /* The offset for global variables MUST be zero, so that we can       *
       * calculate the conpart of arrays and ADD it to this.  Since we      *
       * are also adding the run-time offset to this "address", all         *
       * global variables must have a relative offset of zero before        *
       * they are patched by the run-time relocation routines.              */

      ap->c.addr.offset=0;

      /* However, we should update glob_offset anyway -- this acts more as  *
       * a counter of how much space was used, as opposed to a pointer      *
       * to where the variables will be stored.                             */

      glob_offset += typedesc->size;
      
      /* Now make sure that space for this is allocated on the run-time     *
       * symbol table.                                                      */
         
      add_one_global(ap->name, typedesc->size, NULL);

    }
    else offset += typedesc->size;

    if (offset > maxoffset)
      maxoffset=offset;

    if (glob_offset > maxgloboffset)
      maxgloboffset=glob_offset;
  }
  
  return offset;
}


/* Create a "type descriptor" for an ellipsis function */

FUNCARGS *declare_ellipsis(void)
{
  FUNCARGS *f;

  f=smalloc(sizeof(FUNCARGS));
  f->type=&EllipsisType;
  f->name=sstrdup("Ellipsis");
  f->next=NULL;
  f->ref=FALSE;

  return f;
}





/* Add a constant value to the global data area.  This acts like a global   *
 * variable, in the fact that it's accessed with the 'global' segment,      *
 * except that there is no variable specifically dedicated to this space    *
 * by default.  It's also initialized to this constant value at runtime.    */

struct _conval * AddGlobalConst(VMADDR len, byte *buf, TYPEDESC *type)
{
  struct _conval *cv;

  if (type==&StringType)
    len += sizeof(word);
  
  cv=smalloc(sizeof(struct _conval));
  
  cv->buf=smalloc(len);

  /* Convert the C nul-terminated strings into ones with a length word up   *
   * front.                                                                 */

  if (type==&StringType)
  {
    *(word *)cv->buf=(word)len-sizeof(word);
    memmove(cv->buf+sizeof(word), buf, len-sizeof(word));
  }
  else memmove(cv->buf, buf, len);

  #ifdef DECLDEBUG
    debug("Constant init at Glob:%04d (len %d)", glob_offset, len);
  #endif

  
  cv->len=len;


  /* If we're writing a .VM file, make sure that the offset is zero so that *
   * the run-time patching routines will work properly.                     */
     
  cv->offset=(vm_output ? 0 : glob_offset);
  glob_offset += len;
  
  cv->next=cvlist;
  cvlist=cv;
  
  return cv;
}

char *GlobGetString(VMADDR offset)
{
  struct _conval *cv;
  char *s;
  
  if (vm_output)
    bug("GlobGetString can't be used with -v, since cv->offset will always be zero");
  
  for (cv=cvlist; cv; cv=cv->next)
    if (cv->offset==offset)
    {
      s=smalloc(cv->len-1);
      memmove(s, cv->buf+sizeof(word), cv->len-sizeof(word));
      s[cv->len-sizeof(word)]='\0';
      return s;
    }
    
  bug("Invalid GlobGetString(%d)", offset);
  return NULL;
}



/* This routine creates a type descriptor for an array object, with a       *
 * range of 'r', consisting of an element type of 'el_type'.                */

TYPEDESC *array_descriptor(RANGE *r,TYPEDESC *el_type)
{
  TYPEDESC *nt;

  if (!el_type)
    return NULL;

  if (el_type==&VoidType)
  {
    error(MEXERR_TYPEMUSTNOTBEVOID);
    return NULL;
  }
  
  nt=NewTypeDescriptor();
  nt->size = r->hi==(VMADDR)-1 ? -1 : (el_type->size * ((r->hi - r->lo)+1));

  #ifdef DECLDEBUG
  if (r->hi==(VMADDR)-1)
    debug("Array size is unknown");
  else
    debug("Array size is %d (%d els * %d bytes/el)",
          nt->size, (r->hi - r->lo)+1, el_type->size);
  #endif

  nt->form=FormArray;

  nt->typeinfo.array.bounds=*r;
  nt->typeinfo.array.el_type=el_type;

  return nt;
}


/* Declare a new structure type.  This returns an empty type descriptor     *
 * for the newly created structure, enters it in the main symbol table,     *
 * and creates an auxillary symbol table for that structure itself.  This   *
 * also points the main 'symtab' to the current structure table, such that  *
 * all declarations are made within the new table.                          */

TYPEDESC * define_struct_id(IDTYPE *pidStruct)
{
  TYPEDESC *ptd;
  ATTRIBUTES *a;
  byte fPresent;

  a=st_enter(symtab, pidStruct, &fPresent);

  if (fPresent)
  {
    error(MEXERR_REDECLOFSTRUCT, a->name);
    return NULL;
  }

  /* Allocate a new type descriptor for this structure */

  ptd=NewTypeDescriptor();

  /* Fill out the attributes struct and point it to the typedesc */

  a->class=ClassStructName;
  a->typedesc=ptd;

  /* Initialize the type to a size of 0 and a type of struct */

  ptd->size=0;
  ptd->form=FormStruct;

  /* Save info about the old symbol table */

  ptd->typeinfo.struc.name=a->name;
  ptd->typeinfo.struc.post=symtab;
  ptd->typeinfo.struc.oscope=scope;
  ptd->typeinfo.struc.ooffset=offset;
  ptd->typeinfo.struc.omaxoffset=maxoffset;

  /* Create new symbol table for this struct and use it by default */

  symtab=ptd->typeinfo.struc.pst=st_create(SSYMTAB_SIZE, symtab);

  scope=1;  /* any non-global scope will do */
  offset=0;

  /* Flag that we are declaring a struct */

  iDeclaringStruct++;

#ifdef DECLDEBUG
  debug("Begin declaration of structure '%s'", a->name);
#endif

  return ptd;
}


/* Finish the declaration of the structure body */

void define_struct_body(TYPEDESC *ptd)
{
  ptd->size=offset;

  /* Restore the old activation record settings */

  symtab    = ptd->typeinfo.struc.post;
  scope     = ptd->typeinfo.struc.oscope;
  offset    = ptd->typeinfo.struc.ooffset;
  maxoffset = ptd->typeinfo.struc.omaxoffset;

  /* Set a flag that says that we are no longer declaring a struct */

  iDeclaringStruct--;

#ifdef DECLDEBUG
  debug("End declaration of structure '%s' (size=%d)",
        ptd->typeinfo.struc.name, ptd->size);
#endif
}


/* Declare an actual instance of a structure */

TYPEDESC *declare_struct(IDTYPE *pidStructName)
{
  ATTRIBUTES *a;

  /* Try to find it in our symbol table - this just returns the
   * handle for the structure identifier.  The symbol tables
   * within contain the references and offsets for the
   * individual elements.
   */

  if ((a=st_find(symtab, pidStructName, TRUE))==NULL)
  {
    error(MEXERR_UNDEFSTRUCTNAME, pidStructName);
    return NULL;
  }

  if (a->class != ClassStructName || a->typedesc->form != FormStruct)
  {
    error(MEXERR_SYMISNOTSTRUCT, pidStructName);
    return NULL;
  }

  return a->typedesc;
}



