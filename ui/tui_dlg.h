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

extern word dlg_ok;

#define BUTTON_STD    (CWHITE | _BLUE)      /* Colour for dialog buttons    */
#define BUTTON_SEL    (CYELLOW | _BLUE)     /* Colour for quick select chr  */
#define BUTTON_HIGH   (CWHITE  | _BLACK)    /* Colour for selected button   */

#define STR_NORMAL    (CYELLOW | _BLUE)
#define STR_SEL       (CWHITE  | _BLACK)
#define STR_LEFTRIGHT (CWHITE  | _BLUE)

#define RAD_STD       (CBLACK | _GRAY)
#define RAD_SEL       (CRED   | _GRAY)
#define RAD_HIGH      (CWHITE | _BLACK)

#define CHK_STD       (CBLACK | _GRAY)
#define CHK_SEL       (CRED   | _GRAY)
#define CHK_HIGH      (CWHITE | _BLACK)

#define DLG_NEW       0x1234                /* For the opt->appdata stuff   */
#define DLG_EDIT      0x4321                /* When editing this field      */

#define MAX_NUM       120                   /* Max length of a number field */

word DlgBefore(HVMENU menu);
word DlgAfter(HVMENU menu);

MenuFunction(DlgButAct);
MenuFunction(DlgButOk);
MenuFunction(DlgButCan);
MenuFunction(DlgButShow);
MenuFunction(DlgButReg);

MenuFunction(DlgStrShow);
MenuFunction(DlgStrEdit);
MenuFunction(DlgStrReg);

MenuFunction(DlgIntShow);
MenuFunction(DlgIntEdit);
MenuFunction(DlgIntReg);

MenuFunction(DlgRadAct);
MenuFunction(DlgRadShow);
MenuFunction(DlgRadReg);

MenuFunction(DlgChkAct);
MenuFunction(DlgChkShow);
MenuFunction(DlgChkReg);

MenuFunction(DlgInfReg);

MenuFunction(DlgAddrShow);
MenuFunction(DlgAddrEdit);
MenuFunction(DlgAddrReg);

MenuFunction(DlgValShow);
MenuFunction(DlgValAct);
MenuFunction(DlgValReg);


#define STD_DIALOG(name, sx, sy, bor, type, xwid, ywid) \
    struct _vmenu (name)[]=                                         \
    {                                                               \
      sx, sy, MENU_COLS, bor,                                       \
      (type) | MENU_DIALOG | MENU_HOT | MENU_HOT_ONLY,              \
      xwid, ywid, DlgBefore, DlgAfter, NULL, {

#define TIT_DIALOG(name, sx, sy, bor, type, xwid, ywid, tit) \
    struct _vmenu (name)[]=                                         \
    {                                                               \
      sx, sy, MENU_COLS, bor,                                       \
      (type) | MENU_DIALOG | MENU_HOT | MENU_HOT_ONLY,              \
      xwid, ywid, DlgBefore, DlgAfter, tit, {

#define DEF_DIALOG(name, sx, sy, cw, cb, ci, cs, ch, csh, bor, type, xwid, ywid) \
    struct _vmenu (name)[]=                                         \
    {                                                               \
      sx, sy, cw, cb, ci, cs, ch, csh, bor,                         \
      (type) | MENU_DIALOG | MENU_HOT | MENU_HOT_ONLY,              \
      xwid, ywid, DlgBefore, DlgAfter, NULL, {

#define DLG_STR(name, x, y, dt)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgStrShow, DlgStrEdit, DlgStrReg},
#define DLG_INT(name, x, y, dt)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg},
#define DLG_INV(name, x, y, dt, v)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, v},
#define DLG_INV2(name, x, y, dt, v, p)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, v, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p},
#define DLG_BYT(name, x, y, dt)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AF_BYTE},
#define DLG_LNG(name, x, y, dt)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AF_LONG},
#define DLG_LNV(name, x, y, dt, v)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, v, 0, 0, 0, 0, 0, 0, 0, 0, 0, AF_LONG},
#define DLG_LNV2(name, x, y, dt, v, p)    {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgIntShow, DlgIntEdit, DlgIntReg, v, 0, 0, 0, 0, 0, 0, 0, 0, 0, AF_LONG, 0, 0, p},
#define DLG_ADDR(name, x, y, dt)    {name, NULL,     dt, NULL, NULL, NULL, x, y, DlgAddrShow, DlgAddrEdit, DlgAddrReg},
#define DLG_VSW(name, x, y, dt, vs) {name, DlgValAct,dt, NULL, NULL, NULL, x, y, DlgValShow, NULL,       DlgValReg, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, vs},
#define DLG_VSB(name, x, y, dt, vs) {name, DlgValAct,dt, NULL, NULL, NULL, x, y, DlgValShow, NULL,       DlgValReg, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, AF_BYTE, 0, vs},
#define DLG_BAUD(name, x, y, dt)    {name, DlgBaudAct,     dt, NULL, NULL, NULL, x, y, DlgIntShow, NULL, DlgBaudReg},
#define DLG_PRIV(name, x, y, dt)    {name, DlgPrivAct,     dt, NULL, NULL, NULL, x, y, DlgPrivShow, NULL, DlgPrivReg},
#define DLG_STV(name, x, y, dt, v) {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgStrShow, DlgStrEdit, DlgStrReg, v},
#define DLG_STV2(name, x, y, dt, v, p) {name, NULL,      dt, NULL, NULL, NULL, x, y, DlgStrShow, DlgStrEdit, DlgStrReg, v, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p},
#define DLG_VAL(name, x, y, dt)    {name, DlgStrEdit,dt, NULL, NULL, NULL, x, y, DlgStrShow, DlgStrEdit, DlgStrReg},
#define DLG_BUF(name, x, y, dt, bef) {name, DlgButAct, dt, NULL, bef, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_BUT(name, x, y, dt) {name, DlgButAct, dt, NULL, NULL, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_OK(name, x, y)  {name, DlgButOk,  NULL, NULL, NULL, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_BUA(name, x, y, a) {name, a, NULL, NULL, NULL, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_OKF(name, x, y)  {name, f,  NULL, NULL, NULL, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_CAN(name, x, y) {name, DlgButCan, NULL, NULL, NULL, NULL, x, y, DlgButShow, NULL, DlgButReg},
#define DLG_INF(name, x, y)     {name, NULL,    NULL, NULL, NULL, NULL, x, y, NULL, NULL, DlgInfReg},
#define DLG_RAD(name, x, y, dt) {name, DlgRadAct, dt, NULL, NULL, NULL, x, y, DlgRadShow, NULL, DlgRadReg},
#define DLG_CHK(name, x, y, dt) {name, DlgChkAct, dt, NULL, NULL, NULL, x, y, DlgChkShow, NULL, DlgChkReg},

#define END_DIALOG  {NULL}}};

