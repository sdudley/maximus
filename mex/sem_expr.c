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

/* $Id: sem_expr.c,v 1.2 2004/01/22 08:04:27 wmcbrine Exp $ */

/*# name=Semantic routines for processing expressions
*/

#include <string.h>
#include "alc.h"
#include "prog.h"
#include "mex.h"

TLLIST *temp;

static QUADOP near SelectBinaryOperator(DATAOBJ **o1,word tok_op,DATAOBJ **o2,TYPEDESC **result);

/* This routine produces a data object record for the specified identifier,
 * by retrieving its information out of the symbol table.
 */

DATAOBJ * idref(IDTYPE *id)
{
  ATTRIBUTES *at;
  DATAOBJ *dobj=NewDataObj();
  
  if ((at=st_find(symtab, id, FALSE))==NULL)
  {
    error(MEXERR_UNDEFVARIABLE, id);

    at=st_enter(symtab, id, NULL);
    
    if (at==NULL)
      bug("idref: can't put name in table");

    at->class=ClassError;
  }

  if (at->class==ClassError)
    return NULL;

  if (at->class==ClassFunc)
  {
    error(MEXERR_CANTUSEFUNCASVAR);
    return NULL;
  }

  if (at->class==ClassStructName)
  {
    error(MEXERR_STRUCTMUSTBEDECL, at->name);
    return NULL;
  }

  #ifdef EXPRDEBUG
  debug("Expr: Identifier '%s'",at->name);
  #endif
  
  dobj->type=at->typedesc;
  
  if (at->class != ClassVariable && at->class != ClassArg)
    error(MEXERR_INVALIDIDENTTYPE);
  else
  {
    dobj->objform=ObjformAddress;
    dobj->form.addr=at->c.addr;

    /* Make a copy of the name too.  Only really used for global
     * variables (and their run-time binding), but handy for displaying
     * names of variables for ascii quad output.
     */

    if ((dobj->name=strdup(id))==NULL)
      NoMem();
  }
  
  return dobj;
}




/* Produce a DataObject struct corresponding to a BYTE with the constant    *
 * value 'ct'.                                                              */
   
DATAOBJ * byteref(CONSTTYPE *ct)
{
  DATAOBJ *dobj=NewDataObj();
  
  dobj->type=&ByteType;
  dobj->objform=ObjformValue;
  dobj->form.val.valkind=ByteKind;
  dobj->form.val.kind.byteval=(byte)ct->val;

  if ((byte)ct->val >= (byte)128)
    dobj->type= &UnsignedByteType;

  #ifdef EXPRDEBUG
  debug("Expr: ByteLiteral '%d'",ct->val);
  #endif

  return dobj;
}


/* Produce a DataObject struct corresponding to a WORD with the constant    *
 * value 'ct'.                                                              */

DATAOBJ * wordref(CONSTTYPE *ct)
{
  DATAOBJ *dobj=NewDataObj();
  
  dobj->type=&WordType;
  dobj->objform=ObjformValue;
  dobj->form.val.valkind=WordKind;
  dobj->form.val.kind.wordval=ct->val;

  if ((word)ct->val >= (word)32768u)
    dobj->type = &UnsignedWordType;

  #ifdef EXPRDEBUG
  debug("Expr: WordLiteral '%d'",ct->val);
  #endif

  return dobj;
}


/* Produce a DataObject struct corresponding to a DWORD with the constant   *
 * value 'ct'.                                                              */

DATAOBJ * dwordref(CONSTTYPE *ct)
{
  DATAOBJ *dobj=NewDataObj();
  
  dobj->type=&DwordType;
  dobj->objform=ObjformValue;
  dobj->form.val.valkind=DwordKind;
  dobj->form.val.kind.dwordval=ct->dwval;

  if ((dword)ct->val >= (dword)2147483648Lu)
    dobj->type = &UnsignedDwordType;

  #ifdef EXPRDEBUG
  debug("Expr: DwordLiteral '%ld'", ct->dwval);
  #endif

  return dobj;
}


/* Function for merging two string literals */

CONSTTYPE string_merge(CONSTTYPE c1, CONSTTYPE c2)
{
  int len=strlen(c1.lit) + strlen(c2.lit);
  CONSTTYPE ct;

  ct.lit=smalloc(len+1);
  strcpy(ct.lit, c1.lit);
  strcat(ct.lit, c2.lit);

  free(c1.lit);
  free(c2.lit);

  return ct;
}


/* Produce a DataObject struct corresponding to a STRING with the constant  *
 * value 'ct'.                                                              */

DATAOBJ *stringref(CONSTTYPE *ct)
{
  struct _conval *cv;
  DATAOBJ *dobj=NewDataObj();
  byte *s=ct->lit;

  dobj->type=&StringType;
  dobj->objform=ObjformValue;
  dobj->form.val.valkind=StringKind;
  dobj->form.val.kind.str.segment=SEG_GLOBAL;
  dobj->form.val.kind.str.indirect=FALSE;

  cv=AddGlobalConst(strlen(s), s, dobj->type);

  dobj->form.val.kind.str.offset=cv->offset;

  dobj->init=cv;

  #ifdef EXPRDEBUG
  debug("Expr: StringLiteral:  '%s'", ct->lit);
  #endif

  free(ct->lit);
  return dobj;
}





