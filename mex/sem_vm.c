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
static char rcs_id[]="$Id: sem_vm.c,v 1.1 2002/10/01 17:54:06 sdudley Exp $";
#pragma on(unreferenced)

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "bfile.h"
#include "vm.h"

static BFILE bVm;
static struct _vmh vmh;

static struct _implist *ilist=NULL; /* LList of global variables            */
static struct _fcall *fclist=NULL;  /* LList of functions we CALL           */
static struct _funcdef *fdlist=NULL;/* LList of functions we DEFINED        */

static int near copy_arg(union _lit_or_addr *arg, DATAOBJ *obj, word argn);
static int near WriteQuad(INST *quad);
static void near add_global(char *name, VMADDR size, struct _conval *init, word argn);
static void near add_funccall(ATTRIBUTES *f);
static void near add_funcdef(ATTRIBUTES *f);
static void near handle_func_arg(INST *inst, QUADOP op, DATAOBJ *o, DATAOBJ *o2);

#define VM_BUFSIZE  16384

int open_vm(char *name, long lStackSize, long lHeapSize, char *outfile)
{
  char *dot, *slash;
  
  strcpy(outfile,name);

  /* Find the last backslash, slash and period chars in the filename */
  
  slash=strrstr(outfile,"\\/");
  dot=strrchr(outfile,'.');
  
  /* Now trip off the top, if it comes after the last backslash */
  
  if ((!slash && dot) || (slash && dot && dot > slash))
    *dot='\0';
  
  /* Add the .vm extension to the filename */
  
  strcat(outfile, ".vm");

  if ((bVm=Bopen(outfile, BO_CREAT | BO_TRUNC | BO_RDWR | BO_BINARY,
                  BSH_DENYNO, VM_BUFSIZE))==NULL)
  {
    return -1;
  }

  memset(&vmh, '\0', sizeof(struct _vmh));
  
  /* Copy heap and stack size to file */

  vmh.lHeapSize = lHeapSize;
  vmh.lStackSize = lStackSize;
  vmh.lGlobSize = 0; /* update later after all data/bss objs are known */

  memmove(vmh.vmtext, "MEX Virtual Machine\x1a", 20);

  if (Bwrite(bVm, (char *)&vmh, sizeof(struct _vmh)) != sizeof(struct _vmh))
    return -1;

  return 0;
}


int close_vm(void)
{
  struct _implist *gp;
  struct _ipatlist *pl;
  struct _funcdef *fd, *fdnext;
  struct _fcall *fc, *fcnext;
  int n_imp, n_fdef, n_fcall; 

  /* Append to the end of the file */

  Bseek(bVm, 0L, BSEEK_END);

  /* Scan the linked list of symbol names, and output any import symbols */
  
  for (gp=ilist, n_imp=0; gp; gp=gp->next, n_imp++)
  {
    /* Write the name of the symbol to relocate, the number of              *
     * times it was referenced, etc.                                        */
    
    if (Bwrite(bVm, (char *)&gp->ref, sizeof(struct _imp))
                                              != sizeof(struct _imp))
    {
      return -1;
    }

    /* Write the constant initialization value to the data file */
    
    if (gp->ref.init)
      Bwrite(bVm, (char *)gp->init->buf, gp->ref.size);
    
    /* Now write all of the offsets where we'll need to patch */
    
    for (pl=gp->pat; pl; pl=pl->next)
      Bwrite(bVm, (char *)&pl->pat, sizeof(struct _ipat));
  }
 
  

  /* Now write all of the function export we DEFINE */

  for (fd=fdlist, n_fdef=0; fd; fdnext=fd->next, free(fd), fd=fdnext,n_fdef++)
  {
    struct _dfuncdef dfd;
    
    memset(&dfd, '\0', sizeof(struct _dfuncdef));

    strncpy(dfd.name, fd->name, MAX_GLOB_LEN-1);
    dfd.name[MAX_GLOB_LEN-1]='\0';

    dfd.quad=fd->quad;

    if (Bwrite(bVm, (char *)&dfd, sizeof(struct _dfuncdef))
                                                != sizeof(struct _dfuncdef))
      return -1;

    if (fd->name)
      free(fd->name);
  }
  

  /* Now write out the names of all the functions that we CALL. */

  for (fc=fclist, n_fcall=0; fc; fcnext=fc->next,free(fc),fc=fcnext/*,n_fcall++*/)
  {
    VMADDR *pvma, *pvmaOrig;
    struct _dfcall dfc;
    struct _fcall *tfc;
    int this_n_fcall, size;

    if (!fc->written)
    {
      /* Indicate that we're writing another fcall record */
      n_fcall++;

      /* Fill out the basic on-disk header */

      memset(&dfc, '\0', sizeof(struct _dfcall));
      strncpy(dfc.name, fc->name, MAX_GLOB_LEN-1);
      dfc.name[MAX_GLOB_LEN-1]='\0';

      /* Count the number of places which call this function */

      for (tfc=fc, this_n_fcall=0; tfc; tfc=tfc->next)
        if (!tfc->written && eqstr(fc->name, tfc->name))
          this_n_fcall++;

      dfc.n_quads=this_n_fcall;

      if (Bwrite(bVm, (char *)&dfc, sizeof(struct _dfcall))
                                                  != sizeof(struct _dfcall))
        return -1;

      /* Allocate memory for the list of quads */

      pvma=pvmaOrig=smalloc(size=sizeof(VMADDR) * this_n_fcall);

      /* Fill the list up with the quads where this function is called */

      for (tfc=fc; tfc && pvma < pvmaOrig+this_n_fcall; tfc=tfc->next)
        if (!tfc->written && eqstr(fc->name, tfc->name))
        {
          *pvma++=tfc->quad;
          tfc->written=TRUE;
        }

      /* Write the list to disk */

      if (Bwrite(bVm, (char *)pvmaOrig, (unsigned)size) != size)
        return -1;

      free(pvmaOrig);
    }

    /* Free the name of this quad */

    if (fc->name)
      free(fc->name);
  }

  
  
  Bseek(bVm, 0L, BSEEK_SET);
  
  /* Now go back and update the VMHeader information at the beginning */

  vmh.n_inst=this_quad;
  vmh.n_imp=n_imp;
  vmh.n_fdef=n_fdef;
  vmh.n_fcall=n_fcall;

  /* Re-write the VMHeader at the start of the file */
  
  if (Bwrite(bVm, (char *)&vmh, sizeof(struct _vmh)) != sizeof(struct _vmh))
    return -1;

  Bclose(bVm);
  
  return 0;
}

