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

#include "mm.h"
#include "crc.h"
#include "modem.h"

#if defined(ERROR) && defined(NT)   /* eliminate windows.h definition */
#undef ERROR
#endif

/* Macros */

#define UPDC32(b, c) updcrc32(b, c)

#define zmdm_pputcw(c) {if(iTxBufLeft--==0){sendmo();iTxBufLeft--;}*szTxPtr++=c;}

extern int iTxBufLeft;
extern unsigned char *szTxPtr;

/* Redefinitions for Scott's i/o library */

#define rdchk() mdm_avail()


/* Constants */

#define DEFBYTL 2000000000L     /* default rx file size */
#define UNIXFILE 0xF000         /* The S_IFMT file mask bit for stat */
#define TIMER_RETX_XON  2000    /* Send an XON every 20 seconds if blocked */
#define HOWMANY 255

#define XON ('q'&037)
#define XOFF ('s'&037)
#define CAN ('X'&037)

#define OK 0
#define ERROR (-1)
#define TIMEOUT (-2)
#define RCDO (-3)
#define WCEOT (-10)

/* Ward Christensen / CP/M parameters - Don't change these! */
#define ENQ 005
#define SOH 1
#define STX 2
#define EOT 4
#define ACK 6
#define NAK 025
#define CPMEOF 032
#define WANTCRC 0103    /* send C not NAK to get crc not checksum */
#define ERRORMAX 5
#define RETRYMAX 5

/* Zmodem constants */

#define ZRQINIT 0       /* Request receive init */
#define ZRINIT  1       /* Receive init */
#define ZSINIT 2        /* Send init sequence (optional) */
#define ZSKIP 5         /* To sender: skip this file */
#define ZRPOS 9         /* Resume data trans at this position */
#define ZDATA 10        /* Data packet(s) follow */
#define ZEOF 11         /* End of file */

/* rz.c */

int ZmRzInitStatics(void);
void ZmRzDeinitStatics(void);
int ZmodemRecvFile(char *path, char *name, int fInit);
int readline(int timeout);

/* sz.c */

int ZmSzInitStatics(void);
void ZmSzDeinitStatics(void);
int ZmodemSendFile(char *szName, int fInit, unsigned long ulFiles, unsigned long ulBytes);

/* rbsb.c */

int mode(int m);
void sendbrk(void);


/* zm.c */

int ZmInitStatics(void);
void ZmDeinitStatics(void);

/*void zmdm_pputcw(int c);*/
void sendmo(void);
void dumpmo(void);
void ZmVarsInit(void);
int ZmQueryLocalAbort(void);
int ZmDoLocalAbort(void);
void flushmo(void);
void zmputs(char *s);
void purgeline(void);
int readline(int timeout);
void canit(void);
void zsbhdr(int type, char *hdr);
void zshhdr(int type, char *hdr);
void zsdata(unsigned char *buf, int length, int frameend);
int zrdata(char *buf, int length);
int zgethdr(char *hdr, int eflag);
void stohdr(long pos);
long rclhdr(char *hdr);


/* Variables from crctab.c */

extern dword *cr3tab;
extern word *crctab;

/* Variables from zm.c */

extern int Rxtimeout;
extern int Rxframeind;
extern int Rxtype;
extern int Rxcount;
extern char Rxhdr[4];
extern char Txhdr[4];
extern long Rxpos;
extern long Txpos;
extern int Txfcs32;
extern int Crc32t;
extern int Crc32;
extern char *Attn;
extern int Verbose;
extern int Zctlesc;
extern int z_errors;
extern long Rxbytes;
extern int fSending;

