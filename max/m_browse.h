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

/* Linked list structure for updating lastread pointers after a QWK         *
 * download.                                                                */

struct _lrptr
{
  struct _lrptr *next;

  char *path;             /* Pathname of area */
  word type;              /* This area's type (*.msg, squish, etc) */

  dword msgn;              /* The message number to set to */
};

extern struct _lrptr *lrptr;


int Read_Begin(BROWSE *b);
void Read_First(BROWSE *b);
int Read_Display(BROWSE *b);
int Read_After(BROWSE *b);
int Read_End(BROWSE *b);

int QWK_Begin(BROWSE *b);
int QWK_Status(BROWSE *b,char *aname,int colour);
int QWK_Idle(BROWSE *b);
int QWK_Display(BROWSE *b);
int QWK_After(BROWSE *b);
int QWK_End(BROWSE *b);

int List_Begin(BROWSE *b);
int List_Status(BROWSE *b,char *aname,int colour);
int List_Idle(BROWSE *b);
void Rev_Up(void);
int List_Display(BROWSE *b);
int List_After(BROWSE *b);
int List_End(BROWSE *b);

void Rev_Up(void);

void Lmsg_Free(void);
void Lmsg_Set(BROWSE *b, long msgn);
void Lmsg_Update(BROWSE *b);

int OkToFixLastread(BROWSE *b);
int Browse_Scan_Areas(BROWSE *b);

void Browse_Switch_Back(void);
void Init_Search(SEARCH *s);
int Match_All(BROWSE *b);



