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

/* Prototypes for functions to implement opcodes */

int op_nop(INST *inst, struct _args *arg);
int op_add(INST *inst, struct _args *arg);
int op_subtract(INST *inst, struct _args *arg);
int op_multiply(INST *inst, struct _args *arg);
int op_divide(INST *inst, struct _args *arg);
int op_mod(INST *inst, struct _args *arg);
int op_jmp(INST *inst, struct _args *arg);
int op_logical(INST *inst, struct _args *arg);
int op_b2w(INST *inst, struct _args *arg);
int op_b2dw(INST *inst, struct _args *arg);
int op_w2b(INST *inst, struct _args *arg);
int op_w2dw(INST *inst, struct _args *arg);
int op_dw2b(INST *inst, struct _args *arg);
int op_dw2w(INST *inst, struct _args *arg);
int op_assign(INST *inst, struct _args *arg);
int op_funcstart(INST *inst, struct _args *arg);
int op_funcret(INST *inst, struct _args *arg);
int op_startcall(INST *inst, struct _args *arg);
int op_arg_val(INST *inst, struct _args *arg);
int op_funcjump(INST *inst, struct _args *arg);
int op_slogical(INST *inst, struct _args *arg);
int op_scat(INST *inst, struct _args *arg);
int op_scopy(INST *inst, struct _args *arg);
int op_skill(INST *inst, struct _args *arg);
int op_slval(INST *inst, struct _args *arg);
int op_srval(INST *inst, struct _args *arg);
int op_shl(INST *inst, struct _args *arg);
int op_shr(INST *inst, struct _args *arg);
int op_band(INST *inst, struct _args *arg);
int op_bor(INST *inst, struct _args *arg);
int op_push(INST *inst, struct _args *arg);
int op_pop(INST *inst, struct _args *arg);
int op_range(INST *inst, struct _args *arg);


