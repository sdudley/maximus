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

sword _fast MouseOpen(void);
void _fast MouseShow(void);
void _fast MouseHide(void);
void _fast MouseStatus(word *button, word *col, word *row);
void _fast MouseSetPos(word col, word row);
void _fast MouseGetPress(word button, word *state, word *count, 
                         word *col, word *row);
void _fast MouseGetRelease(word button, word *state, word *count, 
                           word *col, word *row);
void _fast MouseSetRange(word col, word row, word n_col, word n_row);
void _fast MouseSetSoftCursor(word scrmask, word curmask);
void _fast MouseSetEvent(word evtmask, void (far *evtproc)(void));
void _fast MouseClose(void);
void _fast MouseSetMickey(word xmickey, word ymickey);
void _fast MouseFlush(void);

extern word has_mouse;


/* Flags for the 'button' field of the MouseStatus() call */

#define BUT_LEFT    0x01
#define BUT_RIGHT   0x02
#define BUT_MIDDLE  0x04
