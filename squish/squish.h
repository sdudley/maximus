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

#ifndef __SQUISH_H_DEFINED
#define __SQUISH_H_DEFINED

#include "apidebug.h"
#include "arc_def.h"
#include "sqstat.h"
#include "skiplist.h"
#include "ffind.h"
#include "sqver.h"

#if defined(NT)
  #define SQNAME    "Squish/NT"
#elif defined(__FLAT__) && !defined(UNIX)
  #define SQNAME    "Squish/386"
#elif defined(OS_2)
  #define SQNAME    "Squish/2"
#elif defined(LINUX)
  #define SQNAME    "Squish/Linux"
#elif defined(UNIX)
  #define SQNAME    "Squish/UNIX"
#else
  #define SQNAME    "Squish"
#endif

extern byte *version;


/* "fast" i/o routines without cr/lf translation.  You CAN use
   read()/write() for these, but these will be faster.
*/


#ifdef __MSDOS__
  #define fastread(fd, buf, n)  farread(fd, (char far *)buf, n)
  #define fastwrite(fd, buf, n) farwrite(fd, (char far *)buf, n)
#else
  #define fastread(fd, buf, n)  read(fd, (char *)buf, n)
  #define fastwrite(fd, buf, n) write(fd, (char *)buf, n)
#endif


#define ERL_NONE        0
#define ERL_ERROR       1
#define ERL_SENT_ECHO   2
#define ERL_TOSS_NET    3
#define ERL_TOSS_ECHO   4
#define ERL_MAXMSGS     5

/*#define TAGLEN      48*/ /* use max_taglen */

#ifdef __FLAT__
  #define CTEXT_LEN   8192

  #define MAXMSGLEN_SMA    65536u
  #define MAXMSGLEN_MED   131072u
  #define MAXMSGLEN_LAR   262144u

  #define OUTBUF_SMALL    65536u
  #define OUTBUF_MEDIUM   131072u
  #define OUTBUF_LARGE    524288u

  #define WRITEBUF_SMALL  65536u
  #define WRITEBUF_MEDIUM 78000u
  #define WRITEBUF_LARGE  131072u
#else
  #define CTEXT_LEN   8192

  #define WRITEBUF_SMALL  16600u
  #define WRITEBUF_MEDIUM 26000u
  #define WRITEBUF_LARGE  57344u

  #define OUTBUF_SMALL    16384u
  #define OUTBUF_MEDIUM   40000u
  #define OUTBUF_LARGE    57344u

  #define MAXMSGLEN_SMA   16384u
  #define MAXMSGLEN_MED   32768u
  #define MAXMSGLEN_LAR   64512u

#endif


#define CFGLEN  4096  /* make sure to change in sqfeat.h too! */

#define MAX_TAGLEN  128
#define MAX_ADDRLEN 25    /* 12345:12345/12345.12345 = ~25 bytes */
#define MAXORIGIN   62
#define MAX_OB      128

#define ACTION_NONE   0x00
#define ACTION_POLL   0x01
#define ACTION_GET    0x02
#define ACTION_SEND   0x03
#define ACTION_RESCAN 0x04
#define ACTION_UPDATE 0x05

#define MODE_toss 0x01
#define MODE_scan 0x02
#define MODE_pack 0x04
#define MODE_link 0x08

#define MODE_tosscan (MODE_toss | MODE_scan)
#define MODE_all     (MODE_tosscan | MODE_pack)
#define MODE_sp      (MODE_scan | MODE_pack)

#define MAX_ARC_ARGS 32

/* Wrapper for old address-match functions */

#define AddrMatch(ad1, ad2)   MatchNN(ad1, ad2, FALSE)
#define AddrMatchNS(ad1, ad2) MatchNS(ad1, ad2, FALSE)
#define AddrMatchS(ad1, ad2)  MatchSS(ad1, ad2, FALSE)


#ifdef OS_2
  #define INCL_DOSMODULEMGR
  #include <os2.h>

  #include "sqfeat.h"   /* DLL feature library */
