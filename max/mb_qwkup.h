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

static char * near fix_basic_crap(char *str);
static int near Toss_QWK_Packet(char *name);
static int near QWK_Get_Rep_Header(int qfd, char *block);
static void near QWK_To_Xmsg(struct _qmhdr *qh, XMSG *msg, word msgn);
static int near Toss_QWK_Message(struct _qmhdr *qh, XMSG *msg, int qfd, char *block);
static int near all_caps(char *s);
static int near Receive_REP(char *name);
static int near Decompress_REP(char *rep_name);

