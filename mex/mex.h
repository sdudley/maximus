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

#ifndef MEX_H_DEFINED__
#define MEX_H_DEFINED__

#ifndef __TYPEDEFS_H_DEFINED
#include "typedefs.h"
#endif


#ifdef OS_2
  #define INCL_NOPM
  #include <pos2.h>
#endif

/*****************************************************************************
                           Macros and preprocessor stuff
 *****************************************************************************/

#ifdef MEX_INIT
  #define mex_extern
  #define equ(s) =s
#else
  #define mex_extern extern
  #define equ(s)
#endif


#define bcopy(f,t,size)  memmove(t,f,size)
#define isidchar(c)      (isalpha(c) || isdigit(c) || (c)=='_')

/*****************************************************************************
                           Constant definitions
 *****************************************************************************/

#define AR_CONTROL_DATA (sizeof(byte *)+sizeof(VMADDR)) /* Size of word to hold addr + bp */
#define GLOB_CONTROL_DATA 0             /* Start of global data seg         */
#define MAX_TEMP        1000            /* Max # of temporaries of one size */
#define MAX_ID_LEN      32              /* Max length of an identifier      */
#define MAX_ERRMSG_LEN  180             /* Max length of an err msg         */
#define SYMTAB_SIZE     512             /* # of symbol table entries        */
#define SSYMTAB_SIZE    32              /* # of symtab entries for structure*/
#define MAX_TN_LEN      255             /* Max length of a type's ASCII name*/
#define MAX_GLOB_LEN    32              /* Max 16 significant chars for
                                         * exernal indentifiers.
                                         */
#define DEFAULT_STACKSIZE 2048          /* Default stack size */
#define DEFAULT_HEAPSIZE  8192          /* Default heap size */

#define DECLDEBUG                       /* Debug variable decl's            */
#define SCOPEDEBUG                      /* Debug scope open/close           */
#define FUNCDEBUG                       /* Debug function calls             */
#define EXPRDEBUG                       /* Debug expressions                */


/* Defines for the ADDRESS structure */

#define SEG_GLOBAL  0x00                /* Variable is in the global heap   */
#define SEG_AR      0x01                /* Variable is in local AR          */
#define SEG_TEMP    0x02                /* Variable is a temp (register)    */



/*****************************************************************************
                            Type definitions
 *****************************************************************************/

typedef char *string;

typedef dword VMADDR;  /* Virtual machine address */

typedef byte IDTYPE;

typedef enum {ClassVariable, ClassField, ClassTypeName, ClassLabel,
              ClassFunc, ClassValidate, ClassArg, ClassError,
              ClassStruct, ClassStructName} CLASS;


typedef enum {FormByte, FormWord, FormDword, FormString, FormArray, 
              FormStruct, FormAddr, FormVoid, FormEllipsis} FORM;

typedef enum QuadOps
{
  QOP_QUAD_FIRST=0,
  QOP_ADD,
  QOP_SUBTRACT,
  QOP_MULTIPLY,
  QOP_DIVIDE,
  QOP_MODULUS,
    
  QOP_LE,
  QOP_LT,
  QOP_EQ,
  QOP_NE,
  QOP_GE,
  QOP_GT,
  QOP_LOR,
  QOP_LAND,
  
  QOP_ASSIGN,
  QOP_BYTE2WORD,
  QOP_BYTE2DWORD,
  QOP_WORD2BYTE,
  QOP_WORD2DWORD,
  QOP_DWORD2BYTE,
  QOP_DWORD2WORD,
    
  QOP_JZ,
  QOP_JNZ,
  QOP_JMP,
    
  QOP_FUNCSTART,
  QOP_FUNCRET,
  QOP_STARTCALL,
  QOP_ARG_VAL,
  QOP_ARG_REF,
  QOP_FUNCJUMP,
  QOP_PUSH,
  QOP_POP,
    
  QOP_RANGE,
  QOP_SHR,
  QOP_SHL,
  QOP_BAND,
  QOP_BOR,
  QOP_NOP,
    
  QOP_SCAT,
  QOP_SLE,
  QOP_SLT,
  QOP_SEQ,
  QOP_SNE,
  QOP_SGE,
  QOP_SGT,
  QOP_SCOPY,
  QOP_SKILL,
  QOP_SLVAL,
  QOP_SRVAL,
    
  QOP_QUAD_LAST
} QUADOP;


/*****************************************************************************
                      Forward structure declarations
 *****************************************************************************/

struct _typedesc;
typedef struct _typedesc TYPEDESC;

