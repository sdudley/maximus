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

/*# name=Regular-expression include file.
*/

#ifndef __REP_H_INCLUDED
#define __REP_H_INCLUDED

#include "quote.h"

#define TABLE_LEN 255
#define MAX_REP 64

#define CHAR_NORM       0x01
#define CHAR_TABLE      0x02
#define CHAR_SOL        0x04
#define CHAR_EOL        0x08
#define CHAR_ANY        0x10
#define CHAR_ZMORE      0x20
#define CHAR_1MORE      0x40

#define CERR_NOTERM   0x0001
#define CERR_NOQUOTE  0x0002
#define CERR_NOREPEAT 0x0003
#define CERR_EMPTYSET 0x0004
#define CERR_2R       0x0005
#define CERR_NOMEM    -1

struct _rep
{
  char type[MAX_REP];
  char c[MAX_REP];
  char *table[MAX_REP];
  int max_ch;
};

char * _fast stristrx(char *string,struct _rep *rep);
int _fast Compile_REP(char *exp,struct _rep *rep);

#endif /* __REP_H_INCLUDED */

