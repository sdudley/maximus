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
static char rcs_id[]="$Id: vm_run.c,v 1.5 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#define VM_INIT
#define COMPILING_MEX_VM

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "mex.h"
#include "vm.h"
#include "dv.h"

/* Pointer to function to write to log file */

void _stdc NullLogger(char *szStr, ...) { NW(szStr); }

/* Hooks to be called upon entry/exit to intrinsic functions */

static void EXPENTRY (*pfnHookBefore)(void) = 0;
static void EXPENTRY (*pfnHookAfter)(void) = 0;

/* setjmp buffer used for internal VM errors */

static jmp_buf jbError;

#ifdef DEBUGVM
  #define tup(a,b,c) {a, b, c}
#else
  #define tup(a,b,c) {a, b}
#endif

/* opproc - table describing all of the available opcodes */

static struct _opproc opproc[]=
{
  tup(op_add,       QOP_ADD,        "ADD"), /* this entry duplicated to fill zeroth position in table */
  tup(op_add,       QOP_ADD,        "ADD"),
  tup(op_subtract,  QOP_SUBTRACT,   "SUBTRACT"),
  tup(op_multiply,  QOP_MULTIPLY,   "MULTIPLY"),
  tup(op_divide,    QOP_DIVIDE,     "DIVIDE"),
  tup(op_mod,       QOP_MODULUS,    "MOD"),

  tup(op_logical,   QOP_LE,         "LE"),
  tup(op_logical,   QOP_LT,         "LT"),
  tup(op_logical,   QOP_EQ,         "EQ"),
  tup(op_logical,   QOP_NE,         "NE"),
  tup(op_logical,   QOP_GE,         "GE"),
  tup(op_logical,   QOP_GT,         "GT"),
  tup(op_logical,   QOP_LOR,        "LOR"),
  tup(op_logical,   QOP_LAND,       "LAND"),

  tup(op_assign,    QOP_ASSIGN,     "ASSIGN"),
  tup(op_b2w,       QOP_BYTE2WORD,  "B2W"),
  tup(op_b2dw,      QOP_BYTE2DWORD, "B2DW"),
  tup(op_w2b,       QOP_WORD2BYTE,  "W2B"),
  tup(op_w2dw,      QOP_WORD2DWORD, "W2DW"),
  tup(op_dw2b,      QOP_DWORD2BYTE, "DW2B"),
  tup(op_dw2w,      QOP_DWORD2WORD, "DW2W"),

  tup(op_jmp,       QOP_JZ,         "JZ"),
  tup(op_jmp,       QOP_JNZ,        "JNZ"),
  tup(op_jmp,       QOP_JMP,        "JMP"),

  tup(op_funcstart, QOP_FUNCSTART,  "FUNCSTART"),
  tup(op_funcret,   QOP_FUNCRET,    "FUNCRET"),
  tup(op_startcall, QOP_STARTCALL,  "STARTCALL"),
  tup(op_arg_val,   QOP_ARG_VAL,    "ARG_VAL"),
  tup(op_arg_val,   QOP_ARG_REF,    "ARG_REF"),
  tup(op_funcjump,  QOP_FUNCJUMP,   "FUNCJUMP"),
  tup(op_push,      QOP_PUSH,       "PUSH"),
  tup(op_pop,       QOP_POP,        "POP"),

  tup(op_range,     QOP_RANGE,      "RANGE"),
  tup(op_shr,       QOP_SHR,        "SHR"),
  tup(op_shl,       QOP_SHL,        "SHL"),
  tup(op_band,      QOP_BAND,       "BAND"),
  tup(op_bor,       QOP_BOR,        "BOR"),
  tup(op_nop,       QOP_NOP,        "NOP"),

