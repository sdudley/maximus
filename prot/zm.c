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

/* $Id: zm.c,v 1.3 2004/01/22 08:04:28 wmcbrine Exp $ */

/*
 *   Z M . C
 *    ZMODEM protocol primitives
 *    05-09-88  Chuck Forsberg Omen Technology Inc
 *
 * Entry point Functions:
 *      zsbhdr(type, hdr) send binary header
 *      zshhdr(type, hdr) send hex header
 *      zgethdr(hdr, eflag) receive header - binary or hex
 *      zsdata(buf, len, frameend) send data
 *      zrdata(buf, len) receive data
 *      stohdr(pos) store position data in Txhdr
 *      long rclhdr(hdr) recover position offset from header
 */

#define MAX_LANG_protocols
#define INCL_DOSDEVIOCTL
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "zmodem.h"
#include "zsjd.h"
#include "pdata.h"
#include "keys.h"

#ifdef DEBUGZ
  #if defined(OS_2)
  /*  #define zmdm_status() 0*/
    static int near zmdm_status(void)
    {
      BYTE bStatus=0;
      BYTE bCtrlSignals=0;
      BYTE bCtrlSignalsOut=0;
      RXQUEUE rxq;
      HFILE hf=ComGetFH(hcModem);
      OS2UINT rc;


  #ifdef __FLAT__
      ULONG ulRecvLen;
      ULONG ulPLen;

      #define GenericIOCtl(hf, class, func, var) (ulPLen=0, ulRecvLen=sizeof(var),\
                                                 DosDevIOCtl(hf, class, func,  \
                                                             NULL, 0L, &ulPLen,\
                                                             &var, sizeof(var), \
                                                             &ulRecvLen))
  #else
      #define GenericIOCtl(hf, class, func, var) DosDevIOCtl(&var, 0L, func,   \
                                                             class, hf)
  #endif

      rc=GenericIOCtl(hf, IOCTL_ASYNC, ASYNC_GETCOMMSTATUS, bStatus);

      if (rc != 0)
        logit("!IOCtl GETCS rc=%d", rc);



      rc=GenericIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMINPUT, bCtrlSignals);

      if (rc != 0)
        logit("!IOCtl GETMI=%d", rc);



      rc=GenericIOCtl(hf, IOCTL_ASYNC, ASYNC_GETMODEMOUTPUT, bCtrlSignalsOut);

      if (rc != 0)
        logit("!IOCtl GETMO=%d", rc);



      rc=GenericIOCtl(hf, IOCTL_ASYNC, ASYNC_GETOUTQUECOUNT, rxq);

      if (rc != 0)
        logit("!IOCtl GETOQC=%d", rc);


      /*logit("!Async status=%02x, input=%02x, output=%02x numch=%d qsize=%d",
            bStatus, bCtrlSignals, bCtrlSignalsOut,
            rxq.cch, rxq.cb);*/

      return bStatus;
    }
  #elif defined(NT) /* zmdm_status only used for debugging purposes */
    #define zmdm_status() 0
  #else
    #define zmdm_status() mdm_status()
  #endif
#endif /* DEBUGZ */

#define ZmUpdCrc(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)


#define ZRWINDOW 1400

int Rxtimeout;                  /* Tenths of seconds to wait for something */
static int Rxframeind;          /* ZBIN ZBIN32, or ZHEX type of frame received */
static int Rxtype;              /* Type of header received */
int Rxcount;			/* Count of data bytes received */
char Rxhdr[4];			/* Received header */
char Txhdr[4];			/* Transmitted header */
long Rxpos;			/* Received file position */
long Txpos;			/* Transmitted file position */
int Txfcs32;                    /* TRUE means send binary frames with 32 bit FCS */
int Crc32t;			/* Display flag indicating 32 bit CRC being sent */
int Crc32;			/* Display flag indicating 32 bit CRC being received */
char *Attn=NULL; /*[ZATTNLEN + 1];*/ /* Attention string rx sends to tx on err */

static int lastsent;                /* Last char we sent */
static int Lleft;                      /* number of characters in linbuf */
static char *linbuf=NULL; /*[HOWMANY];*/

int Zctlesc=FALSE;              /* Encode control characters? */
int z_errors;                   /* Count of errors while receiving this file */
long Rxbytes;                   /* Current receive position */
int fSending;                   /* TRUE if we are sending a file;
                                 * FALSE if we are receiving a file. */

