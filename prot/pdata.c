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

/* $Id: pdata.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

/*# name=Binkley -=> Maximus interface routines, for protocol code.
*/

#define MAX_INCL_COMMS

#ifdef OS_2
#include <os2.h>
#endif

#define MAX_LANG_protocols
#define MAX_LANG_f_area
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <conio.h>
#include <io.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "pdatap.h"
#include "prog.h"
#include "pdata.h"
#include "xmodem.h"
#include "mm.h"
#include "zsjd.h"

static VWIN *gwXfer;    /* Zmodem transfer status window */

#define MAX_OB 128


static long lTimeStart = 0L;
static long lStartOffset = 0L;

/* Start a file transfer timing calculation */

void ThruStart(void)
{
  lTimeStart = time(NULL);
  lStartOffset = 0L;
}


/* Set the effective starting offset for the file (used for Zmodem
 * resume with ZRPOS)
 */

void ThruSetStartOffset(long lStartOfs)
{
  lStartOffset = lStartOfs;
}

void ThruLog(long lTotalBytes)
{
  long lTimeElapsed;
  long lBytesSent;
  long lCPS;

  /* Calculate total time elapsed and total bytes sent */

  lTimeElapsed = time(NULL) - lTimeStart;
  lBytesSent = lTotalBytes - lStartOffset;

  if (!lTimeElapsed)
    lTimeElapsed = 1L;

  if (lBytesSent < 0)
    lBytesSent = 0;

  lCPS = lBytesSent / lTimeElapsed;
  last_bps = lCPS * 10L;

  logit(log_dl_speed,
        lCPS,
        lBytesSent,
        lCPS*1000L/(long)baud);
}




/*--------------------------------------------------------------------------*/
/* UNIQUE_NAME                                                              */
/* Increments the suffix of a filename as necessary to make the name unique */
/*--------------------------------------------------------------------------*/
void unique_name (char *fname)
{
   static byte suffix[] = ".001";
   char *p;
   int n;

   if (fexist(fname))
      {                                          /* If file already exists...      */
      p = fname;
      while (*p && *p != '.')
         p++;                                    /* ...find the extension, if
                                                  * any  */
      for (n = 0; n < 4; n++)                    /* ...fill it out if
                                                  * neccessary   */
         if (!*p)
            {
            *p = suffix[n];
            *(++p) = '\0';
            }
         else p++;

      while (fexist(fname))                    /* ...If 'file.ext' exists
                                                * suffix++ */
         {
         p = fname + strlen (fname) - 1;
         for (n = 3; n--;)
            {
            if (!isdigit (*p))
               *p = '0';
            if (++(*p) <= '9')
               break;
            else *p-- = '0';
            }                                    /* for */
         }                                       /* while */
      }                                          /* if exist */
}                                                /* unique_name */

/*
            1         2         3         4         5         6
  012345678901234567890123456789012345678901234567890123456789012345
 Ú Download (Ymodem-G) ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
0³ Name:                                                           ³
1³ ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ³
2³ Block check     : Checksum        Transfer rate : 123456789 CPS ³
3³ Bytes to send   : 4,123,456,789   Time left     : 1234567890:12 ³
4³ Bytes sent      : 4,123,456,789   Time elapsed  :         12:12 ³
5³ ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ ³
6³ Status          :                                               ³
7ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

unsigned XferWinOpen(sword protocol, unsigned doing_dl)
{
  char temp[PATHLEN], temp2[PATHLEN];



  if (displaymode != VIDEO_IBM)
    gwXfer=NULL;
  else
  {
    gwXfer=WinOpen(-1, -1, 9, 67, BORDER_SINGLE, CWHITE | _BLUE,
                  CYELLOW | _BLUE, WIN_CENTRE | WIN_NOCSYNC | WFLAG_NOCUR);

    WinCls(gwXfer, CWHITE | _BLUE);
  }

  /* Draw the initial transfer window */

  if (gwXfer)
  {
    WinHline(gwXfer, 1, 1, 63, BORDER_SINGLE, CWHITE | _BLUE);
    WinHline(gwXfer, 5, 1, 63, BORDER_SINGLE, CWHITE | _BLUE);

    WinPutstr(gwXfer, 2, 1, win_block_check);

    if (doing_dl)
    {
      WinPutstr(gwXfer, 3, 1, win_bytes_to_send);
      WinPutstr(gwXfer, 4, 1, win_bytes_sent);
    }
    else
    {
      WinPutstr(gwXfer, 3, 1, win_bytes_to_receive);
      WinPutstr(gwXfer, 4, 1, win_bytes_received);
    }

    WinPutstr(gwXfer, 2, 35, win_transfer_rate);
    WinPutstr(gwXfer, 3, 35, win_time_left);
    WinPutstr(gwXfer, 4, 35, win_time_elapsed);

    WinPutstr(gwXfer, 6, 1, win_status);

    sprintf(temp, doing_dl ? win_download : win_upload,
            Protocol_Name(protocol, temp2));

    WinTitle(gwXfer, temp, TITLE_LEFT);
    WinSync(gwXfer, FALSE);
  }

  return !!gwXfer;
}



