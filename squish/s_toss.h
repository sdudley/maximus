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

struct _partial
{
  char pktname[PATHLEN];      /* Name of the packet being tossed */
  long offset;                /* Offset within that packet */
  long long_packet;           /* Did we encounter a msg that was too long? */
  word tflag;                 /* Flags for tossing from this packet */
};



/* Internal handle passed between tossing functions */

struct _inmsg
{
  char pktname[PATHLEN];                /* Name of this packet */
  int pktfile;                          /* File handle for packet */
  
  XMSG msg;                     /* _msg structure to toss */
  struct _pkthdr pkt;                   /* The header for this bundle */
  struct _pktprefix pktprefix;          /* The header for this message */
  byte *textptr;                        /* Beginning of _xmsg + msgtxt */
  byte *txt;                            /* texdtptr+sizeof(_xmsg) */
  byte *ctrl;                           /* Pointer to control info */
  unsigned length;                      /* Length of txt+sizeof(_xmsg) */
};

static char area_tag[MAX_TAGLEN];

/* 'msgbuf' is an external variable in S_SCAN.C.  This is where all of      *
 * the message text will be placed when running in TOSS/SCAN mode.  (For    *
 * just plain tossing, the text is left in the file-read buffer, to         *
 * conserve time.) This actually points about 60 bytes into the 'msgb'      *
 * array, since the extra space up front is used to insert the 'AREA:'      *
 * tag without shifting the message text around.  This pointer should       *
 * NEVER be passed to free() -- only do so for the below 'msgb' var.        */

extern byte *msgbuf;
extern byte *begin_sb, *end_sb;

static char last_area[MAX_TAGLEN];

static char *msgb;
static unsigned long msglen;


static HAREA sq;
static struct _cfgarea *last_sq;





static struct _talist
{
  HAREA sq;
  struct _cfgarea *ar;
} *talist;

static int talist_init;



static void near TossBadMsgs(struct _cfgarea *ar);
static struct _cfgarea *GetBadmsgsArea(struct _inmsg *in);
static void near Copy_To_Header(struct _inmsg *in);
static void near Toss_Pkt(char *pktname, word tflag);
static int near TossReadMsgFromPkt(struct _inmsg *in);
static int near TossOneMsg(struct _inmsg *in,int badmsg, word tflag);
static int near Process_Transient_Mail(struct _inmsg *in);
static struct _cfgarea * near Get_Area_Tag(struct _inmsg *in, char *txt);
static void near Handle_Dupe(struct _cfgarea *ar);
static int near Open_Area(struct _cfgarea *ar);
static void near Close_Area(void);
static void near Toss_Packet_Sequence(char *path, word tflag);
static int near Decompress_Archive(char *arcname,char *get);
static int near PacketSecure(struct _inmsg *in);
static void near Write_Echotoss(char *echotoss);
static void near Get_TFS(struct _inmsg *in);
static struct _cfgarea *GetDupesArea(struct _inmsg *in);
static void near NewArea(char *name);
static void near ReadMaxMsgs(char *tosslog);
static void near WriteMaxMsgs(void);
static void near ReportSpeed(time_t secs);
static void near TossArchives(struct _tosspath *tp);

