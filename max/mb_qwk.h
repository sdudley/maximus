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

#ifndef __MB_QWK_H_DEFINED
#define __MB_QWK_H_DEFINED

#include "qwk.h"

static struct _len_ch
{
  long rec;
  int blocks;
} *len_chain;




static int near Create_Control_DAT(void);
static int near Create_Messages_DAT(void);
static int near QWK_Compress_Mail(BROWSE *b);
static void _stdc bprintf(char *dest,char *format,...);
static void near Flush_Qidx(void);
static void near Flush_Len_Chain(void);
static void near Update_Length(long rec,int blocks);
static void near GenerateStupidFiles(void);
static void near FinishControlDAT(void);

#endif /* __MB_QWK_H_DEFINED */


