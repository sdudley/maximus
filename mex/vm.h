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

#ifndef VM_H_DEFINED__
#define VM_H_DEFINED__

#ifndef MEX_H_DEFINED__
  #include "mex.h"
#endif

/*****************************************************************************
                          Compile-time flags
 *****************************************************************************/

#define DEBUGVM         /* Enable minimal debugging support */

#if !defined(EXPENTRY) && !defined(OS_2)
#define EXPENTRY far pascal
#endif


/*****************************************************************************
                       Forward type declarations
 *****************************************************************************/

struct _vm_quad;
typedef struct _vm_quad INST;


/*****************************************************************************
                          Macro definitions
 *****************************************************************************/

#ifdef UNIX
/* # define makeVMADDR(x) ((VMADDR)NULL + x) */
# define makeVMADDR(x) x
#else
# define makeVMADDR(x) x
#endif

#ifdef VM_INIT
# define vm_extern
# define VM_IS(x) =makeVMADDR(x)
# define VM_LEN(x) x
#else
# define vm_extern extern
# define VM_IS(x)
# define VM_LEN(x)
#endif

/* Macros for pushing and popping values on stack */

#ifdef DEBUGSTACK
  #define DebPush(val, t) printf("Ps(" #val ",%d) ", sizeof(t));
  #define DebPop(val, t)  printf("Po(" #val ",%d) ", sizeof(t));
#else
  #define DebPush(val, t)
  #define DebPop(val, t)
#endif

#define Push(val, t) do { pbSp -= sizeof(t); *(t *)pbSp=*(t *)&val; DebPush(val,t); } while (0)
#define Pop(val, t)  do { val=*(t *)pbSp; pbSp += sizeof(t); DebPop(val, t); } while (0)


/* This DSEG macro turns a VMADDR offset into a pointer that can be used
 * by C code.
 */

#define DSEG(x) ((char *)(pbDs+(x))) /* Macro for accessing objects in dataseg */


/*****************************************************************************
                          Constant definitions
 *****************************************************************************/

#define END_HEAP      0       /* Marker used in the heap routines to
                               * end of the heap.
                               */

#define VMF_DEBEXE    0x0001  /* VM flags used to indicate what type of   */
#define VMF_DEBHEAP   0x0002  /* debugging support is requested.          */


#define MAX_REGS      100     /* Max number of registers of each size */
#define MAXERRLEN     240     /* Max length of a VM run-time error msg */


/*****************************************************************************
		       Miscellaneous structure definitions
 *****************************************************************************/

/* _args - used to hold the arguments for a particular opcode */

struct _args
{
  byte b1, b2;
  word w1, w2;
  dword dw1, dw2;
  IADDR a1, a2;
};


/* _opproc - used to create table of instruction opcodes, and the supporting
 * functions which get called when that specific opcode is encountered.
 */
   
struct _opproc
{
  int (*op_proc)(INST *inst, struct _args *arg);
  word op_val;
#ifdef DEBUGVM
  char *lit;
#endif
};


/* _rtsym - Run-time symbol table (for global references only) */

struct _rtsym
{
  char name[MAX_GLOB_LEN];
  VMADDR offset;
};

/* _dsheap - head of the data segment heap.  This structure is used to
 * track information about used and free blocks in the MEX program heap.
 */

struct _dsheap
{
#ifdef HEAP_SIGNATURE
  #define DSHEAP_SIG 0x6566
  word sig;
#endif
  VMADDR size;
  VMADDR next;
  byte free;
  byte rsvd;
};

/* _usrfunc - this is used as part of an array of structures to define
 * all of the application-specific functions which can be called
 * by MEX programs.
 */

vm_extern struct _usrfunc
{
  char *name;
  word (EXPENTRY *fn)(void);
  VMADDR quad;
} *usrfn;


/*****************************************************************************
                    Structures describing a *.VM file
 *****************************************************************************/

/* Structure of a .VM file:

{
  struct _vmh;
  struct _vm_quad[vmh.n_inst];
  struct _imp[vmh.n_imp]  { struct _ipat[imp.n_patch] }
  struct _funcdef[vmh.n_fdef];
  struct _funccall[vmh.n_fcall] { VMADDR[dfc.n_quads] };
}

*/



/* Union to hold either a literal integral type or an address */

union _lit_or_addr
{
  IADDR addr;
  byte litbyte;
  word litword;
  dword litdword;
  IADDR litstr;
};


/* A quadruple, or instruction quad.  This is what the VM executes, and
 * what it uses as virtual instructions.
 */

struct _vm_quad
{
  QUADOP opcode; /* Type of opcode - see mex.h */
  FORM opform;    /* Type of instruction: byte, word, dword */
  byte flag;      /* Bit flags for this instruction */
  byte rsvd;      /* Reserved for future use */

  
  #define FLAG_ARG1_LIT     0x01      /* Argument 1 is a literal value */
  #define FLAG_ARG1_ADDR    0x02      /* Argument 1 is an address */
  #define FLAG_ARG2_LIT     0x04      /* Argument 2 is a literal value */
  #define FLAG_ARG2_ADDR    0x08      /* Argument 2 is an address */
  #define FLAG_ARG1_SIGNED  0x10      /* Argument 1 is signed */
  #define FLAG_ARG2_SIGNED  0x20      /* Argument 2 is signed */

