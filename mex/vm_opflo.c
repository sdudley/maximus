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

/* $Id: vm_opflo.c,v 1.3 2004/01/22 08:04:27 wmcbrine Exp $ */

#define COMPILING_MEX_VM
#include "prog.h"
#include "vm.h"

int op_jmp(INST *inst, struct _args *arg)
{
  #ifdef DEBUGVM
  VMADDR oldip=vaIp;
  #endif

  /* If it's an unconditional jump, just do so. */

  if (inst->opcode==QOP_JMP)
    vaIp=inst->res.jump_label;
  else switch (inst->opform)
  {
    case FormByte:
      /* Jump if op==QOP_JZ and b1==0, or if op==QOP_JNZ and b1 != 0. */

      if ((inst->opcode==QOP_JZ)==(arg->b1==0))
        vaIp=inst->res.jump_label;
      break;

    case FormWord:
      if ((inst->opcode==QOP_JZ)==(arg->w1==0))
        vaIp=inst->res.jump_label;
      break;
      
    case FormDword:
      if ((inst->opcode==QOP_JZ)==(arg->dw1==0))
        vaIp=inst->res.jump_label;
      break;
      
    default:
      vm_err(err_invalid_form);
  }

  #ifdef DEBUGVM
  if (deb)
    if (vaIp != oldip)
      printf("jump to cs%08" UINT32_XFORMAT ". ", vaIp);
  #endif
  
  return 0;
}


int op_logical(INST *inst, struct _args *arg)
{
  word res;

  switch (inst->opform)
  {
    case FormByte:
      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=((sbyte)arg->b1 >  (sbyte)arg->b2); break;
          case QOP_GE:  res=((sbyte)arg->b1 >= (sbyte)arg->b2); break;
          case QOP_EQ:  res=((sbyte)arg->b1 == (sbyte)arg->b2); break;
          case QOP_NE:  res=((sbyte)arg->b1 != (sbyte)arg->b2); break;
          case QOP_LE:  res=((sbyte)arg->b1 <= (sbyte)arg->b2); break;
          case QOP_LT:  res=((sbyte)arg->b1 <  (sbyte)arg->b2); break;
          case QOP_LAND:res=((sbyte)arg->b1 && (sbyte)arg->b2); break;
          case QOP_LOR: res=((sbyte)arg->b1 || (sbyte)arg->b2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }
      else
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=(arg->b1 >  arg->b2); break;
          case QOP_GE:  res=(arg->b1 >= arg->b2); break;
          case QOP_EQ:  res=(arg->b1 == arg->b2); break;
          case QOP_NE:  res=(arg->b1 != arg->b2); break;
          case QOP_LE:  res=(arg->b1 <= arg->b2); break;
          case QOP_LT:  res=(arg->b1 <  arg->b2); break;
          case QOP_LAND:res=(arg->b1 && arg->b2); break;
          case QOP_LOR: res=(arg->b1 || arg->b2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }
      break;

    case FormWord:
      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=((sword)arg->w1 >  (sword)arg->w2); break;
          case QOP_GE:  res=((sword)arg->w1 >= (sword)arg->w2); break;
          case QOP_EQ:  res=((sword)arg->w1 == (sword)arg->w2); break;
          case QOP_NE:  res=((sword)arg->w1 != (sword)arg->w2); break;
          case QOP_LE:  res=((sword)arg->w1 <= (sword)arg->w2); break;
          case QOP_LT:  res=((sword)arg->w1 <  (sword)arg->w2); break;
          case QOP_LAND:res=((sword)arg->w1 && (sword)arg->w2); break;
          case QOP_LOR: res=((sword)arg->w1 || (sword)arg->w2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }
      else
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=(arg->w1 >  arg->w2); break;
          case QOP_GE:  res=(arg->w1 >= arg->w2); break;
          case QOP_EQ:  res=(arg->w1 == arg->w2); break;
          case QOP_NE:  res=(arg->w1 != arg->w2); break;
          case QOP_LE:  res=(arg->w1 <= arg->w2); break;
          case QOP_LT:  res=(arg->w1 <  arg->w2); break;
          case QOP_LAND:res=(arg->w1 && arg->w2); break;
          case QOP_LOR: res=(arg->w1 || arg->w2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }

      break;

    case FormDword:
      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=((sdword)arg->dw1 >  (sdword)arg->dw2); break;
          case QOP_GE:  res=((sdword)arg->dw1 >= (sdword)arg->dw2); break;
          case QOP_EQ:  res=((sdword)arg->dw1 == (sdword)arg->dw2); break;
          case QOP_NE:  res=((sdword)arg->dw1 != (sdword)arg->dw2); break;
          case QOP_LE:  res=((sdword)arg->dw1 <= (sdword)arg->dw2); break;
          case QOP_LT:  res=((sdword)arg->dw1 <  (sdword)arg->dw2); break;
          case QOP_LAND:res=((sdword)arg->dw1 && (sdword)arg->dw2); break;
          case QOP_LOR: res=((sdword)arg->dw1 || (sdword)arg->dw2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }
      else
      {
        switch (inst->opcode)
        {
          case QOP_GT:  res=(arg->dw1 >  arg->dw2); break;
          case QOP_GE:  res=(arg->dw1 >= arg->dw2); break;
          case QOP_EQ:  res=(arg->dw1 == arg->dw2); break;
          case QOP_NE:  res=(arg->dw1 != arg->dw2); break;
          case QOP_LE:  res=(arg->dw1 <= arg->dw2); break;
          case QOP_LT:  res=(arg->dw1 <  arg->dw2); break;
          case QOP_LAND:res=(arg->dw1 && arg->dw2); break;
          case QOP_LOR: res=(arg->dw1 || arg->dw2); break;
          default:      vm_err(err_invalid_opcode);
        }
      }
      break;
      
    default:
      vm_err(err_invalid_form);
  }
  

  store(&inst->res.dest, FormWord, &res);
  return 0;
}




