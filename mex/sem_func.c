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
static char rcs_id[]="$Id: sem_func.c,v 1.4 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=Function definition/declaration semantic routines
*/

#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "mex.h"

static PATCH *fr_patch;

static char * MungeTypeName(TYPEDESC *t);

/* Called at beginning of function, right after processing argument         *
 * list.                                                                    */

ATTRIBUTES * function_begin(TYPEDESC *t, IDTYPE *id)
{
  ATTRIBUTES *f;
  byte present;

  /* Set the AR offset to zero, since we're starting a new function */

  offset=maxoffset=0;


  #ifdef FUNCDEBUG
  debug("Function %s - offset=%d",id,offset);
  #endif

  if ((f=st_enter(symtab, id, &present))==NULL)
    NoMem();

  if (present && f->class != ClassFunc)
  {
    error(MEXERR_REDECLOFFUNC, id);
    return NULL;
  }

  if (!present)
  {
    f->class=ClassFunc;
    f->typedesc=NULL;
    f->c.f.ar_size=0;
    f->c.f.param=NULL;
    f->c.f.ret_type=t;
    f->c.f.declared=FALSE;
  }

  if (! f->c.f.declared)
    f->c.f.quad=this_quad;

  return f;
}



/*

 high mem
        +-------------------------+   <--- SP before STARTCALL quad
        | arg1          (word)  6 |
 SS     +-------------------------+
grows | | arg2          (word)  4 |
from  | +-------------------------+
 top  | | return_quad   (word)  2 |
 to   | +-------------------------+
bottom| | saved bp reg  (word)  0 |
      | +-------------------------+   <--- SP (now BP) after FUNCJUMP quad
      V | local1        (word) -2 |
        +-------------------------+
        | local2        (word) -4 |
 low mem+-------------------------+   <--- SP after declaring localvars in AR
*/


void function_args(ATTRIBUTES *func, FUNCARGS *a)
{
  ATTRIBUTES *attr, *lastarg, *firstarg, *protoarg;
  char temp[PATHLEN];
  char temp2[PATHLEN];
  byte present;
  sword ar_ofs=(sword)AR_CONTROL_DATA;
  word size=0;
  int have_prototype;

  have_prototype=!!func->c.f.param;

  for (firstarg=lastarg=NULL, protoarg=func->c.f.param; a; a=a->next)
  {
    if (a->type==&VoidType && !a->ref)
    {
      error(MEXERR_TYPEMUSTNOTBEVOID);
      continue;
    }

    attr=st_enter(symtab, a->name, &present);
  
    if (present)
    {
      error(MEXERR_REDECLOFARG, a->name);
      continue;
    }

    if (!firstarg)
      firstarg=attr;

    if (lastarg)
      lastarg->next_decl=attr;
    
    lastarg=attr;

    attr->class=ClassValidate;
    attr->typedesc=a->type;

    attr->c.addr.segment=SEG_AR;
    attr->c.addr.indirect=FALSE;
    attr->c.addr.typedesc=a->type;
    
    attr->ref=a->ref;

    /* If a prototype was given, compare these arguments with what we       *
     * had before!                                                          */

    if (have_prototype)
    {
      if (protoarg)
      {
        if (!StructEquiv(attr->typedesc, protoarg->typedesc) ||
            attr->ref != protoarg->ref)
        {
          error(MEXERR_ARGMISMATCH,
                attr->ref ? "ref " : "",
                TypeName(attr->typedesc, temp),
                protoarg->ref ? "ref " : "",
                TypeName(protoarg->typedesc, temp2));
        }

        protoarg=protoarg->next_decl;
      }
      else
      {
        error(MEXERR_TOOMANYARGSINDECL);
      }
    }
 

    /* Only use pass-by-ref (indirect) addressing if the variable is NOT    *
     * an integral type.                                                    */

    attr->c.addr.offset=ar_ofs;

    if (attr->typedesc)
    {
      if (attr->ref || PassByRef(attr->typedesc))
      {
        attr->c.addr.indirect=TRUE;
        ar_ofs += sizeof(IADDR);
        size += sizeof(IADDR);
      }
      else
      {
        ar_ofs += a->type->size;
        size += a->type->size;
      }
    }


    #ifdef FUNCDEBUG
      debug("Argument %s (%s) at AR ofs=%d", 
            a->name, TypeName(a->type, temp), ar_ofs);
    #endif
      
  }

  if (have_prototype && protoarg)
    error(MEXERR_TOOFEWARGSINDECL);

  func->c.f.param=firstarg;
  func->c.f.arg_size=size;
}