struct _attributes;
typedef struct _attributes ATTRIBUTES;

struct _address;
typedef struct _address ADDRESS;

struct _dataobj;
typedef struct _dataobj DATAOBJ;

struct _funcargs;
typedef struct _funcargs FUNCARGS;

struct _goto;
typedef struct _goto GOTO;

struct _tlist;
struct _tlistlist;

typedef struct _tlist TLIST;
typedef struct _tlistlist TLLIST;

struct _patch;
typedef struct _patch PATCH;



/**************************************************************************
 *                        MISCELLANEOUS STRUCTURES                        *
 **************************************************************************/

/* Instruction ADDR structure.  Used to represent an address within
 * an INST instruction.
 */
   
typedef struct _iaddr
{
  byte    segment;  /* SEG_AR, SEG_GLOBAL, or SEG_TEMP                    */
  byte    indirect; /* If var is indirect or not                          */
  VMADDR  offset;   /* Offset from start of AR                            */
} IADDR;


typedef struct _opttype
{
  word bool;
} OPTTYPE;

typedef struct _symtab
{
  unsigned len;
  struct _symtab *parent;
  ATTRIBUTES **table;
} SYMTAB;


struct _address
{
  byte segment;       /* SEG_AR, SEG_FORMAL, SEG_GLOBAL, or SEG_TEMP        */
  byte indirect;      /* If var is indirect or not                          */
  VMADDR offset;      /* Offset from start of AR                            */
  TYPEDESC *typedesc; /* Type descriptor of pointed-to object               */
};


/* Linked list of function/procecure arguments */

struct _funcargs
{
  byte *name;         /* Formal name of this argument */
  TYPEDESC *type;     /* Pointer to type descriptor for this argument */
  word ref;           /* Is this argument passed by reference? */
  
  FUNCARGS *next;
};

/* A linked list used internally by ET, for keeping track of constant       *
 * data references.                                                         */

struct _conval
{
  struct _conval *next;
  VMADDR offset;
  VMADDR len;
  byte *buf;
};


/* An internal stack for processing 'goto' statements */

struct _goto
{
  GOTO *next;                     /* Next 'goto' in chain */
  char *name;                     /* Name of the symbol declared */
  VMADDR quad;                    /* Where this quad was referenced */
  VMADDR scope;                   /* Scope number that this was declared in */
};


/* Struct used for range-checking - passed to Generate() routine */

struct _rcheck
{
  VMADDR lo, hi;
  
  DATAOBJ *obj;
};


/* One of these is created in the on-disk .vm file for EACH reference
 * of every global name.  The _imp structure was created only once per
 * global symbol, whereas _ipat is used wherever that global symbol
 * was referenced.
 */

struct _ipat
{
  word argn;  /* 1 for arg1, 2 for arg2 */
  VMADDR ip;
};


/* A patch structure used within the .VM file.  One of these is created
 * for each unique global var name used in the source program.
 */
   
struct _imp
{
  byte name[MAX_GLOB_LEN];    /* Name of this symbol */
  VMADDR size;                /* Size of this sym in data segment */
  byte export;                /* If this is an export var (TRUE) or import  *
                               * (FALSE)                                    */
  byte init;                  /* If this sym was init'd (data follows _imp) */
  
  VMADDR n_patch;             /* Number of refs to this symbol              */
};




/* Linked list of offsets to patch in runtime code segment */

struct _ipatlist
{
  struct _ipat pat;
  struct _ipatlist *next;
};


/* Linked list of symbols to patch in the VM runtime symbol table.  This    *
 * struct includes the name of the symbol, and another linked list of       *
 * offsets to patch.                                                        */

struct _implist
{
  struct _conval *init;         /* Pointer to initial data for this obj */
  struct _imp ref;              /* Name of symbol to patch */
  struct _ipatlist *pat;        /* Linked list of offsets to patch */
  struct _implist *next;        /* Pointer to next symbol to patch */
};



/* List of temporary registers */

struct _tlist
{
  ADDRESS addr;
  TLIST *next;
};


/* List of list of temporary registers */

struct _tlistlist
{
  word size;
  TLIST *tlist;
  TLLIST *next;
};

/**************************************************************************
 *                        SEMANTIC STACK STRUCTURES                       *
 **************************************************************************/


typedef struct
{
  ATTRIBUTES *func;
  ATTRIBUTES *arg;
} FUNCCALL;



/* CONTTYPE - a constant literal, such as a string, byte, word, dword, etc */

typedef struct _consttype
{
  byte *lit;
  word val;
  dword dwval;
} CONSTTYPE;