void GenerateVM(QUADOP op, DATAOBJ *o1, DATAOBJ *o2, ADDRESS *dest)
{
  INST quad;

  /* No operating needed for starting a function call */

  if (op==QOP_STARTCALL)
    return;


  /* Initialize structure to all zeroes */
  
  memset(&quad, '\0', sizeof(INST));
  
  /* Copy in the opcode */
  
  quad.opcode=op;
  
  
  /* Copy the arguments themselves into the quad structure */

  if (op != QOP_JMP && op != QOP_FUNCJUMP && op != QOP_FUNCJUMP &&
      op != QOP_FUNCSTART && op != QOP_FUNCRET &&
      op != QOP_PUSH && op != QOP_POP)
  {
    if (o1 && o1->type)
    {
      if (o1->type->fSigned)
        quad.flag |= FLAG_ARG1_SIGNED;

      if (o1->type->form==FormAddr || o1->type->form==FormArray)
      {
        quad.flag |= FLAG_ARG1_ADDR;

        /* If obj1 is an address, set the type of the quad to be that        *
         * of the second arg.  This is because addr+word=addr has a type      *
         * of 'word', just as addr+byte=addr has a type of 'byte'.  The       *
         * FLAG_ARGx_ADDR are used to determine that one of the args          *
         * is an address,  Obviously, if one of the args is an address,       *
         * the result must be an address too.                                 */

        if (op != QOP_ARG_REF)
        {
          if (o2)
            quad.opform=o2->type->form;
          else
          {
            /* No o2, so we have to go with the form of o1.  This should only *
             * happen when doing a QOP_ARGREF call anyway.                    */

            quad.opform=o1->type->form;
          }
        }
      }
      else
      {
        /* Oterhwise, the quad must have a type equal to that of the first   *
         * argument                                                           */

        quad.opform=o1->type->form;
      }
    }

    if (o2 && o2->type && o2->type->fSigned)
      quad.flag |= FLAG_ARG2_SIGNED;

    /* If both args are addresses, make sure that they're marked as such */

    if (quad.opform==FormAddr)
      quad.flag |= (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR);

    if (op != QOP_ARG_REF && o2 && o2->type->form==FormAddr)
      quad.flag |= FLAG_ARG2_ADDR;

    if (o1)
      copy_arg(&quad.arg1, o1, 1);

    if (o2 && op != QOP_ARG_REF && op != QOP_JZ && op != QOP_JNZ)
      copy_arg(&quad.arg2, o2, 2);
  }

  /* If it's a jump quad, copy in a fake destination address, so           *
   * that it can be properly backpatched later.                             */

  if (op==QOP_JMP || op==QOP_JZ || op==QOP_JNZ)
    quad.res.jump_label=0xffffu;
  else if (op==QOP_PUSH || op==QOP_POP)
  {
    ADDRESS *a=(ADDRESS *)o1;
    
/*    quad.flag |= FLAG_ARG1_ADDR;*/ /*SJD Tue  07-21-1992  17:47:20 */
    quad.arg1.addr.segment=a->segment;
    quad.arg1.addr.offset=a->offset;
    quad.arg1.addr.indirect=a->indirect;

    quad.opform=a->typedesc->form;
    
    if (op==QOP_PUSH)
    {
      if (quad.opform==FormAddr)
      {
        quad.flag |= FLAG_ARG1_ADDR;
        quad.arg1.addr.indirect=TRUE;
      }
    }

    if (op==QOP_POP)
      quad.flag |= FLAG_ARG1_ADDR;
    
    /*o1->objform=0;*/ /*SJD Mon  04-20-1992  22:59:54 */
  }
  else if (op==QOP_FUNCJUMP)
    add_funccall((ATTRIBUTES *)o1);
  else if (op==QOP_FUNCSTART)
    add_funcdef((ATTRIBUTES *)o1);
  else if (op==QOP_ARG_VAL || op==QOP_ARG_REF)
    handle_func_arg(&quad, op, o1, o2);
  else if (op==QOP_FUNCRET)
  {
    ATTRIBUTES *attr;
    PATCH p;
    
    /* Patch in the AR size */

    attr=(ATTRIBUTES *)o1;
    
    p.quad=attr->c.f.quad;
    p.next=NULL;
    
    BackPatchVM(&p, attr->c.f.ar_size);

    quad.arg1.litdword=attr->c.f.arg_size;
    quad.res.jump_label=attr->c.f.ar_size;
  }
  else if (op != QOP_SKILL && op != QOP_ASSIGN &&
           op != QOP_SCOPY)
  {
    /* it must be a destination address */

    quad.res.dest.segment=dest->segment;
    quad.res.dest.offset=dest->offset;
    quad.res.dest.indirect=dest->indirect;
  }
    
  /* Set the bitflags appropriately for either literal or address           *
   * values.                                                                */

  if (op != QOP_PUSH && op != QOP_POP)
  {
    if (o1 && o1->objform==ObjformValue)
      quad.flag |= FLAG_ARG1_LIT;
  
    if (op != QOP_ARG_REF && o2 && o2->objform==ObjformValue)
      quad.flag |= FLAG_ARG2_LIT;
  }
  
  WriteQuad(&quad);
}