  tup(op_scat,      QOP_SCAT,       "SCAT"),
  tup(op_slogical,  QOP_SLE,        "SLE"),
  tup(op_slogical,  QOP_SLT,        "SLT"),
  tup(op_slogical,  QOP_SEQ,        "SEQ"),
  tup(op_slogical,  QOP_SNE,        "SNE"),
  tup(op_slogical,  QOP_SGE,        "SGE"),
  tup(op_slogical,  QOP_SGT,        "SGT"),
  tup(op_scopy,     QOP_SCOPY,      "SCOPY"),
  tup(op_skill,     QOP_SKILL,      "SKILL"),
  tup(op_slval,     QOP_SLVAL,      "SLVAL"),
  tup(op_srval,     QOP_SRVAL,      "SRVAL"),
  tup(0,            0,              "*bug*")
};



/* Entrypoint for the fetch routine; this is used by user-declared          *
 * intrinsic functions.                                                     */

void * EXPENTRY MexFetch(FORM form, IADDR *where)
{
  return (fetch(form, where));
}




/* Returns a VMADDR for a pointer to an object in the data segment */

VMADDR EXPENTRY MexPtrToVM(void *ptr)
{
  return (VMADDR)((char *)ptr-(char *)pbDs);
}




/* Convert an IADDR to a VM offset.  Does NOT use the 'indirect' field! */

VMADDR EXPENTRY MexIaddrToVM(IADDR *pia)
{
  IADDR ia;

  ia=*pia;
  ia.indirect=FALSE;

  return MexPtrToVM(MexFetch(FormByte, pia));
}



/* Return the head of the MEX data segment */

void * EXPENTRY MexDSEG(VMADDR ofs)
{
  return DSEG(ofs);
}



/* Internal version of kill_str */

void kill_str(IADDR *strptr, IADDR *ptrptr)
{
  IADDR new;

  if (strptr->segment != SEG_GLOBAL)
    vm_err("kill_str: non-global segment");

  /* If the offset is zero, then it's just a blank string, so we can quit */

  if (strptr->offset==0)
    return;

  /* Now free the string that this address points to */

  hpfree(strptr->offset);


  /* Now set the string descriptor to point to an empty string at
   * pbDs:0000.
   */

  new.segment=SEG_GLOBAL;
  new.offset=0;
  new.indirect=FALSE;

  store(ptrptr, FormAddr, &new);
}



/* Kill a string in MEX-space */

void EXPENTRY MexKillString(IADDR *pwhere)
{
  IADDR where=*(IADDR *)MexFetch(FormString, pwhere);

  kill_str(&where, pwhere);
}





static int near VmInit(void)
{
  /* Initialize all of our <shudder> global variables */

  pinCs=NULL;
  pbSp=pbBp=NULL;
  pdshDheap=NULL;
  vaIp=high_cs=0;
  n_entry=0;

  memset(regs_1, '\0', sizeof regs_1);
  memset(regs_2, '\0', sizeof regs_2);
  memset(regs_4, '\0', sizeof regs_4);
  memset(regs_6, '\0', sizeof regs_6);

  debheap=0;

  return 0;
}




static int near vm_cleanup(void)
{
  struct _funcdef *fd, *fdnext;

  /* Free the code segment */

  if (pinCs)
  {
    free(pinCs);
    pinCs=NULL;
  }

  /* Free linked list of functions */

  for (fd=fdlist; fd; fdnext=fd->next, free(fd), fd=fdnext)
    if (fd->name)
      free(fd->name);

  fdlist=NULL;

  /* Free the run-time symbol table */

  if (rtsym)
  {
    free(rtsym);
    rtsym=NULL;
  }
  

  /* Free the data segment */

  if (pbDs)
  {
    free(pbDs);
    pbDs=NULL;
  }

  return 0;
}




void _stdc vm_err(char *format,...)
{
  va_list varg;
  char err[MAXERRLEN];
  
  va_start(varg,format);
  vsprintf(err,format,varg);
  va_end(varg);
  
  (*pfnLogger)("!MEX:  fatal cs:%08lx: %s", vaIp-1L, err);

  /* Error recovery!  It's ugly, but it gets us back to the main routine */

  longjmp(jbError, 1);
}


/* Exported function to display a run-time error */

void EXPENTRY MexRTError(char *szMsg)
{
  vm_err("%s", szMsg);
}