#endif

#ifdef UNIX
  #include "sqfeat.h"
#endif

/* If a node is entered in the "personal list", only messages addressed to  *
 * 'name' will be sent to 'node'.                                           */

struct _perlist
{
  struct _perlist *next;
  struct _sblist node;
  char *name;
};

/* Remapping for other nodes */

struct _remap
{
  struct _remap *next;          /* Next node to remap for                   */
  
  struct _sblist node;          /* Where to send msg to                     */
  byte *name;                   /* Addressee name                           */
};


#define AFLAG_STRIPPVT  0x0001  /* Strip pvt bit on inbound msgs for area   */
#define AFLAG_PASSTHRU  0x0002  /* This is a passthru area                  */
#define AFLAG_TOSSEDTO  0x0004  /* Area has been tossed to during sesion    */
#define AFLAG_SCANHERE  0x0008  /* Scan this area (internal to Scan_To_All) */
#define AFLAG_STATDATA  0x0010  /* We have some statistics information      */
#define AFLAG_DUPES     0x0020
#define AFLAG_NET       0x0040
#define AFLAG_BAD       0x0080
#define AFLAG_HIDEMSG   0x0100  /* Add private bit to all incoming msgs     */

/* Area in AREAS.BBS */

struct _cfgarea
{
/*struct _cfgarea *next;*/      /* Next area in entire configuration        */
  struct _cfgarea *next_type;   /* Next area of this type (net/bad_msgs)    */

  word type;                    /* Type of area: MSGTYPE_SDM, MSGTYPE_SQUISH*/
  byte *name;                   /* Echo tag of this area                    */
  byte *path;                   /* Path or .SQD name of area                */

  struct _sblist *scan;         /* Nodes to scan this area to               */
  struct _statlist *statlist;   /* Statistics for each node sent to         */
  
  struct _statarea statarea;    /* General stats for this area              */

  struct _sblist *add;          /* Nodes to add to SEEN-BYs                 */
  struct _sblist *update_ok;    /* Nodes who we can take ACUPDATE msgs from */
  word num_add;                 /* Number of nodes added to SB's for area   */

  word flag;                    /* General-purpose flag.  See AFLAG_XXX     */

  word num_scan;                /* Number of nodes in scan[] list           */
  word dupes;                   /* TOSS: # of dupes.                        */
  word tossed;                  /* TOSS: # of msgs tossed.                  */
  word sent;                    /* SCAN: # of msgs sent                     */
  
  struct _sblist primary;       /* Primary z:n/n.p to use for this area     */
  struct _sblist *norecv;       /* List of nodes to NOT accept msgs from    */
  struct _perlist *plist;       /* List of names/nodes to scan for          */
  
  dword sq_max_msgs;            /* SQ? areas: max # of msgs to keep         */
  dword sq_save_msgs;           /* SQ? areas: # of msgs to skip at begin.   */
  word  sq_keep_days;           /* SQ? areas: # of days of msgs to keep.    */
};



/* Gaterouting */

struct _groute
{
  struct _groute *next;         /* Next system to gateroute to              */

  struct _sblist host;          /* GateRoute TO this system                 */
  struct _sblist *nodes;        /* GateRoute FOR these nodes                */
  struct _sblist *except;       /* EXCEPT for these nodes                   */

  byte flavour;                 /* Flavour of gateroute                     */
  byte rsvd;

  word n_nodes;                 /* # of nodes in 'nodes'.  For ZoneGate only*/

};



/* Node passwords */

struct _nodepwd
{
  struct _nodepwd *next;        /* Next node password structure             */
  struct _sblist addr;          /* Address for node                         */
  char pwd[9];                  /* Eight char password                      */
};


/* List of outbound directories */

struct _outb
{
  word zone;
  byte *dir;
  struct _outb *next;
};


/* Forward to/from nodes */

