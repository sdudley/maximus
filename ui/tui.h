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

#ifndef __TUI_H_DEFINED
#define __TUI_H_DEFINED

#include "prog.h"
#include "keys.h"
#include "win.h"

#ifdef __FARDATA__
#define MENUFAR huge
#else
#define MENUFAR
#endif

#define HOT_NONE      0x00  /* No action on hotspot                         */
#define HOT_PRESS1    0x01  /* Action on left button press                  */
#define HOT_RELEASE1  0x02  /* Action on left button release                */
#define HOT_PRESS2    0x04  /* Action on right button press                 */
#define HOT_RELEASE2  0x08  /* Action on right button release               */
#define HOT_ANYWHERE  0x80  /* Option works anywhere on-screen              */


#define MENU_VERT     0x0001  /* Horizontal menu                            */
#define MENU_HORIZ    0x0002  /* Vertical menu                              */
#define MENU_DIALOG   0x0004  /* Dialog box                                 */
#define MENU_INHERIT  0x0008  /* Inherit keys from parent menu option       */
#define MENU_PLACE    0x0010  /* Automatically place window on screen       *
                               * relative to parent option.                 */
#define MENU_HOT      0x0020  /* Cmds can be activated by alt-keys too      */
#define MENU_HOT_ONLY 0x0040  /* Cmds CAN'T be activated with just letter   */
#define MENU_NHOT     0x0080  /* Cmds can be activated by both alts and norm*/
#define MENU_SHADOW   0x0100  /* Menu has a shadow */
#define MENU__DROP    0x8000  /* Delete this menu from menustk (internal)   */


#define MAX_VOPT          32
#define MAX_OPTNAME_LEN   256

#define VKEY_UP     (K_UP << 8)
#define VKEY_DOWN   (K_DOWN << 8)
#define VKEY_RIGHT  (K_RIGHT << 8)
#define VKEY_LEFT   (K_LEFT << 8)
#define VKEY_STAB   (K_STAB << 8)
#define VKEY_INS    (K_INS << 8)
#define VKEY_DEL    (K_DEL << 8)
#define VKEY_HOME   (K_HOME << 8)
#define VKEY_END    (K_END << 8)

#define MenuFunction(f) sword f(HVOPT opt)
#define NWMenu()    NW(opt); return 0;

/* td 2.0
#define MENU_COLS CBLACK | _GRAY, CBLACK | _GRAY, \
                  CBLACK | _GRAY, CBLACK | _GREEN,\
                  CRED | _GRAY, CRED | _GREEN
*/

#define MENU_COLS CBLACK | _GRAY, CBLACK | _GRAY, \
                  CBLACK | _GRAY, CWHITE | _BLACK,\
                  CRED | _GRAY, CWHITE | _BLACK


