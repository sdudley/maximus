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

#include "voc_data.h"

VOCH VocOpen(char *name);
int VocRead(VOCH handle, blocks *block);
int VocCreate(char* name, VOCH handle);
int VocWriteSound(VOCH handle, blocks *block);
int VocWriteSilence(VOCH handle, duration interval, sample_rates sample_rate);
int VocWriteMarker(VOCH handle, markers mark);
int VocWriteText(VOCH handle, char *text);
int VocWriteRepeat(VOCH handle, repeat_counts count);
int VocWriteEndRepeat(VOCH handle);
int VocClose(VOCH handle);

blklens _VocReadThisLength(VOCH handle);
sample_rates _VocReadThisRate(VOCH handle);
pack_types _VocReadThisPack(VOCH handle);
Two_Byte_Naturals _VocReadThis2(VOCH handle);
int _VocReadGetSound(VOCH handle, blocks *block);
