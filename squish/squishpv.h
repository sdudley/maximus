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

struct _args
{
  byte echotoss[PATHLEN];
  byte cfgname[PATHLEN];
  byte areasbbs[PATHLEN];
  byte sched[PATHLEN];
  byte logfile[PATHLEN];

  word do_pack;
  word action;
  word leave_packets;

  byte **toscan;
  NETADDR n;
};

#if 0
static void near test_harness(char *name);
#endif

static void near ParseArgs(struct _args *ags, byte *argv[]);
static void near CleanupConfig(void);
static void near SquishSquashCycle(void);
static void near InitializeConfig(void);