/* Get a temporary register, suitable for holding an object of type 'td' */

ADDRESS GetTemporary(TYPEDESC *td)
{
  TLLIST *t;
  TLIST *tl, *last, *new;
  word reg;
  
  if ((new=malloc(sizeof(TLIST)))==NULL)
    NoMem();

  /* Set up the new temporary with some defaults */
  
  new->addr.segment=SEG_TEMP;
  new->addr.offset=1+(td->size*MAX_TEMP);
  new->addr.indirect=FALSE;
  new->addr.typedesc=td;
  new->next=NULL;

  /* Search the linked list of linked lists, and find the one containing    *
   * the register size we want.                                             */

  for (t=temp; t; t=t->next)
  {
    /* If this is the linked list containing register elements of this size */

    if (t->size==td->size)
    {
      /* Now scan this list, to find a gap in the registers allocated */
      
      for (reg=1, last=NULL, tl=t->tlist; 
           tl && tl->addr.offset==reg+(td->size*MAX_TEMP);
           last=tl, tl=tl->next, reg++)
        ;
      
      /* If we got this far, we're either at the end of the list, or in     * 
       * the middle of the list, pointing to a gap.  In either case,        *
       * we should insert the temporary right here.                         */

      /* Update the 'next' pointer of the last element */
      
      if (last)
        last->next=new;
      else t->tlist=new;

      new->addr.offset=reg+(td->size*MAX_TEMP);
      new->next=tl;
      
      #ifdef DECLDEBUG
      debug("Allocated temporary at Temp:%d",new->addr.offset);
      #endif
        
      return new->addr;
    }
  }

  /* If we got this far, there was no preexisting linked list containing    *
   * the registers for the type we're looking for, so we create a new one.  */
  
  if ((t=malloc(sizeof(TLLIST)))==NULL)
    NoMem();
  
  t->next=temp;
  temp=t;
  
  t->size=td->size;
  t->tlist=new;

  
  #ifdef DECLDEBUG
  debug("Allocated temporary at Temp:%d",new->addr.offset);
  #endif

  return new->addr;
}


/* If the object 'o' is a temporary register, this routine will indicate
 * that the object can be freed as a temporary.
 */

int WouldWeFreeTemporary(DATAOBJ *o, TLLIST **pptlT,
                         TLIST **pptlTl, TLIST **pptlLast)
{
  TLLIST *t;
  TLIST *tl, *last, *next;

  if (o==NULL)
    return FALSE;

  /* Don't do anything unless the object is a temporary */

  if (o->objform != ObjformAddress || o->form.addr.segment != SEG_TEMP)
    return FALSE;

  /* Scan the linked list of linked lists */
  
  for (t=temp; t; t=t->next)
  {
    for (tl=t->tlist, last=NULL; tl; tl=next)
    {
      next=tl->next;

      if (tl->addr.segment==o->form.addr.segment &&
          tl->addr.offset==o->form.addr.offset)
      {
        if (pptlT)
          *pptlT = t;

        if (pptlTl)
          *pptlTl = tl;

        if (pptlLast)
          *pptlLast = last;

        return TRUE;
      }
      else
      {
        last=tl;
      }
    }
  }

  return FALSE;
}




/* If the object 'o' is a temporary register, this routine will try to      *
 * free the register and make it available for use again.                   */

void MaybeFreeTemporary(DATAOBJ *o, word free_str)
{
  TLLIST *t;
  TLIST *tl;
  TLIST *last;

  if (WouldWeFreeTemporary(o, &t, &tl, &last))
  {
    /* can't use AddrEqual() since indirect may have been set */

    /* Aha!  We found the reg that we're looking for.  Update the
     * 'next' pointer of the prior element to remove this reg.
     */

    if (last)
      last->next=tl->next;
    else
      t->tlist=tl->next;

    /* If the temporary register holds a string, try to free it too.
     * If free_str==2, only free the string if the indirect flag
     * was NOT set.  Otherwise, free it.  This is to allow
     * strings in structs to work properly.
     */

    if (o->type==&StringType && free_str)
      if (!o->form.addr.indirect || o->form.addr.segment != SEG_TEMP)
        Generate(QOP_SKILL, o, NULL, NULL);

    #ifdef EXPRDEBUG
    debug("Freed temporary %d",tl->addr.offset);
    #endif

    free(tl);
  }
}


/* The sizeof() operator.  Simply return a value indicating the size
 * of the specified type.
 */

