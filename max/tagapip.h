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

static int _TagGetMtagMem(struct _mtagidx *pmti, struct _mtagmem *pmtm);
static void near _TagNewPmtm(struct _mtagmem *pmtm);
static int near _TagPackMessages(FILE *in_i, FILE *in_d, FILE *out_i, FILE *out_d, int recnum);
static int near _TagRepackMtag(int recnum);
static int near _TagAddToFree(struct _mtagidx *pmti, int recnum);
static int near _TagReuseIdx(struct _mtagidx *pmti, int recnum);
static int near _TagReadIdx(struct _mtagidx *pmti);


