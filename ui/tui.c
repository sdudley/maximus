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

#include <stdarg.h>
#include "tui.h"

/* Copyright msg to embed in executable */

#define VERSION "0.20"

extern void (_fast *WinHideFunc)(void);
extern void (_fast *WinShowFunc)(void);

#ifndef __MSDOS__
#define Vid_MonoCard 0
#endif

void _fast TuiOpen(void)
{
#ifdef PM
  TuiPmInit();
#else
  WinApiOpen(TRUE);
 
  MouseOpen();

  if (has_mouse)
  {
    WinHideFunc=MouseHide;
    WinShowFunc=MouseShow;
    
#ifdef __MSDOS__
    if (Vid_MonoCard)
      MouseSetSoftCursor(0x0000, 0x0f04); /* One-colour dimaond for MDA */
    else
#endif
      MouseSetSoftCursor(0xffff, 0x3f00);  /* Multicolour block cursor */

    MouseSetPos(0, 0);
    MouseSetRange(0, 0, VidNumCols(), VidNumRows());

    MouseShow();
  }
#endif
}

void _fast TuiClose(void)
{
#ifdef PM
  TuiPmTerm();
#else
  if (has_mouse)
  {
    MouseClose();
    WinHideFunc=WinShowFunc=NULL;
  }

  WinSync(wscrn, TRUE);
  WinApiClose();
#endif
}


/* Mapping function for CGA colours to mono */

byte col(byte c)
{
  if (!Vid_MonoCard)
    return c;

  /* If blue bkg, change fg to gray */

  if (((c >> 4) & 0x07)==0x01)
  {
    c &= ~0x0f;
    c |= 0x07;
  }

  /* Turn off background if not black or white */

  if (((c >> 4) & 0x07) != 7 || (c & 0x08))
    c &= 0x8f;
  
  if ((c & 0x07) != 7 && (c & 0x07) != 0)
  {
    c &= ~0x07;
    c |= 0x07;
  }

  if (c==0x77)
    c=0x0f;

  return c;
}