DATAOBJ *EvalSizeof(TYPEDESC *t)
{
  DATAOBJ *po = NewDataObj();

  if (t && t->form==FormArray &&
      t->typeinfo.array.bounds.hi==(VMADDR)-1)
  {
    error(MEXERR_SIZEOFBOUNDLESSARRAY);
    return NULL;
  }

  po->type=&WordType;
  po->objform=ObjformValue;
  po->form.val.valkind = WordKind;
  po->form.val.kind.wordval = t ? t->size : 1;

  return po;
}




/* Evalute a binary operator, and return a DataObj structure pointing
 * to the location of the result.
 */

DATAOBJ * EvalBinary(DATAOBJ *o1,word operator,DATAOBJ *o2)
{
  DATAOBJ *dobj;
  TYPEDESC *result;
  QUADOP top;
  ADDRESS t;

  if (!o1 || !o2)
    return NULL;

  /* Make sure that we don't do an invalid assignment of a void type */

  if (o1->type==&VoidType || o2->type==&VoidType)
  {
    if (o1->type != o2->type ||
        operator != T_ASSIGN ||
        o1->objform != ObjformAddress || !o1->form.addr.indirect ||
        o2->objform != ObjformAddress || !o2->form.addr.indirect)
    {
      error(MEXERR_TYPEMUSTNOTBEVOID);
      top = QOP_NOP;
    }
    else
    {
      /* We can assign a ref void to a ref void */

      top = QOP_ASSIGN;
    }
  }
  else
  {
    /* Figure out which quad operator we'll need to evaluate the expression */

    top=SelectBinaryOperator(&o1, operator, &o2, &result);
  }

  if (top==QOP_NOP)
    return NULL;

  /* Get a temporary variable to hold the result in. */

  if (top==QOP_ASSIGN || top==QOP_SCOPY)
  {
    if (o2->objform==ObjformValue)
    {
      error(MEXERR_LVALUEREQUIRED);
      return NULL;
    }

    if (o2->type && o2->type->form==FormStruct)
    {
      error(MEXERR_CANTASSIGNSTRUCT);
      return NULL;
    }

    memset(&t, '\0', sizeof(t));
  }
  else t=GetTemporary(result);


  /* Generate the actual quad to evaluate the binary expression */

  Generate(top,o1,o2,&t);

  /* Free the temporaries used by the objects of this expression, if
   * possible.
   */

  MaybeFreeTemporary(o1, TRUE);
  MaybeFreeTemporary(o2, TRUE);
  
  /* Now create a data object to tell the compiler where to find the result */

  if (top==QOP_ASSIGN || top==QOP_SCOPY)
    dobj = o2;
  else
  {
    dobj=NewDataObj();

    dobj->type=result;
    dobj->objform=ObjformAddress;
    dobj->form.addr=t;
  }

  return dobj;
}


/* Evaluate a unary operator.  Currently, the only supported unary op       *
 * is the unary minus ("-1234", for example).                               */

DATAOBJ * EvalUnary(DATAOBJ *o, word operator)
{
  DATAOBJ *res, zero;
  TYPEDESC *typ;
  ADDRESS t;

  char name[MAX_TN_LEN];

  if (o==NULL)
    return NULL;

  if (o->type==&VoidType)
  {
    error(MEXERR_TYPEMUSTNOTBEVOID);
    return NULL;
  }
  
  if (operator != T_MINUS)
    bug("Invalid operator for EvalUnary");

  typ=o->type;
  
  if (typ->form != FormByte && typ->form != FormWord &&
      typ->form != FormDword)
  {
    error(MEXERR_CANNOTAPPLYUNARY, TypeName(typ, name));
    return NULL;
  }

  res=NewDataObj();

  /* If it's a literal value, do the conversion here without generating     *
   * any code.                                                              */

  if (o->objform==ObjformValue)
  {
    *res=*o;
    res->name=NULL;
    
    switch (o->form.val.valkind)
    {
      case ByteKind:
        res->form.val.kind.byteval=(byte)-(sbyte)o->form.val.kind.byteval;
        res->type = &ByteType;    /* ensure that result is signed */
        break;

      case WordKind:
        res->form.val.kind.wordval=(word)-(sword)o->form.val.kind.wordval;
        res->type = &WordType;    /* ensure that result is signed */
        break;

      case DwordKind:
        res->form.val.kind.dwordval=(dword)-(sdword)o->form.val.kind.dwordval;
        res->type = &DwordType;    /* ensure that result is signed */
        break;
    }
    
    return res;
  }


  /* Else it's not a literal, so generate the appropriate code.
   * First, grab a temporary to store the new value in.
   */

  t=GetTemporary(o->type);

  res->objform=ObjformAddress;
  res->form.addr=t;

  /* Ensure that the resulting type is signed */

  if (o->type==&ByteType || o->type==&UnsignedByteType)
    res->type = &ByteType;
  else if (o->type==&WordType || o->type==&UnsignedWordType)
    res->type = &WordType;
  else if (o->type==&DwordType || o->type==&UnsignedDwordType)
    res->type = &DwordType;
  else
    bug("In EvalUnary: invalid object type");

  /* Sneaky way of processing "-ident".  Just generate code for
   * "0-ident", which has the same effect, and requires less code in our
   * VM.
   */

  zero.name=NULL;
  zero.type=o->type;
  zero.objform=ObjformValue;
  zero.form.val.valkind=o->form.val.valkind;
  
  /* Set the value to zero */
  
  memset(&zero.form.val.kind, '\0', sizeof(zero.form.val.kind));


  /* Generate the actual quad to evaluate the binary expression */

  Generate(QOP_SUBTRACT, &zero, o, &t);


  /* If 'o' represents a temporary, free it. */

  MaybeFreeTemporary(o, TRUE);


  /* Return the address of the result */
  
  return res;
}