/* Copy the argument pointed to by data object 'obj', into the              *
 * literal-or-address union 'arg'.                                          */

static int near copy_arg(union _lit_or_addr *arg, DATAOBJ *obj, word argn)
{
  /* Now copy in the arguments appropriately, whether they be literal       *
   * values or actual addresses.                                            */

  if (obj->objform==ObjformValue)
  {
    switch (obj->form.val.valkind)
    {
      case ByteKind:    arg->litbyte= obj->form.val.kind.byteval;  break;
      case WordKind:    arg->litword= obj->form.val.kind.wordval;  break;
      case DwordKind:   arg->litdword=obj->form.val.kind.dwordval; break;
      case StringKind:
        arg->litstr=  obj->form.val.kind.str;

        /* If it's a constant literal string, add it to the constant        *
         * pool.                                                            */

        if (obj->form.val.kind.str.segment==SEG_GLOBAL)
        {
          assert(obj->init != NULL);
          
          add_global(obj->name, obj->init->len, obj->init, argn);
        }
        break;

      default:          bug("Invalid valkind in sem_vm");
    }
    
    return TRUE;
  }
  else /* an address */
  {
    if (obj->form.addr.segment==SEG_GLOBAL && obj->form.addr.typedesc)
      add_global(obj->name, obj->form.addr.typedesc->size, NULL, argn);
    
    arg->addr.segment=obj->form.addr.segment;
    arg->addr.offset=obj->form.addr.offset;
    arg->addr.indirect=obj->form.addr.indirect;
    
    return FALSE;
  }
}


/* Write a quadruple to the .VM file. */

static int near WriteQuad(INST *quad)
{
  if (Bwrite(bVm, (char *)quad, sizeof(INST)) != sizeof(INST))
    return -1;
  
  #ifdef DEBUG
  printf("t=%d, %ld\n", this_quad, Btell(bVm));
  #endif

  this_quad++;
  return 0;
}


/* Backpatch a series of quadruples in the .VM file, setting them to        *
 * jump to the quad 'to_where'.                                            */

