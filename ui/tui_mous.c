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

#include <dos.h>
#include "prog.h"
#include "tui.h"

#ifdef __MSDOS__

/* Install the mouse driver */

static word near buttons=2;
static word near hidden=0;
word has_mouse=FALSE;


static void near _fast CallMouse(word *ax, word *bx, word *cx, word *dx)
{
  union REGS r;

#ifdef __FLAT__
  r.x.eax=*ax;

  if (bx)
    r.x.ebx=*bx;

  if (cx)
    r.x.ecx=*cx;

  if (dx)
    r.x.edx=*dx;

  int386(0x33, &r, &r);

  if (ax)
    *ax=r.x.eax;

  if (bx)
    *bx=r.x.ebx;

  if (cx)
    *cx=r.x.ecx;

  if (dx)
    *dx=r.x.edx;
#else
  r.x.ax=*ax;

  if (bx)
    r.x.bx=*bx;

  if (cx)
    r.x.cx=*cx;

  if (dx)
    r.x.dx=*dx;

  int86(0x33, &r, &r);

  if (ax)
    *ax=r.x.ax;

  if (bx)
    *bx=r.x.bx;

  if (cx)
    *cx=r.x.cx;

  if (dx)
    *dx=r.x.dx;
#endif
}


sword _fast MouseOpen(void)
{
  word ax=0;
  word bx=0, cx=0, dx=0;
  
  CallMouse(&ax, &bx, &cx, &dx);

  if (ax != 0xffff)
    return FALSE;

  if (bx==0xffff)
    buttons=2;
  else buttons=3;
  
  hidden=0;
  has_mouse=TRUE;

  return TRUE;
}


void _fast MouseShow(void)
{
  word ax=1;
  
  if (!has_mouse)
    return;

  do
  {
    ax=1;
    CallMouse(&ax, NULL, NULL, NULL);
    
    if (hidden > 0)
      hidden--;
  }
  while (hidden > 0);
}

void _fast MouseHide(void)
{
  word ax;
  
  if (!has_mouse)
    return;

  ax=2;
  CallMouse(&ax, NULL, NULL, NULL);
  hidden++;
}


void _fast MouseStatus(word *button, word *col, word *row)
{
  word ax=3;
  
  if (!has_mouse)
    return;

  /* See BUT_XXX constants for the 'button' field */

  CallMouse(&ax, button, col, row);
  
  *col >>= 3;
  *row >>= 3;
}

void _fast MouseSetPos(word col, word row)
{
  word ax=4;
  word c=col, r=row;
  
  if (!has_mouse)
    return;

  c <<= 3;
  r <<= 3;
  
  CallMouse(&ax, NULL, &c, &r);
}

void _fast MouseGetPress(word button, word *state, word *count, 
                         word *col, word *row)
{
  word ax=5;
  word bx=button;
  
  if (!has_mouse)
    return;

  CallMouse(&ax, &bx, col, row);
  *state=ax;
  *count=bx;
  
  *col >>= 3;
  *row >>= 3;
}

void _fast MouseGetRelease(word button, word *state, word *count, 
                           word *col, word *row)
{
  word ax=6;
  word bx=button;
  
  if (!has_mouse)
    return;
  
  CallMouse(&ax, &bx, col, row);

  *state=ax;
  *count=bx;
  
  *col >>= 3;
  *row >>= 3;
}

void _fast MouseSetRange(word col, word row, word n_col, word n_row)
{
  word ax;
  word cx, dx;
  
  if (!has_mouse)
    return;

  ax=7;
  cx=col;
  dx=col+n_col-1;
  
  cx <<= 3;
  dx <<= 3;
  
  CallMouse(&ax, NULL, &cx, &dx);

  ax=8;
  cx=row;
  dx=row+n_row-1;
  
  cx <<= 3;
  dx <<= 3;
  
  CallMouse(&ax, NULL, &cx, &dx);
}


void _fast MouseSetSoftCursor(word scrmask, word curmask)
{
  word ax, bx, cx, dx;
  
  if (!has_mouse)
    return;

  ax=0x0a;
  bx=0;
  cx=scrmask;
  dx=curmask;
  
  CallMouse(&ax, &bx, &cx, &dx);
}





void _fast MouseClose(void)
{
  if (!has_mouse)
    return;

  MouseHide();
  has_mouse=FALSE;
}

void _fast MouseFlush(void)
{
}


#ifdef NEVER

void _fast MouseSetMickey(word xmickey, word ymickey)
{
  word ax=0x0f;
  word cx=xmickey;
  word dx=ymickey;

  CallMouse(&ax, NULL, &cx, &dx);
}


#define EVT_MOVE    0x01
#define EVT_PLEFT   0x02
#define EVT_RLEFT   0x04
#define EVT_PRIGHT  0x08
#define EVT_RRIGHT  0x10
#define EVT_PMIDDLE 0x20
#define EVT_RMIDDLE 0x40

void _fast MouseSetEvent(word evtmask, void (far *evtproc)(void))
{
  union REGS r;
  struct SREGS sr;
  
  if (!has_mouse)
    return;

  sr.es=FP_SEG(evtproc);
  r.x.ax=0x0c;
  r.x.cx=evtmask;
  r.x.dx=FP_OFF(evtproc);
  
  int86x(0x33, &r, &r, &sr);
}

#endif /* NEVER */

#endif /* __MSDOS__ */


