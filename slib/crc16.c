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

#include <stdlib.h>
#include "prog.h"
#include "crc.h"

word _fast crc16fn(word ch, word crc)
{
  word i;

  for (i=8, ch <<= 8; i--; ch <<= 1)
    crc=(crc << 1) ^ (((ch ^ crc) & 0x8000u) ? CRC16CCITT : 0);

  return crc;
}

word * _fast mkcrc16tab(void)
{
  word *tab;
  word i;

  if ((tab=malloc(sizeof(word)*256))==NULL)
    return NULL;

  for (i=0; i < 256; i++)
    tab[i]=crc16fn(i, 0);

  return tab;
}

