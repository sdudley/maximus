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
static char rcs_id[]="$Id: vm_symt.c,v 1.3 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#define COMPILING_MEX_VM

#include <stdio.h>
#include <string.h>
#include "prog.h"
#include "vm.h"

static VMADDR vaLastAssigned=0;


/* Initialize the symbol table */

int init_symtab(int cRtSym)
{
  n_rtsym=cRtSym;
  vaLastAssigned=0;

  if ((rtsym=malloc(sizeof(struct _rtsym) * n_rtsym))==NULL)
    return -1;

  return 0;
}


/* Enter a symbol in the rum-time symbol table.  If it already exists,      *
 * return its allocated offset.  Otherwise, allocate a                      *
 * new entry in the global table, and return its offset too.                */

VMADDR EXPENTRY MexEnterSymtab(char *name, word size)
{
  struct _rtsym *rt, *end;
  VMADDR thisofs;
  
  /* Scan from rtsym[0] .. rtsym[n_entry], and look for this var */
  
  if (*name)
    for (rt=rtsym, end=rtsym+n_entry; rt < end; rt++)
      if (eqstr(rt->name, name))
        return rt->offset;
  
  /* Not found, so assign a new one */
    
  if (n_entry >= n_rtsym)
    vm_err(err_too_many_rtsym);
  
  thisofs=vaLastAssigned;
  vaLastAssigned += size;
  
  /* Make sure that there is enough room for the stack */

  if (vaLastAssigned > vmh.lGlobSize)
    vm_err(err_cdata_ovfl);

  if (! *name)
    return thisofs;
  
  strcpy(rtsym[n_entry].name, name);
  rtsym[n_entry++].offset=thisofs;
  
  return thisofs;
}




/* Adds one byte to the global symbol table, with a name of 'name' and      *
 * an initial value of 'b'.                                                 */

VMADDR EXPENTRY MexStoreByte(char *name, byte b)
{
  VMADDR ofs;

  ofs=MexEnterSymtab(name, sizeof(byte));
  *(byte *)DSEG(ofs)=b;

  return ofs;
}

/* Adds one word to the global symbol table, with a name of 'name' and      *
 * an initial value of 'w'.                                                 */

VMADDR EXPENTRY MexStoreWord(char *name, word w)
{
  VMADDR ofs;

  ofs=MexEnterSymtab(name, sizeof(word));
  *(word *)DSEG(ofs)=w;

  return ofs;
}

/* Adds one dword to the global symbol table, with a name of 'name' and     *
 * an initial value of 'dw'.                                                */

VMADDR EXPENTRY MexStoreDword(char *name, dword dw)
{
  VMADDR ofs;

  ofs=MexEnterSymtab(name, sizeof(dword));
  *(dword *)DSEG(ofs)=dw;

  return ofs;
}

/* Adds one string to the global symbol table, with a name of 'name' and    *
 * initially containing the string 'str'.                                   */

VMADDR EXPENTRY MexStoreString(char *name, char *str)
{
  word len;
  VMADDR vmaDesc, vmaStr;
  IADDR ia;

  /* Calculate length of string */

  len=(word)strlen(str);

  /* Allocate space for descriptor and put it in symbol table */

  vmaDesc=MexEnterSymtab(name, sizeof(IADDR));

  /* Allocate space for string itself */

  if ((vmaStr=hpalloc(len+2))==END_HEAP)
    vm_err(err_no_ss);

  /* Create a descriptor which points to the string */

  ia.segment=SEG_GLOBAL;
  ia.offset=vmaStr;
  ia.indirect=FALSE;

  /* Store the descriptor */

  *(IADDR *)DSEG(vmaDesc)=ia;

  /* Store the string, which is the length followed by the actual           *
   * contents of the string.                                                */

  *(word *)DSEG(vmaStr)=len;
  memmove(DSEG(vmaStr+2), str, len);

  return vmaDesc;
}


/* Store a byte string in memory, but don't create the descriptor.          *
 * Simply return the descriptor and let the caller figure out what          *
 * to do with it.                                                           */

IADDR EXPENTRY MexStoreHeapByteString(char *str, int len)
{
  VMADDR vmaStr;
  IADDR ia;

  /* Allocate space for string itself */

  if ((vmaStr=hpalloc(len+2))==END_HEAP)
    vm_err(err_no_ss);

  /* Create a descriptor which points to the string */

  ia.segment=SEG_GLOBAL;
  ia.offset=vmaStr;
  ia.indirect=FALSE;

  /* Store the string, which is the length followed by the actual           *
   * contents of the string.                                                */

  *(word *)DSEG(vmaStr)=len;
  memmove(DSEG(vmaStr+2), str, len);

  return ia;
}


/* Store the string, putting the descriptor at a particular point in the    *
 * address space.                                                           */

VMADDR EXPENTRY MexStoreStringAt(VMADDR vmaDesc, char *str)
{
  word len;
  VMADDR vmaStr;
  IADDR ia;

  /* Calculate length of string */

  len=(word)strlen(str);

  /* Allocate space for string itself */

  if ((vmaStr=hpalloc(len+2))==END_HEAP)
    vm_err(err_no_ss);

  /* Create a descriptor which points to the string */

  ia.segment=SEG_GLOBAL;
  ia.offset=vmaStr;
  ia.indirect=FALSE;

  /* Store the descriptor */

  *(IADDR *)DSEG(vmaDesc)=ia;

  /* Store the string, which is the length followed by the actual           *
   * contents of the string.                                                */

  *(word *)DSEG(vmaStr)=len;
  memmove(DSEG(vmaStr+2), str, len);

  return vmaDesc;
}


/* Store a "byte string" (a string with a specified length, as opposed      *
 * to a length determined by strlen()) at a particular VM location.         */

VMADDR EXPENTRY MexStoreByteStringAt(VMADDR vmaDesc, char *str, int len)
{
  VMADDR vmaStr;
  IADDR ia;

  /* Allocate space for string itself */

  if ((vmaStr=hpalloc(len+2))==END_HEAP)
    vm_err(err_no_ss);

  /* Create a descriptor which points to the string */

  ia.segment=SEG_GLOBAL;
  ia.offset=vmaStr;
  ia.indirect=FALSE;

  /* Store the descriptor */

  *(IADDR *)DSEG(vmaDesc)=ia;

  /* Store the string, which is the length followed by the actual           *
   * contents of the string.                                                */

  *(word *)DSEG(vmaStr)=len;
  memmove(DSEG(vmaStr+2), str, len);

  return vmaDesc;
}