/* This function selects a quad operator to use (based on the token
 * operator 'operator') for evaluating the given two operands.
 */

static QUADOP near SelectBinaryOperator(DATAOBJ **op1,word tok_op,DATAOBJ **op2,TYPEDESC **result)
{
  DATAOBJ *o1, *o2;
  struct _optable *ot;
  QUADOP ret;

  o1=*op1;
  o2=*op2;

  /* If either element is NULL, then there must have been an error */

  if (o1==NULL || o2==NULL)
    return QOP_NOP;
  
  /* Now make sure that each type is structurally equivalent.  If not, we   *
   * may need to do some coercion, or even generate an error message.       */

  if (! StructEquiv(o1->type, o2->type))
    if (! CoerceTypes(op1,tok_op,op2))
      return QOP_NOP;
  
  /* Scan the token table to find the appropriate quad operator */
  
  for (ot=optable; ot->op; ot++)
  {
    if (ot->token==tok_op)
    {
      *result=(*op1)->type;

      /* If one or more of our operands are signed, then the result
       * is signed too.
       */

      if ((*op1)->type->fSigned || (*op2)->type->fSigned)
      {
        if ((*result)->form == FormByte)
          *result = &ByteType;

        if ((*result)->form == FormWord)
          *result = &WordType;

        if ((*result)->form == FormDword)
          *result = &DwordType;
      }

      
      ret=((*result==&StringType) ? ot->strop : ot->op);
      
      /* A logical operation always results in an integer result */

      if (ret==QOP_LE  || ret==QOP_LT  || ret==QOP_EQ   || ret==QOP_NE  ||
          ret==QOP_GE  || ret==QOP_GT  || ret==QOP_LAND || ret==QOP_LOR ||
          ret==QOP_SLE || ret==QOP_SLT || ret==QOP_SEQ  || ret==QOP_SNE ||
          ret==QOP_SGE || ret==QOP_SGE || ret==QOP_SGT)
      {
        *result=&WordType;
      }
      
      return ret;
    }
  }

  /* Not found, so return "no operator" */

  return QOP_NOP;
}





/* Routine to determine structural equivalence of two type descriptors */

int StructEquiv(TYPEDESC *t1, TYPEDESC *t2)
{
  /* ref void's are always structurally equivalent to anything else */

  if (!t1 || !t2)
    return FALSE;

  /* A void is not equivalent to anything except itself */

  if ((t1->form==FormVoid) != (t1->form==FormVoid))
    return FALSE;

  /* If either is pointing to NULL, or if the two types have different
   * sizes, they can never be equal (exception: arrays of unknown size).
   */
  
  if (!t1 || !t2)
    return FALSE;
  
  if (t1->size != t2->size && t2->size != -1)
    return FALSE;

  /* If t1 has one of the basic types, and if t2's type is the same
   * as t1, then the two must be equal.
   */

  if ((t1->form==FormByte  || t1->form==FormWord || 
       t1->form==FormDword || t1->form==FormString) &&
       t2->form==t1->form && !!t1->fSigned==!!t2->fSigned)
  {
    return TRUE;
  }

  /* If it's an array, AND the elements type are equal, AND the bounds are
   * equal, then t1 and t2 are structurally equivalent.
   */

  if (t1->form==FormArray && t2->form==FormArray &&
      StructEquiv(t1->typeinfo.array.el_type, t2->typeinfo.array.el_type) &&
      t1->typeinfo.array.bounds.lo==t2->typeinfo.array.bounds.lo &&
      (t1->typeinfo.array.bounds.hi==t2->typeinfo.array.bounds.hi ||
       t2->typeinfo.array.bounds.hi==(VMADDR)-1L))
  {
    return TRUE;
  }


  /* Two structures are only equal if they have the same type identifiers */

  if (t1->form==FormStruct && t1==t2)
    return TRUE;

  return FALSE;
}








/* Perform an implicit type conversion, given the operator tok_op,
 * the object 'o1' on the LEFT side of the operator, and object
 * 'o2' on the right side.  Generally, this promotes the two types to
 * the largest of the two, and then calls the ConvertTypes routine to
 * perform the actual conversion.
 */
     
