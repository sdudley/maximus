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

/*# name=OS/2 drop-in functions for Scott's Vid...() functions
    credit=Thanks to Peter Fitzsimmons for this module.
*/

#if defined(OS_2)

  #include <string.h>
  #define INCL_NOPM
  #define INCL_VIO
  #define INCL_DOS
  #include <os2.h>
  #include <stdlib.h>
  #include <assert.h>
  #include <stdio.h>
  #include "typedefs.h"
  #include "compiler.h"
  #include "dv.h"
  #include "modem.h"

  static BYTE Vcurattr = 7;
  static USHORT Vnumcols;
  static USHORT Vnumrows;
  static USHORT curRow, curCol;

  #define lastRow  (Vnumrows-1)
  #define lastCol  (Vnumcols-1)

  void _stdc Vidfcur(void);

  static PBYTE lvb = 0L;
  static USHORT lvbLen;
  static int isHidden = FALSE;

  #pragma check_stack(off)

  word _fast VidOpen(int has_snow,int desqview,int dec_rows)
  {
    VIOMODEINFO viomi;

    NW(has_snow);
    NW(desqview);          /* keep /W3 happy*/

    fflush(stdout);


    viomi.cb = sizeof(viomi);
    VioGetMode(&viomi, 0);
    Vnumcols = viomi.col;
    Vnumrows = viomi.row;

    VioGetCurPos(&curRow, &curCol, 0);

  #if 1
      /* Status-line? */
    if (dec_rows)
    {
      Vnumrows--;

      while(curRow >= Vnumrows)
      {
        VioScrollUp(0, 0, 0xFFFF, 0xFFFF, 1, " \x7", 0);
        curRow--;
      }
    }
  #endif

    if (!lvb)
    {
#ifdef __FLAT__
      ULONG ul;

      VioGetBuf((PULONG) &ul, &lvbLen, 0);
      lvb=(PBYTE)thunk16to32(ul);
#else
      VioGetBuf((PULONG) &lvb, &lvbLen, 0);
#endif
    }

    return(1);
  }

  int _fast VidClose(void)
  {
      lvb = NULL;
      return(0);
  }

  int  _fast VidGotoXY(int Col,int Row, int sync)
  {
      if(isHidden){
          VIOCURSORINFO vct;

          VioGetCurType(&vct, 0);
          vct.attr = 0;
          isHidden = FALSE;
          VioSetCurType(&vct, 0);
      }
      curRow = min((USHORT)Row - 1, lastRow);
      curCol = min((USHORT)Col - 1, lastCol);
      if(sync)
          VioSetCurPos(curRow, curCol, 0);
      return(0);
  }

  void _stdc Vidfcur(void)
  {
      VioSetCurPos(curRow, curCol, 0);
  }

  void _fast VidCls(char Attribute)
  {
      BYTE cell[2];
      curRow = curCol = 0;
      cell[0] = ' ';
      cell[1] = Attribute;
      VioScrollUp(0, 0, -1, -1, -1, cell, 0);
      Vidfcur();
  }

  void _fast VidSetAttr(char Attribute)
  {
      Vcurattr = Attribute;
  }

  void _fast VidGetXY(int *Col,int *Row)
  {
    *Col=curCol+1;
    *Row=curRow+1;
  }

  int _fast VidWhereX(void)
  {
    return curCol+1;
  }

  int _fast VidWhereY(void)
  {
    return curRow+1;
  }

  void pascal VidCleol(void)
  {
      BYTE cell[2];
      cell[0] = ' ';
      cell[1] = Vcurattr;

      if(curRow < Vnumrows && curCol < Vnumcols)
          VioWrtNCell(cell, Vnumcols - curCol, curRow, curCol, 0);
  }

  int _fast VidNumRows(void)
  {
    return Vnumrows;
  }
  int _fast VidNumCols(void)
  {
    return Vnumcols;
  }

  char _fast VidGetAttr(void)
  {
    return Vcurattr;
  }


  void cdecl VidPutch(int Row,int Col,char Char,char Attr)
  {
      VioWrtCharStrAtt(&Char, sizeof(Char),  Row, Col, &Attr, 0);
  }

  int cdecl VidGetch(int Row,int Col)
  {
      BYTE b;
  #if 0
      USHORT l = sizeof(b);
      VioReadCharStr(&b, &l, Row, Col, 0);
  #else
      b = lvb[ (Row * Vnumcols + Col) * 2];
  #endif
      return (int)b;
  }

  void _fast VidBios(int use_bios)
  {
      use_bios = use_bios;
  }

  void _fast VidHideCursor(void)
  {
      VIOCURSORINFO vct;

      VioGetCurType(&vct, 0);
      isHidden = TRUE;
      vct.attr = 0xffff;
      VioSetCurType(&vct, 0);
  }


  void pascal _WinBlitz(word start_col,           /* offset from left side of screen.*/
                        word num_col,             /* number of cols.     */
                        char far *from_ofs,       /* data to be written  */
                        sword win_start_col,       /* add to from_ofs     */
                        word this_row)            /* physical screen row.*/
  {

        VioWrtCellStr(from_ofs + win_start_col*2,
                    num_col * 2,
                    this_row,
                    start_col,
                    0);
  }


  /* Scroll a portion of the OS/2 Vio screen */

  void pascal VidScroll(char Direction, char NumOfLines, char Attribute, char LeftCol, char TopRow, char RightCol, char BotRow)
  {
    char bCell[2];

    bCell[0]=' ';
    bCell[1]=Attribute;

    if (Direction==SCROLL_up)
      VioScrollUp(TopRow, LeftCol, BotRow, RightCol, NumOfLines, bCell, 0);
    else VioScrollDn(TopRow, LeftCol, BotRow, RightCol, NumOfLines, bCell, 0);
  }


