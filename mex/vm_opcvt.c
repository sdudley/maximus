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
static char rcs_id[]="$Id: vm_opcvt.c,v 1.1.1.1 2002/10/01 17:54:12 sdudley Exp $";
#pragma on(unreferenced)

#define COMPILING_MEX_VM
#include "prog.h"
#include "vm.h"

int op_b2w(INST *inst, struct _args *arg)
{
  word w;

  NW(inst);
  
  if (inst->flag & FLAG_ARG1_SIGNED)
    w=(word)(sword)(sbyte)arg->b1;
  else
    w=(word)arg->b1;

  store(&inst->res.dest, FormWord, &w);
  return 0;
}

int op_b2dw(INST *inst, struct _args *arg)
{
  dword dw;

  NW(inst);

  if (inst->flag & FLAG_ARG1_SIGNED)
    dw=(dword)(sdword)(sbyte)arg->b1;
  else
    dw=(dword)arg->b1;

  store(&inst->res.dest, FormDword, &dw);
  return 0;
}

int op_w2b(INST *inst, struct _args *arg)
{
  byte b;

  NW(inst);

  if (inst->flag & FLAG_ARG1_SIGNED)
    b = (byte)(sbyte)(sword)arg->w1;
  else
    b = (byte)arg->w1;

  store(&inst->res.dest, FormByte, &b);
  return 0;
}

int op_w2dw(INST *inst, struct _args *arg)
{
  dword dw;

  NW(inst);

  if (inst->flag & FLAG_ARG1_SIGNED)
    dw=(dword)(sdword)(sword)arg->w1;
  else
    dw=(dword)arg->w1;

  store(&inst->res.dest, FormDword, &dw);
  return 0;
}

int op_dw2b(INST *inst, struct _args *arg)
{
  byte b;

  NW(inst);

  if (inst->flag & FLAG_ARG1_SIGNED)
    b = (byte)(sbyte)(sdword)arg->dw1;
  else
    b = (byte)arg->dw1;

  store(&inst->res.dest, FormByte, &b);
  return 0;
}


int op_dw2w(INST *inst, struct _args *arg)
{
  word w;

  NW(inst);

  if (inst->flag & FLAG_ARG1_SIGNED)
    w = (word)(sword)(sdword)arg->dw1;
  else
    w = (word)arg->dw1;

  store(&inst->res.dest, FormWord, &w);
  return 0;
}