#ifdef DEBUGZ
static int Verbose = 4;
#endif

static char *frametypes[] =
{
  "Carrier Lost",		/* -3 */
  "TIMEOUT",			/* -2 */
  "ERROR",			/* -1 */
#define FTOFFSET 3
  "ZRQINIT",
  "ZRINIT",
  "ZSINIT",
  "ZACK",
  "ZFILE",
  "ZSKIP",
  "ZNAK",
  "ZABORT",
  "ZFIN",
  "ZRPOS",
  "ZDATA",
  "ZEOF",
  "ZFERR",
  "ZCRC",
  "ZCHALLENGE",
  "ZCOMPL",
  "ZCAN",
  "ZFREECNT",
  "ZCOMMAND",
  "ZSTDERR",
  "*Error*"
#define FRTYPES 22		/* Total number of frame types in this array */
			/*  not including psuedo negative entries */
};


#ifdef DEBUGZ
static char *Zendnames[] =
{
  "ZCRCE", "ZCRCG", "ZCRCQ", "ZCRCW"
};
#endif


static void near zsendline(int c);
static int near zrbhdr(char *hdr);
static int near zrhhdr(char *hdr);

#define MAX_TXBUF 128

static unsigned char *szTxBuf=NULL; /*[MAX_TXBUF];*/  /* Static buf for output text */
static int fLocalAbort;                   /* TRUE if sysop pressed <esc> */

unsigned char *szTxPtr;                   /* Pointer to write pos in szTxBuf */
int iTxBufLeft;                           /* Bytes available in TX buffer */


/* Routine to initialize static variables.  This must be called before any
 * of the Zmodem send or receive routines are called.
 */

int ZmInitStatics(void)
{
  int ok=TRUE;

  /* Init statics in rz.c and sz.c */

  ok=ZmSzInitStatics();

  ok=ok && ZmRzInitStatics();

  /* Now init statics in zm.c too */

  Attn=malloc(ZATTNLEN+1);
  linbuf=malloc(HOWMANY);
  szTxBuf=malloc(MAX_TXBUF);

  /* Set Attn to null so that we don't blast garbage at the receiver */

  *Attn=0;
  *linbuf=0;
  *szTxBuf=0;

  z_errors=0;

  ok=ok && (Attn && linbuf && szTxBuf);

  if (!ok)
    ZmDeinitStatics();

  return ok;
}



/* Routine to free the memory allocated by ZmInitStatics */

void ZmDeinitStatics(void)
{
  /* Tell the sz.c and rz.c modules to do the same */

  ZmSzDeinitStatics();
  ZmRzDeinitStatics();

  if (Attn)
  {
    free(Attn);
    Attn=NULL;
  }

  if (linbuf)
  {
    free(linbuf);
    linbuf=NULL;
  }

  if (szTxBuf)
  {
    free(szTxBuf);
    szTxBuf=NULL;
  }
}



/* Check for user pressing <esc> */

int ZmQueryLocalAbort(void)
{
  /* Get characters as they are pressed */

  while (khit())
  {
    if (kgetch()==K_ESC)
    {
      fLocalAbort=TRUE;
      return TRUE;
    }
  }

  if (!carrier())
    return TRUE;

  if (fSending && (prm.flags2 & FLAG2_STRICTXFER) && timeleft() <= 0)
  {
    if (!fLocalAbort)
    {
      fLocalAbort = TRUE;
      logit(log_dl_exceeds_time);
    }

    return TRUE;
  }

  return fLocalAbort;
}



/* Check if we should abort, and if we should, send a cancel string */

int ZmDoLocalAbort(void)
{
  if (!ZmQueryLocalAbort())
    return FALSE;

  if (carrier())
    canit();

  return TRUE;
}


/* Send the contents of the TX buffer */

void
sendmo(void)
{
  int iCount=szTxPtr-szTxBuf;
  long t=0;

  szTxPtr=szTxBuf;

  while (iCount > 0)
  {
    int got;

    got=mdm_blockwrite(iCount, szTxPtr);

    if (got <= 0)
    {
      if (!t)
        t=timerset(10000);

      if (timeup(t))
      {
        t=timerset(10000);
#ifdef DEBUGZ
        dlogit(("!Timeout in sendmo - s=%x", zmdm_status()));
#endif
        Mdm_Flow(FLOW_OFF);
        Mdm_Flow(FLOW_NO_CCK);
      }

      Giveaway_Slice();
    }
    else
    {
#if 0
      int i;

      for (i=0; i < got; i++)
        dlogit(("@Tx %#02x '%c'", szTxPtr[i], szTxPtr[i]));
#endif
      iCount -= got;
      szTxPtr += got;
    }

    /* Break out of loop if no carrier or if sysop pressed <esc> */

    if (!carrier() || ZmQueryLocalAbort())
      break;
  }

  szTxPtr=szTxBuf;
  iTxBufLeft=MAX_TXBUF;
}