/* Change the current filename */

void XferWinNewFile(char *filename, long size)
{
  /* Display the filename */

  WinGotoXY(gwXfer, 0, 1, FALSE);
  WinCleol(gwXfer, 0, 1, CWHITE | _BLUE);
  WinPrintf(gwXfer, win_name, filename ? filename : "");
  WinSync(gwXfer, FALSE);

  /* Display the file size */

  WinGotoXY(gwXfer, 3, 19, FALSE);

  if (size==-1L)
    WinPuts(gwXfer, unknown);
  else
    WinPrintf(gwXfer, "%-8ld", size);
}


/* Close the transfer window */

void XferWinClose(void)
{
  if (gwXfer)
  {
    WinClose(gwXfer);
    gwXfer=NULL;
  }
}


/* Display the CPS and time remaining/elapsed for this file */

static void near XferWinShowRates(long size, long sent, long elapsed, word protocol)
{
  long xtime;

  /* Display transfer rate */

  WinGotoXY(gwXfer, 2, 51, FALSE);
  WinPrintf(gwXfer, win_cps, elapsed ? sent/elapsed : 0);

  /* Display time remaining */

  if (size != -1L)
  {
    WinGotoXY(gwXfer, 3, 51, FALSE);

    xtime = (size-sent < 0) ? 0 : XferTime(protocol, size - sent);
    WinPrintf(gwXfer, "%10lu:%02u", xtime / 60L, (unsigned)(xtime % 60L));

    /* Display time elapsed */

    WinGotoXY(gwXfer, 4, 51, FALSE);
    WinPrintf(gwXfer, "%10lu:%02u", elapsed / 60L, (unsigned)(elapsed % 60L));
  }
}


/* Add the specified number of spaces at the specified location */

static void near XferWinSpace(int row, int col, int n_spaces)
{
  WinGotoXY(gwXfer, 0, 1, FALSE);

  while (n_spaces--)
    WinPutch(gwXfer, row, col++, ' ', CWHITE | _BLUE);
}


/* Clear all information from the transfer window */

void XferWinClear(void)
{
  WinCleol(gwXfer, 0, 7, CWHITE | _BLUE);

  XferWinSpace(2, 19, 16);
  XferWinSpace(3, 19, 16);
  XferWinSpace(4, 19, 16);

  XferWinSpace(2, 51, 13);
  XferWinSpace(3, 51, 13);
  XferWinSpace(4, 51, 13);

  WinCleol(gwXfer, 6, 19, CWHITE | _BLUE);
  WinSync(gwXfer, FALSE);
}



/* Returns TRUE once per second */

static int XferWinDoSync(void)
{
  static long lSyncTime=-1L;
  long lNow;

  /* Make sure to sync window only once per second */

  lNow=time(NULL);

  if (lSyncTime != lNow)
  {
    lSyncTime=lNow;
    return TRUE;
  }

  return FALSE;
}



/* Display our status on the local console, for both the Xmodem receiver    *
 * and the transmitter.                                                     */

void XmStatus(unsigned flag, unsigned do_crc, long sent,
              time_t start, sword protocol, long size, long last_ack,
              unsigned n_err, long block)
{
  if (gwXfer)
  {
    int fDoSync=XferWinDoSync();

    if (fDoSync)
    {
      WinPutstr(gwXfer, 2, 19, do_crc ? win_crc16 : win_checksum);

      /* Display bytes sent */

      WinGotoXY(gwXfer, 4, 19, FALSE);
      WinPrintf(gwXfer, "%-10ld", sent);

      XferWinShowRates(size, sent, (long)(time(NULL)-start), protocol);
    }

    /* Display status message */

    if (flag != XS_SENT && flag != XS_RECV)
    {
      WinCleol(gwXfer, 6, 19, CWHITE | _BLUE);

      /* Now display the status message */

      WinGotoXY(gwXfer, 6, 19, FALSE);

      switch (flag)
      {
        case XS_NAK:
          if (last_ack)
            WinPrintf(gwXfer, win_nak_block,
                      last_ack, 10 - n_err);
          break;

        case XS_EOT:    WinPuts(gwXfer, win_sending_eot);  break;
        case XS_GOTEOT: WinPuts(gwXfer, win_got_eot);  break;
      }
    }

    if (fDoSync)
      WinSync(gwXfer, FALSE);
  }
  else
  {
    Lprintf("\r" CLEOL);

    Lprintf("%s %ld",
            flag==XS_NAK ? win_nak : flag==XS_SENT ? win_sent : win_eot,
            block);

    if (protocol==PROTOCOL_SEALINK)
      Lprintf(" : %ld", last_ack);

    Lprintf("\r");

    vbuf_flush();
  }
}

static time_t tZmodemStart;
static unsigned long ulZmSize;