#elif defined(NT)

  #include <string.h>
  #include <stdlib.h>
  #include <assert.h>
  #include <stdio.h>
  #include "typedefs.h"
  #include "compiler.h"
  #include "dv.h"
  #include "modem.h"

  static BYTE Vcurattr = 7;
  static USHORT Vnumcols;
  static USHORT Vnumrows;
  static USHORT curRow, curCol;

  #define lastRow  (Vnumrows-1)
  #define lastCol  (Vnumcols-1)

  void _stdc Vidfcur(void);

  static int isHidden = FALSE;
  HANDLE hStdout;


  #pragma check_stack(off)

  word _fast VidOpen(int has_snow,int desqview,int dec_rows)
  {
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    NW(has_snow); NW(desqview); NW(dec_rows);

    fflush(stdout);

    hStdout=GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hStdout, &csbi);

    Vnumcols = csbi.dwSize.X;
    Vnumrows = csbi.dwSize.Y;

    curCol = csbi.dwCursorPosition.X;
    curRow = csbi.dwCursorPosition.Y;

    return 1;
  }

  int _fast VidClose(void)
  {
      return(0);
  }

  int  _fast VidGotoXY(int Col,int Row, int sync)
  {
    if (isHidden)
    {
      CONSOLE_CURSOR_INFO cci;

      GetConsoleCursorInfo(hStdout, &cci);
      cci.bVisible=TRUE;
      SetConsoleCursorInfo(hStdout, &cci);
      isHidden=FALSE;
    }

    curRow = min((USHORT)Row - 1, lastRow);
    curCol = min((USHORT)Col - 1, lastCol);

    if (sync)
    {
      COORD c;

      c.Y=curRow;
      c.X=curCol;

      SetConsoleCursorPosition(hStdout, c);
    }

    return(0);
  }

  void _stdc Vidfcur(void)
  {
    COORD c;

    c.Y=curRow;
    c.X=curCol;

    SetConsoleCursorPosition(hStdout, c);
  }

  void _fast VidCls(char Attribute)
  {
    SMALL_RECT sr;
    CHAR_INFO ci;
    COORD c;

    ci.Char.AsciiChar=' ';
    ci.Attributes=Attribute;

    sr.Left=0;
    sr.Top=0;
    sr.Bottom=lastRow;
    sr.Right=lastCol;

    c.X=Vnumcols;
    c.Y=Vnumrows;

    ScrollConsoleScreenBuffer(hStdout, &sr, NULL, c, &ci);
  }

  void _fast VidSetAttr(char Attribute)
  {
      Vcurattr = Attribute;
  }

  void _fast VidGetXY(int *Col,int *Row)
  {
    *Col=curCol+1;
    *Row=curRow+1;
  }

  int _fast VidWhereX(void)
  {
    return curCol+1;
  }

  int _fast VidWhereY(void)
  {
    return curRow+1;
  }

  void pascal VidCleol(void)
  {
    SMALL_RECT sr;
    CHAR_INFO ci;
    COORD c;

    ci.Char.AsciiChar=' ';
    ci.Attributes=Vcurattr;

    sr.Left=curCol;
    sr.Top=curRow;
    sr.Bottom=curRow;
    sr.Right=lastCol;

    c.X=Vnumcols;
    c.Y=Vnumrows;

    if (curRow < Vnumrows && curCol < Vnumcols)
      ScrollConsoleScreenBuffer(hStdout, &sr, NULL, c, &ci);
  }

  int _fast VidNumRows(void)
  {
    return Vnumrows;
  }
  int _fast VidNumCols(void)
  {
    return Vnumcols;
  }

  char _fast VidGetAttr(void)
  {
    return Vcurattr;
  }


  void cdecl VidPutch(int Row,int Col,char Char,char Attr)
  {
    CHAR_INFO ci;
    COORD c, origc;
    SMALL_RECT sr;

    ci.Char.AsciiChar=Char;
    ci.Attributes=Attr;

    c.X=c.Y=1;
    origc.X=origc.Y=0;

    sr.Top=sr.Bottom=Row;
    sr.Left=sr.Right=Col;

    WriteConsoleOutput(hStdout, &ci, c, origc, &sr);
  }

  int cdecl VidGetch(int Row,int Col)
  {
    CHAR_INFO ci;
    COORD c, destc;
    SMALL_RECT sr;

    c.X=c.Y=1;
    destc.X=destc.Y=0;

    sr.Top=sr.Bottom=Row;
    sr.Left=sr.Right=Col;

    ReadConsoleOutput(hStdout, &ci, c, destc, &sr);

    return (ci.Char.AsciiChar | (ci.Attributes << 8));
  }

  void _fast VidBios(int use_bios)
  {
    NW(use_bios);
  }

  void _fast VidHideCursor(void)
  {
    CONSOLE_CURSOR_INFO cci;

    if (!isHidden)
    {
      GetConsoleCursorInfo(hStdout, &cci);
      cci.bVisible=FALSE;
      SetConsoleCursorInfo(hStdout, &cci);
      isHidden=TRUE;
    }
  }