/* Send the internal TX buffer, and then flush the TX buffer in the
 * OS or FOSSIL.
 */

void flushmo(void)
{
  /* Send the contents of the internal TX buffer */

  long t=0;

  sendmo();

  /* Now wait for the system output buffer to empty */

  while (!out_empty() && carrier() && !ZmQueryLocalAbort())
  {
    if (!t)
      t=timerset(10000);

    if (timeup(t))
    {
      t=timerset(10000);
#ifdef DEBUGZ
      dlogit(("!Timeout in flushmo - s=%x", zmdm_status()));
#endif
      Mdm_Flow(FLOW_OFF);
      Mdm_Flow(FLOW_NO_CCK);
    }

    Giveaway_Slice();
  }
}



/* Dump the transmit buffer */

void dumpmo(void)
{
  szTxPtr=szTxBuf;
  iTxBufLeft=MAX_TXBUF;

  mdm_dump(DUMP_OUTPUT);
}



/* Send a (buffered) character to the modem */

#if 0
void
zmdm_pputcw(int c)
{
  /* Flush buffer if no space left */

  if (iTxBufLeft--==0)
  {
    sendmo();
    iTxBufLeft--;
  }

  *szTxPtr++=c;
  return;
}
#endif


/*
 * Log an error
 */

static void near
zperr(char *s, ...)
{
  char out[PATHLEN*2];
  va_list va;

  sprintf(out, log_retry_num, ++z_errors);

  va_start(va, s);
  vsprintf(out+strlen(out), s, va);
  va_end(va);

  logit(out);
}



/* Send ZMODEM binary header hdr of type type */

static void near
zsbh32(char *hdr, int type)
{
  int n;
  long crc;

  /* Display the status header */

  ZmStatHdr(TRUE, type, frametypes[type+FTOFFSET], rclhdr(hdr));

  zmdm_pputcw(ZBIN32);
  zsendline(type);

  crc = 0xffffffffL;
  crc = UPDC32(type, crc);

  for (n = 4; --n >= 0; ++hdr)
  {
    crc = UPDC32((0xff & *hdr), crc);
    zsendline(*hdr);
  }

  crc = ~crc;

  for (n = 4; --n >= 0;)
  {
    zsendline((int)crc);
    crc >>= 8;
  }
}


/* Send ZMODEM binary header hdr of type type */

void
zsbhdr(int type, char *hdr)
{
  int n;
  unsigned short crc;

  zmdm_pputcw(ZPAD);
  zmdm_pputcw(ZDLE);

  dlogit(("@zsendhdr: %s %lx", frametypes[type+FTOFFSET], rclhdr(hdr)));

  if (Crc32t = Txfcs32)
    zsbh32(hdr, type);
  else
  {
    /* Display the status header */

    ZmStatHdr(TRUE, type, frametypes[type+FTOFFSET], rclhdr(hdr));

    zmdm_pputcw(ZBIN);
    zsendline(type);
    crc = ZmUpdCrc(type, 0);

    for (n = 4; --n >= 0; ++hdr)
    {
      zsendline(*hdr);
      crc = ZmUpdCrc((0xff & *hdr), crc);
    }
    crc = ZmUpdCrc(0, ZmUpdCrc(0, crc));
    zsendline(crc >> 8);
    zsendline(crc);
  }

  if (type != ZDATA)
    flushmo();
}


/* Send a byte as two hex digits */

static void near
zputhex(int c)
{
  static char digits[] = "0123456789abcdef";

#ifdef DEBUGZ
  if (Verbose > 8)
    dlogit(("@zputhex: %02X", c));
#endif

  zmdm_pputcw(digits[(c & 0xf0) >> 4]);
  zmdm_pputcw(digits[(c) & 0x0f]);
}



/* Send ZMODEM HEX header hdr of type type */

