/* Log of changes:

     Date   Who  Description
   -------- ---  ------------------------------------------------
   91.03.11 sjd  Source documentation for yylex et al
   91.03.11 sjd  Begin symbol table routines
   91.03.11 sjd  Simple variable allocation routines
   91.03.12 sjd  Array declararation code
   91.03.12 sjd  Simple expression evaluation routines
   91.03.13 sjd  Temporary (register) tracking and freeing
   91.03.23 sjd  Type coercion & conversion
   91.03.27 sjd  Start VM code
   91.03.27 sjd  Continue work on VM code
   91.04.03 sjd  Fetch/store code for VM, structure for the actual
                 *.VM input files, run-time global symbol table, and
                 VM code relocation routines.  (RC)
   91.04.03 sjd  Implementation of 'if' statements and associated
                 semantic routines.
   91.04.04 sjd  Added code for writing *.VM files to ET itself.
                 (RC)
   91.04.04 sjd  Added global variable declarations, code in ET for
                 relocation of globals, and fixed up several
                 routines in both ET and VM.  VM can now read and
                 "execute" the *.VM files produced by ET, and
                 although there are currently no quads to produce
                 visible output, expressions can be evaluated and
                 the correct quad operators will be called.
   91.04.08 sjd  Start work on GOTO (eeeek!) code.
   91.04.09 sjd  Finish GOTO code, and fixed bug in backpatching
                 routine.
   91.04.11 sjd  Start on array manipulation code.  Indirect
                 addressing.
   91.04.15 sjd  Add ConPart to array code.  Fixed bug with
                 GenerateTXT() and the supposed type of declared
                 identifiers.  Wrote EvalUnary(), the code to handle
                 the unary minus for both literals and expressions.
   91.04.18 sjd  Added support for multidimensional (indirect)
                 arrays, and cleaned up several problems with 1D
                 arrays.  Begin coding for function/procedure calls.
   91.04.23 sjd  More work on function call code -- implemented
                 support for the declaration of formal parameters,
                 and allocation of same within the activation
                 record.
   91.04.28 sjd  Quads for actual function calls: STARTCALL,
                 parameter passing, and FUNCJUMP.
   91.05.02 sjd  Worked on SEM_VM.C (*.VM output code) and getting
                 the virtual machine up-to-snuff with the function
                 and array code.
   91.05.04 sjd  Spent several hours smoothing out the VM, fixing
                 bugs in the VM's handling of array code, and
                 stepping through the execution of several test
                 programs involving arrays, 'if' statements,
                 assignments, and parameterless function calls.
   91.05.05 sjd  Added support in VM for pass-by-value and
                 pass-by-reference. Major debugging effort for
                 arrays, indirect addressing, and more.
   91.05.10 sjd  Added string support to ET, and added code to write
                 out initialized variables (ie. strings) to the *.VM
                 file.  Wrote basic functions for dynamic heap
                 manager.
   91.05.12 sjd  Began implementation of string support for VM.
                 Tidied up the heap manager, fixed a bug which was
                 causing the virtual machine's stack to be wiped
                 out, and started the implementation of the SCOPY
                 and SEQ/SNE/SLT/SLE/SGT/SGE quads.
   91.05.16 sjd  Implementation of while() loops, some bugfixing of
                 array and string code, and started the
                 implementation of SKILL quads.
   91.05.18 sjd  Added escape characters to strings (newlines, tabs,
                 backspaces, etc.) and modified the lexical analyzer
                 to permit character constants and also the use of
                 escape characters within those constants.
                 Implemented the print() function, which includes
                 the modification of the grammar, the semantic
                 routines, and the virtual machine to display
                 characters, integers, longs and strings.  Fixed a
                 bug with type coercion which caused improper code
                 to be generated when the object on the 'left' side
                 of a binary operator was coerced to a different
                 type.  Implemented optional pass-by-reference for
                 all integral types, and made pass-by-reference
                 optional for strings.
   91.05.20 sjd  Added explicit typecasts for integral types.  Added
                 a 'return' statement to the grammer, with
                 supporting semantic routines. Modified the function
                 call routines to support the return of a value in
                 register 0, and also fixed up some miscellaneous
                 bugs related to parameter passing.  Modified the
                 lexical analyser to support preprocessor directives
                 (in this case, the inclusion of other files). Fixed
                 a bug with type coercion to a dword (long integer).
                 Wrote a random number function completely in .ET
                 source code.  Added support for function
                 prototypes, which permit functions to be called
                 ahead of that function's body.
   91.05.26 sjd  Fixed the bug with global arrays by modifying the
                 variable declaration routine to explicitly allocate
                 space on the run-time symbol table when the
                 variable is declared.  Also changed the relocation
                 module in the VM to simply increment the specified
                 argument by the patch amount, to permit the
                 compile-time computation of array ConParts to work
                 with global arrays.
   91.05.28 sjd  Overloaded the subscript operator for strings, and
                 implemented both lvalue and rvalue versions of the
                 'string subscript' function. Added arithmetic
                 shift-left and arithmetic shift-right operators, in
                 addition to logical AND, logical OR, bitwise AND
                 and bitwise OR.
   91.05.29 sjd  Fixed a bug which caused temporary registers
                 (passed as arguments) to not be freed after the
                 function call. Implemented an intrinsic time()
                 function to obtain the current system time.
                 Function prototypes will no longer erroneously
                 generate SKILL quads when the prototype is
                 declared.
   91.05.30 sjd  Fixed the code which handles pass-by-reference of
                 strings, such that a SKILL quad will NOT be
                 generated when the called function terminates.
                 (This was accidentally freeing the space used by
                 the caller to hold the string.)  Added support for
                 the catenation of dynamic strings, by overloading
                 the "+" operator. Made sure that only integral
                 types could be used for while() and if()
                 statements. Fixed a bug which accidentally left
                 pass-by-reference parameters visible outside of
                 their scope.  Fixed several precedence problems
                 with 'and' and 'or', and made the equalitiy
                 operator left-associative (as opposed to
                 non-associative).  Added safeguards to ensure that,
                 when passing by reference, that the parameter is
                 not coerced, since an lvalue is required for
                 pass-by-ref. Added the readch() intrinstic
                 function, and a readstr() .ET function to read
                 characters and strings from the keyboard,
                 respectively.
   91.05.31 sjd  Fixed a bug which caused char-sized parameters to
                 be processed incorrectly. Fixed a bug with
                 pass-by-reference strings.  Fixed a bug which
                 caused coerced parameters to disrupt the normal
                 order of objects pushed on the stack. Made the *.VM
                 writing routines more bulletproof, and fixed a bug
                 which caused the *.VM file to be corrupted if a
                 function name was more than 16 characters long.
   91.06.01 sjd  Added PUSH and POP quads to save registers across
                 procedure/function calls.  Fixed a bug with the
                 typecast routine which caused temporary registers
                 to be "lost" after typecasting them.  Also fixed a
                 similar bug, dealing with a temporary used as the
                 index for an array.  Finished coding the BlackJack
                 game.  Fixed bugs with passing non pass-by-ref
                 strings to functions.
   91.06.05 sjd  Recoded the instruction fetch code to use a
                 dispatch table instead of using a linear search,
                 thereby increasing performance from roughly 0.02 to
                 0.03 MIPS, or a 50% raw speed improvement.
   04.06.07 wwg  Modified grammar's form to include extra productions
                 const_byte_p, const_word_p, etc., to allow Solaris
		 (SYSV) yacc to run without failure due to type
		 clash errors. Theoretically, output will be the
		 same as these modifications *should* be purely
		 syntactic sugar.
*/


