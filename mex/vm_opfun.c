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
static char rcs_id[]="$Id: vm_opfun.c,v 1.2 2003/06/05 01:10:36 wesgarland Exp $";
#pragma on(unreferenced)

#define COMPILING_MEX_VM
#include <string.h>
#include "prog.h"
#include "vm.h"


int op_funcstart(INST *inst, struct _args *arg)
{
  NW(inst); NW(arg);
  
  #ifdef DEBUGVM
  if (deb)
    printf("Beginning function.  Local AR = %d, SP=%p, BP=%p. ",
           inst->res.jump_label, pbSp, pbBp);
  #endif

  /* Push the base pointer onto the stack */

  Push(pbBp, byte *);
  
  /* Now increase it by the size of the variables in our local AR, and      *
   * make sure that all variables are initialized to zero.                  */

  pbBp=pbSp;

  pbSp -= inst->res.jump_label;

  memset(pbSp, '\0', inst->res.jump_label);

  return 0;
}




int op_funcret(INST *inst, struct _args *arg)
{
  NW(inst); NW(arg);

  #ifdef DEBUGVM
  if (deb)
    printf("Ending fn: AR=%d, ", inst->res.jump_label);
  #endif

  /* Pop the local var section of the local AR off the stack */
  
  pbSp += inst->res.jump_label;
  
  /* Pop the old bp register off the stack */
  
  Pop(pbBp, byte *);
  
  /* Now pop off the return address, and return to the caller */

  Pop(vaIp, VMADDR);
  
  /* Now pop off the calling function's arguments */

  pbSp += inst->arg1.litdword;
  
  #ifdef DEBUGVM
  if (deb)
    printf("args=%" UINT32_FORMAT ", newIP = %08" UINT32_XFORMAT ", newSP=%p", inst->arg1.litdword, vaIp, pbSp);
  #endif
  return 0;
}



int op_startcall(INST *inst, struct _args *arg)
{
  NW(inst); NW(arg);
  
  #ifdef DEBUGVM
  if (deb)
    printf("Start call.  IP=%08" UINT32_XFORMAT ", SP=%p.  ", vaIp, pbSp);
  #endif

  return 0;
}




int op_arg_val(INST *inst, struct _args *arg)
{
  IADDR a;
/*word stk;*/ /*SJD 1991-05-31 11:43:25 */

  NW(inst); NW(arg);

  /* Push an argument (either pass-by-value or pass-by-ref) onto the stack */
  
  if (inst->flag & FLAG_ARG1_ADDR)
  {
    a=arg->a1; /*a=inst->arg1.addr;*/

    /* Process an indirect load */

    if (a.indirect)
    {
      a.indirect=FALSE;
      a=*(IADDR *)fetch(FormAddr, &a);
    }
    

    /* Convert local (relative to BP) references to relative-to-DS refs */
    
    if (a.segment==SEG_AR)
    {
      a.offset += (pbBp-pbDs);
      a.segment=SEG_GLOBAL;
    }
    else if (a.segment==SEG_TEMP)
      vm_err(err_cant_pass_reg_addr); /*SJD Mon  05-20-1991  20:57:27 */
    
    Push(a, IADDR);
  }
  else switch (inst->opform)
  {
/*case FormByte:  stk=(word)arg->b1;  Push(stk, word);              break;*/ /*SJD 1991-05-31 11:43:33 */
    case FormByte:                      Push(arg->b1, byte);          break;
    case FormWord:                      Push(arg->w1, word);          break;
    case FormDword:                     Push(arg->dw1, dword);        break;
    case FormAddr:
    case FormString:                    Push(arg->a1, IADDR);         break;
    default:                            vm_err(err_invalid_form);
  }

  return 0;
}




int op_funcjump(INST *inst, struct _args *arg)
{
  NW(inst); NW(arg);

  #ifdef DEBUGVM
  if (deb)
    printf("Calling function at %08" UINT32_XFORMAT ". IP=%08" UINT32_XFORMAT ", SP=%p",
           inst->res.jump_label, vaIp, pbSp);
  #endif

  /* Push the return address on the stack */

  Push(vaIp, VMADDR);

  /* And set the instruction pointer to the new jump label */

  vaIp=inst->res.jump_label;
  return 0;
}



