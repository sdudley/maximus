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

#if defined(OS_2)

#include <stdio.h>
#include <stdlib.h>
#include "prog.h"
#include "dv.h"
#include "tui_mous.h"

#define INCL_MOU
#define INCL_NOPM
#define INCL_DOS
#include "pos2.h"



/* Install the mouse driver */

word has_mouse = FALSE;

static HMOU hm = 0;

sword _fast MouseOpen(void)
{
    USHORT evmask;
    PTRLOC pl;

    if(MouOpen(NULL, &hm))
        return FALSE;
    else{
        has_mouse = TRUE;
        evmask = MOUSE_MOTION_WITH_BN1_DOWN | MOUSE_MOTION_WITH_BN2_DOWN |
                 MOUSE_BN1_DOWN | MOUSE_BN2_DOWN | MOUSE_MOTION;
        MouSetEventMask(&evmask, hm);
        MouGetPtrPos(&pl, hm);
        return TRUE;
    }
}


void _fast MouseShow(void)
{
    if (!has_mouse)
	return;
    MouDrawPtr(hm);
}

void _fast MouseHide(void)
{
    NOPTRRECT np;

    if (!has_mouse)
	return;

    np.row  = 0;
    np.col  = 0;
    np.cRow = VidNumRows() - 1;
    np.cCol = VidNumCols()-1;
    MouRemovePtr(&np, hm);
}


void _fast MouseStatus(word * button, word * col, word * row)
{
  MOUEVENTINFO me;
  USHORT fWait;
  static word b=0,c=0,r=0;

  if (!has_mouse)
    return;

  fWait = MOU_NOWAIT;

  if (MouReadEventQue(&me, &fWait, hm)==0 && me.time)
  {
    *button=0;

    if (me.fs & (MOUSE_BN1_DOWN|MOUSE_MOTION_WITH_BN1_DOWN))
      *button |= BUT_LEFT;

    if (me.fs & (MOUSE_BN2_DOWN|MOUSE_MOTION_WITH_BN2_DOWN))
      *button |= BUT_RIGHT;

    b = *button;
    *row = r = me.row;
    *col = c = me.col;

/*    printf("x=%3d, y=%3d, but1=%d, but2=%d (fs=%04x)\n", *row, *col,
           !!(*button & BUT_LEFT),
           !!(*button & BUT_RIGHT),
           me.fs);*/
    }
    else{
        *button = b;
        *col    = c;
        *row    = r;
    }
}

void _fast MouseSetPos(word col, word row)
{
    PTRLOC pl;
    if (!has_mouse)
	return;
    pl.col = col;
    pl.row = row;
    //MouSetPtrPos(&pl, hm);
}

void _fast MouseSetRange(word col, word row, word n_col, word n_row)
{
  NW(col);
  NW(row);
  NW(n_col);
  NW(n_row);

  return;
}


void _fast MouseSetSoftCursor(word scrmask, word curmask)
{
  NW(scrmask);
  NW(curmask);
}


void _fast MouseClose(void)
{
    if (!has_mouse)
	return;

    MouseHide();
    has_mouse = FALSE;
    MouClose(hm);
}

void _fast MouseFlush(void)
{
    if(has_mouse){
        MouFlushQue(hm);
    }
}

#elif defined(NT)

#include <stdio.h>
#include <stdlib.h>
#include "prog.h"
#include "dv.h"
#include "tui_mous.h"

word has_mouse = FALSE;

sword _fast MouseOpen(void)
{
  return FALSE;
}

void _fast MouseShow(void)
{
}

void _fast MouseHide(void)
{
}


void _fast MouseStatus(word * button, word * col, word * row)
{
}

void _fast MouseSetPos(word col, word row)
{
}

void _fast MouseSetRange(word col, word row, word n_col, word n_row)
{
}


void _fast MouseSetSoftCursor(word scrmask, word curmask)
{
}

void _fast MouseClose(void)
{
}

void _fast MouseFlush(void)
{
}
#endif

