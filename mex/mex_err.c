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
static char rcs_id[]="$Id: mex_err.c,v 1.3 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "prog.h"
#include "mex.h"

extern int yychar;

struct _mexerr
{
  int errnum;
  char *szMsg;
};

static struct _mexerr errtab[] =
{
  {MEXERR_SYNTAX,                   "syntax error near '%s'"},
  {MEXERR_FATAL_NOMEM,              "ran out of memory"},
  {MEXERR_TOOMANYNESTEDINCL,        "too many nested include files"},
  {MEXERR_CANTOPENREAD,             "can't open file '%s' for read"},
  {MEXERR_UNTERMCHARCONST,          "unterminated character constant"},
  {MEXERR_INVALCHARCONST,           "invalid character constant"},
  {MEXERR_INVALIDHEX,               "invalid hex escape sequence \"\\x%s\""},
  {MEXERR_UNTERMSTRINGCONST,        "unterminated string constant"},
  {MEXERR_INVALIDINCLUDEDIR,        "invalid #include directive"},
  {MEXERR_INVALIDINCLUDEFILE,       "invalid #include filename"},
  {MEXERR_INVALIDDEFINE,            "invalid #define"},
  {MEXERR_MACROALREADYDEFINED,      "macro '%s' is already defined and is not identical"},
  {MEXERR_INVALIDIFDEF,             "invalid #ifdef/#ifndef"},
  {MEXERR_TOOMANYNESTEDIFDEF,       "too many nested #ifdef/#ifndef blocks"},
  {MEXERR_UNMATCHEDENDIF,           "unmatched #endif directive"},
  {MEXERR_UNMATCHEDELSE,            "unmatched #else directive"},
  {MEXERR_UNMATCHEDELIFDEF,         "unmatched #elifdef/#elseifdef directive"},
  {MEXERR_ERRORDIRECTIVE,           "#error directive: '%s'"},
  {MEXERR_UNKNOWNDIRECTIVE,         "unknown directive '%s'"},
  {MEXERR_INVALIDCHAR,              "invalid character: %c (0x%02x)"},
  {MEXERR_UNMATCHEDIFDEF,           "unmatched #ifdef"},
  {MEXERR_REDECLARATIONOF,          "redeclaration of '%s'"},
  {MEXERR_TYPEMUSTNOTBEVOID,        "type must not be 'void'"},
  {MEXERR_REDECLOFSTRUCT,           "redeclaration of structure '%s'"},
  {MEXERR_UNDEFSTRUCTNAME,          "undefined structure name: '%s'"},
  {MEXERR_SYMISNOTSTRUCT,           "symbol '%s' is not a structure type"},
  {MEXERR_INVALIDTYPEFORSTMT,       "invalid type (%s) for '%s' statement"},
  {MEXERR_REDECLOFLABEL,            "redeclaration of label '%s'"},
  {MEXERR_NOTAGOTOLABEL,            "'%s' not a goto label"},
  {MEXERR_UNDEFLABEL,               "undefined label '%s'"},
  {MEXERR_UNDEFVARIABLE,            "undefined variable '%s'"},
  {MEXERR_CANTUSEFUNCASVAR,         "can't use function as a variable"},
  {MEXERR_STRUCTMUSTBEDECL,         "struct '%s' must be declared before use"},
  {MEXERR_LVALUEREQUIRED,           "lvalue required"},
  {MEXERR_CANNOTAPPLYUNARY,         "cannot apply unary minus to '%s'"},
  {MEXERR_VOIDCANTHAVEVALUE,        "the type 'void' cannot have a value"},
  {MEXERR_INVALIDTYPECONV,          "invalid type conversion: '%s' -> '%s'"},
  {MEXERR_VARMUSTBEARRAY,           "'%s' must be an array"},
  {MEXERR_CANTINDEXNONARRAY,        "can't use [] on a non-array"},
  {MEXERR_INVALIDIDXTYPE,           "invalid index type for array '%s'"},
  {MEXERR_OUTOFRANGESUBSCRIPT,      "out-of-range subscript (%d) for array '%s'"},
  {MEXERR_DOTOPERATORFORSTRUCTS,    "dot operator can only be used with structures"},
  {MEXERR_FIELDNOTSTRUCTMEMBER,     "'%s' is not a member of struct %s"},
  {MEXERR_REDECLOFFUNC,             "invalid redeclaration of function %s"},
  {MEXERR_REDECLOFARG,              "redeclaration of argument '%s'"},
  {MEXERR_ARGMISMATCH,              "argument mismatch: function='%s%s', prototype='%s%s'"},
  {MEXERR_TOOMANYARGSINDECL,        "too many arguments in function declaration"},
  {MEXERR_TOOFEWARGSINDECL,         "too few arguments in function declaration"},
  {MEXERR_REDECLOFFUNCBODY,         "redeclaration of function body for %s"},
  {MEXERR_CALLTOFUNCWITHNOPROTO,    "call to '%s' with no prototype"},
  {MEXERR_VARIABLENOTFUNCTION,      "variable '%s' is not a function"},
  {MEXERR_LVALUEREQDARG,            "lvalue required (arg %d of %s())"},
  {MEXERR_TOOFEWARGSINCALL,         "not enough arguments in call to '%s'"},
  {MEXERR_TOOMANYARGSINCALL,        "too many arguments in call to '%s'"},
  {MEXERR_WARN_CONSTANTTRUNCATED,   "constant truncated: %s"},
  {MEXERR_WARN_IDENTTRUNCATED,      "identifier truncated: %s"},
  {MEXERR_INVALIDRANGE,             "invalid range: '%d..%d'"},
  {MEXERR_CANTRETURNVOID,           "void function cannot return a value"},
  {MEXERR_FUNCMUSTRETURNVALUE,      "non-void function must return a value"},
  {MEXERR_INVALIDIDENTTYPE,         "invalid identifier type"},
  {MEXERR_WARN_MEANINGLESSEXPR,     "meaningless use of an expression"},
  {MEXERR_ARRAYMUSTHAVEBOUNDS,      "declared array must have upper bound"},
  {MEXERR_SIZEOFBOUNDLESSARRAY,     "cannot apply sizeof() to a boundless array"},
  {MEXERR_CANTASSIGNSTRUCT,         "cannot assign one structure to another"},
  {0,                               NULL}
};