struct _fwdlist
{
  struct _fwdlist *next;        /* Next node to forward from/to             */
  struct _sblist node;          /* Address of this node to fwd from/to      */
  word file;                    /* Do we support forwarding of files?       */
};


#define TFLAG_NOARC   0x01
#define TFLAG_NOPKT   0x02
#define TFLAG_NOECHO  0x04

struct _tosspath
{
  struct _tosspath *next;

  word flag;
  byte *path;
};


#define FLAG_ONEPASS  0x0001    /* We're doing squish in/out/squash         */
#define FLAG_KILLDUPE 0x0002    /* Kill dupes, instead of tossing dupearea  */
#define FLAG_SAVECTRL 0x0004    /* Save control info for *.SQ? bases.       */
#define FLAG_OLDARCM  0x0008    /* Use old ARCmail extensions               */
#define FLAG_QUIETARC 0x0010    /* Suppress output when spawning archivers  */
#define FLAG_SECURE   0x0020    /* Run in SECURE toss mode                  */
#define FLAG_KILLFWD  0x0040    /* Kill in-transit mail                     */
#define FLAG_FRODO    0x0080    /* If using a D'B/FroDo/Dutchie-type mailer */
#define FLAG_CHECKZ   0x0100    /* Check zone numbers on inbound packets    */
#define FLAG_KILLZ    0x0200    /* Kill zero-length netmail msgs            */
#define FLAG_BSY      0x0400    /* Support Bink 2.40+ .BSY flags            */
#define FLAG_TOSSBAD  0x0800    /* Attempt to toss from bad_msgs            */
#define FLAG_ADDMODE  0x1000    /* Add to arc files of existing flavour     */
#define FLAG_RSVD1    0x2000    /* was: FLAG_STRPATTR                       */
#define FLAG_BATCHXA  0x4000    /* Unarc all packets first, then toss.      */
#define FLAG_STATS    0x8000    /* Statistics-gathering mode                */

#define FLAG2_QUIET   0x0001    /* Suppress info messages                   */
#define FLAG2_NPTHRU  0x0002    /* Only scan non-passthru areas             */
#define FLAG2_SWAP    0x0004    /* Swap to disk for xternal programs        */
#define FLAG2_NUKE    0x0008    /* Nuke bundles who have no attaches        */
#define FLAG2_BINKPT  0x0010    /* Use Bink 2.50+ method for point info     */
#define FLAG2_NOSNDX  0x0020    /* No soundex name matching for remapper    */
#define FLAG2_NOSTOMP 0x0040    /* Don't stomp packet header when routing   */
#define FLAG2_KFFILE  0x0080    /* Kill forwarded files                     */
#define FLAG2_LMSGID  0x0100    /* Link using ^aMSGIDs                      */
#define FLAG2_DHEADER 0x0200    /* Dupecheck using the message header       */
#define FLAG2_DMSGID  0x0400    /* Dupecheck using the MSGID                */
#define FLAG2_LONGHDR 0x0800    /* Use the entire subject line for dupe chk */

struct _config
{
  SLIST *area;                  /* Skip list containing areas.bbs members   */
  struct _cfgarea *badmsgs;     /* Pointer to the bad_msgs list             */
  struct _cfgarea *dupes;       /* Pointer to the dupes list                */
  struct _cfgarea *netmail;     /* Pointer to the netmail list              */
  struct _cfgarea *has_dlist;   /* Pointer to area holding dupelist         */

  struct _sblist def;           /* Default zone/net/node struct             */
  struct _sblist *ats;          /* Add_To_Seen linked list                  */
  struct _arcinfo *arc;         /* Archiver control list                    */
  struct _arcinfo *def_arc;     /* Default archiver                         */
  struct _sblist *tiny;         /* Nodes to receive tiny SB's               */
  struct _sblist *tiny_except;  /* Nodes to NOT receive tiny SB's           */
  struct _sblist *stripattr;    /* Nodes to strip attributes                */
  struct _sblist *stripattr_except; /* Nodes to NOT strip attributes        */
  struct _groute *gate;         /* Gaterouting nodes                        */
  struct _groute *zgat;         /* Zonegate nodes                           */
  struct _tosspath *tpath;      /* Paths to toss from                       */
  struct _outb *outb;           /* Where to find outbound directories       */
#if defined(OS_2) || defined(UNIX)
  struct _feature *feat;        /* DLL features                             */
#endif

