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

void StartPairs()
{
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);    
    init_pair(3, COLOR_RED, COLOR_BLACK);    
    init_pair(4, COLOR_WHITE, COLOR_BLACK);    
    init_pair(5, COLOR_CYAN, COLOR_BLACK);    
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK);        
    init_pair(7, COLOR_YELLOW, COLOR_BLACK);        
    init_pair(8, COLOR_WHITE, COLOR_BLACK);        
}

int cursesAttribute(unsigned char dosAttribute)
{
  /* this doesn't support non-default background. 
   * if we want that, we need to define 64 color
   * pairs and go from there.
   */

  int        ch = 0;
  int		tmp = 0;

  switch (dosAttribute & (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED))
  {
    case FOREGROUND_BLUE:
      ch = 1;
      break;
    case FOREGROUND_GREEN:
      ch = 2;
      break;
    case FOREGROUND_RED:
      ch = 3;
      break;
    case FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED:
      ch = 4;
      break;
    case FOREGROUND_BLUE | FOREGROUND_GREEN:
      ch = 5;
      break;
    case FOREGROUND_BLUE | FOREGROUND_RED:
      ch = 6;
      break;
    case FOREGROUND_GREEN | FOREGROUND_RED:
      ch = 7;
      break;
    default:
      ch = 8;
      break;
  }

/*  if (!(dosAttribute & FOREGROUND_INTENSITY))
    ch |= A_DIM;

  if (dosAttribute & BACKGROUND_INTENSITY)
    ch |= A_REVERSE;*/

  return ch;
}
