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

/*# name=Direct-video #include file
*/

#ifndef __DV_H_DEFINED
#define __DV_H_DEFINED

#include "prog.h"

cpp_begin()
 
extern int * near RowTable;
extern int * First_RowTable;
extern char far * near Vid_Display;
extern int near Vid_StatusPort;
extern int near Vid_HaveSnow;
extern int near Vid_MonoCard;
extern int near Vid_NumRows;
extern int near Vid_NumCols;
extern int near Vid_Row;
extern int near Vid_Col;
extern unsigned int near Vid_Segment;
extern int near Vid_TotChars;
extern char near Vid_Page;
extern char near Vid_Attribute;
extern char near Vid_Open;
extern char near DesqView;
extern char near Vid_Bios;

#define SCROLL_up         0x06 /* Modes for VioScroll() */
#define SCROLL_down       0x07

#define MONOMEM     MK_FP(0xb000,0x0000)
#define CGAMEM      MK_FP(0xb800,0x0000)

#define NUM_COLS    (*((unsigned int far *)MK_FP(0x0000,0x044a)))
#define BUFFER_LEN  (*((unsigned int far *)MK_FP(0x0000,0x044c)))
#define PAGE_OFS    (*((unsigned int far *)MK_FP(0x0000,0x044e)))


#define CBLACK     ((byte)0)
#define CBLUE      ((byte)1)
#define CGREEN     ((byte)2)
#define CCYAN      ((byte)3)
#define CRED       ((byte)4)
#define CMAGENTA   ((byte)5)
#define CBROWN     ((byte)6)
#define CGRAY      ((byte)7)
#define CGREY      ((byte)7)
#define CDGRAY     ((byte)8)
#define CLBLUE     ((byte)9)
#define CLGREEN    ((byte)10)
#define CLCYAN     ((byte)11)
#define CLRED      ((byte)12)
#define CLMAGENTA  ((byte)13)
#define CYELLOW    ((byte)14)
#define CWHITE     ((byte)15)

#define _BLACK    ((byte)0)
#define _BLUE     ((byte)(1 << 4))
#define _GREEN    ((byte)(2 << 4))
#define _CYAN     ((byte)(3 << 4))
#define _RED      ((byte)(4 << 4))
#define _MAGENTA  ((byte)(5 << 4))
#define _BROWN    ((byte)(6 << 4))
#define _GRAY     ((byte)(7 << 4))
#define _GREY     ((byte)(7 << 4))
#define _WHITE    ((byte)(7 << 4))

#define _BLINK    128

word _fast VidOpen(int has_snow,int desqview,int dec_rows);
int _fast VidClose(void);
int _fast VidGotoXY(int Col,int Row,int do_sync);
void _fast VidGetXY(int *Col,int *Row);
void _fast VidCls(char Attribute);
void _fast VidSetAttr(char Attribute);
char _fast VidGetAttr(void);
char _fast VidGetPage(void);
int _fast VidWhereX(void);
int _fast VidWhereY(void);
int _fast VidNumRows(void);
int _fast VidNumCols(void);
void cdecl VidPutch(int Col,int Row,char Char,char Attr);
int cdecl VidGetch(int Col,int Row);
void _fast VidBios(int use_bios);
void _fast VidHideCursor(void);

void pascal VidFlipAttr(char Attribute,int Number);
int  pascal VidGetBPage(void);
void pascal VidSync(void);
void pascal VidSyncCur(void);
void pascal VidSyncDV(void);
void pascal VidSyncDVWithForce(int Force);
int  pascal VidGetMode(void);
void pascal VidSetMode(int mode);
unsigned int pascal VidGetBuffer(int is_mono);
int  pascal VidSetPage(int Pg);
void pascal VidCleol(void);
int  pascal _VidGetNumRows(void);
void pascal _VidGetXYB(int *Col,int *Row);
void pascal VidScroll(char Direction,char NumOfLines,char Attribute,char LeftCol,char TopRow,char RightCol,char BotRow);


/* These two MUST remain CDECL, or else we'll have to change our function ptrs */
void cdecl VidPuts(char *Text);
void cdecl VidPutc(char ch);

void far pascal RegScrollUp(int *col,int *row);
void _fast RegScroll(int lines);

cpp_end()

#endif /* __DV_H_DEFINED */