/* This routine fetches a value from the data segment (at address           *
 * 'where'), returning a void* pointer to the object.  This returned        *
 * pointer must be typecast into the appropriate value by the               *
 * requesting code.                                                         */

void * fetch(FORM form, IADDR *where)
{
  IADDR got;
  void *ret;

  got=*where;

  while (got.indirect)
  {
    got.indirect=FALSE;
    ret=fetch(FormAddr, &got);

    if (ret==NULL)
      vm_err(err_invalid_access, 0xffff, vaIp);

    got=*(IADDR *)ret;
  }
  
  switch (got.segment)
  {
    case SEG_AR:      return (pbBp+got.offset);
    case SEG_GLOBAL:  return (pbDs+got.offset);
    case SEG_TEMP:
      switch(form)
      {
        case FormByte:  return ((void *)&regs_1[got.offset-1000]);
        case FormWord:  return ((void *)&regs_2[got.offset-2000]);
        case FormDword: return ((void *)&regs_4[got.offset-4000]);
        case FormAddr:  
        case FormString:return ((void *)&regs_6[got.offset-6000]);
        default:        vm_err(err_invalid_reg);
      }
      break;
  }

  vm_err(err_invalid_access, got.segment, got.offset);
  return NULL;
}




/* This routine stores a value of an arbitrary type into the                *
 * data segment address pointed to by 'dest'.  The size of the              *
 * value is determined by 'form'.                                           */
   
int store(IADDR *dest, FORM form, void *val)
{
  void *dp;

  dp=fetch(form, dest);
  
  switch (form)
  {
    case FormByte:  *(byte  *)dp=*(byte  *)val; break;
    case FormWord:  *(word  *)dp=*(word  *)val; break;
    case FormDword: *(dword *)dp=*(dword *)val; break;
    case FormAddr:  *(IADDR *)dp=*(IADDR *)val; break;
    default:          vm_err(err_invalid_optype);
  } 
  
  return 0;
}







/* Add to the intrinsic function table */

static int near add_intrinsic_functions(unsigned short uscIntrinsic,
                                        struct _usrfunc *puf)
{
  /* Now add all of the internal "functions" to the list of functions       *
   * callable by the user's program.                                        */
   
  struct _funcdef *fdef;
  struct _usrfunc *uf;
  VMADDR i;
  int rc=TRUE;

  usrfn=puf;

  for (uf=usrfn, i=(VMADDR)-2L; uf < usrfn+uscIntrinsic; uf++, i--)
  {
    /* Assign a unique quad number to each intrinsic function */

    uf->quad=i;

    /* Now allocate a record for each f() on the global function list */

    if ((fdef=malloc(sizeof(struct _funcdef)))==NULL)
    {
      rc=FALSE;
      break;
    }

    fdef->quad=uf->quad;

    fdef->next=fdlist;
    fdlist=fdef;

    if ((fdef->name=strdup(uf->name))==NULL)
    {
      rc=FALSE;
      break;
    }
  }


  if (!rc)
  {
    struct _funcdef *fdNext;

    /* When running out of memory, free the entries that we
     * already allocated
     */

    for (fdef=fdlist; fdef; fdNext=fdef->next, free(fdef), fdef=fdNext)
      if (fdef->name)
        free(fdef->name);

    fdlist=NULL;
  }

  return rc;
}



/* Push an ASCIIZ string as a pass-by-ref argument for the function */
/*
"asdf" - ObjformValue,    val.str=0xZZZZ, indirect=FALSE
s1     - ObjformAddress,  addr=0xZZZZ, indirect=FALSE
ref s1 - ObjformAddress,  addr=0xZZZZ, indirect=TRUE
*/


static void near PushArgv(char *pszArgs)
{
  if (!pszArgs)
    pszArgs="";

  pbSp -= sizeof(IADDR);
  MexStoreStringAt(pbSp-pbDs, pszArgs);
}




/* Check to see if a subscript is in range */

int op_range(INST *inst, struct _args *arg)
{
  NW(inst);
  NW(arg);
  return 0;
}




int op_nop(INST *inst, struct _args *arg)
{
  NW(inst); NW(arg); /* no operation */
  return 0;
}