%{

  #define MEX_PARSER

  #include <stdio.h>
  #include <stdlib.h>
  #include <mem.h>
  #include "alc.h"
  #include "prog.h"
  #include "mex.h"

  #ifdef __TURBOC__
  #pragma warn -cln
  #endif
    
  ATTRIBUTES *curfn=NULL;

  /* $Id: mex_tab.y,v 1.3 2004/01/23 10:52:59 wmcbrine Exp $ */

%}


/* Semantic stack structure */

%union  {
          IDTYPE *id;
          TYPEDESC *typedesc;
          ATTRIBUTES *attrdesc;
          DATAOBJ *dataobj;
          RANGE range;
          CONSTTYPE constant;
          TOKEN token;
          PATCH patch;
          ELSETYPE elsetype;
          FUNCARGS *arg;
          FUNCCALL fcall;
          WHILETYPE whil;
          OPTTYPE opt;
          FORTYPE fr;
          word size;
        }

%start program

%type <typedesc> typename typedefn
%type <dataobj>  lval_ident ident literal primary paren_expr
%type <dataobj>  expr useless_expr useful_expr function_call
%type <dataobj>  expr_list opt_expr
%type <attrdesc> id_list
%type <id>       id 
%type <range>    range
%type <arg>      arg_list argument
%type <constant> const_string
%type <elsetype> else_part
%type <size>     trailing_part
%type <opt>      opt_ref

%token T_BYTE T_WORD T_DWORD T_STRING T_VOID
%token T_BEGIN T_END T_IF T_THEN T_ELSE
%token T_GOTO T_WHILE T_DO T_FOR
%token T_STRUCT T_DOT T_ELLIPSIS