/* Initialize with a blank window */

void ZmStatInit(void)
{
  tZmodemStart=time(NULL);
  ulZmSize=0L;
  XferWinClear();
  Lputc('\n');
  WinSync(gwXfer, FALSE);
}


/* Display a header status message */

void ZmStatHdr(int fSend, int iHdr, char *szHdr, unsigned long ulHdr)
{
  char szAt[PATHLEN];

  /* Don't display ZDATA frames */

  if (iHdr==ZDATA)
    return;

  WinCleol(gwXfer, 6, 19, CWHITE | _BLUE);
  WinGotoXY(gwXfer, 6, 19, FALSE);


  /* Display the header location, except for special situations */

  if (iHdr==ZRQINIT || iHdr==ZRINIT || iHdr==ZSINIT)
    *szAt=0;
  else
    sprintf(szAt, win_at_offset, ulHdr);


  if (iHdr==ZRPOS && ulHdr==0)
    WinPrintf(gwXfer, win_beginning_transfer);
  else
    WinPrintf(gwXfer,
              "%s %s%s",
              fSend ? win_sent : win_got,
              szHdr,
              szAt);

/*  if (XferWinDoSync())*/
    WinSync(gwXfer, FALSE);
}

void ZmStatFile(char *szPath, unsigned long ulSize, unsigned fCrc32)
{
  tZmodemStart=time(NULL);
  ThruStart();

  /* Change to a new filename */

  XferWinNewFile(szPath, ulZmSize=ulSize);
  ZmStatData(0L, 0, fCrc32);

  WinSync(gwXfer, FALSE);
}


void ZmStatData(unsigned long ulPos, unsigned uiBlockLen, unsigned fCrc32)
{
  char temp[PATHLEN];

  sprintf(temp, win_crchdr, fCrc32 ? 32 : 16, uiBlockLen);
  WinPutstr(gwXfer, 2, 19, temp);

  WinGotoXY(gwXfer, 4, 19, FALSE);
  WinPrintf(gwXfer, "%-10ld", ulPos); /* update file position */

  WinGotoXY(gwXfer, 2, 51, FALSE);

  XferWinShowRates(ulZmSize,
                   ulPos,
                   time(NULL) - tZmodemStart,
                   PROTOCOL_ZMODEM);

  if (XferWinDoSync())
    WinSync(gwXfer, FALSE);
}

#if 0 /* unused */

/* Display our status on the local console */

void ZmStatus(unsigned crc32, unsigned block_size, long size, long sent, time_t start, char *status)
{
  char temp[PATHLEN];
  time_t now;
  long xtime;

  if (gwXfer)
  {
    sprintf(temp, crc32 ? win_crc32_size : win_crc16_size, block_size);
    WinPutstr(gwXfer, 2, 19, temp);

    WinGotoXY(gwXfer, 3, 19, FALSE);
    WinPrintf(gwXfer, pl, size);

    WinGotoXY(gwXfer, 4, 19, FALSE);
    WinPrintf(gwXfer, "%-10ld", sent);

    WinGotoXY(gwXfer, 2, 51, FALSE);

    now=time(NULL);

    if (now != start)
      WinPrintf(gwXfer, win_cps, sent / (long)(now-start));

    WinGotoXY(gwXfer, 3, 51, FALSE);
    xtime=XferTime(PROTOCOL_ZMODEM, size-sent);
    WinPrintf(gwXfer, win_time, xtime / 60L, (unsigned)(xtime % 60L));

    WinGotoXY(gwXfer, 4, 51, FALSE);
    xtime=time(NULL)-start;
    WinPrintf(gwXfer, win_time, xtime / 60L, (unsigned)(xtime % 60L));

    /* Now display the status message */

    if (status)
    {
      WinCleol(gwXfer, 6, 19, CWHITE | _BLUE);
      WinPutstr(gwXfer, 6, 19, status);
    }

    if (XferWinDoSync())
      WinSync(gwXfer, FALSE);
  }
  else
  {
    Lprintf("\r" CLEOL "%7lu %4u\r",
            sent, block_size);

    vbuf_flush();
  }
}
#endif


/* Get a character from the modem within the specified time range */

int mdm_getct(unsigned timeout)
{
  long t1=timerset(timeout);
  int ch;

  while (!timeup(t1))
  {
    if ((ch=mdm_getc()) != -1)
      return ch;
    else if (!carrier())
      break;
    else Giveaway_Slice();
  }

  return -1;
}


/* Send ten CANs to abort a transfer */

void XmSendCAN(void)
{
  unsigned i;

  /* Stop the file we're sending */

  mdm_dump(DUMP_OUTPUT);

  /* Send ten CANs */

  for (i=0; i < 16; i++)
    mdm_pputcw(XM_CAN);

  /* Send 10 backspaces */

  for (i=0; i < 16; i++)
  {
    mdm_pputcw('\x08');
    mdm_pputcw(' ');
    mdm_pputcw('\x08');
  }
}