/* Process one instruction */

static int near proc_instruction(INST *inst)
{
  IADDR got;
  struct _args arg;
  int ret=0;
  FORM opform;
  
  opform=inst->opform;

  /* Find out the type of our operands.  If necessary, fetch the            *
   * appropriate values from the data segment, and place them in            *
   * local variables for later access by the actual opcode-processing       *
   * routines.                                                              */

  if (inst->opcode != QOP_FUNCRET)
  {
    if (inst->flag & FLAG_ARG1_ADDR)
    {
      if (inst->arg1.addr.indirect &&
          inst->opcode != QOP_ARG_VAL && inst->opcode != QOP_ARG_REF)
      {
        got=inst->arg1.addr;
        got.indirect=FALSE;

        arg.a1=*(IADDR *)fetch(FormAddr, &got);
      }
      else arg.a1=inst->arg1.addr;
    }
    else switch (opform)
    {
      case FormByte:
        if (inst->flag & FLAG_ARG1_LIT)
          arg.b1=inst->arg1.litbyte;
        else arg.b1=*(byte *)fetch(FormByte, &inst->arg1.addr);
        break;

      case FormWord:
        if (inst->flag & FLAG_ARG1_LIT)
          arg.w1=inst->arg1.litword;
        else arg.w1=*(word *)fetch(FormWord, &inst->arg1.addr);
        break;

      case FormDword:
        if (inst->flag & FLAG_ARG1_LIT)
          arg.dw1=inst->arg1.litdword;
        else arg.dw1=*(dword *)fetch(FormDword, &inst->arg1.addr);
        break;

      case FormString:
        if (inst->flag & FLAG_ARG1_LIT)
          arg.a1=inst->arg1.litstr;
        else arg.a1=*(IADDR *)fetch(FormString, &inst->arg1.addr);
    }


    /* If we're using the stringchar lval/rval ruples, the second           *
     * argument is always a word.                                           */
  
    if (inst->opcode==QOP_SLVAL || inst->opcode==QOP_SRVAL)
      opform=FormWord;

    /* Now handle the second argument */

    if (inst->opcode != QOP_ARG_VAL && inst->opcode != QOP_ARG_REF &&
        inst->opcode != QOP_BYTE2WORD && inst->opcode != QOP_BYTE2DWORD &&
        inst->opcode != QOP_WORD2BYTE && inst->opcode != QOP_WORD2DWORD &&
        inst->opcode != QOP_DWORD2BYTE && inst->opcode != QOP_DWORD2WORD)
    {
      if (inst->flag & FLAG_ARG2_ADDR)
      {
        if (inst->arg2.addr.indirect &&
            inst->opcode != QOP_ARG_VAL && inst->opcode != QOP_ARG_REF)
        {
          got=inst->arg2.addr;
          got.indirect=FALSE;

          arg.a2=*(IADDR *)fetch(FormAddr, &got);
        }
        else arg.a2=inst->arg2.addr;
      }
      else switch(opform)
      {
        case FormByte:
          if (inst->flag & FLAG_ARG2_LIT)
            arg.b2=inst->arg2.litbyte;
          else arg.b2=*(byte *)fetch(FormByte, &inst->arg2.addr);
          break;

        case FormWord:
          if (inst->flag & FLAG_ARG2_LIT)
            arg.w2=inst->arg2.litword;
          else arg.w2=*(word *)fetch(FormWord, &inst->arg2.addr);
          break;

        case FormDword:
          if (inst->flag & FLAG_ARG2_LIT)
            arg.dw2=inst->arg2.litdword;
          else arg.dw2=*(dword *)fetch(FormDword, &inst->arg2.addr);
          break;

        case FormString:
          if (inst->flag & FLAG_ARG2_LIT)
            arg.a2=inst->arg2.litstr;
          else arg.a2=*(IADDR *)fetch(FormString, &inst->arg2.addr);
      }
    }
  }


  /* Now find and execute the appropriate opcode */

  if (inst->opcode < QOP_QUAD_LAST)
  {
      #ifdef DEBUGVM
      if (deb)
        printf("%-10s: ", opproc[inst->opcode].lit);
      #endif

    (*opproc[inst->opcode].op_proc)(inst, &arg);
  }
  else
  {
    vm_err(err_invalid_opcode);
  }

  return ret;
}



