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
static char rcs_id[]="$Id: vm_opstk.c,v 1.1 2002/10/01 17:54:16 sdudley Exp $";
#pragma on(unreferenced)

#define COMPILING_MEX_VM
#include "prog.h"
#include "vm.h"

int op_push(INST *inst, struct _args *arg)
{
  switch (inst->opform)
  {
    case FormByte:  Push(arg->b1, byte);  break;
    case FormWord:  Push(arg->w1, word);  break;
    case FormDword: Push(arg->dw1,dword); break;
    case FormAddr:  Push(arg->a1, IADDR); break;
    case FormString:Push(arg->a1, IADDR); break;
    default:        vm_err(err_invalid_form);
  }
  
  return 0;
}

int op_pop(INST *inst, struct _args *arg)
{
  byte b1;
  word w1;
  dword dw1;
  IADDR a1;
  
  NW(arg);
  
  switch (inst->opform)
  {
    case FormByte: Pop(b1,byte); store(&inst->arg1.addr,FormByte,&b1);break;
    case FormWord: Pop(w1,word); store(&inst->arg1.addr,FormWord,&w1);break;
    case FormDword:Pop(dw1,dword);store(&inst->arg1.addr,FormDword,&dw1);break;
    case FormAddr: Pop(a1, IADDR);store(&inst->arg1.addr,FormAddr,&a1);break;
    case FormString:Pop(a1, IADDR);store(&inst->arg1.addr,FormAddr,&a1);break;
    default:        vm_err(err_invalid_form);
  }
  
  return 0;
}



