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

void BackSpace(void);
void Delete_Char(void);
void Delete_Line(int cx);
void Delete_Word(void)   /*ABK 1990-09-02 14:25:34 */;
void EdMemOvfl(void);
void Add_Character(int ch);
void New_Line(int col);
word Carriage_Return(int hard);
int Insert_Line_Before(int cx);
void MagnEt_Help(void);
void MagnEt_Menu(void);
void Piggy(void);
void MagnEt_Bad_Keystroke(void);
void Cursor_Left(void);
void Cursor_Right(void);
void Cursor_Up(void);
void Cursor_Down(int update_pos);
void Cursor_BeginLine(void);
void Cursor_EndLine(void);
void Word_Left(void);
void Word_Right(void);
void Scroll_Up(int n,int location);
void Scroll_Down(int n,int location);
void Page_Up(void);
void Page_Down(void);
void Quote_OnOff(struct _replyp *pr);
void Quote_Up(void);
void Quote_Down(void);
void Quote_Copy(void);
void Read_DiskFile(void);
void Load_Message(HMSG msgh);
void Update_Line(word cx, word cy, word inc, word update_cursor);
int Word_Wrap(word mode);
void Toggle_Insert(void);
void Update_Position(void);
void NoFF_CLS(void);
void Fix_MagnEt(void);
void Do_Update(void);
int Mdm_getcwcc(void);

