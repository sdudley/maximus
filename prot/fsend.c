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

/* $Id: fsend.c,v 1.2 2004/01/22 08:04:28 wmcbrine Exp $ */

/*#define XM_NO_CRC*/
#define MAX_LANG_protocols
#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "prog.h"
#include "mm.h"
#include "xmodem.h"
#include "keys.h"
#include "win.h"
#include "pdata.h"


#ifdef NEVER
static int near log_mdm_getc(void)
{
  int ch;

  if ((ch=mdm_getc())==-1)
    return -1;

  dlogit(("@got char %02x ('%c')", ch, ch));
  return ch;

}
#endif


/* Status function for the Xmodem sender */

static void near XmStatusTx(struct _xmstuff *px, unsigned flag)
{
  if (!px->filename)
    return;

  XmStatus(flag, px->do_crc,
           (px->protocol==PROTOCOL_YMODEMG ? px->block : px->last_ack) *
                (long)px->blocksize, px->start, px->protocol, px->size,
                px->last_ack, px->n_err, px->block);
}


/* Wait for modem transmit buffer to clear */

static void near XmTxClear(void)
{
  while (!out_empty() && carrier() && loc_peek() != K_ESC)
    Giveaway_Slice();
}





/* Transmit one Xmodem block to the remote end */

static void near XmTxBlock(struct _xmstuff *px, long block_num, unsigned blocksize, unsigned do_clear)
{
  word crc;
  byte *data=px->xm_buffer;
  byte *p, *e;
  byte sum;
  int got;

  /* Set up the block header */

  data[-3]=blocksize==128 ? XM_SOH : XM_STX;    /* Start of header */
  data[-2]=(byte)block_num;                     /* Xmodem block number */
  data[-1]=(byte)~block_num;                    /* One's complement of      *
                                                 * block number             */

  /* Set the pointers for the beginning and end of block */

  p=data-3;
  e=data+blocksize;

  /* Now dump it out to the modem */

  while (p < e && carrier())
  {
    /* Send block */

    got=mdm_blockwrite(e-p, p);

    /* If we didn't send it all, try again */

    if (got > 0)
      p += (size_t)got;
    else
      Giveaway_Slice();
  }


  if (px->do_crc)
  {
    /* Calculate a CRC instead of a checksum */

    for (p=data, e=p+blocksize, crc=0; p < e; p++)
      crc=updcrc(*p, crc);

    /* Transmit the CRC */

    mdm_pputcw((byte)(crc >> 8));
    mdm_pputcw((byte)crc);

    dlogit(("@XmTxBlock; block=%ld, crc=%04x", px->block, crc));
  }
  else
  {
    /* Calculate checksum of block */

    for (p=data, e=p+blocksize, sum=0; p < e; )
      sum += (byte)*p++;

    /* Send the checksum after the data */

    mdm_pputcw(sum);

    dlogit(("@XmTxBlock; block=%ld, sum=%d", px->block, sum));
  }

  if (brk_trapped)
  {
    brk_trapped=0;
    data[blocksize]++;
  }

  /* Wait for tx buffer to clear, as long as we have carrier */

  if (do_clear)
  {
    /* Normally, this should be done the other way around (dump
     * after the clear) -- but Giveaway_Slice() under OS/2-DOS waits
     * too long, so we end up missing the ACK/NAK that the remote
     * sends to us!
     */

    mdm_dump(DUMP_INPUT);
    XmTxClear();
  }
}


/* Read a block from disk for transmission using Xmodem */

static void near XmReadBlock(struct _xmstuff *px)
{
  long offset;
  int iGot;


  /* Seek to the right place in the file */

  offset=(px->block-1L) * (long)px->blocksize;

  if (Btell(px->bf) != offset)
    Bseek(px->bf, offset, SEEK_SET);

  /* Now read in data from the file.  If we don't get all of it, it doesn't *
   * matter, since the block is padded with EOF to 128 bytes anyway.        */

  iGot=Bread(px->bf, px->xm_buffer, px->blocksize);

  if (iGot > 0)
  {
    /* Initialize the rest of the block to EOF */

    (void)memset(px->xm_buffer + iGot, 0x1a, px->blocksize - iGot);
  }
}




