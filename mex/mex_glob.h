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

#ifndef MEX_GLOB_H_DEFINED__
#define MEX_GLOB_H_DEFINED__

mex_extern word scope;              /* The current scope number             */
mex_extern VMADDR offset;           /* Offset into local AR for variables   */
mex_extern VMADDR maxoffset;        /* Largest amount of space allocated    */
mex_extern VMADDR glob_offset;      /* Offset in global ds for vars         */
mex_extern VMADDR maxgloboffset;    /* Max space allocated in ds:.          */
mex_extern SYMTAB *symtab;          /* The symbol table                     */

mex_extern int iDeclaringStruct equ(0);  /* Are we declaring a structure?   */
mex_extern char filename[PATHLEN];  /* Name of file we're processing        */
mex_extern char yytext[MAX_ID_LEN]; /* The current token                    */
mex_extern char vm_output equ(TRUE);/* Generate *.vm output file (vs txt)   */
mex_extern char show_addr equ(FALSE);/* Show addresses for quad output      */
mex_extern long linenum;            /* Current line number of input file    */
mex_extern char check_subs;         /* Check subscript bounds               */
mex_extern unsigned n_errors;       /* number of errors in compile          */
mex_extern unsigned n_warnings;     /* number of warns  in compile          */

mex_extern VMADDR this_quad equ(0);

mex_extern GOTO *gstack equ(NULL);

mex_extern struct _conval *cvlist equ(NULL);

mex_extern TYPEDESC ByteType
#ifdef MEX_INIT
                             = { sizeof(byte),   FormByte,    TRUE}
#endif
;

mex_extern TYPEDESC UnsignedByteType
#ifdef MEX_INIT
                             = { sizeof(byte),   FormByte,    FALSE}
#endif
;

mex_extern TYPEDESC WordType
#ifdef MEX_INIT
                             = { sizeof(word),   FormWord,    TRUE}
#endif
;

mex_extern TYPEDESC UnsignedWordType
#ifdef MEX_INIT
                             = { sizeof(word),   FormWord,    FALSE}
#endif
;

mex_extern TYPEDESC DwordType
#ifdef MEX_INIT
                             = { sizeof(dword),  FormDword,   TRUE}
#endif
;

mex_extern TYPEDESC UnsignedDwordType
#ifdef MEX_INIT
                             = { sizeof(dword),  FormDword,   FALSE}
#endif
;

mex_extern TYPEDESC VoidType
#ifdef MEX_INIT
                             = { 0,  FormVoid }
#endif
;

mex_extern TYPEDESC StringType
#ifdef MEX_INIT
                              = { sizeof(IADDR), FormString }
#endif
;

mex_extern TYPEDESC AddrType
#ifdef MEX_INIT
                             = { sizeof(IADDR),   FormAddr  }
#endif
;

mex_extern TYPEDESC EllipsisType
#ifdef MEX_INIT
                             = { sizeof(word),   FormEllipsis  }
#endif
;


mex_extern struct _optable
{
  word token;
  QUADOP op;
  QUADOP strop;
  char *str;
} optable[]
#ifdef MEX_INIT
=         { {T_BMULTIPLY,QOP_MULTIPLY,   QOP_NOP,  "*"  },
            {T_BDIVIDE,  QOP_DIVIDE,     QOP_NOP,  "/"  },
            {T_BMODULUS, QOP_MODULUS,    QOP_NOP,  "%"  },
            {T_BPLUS,    QOP_ADD,        QOP_SCAT, "+"  },
            {T_MINUS,    QOP_SUBTRACT,   QOP_NOP,  "-"  },
            {T_LE,       QOP_LE,         QOP_SLE,  "<=" },
            {T_LT,       QOP_LT,         QOP_SLT,  "<"  },
            {T_EQUAL,    QOP_EQ,         QOP_SEQ,  "=" },
            {T_NOTEQUAL, QOP_NE,         QOP_SNE,  "!=" },
            {T_GE,       QOP_GE,         QOP_SGE,  ">=" },
            {T_GT,       QOP_GT,         QOP_SGT,  ">"  },
            {T_ASSIGN,   QOP_ASSIGN,     QOP_SCOPY,":="  },
            {T_BAND,     QOP_BAND,       QOP_NOP,  "&"  },
            {T_BOR,      QOP_BOR,        QOP_NOP,  "|"  },
            {T_LAND,     QOP_LAND,       QOP_NOP,  "and"},
            {T_LOR,      QOP_LOR,        QOP_NOP,  "or" },
            {T_SHL,      QOP_SHL,        QOP_NOP,  "shl"},
            {T_SHR,      QOP_SHR,        QOP_NOP,  "shr"},
            {0,          QOP_BYTE2WORD,  QOP_NOP,  "ByteToWord"},
            {0,          QOP_BYTE2DWORD, QOP_NOP,  "ByteToDword"},
            {0,          QOP_WORD2BYTE,  QOP_NOP,  "WordToByte"},
            {0,          QOP_WORD2DWORD, QOP_NOP,  "WordToDword"},
            {0,          QOP_DWORD2BYTE, QOP_NOP,  "DwordToByte"},
            {0,          QOP_DWORD2WORD, QOP_NOP,  "DwordToWord"},
            {0,          QOP_NOP,        QOP_NOP,  "NOP"},
            {0,          QOP_JZ,         QOP_NOP,  "JZ"},
            {0,          QOP_JNZ,        QOP_NOP,  "JNZ"},
            {0,          QOP_JMP,        QOP_NOP,  "JMP"},
            {0,          QOP_RANGE,      QOP_NOP,  "RANGE"},
            {0,          QOP_FUNCSTART,  QOP_NOP,  "FUNCSTART"},
            {0,          QOP_FUNCRET,    QOP_NOP,  "FUNCRET"},
            {0,          QOP_FUNCJUMP,   QOP_NOP,  "FUNCJUMP"},
            {0,          QOP_STARTCALL,  QOP_NOP,  "STARTCALL"},
            {0,          QOP_ARG_VAL,    QOP_NOP,  "ARG_VAL"},
            {0,          QOP_ARG_REF,    QOP_NOP,  "ARG_REF"},
            {0,          QOP_SKILL,      QOP_NOP,  "SKILL"},
            {0,          QOP_SLVAL,      QOP_NOP,  "SLVAL"},
            {0,          QOP_SRVAL,      QOP_NOP,  "SRVAL"},
            {0,          QOP_PUSH,       QOP_NOP,  "PUSH"},
            {0,          QOP_POP,        QOP_NOP,  "POP"},
            {0,          0,              QOP_NOP,  ""}
          }
#endif
;



#endif /* MEX_GLOB_H_DEFINED__ */

