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

/*# name=Prototypes for all global compiler routines
*/

#ifndef MEX_PROT_H_DEFINED__
#define MEX_PROT_H_DEFINED__

void yyerror(char *s);
int yyparse(void);
int yylex(void);
void _stdc fatal_error(int errnum, ...);
void _stdc error(int errnum, ...);
void _stdc warn(int errnum, ...);
void _stdc debug(char *s,...);
void _stdc bug(char *s,...);
void _fast NoMem(void);
int AddrEqual(ADDRESS *a1,ADDRESS *a2);
FUNCARGS *declare_ellipsis(void);
VMADDR declare_vars(TYPEDESC *typedesc,ATTRIBUTES *attr);
ATTRIBUTES * var_list(IDTYPE *id,ATTRIBUTES *next);
void DeclareLabel(IDTYPE *id);
TYPEDESC * define_struct_id(IDTYPE *pidStruct);
void define_struct_body(TYPEDESC *ptd);
TYPEDESC *declare_struct(IDTYPE *pidStructName);
DATAOBJ * ProcessStruct(DATAOBJ *struc, IDTYPE *pid);
DATAOBJ * idref(IDTYPE *id);
DATAOBJ * byteref(CONSTTYPE *ct);
DATAOBJ * wordref(CONSTTYPE *ct);
DATAOBJ * dwordref(CONSTTYPE *ct);
DATAOBJ *stringref(CONSTTYPE *ct);
DATAOBJ * EvalBinary(DATAOBJ *o1,word operator,DATAOBJ *o2);
DATAOBJ * EvalUnary(DATAOBJ *op,word operator);
void MaybeFreeTemporary(DATAOBJ *o, word free_str);
void ProcessGoto(IDTYPE *id);
ADDRESS GetTemporary(TYPEDESC *td);
ATTRIBUTES * function_begin(TYPEDESC *t, IDTYPE *id);
void function_end(ATTRIBUTES *f, int body, VMADDR end_quad);
void scope_open(void);
void scope_close(void);
TYPEDESC * NewTypeDescriptor(void);
DATAOBJ * NewDataObj(void);
TYPEDESC * array_descriptor(RANGE *r,TYPEDESC *el_type);
SYMTAB *st_create(unsigned len, SYMTAB *parent);
void st_destroy(SYMTAB *st);
ATTRIBUTES * st_enter(SYMTAB *st, byte *name, byte *present);
ATTRIBUTES * st_find(SYMTAB *st, byte *name, int search_parent);
VMADDR st_killscope(SYMTAB *st, VMADDR scope);
void patch_gotos(void);
void ProcessGoto(IDTYPE *id);
void DeclareLabel(IDTYPE *id);
DATAOBJ * ProcessIndex(DATAOBJ *array, DATAOBJ *index, int fLvalue);
PATCH IfTest(DATAOBJ *obj);
void IfEnd(PATCH *if_test, ELSETYPE *elses);
void Generate(QUADOP op,DATAOBJ *o1,DATAOBJ *o2,ADDRESS *r);
void GenerateTXT(QUADOP op,DATAOBJ *o1,DATAOBJ *o2,ADDRESS *r);
void GenerateVM(QUADOP op,DATAOBJ *o1,DATAOBJ *o2,ADDRESS *r);
int BackPatch(PATCH *pat, VMADDR to_where);
int BackPatchTXT(PATCH *pat, VMADDR to_where);
int BackPatchVM(PATCH *pat, VMADDR to_where);
int open_vm(char *name, long lStackSize, long lHeapSize, char *outfile);
int close_vm(void);
void WhileTest(WHILETYPE *w, DATAOBJ *obj);
void GenWhileOut(WHILETYPE *w);
void GenDoWhileOut(WHILETYPE *w, DATAOBJ *obj);
char * TypeName(TYPEDESC *t,char *s);
void function_args(ATTRIBUTES *func, FUNCARGS *a);
FUNCCALL StartFuncCall(IDTYPE *id);
DATAOBJ * expr_list(DATAOBJ *expr, DATAOBJ *next);
DATAOBJ * EndFuncCall(FUNCCALL *f, DATAOBJ *args);
int StructEquiv(TYPEDESC *t1, TYPEDESC *t2);
void GenFuncStartQuad(ATTRIBUTES *f);
struct _conval * AddGlobalConst(VMADDR len, byte *buf, TYPEDESC *type);
char *GlobGetString(VMADDR offset);
void Print(DATAOBJ *args);
int IsIntegral(TYPEDESC *t);
word PassByRef(TYPEDESC *t);
DATAOBJ *TypeCast(DATAOBJ *o, TYPEDESC *t);
void GenFuncRet(DATAOBJ *o, ATTRIBUTES *f);
int ConvertTypes(DATAOBJ *fromobj, TYPEDESC *fromtype,
                 DATAOBJ *toobj, TYPEDESC *totype);
int CoerceTypes(DATAOBJ **o1, int tok_op, DATAOBJ **o2);
struct _implist * add_one_global(char *name, VMADDR size, struct _conval *init);
void ElseHandler(ELSETYPE *e);
void bad_conversion(TYPEDESC *t1, TYPEDESC *t2);
void GenPushQuads(void);
void GenPopQuads(void);
void FuncValidateArgs(ATTRIBUTES *f);
int push_fstk(char *fname);
CONSTTYPE string_merge(CONSTTYPE c1, CONSTTYPE c2);
void GenForTest(FORTYPE *pfr, DATAOBJ *pdo);
void GenForJmpTest(FORTYPE *pfr);
void GenForJmpPostAndCleanup(FORTYPE *pfr);
DATAOBJ * EvalSizeof(TYPEDESC *t);
int WouldWeFreeTemporary(DATAOBJ *o, TLLIST **pptlT,
                         TLIST **pptlTl, TLIST **pptlLast);


#endif /* MEX_PROT_H_DEFINED__ */