%token T_LPAREN T_RPAREN T_LBRACKET T_RBRACKET T_REF T_RETURN
%token T_COMMA T_SEMICOLON T_COLON T_ARRAY T_RANGE T_OF T_UNSIGNED T_SIGNED
%token T_SIZEOF

%right T_ASSIGN
%left T_LAND T_LOR
%left T_NOTEQUAL T_EQUAL
%left T_LE T_LT T_GE T_GT
%left T_SHL T_SHR T_BAND T_BOR
%left T_BPLUS T_MINUS  
%left T_BMULTIPLY T_BDIVIDE T_BMODULUS

%token T_CONSTBYTE
%token T_CONSTWORD
%token T_CONSTDWORD
%token T_CONSTSTRING
%token T_ID

%type <dataobj> const_byte_p 
%type <dataobj> const_word_p
%type <dataobj> const_dword_p
%type <dataobj> const_string_p
%type <constant> T_CONSTWORD T_CONSTBYTE T_CONSTDWORD T_CONSTSTRING
%type <id> T_ID

%%

program         :       top_list
                ;

top_list        :       /* epsilon */
                |       top_list func_or_decl
                ;

func_or_decl    :       function
                |       declaration
                                { /* nothing */ }
                ;

function        :       typedefn id
                                { $<attrdesc>$=curfn=function_begin($1, $2); }
                        T_LPAREN 
                                { $<size>$=offset; scope_open(); }
                        arg_list T_RPAREN
                                { function_args($<attrdesc>3, $6); }
                        trailing_part
                                { VMADDR end_quad=this_quad;
                                  scope_close();
                                  function_end($<attrdesc>3, $9, end_quad);
                                  offset=$<size>5;
                                }
                ;

trailing_part   :       function_block
                                { $$=TRUE; }
                |       T_SEMICOLON
                                { $$=FALSE; }
                ;

function_block  :       T_BEGIN
                                { $<size>$=offset; GenFuncStartQuad(curfn); }
                        declarator_list statement_list T_END
                                {
                                  /* Reset the value of the offset pointer  *
                                   * for the local activation record.       */

                                  offset=$<size>2;
                                }
                ;
                
arg_list        :       /* epsilon */
                                { $$=NULL; }
                |       T_ELLIPSIS
                                { $$=declare_ellipsis(); }
                |       argument T_COMMA arg_list
                                { if ($1) $1->next=$3;
                                  $$=$1;
                                }
                |       argument
                                { if ($1) $1->next=NULL;
                                  $$=$1;
                                }
                ;

argument        :       opt_ref typename id
                                { $$=smalloc(sizeof(FUNCARGS));
                                  $$->type=$2; $$->name=sstrdup($3);
                                  $$->next=NULL;
                                  $$->ref=$1.bool;
                                }
                ;
                
opt_ref         :       /* epsilon */
                                { $$.bool=FALSE; }
                |       T_REF
                                { $$.bool=TRUE; }
                ;
                            

block           :       T_BEGIN
                                { scope_open(); $<size>$=offset; }
                        declarator_list statement_list T_END
                                {
                                  /* Reset the value of the offset pointer  *
                                   * for the local activation record.       */

                                  offset=$<size>2;
                                  scope_close();
                                }
                ;

                
declarator_list :       /* epsilon */
                |       declarator_list declaration
                ;

declaration     :       typename id_list T_SEMICOLON
                                { declare_vars($1,$2); }
                |       T_STRUCT id T_BEGIN
                                { $<typedesc>$=define_struct_id($2); }
                                declarator_list T_END T_SEMICOLON
                                { define_struct_body($<typedesc>4); }
                ;

typename        :       typedefn T_COLON
                                { $$=$1; /* default action */ }
                ;

typedefn        :       T_BYTE
                                { $$=&UnsignedByteType; }
                |       T_WORD
                                { $$=&WordType; }
                |       T_DWORD
                                { $$=&DwordType; }
                |       T_SIGNED T_BYTE
                                { $$=&ByteType; }
                |       T_SIGNED T_WORD
                                { $$=&WordType; }
                |       T_SIGNED T_DWORD
                                { $$=&DwordType; }
                |       T_UNSIGNED T_BYTE
                                { $$=&UnsignedByteType; }
                |       T_UNSIGNED T_WORD
                                { $$=&UnsignedWordType; }
                |       T_UNSIGNED T_DWORD
                                { $$=&UnsignedDwordType; }
                |       T_VOID
                                { $$=&VoidType; }
                |       T_STRING
                                { $$=&StringType; }
                |       T_ARRAY T_LBRACKET range T_RBRACKET T_OF typedefn
                                { $$=array_descriptor(&$3,$6); }
                |       T_STRUCT id
                                { $$=declare_struct($2); }
                ;