/* Prepare a block 0 for Ymodem, using the file's name, size and date */

static void near XmPrepareBlock0(struct _xmstuff *px)
{
  struct stat st;
  struct _sealink *sh;
  char *p, *s;

  /* Set block to NULs */

  (void)memset(px->xm_buffer, '\0', 1024);


  /* Remove path from the file */

  s=px->filename ? No_Path(px->filename) : "";


  /* Get file information */

  if (!px->filename)
    memset(&st, '\0', sizeof st);
  else
  {
    fstat(Bfileno(px->bf), &st);

  #if defined(__WATCOMC__)  /* adjust WC return value for time zone */
    st.st_mtime -= timezone + (daylight ? 60 : 0);
  #endif
  }


  /* Do a SEAlink header? */

  if (px->protocol==PROTOCOL_SEALINK)
  {
    sh=(struct _sealink *)px->xm_buffer;

    sh->file_size=st.st_size;
    sh->file_time=st.st_mtime;

    strncpy(sh->name, s, sizeof(sh->name)-1);
    sh->name[sizeof(sh->name)-1]='\0';
    lower_fn(sh->name);

    strcpy(sh->program, xfer_id);
    sh->overdrive=FALSE;

    dlogit(("@header size=%08lx", sh->file_size));
    dlogit(("@header time=%08lx", sh->file_time));
    dlogit(("@header name=%-0.12s", sh->name));
    dlogit(("@header prog=%s", sh->program));
  }
  else  /* do Ymodem */
  {
    /* Copy filename to beginning of block */

    strcpy(p=px->xm_buffer, s);
    lower_fn(p);
    p += strlen(p)+1;


    /* Now add size, date and attributes */
    /* (Serial number is 0) */

    if (px->filename)
      sprintf(p, "%ld %lo %lo 0 %lu %lu", st.st_size, st.st_mtime, 01000644L,
              px->ulTotalFiles, px->ulTotalBytes);
  }
}





/* Get the remaining parts of an ACK/NAK packet, assuming that the          *
 * first character has already been processed.  If successful, returns      *
 * TRUE and stores the ACK/NAKed block number in *pLastACK.                 */

static unsigned near XmGetACK(struct _xmstuff *px)
{
  int bn, cb;

  if (px->protocol != PROTOCOL_SEALINK || px->block==1)
  {
    px->last_ack=px->block-1;
    return TRUE;
  }

  if ((bn=mdm_getct(100))==-1)
  {
    dlogit(("@short ack 1"));
    return FALSE;
  }

  if ((cb=mdm_getct(100))==-1)
  {
    dlogit(("@short ack 2"));
    return FALSE;
  }

  if ((byte)cb != (byte)~(byte)bn)
  {
    dlogit(("@ack err; %02x != ~%02x (%02x, %02x)", (int)cb, (int)bn,
          (byte)cb, (byte)~(byte)bn));
    return FALSE;
  }

  /* Convert the modulo-256 block number into a real block number, based    *
   * on the current block.                                                  */

  px->last_ack=px->block-(long)(byte)(px->block-(long)bn);

  dlogit(("@Got A/N %ld (current: %ld)", px->last_ack, px->block));
  return TRUE;
}



/* Initialize the Xmodem transmit engine */

