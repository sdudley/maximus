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

#define INCL_PM
#include <pos2.h>

extern HAB hab;
extern HWND hwndFrame, hwndClient;

void TuiPmInit(void);
void TuiPmTerm(void);
int TuiPmEvent(HVMENU vmenu);

#define ID_RESOURCE 1

/* Structure of the menu template */

typedef struct _menutemplate
{
  USHORT cb;
  USHORT version;
  USHORT codepage;
  USHORT iInputSize;
  USHORT cMti;
} MENUTEMPLATE;

/* Structure of a menu item within a template */

typedef struct _menuitem
{
  USHORT afStyle;
  USHORT afAttribute;
  USHORT idItem;
} MTI;

/* Definitions controlling the assignment of ID numbers to menu items */

#define PM_BLOCK_START  0
#define PM_BLOCK_SIZE   1024
#define PM_BLOCK_DIV    32