void zshhdr(int type, char *hdr)
{
  int n;
  unsigned short crc;

  /* Display the status header */

  dlogit(("@zsendhdr: %s %lx", frametypes[type+FTOFFSET], rclhdr(hdr)));
  ZmStatHdr(TRUE, type, frametypes[type+FTOFFSET], rclhdr(hdr));

  zmdm_pputcw(ZPAD);
  zmdm_pputcw(ZPAD);
  zmdm_pputcw(ZDLE);
  zmdm_pputcw(ZHEX);

  zputhex(type);
  Crc32t = 0;

  crc = ZmUpdCrc(type, 0);

  for (n = 4; --n >= 0; ++hdr)
  {
    zputhex(*hdr);
    crc = ZmUpdCrc((0xff & *hdr), crc);
  }

  crc = ZmUpdCrc(0, ZmUpdCrc(0, crc));

  zputhex(crc >> 8);
  zputhex(crc);

  /* Make it printable on remote machine */

  zmdm_pputcw(015);
  zmdm_pputcw(0x8a);

  /*
   * Uncork the remote in case a fake XOFF has stopped data flow
   */

  if (type != ZFIN && type != ZACK)
  {
    zmdm_pputcw(0x11);
  }

  flushmo();
}


/* Send binary array buf of length length, with 32-bit ending ZDLE */

static void near
zsda32(char *buf, int length, int frameend)
{
  int c;
  long crc;

  crc = 0xffffffffL;

  while (--length >= 0)
  {
    c = *buf++ & 0xff;

    if (c & 0x60)
    {
      zmdm_pputcw(lastsent = c);
    }
    else
    {
      zsendline(c);
    }

    crc = UPDC32(c, crc);
  }

  zmdm_pputcw(ZDLE);
  zmdm_pputcw(frameend);

  crc = UPDC32(frameend, crc);
  crc = ~crc;

  for (length = 4; --length >= 0;)
  {
    zsendline((int)crc);
    crc >>= 8;
  }
}


/*
 * Send binary array buf of length length, with ending ZDLE sequence frameend
 */

void 
zsdata(unsigned char *buf, int length, int frameend)
{
  unsigned short crc;

#ifdef DEBUGZ
  dlogit(("@zsdata: %d %s", length, Zendnames[frameend - ZCRCE & 3]));
#endif

  if (Crc32t)
    zsda32(buf, length, frameend);
  else
  {
    crc = 0;

    for (; --length >= 0; ++buf)
    {
      zsendline(*buf);
      crc = ZmUpdCrc((0xff & *buf), crc);
    }

    zmdm_pputcw(ZDLE);
    zmdm_pputcw(frameend);

    crc = ZmUpdCrc(frameend, crc);
    crc = ZmUpdCrc(0, ZmUpdCrc(0, crc));

    zsendline(crc >> 8);
    zsendline(crc);
  }

  if (frameend == ZCRCW)
  {
    zmdm_pputcw(XON);
    flushmo();
  }
}



/*
 * Read a byte, checking for ZMODEM escape encoding
 *  including CAN*5 which represents a quick abort
 */

static int near
zdlread(void)
{
  int c;
#ifdef DEBUGZ
  static long lInvoke=0L;

  lInvoke++;
#endif

again:

  /* Quick check for non control characters */

  if ((c = readline(Rxtimeout)) & 0x60)
    return c;

  switch (c)
  {
    case ZDLE:
      break;

    case 0x13:
    case 0x93:
    case 0x11:
    case 0x91:
#ifdef DEBUGZ
      dlogit(("@1! got escaped %#02x (%ld)", c, lInvoke));
#endif
      goto again;

    default:
      if (Zctlesc && !(c & 0x60))
      {
#ifdef DEBUGZ
        dlogit(("@2! got escaped %#02x (%ld)", c, lInvoke));
#endif
        goto again;
      }
      return c;
  }

again2:

  if ((c = readline(Rxtimeout)) < 0)
    return c;

  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;

  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;

  if (c == CAN && (c = readline(Rxtimeout)) < 0)
    return c;

  switch (c)
  {
    case CAN:
      return GOTCAN;

    case ZCRCE:
    case ZCRCG:
    case ZCRCQ:
    case ZCRCW:
      return (c | GOTOR);

    case ZRUB0:
      return 0x7f;

    case ZRUB1:
      return 0xff;

    case 0x13:
    case 0x93:
    case 0x11:
    case 0x91:
#ifdef DEBUGZ
      dlogit(("@3! got escaped %#02x (%ld)", c, lInvoke));
#endif
      goto again2;

    default:
      if (Zctlesc && !(c & 0x60))
      {
        goto again2;
      }

      if ((c & 0x60) == 0x40)
        return (c ^ 0x40);
      break;
  }

#ifdef DEBUGZ
  dlogit(("@4! got bad escape (%ld)", lInvoke));
#endif
  zperr(bad_escape_seq, c);

  return ERROR;
}



