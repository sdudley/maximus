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

#pragma off(unreferenced)
static char rcs_id[]="$Id: sz.c,v 1.1 2002/10/01 17:54:43 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include "zsjd.h"
#include "zmodem.h"
#include "pdata.h"

static unsigned Txwindow;	/* Control the size of the transmitted window */
static unsigned Txwspac;	/* Spacing between zcrcq requests */
static unsigned Txwcnt;		/* Counter used to space ack requests */
static long Lrxpos;		/* Receiver's last reported offset */
static FILE *in;
static char *txbuf; /*[1024];*/
static int errcnt;		/* number of files unreadable */
static int blklen;		/* length of transmitted records */
static int Eofseen;		/* EOF seen on input set by zfilbuf */
static unsigned Rxbuflen;	/* Receiver's max buffer length */
static int Tframlen;		/* Override for tx frame length */
static int Rxflags;
static long bytcnt;
static int Wantfcs32;		/* want to send 32 bit FCS */
#define LZCONV  0
#define LZMANAG 0
#define LZTRANS 0
static long Lastsync;		/* Last offset to which we got a ZRPOS */
static int Beenhereb4;		/* How many times we've been ZRPOS'd same place */
static long zfilesize;          /* Size of the file to be transmitted */
static int zslugbait;           /* Have we sent the last subpacket of file? */


/* Initialize the static variables used in sz.c */

int ZmSzInitStatics(void)
{
  txbuf=malloc(1024);

  return !!txbuf;
}



/* Free the memory allocated by ZmSzInitStatics */

void ZmSzDeinitStatics(void)
{
  if (txbuf)
  {
    free(txbuf);
    txbuf=NULL;
  }
}



/* Say "bibi" to the receiver, try to do it cleanly */

static void near
saybibi(void)
{
  for (;;)
  {
    stohdr(0L);			/* CAF Was zsbhdr - minor change */
    zshhdr(ZFIN, Txhdr);	/*  to make debugging easier */

    switch (zgethdr(Rxhdr, 0))
    {
      case ZFIN:
        zmdm_pputcw('O');
        zmdm_pputcw('O');
	flushmo();

      case ZCAN:
      case TIMEOUT:
	return;
    }
  }
}


/* Fill buffer with blklen chars */

static int near
zfilbuf(void)
{
  int n;

  n = fread(txbuf, 1, blklen, in);

  if (n < blklen)
    Eofseen = 1;

  return n;
}



/* Send send-init information

 * Returns OK if ZSINIT info sent successfully.
 * Returns ERROR if ZSINIT transfer failed.
 *
 */

static int near
sendzsinit(void)
{
  int c;
  static char Myattn[] = "";

  if (!Zctlesc || (Rxflags & TESCCTL))
    return OK;

  z_errors = 0;

  for (;;)
  {
    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
      return ERROR;

    stohdr(0L);

    if (Zctlesc)
    {
      Txhdr[ZF0] |= TESCCTL;
      zshhdr(ZSINIT, Txhdr);
    }
    else
      zsbhdr(ZSINIT, Txhdr);

    zsdata(Myattn, 1 + strlen(Myattn), ZCRCW);

    c = zgethdr(Rxhdr, 1);

    switch (c)
    {
      case ZCAN:
	return ERROR;

      case ZACK:
	return OK;

      default:
        if (++z_errors > 19)
	  return ERROR;
	continue;
    }
  }
}





/*
 * Get the receiver's init parameters
 *
 * Returns OK (from sendzsinit()) if sucessful.
 * Returns ERROR if failed.
 *
 */