  word flag;                    /* General-purpose flag                     */
  word flag2;                   /*  "          "     "                      */
  word pointnet;                /* Pointnet number that we host             */
  word dupe_msgs;               /* Number of dupe msgs to track per area    */
  word num_ats;                 /* Number of nodes to add to seenbys        */
  word max_msgs;                /* Max # of msgs to toss before packing     */
  word max_archive;             /* Max size of one ARCmail archive          */

  word maxpkt;                  /* Max # of pkts in OUT.SQ at once          */
  word maxattach;               /* Max # of attach msgs in netmail at once  */

  byte *areasbbs;               /* Pointer to AREAS.BBS file                */
  byte *routing;                /* Name of ROUTE.CFG                        */
  byte *logfile;                /* Name of general-purpose log file         */
  byte *tracklog;               /* Where to put MSGTRACK.LOG                */
  byte *origin;                 /* Default origin line                      */
  byte *compress_cfg;           /* Where to find COMPRESS.CFG               */
  byte *swappath;               /* Disk swap path                           */
  byte *statfile;               /* Name of statistics file */

  struct _sblist *addr;         /* Our addresses                            */
  struct _remap *remap;         /* Remap for these nodes                    */
  struct _nodepwd *pwd;         /* Passwords for nodes                      */
  struct _fwdlist *fwdto;       /* Nodes to forward TO                      */
  struct _fwdlist *fwdfrom;     /* Nodes to forward FROM                    */
  
  word ob[MAX_OB];              /* List of available outbound zone numbers  */
  word num_ob;                  /* Number of outbound areas                 */
  word max_talist;              /* Max# of Squish areas open at once        */
  word max_handles;             /* Max# of file handles to open at once     */
  byte rsvd;                    /* RESERVED                                 */
  byte loglevel;                /* Log level to use for SQUISH.LOG (1-5)    */
};

#define BUF_LARGE       0x00
#define BUF_MEDIUM      0x01
#define BUF_SMALL       0x02

#define OS_MAX_HANDLES  20                    /* standard 20 files open   */
#define OS_MAX_TALIST   (OS_MAX_HANDLES-12)/2 /* max 4 areas open at once */


#define BOUNCE_netmail    0x01
#define BOUNCE_echomail   0x02




struct _blist
{

#define BL_ID (unsigned char)0xee

  unsigned char id;

  struct _blist *next;

  word zone, net, node, point;
  struct _cfgarea *ar;
  int done;

  int flavour;
  long len;
};






#define MATCH_FLO       0x01
#define MATCH_OUT       0x02
#define MATCH_UNKNOWN   0x04

#define MATCH_ALL       (MATCH_FLO | MATCH_OUT | MATCH_UNKNOWN)

/* Descriptor for a message held in the out.sq area */

struct _hpkt
{
  /* 123456789012345678  */
  /* .xxx\12345678.pkt\0 */

  #define MAX_HPKT_NAME 18

  struct _sblist from, to;
  byte name[MAX_HPKT_NAME];
  dword attr;
  byte trunc, del;
};


/* Descriptor for a netmail attach held in a netarea */

struct _netinf
{
  struct _sblist from, to;
  byte *name;
  dword attr;
  byte trunc, del;
};



typedef struct _matchout
{
  struct _hpkt *hpkt;
  FFIND *ff, *parentff;
  NETADDR who;
  sword got_type;

  struct _config *config;

  char name[PATHLEN];
  NETADDR found;

  sword cur_ob;
  sword high_ob;
  sword type;
  byte flavour;
  byte fFromHole;
} MATCHOUT;


#include "old_msg.h"