static unsigned near XmTxInit(struct _xmstuff *px)
{
  dlogit(("@XmTxInit"));

  px->block=1;
  px->last_ack=-1;
  px->window=(px->protocol==PROTOCOL_SEALINK ? 6 : 1);
  px->blocksize=128;

  if (px->protocol==PROTOCOL_XMODEM1K ||
      px->protocol==PROTOCOL_YMODEM ||
      px->protocol==PROTOCOL_YMODEMG)
  {
    px->blocksize=1024;
  }

  px->n_blocks=(long)(px->size+(long)px->blocksize-1L)/(long)px->blocksize;

  dlogit(("@XmTxInit - size=%ld, blocksize=%ld, n_blocks=%ld",
        (long)px->size, (long)px->blocksize, (long)px->n_blocks));

  if (px->n_blocks==0 && px->filename)  /* We always hafta transmit 1 block */
    px->n_blocks++;

  px->do_crc=FALSE;
  px->need_ack=TRUE;
  px->ackless=FALSE;
  px->start=time(NULL);

  px->n_err=px->n_can=0;

  return TRUE;  /* everything's ok */
}


/* Do we stop transmission?  Check keyboard buffer and return TRUE if       *
 * we should stop.                                                          */

static unsigned near XmTxStop(struct _xmstuff *px)
{
  if ((prm.flags2 & FLAG2_STRICTXFER) && timeleft() <= 0)
  {
    logit(log_dl_exceeds_time);
    px->n_can=NUM_CAN;
    px->n_err=10;
    return TRUE;
  }

  if (!loc_kbhit())
    return FALSE;

  if (loc_getch() != K_ESC)
    return FALSE;

  px->n_can=NUM_CAN;
  px->n_err=10;

  return TRUE;
}



/* If we received a CAN from the remote */

static void near XmTxGotCAN(struct _xmstuff *px)
{
  dlogit(("@got can"));

  /* Give us time to find multiple incoming CANs */

  if (px->protocol==PROTOCOL_YMODEMG)
    Delay(10);

  if (px->last_ch==XM_CAN)
    px->n_can++;
  else px->n_can=2;
}



/* Transmit the filename and/or header block */

static unsigned near XmTxHeader(struct _xmstuff *px)
{
  long t1=timerset(6000);
  int ch;

  dlogit(("@XmTxHeader"));

  /* No header necessary for Xmodem or Xmodem-1K */

  if (px->protocol==PROTOCOL_XMODEM || px->protocol==PROTOCOL_XMODEM1K)
    return TRUE;

  /* Don't bother transmitting a null header for SEAlink sessions */

  if (!px->filename && px->protocol==PROTOCOL_SEALINK)
    return TRUE;

  /* Create the header block containing the filename */

  XmPrepareBlock0(px);

  if (px->protocol==PROTOCOL_YMODEMG && !px->filename)
  {
    px->do_crc=TRUE;
    XmTxBlock(px, 0, 128, FALSE);
  }

  while (px->n_err < 10 && px->n_can < NUM_CAN && carrier())
  {
    if (XmTxStop(px)) /* Did sysop press <esc>? */
      break;

    /* If no character is available, and if we need one, wait. */

    if (!mdm_avail())
    {
      if (timeup(t1)) /* If the receiver has died, exit */
        return FALSE;

      Giveaway_Slice(); /* Give away time to multitasker */
      continue;
    }

    switch (ch=mdm_getc())     /* Decide based on the character we got */
    {
      case XM_CAN:  XmTxGotCAN(px); break;
#ifndef XM_NO_CRC
      case XM_G:                        /* fall thru */
        if (px->do_crc)   /* if we already sent the header once */
        {
          /* If we got Ymodem-G, go ahead to send file */
          dlogit(("@hdr: autoACK ym-g"));
          px->ackless=TRUE;
          ch=XM_ACK;
          break;
        }
        /* fall thru */

      case XM_C:    px->do_crc=TRUE;    /* fall thru */
#endif
      case XM_NAK:  /* transmit the header block */
        px->crc_set=TRUE;
        dlogit(("@hdr: got nak; sending block 0"));
        XmPrepareBlock0(px);
        XmTxBlock(px, 0, 128, px->protocol != PROTOCOL_YMODEMG /*TRUE*/);
        break;

      case XM_ACK:  /* processed at end of loop */ break;

      default:      dlogit(("@hdr: got junk '%c' (%#02x)", ch==0 ? ' ' : ch, ch));
    }

    px->last_ch=ch;

    /* if we got an ACK for the header, get out */

    if (ch==XM_ACK)
    {
      /* Start the SEAlink transmitter sending right away; otherwise,       *
       * leave last_ack alone so that we need a NAK to get started.         */

      if (px->protocol==PROTOCOL_SEALINK)
        px->last_ack=0;

      dlogit(("@hdr: got ack"));

      /* If we got an ACK, get out of the loop.  Otherwise, the
       * ACK was bad, so try to get another one.
       */

      if (XmGetACK(px))
        break;
    }
  }

  dlogit(("@hdr: n_can=%d, n_err=%d", px->n_can, px->n_err));

  if (px->n_can >= NUM_CAN || px->n_err >= 10)
    XmSendCAN();

  return (px->n_err < 10 ? TRUE : FALSE);
}


