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

#ifndef __WIN_H_DEFINED
#define __WIN_H_DEFINED

#include "dv.h"
#include "typedefs.h"

#define VWIN_ID   0x4e495756L

#define WIN_INHERIT   0x01  /* Inherit text which is currently "below" win  */
#define WIN_SHADOW    0x02  /* Window has a shadow                          */
#define WIN_NOCSYNC   0x04  /* Don't sync cursor when opening window        */
#define WIN_CENTRE    0x08  /* Automatically centre window on screen        */
#define WIN_CENTER    0x08  /* Automatically centre window on screen        */
#define WIN_NODRAW    0x10  /* Don't draw the opened window; wait for sync. */

#define SHADOW_NONE   0xff

#define BORDER_NONE   0
#define BORDER_SINGLE 1
#define BORDER_DOUBLE 2
#define BORDER_HSVD   3
#define BORDER_HDVS   4

#define TITLE_LEFT    0
#define TITLE_MIDDLE  1
#define TITLE_RIGHT   2

#define WFLAG_NOCUR   0x01    /* Cursor hidden in this window */

typedef struct _vwin
{
  struct _vwin *next;

  long id;
  long ctr;           /* Counter for this window number */
  
  byte far **rowtable;
  byte far **orig_rt;

  int *dleft;
  int *orig_dl;

  int *dright;
  int *orig_dr;

  byte *blitz;
  byte *orig_blitz;

  char far *buf;

  int row;
  int col;
  byte attr;
  byte battr;
  unsigned int border;
  unsigned int bdirty;

  unsigned int s_col;
  unsigned int s_row;
  unsigned int b_col;
  unsigned int b_row;
  unsigned int s_width;
  unsigned int s_height;
  unsigned int b_width;
  unsigned int b_height;

  sbyte avtstate;
  byte rsvd1;
  byte lockback;
  byte shadattr;
  unsigned int flag;
} VWIN;


typedef struct _pickopt
{
  char *name;
  int value;
} PLIST;



#define PICK_UP         -1
#define PICK_DOWN       -2
#define PICK_SELECT     -3
#define PICK_ABORT      -4

typedef struct _vpicklist
{
  VWIN *win;

  int row;
  int col;

  int height;
  int width;

  PLIST *items;

  int it_current;
  int it_last;
  int it_top;

  int col_item;
  int col_select;

} VPICK;

#ifdef NT
  #define Cell(attr, ch) ((dword)(byte)ch | ((dword)attr << 16Lu))
  #define CharOf(cell) ((byte)cell)
  #define AttrOf(cell) ((byte)(cell >> 16))

  typedef unsigned long WNCELL;
  #define CELLSHIFT 2 /* log2(sizeof(WNCELL) */
#else
  #define Cell(attr, ch) ((word)(byte)ch | ((word)(byte)attr << 8));
  #define CharOf(cell) ((byte)cell)
  #define AttrOf(cell) ((byte)(cell >> 8))

  typedef unsigned short WNCELL;
  #define CELLSHIFT 1 /* log2(sizeof(WNCELL) */
#endif


#define WinOfs(win,row,col) ((WNCELL far *)(win->rowtable[(row)]+((col) << CELLSHIFT)))
#define WinGetAttr(win) (win->attr)


void _fast WinPutch(VWIN *win,int Col,int Row,byte Char,byte Attr);
void _fast WinVline(VWIN *win,int scol,int srow,int erow,int border,byte attr);
void _fast WinHline(VWIN *win,int scol,int srow,int ecol,int border,byte attr);
void _fast WinBox(VWIN *win,int scol,int srow,int ecol,int erow,int border,byte attr);
void _fast WinGotoXY(VWIN *win,int row,int col,int do_sync);
void _fast WinCls(VWIN *win,byte attr);
void _fast WinSync(VWIN *win,int sync_cursor);
VWIN * _fast WinOpen(int col,int row,int width,int height,int border,int attr,int battr,int flag);
void _fast WinClose(VWIN * win);
void _fast WinScrl(VWIN *win,int dir,int srow,int erow,int attr);
void _fast WinPutc(VWIN *win,byte Char);
void _fast WinCleol(VWIN *win,int Col,int Row,byte Attr);
void _fast WinPuts(VWIN *win,char *s);
void _fast WinDirtyAll(VWIN *win);
void _fast WinSetDirty(VWIN *win,int Row,int Col);
void _fast WinUpdateUnder(VWIN *w);
void _fast WinApiOpen(int save);
void _fast WinApiClose(void);
void _fast WinSetAttr(VWIN *win,byte Attr);
int _fast WinWhereX(VWIN *win);
int _fast WinWhereY(VWIN *win);
VWIN * cdecl WinMsg(int border,int attr,int battr,...);
void _fast WinCenter(VWIN *win,int Row,char *s);
void _fast WinPutstr(VWIN *win,int Row,int Col,char *s);
void _fast WinToTop(VWIN *win);
void _fast WinToBottom(VWIN *win);
void _fast WinPutsA(VWIN *win,char *s);
void _fast WinPutcA(VWIN *win,byte ch);
void _fast WinSyncAll(void);
int _fast WinTitle(VWIN *win,char *title,int location);
void _fast WinPutstra(VWIN *win,int Row,int Col,int attr,char *s);
VPICK * _fast WinCreatePickList(VWIN *win,int row,int col,int height,int col_item,int col_select,PLIST *picklist,int it_current);
int _fast WinPickAction(VPICK *vp,int action);
int _fast WinClosePickList(VPICK *vp);
void pascal _WinBlitz(word start_col, word num_col, char far *from_ofs, sword win_start_col, word this_row);
void _fast WinCursorHide(VWIN *win);
void _fast WinCursorShow(VWIN *win);
void _fast WinShadow(VWIN *win, byte attr);
void _stdc WinPrintf(VWIN *win, char *format, ...);

#define WinGetRow(win) WinWhereX(win)
#define WinGetCol(win) WinWhereY(win)
#define WinGetNumRows(win)  ((win)->s_height)
#define WinGetNumCols(win)  ((win)->s_width)

extern VWIN *wscrn;

#endif