typedef struct _blist BLIST; /* For outbuf[] bundles */

#include "squishp.h"

#define CVT_FLO  1
#define CVT_ARC  2
#define CVT_KILL 3
#define CVT_SFLO 4

#ifdef INITSQUISH
/*  byte *config_delim=" \t\n";*/
  byte *buffer=NULL;
  byte *orig_buffer=NULL;
  word bad_packet=FALSE;
  word erl_sent=0, erl_net=0, erl_echo=0, erl_max=0;
  byte *cantopen="!Can't open %s";
  byte *cantread="!Can't read %s";
/*  byte convert=0;*/
  unsigned outbufmax=OUTBUF_LARGE;
  unsigned writebufmax=WRITEBUF_LARGE;
  unsigned maxmsglen=MAXMSGLEN_MED;
  dword nmsg_tossed=0L, nmsg_scanned=0L, nmsg_sent=0L;
#else
/*  extern byte *config_delim;*/
  extern byte *buffer;
  extern byte *orig_buffer;
  extern word bad_packet;
  extern word erl_sent, erl_net, erl_echo, erl_max;
  extern byte *cantopen, *cantread;
/*  extern byte convert;*/
  extern unsigned maxmsglen;
  extern unsigned outbufmax;
  extern unsigned writebufmax;
  extern dword nmsg_tossed, nmsg_scanned, nmsg_sent;
#endif

extern struct _config config;


#ifndef INITSQUISH
extern
#endif
struct _flotype
{
  byte *name;
  byte flavour;
} flo_str[]

#ifdef INITSQUISH
 =
  {
    {"Hold",    'H'},
    {"Crash",   'C'},
    {"Express", 'C'},
    {"Direct",  'D'},
    {"Normal",  'F'},
    {NULL,       0}
  }
#endif
;


#ifndef INITSQUISH
  extern dword cr3tab[];
#else
/* First, the polynomial itself and its table of feedback terms.  The  */
/* polynomial is                                                       */
/* X^32+X^26+X^23+X^22+X^16+X^12+X^11+X^10+X^8+X^7+X^5+X^4+X^2+X^1+X^0 */
/* Note that we take it "backwards" and put the highest-order term in  */
/* the lowest-order bit.  The X^32 term is "implied"; the LSB is the   */
/* X^31 term, etc.  The X^0 term (usually shown as "+1") results in    */
/* the MSB being 1.                                                    */

/* Note that the usual hardware shift register implementation, which   */
/* is what we're using (we're merely optimizing it by doing eight-bit  */
/* chunks at a time) shifts bits into the lowest-order term.  In our   */
/* implementation, that means shifting towards the right.  Why do we   */
/* do it this way?  Because the calculated CRC must be transmitted in  */
/* order from highest-order term to lowest-order term.  UARTs transmit */
/* characters in order from LSB to MSB.  By storing the CRC this way,  */
/* we hand it to the UART in the order low-byte to high-byte; the UART */
/* sends each low-bit to hight-bit; and the result is transmission bit */
/* by bit from highest- to lowest-order term without requiring any bit */
/* shuffling on our part.  Reception works similarly.                  */

/* The feedback terms table consists of 256, 32-bit entries.  Notes:   */
/*                                                                     */
/*     The table can be generated at runtime if desired; code to do so */
/*     is shown later.  It might not be obvious, but the feedback      */
/*     terms simply represent the results of eight shift/xor opera-    */
/*     tions for all combinations of data and CRC register values.     */
/*                                                                     */
/*     The values must be right-shifted by eight bits by the "updcrc"  */
/*     logic; the shift must be unsigned (bring in zeroes).  On some   */
/*     hardware you could probably optimize the shift in assembler by  */
/*     using byte-swap instructions.                                   */