  union _lit_or_addr arg1, arg2;      /* First and second arguments */
  
  union                               /* Where to put the result */
  {
    VMADDR jump_label;
    IADDR dest;
  } res;
};


/* Virtual machine header structure.  Used at the beginning of a .VM file */

struct _vmh
{
  char vmtext[20];      /* Copyright text etc. when file is "type"d */
  VMADDR n_inst;        /* Number of instructions to read in */
  VMADDR n_imp;         /* Number of global data refs */
  VMADDR n_fdef;        /* Number of functions defined */
  VMADDR n_fcall;       /* Number of function calls */
  VMADDR lHeapSize;     /* Size of heap */
  VMADDR lStackSize;    /* Size of stack */
  VMADDR lGlobSize;     /* Size of DATA/BSS globals */
};



/* On-disk representation of a function declaration record */

struct _dfuncdef
{
  char name[MAX_GLOB_LEN];
  VMADDR quad;
};


/* On-disk representation of a function which is called */

struct _dfcall
{
  char name[MAX_GLOB_LEN];   /* Name of function we're calling */
  dword n_quads;
  /* Following this structure:
  VMADDR quad[n_quads];   Quad number FROM WHICH THE FUNCTION WAS CALLED
  */
};


/* Linked list of all declared functions in this file, plus their starting
 * quad numbers.
 */

struct _funcdef
{
  char *name;
  VMADDR quad;
  struct _funcdef *next;
};


/* LList of structure used for each function CALLED in our program */

struct _fcall
{
  char *name;   /* Name of function we're calling */
  VMADDR quad; /* Quad number FROM WHICH THE FUNCTION WAS CALLED */
  word written; /* Has this already been written to disk? */
  struct _fcall *next;
};


/*****************************************************************************
                            Global variables
 *****************************************************************************/

#ifdef COMPILING_MEX_VM  /* Only define if we're compiling the VM */
  vm_extern void (_stdc *pfnLogger)(char *szStr, ...);
  vm_extern INST *pinCs;           /* The code segment - array of quadruples */

  vm_extern byte *pbDs;              /* Data segment.  For global vars only! */
  vm_extern byte *pbSp;              /* Stack pointer */
  vm_extern byte *pbBp;              /* Value of 'sp' at function entry */

  vm_extern struct _dsheap *pdshDheap;

  vm_extern struct _rtsym *rtsym;  /* Pointer to run-time symbol table */
  vm_extern struct _vmh vmh;       /* Header from beginning of file */

  vm_extern VMADDR vaIp, high_cs;
  vm_extern VMADDR n_rtsym VM_IS(256);      /* Max 256 global symbols */
  vm_extern VMADDR n_entry VM_IS(0);        /* Current number of entries in table */

  vm_extern struct _funcdef *fdlist VM_IS(NULL);

  vm_extern int deb VM_IS(FALSE);
  vm_extern int debheap VM_IS(FALSE);

  /***************************************************************************
                              Error messages
   ***************************************************************************/

  vm_extern char err_cdata_ovfl[] VM_IS("Stack/heap collision. (Constant data area too large!)");
  vm_extern char err_divby0[] VM_IS("Division by zero");
  vm_extern char err_invalid_opcode[] VM_IS("Invalid opcode");
  vm_extern char err_invalid_reg[] VM_IS("Internal err: invalid register");
  vm_extern char err_invalid_access[] VM_IS("Invalid data access: seg %x ofs %08lx");
  vm_extern char err_invalid_optype[] VM_IS("Invalid optype");
  vm_extern char err_invalid_form[] VM_IS("Invalid form");
  vm_extern char err_too_many_rtsym[] VM_IS("Too many global variables");
  vm_extern char err_patch_ofs[] VM_IS("Patch bounds exception");
  vm_extern char err_cant_pass_reg_addr[] VM_IS("Cannot pass address of register");
  vm_extern char err_no_ss[] VM_IS("Out of string space");
  vm_extern char err_instr_too_big[] VM_IS("Cannot read .vm file larger than 64k");
#endif

/*****************************************************************************
			Global function prototypes
 *****************************************************************************/

#ifdef COMPILING_MEX_VM
  int init_symtab(int cRtSym);
  void hpinit(void);
  VMADDR hpalloc(word len);
  void hpfree(VMADDR ofs);
  void hpdbug(void);
  void *fetch(FORM form, IADDR *where);
  int store(IADDR *dest, FORM form, void *val);
  void _stdc vm_err(char *format,...);
  int VmRead(char *name);
  void kill_str(IADDR *strptr, IADDR *ptrptr);
#endif

/*****************************************************************************
                   Include of MEXVM.DLL interface definitions
 *****************************************************************************/

#include "vm_ifc.h"

#ifdef COMPILING_MEX_VM
  #include "vm_op.h"
#endif


#endif /* VM_H_DEFINED__ */