/* Handle the final EOT */

static unsigned near XmTxEnd(struct _xmstuff *px)
{
  int ch;
  unsigned n_eot=0;
  long t1;

  XmStatusTx(px, XS_EOT);
  XmTxClear();      /* Clear transmit buffer */
  t1=timerset(0);   /* Send EOT immediately */

  /* Loop until we complete or error out */

  while (px->n_can < NUM_CAN && carrier())
  {
    if (XmTxStop(px)) /* Did sysop press <esc>? */
      break;

    if (timeup(t1))   /* Send an EOT every three seconds */
    {
      if (++n_eot > 10)
      {
        XmSendCAN();
        return FALSE;
      }

      mdm_pputcw(XM_EOT);

      /* If doing Ymodem-G, only send one EOT, and err out if it fails */

      if (px->protocol==PROTOCOL_YMODEMG)
      {
        n_eot=10;
        t1=timerset(6000);
      }
      else
      {
        t1=timerset(300);
      }
    }

    /* If no character is available, wait. */

    if (!mdm_avail())
    {
      Giveaway_Slice(); /* Give away time to multitasker */
      continue;
    }

    switch (ch=mdm_getc())
    {
      case XM_CAN:  XmTxGotCAN(px); return FALSE;
#ifndef XM_NO_CRC
      case XM_G:
      case XM_C:    
#endif
      case XM_NAK:    /* Send another EOT */
        dlogit(("@eot: got nak"));

        /* Eat the block number, if any */

        if (px->protocol==PROTOCOL_SEALINK)
        {
          int ch1=mdm_getct(50);
          int ch2=mdm_getct(50);

          dlogit(("@eot: ch1=%d, ch2=%d", ch1, ch2));

          if (ch1==XM_ACK || ch2==XM_ACK)
          {
            dlogit(("@eot: got ack after nak"));
            return TRUE;
          }
        }

        mdm_pputcw(XM_EOT);
        break;

      case XM_ACK:
        if (px->protocol==PROTOCOL_SEALINK)
        {
          (void)mdm_getct(50);
          (void)mdm_getct(50);
        }

        dlogit(("@eot: got ack"));
        return TRUE;  /* Got it! */

      default:      dlogit(("@eot: got junk '%c' (%#02x)", ch==0 ? ' ' : ch, ch));
    }

    px->last_ch=ch;
  }

  XmSendCAN();
  return FALSE;
}





