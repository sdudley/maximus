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

  #define INCL_SUB

  #include "pos2.h"
  #include "prog.h"

  static int last_scan;

  int pascal kgetch(void)
  {
    int ret;
    KBDKEYINFO kki;

    if (last_scan)
    {
      ret=last_scan;
      last_scan=0;
      return ret;
    }

    KbdCharIn(&kki, IO_WAIT, 0);

    if (kki.chChar==0 ||
        (kki.chChar==0xe0 && (kki.fbStatus & 2 /*KBDTRF_EXTENDED_CODE*/)))
    {
      last_scan=kki.chScan;
      kki.chChar=0;
    }

    return kki.chChar;
  }



  int pascal kpeek(void)
  {
    KBDKEYINFO kki;

    if (last_scan)
      return last_scan;

    if (KbdPeek(&kki, 0)==0 && (kki.fbStatus & 0xe0))
      return kki.chChar;

    return -1;
  }

  int pascal khit(void)
  {
    return (kpeek() != -1);
  }

#elif defined(NT)

  #include "prog.h"
  #include "pwin.h"

  static int console_inited=FALSE;
  static HANDLE hIn;
  static int last_scan=0;

  static void _console_init(void)
  {
    DWORD dwMode;

    hIn=GetStdHandle(STD_INPUT_HANDLE);

    GetConsoleMode(hIn, &dwMode);
    dwMode |= ENABLE_PROCESSED_INPUT;
    dwMode &= ~ENABLE_WINDOW_INPUT | ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hIn, ENABLE_PROCESSED_INPUT);

    last_scan=0;
    console_inited=TRUE;
  }


  static int InputRecToKey(PINPUT_RECORD pir, int peek)
  {
    if (pir->Event.KeyEvent.uChar.AsciiChar==9 &&
        (pir->Event.KeyEvent.dwControlKeyState & SHIFT_PRESSED))
    {
      if (!peek)
        last_scan=pir->Event.KeyEvent.wVirtualScanCode;

      return 0;
    }

    if (pir->Event.KeyEvent.uChar.AsciiChar==0 ||
       (pir->Event.KeyEvent.dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)))
    {
      if (!peek)
        last_scan=pir->Event.KeyEvent.wVirtualScanCode;

      return 0;
    }

    return pir->Event.KeyEvent.uChar.AsciiChar;
  }

  /* Screen out input records which are not key events, downward keypresses,  *
   * or actual characters.                                                    */

  static int BadKeyRec(PINPUT_RECORD pir)
  {
    if (pir->EventType != KEY_EVENT || !pir->Event.KeyEvent.bKeyDown)
      return TRUE;

    switch (pir->Event.KeyEvent.wVirtualScanCode)
      case 0x2a: case 0x1d: case 0x38: case 0x3a: case 0x36:
      case 0x46: case 0x45:
        return TRUE;

    return FALSE;
  }

  int pascal kpeek(void)
  {
    INPUT_RECORD ir;
    DWORD dwGot;

    if (last_scan)
      return last_scan;

    if (!console_inited)
      _console_init();

    do
    {
      if (!PeekConsoleInput(hIn, &ir, 1, &dwGot) || dwGot==0)
        return -1;

      if (BadKeyRec(&ir))
        ReadConsoleInput(hIn, &ir, 1, &dwGot);
    }
    while (BadKeyRec(&ir));

    return InputRecToKey(&ir, TRUE);
  }

  int pascal kgetch()
  {
    INPUT_RECORD ir;
    DWORD dwGot;

    if (last_scan)
    {
      int rc;
      rc=last_scan;
      last_scan=0;
      return rc;
    }

    if (!console_inited)
      _console_init();

    do
    {
      if (!ReadConsoleInput(hIn, &ir, 1, &dwGot) || dwGot==0)
        return -1;
    }
    while (BadKeyRec(&ir));

    return InputRecToKey(&ir, FALSE);
  }

  int pascal khit(void)
  {
    return (kpeek() != -1);
  }

#elif defined(UNIX)

#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <curses.h>

int kgetch() /* might conflict with newer ncurses */
{
  fd_set rfds;
  struct timeval tv;
 
  FD_ZERO(&rfds);
  FD_SET(fileno(stdin), &rfds);
 
  tv.tv_sec = 0;
  tv.tv_usec = 10; /* very short wait! */
 
  if (select(fileno(stdin) + 1, &rfds, NULL, NULL, &tv) <= 0)
    return -1; /* Nothing pending; don't block! */

  if (stdscr)
    return getch(); /* curses - preferred way */

  return getchar(); /* fall back to stdio */
}

int kpeek()
{
  int ch;

  ch = kgetch();

  if (stdscr)
  {
    if (ch >= 0)
      ungetch(ch);
  }
  else
  {
    if (ch >= 0)
      ungetc(ch, stdin);
  }

  return ch;
}

int khit(void)
{
  return (kpeek() != -1);
}

#else
  #error Unknown OS!
#endif



