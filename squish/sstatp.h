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



struct _nodtot
{
  NETADDR node;
  double total_percent_bytes;   /* Amount this node pays of total bill.     */
  double total_percent_msgs;    /* Amount this node pays of total bill.     */
  struct _nodtot *next;
};

struct _stlist
{
  NETADDR node;                 /* Address of this node                     */

  dword out_msgs;               /* Total # of msgs sent to this node        */
  dword out_bytes;              /* Total # of bytes sent to this node       */
  struct _stlist *next;
};

struct _ahlist
{
  char tag[AH_TAGLEN];

  dword in_msgs;
  dword in_bytes;

  dword total_out_bytes;
  dword total_out_msgs;

  struct _stlist *slist;
  struct _ahlist *next;
};

struct _nodelist
{
  NETADDR n;
  struct _nodelist *next;
};

struct _arealist
{
  char *tag;
  struct _arealist *next;
};

static struct _sscfg
{
  struct _nodelist *node;
  struct _arealist *area;
  word do_all;
};

