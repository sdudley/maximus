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


MenuFunction(DlgLstShow);
MenuFunction(DlgLstAct);
MenuFunction(DlgListUp);
MenuFunction(DlgListDown);
MenuFunction(DlgLstReg);

#define LISTBOX_COL           (CYELLOW | _BLUE)
#define LISTBOX_SELECT_COL    (CWHITE | _BLACK)
#define LISTBOX_SCROLL_COL    (CLCYAN | _BLUE)
#define DLG_LST(name, x, y, dt) {name, DlgLstAct, dt, NULL, NULL, NULL, x, y, DlgLstShow, NULL, DlgLstReg},

/* Doubly-linked list used for storing strings in our listbox */

typedef struct _uilist
{
  char *txt;
  void *app_inf;
  struct _uilist *next;
  struct _uilist *prior;
} UILIST, *PUILIST;

typedef struct _uihead
{
  PUILIST head;
  PUILIST tail;

  PUILIST top;
  PUILIST cur;

  unsigned uiNumItems;
  unsigned uiTop;
  unsigned uiCur;

  int (*pfnSelect)(int idx, PUILIST pui);
} UIHEAD, *PUIHEAD;

void LstCreateList(PUIHEAD puh, int (*pfnSelect)(int idx, PUILIST pui));
int LstAddList(PUIHEAD puh, char *txt, void *app_inf);
void LstDestroyList(PUIHEAD puh); /*SJD Tue  03-30-1993  00:44:09 */