void function_end(ATTRIBUTES *f, int body, VMADDR end_quad)
{
  PATCH *p, *next;
  
  if (!body)
  {
    FuncValidateArgs(f);
    debug("Prototype for %s", f->name);
  }
  else
  {
    if (f->c.f.declared)
      error(MEXERR_REDECLOFFUNCBODY, f->name);
    
    f->c.f.declared=TRUE;
    f->c.f.ar_size=maxoffset;
    
    #ifdef FUNCDEBUG
      debug("Function %s needs AR of %d bytes", f->name, maxoffset);
    #endif


    /* Backpatch all of the 'return' statements to jump to this quad */

    BackPatch(fr_patch, end_quad);
  

    /* Free the chain of patch quads */
  
    for (p=fr_patch; p; next=p->next, free(p), p=next)
      ;
  
    fr_patch=NULL;
    
    
    Generate(QOP_FUNCRET, (DATAOBJ *)f, NULL, NULL);
  }
}



/* This code handles the return of an explicit value from a function */

void GenFuncRet(DATAOBJ *o, ATTRIBUTES *f)
{
  DATAOBJ *fret, *obj;
  PATCH *p;
  
  obj=o;

  /* If we need to return a value, ensure that it's placed in the right     *
   * register.                                                              */
     
  if (!obj)
  {
    if (f->c.f.ret_type != &VoidType)
      error(MEXERR_FUNCMUSTRETURNVALUE);
  }
  else
  {
    /* Attempt to coerce the return value into the required type, if it's   *
     * not already the same.                                                */

    if (! StructEquiv(o->type, f->c.f.ret_type))
    {
      obj=NewDataObj();

      if (f->c.f.ret_type == &VoidType)
        error(MEXERR_CANTRETURNVOID);
      else
      {
        if (ConvertTypes(o, o->type, obj, f->c.f.ret_type))
          MaybeFreeTemporary(o, TRUE);
      }
    }

    fret=NewDataObj();

    fret->type=f->c.f.ret_type;
    fret->objform=ObjformAddress;

    fret->form.addr.segment=SEG_TEMP;
    fret->form.addr.offset=fret->type->size*1000;
    fret->form.addr.indirect=FALSE;

    Generate(fret->type==&StringType ? QOP_SCOPY : QOP_ASSIGN, obj, fret, NULL);
    MaybeFreeTemporary(obj, TRUE);
  }


  /* Backpatch the jump to make it jump to the end of the function */
  
  p=smalloc(sizeof(PATCH));
  p->quad=this_quad;


  /* Now generate the jump to the end of the function block */
  
  Generate(QOP_JMP, NULL, NULL, NULL);


  /* Add to the linked list of patch offsets */
  
  p->next=fr_patch;
  fr_patch=p;
}



/* Begin a function call.  Generate the quad which begins the call, and
 * get our funccall record ready for use.
 */

FUNCCALL StartFuncCall(IDTYPE *id)
{
  FUNCCALL f;
  
  memset(&f, '\0', sizeof(FUNCCALL));
  
  if ((f.func=st_find(symtab, id, FALSE))==NULL)
  {
    error(MEXERR_CALLTOFUNCWITHNOPROTO, id);
    return f;
  }
  
  if (f.func->class != ClassFunc)
  {
    error(MEXERR_VARIABLENOTFUNCTION, id);
    return f;
  }
  
  f.arg=f.func->c.f.param;

  Generate(QOP_STARTCALL, (DATAOBJ *)id, NULL, NULL);
  return f; 
}


/* Munge a function name for use with a vararg call */

static char * MungeTypeName(TYPEDESC *t)
{
  static char temp[120];
  char *p;

  TypeName(t, temp);

  for (p=temp; *p; p++)
    if (*p=='.' || *p=='*' || *p=='.' || *p=='[' || *p==']' || *p==' ')
      *p='_';

  return temp;
}


/* Generate the name of a variable-argument function */

static ATTRIBUTES * near GenVarArgName(FUNCCALL *f, DATAOBJ *this, char *name)
{
  ATTRIBUTES attr, *found;

  if (!this->type)
  {
    strcpy(name, f->func->name);
    return NULL;
  }

  /* Munge a new function name based on the type of this object */

  sprintf(name,
          "__%s%s",
          f->func->name,
          strupr(MungeTypeName(this->type)));

  attr.name=name;

  /* Make sure that this function has been declared, to be on the     *
   * safe side.                                                       */

  if ((found=st_find(symtab, name, FALSE))==NULL)
  {
#ifdef WES_HACK
    /* I don't really understand why UNIX MEX is munging
     * print(var, "hello") as __print.. this is wierd - wes
     *
     * Never mind, I read the user docs and know now.  This
     * function translates print(...) into multiple __print%s
     * statements, one for each argument, depending on its
     * type. This tells me that something is probably wrong
     * in the symbol table somewhere.
     */
    if (name[0] == '_' && name[1] == '_' && 
	(found = st_find(symtab, name + 2, FALSE)))
    {
      name += 2;
      attr.name += 2;
    }
    else
#endif
    {
      error(MEXERR_CALLTOFUNCWITHNOPROTO, name);
      return NULL;
    }
  }

  return found;
}