/* Begin execution of the virtual machine.  */

static int near VmRun(char *pszArgs)
{
  struct _funcdef *fd;
  struct _usrfunc *uf;
  unsigned pop_size;
  VMADDR start_addr;

  /* Initialize instruction pointer to start of program */

  for (fd=fdlist; fd; fd=fd->next)
    if (eqstr(fd->name, "main"))
    {
      vaIp=fd->quad;
      break;
    }

  if (fd==NULL)
    vm_err("no main() function");
  
/*  printf("Execution begins - a program is hence born (at cs%04x)\n\n", vaIp);*/

  /* Initialize stack and base pointers to start of stack area.  (See
   * diagram at beginning of vm.h.)
   */

  pbBp = pbSp = pbDs + vmh.lGlobSize + vmh.lStackSize;

  PushArgv(pszArgs);

  /* The main() function should return() to offset 0xffff, which will       *
   * terminate execution.                                                   */

  start_addr=(VMADDR)-1;
  Push(start_addr, VMADDR);

  do
  {
    while (vaIp < high_cs)
    {
      #ifdef DEBUGVM
      if (deb)
      {
        printf("cs%08" UINT32_XFORMAT ": ", vaIp);
        fflush(stdout);
      }
      #endif

      proc_instruction(pinCs + vaIp++);

      #ifdef DEBUGVM
      if (deb)
        printf("\n");
      #endif
    }
    
    for (uf=usrfn; uf->name; uf++)
      if (vaIp==uf->quad)
      {
        Push(pbBp, byte *);

        pbBp=pbSp;

        if (pfnHookBefore)
          (*pfnHookBefore)();

        pop_size=(*uf->fn)();

        if (pfnHookAfter)
          (*pfnHookAfter)();

        Pop(pbBp, byte *);
        Pop(vaIp, VMADDR);

        pbSp += pop_size;

        break;
      }
      
    if (uf->name==NULL && vaIp != (VMADDR)-1L)
      vm_err("abnormal program termination");
  }
  while (vaIp != (VMADDR)-1L);

  #ifdef DEBUGVM
    if (debheap)
      hpdbug();
  #endif
  
  /* Return the value currently held in the FIRST word register */
  
  return (regs_2[0]);
}


/* As called from Maximus */

int EXPENTRY MexExecute(char *pszFile, char *pszArgs, dword fFlag,
                        unsigned short uscIntrinsic, struct _usrfunc *puf,
                        int (EXPENTRY *pfnSetup)(void),
                        void (EXPENTRY *pfnTerm)(short *psRet),
                        void (_stdc *pfnLog)(char *szStr, ...),
                        void (EXPENTRY *pfnHkBefore)(void),
                        void (EXPENTRY *pfnHkAfter)(void))
{
  short ret;

  pfnLogger = pfnLog ? pfnLog : NullLogger;
  pfnHookBefore = pfnHkBefore;
  pfnHookAfter = pfnHkAfter;

  /* Trap for an error further down */

  if (setjmp(jbError))
  {
    ret=1;
  }
  else
  {
    VmInit();

    /* Set our debugging options */

    if (fFlag & VMF_DEBEXE)
      deb=TRUE;

    if (fFlag & VMF_DEBHEAP)
      debheap=TRUE;


    /* Try to add the user's intrinsic functions */

    if (! add_intrinsic_functions(uscIntrinsic, puf))
      ret=1;
    else
    {
      /* Read in the MEX program */

      if (VmRead(pszFile)==-1)
      {
        ret=1;
      }
      else
      {
        /* If we have a user-defined initialization or set-up function,
         * call it now.
         */

        if (!pfnSetup || (ret=(*pfnSetup)())==0)
        {
          ret=VmRun(pszArgs);

          if (pfnTerm)
            (*pfnTerm)(&ret);
        }
      }
    }
  }

  vm_cleanup();
  return ret;
}


