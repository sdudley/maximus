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

#include "skiplist.h"

#define MAXLEVEL 16
#define PARTITION 4

static SLNODE * near _fast _SkipCreateNode(word levels, void *data);
static void near _fast _SkipBlank(SLIST *sl);
static SLNODE * near _fast _SkipFindNode(SLIST *sl, void *data, SLNODE *ptrs[]);
static void near _fast _SkipFreeNode(SLNODE *sn);
static word near _fast _SkipRandomLevel(SLIST *sl);