#define MAX_COL 160

static CHAR_INFO aci[MAX_COL];

  void pascal _WinBlitz(word start_col,           /* offset from left side of screen.*/
                        word num_col,             /* number of cols.     */
                        char far *from_ofs,       /* data to be written  */
                        sword win_start_col,       /* add to from_ofs     */
                        word this_row)            /* physical screen row.*/
  {
    COORD c, origc;
    SMALL_RECT sr;
    PCHAR_INFO pci;
    char *from;

    c.X=num_col;
    c.Y=1;

    origc.X=origc.Y=0;

    sr.Top=this_row;
    sr.Bottom=this_row;
    sr.Left=start_col;
    sr.Right=start_col+num_col - 1;

    from = from_ofs + win_start_col*2;
    pci = aci;

    /* Adjust pointers so that we can use the faster preincrement/decrement
     * instead of the postinc/dec.
     */

    if (num_col > MAX_COL)
      num_col =  MAX_COL;

    --from;
    --pci;

    do
    {
      (++pci)->Char.AsciiChar=*++from;
      pci->Attributes=*++from;
    }
    while (--num_col);

    WriteConsoleOutput(hStdout, aci, c, origc, &sr);
  }


  /* Scroll a portion of the OS/2 Vio screen */

  void pascal VidScroll(char Direction, char NumOfLines, char Attribute,
                        char LeftCol, char TopRow, char RightCol, char BotRow)
  {
    CHAR_INFO ci;
    COORD cDest;
    SMALL_RECT srScroll, srClip;

    ci.Char.AsciiChar=' ';
    ci.Attributes=Attribute;

    srScroll.Left=LeftCol;
    srScroll.Right=RightCol;

    srClip.Left=srClip.Top=0;
    srClip.Right=lastCol;
    srClip.Bottom=lastRow;

    if (Direction==SCROLL_up)
    {
      srScroll.Top=TopRow+NumOfLines;
      srScroll.Bottom=BotRow;
      cDest.X=0;
      cDest.Y=TopRow;
    }
    else
    {
      srScroll.Top=TopRow;
      srScroll.Bottom=BotRow-NumOfLines;
      cDest.X=0;
      cDest.Y=TopRow+NumOfLines;
    }

    ScrollConsoleScreenBuffer(hStdout, &srScroll, NULL, cDest, &ci);
  }
}