/* TOKENTYPE - a record to indicate a token operator */

typedef struct _tokentype
{
  word operator;
} TOKEN;


/* RANGE - produced by 'range', when given the input 'x .. y' */

typedef struct _range
{
  VMADDR lo;
  VMADDR hi;
} RANGE;







typedef struct _valtype
{
  enum _valkind {ByteKind, WordKind, DwordKind,
                 StringKind} valkind;

  union
  {
    byte byteval;
    word wordval;
    dword dwordval;
    IADDR str;
  } kind;
} VALTYPE;

/* DATAOBJ - produced by expressions, records, structures, etc. */

struct _dataobj
{
  char *name;   /* If applicable.  Only for global variables, since the     *
                 * run-time routines need to know the name of this          *
                 * symbol for proper relocation at runtime.                 */

  word ref;           /* If doing func call, this arg should be pass-by-ref */
  TYPEDESC *type;
  TYPEDESC *argtype;  /* for use with fcalls only */
  DATAOBJ *next_arg;  /* for use with fcalls only */
  enum _objform {ObjformValue, ObjformAddress} objform;
  struct _conval *init; /* object represents initialized constant, SEG_GLOB */

  union
  {
    VALTYPE val;
    ADDRESS addr;
  } form;

};


struct _patch
{
  VMADDR quad;
  PATCH *next;
};


typedef struct _whiletype
{
  PATCH jump;
  VMADDR top_quad;
} WHILETYPE;


typedef struct _fortype
{
  VMADDR vmTest;
  VMADDR vmPost;
  VMADDR vmBody;
  PATCH paJzDone;
  PATCH paJmpBody;
  PATCH paJmpTest;
  PATCH paJmpPost;
} FORTYPE;

typedef struct
{
  PATCH *patchout;
  VMADDR else_label;
} ELSETYPE;





/**************************************************************************
 *                        TYPE DESCRIPTOR STRUCTURE                       *
 **************************************************************************/

struct _typedesc
{
  VMADDR size;                /* Size of the described type object */
  FORM form;                  /* FormWord, dword, byte, string, etc */
  int fSigned;                /* For byte,word,dword, TRUE if a signed type */

  union
  {
    struct  /* an array */
    {
      RANGE bounds;
      TYPEDESC *el_type;
    } array;

    struct  /* a structure */
    {
      char *name;     /* Name of this structure.  Points to attr.name. */
      SYMTAB *pst;    /* Symbol table for this structure */
      SYMTAB *post;   /* Revert to this table when done processing structure */
      VMADDR oscope;    /* Revert to this scope level when done processing */
      VMADDR ooffset;   /* Revert to this offset when done processing */
      VMADDR omaxoffset;/* Revert to this max offset when done processing */
    } struc;
  } typeinfo;

};



/**************************************************************************
 *                           ATTRIBUTES STRUCTURE                         *
 **************************************************************************/

typedef struct _funcattr
{
  VMADDR quad;                /* Quad number where this function starts  */
  VMADDR ar_size;             /* Size of this f()'s activation record     */
  VMADDR arg_size;            /* wSize of this f()'s formal parameters     */
  ATTRIBUTES *param;          /* Linked list of parameters for this f()   */
  TYPEDESC *ret_type;         /* Type descriptor for this f()'s return type */
  byte declared;              /* Whether or not func body was declared    */
} FUNCATTR;


struct _attributes
{
  char *name;
  VMADDR scope;               /* Scope number of this variable          */
  word ref;                   /* If class==ClassArg: pass-by-reference  */

  ATTRIBUTES *_next;          /* For use by symtab routines ONLY!       */
  ATTRIBUTES *next_decl;      /* The next declaration in list           */

  CLASS class;                /* Class (variable, constant, typename)   */
  TYPEDESC *typedesc;         /* Type of this object                    */

  union
  {
    VMADDR label_quad_no;     /* If class==ClassLabel                   */
    ADDRESS addr;             /* If class==ClassVariable or ClassArg    */
    FUNCATTR f;               /* If class==ClassProc                    */
    SYMTAB *pst;              /* If class==ClassStruct                  */
  } c;
};


#ifndef MEX_PARSER
#include "mex_tab.h"  /* Constants produced by the yacc (bison) compiler    */
#endif

#include "mex_lex.h"  /* Lexical analysis structs and constants */
#include "mex_prot.h" /* Function prototypes */
#include "mex_glob.h" /* Global variables */
#include "mex_err.h"  /* Error numbers */

#endif /* MEX_H_DEFINED__ */