static int near
getzrxinit(void)
{
  int ch, n;

  dlogit(("@In Getzrxinit"));

  for (n = 10; --n >= 0;)
  {
    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
      return ERROR;

    dlogit(("@Calling zgethdr"));
    ch = zgethdr(Rxhdr, 1);
    dlogit(("@getzrxinit - got %d", ch));

    switch (ch)
    {
      case ZCHALLENGE:		/* Echo receiver's challenge numbr */
	stohdr(Rxpos);
	zshhdr(ZACK, Txhdr);
	continue;

      case ZCOMMAND:		/* They didn't see out ZRQINIT */
	stohdr(0L);
	zshhdr(ZRQINIT, Txhdr);
	continue;

      case ZRINIT:
        Rxflags = 0xff & Rxhdr[ZF0];
	Txfcs32 = (Wantfcs32 && (Rxflags & CANFC32));
	Zctlesc |= Rxflags & TESCCTL;

        Rxbuflen = (0xff & Rxhdr[ZP0]) + ((0xff & Rxhdr[ZP1]) << 8);

	if (!(Rxflags & CANFDX))
	  Txwindow = 0;

	mode(2);		/* Set cbreak, XON/XOFF, etc. */

	/* Use 1024 byte frames if no sample/interrupt */

	/*if (Rxbuflen < 32 || Rxbuflen > 1024)
	   Rxbuflen = 1024; */

	/* Override to force shorter frame length */

	if (Rxbuflen && (Rxbuflen > Tframlen) && (Tframlen >= 32))
	  Rxbuflen = Tframlen;

	if (!Rxbuflen && (Tframlen >= 32) && (Tframlen <= 1024))
	  Rxbuflen = Tframlen;

	/* Set initial subpacket length */

	if (blklen < 1024)
	{			/* Command line override? */
	  if (baud >= 2400)
	    blklen = 1024;
	  else if (baud >= 1200)
	    blklen = 512;
	  else if (baud >= 300)
	    blklen = 256;
	}

	if (Rxbuflen && blklen > Rxbuflen)
	  blklen = Rxbuflen;

        dlogit(("@Rxbuflen=%d Tframlen=%d blklen=%d", Rxbuflen, Tframlen, blklen));
        dlogit(("@Txwindow = %u Txwspac = %d", Txwindow, Txwspac));

	ch = sendzsinit();
        dlogit(("@getzrxinit - returning (sendzsinit returned %d)", ch));
	return ch;

      case ZCAN:
      case TIMEOUT:
	return ERROR;

      case ZRQINIT:
	if (Rxhdr[ZF0] == ZCOMMAND)
	  continue;

      default:
	zshhdr(ZNAK, Txhdr);
	continue;
    }
  }

  dlogit(("@Done getzrxinit - ERROR"));

  return ERROR;
}


/*
 * Respond to receiver's complaint, get back in sync with receiver
 */

static int near
getinsync(int flag)
{
  int c;

  dlogit(("@Entering getinsync"));

  for (;;)
  {
    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
      return ERROR;

    c = zgethdr(Rxhdr, 0);

    dlogit(("@getinsync got %d", c));

    switch (c)
    {
      case ZCAN:
      case ZABORT:
      case ZFIN:
      case TIMEOUT:
	return ERROR;

      case ZRPOS:
        dumpmo();

	clearerr(in);		/* In case file EOF seen */

        dlogit(("@getinsync seeking to offset %ld", Rxpos));

	if (fseek(in, Rxpos, SEEK_SET))
	  return ERROR;

	Eofseen = 0;
	bytcnt = Lrxpos = Txpos = Rxpos;

	if (Lastsync == Rxpos)
	{
	  if (++Beenhereb4 > 4)
	    if (blklen > 32)
	      blklen /= 2;
	}

	Lastsync = Rxpos;
	return c;

      case ZACK:
	Lrxpos = Rxpos;

	if (flag || Txpos == Rxpos)
	  return ZACK;
	continue;

      case ZRINIT:
      case ZSKIP:
        fclose(in);
        return c;

      case ERROR:
      default:
	zsbhdr(ZNAK, Txhdr);
	continue;
    }
  }
}




/* Send the data in the file */

