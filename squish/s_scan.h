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

static void near AddAreaTag(char *txt, struct _cfgarea *ar);
static int _stdc sbl_comp_nz(const void *s1, const void *s2);
static void near Insert_Sb_Node(struct _sblist *look, struct _sblist *sb, unsigned *num_sb);
static int near Need_To_Scan(struct _cfgarea *ar, struct _sblist *scanto, struct _sblist *sb, unsigned num_sb, XMSG *msg);
static void near Add_Outbuf(struct _sblist *to,struct _pktprefix *pp, byte *lump, unsigned long lumplen, char *text, unsigned long textlen, int flavour, struct _cfgarea *ar);
static void near Fix_SeenBys(struct _sblist *sb, unsigned num_sb, char *mbuf, unsigned smask, struct _cfgarea *ar);
static void near Add_Us_To_Path(struct _cfgarea *ar, char *msgbuf);
static void near Add_Tear_Line(char *msgbuf,struct _cfgarea *ar,XMSG *msg);
/*static int near dowrite(int file,char *p,int len);*/
static int near NodeInSlist(struct _sblist *node, struct _sblist *check_list);
static int near GetsTinySeenbys(struct _sblist *node);

static byte far *outbuf;
static byte far *cur_ob;
static byte far *end_ob;
static byte *ctext;
byte *msgbuf;
static char *msgdelim=" \r\n\x8d";

#define SEEN_BY_LEN 8
char *seen_by_str="SEEN-BY:";


static unsigned strlen_msgbuf;
static NETADDR *resc;

byte *begin_sb, *end_sb;


/* Bit masks for scanner routine -- determines whether or not the specified *
 * node should see point and/or tiny seenbys...                             */

#define SCAN_NOPOINT    0x01
#define SCAN_TINY       0x02