#elif defined(UNIX)

/* curses replacements for scott's direct video (dv*.*) by wes */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include "typedefs.h"
#include "compiler.h"
#include "dv.h"
#include "modem.h"
#include <curses.h>
#include "viocurses.h"
 
static BYTE Vcurattr = 7;
static USHORT Vnumcols;
static USHORT Vnumrows;
static USHORT curRow, curCol;

#define lastRow  (Vnumrows-1)
#define lastCol  (Vnumcols-1)

void setsize()
{
  if (!stdscr)
  {
    Vnumcols = atoi(getenv("COLS") ? : "80");
    Vnumrows = atoi(getenv("ROWS") ? : "25");
  }
  else
  {
    getmaxyx(stdscr, Vnumrows, Vnumcols); 
    getyx(stdscr, curRow, curCol);
    setscrreg(0, lastRow);
  }
}

void resize(int sig)
{
  if (stdscr)
  {
    /* SIGWINCH -- not sure how well this will work.. */
    setsize();
#ifdef NCURSES_VERSION
    wresize(stdscr, Vnumrows, Vnumcols);
#endif
    refresh();
  }
}

word VidOpen(int has_snow,int desqview,int dec_rows)
{
  if (!stdscr)
  {
    fflush(stdout);

    if (!getenv("TERM"))
      putenv("TERM=vt100");

    initscr();             	/* init curses */
    start_color();
    printf("\033(U");
    InitPairs();

/* If enabled wouldn't our keytable work */

#if 0
    keypad(stdscr, TRUE);	/* enable keyboard mapping */
#endif
    cbreak();			/* char-by-char instead of line-mode input */    
    nodelay(stdscr, TRUE);	/* Make getch() non-blocking */
    nonl();			/* No LF->CRLF mapping on output */
    noecho();			/* do not echo input */
    noqiflush();		/* Do not flush on INTR, QUIT, SUSP */

#if 0
    raw();		   	/* Do not process ^C as SIGINT, etc + cbreak() */
    timeout(0);			/* set getch timeout */
#endif

    scrollok(stdscr, TRUE);	/* Allow automatic scrolling */
    immedok(stdscr, TRUE);	/* call refresh with many curses functions (slow) */
    atexit((void *)VidClose);	/* Try to reset the screen to a usable state on exit */
  }

  setsize();
  signal(SIGWINCH, resize);	/* Hope and pray */
  return 1;
}

int VidClose(void)
{
  if (stdscr)
    endwin();            /* destroy curses instance */

  stdscr = NULL;
  printf("\033(B");
  return 0;
}

int VidGotoXY(int Col,int Row, int sync)
{
  curRow = min((USHORT)Row - 1, lastRow);
  curCol = min((USHORT)Col - 1, lastCol);
  if(sync)
    VioSetCurPos(curRow, curCol, 0);
      return(0);
}

int VidNumRows(void)
{
  return Vnumrows;
}

int VidNumCols(void)
{
  return Vnumcols;
}
 
char VidGetAttr(void)
{
  return Vcurattr;
}

void VidBios(int use_bios)
{
  return;
}

void VidCls(char Attribute)
{
  curRow = curCol = 0;
  if (stdscr)
  {
    clear();
    refresh();
  }
}

