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

int Exec_Xtern(int type, struct _opt *thisopt, char *arg, char **result, char *mname);
int Exec_Main(int type, char **result);
int Exec_Msg(int type, char **result, int key, char *arg, char *menuname);
int Exec_File(int type, char **result, char *menuname);
int Exec_Change(int type, char **result);
int Exec_Edit(int type, char **result, XMSG *msg, unsigned *puiFlag);
int Exec_Chat(int type, char **result);