int CoerceTypes(DATAOBJ **o1, int tok_op, DATAOBJ **o2)
{
  DATAOBJ *fromobj, *result;
  TYPEDESC *t1, *t2, *fromtype, *totype;
  int fDoFree;
  


  /* Create a separate pointer to each object's typedescriptor, for
   * convenience.
   */
     
  t1=(*o1)->type;
  t2=(*o2)->type;


  if (t1==NULL || t2==NULL)
    return FALSE;

  /* If the two are arithmetic types, we can coerce between values as
   * necessary.  If it's not a basic type, then no coercion is possible,
   * so we don't do anything and return an error message at the end of
   * the function.
   */

  if (! (IsIntegral(t1) && IsIntegral(t2)))
  {
    bad_conversion(t1, t2);
    return FALSE;  
  }

  
  /* If we're assigning, we HAVE to convert 'o2' to the type of 'o1'.
   * But if we're doing anything except assignment, coerce the
   * expression into the type with the LARGER size.
   */

  if (tok_op==T_ASSIGN)
  {
    fromobj=*o1; fromtype=t1;
                 totype=t2;
  }
  else if (t1->size > t2->size)
  {
    /* t1 is bigger, so go FROM the type of 'o2' TO the type of 'o1' */

    fromobj=*o2; fromtype=t2;
                 totype=t1;
  }
  else if (t1->size < t2->size)
  {
    /* t2 is bigger, so go FROM the type of 'o1' TO the type of 'o2' */

    fromobj=*o1; fromtype=t1;
                 totype=t2;
  }
  else
  {
    /* both are same size, so convert the unsigned one to signed */

    if (t2->fSigned)
    {
      fromobj=*o1; fromtype = t1;
                   totype = t2;
    }
    else
    {
      fromobj=*o2; fromtype = t2;
                   totype = t1;
    }
  }

  /* Create a new data object to hold the result of the coercion. */

  result=NewDataObj();
  result->type=totype;


  /* Perform the actual type conversion */
  
  fDoFree = ConvertTypes(fromobj, fromtype, result, totype);


  /* Now check to see if the converted value was a temporary, and if so,    *
   * free it.                                                               */
     
  if (*o1==fromobj)
  {
    if (fDoFree)
      MaybeFreeTemporary(*o1, TRUE);

    *o1=result;
  }
  else
  {
    if (fDoFree)
      MaybeFreeTemporary(*o2, TRUE);

    *o2=result;
  }

  return TRUE;
}


/* Typecasting - this routine performs explicit type conversion, which is   *
 * activated by the production "(typename)variable" in the source grammar.  */


DATAOBJ *TypeCast(DATAOBJ *o, TYPEDESC *t)
{
  DATAOBJ *toobj=NewDataObj();

  if (t==&VoidType)
  {
    error(MEXERR_TYPEMUSTNOTBEVOID);
    return NULL;
  }
  
  if (!o || StructEquiv(o->type, t))
  {
    debug("Unnecessary typecast - ignored");
    return o;
  }

  if (! (IsIntegral(o->type) && IsIntegral(t)))
  {
    bad_conversion(o->type, t);
    return NULL;
  }
  
  if (ConvertTypes(o, o->type, toobj, t))
    MaybeFreeTemporary(o, TRUE);

  return toobj;
}


/* Convert the object 'fromobj' of type 'fromtype' to the object pointed
 * to by 'toobj', with a final type of 'totype'.  This will either perform
 * a run-time conversion if necessary, but it will also convert values
 * at compile-time.
 *
 * Returns TRUE if the storage associated with the source object should
 * be freed, or FALSE if otherwise (such as when converting a signed
 * integer to an unsigned integer, since that only requires a change
 * in the type descriptor).
 */