static void near XmTxGotNAK(struct _xmstuff *px)
{
  /* If the character we got was actually a 'C', instead of just fallthru */

  if (!XmGetACK(px))
  {
    dlogit(("@got short nak %ld", px->last_ack));
    return;
  }

  /* If we got a NAK, shift back to the last block, unless we're doing YmG */

  if (px->ackless)
  {
    dlogit(("@ymodem-g nak; xfer aborted"));
    px->n_err=10;
    return;
  }

  /* Switch into Ymodem-G mode */

  if (px->protocol==PROTOCOL_YMODEMG)
    px->ackless=TRUE;

  dlogit(("@got nak block=%ld (last=%ld)", px->block, px->last_ack));

  if (px->protocol != PROTOCOL_YMODEM || px->block != 1)
    XmStatusTx(px, XS_NAK);       /* State that we got a NAK */

  px->block=px->last_ack;     /* Start resending at the specified block */

  /* If we're doing SEAlink, the number retrieved from the block header     *
   * is actually the block that we want to resend.  However, the XmGetACK   *
   * call will return the last ACKed block if we're doing a non-sliding     *
   * protocol, so we want to resent the one AFTER the last ACKed one.       */

  if (px->block==0)
    px->block++;

  if (px->block > px->n_blocks)
    px->block=px->n_blocks;

  px->last_ack=px->block-1;   /* Pretend we've only ACKed prev block */
  px->n_err++;                /* Increment number of errors */

  dlogit(("@after nak, block=%ld, last_ack=%ld", px->block, px->last_ack));

  if (px->protocol==PROTOCOL_SEALINK && px->block != 1)
    mdm_dump(DUMP_OUTPUT);
}




/* Handle an ACK received from remote */

static unsigned near XmTxGotACK(struct _xmstuff *px)
{
  /* If we really got an ACK */

  if (!XmGetACK(px))
  {
    dlogit(("@short ack %ld", px->last_ack));
    return FALSE;
  }

  if (px->protocol=='S' && px->block >= px->n_blocks)
    XmStatusTx(px, XS_SENT);

  dlogit(("@got ACK %ld", px->last_ack));
  px->n_err=0;

  return TRUE;
}



/* Transmit the current block and increment pointers appropriately */

static void near XmTxOneBlock(struct _xmstuff *px)
{
  /* Display current status */

  XmStatusTx(px, XS_SENT);

  dlogit(("@sending block %ld (last %ld)", px->block, px->last_ack));

  /* Read block from disk and send it */

  XmReadBlock(px);
  XmTxBlock(px, px->block, px->blocksize,
            (px->protocol != PROTOCOL_YMODEMG &&
             px->protocol != PROTOCOL_SEALINK));

  px->block++;
}




/* Transmit all of the blocks in the file */

static unsigned near XmTxBlocks(struct _xmstuff *px)
{
  int started=FALSE;
  long t1;
  int ch;

  t1=timerset(6000);     /* 60-second timeout */

  /* Send blocks while we still can */

  while (px->last_ack < px->n_blocks && px->n_err < 10 &&
         px->n_can < NUM_CAN && carrier())
  {
    if (XmTxStop(px)) /* Did sysop press <esc>? */
      break;

    /* Begin transmitting a block, if we're allowed to do so */

    if (px->ackless || px->block <= px->last_ack+px->window)
      if (px->block <= px->n_blocks)
      {
        if (!started)
        {
          started=TRUE;
          px->start=time(NULL);
          ThruStart();
        }

        XmTxOneBlock(px);

        /* Don't wait for last ACK if doing Ymodem-G */

        if (px->protocol==PROTOCOL_YMODEMG && px->block > px->n_blocks)
          break;

        /* Otherwise, loop back to top and send blocks as necessary */

        if (!mdm_avail())
          continue;
      }

    /* If no character is available, and if we need one, wait. */

    if (!mdm_avail())
    {
      if (timeup(t1)) /* If the receiver has died, exit */
      {
        dlogit(("@timeout"));
        return FALSE;
      }

      Giveaway_Slice(); /* Give away time to multitasker */
      continue;
    }

    /* Now pull as many characters as we can from the input stream in the
     * case of error.
     */

    do
    {
      ch=mdm_getc();

      dlogit(("@rec'd '%c' (%#02x)", ch, ch));

      switch (ch)
      {
        case XM_CAN:  XmTxGotCAN(px); break;
#ifndef XM_NO_CRC
        case XM_G:
        case XM_C:    if (!px->crc_set) px->do_crc=TRUE;    /* fall thru */
#endif
        case XM_NAK:  px->crc_set=TRUE; XmTxGotNAK(px); break;
        case XM_ACK:  XmTxGotACK(px); t1=timerset(6000); break;
        default:      dlogit(("@got junk '%c' (%#02x)", ch==0 ? ' ' : ch, ch));
      }

      px->last_ch=ch;
    }
    while ((ch==XM_NAK || ch==XM_CAN) &&
           (mdm_avail() && px->n_err < 10 && px->n_can < NUM_CAN && carrier()));
  }

  dlogit(("@n_can=%d, n_err=%d", px->n_can, px->n_err));
  dlogit(("@last_ack=%ld, n_blocks=%ld", px->last_ack, px->n_blocks));

  if (px->n_can >= NUM_CAN || px->n_err >= 10)
    XmSendCAN();

  if (!carrier())
    dlogit(("@lost carrier"));
  else
    dlogit(("@transfer complete %ld", px->block));

  return (px->n_err < 10 && px->n_can < NUM_CAN);
}