/* End a function call.  Push all of the arguments on the stack using the   *
 * appropriate method.                                                      */

DATAOBJ * EndFuncCall(FUNCCALL *f, DATAOBJ *args)
{
  ATTRIBUTES *paVarArg = NULL;
  DATAOBJ *this, *last, *ret, *res, *obj, *oldobj;
  int varargs, argno;


  if (f->func==NULL || f->func->name==NULL)
    return NULL;

  /* Use a variable-argument calling convention for ellipsis functions */

  varargs=(f->arg && f->arg->typedesc==&EllipsisType);


  /* If we're not dealing with an ellipsis, check the number of arguments */

  if (!varargs)
  {
    for (argno=1, this=args;
         f->arg && this;
         this=this->next_arg, f->arg=f->arg->next_decl, argno++)
    {
      this->argtype=f->arg->typedesc;
      this->ref=f->arg->ref;

      /* If it's a pass-by-reference argument, and we have a valid
       * type descriptor....
       */

      if (this->ref && this->type)
      {
        /* Spit out an error if the argument is a literal, or if it
         * is a temporary register.
         */

        if (this->objform==ObjformValue ||
            (this->objform==ObjformAddress &&
             this->form.addr.segment==SEG_TEMP &&
             !this->form.addr.indirect))
        {
          error(MEXERR_LVALUEREQDARG, argno, f->func->name);
        }
      }
    }
  
    if (f->arg)
    {
      error(MEXERR_TOOFEWARGSINCALL, f->func->name);
      return NULL;
    }
  
    if (this)
    {
      error(MEXERR_TOOMANYARGSINCALL, f->func->name);
      return NULL;
    }
  }

  GenPushQuads();

  /* Now push the args in reverse order, so that they fit with our          *
   * stack structure.                                                       */

  for (this=args, last=NULL; this;)
  {
    if (varargs ? (this != NULL) : (this->next_arg==last))
    {
      last=this;

      /* Obj is the value that we're passing to the func.  Because of type  *
       * coercion for the function call, this may not be the actual argument*
       * passed, but rather it could be a temporary.  For now, assume that  *
       * it's the argument itself.                                          */

      obj=this;


      /* If we're handling variable arguments, we need to decide whether    *
       * to pass the variable by reference or by value.  To do this,        *
       * retrieve the function's attribute record, and check the 'ref'      *
       * value of the first argument.  If this indicates that we should     *
       * pass-by-ref, set that flag in the argument we're about to          *
       * push.                                                              */

      if (varargs)
      {
        char temp[120];

        paVarArg=GenVarArgName(f, this, temp);

        if (paVarArg && paVarArg->c.f.param->ref)
          this->ref=TRUE;
      }


      /* Copy in the pass-by-reference information from the function's      *
       * declaration.                                                       */

      if (!varargs &&
          (!StructEquiv(this->type, this->argtype) && this->argtype != &VoidType))
      {
        DATAOBJ *oldthis;

        /* Make sure that we don't coerce a pass-by-ref parameter. */
           
        if (this->ref)
        {
          bad_conversion(this->type, this->argtype);
          obj = this;
        }
        else
        {
          /* Try to coerce the types for the func argument */

          oldthis=this;
          obj=oldobj=NewDataObj();
          obj->type=this->argtype;

          CoerceTypes(&this, T_ASSIGN, &obj);

          /* If it changed, then something was coerced */

          if (obj==oldobj)
          {
            obj=this;
            MaybeFreeTemporary(oldthis, TRUE);
          }
        }
      }


      /* Handle pass-by-reference normally */
      
      if (obj->type)
      {
        if (this->ref || PassByRef(obj->type))
        {
          /* The second flag is a kludge to let the VM generation routines    *
           * know whether or not the 'indirect' flag should be set.  The      *
           * vararg routine for print() needs to work with both               *
           * lvalue and rvalue strings, so this tells the VM when to          *
           * set the indirect flag.                                           */

	  DATAOBJ *dataObjPtr = (varargs && this->ref && this->type==&StringType) ? (DATAOBJ *)1 : NULL;
          Generate(QOP_ARG_REF, obj, dataObjPtr, NULL);
#if 0
          Generate(QOP_ARG_REF, obj,
                   (DATAOBJ *)(varargs && this->ref && this->type==&StringType),
                   NULL);
#endif
        }
        else
        {
          ADDRESS tstr;
          DATAOBJ *tdo=NULL;
          word is_str=FALSE;

          /* If we're passing a string by value, allocate a temp reg          *
           * to hold the proc's copy of the string.                           */

          if (this->type==&StringType /*&& this->objform==ObjformAddress*/)
          {
            tstr=GetTemporary(&AddrType);

            tdo=NewDataObj();
            tdo->type=this->type;
            tdo->objform=ObjformAddress;
            tdo->form.addr=tstr;

            Generate(QOP_SCOPY, this, tdo, NULL);

            MaybeFreeTemporary(this, TRUE);

            /* Now make sure that the call passes the COPY of the arg */

            obj=tdo;

            is_str=TRUE;
          }

          /* Now pass it by value */

          Generate(QOP_ARG_VAL, obj, NULL, NULL);

          /* If it was a string, free the temp reg we used */

          if (is_str)
            MaybeFreeTemporary(tdo, FALSE);
        }
      }
           
       

      /* If this was a temporary register, free it. */

      MaybeFreeTemporary(this, TRUE);

/************************************************************************/
      if (varargs)
      {
        Generate(QOP_FUNCJUMP, (DATAOBJ *)paVarArg, NULL, NULL);
        GenPopQuads();

        if (this->next_arg)
        {
          Generate(QOP_STARTCALL, (DATAOBJ *)f->func->name, NULL, NULL);
          GenPushQuads();
        }
      }
/************************************************************************/
      this=varargs ? this->next_arg : args;


      continue;
    }

    /* We only get here when !varargs (which is when walking the            *
     * parameter list in reverse order).                                    */

    if (this==last)
      break;

    this=this->next_arg;
  }

  /* Generate the final call, as long as it wasn't already taken care       *
   * of by the vararg stuff, earlier above.                                 */

  if (!varargs)
    Generate(QOP_FUNCJUMP, (DATAOBJ *)f->func, NULL, NULL);

  GenPopQuads();

  /* Generate an object to hold the return value of the function call,      *
   * using the register that the caller placed the value in.                */
  
  ret=NewDataObj();
  ret->type=f->func->c.f.ret_type;

  if (f->func->c.f.ret_type == &VoidType)
  {
    ret->objform=ObjformAddress;
    ret->form.addr.typedesc=&VoidType;
    return ret;
  }
  else
  {
    /* If this function isn't one that returns 'void', then it's okay to      *
     * create a temporary to hold the return value.                           */

    ret->objform=ObjformAddress;
    ret->form.addr.segment=SEG_TEMP;
    ret->form.addr.offset=f->func->c.f.ret_type->size*1000;
    ret->form.addr.indirect=FALSE;
    ret->form.addr.typedesc=f->func->c.f.ret_type;


    /* However, since this temporary is used by ALL function returns, we    *
     * must move the value out of the x000 register as soon as possible.    */

    res=NewDataObj();
    res->type=ret->type;
    res->objform=ObjformAddress;
    res->form.addr=GetTemporary(res->type);

    Generate(QOP_ASSIGN, ret, res, NULL);

    /* It's now okay to return the value of the new register, since it's    *
     * safe from any overwrites.                                            */

    return res;
  }
}



