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

#ifndef __MAX_AREA_H_DEFINED
#define __MAX_AREA_H_DEFINED

#include "areaapi.h"

#ifndef ORACLE
extrn HAF haf;              /* Handle for farea.dat */
extrn HAF ham;              /* Handle for marea.dat */
#endif

extrn MAH mah;              /* Current message area */
extrn FAH fah;              /* Current file area */

#if 0
extrn struct _area area;    /* AREA.DAT entry for this area                */
extrn int barricade_priv;   /* Priv. for this level via barricade */
extrn char areatag[LEN(30)];/* The short name (leaf directory) for this area*/
extrn char entry;           /* If we just entered a message area */
#endif

#endif /* __MAX_AREA_H_DEFINED */