int ConvertTypes(DATAOBJ *fromobj, TYPEDESC *fromtype,
                 DATAOBJ *toobj, TYPEDESC *totype)
{
  ADDRESS t;

  #ifdef EXPRDEBUG
    byte name1[MAX_TN_LEN];
    byte name2[MAX_TN_LEN];
  #endif

  byte byteval;
  word wordval;
  dword dwordval;

  word do_static;  /* perform static, compile-time conversion */
  

  /* If we somehow ended up trying to convert a value into the same type, *
   * abort.                                                               */

  if ((fromtype->form==totype->form && !!totype->fSigned==!!fromtype->fSigned) ||
      StructEquiv(fromtype, totype))
  {
    bug("Cannot coerce value to same type");
  }


  toobj->type=totype;
  
  
  /* If we have to convert a constant, do it at compile-time.  Otherwise, *
   * allocate a temporary register for the conversion.                    */

  if (fromobj->objform==ObjformValue)
  {
    do_static=TRUE; /* doing a static coercion */

    toobj->objform=ObjformValue;


    /* Set up the form of the converted value */

    if (totype->form==FormByte)
      toobj->form.val.valkind=ByteKind;
    else if (totype->form==FormWord)
      toobj->form.val.valkind=WordKind;
    else if (totype->form==FormDword)
      toobj->form.val.valkind=DwordKind;
    else if (totype->form==FormVoid)
    {
      error(MEXERR_VOIDCANTHAVEVALUE);
      return TRUE;
    }
    else
      bug("Unknown coercion 'to' type");

    #ifdef EXPRDEBUG
      debug("Coerce literal from %s to %s",
            TypeName(fromtype, name1),
            TypeName(totype, name2));
    #endif
  }
  else 
  {
    /* Otherwise we need to coerce at runtime, so get a temporary         *
     * register to hold the value of the coerced type in.                 */

    t=GetTemporary(totype);

    toobj->objform=ObjformAddress;
    toobj->form.addr=t;

    do_static=FALSE;

    #ifdef EXPRDEBUG
      debug("Coerce object from %s to %s",
            TypeName(fromtype, name1),
            TypeName(totype, name2));
    #endif
  }

  if (fromtype->form == totype->form)
  {
    /* If we are converting byte->byte, word->word, or dword->dword,
     * it means that only a sign change occurs.  Since this needs
     * no extra storage, just create a new dataobj descriptor that
     * points to the original object.
     *
     * Return FALSE since we don't want the caller to free
     * any temporaries which may be associated with the source object.
     */

    MaybeFreeTemporary(toobj, FALSE);
    toobj->form = fromobj->form;
    toobj->name = fromobj->name ? strdup(fromobj->name) : NULL;

    return FALSE;
  }
  else switch (fromtype->form)
  {
    case FormByte:      /* Converting FROM a byte */

      /* If we're doing the conversion at compile-time, put the value     *
       * of the 'fromobj' expression in an accessible place.              */

      if (do_static)
        byteval=fromobj->form.val.kind.byteval;
      else
        byteval=0;


      switch (totype->form)
      {
        case FormByte: /* BYTE -> BYTE (sign conversion) - handled above */
          break;

        case FormWord:  /* BYTE -> WORD */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.wordval = (word)(sword)(sbyte)byteval;
            else
              toobj->form.val.kind.wordval = (word)byteval;
          }
          else
          {
            Generate(QOP_BYTE2WORD, fromobj, toobj, &t);
          }
          break;

        case FormDword: /* BYTE -> DWORD */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.dwordval = (dword)(sdword)(sbyte)byteval;
            else
              toobj->form.val.kind.dwordval = (dword)byteval;
          }
          else
          {
            Generate(QOP_BYTE2DWORD, fromobj, toobj, &t);
          }
          break;

        default:
          bug("Invalid conversion type: byte -> %d", totype->form);
      }
      break;

    case FormWord:            /* Converting FROM a word */

      if (do_static)
        wordval=fromobj->form.val.kind.wordval;
      else
        wordval=0;

      switch(totype->form)
      {
        case FormByte:        /* WORD -> BYTE */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.byteval = (byte)(sbyte)(sword)wordval;
            else
              toobj->form.val.kind.byteval = (byte)wordval;
          }
          else
          {
            Generate(QOP_WORD2BYTE, fromobj, toobj, &t);
          }
          break;

        case FormWord:        /* WORD -> WORD (sign conversion) - handled above */
          break;

        case FormDword:       /* WORD -> DWORD */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.dwordval = (dword)(sdword)(sword)wordval;
            else
              toobj->form.val.kind.dwordval = (dword)wordval;
          }
          else
          {
            Generate(QOP_WORD2DWORD, fromobj, toobj, &t);
          }
          break;

        default:
          bug("Invalid conversion type: word -> %d", totype->form);
      }
      break;

    case FormDword:           /* Converting FROM a dword */

      if (do_static)
        dwordval=fromobj->form.val.kind.dwordval;
      else
        dwordval=0;

      switch(totype->form)
      {
        case FormByte:        /* DWORD -> BYTE */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.byteval=(byte)(sbyte)(sdword)dwordval;
            else
              toobj->form.val.kind.byteval=(byte)dwordval;
          }
          else
          {
            Generate(QOP_DWORD2BYTE, fromobj, toobj, &t);
          }
          break;

        case FormWord:        /* DWORD -> WORD */
          if (do_static)
          {
            if (fromtype->fSigned)
              toobj->form.val.kind.wordval=(word)(sword)(sdword)dwordval;
            else
              toobj->form.val.kind.wordval=(word)dwordval;
          }
          else
          {
            Generate(QOP_DWORD2WORD, fromobj, toobj, &t);
          }
          break;

        case FormDword:       /* DWORD -> DWORD (sign conversion) - handled above */
          break;

        default:
          bug("Invalid conversion type: dword -> %d", totype->form);
      }
      break;

    default:
      bug("Unknown type conversion");
  }

  return TRUE;
}


void bad_conversion(TYPEDESC *t1, TYPEDESC *t2)
{
  byte name1[MAX_TN_LEN];
  byte name2[MAX_TN_LEN];

  if (t1 && t2)
    error(MEXERR_INVALIDTYPECONV,
          TypeName(t1, name1), TypeName(t2, name2));
}



