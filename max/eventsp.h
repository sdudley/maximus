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

static void near Write_Events(int dateit);
static void near Parse_Event_File(char *ebbsname);
static int near Process_Event_Line(char *line,struct _event *e);
static void near Parse_Day(struct _event *e,char *word);
static void near Parse_Time(struct _etime *t,char *word);
static void near Parse_Flag(struct _event *e,char *word);
static void near Event_Syntax_Err(char *err);
static void near Init_Event(struct _event *e);
static int near InEvent(struct _event *e, int for_now);

