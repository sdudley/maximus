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

static int near CanDownload(PFAH pfah, char *name, word *flag, char *path);
static int near InFileList(PFAH pfah, char *name, word *flag, char *pt);
static int near FileExist(char *path, char *name);
static int near IndexSearch(char *name, int do_tag, sword protocol);
static int near FindIndexFile(struct _fidx *fidx, int do_tag, sword protocol);
static int near FileLimitsOkay(unsigned long ulSize, int flags,
                               sword protocol, long *realbytes,
                               long *prtime);
static int near GotFile(char *fname, long size, int do_tag, sword protocol,
                        int flags);