#define STD_MENU(name, sx, sy, bor, type)  \
    struct _vmenu MENUFAR (name)[1]=                                         \
    {                                                               \
      sx, sy, MENU_COLS, bor, type, 0xffff, 0xffff,                 \
      NULL, NULL, NULL, {

#define DEF_MENU(name, sx, sy, cw, cb, ci, cs, ch, csh, bor, type)  \
    struct _vmenu MENUFAR (name)[1]=                                         \
    {                                                               \
      sx, sy, cw, cb, ci, cs, ch, csh, bor, type, 0xffff, 0xffff,   \
      NULL, NULL, NULL, {

#define MNU_ITEM(name, handler, menu)       {name, handler, NULL, menu, NULL, NULL, 0xffffu, 0xffffu},
#define STD_ITEM(name, handler)             {name, handler, NULL, NULL, NULL, NULL, 0xffffu, 0xffffu},
#define DAT_ITEM(name, handler, data)       {name, handler, data, NULL, NULL, NULL, 0xffffu, 0xffffu},

#define END_MENU    {NULL}}};

struct _vmenu;
struct _vopt;
struct _vkey;

typedef struct _vmenu *HVMENU;
typedef struct _vopt *HVOPT;
typedef struct _vkey *HVKEY;

#define VKF_MENUKEYS      0x01    /* Start of menukeylist (this key record  *
                                   * and beyond are shared by all menu      *
                                   * options on this menu.                  */
#define VKF_AFTER         0x02    /* This menu key is part of the current   *
                                   * menu.  It is not inherited!            */


/* A value list - for handling pop-up menus containing lists of values */

struct _val_list
{
  char *name;
  word value;
};


/* key list - contains actions to perform when a particular key is pressed  *
 * (or for a particular mouse event)                                        */

struct _vkey
{
  HVKEY next;                     /* Pointer to next key in linked list     */

  word key;                       /* Key to invoke option.  Scan code is in *
                                   * the high byte, ascii code in the low.  */
  word flag;                      /* Flags for this key.  See VKF_XXX.      */
  
  word hot_flag;                  /* Flags for mouse event                  */
  
  MenuFunction((*menufn));        /* Function to call when selected         */

  HVOPT newopt;                   /* Option to transfer to when selected    */
  HVMENU newmenu;                 /* Menu to transfer to when selected      */
};

/* appflag values */

#define AF_BYTE           0x01    /* Menu option has a byte type (DlgIntReg)*/
#define AF_LONG           0x02    /* Menu option has a long type (DlgIntReg)*/

/* option - This is the data recorded for each option on a menu */

struct _vopt
{
  char *name;                   /* Name of menu option                      */
  MenuFunction((*menufn));      /* Default function when <enter> is presed  */
  void *data;                   /* Application-specific function data       */
  HVMENU menu;                  /* If this has a nested pull-down, ptr to it*/

  MenuFunction((*ubefore));     /* User function to call when selected      */
  MenuFunction((*uafter));      /* User function to call when deselected    */

  /**** Everything after here is optionally filled in by TuiRegisterMenu ****/

  word cx, cy;                  /* Screen location of this menu option      */
  MenuFunction((*display));     /* Function to draw this menu item          */
  MenuFunction((*unknown));     /* Called when an unknown char is received  */
  MenuFunction((*regist));      /* Function to call when registering option */

  MenuFunction((*uafter2));     /* User function to call when deselected    */

  /**** Everything after here is always filled in by TuiRegisterMenu ****/

  HVMENU parent;                /* Parent menu for this option              */
  HVKEY keys;                   /* Linked list of keys for this menu        */
  
  MenuFunction((*fbefore));     /* F() to call when this opt selected       */
  MenuFunction((*fafter));      /* F() to call when this opt deselected     */

  byte hot_col;                   /* Starting column of hotspot             */
  byte hot_row;                   /* Starting row of hotspot                */
  byte hot_n_cols;                /* Number of columns in hotspot           */
  byte hot_n_rows;                /* Number of rows in hotspot              */
  
  word appdata;                   /* Data space for menu functions          */
  word appflag;                   /* Data space for menu functions          */
  void *appdatap;                 /* Data space for menu functions          */
  struct _val_list *valdatap;     /* Pointer to the value-list for this opt */
  void *appdata2p;                /* Data space for menu functions          */
};

/* Menu.  This is the structure recorded for each individual menu */

struct _vmenu
{
  word start_x, start_y;
  byte col_win, col_bor, col_item, col_sel, col_hot, col_selhot;
  sword border;
  word type;
  word sizex, sizey;

  word (*before)(HVMENU menu);
  word (*after)(HVMENU menu);

  char *title;

  struct _vopt opt[MAX_VOPT];

  /**** Everything after here is filled in by TuiRegisterMenu ****/

  word wid;                     /* Width of largest option on menu (width)  */
  word num_opt;                 /* Number of options on menu (height)       */
  HVMENU next;
  HVMENU parent;                /* Parent of this menu                      */
  struct _vopt *curopt;         /* Pointer to current option on this menu   */
  struct _vopt *lastopt;        /* Pointer to last option on this menu      */
  VWIN *win;                    /* Window which holds this menu option      */
  
  word laststroke;              /* Last keystroke used on this menu         */
  word cx, cy;                  /* Working cursorx/cursory for menu building*/
  
  void **dlgsave;               /* Saved information for dialog box         */
  word def_opt;                 /* Default option (index for opt[])         */
};




void _fast TuiOpen(void);
void _fast TuiClose(void);

HVMENU _fast TuiRegisterMenu(HVMENU vmenu);
HVMENU _TuiRegisterMenu1(HVMENU vmenu, HVKEY keylist);;
sword _fast TuiExecMenu(HVMENU vmenu);
sword _fast TuiDestroyMenu(HVMENU vmenu);
void _fast TuiSetMenuType(word attr);
HVKEY _TuiAddEscKey(HVOPT opt);
void _TuiFindMenuStartLoc(HVMENU menu);
void _TuiFindMenuStartLoc(HVMENU menu);
void _TuiCloseOldMenu(HVMENU oldmenu, HVMENU newmenu, HVOPT newopt);
HVOPT _TuiGetNextOpt(HVMENU menu, HVOPT opt);
HVOPT _TuiGetPriorOpt(HVMENU menu, HVOPT opt);
void _stdc WinPrintf(VWIN *win, char *format, ...);
sword _TuiMenuAddKey(HVOPT opt, word kb, MenuFunction((*menufn)), HVOPT newopt, HVMENU newmenu, word hotspot, word flag);
MenuFunction(_TuiMenuOptNormal);

word _stdc WinGetYN(char *fmt, ...);
word WinInfo(char *out);
void _stdc WinErr(char *format, ...);
void _stdc WinPrintf(VWIN *win, char *format, ...);
void WinExit(int erl);
void WinFill(VWIN *win, byte ch, byte attr);
void WinAssertDir(char *path);
VWIN *WinStatus(char *pszText);
byte col(byte c);


#include "tui_dlg.h"
#include "tui_list.h"
#include "tui_mous.h"

#ifdef PM
#include "tui_pm.h"
#endif

#endif /* __TUI_H_DEFINED */