/* Transmit a file using Xmodem.  Returns TRUE if the transfer succeeded,   *
 * or FALSE if the transfer failed.                                         */

static unsigned XmTxFile(struct _xmstuff *px)
{
  /* Init the sender engine */

  if (!XmTxInit(px))
    return FALSE;

  /* Transmit the header and/or filename */

  if (!XmTxHeader(px))
    return FALSE;

  /* If we have a file to send... */

  if (px->filename)
  {
    /* Send the file itself */

    if (!XmTxBlocks(px))
      return FALSE;

    /* End the transfer */
  }

  if (px->filename || px->protocol==PROTOCOL_SEALINK)
    if (!XmTxEnd(px))
      return FALSE;

  return TRUE;
}





/* Transmit the named file using Xmodem */

unsigned XmTx(char *filename, word protocol, unsigned long ulFiles, unsigned long ulBytes, char *szRealName)
{
  struct _xmstuff x;
  byte *buf;

  long siz;
  unsigned ret;
  BFILE bf=NULL;

  if (filename && is_devicename(filename))
  {
    logit(log_cant_send_dev, filename);
    return FALSE;
  }

  /* Allocate buffer for transfer */

  if ((buf=malloc(MAX_XMODEM_BUF))==NULL)
  {
    logit(mem_none);
    return FALSE;
  }


  /* Open the file to be sent */

  if (filename &&
      (bf=Bopen(filename, BO_RDONLY | BO_BINARY, BSH_DENYNO, 8192))==NULL)
  {
    logit(cantfind, filename);
    return FALSE;
  }

  /* Make sure that the user doesn't try to download a device */

  if (bf && is_device(Bfileno(bf)))
  {
    logit(udev, filename);
    Bclose(bf);
    return FALSE;
  }

  siz = filename ? fsize(filename) : 0L;


  /* Initialize the xmstuff data structure */

  (void)memset(&x, 0, sizeof x);

  x.protocol=(sword)protocol;
  x.filename=filename;
  x.bf=bf;
  x.size=siz;
  x.xm_buffer=buf+3;
  x.ulTotalFiles=ulFiles;
  x.ulTotalBytes=ulBytes;

  ThruStart();

  dlogit(("@XmTxFile(%s,%d)",
        filename ? filename : "(null)",
        x.protocol));

  XferWinNewFile(x.filename, x.size);

  /* Now send the file itself */

  ret=XmTxFile(&x);

  if (filename && ret)
  {
    ThruLog(x.size);
    logit(log_dl, Get_Protocol_Letter(x.protocol), szRealName);
  }

  /* Clean up */

  if (filename)
    Bclose(bf);

  free(buf);

  return ret;
}