dword cr3tab[] = {                         /* CRC polynomial 0xedb88320 */
0x00000000Lu, 0x77073096Lu, 0xee0e612cLu, 0x990951baLu, 0x076dc419Lu,
0x706af48fLu, 0xe963a535Lu, 0x9e6495a3Lu, 0x0edb8832Lu, 0x79dcb8a4Lu,
0xe0d5e91eLu, 0x97d2d988Lu, 0x09b64c2bLu, 0x7eb17cbdLu, 0xe7b82d07Lu,
0x90bf1d91Lu, 0x1db71064Lu, 0x6ab020f2Lu, 0xf3b97148Lu, 0x84be41deLu,
0x1adad47dLu, 0x6ddde4ebLu, 0xf4d4b551uL, 0x83d385c7Lu, 0x136c9856Lu,
0x646ba8c0Lu, 0xfd62f97aLu, 0x8a65c9ecLu, 0x14015c4fLu, 0x63066cd9Lu,
0xfa0f3d63Lu, 0x8d080df5Lu, 0x3b6e20c8Lu, 0x4c69105eLu, 0xd56041e4Lu,
0xa2677172Lu, 0x3c03e4d1Lu, 0x4b04d447Lu, 0xd20d85fdLu, 0xa50ab56bLu,
0x35b5a8faLu, 0x42b2986cLu, 0xdbbbc9d6Lu, 0xacbcf940Lu, 0x32d86ce3Lu,
0x45df5c75Lu, 0xdcd60dcfLu, 0xabd13d59Lu, 0x26d930acLu, 0x51de003aLu,
0xc8d75180Lu, 0xbfd06116Lu, 0x21b4f4b5Lu, 0x56b3c423Lu, 0xcfba9599Lu,
0xb8bda50fLu, 0x2802b89eLu, 0x5f058808Lu, 0xc60cd9b2Lu, 0xb10be924Lu,
0x2f6f7c87Lu, 0x58684c11Lu, 0xc1611dabLu, 0xb6662d3dLu, 0x76dc4190Lu,
0x01db7106Lu, 0x98d220bcLu, 0xefd5102aLu, 0x71b18589Lu, 0x06b6b51fLu,
0x9fbfe4a5Lu, 0xe8b8d433Lu, 0x7807c9a2Lu, 0x0f00f934Lu, 0x9609a88eLu,
0xe10e9818Lu, 0x7f6a0dbbLu, 0x086d3d2dLu, 0x91646c97Lu, 0xe6635c01Lu,
0x6b6b51f4Lu, 0x1c6c6162Lu, 0x856530d8Lu, 0xf262004eLu, 0x6c0695edLu,
0x1b01a57bLu, 0x8208f4c1Lu, 0xf50fc457Lu, 0x65b0d9c6Lu, 0x12b7e950Lu,
0x8bbeb8eaLu, 0xfcb9887cLu, 0x62dd1ddfLu, 0x15da2d49Lu, 0x8cd37cf3Lu,
0xfbd44c65Lu, 0x4db26158Lu, 0x3ab551ceLu, 0xa3bc0074Lu, 0xd4bb30e2Lu,
0x4adfa541Lu, 0x3dd895d7Lu, 0xa4d1c46dLu, 0xd3d6f4fbLu, 0x4369e96aLu,
0x346ed9fcLu, 0xad678846Lu, 0xda60b8d0Lu, 0x44042d73Lu, 0x33031de5Lu,
0xaa0a4c5fLu, 0xdd0d7cc9Lu, 0x5005713cLu, 0x270241aaLu, 0xbe0b1010Lu,
0xc90c2086Lu, 0x5768b525Lu, 0x206f85b3Lu, 0xb966d409Lu, 0xce61e49fLu,
0x5edef90eLu, 0x29d9c998Lu, 0xb0d09822Lu, 0xc7d7a8b4Lu, 0x59b33d17Lu,
0x2eb40d81Lu, 0xb7bd5c3bLu, 0xc0ba6cadLu, 0xedb88320Lu, 0x9abfb3b6Lu,
0x03b6e20cLu, 0x74b1d29aLu, 0xead54739Lu, 0x9dd277afLu, 0x04db2615Lu,
0x73dc1683Lu, 0xe3630b12Lu, 0x94643b84Lu, 0x0d6d6a3eLu, 0x7a6a5aa8Lu,
0xe40ecf0bLu, 0x9309ff9dLu, 0x0a00ae27Lu, 0x7d079eb1Lu, 0xf00f9344Lu,
0x8708a3d2Lu, 0x1e01f268Lu, 0x6906c2feLu, 0xf762575dLu, 0x806567cbLu,
0x196c3671Lu, 0x6e6b06e7Lu, 0xfed41b76Lu, 0x89d32be0Lu, 0x10da7a5aLu,
0x67dd4accLu, 0xf9b9df6fLu, 0x8ebeeff9Lu, 0x17b7be43Lu, 0x60b08ed5Lu,
0xd6d6a3e8Lu, 0xa1d1937eLu, 0x38d8c2c4Lu, 0x4fdff252Lu, 0xd1bb67f1Lu,
0xa6bc5767Lu, 0x3fb506ddLu, 0x48b2364bLu, 0xd80d2bdaLu, 0xaf0a1b4cLu,
0x36034af6Lu, 0x41047a60Lu, 0xdf60efc3Lu, 0xa867df55Lu, 0x316e8eefLu,
0x4669be79Lu, 0xcb61b38cLu, 0xbc66831aLu, 0x256fd2a0Lu, 0x5268e236Lu,
0xcc0c7795Lu, 0xbb0b4703Lu, 0x220216b9Lu, 0x5505262fLu, 0xc5ba3bbeLu,
0xb2bd0b28Lu, 0x2bb45a92Lu, 0x5cb36a04Lu, 0xc2d7ffa7Lu, 0xb5d0cf31Lu,
0x2cd99e8bLu, 0x5bdeae1dLu, 0x9b64c2b0Lu, 0xec63f226Lu, 0x756aa39cLu,
0x026d930aLu, 0x9c0906a9Lu, 0xeb0e363fLu, 0x72076785Lu, 0x05005713Lu,
0x95bf4a82Lu, 0xe2b87a14Lu, 0x7bb12baeLu, 0x0cb61b38Lu, 0x92d28e9bLu,
0xe5d5be0dLu, 0x7cdcefb7Lu, 0x0bdbdf21Lu, 0x86d3d2d4Lu, 0xf1d4e242Lu,
0x68ddb3f8Lu, 0x1fda836eLu, 0x81be16cdLu, 0xf6b9265bLu, 0x6fb077e1Lu,
0x18b74777Lu, 0x88085ae6Lu, 0xff0f6a70Lu, 0x66063bcaLu, 0x11010b5cLu,
0x8f659effLu, 0xf862ae69Lu, 0x616bffd3Lu, 0x166ccf45Lu, 0xa00ae278Lu,
0xd70dd2eeLu, 0x4e048354Lu, 0x3903b3c2Lu, 0xa7672661Lu, 0xd06016f7Lu,
0x4969474dLu, 0x3e6e77dbLu, 0xaed16a4aLu, 0xd9d65adcLu, 0x40df0b66Lu,
0x37d83bf0Lu, 0xa9bcae53Lu, 0xdebb9ec5Lu, 0x47b2cf7fLu, 0x30b5ffe9Lu,
0xbdbdf21cLu, 0xcabac28aLu, 0x53b39330Lu, 0x24b4a3a6Lu, 0xbad03605Lu,
0xcdd70693Lu, 0x54de5729Lu, 0x23d967bfLu, 0xb3667a2eLu, 0xc4614ab8Lu,
0x5d681b02Lu, 0x2a6f2b94Lu, 0xb40bbe37Lu, 0xc30c8ea1Lu, 0x5a05df1bLu,
0x2d02ef8dLu };

#endif /* !INITSQUISH */

#ifdef __MSDOS__
void relocate(int num_fd);
#endif


#endif /* !__SQUISH_H_DEFINED */

