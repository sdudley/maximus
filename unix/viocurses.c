#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <curses.h>
#include "viocurses.h"

/* vfossil/os2-style vio routines for curses.
 * Assumes curses has already been initialized in
 * single-screen mode. Not by any stretch a full
 * implemenation.
 */

int VioSetCurPos(int row, int column, void *handle)
{
  if (stdscr)
    move(row,column);
  return 0;
}

int VioWrtTTY(const char *string, size_t length, void *handle)
{
  char *buf = NULL;

  if (string[length] != (char)0) /* asciz? */
  {
    buf = malloc(length + 1);
    if (buf)
    {
      memcpy(buf, string, length);
      buf[length] = (char)0;
      string = buf;
    }
  }

  if (string)
  {
    /* putp(string); */
    if (stdscr)
    {
      addstr(string);
      refresh();
    }
  }
  else
    return 1;

  if (buf)
    free(buf);

  return 0;  
}

chtype cursesAttribute(unsigned char dosAttribute)
{
  /* this doesn't support non-default background. 
   * if we want that, we need to define 64 color
   * pairs and go from there.
   */

  chtype        ch;

/* busted */
 //return 0;
	
  switch (dosAttribute & (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED))
  {
    case FOREGROUND_BLUE:
      ch = COLOR_BLUE;
      break;
    case FOREGROUND_GREEN:
      ch = COLOR_GREEN;
      break;
    case FOREGROUND_RED:
      ch = COLOR_RED;
      break;
    case FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED:
      ch = COLOR_WHITE;
      break;
    case FOREGROUND_BLUE | FOREGROUND_GREEN:
      ch = COLOR_CYAN;
      break;
    case FOREGROUND_BLUE | FOREGROUND_RED:
      ch = COLOR_MAGENTA;
      break;
    case FOREGROUND_GREEN | FOREGROUND_RED:
      ch = COLOR_YELLOW;
      break;
    default:
      ch = COLOR_BLACK;
      break;
  }

  if (!(dosAttribute & FOREGROUND_INTENSITY))
    ch |= A_DIM;

  if (dosAttribute & BACKGROUND_INTENSITY)
    ch |= A_REVERSE;

  return ch;
}


