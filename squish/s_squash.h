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

#define cvlen (sizeof(cv)/sizeof(cv[0]))

#define COPYSIZE        4096    /* Copy buffer for merging .PKT files */

#define MAX_ROUTE_ARGS    64
#define MAX_ROUTE_LEN     64
#define MAX_ROUTE_LINELEN 256
#define MAX_ROUTE_NODES   64


static char scratch[PATHLEN];
static int linenum;


  
/* non-static -- used in s_toss.c */
char *arcm_exts[]={".su",".mo",".tu",".we",".th",".fr",".sa",NULL};

/*
static char *obflo="%s%04x%04x.%clo";
char *obout="%s%04x%04x.%cut";
static char *psp04xp04x;
*/

static char *routedelim=" \t\n\r";
                 
                 
static void near Check_Outbound_Areas(void);


static struct _defn
{
  char *name;
  char *xlat;
  struct _defn *next;
} *defns;