void _fast VidGetXY(int *Col,int *Row)
{
  *Col=curCol+1;
  *Row=curRow+1;
}
 
int VidWhereX(void)
{
  return curCol+1;
}
 
int VidWhereY(void)
{
  return curRow+1;
}

void VidHideCursor(void)
{
/*  if (stdscr)
    curs_set(0);*/
}

void _fast VidSetAttr(char Attribute)
{
  Vcurattr = Attribute;
}

int VidGetch(int Row,int Col)
{
   chtype c;

   if(stdscr)
   {
	c = mvinch(Col, Row) & A_CHARTEXT;
	return c;
   }
   else
	return 0;
}

void VidPutch(int Row, int Col, char Char, char Attr)
{
  chtype ch = (unsigned char)Char | (Attr & FOREGROUND_INTENSITY ? A_NORMAL : A_DIM);
  VidSetAttr(Attr);

#ifdef MANUAL_SCROLL
  if ((Row == lastRow) && (Col == lastCol))
  {
    scrl(1);
    Row--;
  }
#endif

  if (stdscr)
  {
    mvaddch(Row, Col, ch);
    refresh();
  }
}

typedef unsigned char CHAR_INFO, *PCHAR_INFO;

void pascal _WinBlitz(word start_col,           /* offset from left side of screen.*/
                      word num_col,             /* number of cols.     */
                      char far *from_ofs,       /* data to be written  */
                      sword win_start_col,       /* add to from_ofs     */
                      word this_row)            /* physical screen row.*/
{
/*    mvaddstr(this_row, start_col, from_ofs + win_start_col * 2); */

  /* We're putting out tonnes of garbage, mostly control-Gs - attr 7? I think
   * this is no ordinary char *buffer, it's a buffer full of 16-bit words, with
   * the high byte being an attribute and the low byte being the character
   * we're interested in.   
   *
   * The Windows NT version of this function suggests we're actually creating
   * a one-line rectangle. We don't need to be that fancy, let's just create
   * a curses chtype buffer, we can fill in the attributes later.
   */

  chtype        chbuf[num_col];
  int           i;
  int  		ch, attr, tmpattr;
  unsigned char *start;
#ifdef MANUAL_SCROLL
  int		newlineCount = 0;
#endif

  start = (unsigned char *)(from_ofs + (win_start_col * 2));

  for (i = 0; i < num_col; i++)
  {
#ifdef BIG_ENDIAN
    attr = start[i * 2];
    ch = start[(i * 2) + 1];
#else
    ch = start[i * 2];
    attr = start[(i * 2) + 1];
#endif

#ifdef MANUAL_SCROLL
    if (ch == '\n')
      newlineCount++;
#endif
    tmpattr = cursesAttribute(attr);
    chbuf[i] = ch | (COLOR_PAIR(tmpattr) | A_BOLD);
  }

#ifdef DEBUG_WINBLITZ
  {
    char buf[num_col + 1];

    for (i = 0; i < num_col; i++)
      buf[i] = ((start[i * 2] == '\n') ? ' ' : start[i * 2]);

    buf[i] = (char)0;
  }
#endif

#ifdef MANUAL_SCROLL
  /* Wrap around */
  if ((start_col + num_col) > lastCol)
    newlineCount += (start_col + num_col) / lastCol;
#endif

#ifdef MANUAL_SCROLL
  if ((this_row + newlineCount) > lastRow)
  {
    /* maybe we need to manually scroll the screen? it's 
     * certainly not scrolling by itself.
     */

    scrl(newlineCount);
    if (newlineCount < this_row)
      this_row -= newlineCount;
    else
      this_row = 0;
  }
#endif

  if (stdscr)
  {
    move(this_row, start_col);
    addchnstr(chbuf, num_col);
    refresh();
  }

  return;
}


#endif

				    

void pascal VidSyncDVWithForce(int fForce)
{
#ifndef UNIX
    // not implemented
    NW(fForce);
#else
  if (stdscr)
    refresh();
#endif
}

void pascal VidSyncDV(void)
{
  if (stdscr)
    refresh();
}