range           :       T_CONSTWORD T_RANGE T_CONSTWORD
                                {
                                  $$.lo=$1.val;
                                  $$.hi=$3.val;

                                  if ($$.hi < $$.lo ||
                                      $$.hi > 0x7fff ||
                                      $$.lo > 0x7fff)
                                  {
                                    error(MEXERR_INVALIDRANGE,
                                          $$.lo,$$.hi);

                                    $$.hi=$$.lo;
                                  }
                                }
                |       T_CONSTWORD T_RANGE
                                {
                                  $$.lo = $1.val;
                                  $$.hi = (VMADDR)-1;

                                  if ($$.lo > 0x7fff)
                                  {
                                    error(MEXERR_INVALIDRANGE,
                                          $$.lo, -1);
                                  }
                                }
                ;

id_list         :       id_list T_COMMA id
                                { $$=var_list($3,$1); }
                |       id
                                { $$=var_list($1,NULL); }
                ;


statement_list  :       /* epsilon */
                |       statement_list statement
                ;

opt_statement   :       /* epsilon */
                |       statement
                ;

statement       :       block
                                { }
                |       useful_expr T_SEMICOLON
                                { MaybeFreeTemporary($1, TRUE); }
                |       useless_expr T_SEMICOLON
                                {
                                  warn(MEXERR_WARN_MEANINGLESSEXPR);
                                  MaybeFreeTemporary($1, TRUE);
                                }
                |       T_IF paren_expr
                                { $<patch>$=IfTest($2); }
                        statement else_part
                                { IfEnd(& $<patch>3, & $5); }
                |       T_GOTO id T_SEMICOLON
                                { ProcessGoto($2); }
                |       id T_COLON
                                { DeclareLabel($1); }
                        statement
                |       T_WHILE
                                { $<whil>$.top_quad=this_quad; }
                        paren_expr
                                { WhileTest(&$<whil>2, $3); }
                        statement
                                { GenWhileOut(&$<whil>2); }
                |       T_DO
                                { $<whil>$.top_quad=this_quad; }
                        statement T_WHILE paren_expr T_SEMICOLON
                                { GenDoWhileOut(&$<whil>2, $5); }
                 /* The for() code is a bit tricky.  The
                  * quads for a standard for() look like this:
                  *
                  * ; for (init; test; post)
                  * ;   stmt;
                  *
                  *      <init code>
                  * test:
                  *      <test code>
                  *      jz  done
                  *      jmp body
                  * post:
                  *      <post code>
                  *      jmp test
                  * body:
                  *      <stmt code>
                  *      jmp post
                  * done:
                  */
                |       T_FOR T_LPAREN opt_expr T_SEMICOLON
                                { $<fr>$.vmTest = this_quad;
                                  MaybeFreeTemporary($3, TRUE);
                                }
                        opt_expr T_SEMICOLON
                                { GenForTest(&$<fr>5, $6);
                                  $<fr>5.vmPost = this_quad;
                                }
                        opt_expr
                                {
                                  GenForJmpTest(&$<fr>5);
                                  MaybeFreeTemporary($9, TRUE);
                                  $<fr>5.vmBody = this_quad;
                                }
                        T_RPAREN statement
                                {
                                  GenForJmpPostAndCleanup(&$<fr>5);
                                }
                |       T_RETURN opt_expr T_SEMICOLON
                                { GenFuncRet($2, curfn); }
                |       error T_SEMICOLON
                                { yyerrok; }
                |       T_SEMICOLON
                                { /* null statement */ }
                ;

else_part       :       /* epsilon */
                                { $$.patchout=NULL;
                                  $$.else_label=this_quad;
                                }
                |       T_ELSE 
                                { ElseHandler(&$$); }
                        statement
                                { $$=$<elsetype>2; }
                ;

function_call   :       id T_LPAREN 
                                { $<fcall>$=StartFuncCall($1); }
                        expr_list T_RPAREN
                                { $$=EndFuncCall(&$<fcall>3, $4); }
                ;

expr_list       :       /* epsilon */
                                { $$=NULL; }
                |       expr T_COMMA expr_list
                                {
                                  if (!$1)
                                  {
                                    $1 = NewDataObj();
                                    $1->type = NULL;
                                    $1->argtype = NULL;
                                  }

                                  $1->next_arg=$3;
                                  $$=$1;
                                }
                |       expr
                                {
                                  if ($1)
                                    $1->next_arg=NULL;
                                  else
                                  {
                                    $1 = NewDataObj();
                                    $1->type = NULL;
                                    $1->argtype = NULL;
                                  }

                                  $$=$1;
                                }
                ;

