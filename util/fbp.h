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

#ifdef __FLAT__
  #define MAX_FINF     32768
#else
  #define MAX_FINF     2048
#endif

struct _finf
{
  byte  name[12]; /* NOT NUL TERMINATED */
  dword size;
  SCOMBO cdate;
  SCOMBO wdate;
};


struct _alist
{
  struct _alist *next;

  char *path;
  long size;
  word anum;
};

static void near make_ext(char *name, char *ext);
static int near process_files_bbs(PFAH pfah, char *fbbs, char *path, int ffsUpdate);
static int near process_line(PFAH pfah, byte *line,char *path);
static void near write_entry(FDAT *fdat, char *desc, char *ul, char *path, char *acs);
static void near cant_open(char *name);
static void near sort_idx(char *path);
static int _stdc idxcomp(const void *i1,const void *i2);
static void near say_done(void);
/*static*/ void near merge_lists(int len);
/*static*/ void near get_smallest(struct _alist **smaller,struct _alist **bigger);
/*static*/ int near merge_these(struct _alist *f1,struct _alist *f2,char *into);
/*static*/ int near unlist(struct _alist *rem);
/*static*/ void near addlist(struct _alist *p);
void _fast NoMem(void);
static word near do_this_area(PFAH pfah, char *argv[], int fDoSlow);
static void near sort_index(char *master_idx, struct _alist *al, long size);
static word near MatchWC(char *pat,char *fn);
static sword _fast near cBwrite(BFILE b, void *buf, word size);

