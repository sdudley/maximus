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

#pragma off(unreferenced)
static char rcs_id[]="$Id: vm_opmth.c,v 1.1 2002/10/01 17:54:16 sdudley Exp $";
#pragma on(unreferenced)

#define COMPILING_MEX_VM
#include "prog.h"
#include "vm.h"



int op_assign(INST *inst, struct _args *arg)
{
  switch (inst->opform)
  {
    case FormByte:  store(&inst->arg2.addr, FormByte, &arg->b1);   break;
    case FormWord:  store(&inst->arg2.addr, FormWord, &arg->w1);   break;
    case FormDword: store(&inst->arg2.addr, FormDword,&arg->dw1);  break;
    case FormAddr:
    case FormString:store(&inst->arg2.addr, FormAddr, &arg->a1);   break;
    default:        vm_err(err_invalid_form);
  }

  return 0;
}




int op_add(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  IADDR ra;
  
  switch (inst->opform)
  {
    case FormByte:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormByte, (rb=arg->b1+arg->b2, &rb));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset += ((inst->flag & FLAG_ARG1_ADDR) ? arg->b2 : arg->b1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;

    case FormWord:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormWord, (rw=arg->w1+arg->w2, &rw));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset += ((inst->flag & FLAG_ARG1_ADDR) ? arg->w2 : arg->w1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;

    case FormDword:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormDword, (rdw=arg->dw1+arg->dw2, &rdw));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset += (word)((inst->flag&FLAG_ARG1_ADDR) ? arg->dw2 :arg->dw1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;
      
    default:
      vm_err(err_invalid_form);
  }
  
  return 0;
}







int op_subtract(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  IADDR ra;
  
  switch (inst->opform)
  {
    case FormByte:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormByte, (rb=arg->b1-arg->b2, &rb));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset -= ((inst->flag & FLAG_ARG1_ADDR) ? arg->b2 : arg->b1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;

    case FormWord:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormWord, (rw=arg->w1-arg->w2, &rw));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset -= ((inst->flag & FLAG_ARG1_ADDR) ? arg->w2 : arg->w1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;

    case FormDword:
      if ((inst->flag & (FLAG_ARG1_ADDR|FLAG_ARG2_ADDR))==0)
        store(&inst->res.dest, FormDword, (rdw=arg->dw1-arg->dw2, &rdw));
      else
      {
        ra=(inst->flag & FLAG_ARG1_ADDR) ? arg->a1 : arg->a2;
        ra.offset -= (word)((inst->flag&FLAG_ARG1_ADDR) ? arg->dw2 :arg->dw1);
        store(&inst->res.dest, FormAddr, &ra);
      }
      break;

    default:
      vm_err(err_invalid_form);
  }
  
  return 0;
}


int op_multiply(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      store(&inst->res.dest, FormByte, (rb=arg->b1*arg->b2, &rb));
      break;

    case FormWord:
      store(&inst->res.dest, FormWord, (rw=arg->w1*arg->w2, &rw));
      break;

    case FormDword:
      store(&inst->res.dest, FormDword, (rdw=arg->dw1*arg->dw2, &rdw));
      break;

    default:
      vm_err(err_invalid_form);
  }

  return 0;
}




int op_shl(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      store(&inst->res.dest, FormByte, (rb=arg->b1 << arg->b2, &rb));
      break;

    case FormWord:
      store(&inst->res.dest, FormWord, (rw=arg->w1 << arg->w2, &rw));
      break;

    case FormDword:
      store(&inst->res.dest, FormDword, (rdw=arg->dw1 << arg->dw2, &rdw));
      break;

    default:
      vm_err(err_invalid_form);
  }

  return 0;
}




int op_shr(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      if (inst->flag & FLAG_ARG1_SIGNED)
        rb=(byte)((sbyte)arg->b1 >> arg->b2);
      else
        rb=(byte)(arg->b1 >> arg->b2);

      store(&inst->res.dest, FormByte, &rb);
      break;

    case FormWord:
      if (inst->flag & FLAG_ARG1_SIGNED)
        rw=(word)((sword)arg->w1 >> arg->w2);
      else
        rw=(word)(arg->w1 >> arg->w2);

      store(&inst->res.dest, FormWord, &rw);
      break;

    case FormDword:
      if (inst->flag & FLAG_ARG1_SIGNED)
        rdw=(dword)((sdword)arg->dw1 >> arg->dw2);
      else
        rdw=(dword)(arg->dw1 >> arg->dw2);

      store(&inst->res.dest, FormDword, &rdw);
      break;

    default:
      vm_err(err_invalid_form);
  }

  return 0;
}





int op_band(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      store(&inst->res.dest, FormByte, (rb=arg->b1 & arg->b2, &rb));
      break;

    case FormWord:
      store(&inst->res.dest, FormWord, (rw=arg->w1 & arg->w2, &rw));
      break;

    case FormDword:
      store(&inst->res.dest, FormDword, (rdw=arg->dw1 & arg->dw2, &rdw));
      break;

    default:
      vm_err(err_invalid_form);
  }

  return 0;
}




int op_bor(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      store(&inst->res.dest, FormByte, (rb=arg->b1 | arg->b2, &rb));
      break;

    case FormWord:
      store(&inst->res.dest, FormWord, (rw=arg->w1 | arg->w2, &rw));
      break;

    case FormDword:
      store(&inst->res.dest, FormDword, (rdw=arg->dw1 | arg->dw2, &rdw));
      break;

    default:
      vm_err(err_invalid_form);
  }

  return 0;
}




int op_divide(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      if (!arg->b2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rb=(sbyte)arg->b1/(sbyte)arg->b2;
      else
        rb=arg->b1/arg->b2;

      store(&inst->res.dest, FormByte, &rb);
      break;

    case FormWord:
      if (!arg->w2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rw=(sword)arg->w1/(sword)arg->w2;
      else
        rw=arg->w1/arg->w2;

      store(&inst->res.dest, FormWord, &rw);
      break;

    case FormDword:
      if (!arg->dw2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rdw=(sdword)arg->dw1/(sdword)arg->dw2;
      else
        rdw=arg->dw1/arg->dw2;

      store(&inst->res.dest, FormDword, &rdw);
      break;

    default:
      vm_err(err_invalid_form);
  }
  
  return 0;
}




int op_mod(INST *inst, struct _args *arg)
{
  byte rb;
  word rw;
  dword rdw;
  
  switch (inst->opform)
  {
    case FormByte:
      if (!arg->b2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rb=(sbyte)arg->b1 % (sbyte)arg->b2;
      else
        rb=arg->b1 % arg->b2;

      store(&inst->res.dest, FormByte, &rb);
      break;

    case FormWord:
      if (!arg->w2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rw=(sword)arg->w1 % (sword)arg->w2;
      else
        rw=arg->w1 % arg->w2;

      store(&inst->res.dest, FormWord, &rw);
      break;

    case FormDword:
      if (!arg->dw2)
        vm_err(err_divby0);

      if (inst->flag & (FLAG_ARG1_SIGNED|FLAG_ARG2_SIGNED))
        rdw=(sdword)arg->dw1 % (sdword)arg->dw2;
      else
        rdw=arg->dw1 % arg->dw2;

      store(&inst->res.dest, FormDword, &rdw);
      break;

    default:
      vm_err(err_invalid_form);
  }
  
  return 0;
}