static int near
zsendfdata(void)
{
  int c, e, n;
  int newcnt;
  long tcount = 0;
  int junkcount;		/* Counts garbage chars received by TX */

  Lrxpos = 0;
  junkcount = 0;
  Beenhereb4 = FALSE;

  /* To kludge around original setjmp */

  goto after_error_code;

somemore:
waitack:

  junkcount = 0;
  c = getinsync(0);

gotack:
  switch (c)
  {
    default:
    case ZCAN:
      fclose(in);
      return ERROR;
    case ZSKIP:
      fclose(in);
      return c;
    case ZACK:
    case ZRPOS:
      break;
    case ZRINIT:
      return OK;
  }

  dlogit(("@doing rdchk"));

  /*
   * If the reverse channel can be tested for data,
   *  this logic may be used to detect error packets
   *  sent by the receiver, in place of setjmp/longjmp
   *  rdchk() returns non 0 if a character is available
   */

  while (rdchk() && carrier() && !ZmQueryLocalAbort())
  {
    switch (readline(1))
    {
      case CAN:
      case ZPAD:
	c = getinsync(1);
        dlogit(("@readchk gotack"));
        goto gotack;

      case XOFF:		/* Wait a while for an XON */
      case XOFF | 0x80:
        dlogit(("@readchk got an xoff; continuing"));
        readline(100);
    }
  }

  dlogit(("@done rdchk"));

after_error_code:		/*SJD Mon  04-11-1994  20:20:42 */

  newcnt = Rxbuflen;
  Txwcnt = 0;
  stohdr(Txpos);
  zsbhdr(ZDATA, Txhdr);

  do
  {
    n = zfilbuf();
/*    dlogit(("@zfilbuf got %ld - blklen=%ld, newcnt=%ld, Rxbuflen=%ld",
   (long)n, (long)blklen, (long)newcnt, (long)Rxbuflen)); */

    if (Eofseen)
      e = ZCRCE;
    else if (junkcount > 3)
      e = ZCRCW;
    else if (bytcnt == Lastsync)
      e = ZCRCW;
    else if (Rxbuflen && (newcnt -= n) <= 0)
     e = ZCRCW;
    else if (Txwindow && (Txwcnt += n) >= Txwspac)
    {
      Txwcnt = 0;
      e = ZCRCQ;
    }
    else
      e = ZCRCG;

    /* Display status screen */

    ZmStatData(Txpos, n, Crc32t);

    /* Leech Zmodem check */

    if (Txpos+n >= zfilesize-1)
      zslugbait=TRUE;

    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
    {
      fclose(in);
      return ERROR;
    }

    zsdata(txbuf, n, e);

    bytcnt = Txpos += n;

    if (e == ZCRCW)
      goto waitack;

    /*
       * If the reverse channel can be tested for data,
       *  this logic may be used to detect error packets
       *  sent by the receiver, in place of setjmp/longjmp
       *  rdchk() returns non 0 if a character is available
     */
    /*mdm_fflush();*/

    while (rdchk())
    {
      dlogit(("@test2 got rdchk"));

      switch (readline(1))
      {
	case CAN:
	case ZPAD:
	  c = getinsync(1);

	  if (c == ZACK)
	    break;

          dlogit(("@test2 doing flushmo"));
          flushmo(); /*mdm_fflush();*/ /*SJD Sat  04-16-1994  23:22:23 */
          dlogit(("@test2 done flushmo"));

	  /* zcrce - dinna wanna starta ping-pong game */
	  zsdata(txbuf, 0, ZCRCE);
	  goto gotack;

	case XOFF:		/* Wait a while for an XON */
        case XOFF | 0x80:
	  readline(100);

	default:
	  ++junkcount;
      }
    }

    if (Txwindow)
    {
      while ((tcount = Txpos - Lrxpos) >= Txwindow)
      {
        dlogit(("@%ld window >= %u", tcount, Txwindow));

	if (e != ZCRCQ)
	  zsdata(txbuf, 0, e = ZCRCQ);

	c = getinsync(1);

	if (c != ZACK)
	{
	  mdm_fflush();
	  zsdata(txbuf, 0, ZCRCE);
	  goto gotack;
	}
      }

      dlogit(("@window = %ld", tcount));
    }
  }
  while (!Eofseen);

  for (;;)
  {
    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
    {
      fclose(in);
      return ERROR;
    }

    stohdr(Txpos);
    zsbhdr(ZEOF, Txhdr);

    switch (getinsync(0))
    {
      case ZACK:
	continue;

      case ZRPOS:
	goto somemore;

      case ZRINIT:
	return OK;

      case ZSKIP:
	fclose(in);
	return c;

      default:
	fclose(in);
	return ERROR;
    }
  }
}