/* Receive array buf of max length with ending ZDLE sequence and 32-bit CRC */

static int near
zrdat32(char *buf, int length)
{
  int c;
  long crc;
  char *end;
  int d;

  crc = 0xFFFFFFFFL;
  Rxcount = 0;
  end = buf + length;

  while (buf <= end)
  {
    if ((c = zdlread()) & ~0xff)
    {
    crcfoo:

      switch (c)
      {
        case GOTCRCE:
        case GOTCRCG:
        case GOTCRCQ:
        case GOTCRCW:
          d = c;
          c &= 0xff;

          crc = UPDC32(c, crc);

          if ((c = zdlread()) & ~0xff)
            goto crcfoo;

          crc = UPDC32(c, crc);

          if ((c = zdlread()) & ~0xff)
            goto crcfoo;

          crc = UPDC32(c, crc);

          if ((c = zdlread()) & ~0xff)
            goto crcfoo;

          crc = UPDC32(c, crc);

          if ((c = zdlread()) & ~0xff)
            goto crcfoo;

          crc = UPDC32(c, crc);

          if (crc != 0xDEBB20E3)
          {
            zperr(bad_crc_at, Rxbytes);
            return ERROR;
          }

          Rxcount = length - (end - buf);

#ifdef DEBUGZ
          dlogit(("@zrdat32: %d %s", Rxcount,
                Zendnames[d - GOTCRCE & 3]));
#endif
          return d;

        case GOTCAN:
          zperr(sender_cancelled);
          return ZCAN;

        case TIMEOUT:
          zperr(transfer_timeout);
          return c;

        default:
          zperr(bad_data_subpkt);
          return c;
      }
    }

    *buf++ = c;
    crc = UPDC32(c, crc);
  }

  zperr(data_subpkt_long);
  return ERROR;
}


/*
 * Receive array buf of max length with ending ZDLE sequence
 *  and CRC.  Returns the ending character or error code.
 *  NB: On errors may store length+1 bytes!
 */

int 
zrdata(char *buf, int length)
{
  int c;
  unsigned short crc;
  char *end;
  int d;

  if (Rxframeind == ZBIN32)
    return zrdat32(buf, length);

  crc = Rxcount = 0;
  end = buf + length;

  while (buf <= end)
  {
    if ((c = zdlread()) & ~0xff)
    {
    crcfoo:

      switch (c)
      {
	case GOTCRCE:
	case GOTCRCG:
	case GOTCRCQ:
	case GOTCRCW:
          crc = ZmUpdCrc((d = c) & 0xff, crc);

          if ((c = zdlread()) & ~0xff)
	    goto crcfoo;

          crc = ZmUpdCrc(c, crc);

          if ((c = zdlread()) & ~0xff)
	    goto crcfoo;

          crc = ZmUpdCrc(c, crc);

	  if (crc & 0xFFFF)
	  {
            zperr(bad_crc_at, Rxbytes);
            return ERROR;
	  }

	  Rxcount = length - (end - buf);

#ifdef DEBUGZ
          dlogit(("@zrdata: %d  %s", Rxcount,
                Zendnames[d - GOTCRCE & 3]));
#endif
	  return d;

	case GOTCAN:
          zperr(sender_cancelled);
          return ZCAN;

	case TIMEOUT:
          zperr(transfer_timeout);
          return c;

	default:
          zperr(bad_data_subpkt);
          return c;
      }
    }

    *buf++ = c;
    crc = ZmUpdCrc(c, crc);
  }

  zperr(data_subpkt_long);
  return ERROR;
}



/* Receive a binary style header (type and position) with 32 bit FCS */

