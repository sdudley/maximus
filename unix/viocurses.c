#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <curses.h>
#include "dv.h"
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

void InitPairs()
{
    init_pair(CBLACK | _BLACK, COLOR_BLACK, COLOR_BLACK); /*0*/
    init_pair(CBLUE | _BLACK, COLOR_BLUE, COLOR_BLACK); /*1*/
    init_pair(CGREEN | _BLACK, COLOR_GREEN, COLOR_BLACK); /*2*/
    init_pair(CCYAN | _BLACK, COLOR_CYAN, COLOR_BLACK); /*3*/       
    init_pair(CRED | _BLACK, COLOR_RED, COLOR_BLACK); /*4*/   
    init_pair(CMAGENTA | _BLACK, COLOR_MAGENTA, COLOR_BLACK); /*5*/
    init_pair(CBROWN | _BLACK, COLOR_YELLOW, COLOR_BLACK); /*6*/
    init_pair(CGRAY | _BLACK, COLOR_WHITE, COLOR_BLACK); /*7*/
    init_pair(CDGRAY | _BLACK, COLOR_WHITE, COLOR_BLACK); /*8*/
    init_pair(CLBLUE | _BLACK, COLOR_BLUE, COLOR_BLACK);
    init_pair(CLGREEN | _BLACK, COLOR_GREEN, COLOR_BLACK);
    init_pair(CLCYAN | _BLACK, COLOR_CYAN, COLOR_BLACK);
    init_pair(CLRED | _BLACK, COLOR_RED, COLOR_BLACK); /*4*/   
    init_pair(CLMAGENTA | _BLACK, COLOR_MAGENTA, COLOR_BLACK); /*5*/
    init_pair(CYELLOW | _BLACK, COLOR_YELLOW, COLOR_BLACK); /*14*/
    init_pair(CWHITE | _BLACK, COLOR_WHITE, COLOR_BLACK); /*15*/
    // Blue   
    init_pair(CBLUE | _GREEN, COLOR_BLUE, COLOR_GREEN);    
    init_pair(CBLUE | _RED, COLOR_BLUE, COLOR_RED);    
    init_pair(CBLUE | _WHITE, COLOR_BLUE, COLOR_WHITE);    
    init_pair(CBLUE | _CYAN, COLOR_BLUE, COLOR_CYAN);    
    init_pair(CBLUE | _MAGENTA, COLOR_BLUE, COLOR_MAGENTA);    
    init_pair(CBLUE | _BROWN, COLOR_BLUE, COLOR_YELLOW);
    init_pair(CBLUE | _GRAY, COLOR_BLUE, COLOR_WHITE);
    // Green
    init_pair(CGREEN | _BLUE, COLOR_GREEN, COLOR_BLUE);
    init_pair(CGREEN | _RED, COLOR_GREEN, COLOR_GREEN);   
    init_pair(CGREEN | _WHITE, COLOR_GREEN, COLOR_WHITE);    
    init_pair(CGREEN | _CYAN, COLOR_GREEN, COLOR_CYAN);    
    init_pair(CGREEN | _MAGENTA, COLOR_GREEN, COLOR_MAGENTA);
    init_pair(CGREEN | _BROWN, COLOR_GREEN, COLOR_YELLOW);    
    init_pair(CGREEN | _GRAY, COLOR_GREEN, COLOR_WHITE);    
    // Cyan
    init_pair(CCYAN | _BLUE, COLOR_CYAN, COLOR_BLUE);
    init_pair(CCYAN | _RED, COLOR_CYAN, COLOR_GREEN);   
    init_pair(CCYAN | _WHITE, COLOR_CYAN, COLOR_WHITE);    
    init_pair(CCYAN | _GREEN, COLOR_CYAN, COLOR_GREEN);    
    init_pair(CCYAN | _MAGENTA, COLOR_CYAN, COLOR_MAGENTA);
    init_pair(CCYAN | _BROWN, COLOR_CYAN, COLOR_YELLOW);    
    init_pair(CCYAN | _GRAY, COLOR_CYAN, COLOR_WHITE);    
    // Red
    init_pair(CRED | _BLUE, COLOR_RED, COLOR_BLUE);
    init_pair(CRED | _GREEN, COLOR_RED, COLOR_GREEN);   
    init_pair(CRED | _WHITE, COLOR_RED, COLOR_WHITE);    
    init_pair(CRED | _CYAN, COLOR_RED, COLOR_CYAN);    
    init_pair(CRED | _MAGENTA, COLOR_RED, COLOR_MAGENTA);
    init_pair(CRED | _BROWN, COLOR_RED, COLOR_YELLOW);    
    init_pair(CRED | _GRAY, COLOR_RED, COLOR_WHITE);    
    // Magenta
    init_pair(CMAGENTA | _BLUE, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(CMAGENTA | _RED, COLOR_MAGENTA, COLOR_GREEN);   
    init_pair(CMAGENTA | _WHITE, COLOR_MAGENTA, COLOR_WHITE);    
    init_pair(CMAGENTA | _GREEN, COLOR_MAGENTA, COLOR_GREEN);    
    init_pair(CMAGENTA | _CYAN, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(CMAGENTA | _BROWN, COLOR_MAGENTA, COLOR_YELLOW);    
    init_pair(CMAGENTA | _GRAY, COLOR_MAGENTA, COLOR_WHITE);    
    // Brown
    init_pair(CBROWN | _BLUE, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CBROWN | _RED, COLOR_YELLOW, COLOR_GREEN);   
    init_pair(CBROWN | _WHITE, COLOR_YELLOW, COLOR_WHITE);    
    init_pair(CBROWN | _GREEN, COLOR_YELLOW, COLOR_GREEN);    
    init_pair(CBROWN | _CYAN, COLOR_YELLOW, COLOR_CYAN);
    init_pair(CBROWN | _MAGENTA, COLOR_YELLOW, COLOR_MAGENTA);    
    init_pair(CBROWN | _GRAY, COLOR_YELLOW, COLOR_WHITE);    
    // Gray
    init_pair(CGRAY | _BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(CGRAY | _RED, COLOR_WHITE, COLOR_GREEN);   
    init_pair(CGRAY | _WHITE, COLOR_WHITE, COLOR_WHITE);    
    init_pair(CGRAY | _GREEN, COLOR_WHITE, COLOR_GREEN);    
    init_pair(CGRAY | _CYAN, COLOR_WHITE, COLOR_CYAN);
    init_pair(CGRAY | _MAGENTA, COLOR_WHITE, COLOR_MAGENTA);    
    init_pair(CGRAY | _BROWN, COLOR_WHITE, COLOR_YELLOW);    
    // Gray
    init_pair(CDGRAY | _BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(CDGRAY | _RED, COLOR_WHITE, COLOR_GREEN);   
    init_pair(CDGRAY | _WHITE, COLOR_WHITE, COLOR_WHITE);    
    init_pair(CDGRAY | _GREEN, COLOR_WHITE, COLOR_GREEN);    
    init_pair(CDGRAY | _CYAN, COLOR_WHITE, COLOR_CYAN);
    init_pair(CDGRAY | _MAGENTA, COLOR_WHITE, COLOR_MAGENTA);    
    init_pair(CDGRAY | _BROWN, COLOR_WHITE, COLOR_YELLOW);    

    // LBlue   
    init_pair(CLBLUE | _GREEN, COLOR_BLUE, COLOR_GREEN);    
    init_pair(CLBLUE | _RED, COLOR_BLUE, COLOR_RED);    
    init_pair(CLBLUE | _WHITE, COLOR_BLUE, COLOR_WHITE);    
    init_pair(CLBLUE | _CYAN, COLOR_BLUE, COLOR_CYAN);    
    init_pair(CLBLUE | _MAGENTA, COLOR_BLUE, COLOR_MAGENTA);    
    init_pair(CLBLUE | _BROWN, COLOR_BLUE, COLOR_YELLOW);
    init_pair(CLBLUE | _GRAY, COLOR_BLUE, COLOR_WHITE);
    // LGreen
    init_pair(CLGREEN | _BLUE, COLOR_GREEN, COLOR_BLUE);
    init_pair(CLGREEN | _RED, COLOR_GREEN, COLOR_GREEN);   
    init_pair(CLGREEN | _WHITE, COLOR_GREEN, COLOR_WHITE);    
    init_pair(CLGREEN | _CYAN, COLOR_GREEN, COLOR_CYAN);    
    init_pair(CLGREEN | _MAGENTA, COLOR_GREEN, COLOR_MAGENTA);
    init_pair(CLGREEN | _BROWN, COLOR_GREEN, COLOR_YELLOW);    
    init_pair(CLGREEN | _GRAY, COLOR_GREEN, COLOR_WHITE);    
    // LCyan
    init_pair(CLCYAN | _BLUE, COLOR_CYAN, COLOR_BLUE);
    init_pair(CLCYAN | _RED, COLOR_CYAN, COLOR_GREEN);   
    init_pair(CLCYAN | _WHITE, COLOR_CYAN, COLOR_WHITE);    
    init_pair(CLCYAN | _GREEN, COLOR_CYAN, COLOR_GREEN);    
    init_pair(CLCYAN | _MAGENTA, COLOR_CYAN, COLOR_MAGENTA);
    init_pair(CLCYAN | _BROWN, COLOR_CYAN, COLOR_YELLOW);    
    init_pair(CLCYAN | _GRAY, COLOR_CYAN, COLOR_WHITE);    
    // LRed
    init_pair(CLRED | _BLUE, COLOR_RED, COLOR_BLUE);
    init_pair(CLRED | _GREEN, COLOR_RED, COLOR_GREEN);   
    init_pair(CLRED | _WHITE, COLOR_RED, COLOR_WHITE);    
    init_pair(CLRED | _CYAN, COLOR_RED, COLOR_CYAN);    
    init_pair(CLRED | _MAGENTA, COLOR_RED, COLOR_MAGENTA);
    init_pair(CLRED | _BROWN, COLOR_RED, COLOR_YELLOW);    
    init_pair(CLRED | _GRAY, COLOR_RED, COLOR_WHITE);    
    // LMagenta
    init_pair(CLMAGENTA | _BLUE, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(CLMAGENTA | _RED, COLOR_MAGENTA, COLOR_GREEN);   
    init_pair(CLMAGENTA | _WHITE, COLOR_MAGENTA, COLOR_WHITE);    
    init_pair(CLMAGENTA | _GREEN, COLOR_MAGENTA, COLOR_GREEN);    
    init_pair(CLMAGENTA | _CYAN, COLOR_MAGENTA, COLOR_CYAN);
    init_pair(CLMAGENTA | _BROWN, COLOR_MAGENTA, COLOR_YELLOW);    
    init_pair(CLMAGENTA | _GRAY, COLOR_MAGENTA, COLOR_WHITE);    
    // Yellow    
    init_pair(CYELLOW | _BLUE, COLOR_YELLOW, COLOR_BLUE);
    init_pair(CYELLOW | _GREEN, COLOR_YELLOW, COLOR_GREEN);    
    init_pair(CYELLOW | _RED, COLOR_YELLOW, COLOR_RED);    
    init_pair(CYELLOW | _WHITE, COLOR_YELLOW, COLOR_WHITE);    
    init_pair(CYELLOW | _CYAN, COLOR_YELLOW, COLOR_CYAN);    
    init_pair(CYELLOW | _MAGENTA, COLOR_YELLOW, COLOR_MAGENTA);    
    init_pair(CYELLOW | _GRAY, COLOR_YELLOW, COLOR_WHITE);
    // White
    init_pair(CWHITE | _BLUE, COLOR_WHITE, COLOR_BLUE);
    init_pair(CWHITE | _RED, COLOR_WHITE, COLOR_GREEN);   
    init_pair(CWHITE | _WHITE, COLOR_WHITE, COLOR_WHITE);    
    init_pair(CWHITE | _GREEN, COLOR_WHITE, COLOR_GREEN);    
    init_pair(CWHITE | _CYAN, COLOR_WHITE, COLOR_CYAN);
    init_pair(CWHITE | _MAGENTA, COLOR_WHITE, COLOR_MAGENTA);    
    init_pair(CWHITE | _BROWN, COLOR_WHITE, COLOR_YELLOW);    
    // Black
    init_pair(CBLACK | _BLUE, COLOR_BLACK, COLOR_BLUE);
    init_pair(CBLACK | _RED, COLOR_BLACK, COLOR_GREEN);   
    init_pair(CBLACK | _BLACK, COLOR_BLACK, COLOR_BLACK);    
    init_pair(CBLACK | _GREEN, COLOR_BLACK, COLOR_GREEN);    
    init_pair(CBLACK | _CYAN, COLOR_BLACK, COLOR_CYAN);
    init_pair(CBLACK | _MAGENTA, COLOR_BLACK, COLOR_MAGENTA);    
    init_pair(CBLACK | _BROWN, COLOR_BLACK, COLOR_YELLOW);    
    
}

int cursesAttribute(unsigned char dosAttribute)
{
  /* this doesn't support non-default background. 
   * if we want that, we need to define 64 color
   * pairs and go from there.
   */

  int        ch = 0;

  short a, b;

  pair_content(dosAttribute, &a, &b);
  
  if(!a && !b)
    ch = COLOR_PAIR(COLOR_WHITE) | A_BOLD;
  else
    ch = COLOR_PAIR(dosAttribute);

  if (dosAttribute > 8)
  {
    ch |= A_BOLD;
  }
  else if ((dosAttribute & (7 << 4))  > 8)
  {
    ch |= A_BOLD;
  }

  if (!(dosAttribute & FOREGROUND_INTENSITY))
    ch |= A_DIM;
    
  if (dosAttribute & BACKGROUND_INTENSITY)
    ch |= A_REVERSE;


  return ch;
}
