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

/*# name=Message-area header file
*/

#ifndef __MAX_MSG_H_DEFINED
#define __MAX_MSG_H_DEFINED

#define MAX_LANG_m_area /* Include msg-area language items */
#define MAX_LANG_m_browse

#include <setjmp.h>

#ifndef __SQAPI_H_DEFINED
  #include "msgapi.h"
#endif

#include "mm.h"
#include "max_area.h"
#include "tagapi.h"

#ifndef ORACLE
extrn HAREA sq;
#endif

/*extrn char echo_written_in[(MAX_AREAS/CHAR_BITS)+1];*/
                            /* A bit field for all possible areas (1296    *
                             * max.), which tells whether or not the user  *
                             * entered an echo message in that area...     */

extrn int msgeof;           /* If we're at the end of the current msg. */

extrn dword last_msg;        /* The last msg. we read in this area */
        
extrn char direction;       /* Direction in which to read msgs (next/prev) */

#define ORIG_MSGID_LEN 160
extrn char orig_msgid[LEN(ORIG_MSGID_LEN)]; /* MSGID of message we're replying to. */

#define MAX_NETNODE 50
extrn char netnode[LEN(MAX_NETNODE)];/* String version of destination addr. */

extrn char isareply;        /* If the current msg. is a reply to another */
extrn char isachange;       /* If we're changing a msg */

extrn struct _mtagmem mtm;    /* Tagged message areas */
/*extrn struct _tagdata tma;*/   /* Tagged message areas */


/* These definitions provide ordinal offsets for the msg_attrxxx definitions
 * in english.mad.
 */

#define MSGKEY_PRIVATE      0
#define MSGKEY_CRASH        1
#define MSGKEY_READ         2
#define MSGKEY_SENT         3
#define MSGKEY_FILE         4
#define MSGKEY_FWD          5
#define MSGKEY_ORPHAN       6
#define MSGKEY_KILL         7
#define MSGKEY_LOCAL        8
#define MSGKEY_HOLD         9
#define MSGKEY_XX2         10
#define MSGKEY_FRQ         11
#define MSGKEY_RRQ         12
#define MSGKEY_CPT         13
#define MSGKEY_ARQ         14
#define MSGKEY_URQ         15
#define MSGKEY_LATTACH     16     /* Not available as a key, but used in
                                   * internal handling.
                                   */
enum
{
  TAG_TOGGLE=0,
  TAG_ADD=1,
  TAG_DEL=2
};

#endif /* !__MAX_MSG_H_DEFINED */