static int near
zrbhdr32(char *hdr)
{
  int c, n;
  long crc;

  if ((c = zdlread()) & ~0xff)
    return c;

  Rxtype = c;
  crc = 0xFFFFFFFFL;
  crc = UPDC32(c, crc);

#ifdef DEBUGZ
/*  dlogit(("@zrbhdr32 c=%X  crc=%lX", c, crc));*/
#endif

  for (n = 4; --n >= 0; ++hdr)
  {
    if ((c = zdlread()) & ~0xff)
      return c;

    crc = UPDC32(c, crc);
    *hdr = c;

#ifdef DEBUGZ
/*    dlogit(("@zrbhdr32 c=%X  crc=%lX", c, crc));*/
#endif
  }

  for (n = 4; --n >= 0;)
  {
    if ((c = zdlread()) & ~0xff)
      return c;
    crc = UPDC32(c, crc);

#ifdef DEBUGZ
/*    dlogit(("@zrbhdr32 c=%X  crc=%lX", c, crc));*/
#endif
  }

  if (crc != 0xDEBB20E3)
  {
    zperr(bad_crc_hdr);
    return ERROR;
  }

  return Rxtype;
}


/*
 * Read a character from the modem line with timeout.
 *  Eat parity, XON and XOFF characters.
 */

static int near
noxrd7(void)
{
  int c;

  for (;;)
  {
    if ((c = readline(Rxtimeout)) < 0)
      return c;

    switch (c &= 0x7f)
    {
      case XON:
      case XOFF:
        continue;

      default:
        if (Zctlesc && !(c & 0x60))
          continue;

      case '\r':
      case '\n':
      case ZDLE:
        return c;
    }
  }
}


/*
 * Read a ZMODEM header to hdr, either binary or hex.
 *  eflag controls local display of non zmodem characters:
 *      0:  no display
 *      1:  display printing characters only
 *      2:  display all non ZMODEM characters
 *  On success, set Zmodem to 1, set Rxpos and return type of header.
 *   Otherwise return negative on error.
 *   Return ERROR instantly if ZCRCW sequence, for fast error recovery.
 */

int 
zgethdr(char *hdr, int eflag)
{
  int c, cancount;
  long n;

  n = ZRWINDOW + baud;      /* Max bytes before start of frame */

  Rxframeind = Rxtype = 0;

startover:
  cancount = 5;

again:

  /* Return immediate ERROR if ZCRCW sequence seen */

  switch (c = readline(Rxtimeout))
  {
    case RCDO:
    case TIMEOUT:
      goto fifi;

    case CAN:
    gotcan:
      if (--cancount <= 0)
      {
	c = ZCAN;
	goto fifi;
      }

      switch (c = readline(1))
      {
	case TIMEOUT:
	  goto again;
/*        case ZCRCW:
          dlogit(("@zgethdr received zcrcw"));
          c = ERROR;*/
	  /* **** FALL THRU TO **** */
	case RCDO:
	  goto fifi;
	default:
	  break;
	case CAN:
	  if (--cancount <= 0)
	  {
	    c = ZCAN;
	    goto fifi;
	  }
	  goto again;
      }
      /* **** FALL THRU TO **** */

    default:
    agn2:
      if (--n == 0)
      {
        zperr(garbage_count_exceeded);
        return (ERROR);
      }

      if (eflag && ((c &= 0x7f) & 0x60))
	/*bttyout(c) */ ;
      else if (eflag > 1)
	/*bttyout(c) */ ;

#ifdef UNIX
      fflush(stderr);
#endif

      goto startover;

    case ZPAD | 0x80:           /* This is what we want. */
      /*Not8bit = c;*/
      break;

    case ZPAD:			/* This is what we want. */
      break;
  }

  cancount = 5;

splat:

  switch (c = noxrd7())
  {
    case ZPAD:
      goto splat;

    case RCDO:
    case TIMEOUT:
      goto fifi;

    default:
      goto agn2;

    case ZDLE:			/* This is what we want. */
      break;
  }

  switch (c = noxrd7())
  {
    case RCDO:
    case TIMEOUT:
      goto fifi;

    case ZBIN:
      Rxframeind = ZBIN;
      Crc32 = FALSE;
      c = zrbhdr(hdr);
      break;

    case ZBIN32:
      Crc32 = Rxframeind = ZBIN32;
      c = zrbhdr32(hdr);
      break;

    case ZHEX:
      Rxframeind = ZHEX;
      Crc32 = FALSE;
      c = zrhhdr(hdr);
      break;

    case CAN:
      goto gotcan;

    default:
      goto agn2;
  }

  Rxpos = hdr[ZP3] & 0xff;
  Rxpos = (Rxpos << 8) + (hdr[ZP2] & 0xff);
  Rxpos = (Rxpos << 8) + (hdr[ZP1] & 0xff);
  Rxpos = (Rxpos << 8) + (hdr[ZP0] & 0xff);

fifi:


  /* Display a status message */

  ZmStatHdr(FALSE, c, frametypes[c+FTOFFSET], Rxpos);

  switch (c)
  {
    case GOTCAN:
      dlogit(("@zgethdr received gotcan"));
      c = ZCAN;
      /* **** FALL THRU TO **** */

    case ZNAK:
    case ZCAN:
    case ERROR:
    case TIMEOUT:
    case RCDO:
#ifdef DEBUGZ
      dlogit(("@zgethdr got %s", frametypes[c + FTOFFSET]));
#endif
      /* **** FALL THRU TO **** */

    default:
      if (c >= -3 && c <= FRTYPES)
        dlogit(("@zgethdr: %s %lx", frametypes[c + FTOFFSET], Rxpos));
      else
        dlogit(("@zgethdr: %d %lx", c, Rxpos));
  }

  return c;
}