/* Send file name and related info */

static int near
zsendfile(char *buf, int blen)
{
  int c;
  unsigned long crc;

  dlogit(("@Entering ZSendFile"));

  for (;;)
  {
    Txhdr[ZF0] = LZCONV;        /* file conversion request */
    Txhdr[ZF1] = LZMANAG;       /* file management request */
    Txhdr[ZF2] = LZTRANS;       /* file transport request */
    Txhdr[ZF3] = 0;

    dlogit(("@ZSendFile sending ZFILE"));

    zsbhdr(ZFILE, Txhdr);
    dlogit(("@ZSendFile sending ZCRCW"));
    zsdata(buf, blen, ZCRCW);

  again:

    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
      return ERROR;

    dlogit(("@ZSendFile calling zgethdr"));
    c = zgethdr(Rxhdr, 1);
    dlogit(("@ZSendFile got zgethdr=%d", c));

    switch (c)
    {
      case ZRINIT:
	while ((c = readline(50)) > 0)
	  if (c == ZPAD)
	  {
	    goto again;
	  }
	/* **** FALL THRU TO **** */

      default:
	continue;

      case ZCAN:
      case TIMEOUT:
      case ZABORT:
      case ZFIN:
        dlogit(("@ZSendFile exiting - got error %d", c));
	return ERROR;

      case ZCRC:
	crc = 0xFFFFFFFFL;

	while (((c = getc(in)) != EOF) && --Rxpos)
	  crc = UPDC32(c, crc);

	crc = ~crc;

	clearerr(in);		/* Clear EOF */
	fseek(in, 0L, SEEK_SET);

	stohdr(crc);
	zsbhdr(ZCRC, Txhdr);
	goto again;

      case ZSKIP:
        dlogit(("@ZSendFile got zskip"));
	fclose(in);
	return c;

      case ZRPOS:
        dlogit(("@ZSendFile got ZRPOS"));

	/*
	 * Suppress zcrcw request otherwise triggered by
	 * lastyunc==bytcnt
	 */

	if (Rxpos && fseek(in, Rxpos, SEEK_SET))
	  return ERROR;

        /* Set offset for throughput calculations */

        ThruSetStartOffset(Rxpos);

        Lastsync = (bytcnt = Txpos = Rxpos) - 1;

	c = zsendfdata();
        dlogit(("@ZSendFile exiting - zsendfdata returned %d", c));
	return c;
    }
  }
}



/*
 * generate and transmit pathname block consisting of
 *  pathname (null terminated),
 *  file length, mode time and file mode in octal
 *  as provided by the Unix fstat call.
 *  N.B.: modifies the passed name, may extend it!
 */

static int near
wctxpn(char *name, unsigned long ulFiles, unsigned long ulBytes)
{
  char *p, *q;
  struct stat f;

  q = NULL;

  strcpy(txbuf, No_Path(name));
  q = txbuf + strlen(txbuf) + 1;

  p = q;

  while (q < (txbuf + 1024))
    *q++ = 0;

  if (*name && fstat(fileno(in), &f) != -1)
    sprintf(p, "%lu %lo %o 0 %lu %lu", f.st_size, f.st_mtime,
	    f.st_mode, ulFiles, ulBytes);

  /* force 1k blocks if name won't fit in 128 byte block */

  if (txbuf[125])
    blklen = 1024;
  else
  {				/* A little goodie for IMP/KMD */
    txbuf[127] = (f.st_size + 127) >> 7;
    txbuf[126] = (f.st_size + 127) >> 15;
  }

  /* Display the filename */

  zfilesize=f.st_size;
  ZmStatFile(name, f.st_size, Crc32);

  /* Send the file */

  return zsendfile(txbuf, 1 + strlen(p) + (p - txbuf));
}


