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

#ifndef __MAX_EDIT_H_DEFINED
#define __MAX_EDIT_H_DEFINED

#include <setjmp.h>

extrn jmp_buf jumpto;       /* "Jump" location for MaxEd/BORED errors */
extrn byte *screen[LEN(MAX_LINES+2)];
extrn word num_lines;
extrn word max_lines;        /* Virtual maximum number of msg. lines */
extrn byte usrwidth;         /* Effective screen width for editors */

#endif /* __MAX_EDIT_H_DEFINED */