/* Receive a binary style header (type and position) */

static int near
zrbhdr(char *hdr)
{
  int c, n;
  unsigned short crc;

  if ((c = zdlread()) & ~0xff)
    return c;

  Rxtype = c;
  crc = ZmUpdCrc(c, 0);

  for (n = 4; --n >= 0; ++hdr)
  {
    if ((c = zdlread()) & ~0xff)
      return c;

    crc = ZmUpdCrc(c, crc);
    *hdr = c;
  }

  if ((c = zdlread()) & ~0xff)
    return c;

  crc = ZmUpdCrc(c, crc);

  if ((c = zdlread()) & ~0xff)
    return c;

  crc = ZmUpdCrc(c, crc);

  if (crc & 0xFFFF)
  {
    zperr(bad_crc_hdr);
    return ERROR;
  }

  return Rxtype;
}


/* Decode two lower case hex digits into an 8 bit byte value */

static int near
zgeth1(void)
{
  int c, n;

  if ((c = noxrd7()) < 0)
    return c;

  n = c - '0';

  if (n > 9)
    n -= ('a' - ':');

  if (n & ~0xF)
    return ERROR;

  if ((c = noxrd7()) < 0)
    return c;

  c -= '0';

  if (c > 9)
    c -= ('a' - ':');

  if (c & ~0xF)
    return ERROR;

  c += (n << 4);

  return c;
}




#ifdef DEBUGZ
  /* Debugging wrapper for zgeth1() */

  static int near
  zgethex(void)
  {
    int c;

    c = zgeth1();

    if (Verbose > 8)
      dlogit(("@zgethex: %02X", c));

    return c;
  }
#else
  #define zgethex() zgeth1()
#endif






/* Receive a hex style header (type and position) */

static int near
zrhhdr(char *hdr)
{
  int c;
  unsigned short crc;
  int n;

  if ((c = zgethex()) < 0)
    return c;

  Rxtype = c;
  crc = ZmUpdCrc(c, 0);

  for (n = 4; --n >= 0; ++hdr)
  {
    if ((c = zgethex()) < 0)
      return c;
    crc = ZmUpdCrc(c, crc);
    *hdr = c;
  }

  if ((c = zgethex()) < 0)
    return c;

  crc = ZmUpdCrc(c, crc);

  if ((c = zgethex()) < 0)
    return c;

  crc = ZmUpdCrc(c, crc);

  if (crc & 0xFFFF)
  {
    zperr(bad_crc_hdr);
    return ERROR;
  }

  switch (c = readline(1))
  {
    case 0x8d:
      /*Not8bit = c;*/
      /* **** FALL THRU TO **** */

    case 0x0d:
      /* Throw away possible cr/lf */

      switch (c = readline(1))
      {
        case 0x0a:
          /*Not8bit |= c;*/
          ;
      }
  }

  return Rxtype;
}



/*
 * Send character c with ZMODEM escape sequence encoding.
 *  Escape XON, XOFF. Escape CR following @ (Telenet net escape)
 */