static char *err_string(int errnum)
{
  struct _mexerr *pme;

  for (pme=errtab; pme->errnum; pme++)
    if (pme->errnum==errnum)
      return pme->szMsg;

  return "Unknown error";
}

static void report_err(int errnum, char *type, char *what)
{
  fprintf(stdout,
          "%s(%ld) : %s %d: %s\n",
          filename,
          linenum,
          type,
          errnum,
          what);
}



void _stdc warn(int errnum, ...)
{
  va_list var_args;
  char string[MAX_ERRMSG_LEN];

  /* Convert the variable-argument format into a string */
  
  va_start(var_args, errnum);
  vsprintf(string, err_string(errnum), var_args);
  va_end(var_args);

  n_warnings++;
  report_err(errnum, "warning", string);
}

void _stdc error(int errnum, ...)
{
  va_list var_args;
  char string[MAX_ERRMSG_LEN];

  /* Convert the variable-argument format into a string */
  
  va_start(var_args, errnum);
  vsprintf(string, err_string(errnum), var_args);
  va_end(var_args);

  n_errors++;
  report_err(errnum, "error", string);
}

void _stdc fatal_error(int errnum, ...)
{
  va_list var_args;
  char string[MAX_ERRMSG_LEN];

  /* Convert the variable-argument format into a string */
  
  va_start(var_args, errnum);
  vsprintf(string, err_string(errnum), var_args);
  va_end(var_args);

  n_errors++;
  report_err(errnum, "fatal error",string);
  exit(1);
}

void _stdc bug(char *s,...)
{
  va_list var_args;
  char string[MAX_ERRMSG_LEN];

  /* Convert the variable-argument format into a string */
  
  va_start(var_args,s);
  vsprintf(string, s, var_args);
  va_end(var_args);

  n_errors++;
  report_err(MEXERR_FATAL_INTERNAL, "fatal bug", string);
  exit(1);
}


void _stdc debug(char *s,...)
{
  va_list var_args;
  char string[MAX_ERRMSG_LEN];
  extern int sdebug;

  if (!sdebug)
    return;

  /* Convert the variable-argument format into a string */

  va_start(var_args,s);
  vsprintf(string,s,var_args);
  va_end(var_args);

  report_err(MEXERR_DEBUG, "debug", string);
}


void yyerror(char *s)
{
  char *where=NULL;
  struct _id *i;

  NW(s);

  /* Search through the list of reserved words, and translate token to      *
   * its string representation, if necessary.                               */

  for (i=idlist; i->name; i++)
    if (i->value==yychar)
    {
      where=i->name;
      break;
    }

  /* No go, so check to see if it's an operator */

  if (i->name==NULL)
    for (i=ops; i->name; i++)
      if (i->value==yychar)
      {
        where=i->name;
        break;
      }

  /* Finally, if it isn't an op or a reserved work, check to see if it's    *
   * an identifier.                                                         */

  if (i->name==NULL)
    where=yytext;

  error(MEXERR_SYNTAX, where ? where : "(null)");
}

void _fast NoMem(void)
{
  fatal_error(MEXERR_FATAL_NOMEM);
}


