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
static char rcs_id[]="$Id: sem_gen.c,v 1.4 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "mex.h"


static char * near OpName(QUADOP op);
static char * near AddressName(ADDRESS *a,char *out);
static char * near ObjName(DATAOBJ *o,char *out);


/*
static void near hc(void)
{
  printf("hc=%d\n", _heapchk());
}
*/

void Generate(QUADOP op, DATAOBJ *o1, DATAOBJ *o2, ADDRESS *r)
{
  if (vm_output)
    GenerateVM(op, o1, o2, r);
  else GenerateTXT(op, o1, o2, r);
}



void GenerateTXT(QUADOP op,DATAOBJ *o1,DATAOBJ *o2,ADDRESS *r)
{
  struct _rcheck *rc;
  char l1[80],
       l2[80],
       l3[80];

  if (op==QOP_STARTCALL)
    return;

  printf("#%-3x ", this_quad++);

  switch (op)
  {
    case QOP_RANGE:
      rc=(struct _rcheck *)o1;
      printf("(RANGE, %s, %d, %d)\n", ObjName(rc->obj, l1), rc->lo, rc->hi);
      break;

    case QOP_FUNCRET:
    case QOP_FUNCSTART:
    case QOP_FUNCJUMP:
      if (o1)
        printf("(%s, %s)\n", OpName(op), ((ATTRIBUTES *)o1)->name);
      break;

    case QOP_SKILL:
      printf("(%s, %s)\n", OpName(op), ObjName(o1, l1));
      break;

    case QOP_STARTCALL:
      printf("(%s, %s)\n", OpName(op), (char *)o1);
      break;

    case QOP_ARG_VAL:
    case QOP_ARG_REF:
      printf("(%s, %s)\n", OpName(op), ObjName(o1, l1));
      break;

    case QOP_ASSIGN:
    case QOP_BYTE2WORD:
    case QOP_BYTE2DWORD:
    case QOP_WORD2BYTE:
    case QOP_WORD2DWORD:
    case QOP_DWORD2BYTE:
    case QOP_DWORD2WORD:
      printf("(%s, %s -> %s)\n",OpName(op),ObjName(o1,l1),ObjName(o2,l2));
      break;

    case QOP_SCOPY:
      printf("(SCOPY, %s -> %s)\n", ObjName(o1,l1), ObjName(o2,l2));
      break;
      
    case QOP_PUSH:
    case QOP_POP:
      printf("(%s, %s)\n", OpName(op), AddressName((ADDRESS *)o1, l1));
      break;

    case QOP_JMP:
      printf("(JMP, ?""?""?)\n");
      break;

    case QOP_JZ:
    case QOP_JNZ:
      printf("(%s, %s, ?""?""?)\n",OpName(op), ObjName(o1, l1));
      break;

    default:
    {
      char *p1, *p2, *p3, *p4;

      p1=OpName(op);
      p2=ObjName(o1,l1);
      p3=ObjName(o2,l2);
      p4=AddressName(r,l3);

      printf("(%s, %s, %s, %s)\n", p1, p2, p3, p4);
    }

    /*
      printf("(%s, %s, %s, %s)\n",
             OpName(op),ObjName(o1,l1),ObjName(o2,l2),AddressName(r,l3));
    */
  }
}


int BackPatch(PATCH *pat, VMADDR to_where)
{
  if (vm_output)
    return BackPatchVM(pat, to_where);
  else return BackPatchTXT(pat, to_where);
}
 
int BackPatchTXT(PATCH *pat, VMADDR to_where)
{
  PATCH *p;
  
  for (p=pat; p; p=p->next)
    if (p->quad)
      printf("Backpatch #%" UINT32_FORMAT ": jump to %" UINT32_XFORMAT "\n", p->quad, to_where);

  return 0;
}


static char * near OpName(QUADOP op)
{
  struct _optable *ot;
  
  for (ot=optable; ot->op; ot++)
    if (op==ot->op || op==ot->strop)
      return ot->str;

  return "<bug>";
}



static char * near AddressName(ADDRESS *a,char *out)
{
  char *o=out;

  *o='\0';


  if (a->indirect)
  {
    strcpy(o,"*");
    o++;
  }

  if (a->typedesc==&ByteType)
    { strcpy(o,"(byte)"); o += strlen(o); }
  else if (a->typedesc==&UnsignedByteType)
    { strcpy(o,"(ubyte)"); o += strlen(o); }
  else if (a->typedesc==&WordType)
    { strcpy(o,"(word)"); o += strlen(o); }
  else if (a->typedesc==&UnsignedWordType)
    { strcpy(o,"(uword)"); o += strlen(o); }
  else if (a->typedesc==&DwordType)
    { strcpy(o,"(dword)"); o += strlen(o); }
  else if (a->typedesc==&UnsignedDwordType)
    { strcpy(o,"(udword)"); o += strlen(o); }
  else if (a->typedesc==&StringType)
    { strcpy(o,"(string)"); o += strlen(o); }
  else if (a->typedesc==&AddrType)
    { strcpy(o,"(address)"); o += strlen(o); }

  if (a->segment==SEG_TEMP)
    sprintf(o, "Temp:%d", a->offset);
  else if (a->segment==SEG_AR)
    sprintf(o, "Local:%d", a->offset);
  else if (a->segment==SEG_GLOBAL)
    sprintf(o, "Global:%d", a->offset);
  else *o='\0';

  return out;
}       


static char * near ObjName(DATAOBJ *o,char *out)
{
  char *orig=out;
  char *s;
  
  *out='\0';

  if (o->objform==ObjformValue)
  {
    if (o->form.val.valkind==StringKind && o->form.addr.indirect)
    {
      strcpy(out,"*");
      out++;
    }

    switch (o->form.val.valkind)
    {
      case ByteKind:
        sprintf(out,
                "(%sbyte)'%c'",
                &"u"[o->type->fSigned],
                (unsigned int)o->form.val.kind.byteval);
        break;

      case WordKind:
        if (o->type->fSigned)
          sprintf(out,
                  "(word)%d",
                  (int)(sword)o->form.val.kind.wordval);
        else
          sprintf(out,
                  "(uword)%u",
                  (unsigned int)o->form.val.kind.wordval);
        break;

      case DwordKind:
        if (o->type->fSigned)
          sprintf(out,
                  "(dword)%ld",
                  (long)(sdword)o->form.val.kind.dwordval);
        else
          sprintf(out,
                  "(udword)%lu",
                  (unsigned long)o->form.val.kind.dwordval);
        break;
        
      case StringKind:
        s=GlobGetString(o->form.val.kind.str.offset);
        
        strcpy(out,"(string)");
        
        if (s)
        {
          sprintf(out+strlen(out), "\"%s\"", s);
          free(s);
        }
        break;
    }

    return orig;
  }


  if (o->name && !show_addr)
  {
    if (o->form.addr.indirect)
    {
      strcpy(out,"*");
      out++;
    }

    if (o->type)
      switch (o->type->form)
      {
        case FormByte:
          sprintf(out,
                  "(%sbyte)",
                  &"u"[o->type->fSigned]);
          break;

        case FormWord:
          sprintf(out,
                  "(%sword)",
                  &"u"[o->type->fSigned]);
          break;

        case FormDword:
          sprintf(out,
                  "(%sdword)",
                  &"u"[o->type->fSigned]);
          break;

        case FormString:
          sprintf(out, "(string)");
          break;

        default:
          *out='\0';
      }

    strcat(out, o->name);
    return orig;
  }

  if (o->type && o->type->form==FormAddr)
    strcpy(out, "(address)");

  return AddressName(&o->form.addr, out+strlen(out));
}


