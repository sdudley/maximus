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

/*# name=SILT include file
*/

#ifndef __SILT_H_INCLUDED
#define __SILT_H_INCLUDED

#include "s_heap.h"


typedef struct _ovlist
{
  struct _ovride or;
  struct _ovlist *next;
} *OVRLIST;


typedef struct
{
  int marea;  /* always false */
  HEAP h;     /* must always be 2nd element */
  OVRLIST ol; /* must always be 3rd element */
  FAREA fa;
} FAINFO;


typedef struct
{
  int marea;  /* always true */
  HEAP h;     /* must always be 2nd element */
  OVRLIST ol; /* must always be 3rd element */
  MAREA ma;
} MAINFO;


struct _vbtab
{
  void (near *f)(void *pmi, char *words[], char *line);
  char *verb;
  zstr *pzstr;
};

#ifdef __TURBOC__
#pragma warn -rch
#endif


#define PFI(v) ((FAINFO *)(v))
#define PMI(v) ((MAINFO *)(v))

#define MAX_PARSE_WORDS   16
#define MAX_MSG_HEAP      2048
#define MAX_FILE_HEAP     2048


#define LAST_NONE         0x00  /* Used in SILT to control the status      */
#define LAST_SECTION      0x01  /* display.                                */
#define LAST_AREA         0x02
#define LAST_MENU         0x03
#define LAST_ACCESS       0x04

#define AREA_BUF_SIZE   6144
#define HEAP_SIZE       0x2000
#define MAX_BUFR        512

#define Make_String(item,value)   (item)=Make_Strng(value,FALSE);
#define Make_Filename(item,value) (item)=Make_Strng(value,TRUE);
#define Make_Path(item, str)      (item)=Make_Pth(str)


#ifdef SILT_INIT
  #define silt_extern
  #define SILT_IS(x) =x
  #define SILT_SIZE(s) s
#else
  #define silt_extern extern
  #define SILT_IS(x)
  #define SILT_SIZE(s)
#endif

silt_extern struct _menu menu;
silt_extern struct m_pointers prm;

silt_extern char cantfind_file_ctl[] SILT_IS("\n\aError!  Can't find file `%s', line %d of CTL file!\n");
silt_extern char cc_section[] SILT_IS("Compiling Section: %s");
silt_extern char cc_area[] SILT_IS("Compiling Area:    %s");
silt_extern char cc_menu[] SILT_IS("Compiling Menu:    %s");
silt_extern char cc_accs[] SILT_IS("Compiling Access:  %s");
silt_extern char protocol_max[] SILT_IS("%sprotocol.max");
silt_extern char max20area[SILT_SIZE(PATHLEN)] SILT_IS("area");
silt_extern char ctl_delim[] SILT_IS(" \t\n");
silt_extern char strings[SILT_SIZE(HEAP_SIZE)] SILT_IS("");
silt_extern char ctl_name[SILT_SIZE(120)];
silt_extern char line[SILT_SIZE(MAX_LINE)];

silt_extern int offset SILT_IS(1);
silt_extern int num_class SILT_IS(0);
silt_extern int protocol_num SILT_IS(0);
silt_extern int menuopt;
silt_extern int linenum;
silt_extern int priv_word;

silt_extern char last SILT_IS(LAST_NONE);
silt_extern char do_prm SILT_IS(FALSE);
silt_extern char do_farea SILT_IS(FALSE);
silt_extern char do_marea SILT_IS(FALSE);
silt_extern char do_menus SILT_IS(FALSE);
silt_extern char do_unattended SILT_IS(FALSE);
silt_extern char do_2areas SILT_IS(FALSE);
silt_extern char do_ul2areas SILT_IS(FALSE);
silt_extern char do_short2areas SILT_IS(FALSE);
silt_extern char done_sys SILT_IS(FALSE);
silt_extern char done_access SILT_IS(FALSE);
silt_extern char done_equip SILT_IS(FALSE);
silt_extern char done_matrix SILT_IS(FALSE);
silt_extern char done_session SILT_IS(FALSE);
silt_extern char done_language SILT_IS(FALSE);
silt_extern char done_colours SILT_IS(FALSE);

extern struct _maxcol col;

#undef silt_extern
#undef SILT_IS
#undef SILT_SIZE

#include "siltprot.h"

#endif /* __SILT_H_INCLUDED */

