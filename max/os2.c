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

#pragma off(unreferenced)
static char rcs_id[]="$Id: os2.c,v 1.1.1.1 2002/10/01 17:52:59 sdudley Exp $";
#pragma on(unreferenced)

/*# name=OS/2-specific functions for Maximus
    credit=Mostly written by Pete, but crunched into this single
    credit=file by Scott :-)
*/

#ifndef OS_2
void OS2Init(void) {}
#else

#include <stdio.h>
#include "mm.h"

#define INCL_DOS
#define INCL_KBD
#define INCL_NOPM
#include <os2.h>


void OS2Init(void)
{
  /* WC defaults to ASCII mode for conio stuff.  We need to parse ^s and ^q *
   * so set it to raw mode.                                                 */

#ifdef __WATCOMC__
  KBDINFO kbdi;

  kbdi.cb=sizeof(kbdi);
  KbdGetStatus(&kbdi, 0);
  kbdi.fsMask &= ~(KEYBOARD_ASCII_MODE|KEYBOARD_ECHO_ON);
  kbdi.fsMask |= KEYBOARD_BINARY_MODE|KEYBOARD_ECHO_OFF;
  KbdSetStatus(&kbdi, 0);
#endif /* __WATCOMC__ */

  /* Buffer stdout appropriately */

  setvbuf(stdout, NULL, _IOFBF, 1024);

  /* Install exception handler after Fossil_Install() and LogOpen()
   * so that snserver.dll and comm.dll install their DosExitList()
   * rutines before we do.  This ensures that our cleanup routine
   * occurs first,  so that we can still use functions exported
   * by those dlls.
   */

  medinit();
}

#endif /* OS_2 */

