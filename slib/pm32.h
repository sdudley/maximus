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

/* Handle differences between the OS/2 2.0 32-bit and OS/2 1.3 16-bit       *
 * Presentation Manager interface.                                          */

#ifdef __FLAT__
  #define MSGDEF    ULONG         /* PM's "msg" is long */
  #define WinQWindow(hwnd, id)  WinQueryWindow(hwnd, id)
#else
  #define MSGDEF    USHORT        /* PM's "msg" is short */
  #define WinQWindow(hwnd, id)  WinQueryWindow(hwnd, id, FALSE)
#endif

