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
static char rcs_id[]="$Id: vm_read.c,v 1.1.1.1 2002/10/01 17:54:17 sdudley Exp $";
#pragma on(unreferenced)

#define COMPILING_MEX_VM

#include <string.h>
#include "prog.h"
#include "bfile.h"
#include "vm.h"


static void NoMem(void)
{
  (*pfnLogger)("!MEX:  out of memory reading file");
}

/* Read in the file header for a .vm file and all of the instructions. */

static int VmReadFileHdr(BFILE b)
{
  unsigned int byt;
  VMADDR ofs;

  /* Read in the _vmheader structure, which contains all of the necessary
   * information to read in this source file.
   */

  if (Bread(b, (char *)&vmh, sizeof(struct _vmh)) != sizeof(struct _vmh))
    return -1;

  high_cs=vmh.n_inst;

  /* Add enough extra space to add the empty string below.
   *
   * Also allow an additional 2K for variables that are added by
   * Maximus itself, such as user name, file area, and so on.
   */

  vmh.lGlobSize += 2048;


  /* Allocate space in the data segment */
  
  if ((pbDs=malloc(vmh.lGlobSize + vmh.lStackSize + vmh.lHeapSize))==NULL)
  {
    NoMem();
    return -1;
  }


  /* Initialize the system heap */

  hpinit();


  /* Allocate the run-time symbol table.  Allow for 256 symbols plus
   * whatever we refer to in the file.
   */

  if (init_symtab(256 + vmh.n_imp)==-1)
    return -1;
  
  /* Before we do anything else, create an empty string at pbDs:0.  This
   * is required because our string constants need to be initialized
   * to a certain value!
   */

  ofs=MexEnterSymtab(" NULL ", sizeof(word));
  *(VMADDR *)(pbDs+ofs)=0;


  /* Grab enough memory for the code segment, and return if it fails */
  
  if ((pinCs=malloc(sizeof(INST) * high_cs))==NULL)
  {
    NoMem();
    return -1;
  }

  byt=sizeof(INST)*vmh.n_inst;


#if defined(__MSDOS__) && !defined(__FLAT__)
  if ((long)sizeof(INST) * (long)vmh.n_inst > 65000L)
  {
    vm_err(err_patch_ofs);
    return -1;
  }
#endif

  /* Try to grab all of the instructions into memory */

  if (Bread(b, (char *)pinCs, byt) != (signed int)byt)
    return -1;

  return 0;
}



/* Read in the global data reference locations and perform fixups */

static int VmReadGlobDataRefs(BFILE b)
{
  int ref;
  int pat;
  VMADDR ofs;
  struct _ipat gpat;
  struct _imp iref;

  for (ref=vmh.n_imp; ref--; )
  {
    if (Bread(b, (char *)&iref, sizeof(struct _imp)) != sizeof(struct _imp))
      return -1;

    /* Enter this symbol in the run-time symbol table */

    ofs=MexEnterSymtab(iref.name, iref.size);

    /* Initialize constant values */

    if (iref.init)
    {
      if (Bread(b, (char *)pbDs+ofs, iref.size) != (signed)iref.size)
        return -1;
    }
    else
    {
      /* No specific initializer, so just set it to zero */

      memset((char *)pbDs+ofs, '\0', iref.size);
    }

    /* Read in all of the patch offsets, and patch the instructions
     * appropriately, so that they refer to the right portion of our
     * RT symbol table.
     */

    for (pat=iref.n_patch; pat--;)
    {
      /* Read in this patch structure */

      if (Bread(b, (char *)&gpat, sizeof(struct _ipat)) != sizeof(struct _ipat))
        return -1;

      /* Make sure that this patch won't overwrite our memory */

      if (gpat.ip > high_cs)
        vm_err(err_patch_ofs);

      /* Now patch the appropriate argument, filling it in with the
       * offset allocated to the symbol's name.
       */

      if (gpat.argn==1)
        pinCs[gpat.ip].arg1.addr.offset += ofs;
      else
        pinCs[gpat.ip].arg2.addr.offset += ofs;
    }
  }

  return 0;
}



/* Read in the list of function exports and add to run-time symbol table */

static int VmReadFuncExports(BFILE b)
{
  int fdef;

  /* Now read in all of the function definition recorpbDs */

  for (fdef=vmh.n_fdef; fdef--; )
  {
    struct _dfuncdef dfd;
    struct _funcdef *pfd;

    if (Bread(b, (char *)&dfd, sizeof(dfd)) != sizeof(dfd) ||
        (pfd=malloc(sizeof(struct _funcdef)))==NULL)
    {
      NoMem();
      return -1;
    }

    pfd->quad=dfd.quad;
    pfd->next=fdlist;
    fdlist=pfd;

    if ((pfd->name=strdup(dfd.name))==NULL)
      return -1;
  }

  return 0;
}



/* Read in imported function call references and patch the calls */

static int VmReadFuncImports(BFILE b)
{
  int fcall;

  for (fcall=vmh.n_fcall; fcall--; )
  {
    struct _dfcall dfc;
    struct _funcdef *fdl;
    VMADDR *pvma, *pvmaOrig;
    int size;

    /* Read the name of the function */

    if (Bread(b, (char *)&dfc, sizeof(dfc)) != sizeof(dfc))
    {
      return -1;
    }

    pvma=pvmaOrig=malloc(size=sizeof(VMADDR)*dfc.n_quads);

    if (pvma==NULL || Bread(b, (char *)pvma, (unsigned)size) != size)
    {
      NoMem();
      return -1;
    }

    /* Now scan the linked list of function declarations, and use this      *
     * to patch the appropriate offset for the appropriate FUNCJUMP quads. */

    for (fdl=fdlist; fdl; fdl=fdl->next)
      if (eqstr(dfc.name, fdl->name))
      {
        while (pvma < pvmaOrig+dfc.n_quads)
          pinCs[*pvma++].res.jump_label=fdl->quad;

        break;
      }

    free(pvmaOrig);

    /* If the function wasn't found, generate an error */

    if (fdl==NULL)
      vm_err("Undefined function '%s'", dfc.name);
  }

  return 0;
}



/* Read in the code/data from a .vm file */

static int near VmReadProc(BFILE b)
{
  int rc=0;

  pinCs=NULL;

  if (VmReadFileHdr(b)==-1)
    return -1;

  if (rc != -1 && VmReadGlobDataRefs(b)==-1)
    rc=-1;

  if (rc != -1 && VmReadFuncExports(b)==-1)
    rc=-1;

  if (rc != -1 && VmReadFuncImports(b)==-1)
    rc=-1;

  return rc;
}


/* Open and read a .vm file */

int VmRead(char *name)
{
  #define VM_BUFSIZE  8192

  char temp[PATHLEN];
  int ret;
  BFILE b;
  
  strcpy(temp,name);
  
  if ((b=Bopen(temp, BO_RDONLY | BO_BINARY, BSH_DENYNO, VM_BUFSIZE))==NULL)
  {
    strcat(temp, ".vm");
    
    if ((b=Bopen(temp, BO_RDONLY | BO_BINARY, BSH_DENYNO, VM_BUFSIZE))==NULL)
    {
      (*pfnLogger)("!MEX:  can't read file '%s'", name);
      return -1;
    }
  }
  
  if ((ret=VmReadProc(b))==-1)
      (*pfnLogger)("!MEX:  file format error in '%s'", temp);
  
  Bclose(b);
  return ret;
}