static void near
zsendline(int c)
{

  /* Quick check for non control characters */
  if (c & 0x60)
  {
    zmdm_pputcw(lastsent = c);
  }
  else
  {
    switch (c &= 0xff)
    {
      case ZDLE:
        zmdm_pputcw(ZDLE);
        zmdm_pputcw(lastsent = (c ^= 0x40));
	break;

      case 0x0d:
      case 0x8d:
        if (!Zctlesc && (lastsent & 0x7f) != '@')
	  goto sendit;
	/* **** FALL THRU TO **** */

      case 0x10:
      case 0x11:
      case 0x13:
      case 0x90:
      case 0x91:
      case 0x93:
        zmdm_pputcw(ZDLE);
        c ^= 0x40;

      sendit:
        zmdm_pputcw(lastsent = c);
	break;

      default:
        if (Zctlesc && !(c & 0x60))
	{
          zmdm_pputcw(ZDLE);
          c ^= 0x40;
	}
        zmdm_pputcw(lastsent = c);
    }
  }
}


/* Store long integer pos in Txhdr */

void 
stohdr(long pos)
{
  Txhdr[ZP0] = pos;
  Txhdr[ZP1] = pos >> 8;
  Txhdr[ZP2] = pos >> 16;
  Txhdr[ZP3] = pos >> 24;
}


/* Recover a long integer from a header */

long
rclhdr(char *hdr)
{
  long l;

  l = (hdr[ZP3] & 0xff);
  l = (l << 8) | (hdr[ZP2] & 0xff);
  l = (l << 8) | (hdr[ZP1] & 0xff);
  l = (l << 8) | (hdr[ZP0] & 0xff);
  return l;
}





/*
 * Send a string to the modem, processing for \336 (sleep 1 sec)
 *   and \335 (break signal)
 */

void
zmputs(char *s)
{
  int c;

  while (*s)
  {
    switch (c = *s++)
    {
      case '\336':
        Delay(100);
        continue;

      case '\335':
        sendbrk();
        continue;

      default:
        zmdm_pputcw(c);
    }
  }
}



/* send cancel string to get the other end to shut up */

void
canit(void)
{
  static char canistr[] =
  {
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
  };


  /* Make sure that the transmitter is not stuck */

  mode(1);
  dumpmo();
  Delay(50);
  mode(2);

  /* Output cancel string */

  zmputs(canistr);


  /* Make sure that it gets out */

  flushmo();

  Lleft = 0;                    /* Kill existing read buffer */
}


/*
 * This version of readline is reasoably well suited for
 * reading many characters.
 *  (except, currently, for the Regulus version!)
 *
 * timeout is in tenths of seconds
 */

int readline(int timeout)
{
  static unsigned char *cdq;    /* pointer for removing chars from linbuf */
  long lTimer;
  long t=0;

  if (Lleft-- > 0)
  {
#ifdef DEBUGZ
    if (Verbose > 8)
    {
      Lprintf("%02x ", *cdq & 0xff);
      vbuf_flush();
    }
#endif

    return (*cdq++ & 0xff);
  }

#ifdef DEBUGZ
  if (Verbose > 5)
  {
    Lprintf("Calling read: ");
    vbuf_flush();
  }
#endif

  cdq=linbuf;
  lTimer=timerset(timeout * 10);
  Lleft=0;

  do
  {
    int got;

    got=mdm_blockread(HOWMANY, cdq);

    if (got > 0)
    {
      Lleft += got;
      break;
    }

    if (!t)
      t=timerset(10000);

    if (timeup(t))
    {
      t=timerset(10000);
#ifdef DEBUGZ
      dlogit(("!Timeout in readline - s=%x", zmdm_status()));
#endif
      Mdm_Flow(FLOW_OFF);
      Mdm_Flow(FLOW_NO_CCK);
    }

    Giveaway_Slice();
  }
  while (!timeup(lTimer) && carrier() && !ZmQueryLocalAbort());

#ifdef DEBUGZ
  if (Verbose > 5)
    Lprintf("  Read returned %d bytes\n", Lleft);
#endif

  if (Lleft < 1)
    return TIMEOUT;

  --Lleft;

  /*if (Verbose > 8)
  {
    Lprintf("%02x ", *cdq & 0xff);
    vbuf_flush();
  }*/

  return (*cdq++ & 0xff);
}



/*
 * Purge the modem input queue of all characters
 */

void
purgeline(void)
{
  Lleft = 0;
  mdm_dump(DUMP_INPUT);
}


/* Initialize the static variables in zm.c for either a tx or rx session */

void ZmVarsInit(void)
{
  Lleft=0;
  szTxPtr=szTxBuf;
  iTxBufLeft=MAX_TXBUF;
  fLocalAbort=FALSE;
}

