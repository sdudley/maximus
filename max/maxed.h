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

#ifndef __MAXED_H_DEFINED
#define __MAXED_H_DEFINED

#define MAX_LANG_max_bor
#define MAX_LANG_m_area

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include <mem.h>
#include <setjmp.h>
#include <string.h>
#include <assert.h>
#include "prog.h"
#include "alc.h"
#include "mm.h"
#include "max_area.h"
#include "max_edit.h"
#include "max_msg.h"

#ifdef INIT_MAXED
  #define med_extern
  #define MIS(x) =x
  #define MLEN(x) x
#else
  #define med_extern extern
  #define MIS(x)
  #define MLEN(x)
#endif

#define linelen(x) strlen(screen[x]+1)

med_extern XMSG *mmsg;

med_extern word cursor_x;
med_extern word cursor_y;
med_extern word offset;
med_extern byte usrlen;
med_extern sword cur_quotebuf;      /* Current pointer to quotebuf_pos[] */
med_extern sword last_quote;        /* Pointer to endofmsg in quotebuf_pos */

med_extern byte *update_table;
med_extern char *quotebuf;
med_extern char last_msg_attr;
med_extern char insert;
med_extern char initials[MLEN(10)];
med_extern char pos_to_be_updated;
med_extern char skip_update;
med_extern char quoting;

med_extern HMSG qmh MIS(NULL);

med_extern long *quote_pos;

#include "maxedp.h"

#endif /* __MAXED_H_DEFINED */