/* Routine for performing a "str[xx]" reference to extract a character from *
 * a specific positon in a string.                                          */
   

DATAOBJ * ProcessStringChar(DATAOBJ *str, DATAOBJ *index, int fLvalue)
{
  DATAOBJ *ret;
  ADDRESS t;
  
  /* There are two fundamentally-separate ways of obtaining a character
   * from a string.  The first is used to obtain an lvalue for a string
   * (an indirect address) which can be assigned to.  The second way
   * is to obtain an rvalue, which simply extracts a character and puts
   * it in a temporary.
   *
   * The distinction between rvalues and lvalues must be made for the
   * following chunk of code:
   *
   *    str="asdf";
   *    str[8]=str[4];
   *
   * Since str[8] is beyond the original length of the string, the string
   * must be dynamically expanded (and hence relocated) at the time that
   * the quad is parsed.  However, if str[4] was taken as a lvalue
   * (an address), the address would still point to the unallocated
   * string area.  Hence, anything to the left of the equals sign is
   * taken as an lvalue, and everything to the right is treated as an
   * rvalue.  Due to the way the grammar is structured, most of the
   * statements besides assignments are treated as lvalues, but
   * that's only a slight inefficiency compared to the value of having
   * dynamic strings.
   */

  ret=NewDataObj();
  
  if (fLvalue)
  {
    t=GetTemporary(&AddrType);

    ret->type=&ByteType;
    ret->objform=ObjformAddress;
    ret->form.addr=t;
    
    Generate(QOP_SLVAL, str, index, &t);

    /* Point the result code to whatever the result of this operation       *
     * points to.                                                           */

    ret->form.addr.indirect=TRUE;
  }
  else
  {
    t=GetTemporary(&ByteType);

    ret->type=&ByteType;
    ret->objform=ObjformAddress;
    ret->form.addr=t;
    
    Generate(QOP_SRVAL, str, index, &t);
  }

  MaybeFreeTemporary(str, FALSE);
  MaybeFreeTemporary(index, TRUE);

  return ret;
}


/* Routine for processing an array subscript */

DATAOBJ * ProcessIndex(DATAOBJ *array, DATAOBJ *index, int fLvalue)
{
  DATAOBJ el_size, add, idx, *res;
  struct _rcheck range;
  VMADDR conpart;
  ADDRESS t, outaddr, tempidx;

 
  if (!array || !array->type || !index || !index->type)
    return NULL;

  /* This semantic routine is overloaded for both string and array
   * references.  If it's a string, call the appropriate function.
   */
     
  if (array->type->form==FormString)
    return (ProcessStringChar(array, index, fLvalue));
  
  if (array->type->form != FormArray)
  {
    if (array->name)
      error(MEXERR_VARMUSTBEARRAY, array->name);
    else
      error(MEXERR_CANTINDEXNONARRAY);

    return NULL;
  }
  
  if (index->type->form != FormByte &&
      index->type->form != FormWord &&
      index->type->form != FormDword)
  {
    error(MEXERR_INVALIDIDXTYPE, array->name);
    return NULL;
  }
  

  /* Get an index to handle a pointer to the index */

  t=GetTemporary(&WordType);

  outaddr=GetTemporary(&AddrType);
  
  /* If we're supposed to check subscript refs */

  if (check_subs && array->type->typeinfo.array.bounds.hi != (VMADDR)-1L)
  {
    range.lo=array->type->typeinfo.array.bounds.lo;
    range.hi=array->type->typeinfo.array.bounds.hi;
    range.obj=index;
  
    /* If the index isn't a constant, generate code to check at runtime */
    
    if (index->objform != ObjformValue)
      Generate(QOP_RANGE, (DATAOBJ *)&range, NULL, NULL);
    else
    {
      /* Check constant subscript at compile time */
      
      if (index->form.val.kind.wordval < range.lo ||
          index->form.val.kind.wordval > range.hi)
      {
        error(MEXERR_OUTOFRANGESUBSCRIPT,
              index->form.val.kind.wordval, array->name);

        return NULL;
      }
    }
  }
  
  el_size.name=NULL;
  el_size.type=&WordType;
  el_size.objform=ObjformValue;
  el_size.form.val.valkind=WordKind;
  el_size.form.val.kind.wordval=array->type->typeinfo.array.el_type->size;
  
  /* Now compute the ConPart */

  conpart=(array->type->typeinfo.array.el_type->size*
           array->type->typeinfo.array.bounds.lo);

  /* Find the byte offset to use */

  if (array->form.addr.indirect)
  {
    DATAOBJ idx;

    /* Get a new temporary to hold the result of the index calculation */

    tempidx=GetTemporary(&WordType);

    idx.name=NULL;
    idx.type=&WordType;
    idx.objform=ObjformAddress;
    idx.form.addr=tempidx;


    /* Now figure out how much we need to subtract */

    add.name=NULL;
    add.type=&WordType;
    add.objform=ObjformValue;
    add.form.val.valkind=WordKind;
    add.form.val.kind.wordval=conpart;

    /* Multiply by the size of each element */

    Generate(QOP_MULTIPLY, index, &el_size, &tempidx);

    /* Now subtract the offset to find the real beginning of the array. */

    Generate(QOP_SUBTRACT, &idx, &add, &t);

    MaybeFreeTemporary(index, TRUE);
    MaybeFreeTemporary(&idx, TRUE);
  }
  else
  {
    Generate(QOP_MULTIPLY, index, &el_size, &t);
  
    /* Now free the index object, if necessary */

    MaybeFreeTemporary(index, TRUE);
  }
    
  idx.name=NULL;
  idx.type=&WordType; /*array->type->typeinfo.array.el_type;*/
  idx.objform=ObjformAddress;
  idx.form.addr=t;

  res=NewDataObj();
  res->name=/*NULL*/ array->name;
  res->type=array->type->typeinfo.array.el_type;
  res->objform=ObjformAddress;
  res->form.addr=outaddr;

  /* Add the start of the array: if it's an indirect value, load that       *
   * address instead.                                                       */

  add.name=array->name;
  add.type=&AddrType;
  add.form.addr=array->form.addr;
  add.objform=ObjformAddress;

  if (array->form.addr.indirect)
  {
    add.form.addr.indirect=TRUE;
    add.form.addr.typedesc=&AddrType;
  }
  else
  {
    add.form.addr.indirect=FALSE;
    add.form.addr.typedesc=&AddrType;

    /* Subtract the constant conpart to find the start of the array */

    add.form.addr.offset -= conpart;
  }

  Generate(QOP_ADD, &idx, &add, &outaddr);

  MaybeFreeTemporary(&idx, TRUE);
  
  /* Make sure that this is marked as an indirect address */
  
  res->form.addr.indirect=TRUE;
  res->form.addr.typedesc=res->type;

  MaybeFreeTemporary(array, TRUE);

  /* And return the new data object */
  return res;
}