/* Validate our argument list to check for unvalidated arguments */

void FuncValidateArgs(ATTRIBUTES *f)
{
  ATTRIBUTES *a;
  
  for (a=f->c.f.param; a; a=a->next_decl)
    if (a->class != ClassValidate)
      bug("Unvalidated parameter");
    else a->class=ClassArg;
}

/* Generate a function start tupe, plus check for unval'd args */

void GenFuncStartQuad(ATTRIBUTES *f)
{
  FuncValidateArgs(f);
  Generate(QOP_FUNCSTART, (DATAOBJ *)f, NULL, NULL);
}


struct _poplist
{
  ADDRESS a;
  struct _poplist *next;
};


static struct _poplist *poplist=NULL;

/* Generate code to push all of the temporary registers that are            *
 * currently in use.                                                        */

void GenPushQuads(void)
{
  extern TLLIST *temp;
  struct _poplist *pl;
  TLLIST *t;
  TLIST *tl;
  
  for (t=temp; t; t=t->next)
    for (tl=t->tlist; tl; tl=tl->next)
    {
      Generate(QOP_PUSH, (DATAOBJ *)&tl->addr, NULL, NULL);

      pl=smalloc(sizeof(struct _poplist));

      /* Add this to the linked list of addresses to pop */

      pl->a=tl->addr;
      pl->next=poplist;
      poplist=pl;
    }
}

void GenPopQuads(void)
{
  struct _poplist *p, *plnext;

  for (p=poplist; p; plnext=p->next, free(p), p=plnext)
    Generate(QOP_POP, (DATAOBJ *)&p->a, NULL, NULL);

  poplist=NULL;
}