int BackPatchVM(PATCH *pat, VMADDR to_where)
{
  INST inst;
  PATCH *p;

  /* Scan the linked list of quads */

  for (p=pat; p; p=p->next)
  {
    /* Seek to the offset of this quad */

    Bseek(bVm, sizeof(struct _vmh)+(sizeof(INST)*(long)p->quad), BSEEK_SET);

    /* Read it in */

    if (Bread(bVm, (char *)&inst, sizeof(INST)) != sizeof(INST))
      return -1;

    /* Seek back to the start of the quad, so we can later write
     * over it.
     */

    Bseek(bVm, -(long)sizeof(INST), BSEEK_CUR);

    inst.res.jump_label=to_where;

    if (Bwrite(bVm, (char *)&inst, sizeof(INST)) != sizeof(INST))
      return -1;
  }

  /* Seek back to the end of the file, for appending more quadruples to     *
   * the end of the file.                                                   */

  Bseek(bVm, 0L, BSEEK_END);
  return 0;
}



/* Creates a record which will cause VM to allocate space for this symbol   *
 * on the run-time global heap.  No patch offsets are assigned by this      *
 * action.                                                                  */

struct _implist * add_one_global(char *name, VMADDR size, struct _conval *init)
{
  struct _implist *gp;

  /* If this identifier was not found, add a new one to the end of the list */

  if ((gp=malloc(sizeof(struct _implist)))==NULL)
    NoMem();

  memset(gp, '\0', sizeof(struct _implist));

  if (name)
    strncpy(gp->ref.name, name, MAX_GLOB_LEN-1);

  gp->ref.name[MAX_GLOB_LEN-1]='\0';

  gp->ref.size=size;
  gp->ref.n_patch=0;
  gp->ref.init=(byte)!!init;
  gp->init=init;
  gp->pat=NULL;

  gp->next=ilist;
  ilist=gp;

  /* Add the size of this global to our global data size */

  vmh.lGlobSize += gp->ref.size;
  
  return gp;
}



/* Add the following global variable to the list of global args, for        *
 * future relocation at runtime.                                            */


static void near add_global(char *name, VMADDR size, struct _conval *init, word argn)
{
  struct _implist *gp;
  struct _ipatlist *newpat;

  /* Allocate a new patch struct for the reference that we're about to add. */

  newpat=smalloc(sizeof(struct _ipatlist));


  /* Initialize it with this quad number and the proper argument to patch */
  
  newpat->pat.argn=argn;
  newpat->pat.ip=this_quad;
  newpat->next=NULL;

  /* Search the linked list of names, and try to find this object */

  gp=NULL;

  if (name)
  {
    for (gp=ilist; gp; gp=gp->next)
    {
      if (eqstr(name, gp->ref.name))
      {
        /* Stick this in at the beginning of the linked list */
  
        newpat->next=gp->pat;
        gp->pat=newpat;
        gp->ref.n_patch++;
        break;
      }
    }
  }

  /* If this identifier was not found, add a new one to the end of the      *
   * list, and add this as the first patch.                                 */

  if (!gp)
  {
    gp=add_one_global(name, size, init);
    gp->ref.n_patch++;
    gp->pat=newpat;
  }

  return;
}


/* Add the name of a function which was called to the _fcall linked list */

static void near add_funccall(ATTRIBUTES *f)
{
  struct _fcall *fc;
  
  if (!f)
    return;

  fc=smalloc(sizeof(struct _fcall));
  
  fc->name=sstrdup(f->name);
  fc->quad=this_quad;
  fc->next=fclist;
  fc->written=FALSE;
  fclist=fc;
}


static void near add_funcdef(ATTRIBUTES *f)
{
  struct _funcdef *fd;

  /* The beginning of a function -- add this to the linked list of        *
   * defined functions.                                                   */

  fd=smalloc(sizeof(struct _funcdef));

  fd->name=sstrdup(f->name);
  fd->quad=this_quad;

  fd->next=fdlist;
  fdlist=fd;
}



static void near handle_func_arg(INST *inst, QUADOP op, DATAOBJ *o, DATAOBJ *o2)
{
  /* Nothing needed to implement pass-by-value */
  
  if (op==QOP_ARG_VAL)
    return;
  
  /* Otherwise, it must be pass by reference.  So use the address of the    *
   * variable instead, and set to 'indirect'.                               */

  inst->flag |= FLAG_ARG1_ADDR;

  if (o->objform==ObjformAddress &&
      ((int)o2 != 0 || o->form.addr.indirect)) /*SJD Sat  05-18-1991  23:12:41 */
  {
    inst->arg1.addr.indirect=TRUE;
  }
}