DATAOBJ * ProcessStruct(DATAOBJ *struc, IDTYPE *pid)
{
  DATAOBJ *res;
  SYMTAB *pst;
  ATTRIBUTES *pa;

  if (!struc)
    return NULL;

  if (!struc || !struc->type)
    return NULL;

  if (struc->type->form != FormStruct)
  {
    error(MEXERR_DOTOPERATORFORSTRUCTS);
    return NULL;
  }

  /* Create easy-to-type copy of symbol table handle */

  pst=struc->type->typeinfo.struc.pst;


  /* Try to find structure member in symbol table */

  if ((pa=st_find(pst, pid, FALSE))==NULL)
  {
    error(MEXERR_FIELDNOTSTRUCTMEMBER,
          pid, struc->type->typeinfo.struc.name);

    return NULL;
  }


  /* Get a data object to hold structure member */

  res=NewDataObj();
  res->name=struc->name;
  res->type=pa->typedesc;

  /* If the structure itself is non-indirect (we have the real address),    *
   * we can perform the structure dereferencing at compile time.            */

  if (!struc->form.addr.indirect || pa->c.addr.offset==0)
  {
    res->objform=ObjformAddress;
    res->form.addr=struc->form.addr;
    res->form.addr.offset += pa->c.addr.offset;
    res->form.addr.typedesc=res->type;

    #ifdef EXPRDEBUG
    debug("Dereference struct member '%s.%s' - at offset %d",
          struc->name ? struc->name : "??", pid, res->form.addr.offset);

    #endif
  }
  else  /* Generate runtime code to compute the address of the member */
  {
    DATAOBJ add, base;
    ADDRESS outaddr;


    /* Create object to represent base address of the structure */

    base.name=struc->name;
    base.type=&AddrType;
    base.form.addr=struc->form.addr;
    base.objform=ObjformAddress;
    base.form.addr.typedesc=&AddrType;


    /* We need to add the offset to the value of the address itself.  To    *
     * do this, create a data object which represents the constant          *
     * that we wish to add.                                                 */

    add.name=NULL;
    add.type=&WordType;
    add.objform=ObjformValue;
    add.form.val.valkind=WordKind;
    add.form.val.kind.wordval=pa->c.addr.offset;

    /* Get a temporary to hold the result of the addition, and generate the *
     * required instruction.                                                */

    outaddr=GetTemporary(&AddrType);
    Generate(QOP_ADD, &base, &add, &outaddr);

    /* Now fill out the "result" object which contains the new address. */

    res->objform=ObjformAddress;

    res->form.addr=outaddr; /* copy of the GetTemporary addres */
    res->form.addr.typedesc=res->type;
    res->form.addr.indirect=TRUE;

    MaybeFreeTemporary(struc, TRUE);
  }

  return res;
}


