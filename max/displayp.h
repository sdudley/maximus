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

#include "display.h"

static void near DispReadline(DSTK *d, char *str, sword limit);
static DSTK * near DispNewDstk(char *fname);
static void DisplayCleanup(DSTK *d);
static void near DisplayInitDstk(DSTK *d, byte *nonstop);
static sword near DisplayOneFile(DSTK *d);
static sword near DisplayOpenFile(DSTK *d);
static void near DisplayFixFilename(DSTK *d);
static sword near DispCloseFiles(DSTK *d, sword ret);
static sword near DisplayNormal(DSTK *d);
static sword near DisplayHandleHotkey(DSTK *d);
static word near ProcessFBBSLine(DSTK *d, word ch);
word near DCNul(DSTK *d);
word near DCEnter(DSTK *d);
word near DCCKOff(DSTK *d);
word near DCCKOn(DSTK *d);
word near DCMore(DSTK *d);
word near DCMoreOn(DSTK *d);
word near DCParseData(DSTK *d);
word near DCBell(DSTK *d);
word near DCMoreOff(DSTK *d);
word near DCQuest(DSTK *d);
word near DCPriv(DSTK *d);
word near DCAvatar(DSTK *d);
word near DCMaximus(DSTK *d);
word near DCRLE(DSTK *d);
word near DCZ(DSTK *d);
word near DCCR(DSTK *d);
word near DCLF(DSTK *d);
word near DCNormal(DSTK *d, word ch);
word near DCCls(DSTK *d);
word near DC8(DSTK *d);
word near DC9(DSTK *d);
word near DCe(DSTK *d);
word near DC11(DSTK *d);
word near DC12(DSTK *d);
word near DC13(DSTK *d);
word near DC14(DSTK *d);
word near DC15(DSTK *d);
word near DC18(DSTK *d);



static word (near *dispfn[])(DSTK *d)=
{
  /* 0*/  DCNul,
  /*01*/  DCEnter,
  /*02*/  DCCKOff,
  /*03*/  DCCKOn,
  /*04*/  DCMore,
  /*05*/  DCMoreOn,
  /*06*/  DCParseData,
  /*07*/  DCBell,
  /*08*/  DC8,
  /*09*/  DC9,
  /*0a*/  DCLF,
  /*0b*/  DCMoreOff,
  /*0c*/  DCCls,
  /*0d*/  DCCR,
  /*0e*/  DCe,
  /*0f*/  DCQuest,
  /*10*/  DCPriv,
  /*11*/  DC11,
  /*12*/  DC12,
  /*13*/  DC13,
  /*14*/  DC14,
  /*15*/  DC15,
  /*16*/  DCAvatar,
  /*17*/  DCMaximus,
  /*18*/  DC18,
  /*19*/  DCRLE,
  /*1a*/  DCZ,
  /*1b*/  DCNul
};



/* Top of 'display file' variable pseudo-stack */

static DSTK *dtop=NULL;

/* Count of nested Display_File invocations */

static sword nested=0;


#if defined(__WATCOMC__)
#pragma check_stack(on)
#endif
