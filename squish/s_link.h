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

static void Link_Area(struct _cfgarea *ar, HAREA opensq);
static void near LinkIt(HAREA sq, struct _cfgarea *ar);
static int _stdc msgcomp(const void *i1, const void *i2);
static int _stdc msgidcomp(const void *i1, const void *i2);
static size_t msgidsearch(struct _link **link, long nl, struct _link *find);
static int _stdc umsgidcomp(const void *i1, const void *i2);

struct _link
{
  byte subj[30];              /* Subject line of this msg */
  dword mnum;                 /* Message number of this msg */
  UMSGID up;                  /* We are reply to this msg */
  UMSGID downs[MAX_REPLY];    /* Replies to this msg */
  unsigned delta;             /* Has this msg been changed? */

  dword msgid_hash;           /* Hash of address portion of ^aMSGID */
  dword msgid_stamp;          /* ^aMSGID timestamp */

  dword reply_hash;           /* Hash of address portion of ^aREPLY */
  dword reply_stamp;          /* ^aREPLY timestamp */
};

