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

/* Definitions used by Squish for stats processing */

#include "sstat2.h"


/* Statistics for this area. Used internally by sstat. */

struct _statarea
{
  dword in_msgs;                /* Total # of msgs received in this area    */
  dword in_bytes;               /* Total # of bytes received in this area   */
};


/* Linked list of statistics information which parallels the linked         *
 * list of nodes to scan, 'scan[]'.  Used internally by sstat               */

struct _statlist
{
  NETADDR node;                 /* Address of this node                     */

  dword out_msgs;               /* Total # of msgs sent to this node        */
  dword out_bytes;              /* Total # of bytes sent to this node       */
  struct _statlist *next;
};