primary         :       paren_expr
                                { $$=$1; }
                |       T_LPAREN typedefn T_RPAREN primary
                                { $$=TypeCast($4, $2); }
                |       T_SIZEOF T_LPAREN typedefn T_RPAREN
                                { $$=EvalSizeof($3); }
                |       function_call
                                { $$=$1; }
                |       literal
                                { $$=$1; }
                |       ident
                                { $$=$1; }
                ;


opt_expr        :       /* epsilon */
                                { $$=NULL; }
                |       expr
                                { $$=$1; }
                ;


paren_expr      :       T_LPAREN expr T_RPAREN
                                { $$=$2; }
                ;

expr            :       useful_expr
                                {$$ = $1; }
                |       useless_expr
                                {$$ = $1; }
                ;

useless_expr    :       expr T_BMULTIPLY expr
                                { $$=EvalBinary($1,T_BMULTIPLY,$3); }
                |       expr T_BDIVIDE expr
                                { $$=EvalBinary($1,T_BDIVIDE,$3); }
                |       expr T_BMODULUS expr
                                { $$=EvalBinary($1,T_BMODULUS,$3); }
                |       expr T_BPLUS expr
                                { $$=EvalBinary($1,T_BPLUS,$3); }
                |       expr T_MINUS expr
                                { $$=EvalBinary($1,T_MINUS,$3); }
                |       expr T_LE expr
                                { $$=EvalBinary($1,T_LE,$3); }
                |       expr T_LT expr
                                { $$=EvalBinary($1,T_LT,$3); }
                |       expr T_SHR expr
                                { $$=EvalBinary($1,T_SHR,$3); }
                |       expr T_SHL expr
                                { $$=EvalBinary($1,T_SHL,$3); }
                |       expr T_BAND expr
                                { $$=EvalBinary($1,T_BAND,$3); }
                |       expr T_BOR expr
                                { $$=EvalBinary($1,T_BOR,$3); }
                |       expr T_LAND expr
                                { $$=EvalBinary($1,T_LAND,$3); }
                |       expr T_LOR expr
                                { $$=EvalBinary($1,T_LOR,$3); }
                |       expr T_EQUAL expr
                                { $$=EvalBinary($1,T_EQUAL,$3); }
                |       expr T_NOTEQUAL expr
                                { $$=EvalBinary($1,T_NOTEQUAL,$3); }
                |       expr T_GE expr
                                { $$=EvalBinary($1,T_GE,$3); }
                |       expr T_GT expr
                                { $$=EvalBinary($1,T_GT,$3); }
                |       T_MINUS primary   %prec T_BMULTIPLY
                                { $$=EvalUnary($2,T_MINUS); }
                ;


useful_expr     :       lval_ident T_ASSIGN expr
                                {
                                  /* The binary operator expects
                                   * assignments to be given
                                   * in the order "src -> dest";
                                   * hence the $3 $1 ordering.
                                   */
                                  $$=EvalBinary($3,T_ASSIGN,$1);
                                }
                |       primary
                                { $$=$1; }
                ;

const_byte_p	:	T_CONSTBYTE
                                { $$ = &$1; }
                ;

const_word_p	:	T_CONSTWORD
                                { $$ = &$1; }
                ;

const_dword_p	:	T_CONSTDWORD
                                { $$ = &$1; }
                ;

const_string_p	:	const_string
                                { $$ = &$1; }
                ;

literal         :       const_byte_p
                                { $$=byteref($1); }
                |       const_word_p
                                { $$=wordref($1); }
                |       const_dword_p
                                { $$=dwordref($1); }
                |       const_string_p
                                { $$=stringref($1); }
                ;


const_string    :       T_CONSTSTRING
                                { $$=$1; }
                |       const_string T_CONSTSTRING
                                { $$=string_merge($1, $2); }

ident           :       T_ID
                                { $$=idref($1); }
                |       ident T_LBRACKET expr T_RBRACKET
                                { $$=ProcessIndex($1, $3, FALSE); }
                |       ident T_DOT id
                                { $$=ProcessStruct($1, $3); }
                ;

lval_ident      :       T_ID
                                { $$=idref($1); }
                |       ident T_LBRACKET expr T_RBRACKET
                                { $$=ProcessIndex($1, $3, TRUE); }
                |       ident T_DOT id
                                { $$=ProcessStruct($1, $3); }
                ;

id              :       T_ID
                                { $$=$1; }
                ;

%%



