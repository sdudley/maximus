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

#include "events.h"

static void near Get_Next_Event(void);
static void near WFC_Init(void);
static void near WFC_Uninit(void);
static char * near Get_Modem_Response(void);
static int near Process_Modem_Response(char *rsp);
static int near WFC_IdleInternal(void);
static void near WFC_Init_Modem(void);
static void near Clear_MNP_Garbage(void);
static void near Get_Next_Event(void);
static void near DrawMaxHeader(void);
static void near UpdateStatWindow(char *status);
static void near Update_Lastuser(void);
static void near Update_Callers(void);
static void near Update_Status(char *status);
static void near Update_Event_Time(void);


void WFC_LogMsg(char *s);



static VWIN *win_stat;
static VWIN *win_modem;
static VWIN *win_activ;
static VWIN *win_keys;

static int kexit;
static struct _event next_event;
static time_t next_event_time=0x7fffffffL;
static int do_next_event;
static union stamp_combo today;