/* Send a single file using Zmodem (assuming protocol already started)

 * Returns:
 *
 *  ERROR - if transfer failed
 *  ZSKIP - if file failed
 *  OK    - if file transferred successfully
 */

static int near
wcs(char *oname, unsigned long ulFiles, unsigned long ulBytes)
{
  int c;
  struct stat f;
  char name[PATHLEN];

  strcpy(name, oname);

  if ((in = shfopen(oname, fopen_readb, O_RDONLY | O_BINARY)) == NULL)
  {
    ++errcnt;
    return ZSKIP;		/* pass over it, there may be others */
  }

  Eofseen = 0;

  /* Check for directory or block special files */

  fstat(fileno(in), &f);

  c = f.st_mode & S_IFMT;

  if (c == S_IFDIR)
  {
    fclose(in);
    return ERROR;
  }

  switch (wctxpn(name, ulFiles, ulBytes))
  {
    case ERROR:
      return ERROR;

    case ZSKIP:
      return ZSKIP;
  }

  return OK;
}



/* Initialize a Zmodem session.  Send ZRQINIT frames until we time out
 * or see a ZRINIT from the receiver.
 *
 * Returns:
 *
 *   OK      - if transfer successfully initialized
 *   CAN     - if transfer cancelled
 *   ERROR   - if unrecoverable error occured in init
 *   TIMEOUT - if no transfer started
 */

static int
ZmInitSession(void)
{
  int ch, last;			/* Current and last characters received */
  int iSendTries = 8;		/* Wait up to a minute to start xfer */
  long lSendTimer;

  zmputs("rz\r");

  Rxtimeout = 600;
  Rxbuflen = 16384;
  Tframlen = 0;
  Rxflags = 0;
  errcnt = 0;
  blklen = 128;
  Wantfcs32 = TRUE;
  Zctlesc = FALSE;
  last = 0;
  z_errors = 0;

  /* Repeat up to eight times... */

  do
  {
    /* Send a ZRQINIT header every eight seconds */

    lSendTimer = timerset(800);
    stohdr(0L);
    zshhdr(ZRQINIT, Txhdr);

    do
    {
      /* Check if sysop hit <esc> */

      if (ZmDoLocalAbort())
        return ERROR;

      switch (ch = readline(10))
      {
	case ZPAD:
	  if (getzrxinit() != OK)
	    return ERROR;

	  return OK;

	case CAN:
	  if ((ch = readline(20)) == CAN && last == CAN)
	    return CAN;

	case TIMEOUT:		/* No action on timeout */
	  break;
      }

      last = ch;
    }
    while (!timeup(lSendTimer));
  }
  while (--iSendTries);

  return TIMEOUT;
}


/* Send one file using Zmodem.

 * If szName == NULL && fInit:  initialize Zmodem session
 * If szName == NULL && !fInit: terminate Zmodem session
 * if szName != NULL:           send specified file
 *
 */

int 
ZmodemSendFile(char *szName, int fInit,
	       unsigned long ulFiles, unsigned long ulBytes)
{
  int rc;

  fSending = TRUE;

  /* If no filename, we are either doing start-up or ending stuff */

  ThruStart();

  /* No Leech Zmodem attempt yet */

  zslugbait=FALSE;

  if (!szName)
  {
    if (fInit)
    {
      ZmStatInit();
      ZmVarsInit();
      mode(1);
      return ZmInitSession();
    }
    else
    {
      saybibi();
      mode(0);
      return OK;
    }
  }

  bytcnt = -1;

  rc=wcs(szName, ulFiles, ulBytes);

  /* Leech Zmodem check */

  if (zslugbait && rc != OK)
    rc=-100;

  return rc;
}

