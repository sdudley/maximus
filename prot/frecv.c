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
static char rcs_id[]="$Id: frecv.c,v 1.2 2003/06/05 01:16:01 wesgarland Exp $";
#pragma on(unreferenced)

#define PROT_LOG
#define MAX_LANG_protocols
#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "mm.h"
#include "xmodem.h"
#include "keys.h"
#include "win.h"
#include "pdata.h"
#include "f_up.h"

#if defined(PROT_LOG) && defined(DEBUGX)
  #define mdm_fpputcw(ch)  mdm_pputcw(ch), logit("@sent char %02x ('%c')", ch, ch)
#else
  #define mdm_fpputcw(ch)  mdm_pputcw(ch)
#endif

static unsigned near XmRxStop(struct _xmrx *px)
{
  if (!loc_kbhit())
    return FALSE;

  if (loc_getch() != K_ESC)
    return FALSE;

  px->n_can=NUM_CAN;
  px->n_err=10;

  return TRUE;
}


/* Status message for the Xmodem receiver */

static void near XmStatusRx(struct _xmrx *px, unsigned flag)
{
  XmStatus(flag, px->do_crc, px->received, px->start,
           px->protocol, px->size, 0L, px->n_err, px->block);
}



/* Send a packetized ack/nak to the remote */

static void near XmRxSendPacketACK(unsigned do_complement, int ch, long block)
{
  byte temp[5];
  char *p, *e;
  int got;

  /* For normal protocols, just send the ack character */

  if (!do_complement)
  {
    mdm_fpputcw(ch);
    return;
  }

  /* Otherwise, output the block ack/nak packet */

  temp[0]=(byte)ch;
  temp[1]=(byte)(block & 0xff);
  temp[2]=(byte)~(block & 0xff);

  p=temp;
  e=p+3;

  /* Output to modemm as a block */

  while (p < e && carrier())
  {
    got=mdm_blockwrite(e-p, p);

    if (got > 0)
      p += (size_t)got;
  }

#ifdef PROT_LOG
  dlogit(("@sent block %02x %02x %02x (%c%c%c)",
         temp[0], temp[1], temp[2], temp[0], temp[1], temp[2]));
#endif
}


/* ACK the specified block number */

static void near XmRxSendACK(struct _xmrx *px, long block)
{
  px->n_nak=0;

  dlogit(("@XmRxSendACK - sent ack %d", block));

  /* If we're doing Ymodem-G, don't ACK anything except block0 and EOT */

  if (px->protocol==PROTOCOL_YMODEMG)
  {
    switch (block)
    {
      case -1:  mdm_fpputcw(XM_ACK); dlogit(("@ymg- put ACK")); break;  /* ACK for EOT */
      case  0:  mdm_fpputcw(XM_G);   break;  /* G for block 0 */
    }
  }
  else
  {
    /* State that we received a block */

    if (block != -1)
      XmStatusRx(px, XS_RECV);

    XmRxSendPacketACK(px->protocol==PROTOCOL_SEALINK, XM_ACK,
                      block==-1 ? px->block : block);
  }
}


/* Send a NAK to the remote side */

static void near XmRxSendNAK(struct _xmrx *px, long block)
{
  int chNAK;

  /* If we sent five NAKs, flip from CRC to NAK and vice versa */

  if ((++px->n_nak % 5)==0 && !px->crc_set &&
      px->protocol != PROTOCOL_YMODEM && px->protocol != PROTOCOL_YMODEMG &&
      px->protocol != PROTOCOL_SEALINK)
  {
    px->do_crc=!px->do_crc;
    dlogit(("@XmRxSendNAK: switching to %s", px->do_crc ? "crc" : "checksum"));
  }

  /* Send the NAK character to the remote side */

  if (px->protocol==PROTOCOL_YMODEMG)
    chNAK=XM_G;
  else if (px->do_crc && !px->crc_set)
    chNAK=XM_C;
  else chNAK=XM_NAK;

  dlogit(("@XmRxSendNAK - sent nak %d (%c)", block, chNAK));

  /* Now send the nak, with the packet stuff for sealink */

  XmRxSendPacketACK(px->protocol==PROTOCOL_SEALINK && px->block != 0, chNAK,
                    block==-1 ? px->block : block);

  /* Do not display "nak block 0" for the beginning of the transfer */

  if (px->crc_set)
    XmStatusRx(px, XS_NAK);
}





/* This function fetches the block complement from remote */

static unsigned near XmRxGetBlockComplement(byte *pbn)
{
  int bn, bc;

  /* Get the block number and the block complement, timeout 1 sec */

  if ((bn=mdm_getct(100))==-1)
  {
    dlogit(("@XmRxGetBlockComplement - timeout 1"));
    return FALSE;
  }

  if ((bc=mdm_getct(100))==-1)
  {
    dlogit(("@XmRxGetBlockComplement - timeout 2"));
    return FALSE;
  }

  /* Make sure that blocknum and block complement are the same */

  if ((byte)bn != (byte)~(byte)bc)
  {
    dlogit(("@XmRxGetBlockComplement - garbled block number"));
    return FALSE;
  }

  *pbn=(byte)bn;

  return TRUE;
}





/* Calculate the actual CRC for the specified block */

static word near XmRxCalcCRC(byte *buf, unsigned block_size)
{
  word crc=0;
  byte *p, *e;

/*  dlogit(("@crc: first 5 bytes are '%c%c%c%c%c'",
        buf[0], buf[1], buf[2], buf[3], buf[4]));*/

  for (p=buf, e=buf+block_size; p < e; p++)
    crc=updcrc(*p, crc);

/*  dlogit(("@crc: last 5 bytes are '%c%c%c%c%c'");
        p[-5], p[-4], p[-3], p[-2], p[-1]));*/

  return crc;
}


/* Calculate the actual checksum for the specified block */

static byte near XmRxCalcSum(byte *buf, unsigned block_size)
{
  byte sum=0;
  char *p, *e;

  p=buf;

  dlogit(("@sum - first 10 bytes: %c%c%c%c%c%c%c%c%c%c",
        p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9]));

  for (e=buf+block_size; p < e; )
    sum += *p++;

  dlogit(("@sum - last 10 bytes: %c%c%c%c%c%c%c%c%c%c",
        p[-10], p[-9], p[-8], p[-7], p[-6], p[-5], p[-4], p[-3], p[-2], p[-1]));

  return sum;
}


/* Display the name of the file to be received */

static void near XmRxShowName(struct _xmrx *px, char *filename)
{
  char temp[PATHLEN];

  if (filename && *filename)
  {
    strcpy(temp, px->path);
    strcat(temp, px->filename);
  }
  else strcpy(temp, xmodem_unknown_filename);

  XferWinNewFile(temp, px->size);
}



/* Handle reception and parsing of a Ymodem or SEAlink block 0 */

static unsigned near XmRxHandleBlock0(struct _xmrx *px)
{
  byte *p;

  /* Nothing to do for Xmodem or Xmodem/1K */

  if (px->protocol==PROTOCOL_XMODEM || px->protocol==PROTOCOL_XMODEM1K)
    return TRUE;

  /* Extract the protocol data from a SEAlink header */

  if (px->protocol==PROTOCOL_SEALINK)
  {
    struct _sealink *ps=(struct _sealink *)px->xm_buffer;

    px->date.ldate=ps->file_time;
    px->size=ps->file_size;
    strnncpy(px->filename, ps->name, PATHLEN-1);

    if (! *px->filename)
    {
      px->ch=XM_EOT;      /* We're done */
      *px->pusEOB=TRUE;
    }
  }
  else /* Ymodem or Ymodem-G header */
  {
    time_t tim;

    dlogit(("@XmRxHandleBlock0 - Header=%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
          px->xm_buffer[0], px->xm_buffer[1], px->xm_buffer[2],
          px->xm_buffer[3], px->xm_buffer[4], px->xm_buffer[5],
          px->xm_buffer[6], px->xm_buffer[7], px->xm_buffer[8],
          px->xm_buffer[9], px->xm_buffer[10], px->xm_buffer[11],
          px->xm_buffer[12], px->xm_buffer[13], px->xm_buffer[14],
          px->xm_buffer[15], px->xm_buffer[16], px->xm_buffer[17],
          px->xm_buffer[18], px->xm_buffer[19], px->xm_buffer[20],
          px->xm_buffer[21], px->xm_buffer[22], px->xm_buffer[23],
          px->xm_buffer[24], px->xm_buffer[25], px->xm_buffer[26],
          px->xm_buffer[27], px->xm_buffer[28], px->xm_buffer[29],
          px->xm_buffer[30], px->xm_buffer[31], px->xm_buffer[32],
          px->xm_buffer[33], px->xm_buffer[34], px->xm_buffer[35],
          px->xm_buffer[36], px->xm_buffer[37], px->xm_buffer[38],
          px->xm_buffer[39], px->xm_buffer[40]));

    if (! *px->xm_buffer)
    {
      *px->pusEOB=TRUE;
      px->ch=XM_EOT;      /* We're done */
    }

    /* Copy in the filename */

    strnncpy(px->filename, px->xm_buffer, PATHLEN-1);

    p=px->xm_buffer;
    p += strlen(p)+1;

    if ((p=strtok(p, " ")) != NULL)
    {
      px->size=atol(p);   /* Get the file size */

      if ((p=strtok(NULL, " ")) != NULL)  /* Get the file date */
      {
        struct tm *ptm;

        /* Get the file date and convert it to DOS format */

        tim=atoi(p);
        ptm=localtime(&tim);
        TmDate_to_DosDate(ptm, &px->date);
      }
    }
  }

  /* Strip the filename or path */

  if ((p=strrstr(px->filename, "/\\:")) != NULL)
    strocpy(px->filename, p+1);

  dlogit(("@XmRxHandleBlock0 - name=%s, size=%ld, date=%lu",
        px->filename, px->size, px->date.ldate));

  if ((prm.flags2 & FLAG2_CHECKDUPE) && FileIsDupe(px->filename))
  {
    dlogit((log_dupe_file_recd, px->filename));

    /* Make filename begin with a '*' so that Max knows that the      *
     * file is a dupe.                                                */

    strocpy(px->filename+1, px->filename);
    *px->filename='*';
    return FALSE;
  }

  if (px->size != -1L &&
      px->size > (zfree(px->path) - (long)prm.k_free*1000L))
  {
    logit(log_no_space_to_rec, px->filename);
    return FALSE;
  }

  /* Now display the name of the file that we're receiving */

  if (*px->filename)
    XmRxShowName(px, px->filename);

  return TRUE;
}




/* Validate the CRC or checksum for this block.  Returns TRUE if ok */

static unsigned near XmRxValidateBlock(struct _xmrx *px, unsigned block_size)
{
  byte sum, hdr_sum;
  word crc, hdr_crc;

  if (px->do_crc)
  {
    crc=XmRxCalcCRC(px->xm_buffer, block_size);

    hdr_crc=(word)((word)px->xm_buffer[block_size] << 8 |
                   (word)(byte)px->xm_buffer[block_size+1]);

    /* If the CRC did not match, NAK it */

    if (crc != hdr_crc)
    {
      logit(log_crc_error_block, px->block);
      dlogit(("@XmRxValidateBlock - actual crc %04x, but header says %04x (blocksize=%d)",
            crc, hdr_crc, block_size));

      return FALSE;
    }
  }
  else
  {
    sum=XmRxCalcSum(px->xm_buffer, block_size);
    hdr_sum=px->xm_buffer[block_size];

    if (sum != hdr_sum)
    {
      logit(log_checksum_err_block, px->block);
      dlogit(("@XmRxGetBlockData - actual sum %02x, but header says %02x (blocksize=%d)",
            sum, hdr_sum, block_size));

      return FALSE;
    }
  }

  return TRUE;  /* block was okay */
}



/* Get the block data following an SOH or STX */

static unsigned near XmRxGetBlockData(struct _xmrx *px, unsigned block_size)
{
  unsigned real_block_size;
  unsigned got;
  long t1=timerset(20000);  /* Up to 20 seconds to receive one block */
  long t2=t1;
  byte bn;
  byte *p, *e;

  /* Get the block number */

  if (!XmRxGetBlockComplement(&bn))
    return FALSE;

  if (bn != (byte)px->block)  /* If we're out of sync, abort xfer */
  {
    dlogit(("@XmRxGetBlockData - block num out of sync. Current=%ld (%d), got=%d",
          px->block, (byte)px->block, bn));

    /* This isn't an error for SEAlink */

    if (px->protocol != PROTOCOL_SEALINK)
    {
      XmSendCAN();
      px->n_can=NUM_CAN;
      return FALSE;
    }
  }

  /* In addition to data, also get the CRC or checksum */

  real_block_size=block_size;

  if (px->do_crc)
    block_size += 2;
  else block_size++;


  /* Now read data until the buffer is full */

  for (p=px->xm_buffer, e=p+block_size; p < e; )
  {
    if (!carrier() || timeup(t1) || XmRxStop(px))
    {
      dlogit(("@XmRxGetBlockData - carrier=%d, timeup=%d",
            carrier(), timeup(t1)));
      return FALSE;
    }

    got=(unsigned)mdm_blockread(e-p, p);
    p += got;

    if (got)
      t2=timerset(500);
    else
    {
      if (timeup(t2))
      {
        dlogit(("@XmRxGetBlockData - short block. (got=%d, expected=%d)",
              p - px->xm_buffer, e-p));

        return FALSE;
      }

      Giveaway_Slice();
    }
  }

  /* If we're doing SEAlink and this is the wrong block number, just        *
   * ignore it.                                                             */

  if (px->protocol==PROTOCOL_SEALINK && bn != (byte)px->block)
    return 42;  /* magic number */

  /* Check CRC or sum */

  if (!XmRxValidateBlock(px, real_block_size))
    return FALSE;

  /* If we got here, the block was okay.  Write to disk and update the      *
   * counters.                                                              */

  if (px->block==0)
  {
    /* If the filename was a duplicate, abort the transfer */

    if (!XmRxHandleBlock0(px))
    {
      /* Wait for half a sec, then get out */

      Delay(150);

      XmSendCAN();
      px->n_can=NUM_CAN;
      return FALSE;
    }
  }
  else
  {
    dlogit(("@XmRxGetBlockData - got block %d", px->block));

    /* Only write a partial block if this is the last block */

    if (px->size != -1L && px->received+(long)real_block_size >= px->size)
      real_block_size=(unsigned)(px->size - px->received);

    if (real_block_size &&
        write(px->fd, px->xm_buffer, real_block_size) != (int)real_block_size)
    {
      logit(log_err_write_block);
      XmSendCAN();
      px->n_can=NUM_CAN;
      return FALSE;
    }

    px->received += (long)real_block_size;
  }

  return TRUE;
}



/* Get one block from remote.  The character px->ch has already been        *
 * received.                                                                */

static unsigned XmRxGetBlock(struct _xmrx *px)
{
  int rc;
  int fDoTelixKludge;


  /* Handle a CAN before anything else.  If more than 4, abort. */

  if (px->ch != XM_CAN)
    px->n_can=0;
  else if (px->last_ch==XM_CAN)
  {
    dlogit(("@XmRxGetBlock - got CAN %d", px->n_can+1));

    if (++px->n_can >= NUM_CAN)
      return FALSE;
  }
  else
  {
    dlogit(("@XmRxGetBlock - got CAN 1"));
    px->n_can=1;
  }

  do
  {
    fDoTelixKludge=FALSE;

    switch (px->ch)
    {
      case XM_SOH:  /* Receive a block */
      case XM_STX:
        px->tSendNAK=timerset(1000);

        if (!px->crc_set)
        {
          px->start=time(NULL);
          ThruStart();
        }

        dlogit(("@XmRxGetBlock - got %s", px->ch==XM_SOH ? "SOH" : "STX"));

        /* Get the block of data.  If it is magic number 42, it means         *
         * that it was a SEAlink send of something other than the             *
         * current block.  Consequently, just ignore it.  Otherwise,          *
         * if TRUE, it meant that the block was sent okay (ACK).  If          *
         * FALSE, it meant that there was an error (NAK).                     */

        if ((rc=XmRxGetBlockData(px, px->ch==XM_SOH ? 128 : 1024))==42)
        {
          /* Telix kludge */

          if (px->fGotEOT)
          {
            dlogit(("@sending telix kludge ACK"));
            XmRxSendACK(px, px->block-1);
          }
        }
        else if (rc) /* if the block was received okay */
        {
          if (px->block==0)
          {
            /* Start the rest of the transfer */

            px->start=time(NULL);
            ThruStart();
            px->crc_set=FALSE;
          }

          if (px->protocol==PROTOCOL_YMODEMG)
          {
            XmStatusRx(px, XS_RECV);

            /* Only NAK it if this was not the end-of-batch header block */

            if (*px->filename && px->block==0)
              XmRxSendNAK(px, 0);

            px->block++;
          }
          else
          {
            XmRxSendACK(px, px->block++);

            /* If we're doing Ymodem, send one NAK to start the rest of the   *
             * xfer, unless we just got the end-of-batch header.              */

            if (px->protocol==PROTOCOL_YMODEM && px->block==1 &&
                *px->filename)
            {
              XmRxSendNAK(px, 0);
            }
          }

          px->crc_set=TRUE;
        }
        else if (px->n_can < NUM_CAN)
        {
          px->crc_set=TRUE;

          if (px->protocol != PROTOCOL_YMODEMG)
            XmRxSendNAK(px, px->block);
          else /* if Ymodem-G, this is a fatal error */
          {
            XmSendCAN();
            px->n_can=NUM_CAN;
            return FALSE;
          }
        }
        break;

      case XM_EOT:
        px->tSendNAK=timerset(1000);

        Delay(20); /* Wait for one second - telix kludge */

        /* SEAlink must always NAK the EOT once before sending the ACK */

        if (px->protocol==PROTOCOL_SEALINK)
        {
          /* If we're in the middle of the file, NAK the block once           *
           * (as per SEA's original sealink.c)                                */

  #if 0
          long lTimeWait;
          int ch;

          /* Toss away input characters for 200 ms */

          lTimeWait=timerset(20);

          do
          {
            if ((ch=mdm_getc()) == -1)
              Giveaway_Slice();
          }
          while (!timeup(lTimeWait));

          /* Now make sure that the EOT was at the right position */

          if (px->size != -1L && px->block < px->size/128L)
          {
              dlogit(("@XmRxGetBlock - EOT at bad posn; cur=%ld end=%ld",
                     (long)px->block, (long)px->size));

              XmRxSendNAK(px, px->block);
              px->ch=XM_NAK;
              break;
          }
  #else
          if (px->block && px->fGotEOT < 2)
          {
            px->fGotEOT++;
            dlogit(("@XmRxGetBlock - got 1st eot, send nak"));

            XmRxSendNAK(px, px->block);
            mdm_dump(DUMP_INPUT);


            if ((px->ch=mdm_getct(500)) != XM_EOT)
            {
              dlogit(("@XmRxGetBlock - didn't get 2nd eot; got %#02x", px->ch));
              fDoTelixKludge=TRUE;
              break;
            }
          }

  #endif
          /* Handle end of batch SEAlink transfer */

          if (px->block==0)
          {
            dlogit(("@XmRxGetBlock - end of batch"));
            *px->pusEOB=TRUE;
          }
        }

        dlogit(("@XmRxGetBlock - got final EOT"));

        /* If we got an EOT, do a validity check.  If the protocol is         *
         * Xmodem or Xmodem/1K, the EOT can happen at any time.  However,     *
         * with any of the other protocols, we need to make sure that         *
         * we have received the entire file.                                  */

        if (px->protocol==PROTOCOL_XMODEM || px->protocol==PROTOCOL_XMODEM1K ||
            px->size==-1 || px->received >= px->size)
        {
          if (px->protocol==PROTOCOL_SEALINK)
          {
            Delay(50);
            mdm_fflush();
          }

          /* If we're at block 0 and we got an EOT, but we didn't
           * get the null header to signify the end of a batch
           * transfer, abort.
           */

          if (px->block==0 && !*px->pusEOB &&
              (px->protocol==PROTOCOL_YMODEM ||
               px->protocol==PROTOCOL_YMODEMG))
          {
            XmSendCAN();
            px->n_can=NUM_CAN;
            return FALSE;
          }

          dlogit(("@sending plain ACK"));
          XmRxSendACK(px, -1);

          if (px->fGotEOT==2 && px->protocol==PROTOCOL_SEALINK)
          {
            int ch;

            Delay(100);
            mdm_fpputcw('C');
            Delay(250);

            if ((ch=mdm_getct(500))==XM_EOT)
            {
              dlogit(("@got 500 eot"));
              XmRxSendACK(px, -1);
              Delay(300);
            }
            else
            {
              dlogit(("@got 500 '%c'", ch));
            }

            mdm_dump(DUMP_INPUT);
          }

          if (px->protocol==PROTOCOL_SEALINK && *px->pusEOB)
          {
            int ch;

            /* Kludge around a telix problem when it does a fake
             * batch EOT
             */

            Delay(100);
            mdm_fpputcw('C');

            if ((ch=mdm_getct(300))==XM_SOH)
            {
              px->ch=ch;
              *px->pusEOB=FALSE;
              fDoTelixKludge=TRUE;
              break;
            }

            mdm_fpputcw('\b');
            mdm_fpputcw(' ');
            mdm_fpputcw('\b');
          }
        }

        XmStatusRx(px, XS_GOTEOT);
        dlogit(("@XmRxGetBlock - eot sequence done"));

        if (*px->pusEOB)  /* If it's end-of-batch, give some extra time */
          Delay(100);
        break;

      case XM_CAN:
        break;

      default:
        ; /* happy lint */
        dlogit(("@XmRxGetBlock - got junk %#02x", px->ch));
    }
  }
  while (fDoTelixKludge && carrier());

  return TRUE;
}



/* Receive a number of blocks */

static unsigned near XmRxBlocks(struct _xmrx *px)
{
  px->n_nak=0;
  px->tSendNAK=timerset(0); /* Send a NAK immediately */

  do
  {
    if (timeup(px->tSendNAK))
    {
      dlogit(("@XmRxBlocks - timeout, sending NAK"));
      XmRxSendNAK(px, px->block);
      px->tSendNAK=timerset(px->crc_set ? 1000 : 300);
    }

    if ((px->ch=mdm_getct(100)) != -1)
    {
      XmRxGetBlock(px);
      px->last_ch=px->ch;
    }
  }
  while (carrier() && px->n_nak < NUM_NAK && px->n_can < NUM_CAN &&
         px->ch != XM_EOT && !XmRxStop(px));

  dlogit(("@XmRxBlocks: n_nak=%d, n_can=%d", px->n_nak, px->n_can));

  if (px->n_nak >= NUM_NAK || px->n_can >= NUM_CAN || !carrier())
  {
    XmSendCAN();
    return FALSE;
  }

  return TRUE;
}


/* Initialize the Xmodem receiver */

static unsigned near XmRxInit(struct _xmrx *px)
{
  px->start=time(NULL);
  px->received=0L;
  px->block=1;
  px->do_crc=TRUE;    /* Try doing C's before we do NAKs */
  px->crc_set=FALSE;
  px->ackless=FALSE;
  px->n_err=px->n_can=0;
  *px->pusEOB=FALSE;
  Get_Dos_Date(&px->date);

  switch (px->protocol)
  {
    case PROTOCOL_XMODEM:
    case PROTOCOL_XMODEM1K:
      *px->pusEOB=TRUE;     /* It's always the end for Xmodem or Xmodem-1K */
      break;

    case PROTOCOL_YMODEMG:
    case PROTOCOL_YMODEM:
    case PROTOCOL_SEALINK:
      px->block=0;
  }

  return TRUE;
}


/* Receive a file.  Init the receiver, and then get as many blocks as       *
 * necessary.                                                               */

static unsigned near XmRxFile(struct _xmrx *px)
{
  if (!XmRxInit(px))
    return FALSE;

  if (!XmRxBlocks(px))
    return FALSE;

  return TRUE;
}

/* Receive one file using Xmodem-family protocols */

unsigned XmRx(byte *path, byte *filename, word protocol, unsigned *pusEOB)
{
  struct _xmrx x;
  byte tempname[PATHLEN];
  byte *buf;
  unsigned ret;
  int fd=-1;

  /* Allocate buffer for transfer */

  if ((buf=malloc(MAX_XMODEM_BUF))==NULL)
  {
    logit(mem_none);
    return FALSE;
  }

  /* Create a temporary filename to use for the transfer */

  if (path)
  {
    sprintf(tempname, "%s_tmp%02x_.$$$", PRM(temppath), task_num);

    if ((fd=sopen(tempname, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                 SH_DENYNO, S_IREAD | S_IWRITE))==-1)
    {
      dlogit(("@errno=%d", errno));
      cant_open(tempname);
      return FALSE;
    }
  }

  /* Initialize the xmstuff data structure */

  (void)memset(&x, 0, sizeof x);

  x.protocol=(sword)protocol;
  x.path=path;

  strcpy(x.filename, filename ? filename : (byte *)"");

  x.fd=fd;
  x.size=-1L;
  x.xm_buffer=buf;
  x.pusEOB=pusEOB;

  dlogit(("@XmRxFile(%s)", filename ? filename : "-* null *-"));

  /* Display the name of the file to receive */

  XmRxShowName(&x, filename);

  ThruStart();            /* Begin transfer statistics */

  ret=XmRxFile(&x);       /* Now receive the file itself */

  if (path && ret)
  {
    if (! *x.filename &&
        (x.protocol==PROTOCOL_XMODEM || x.protocol==PROTOCOL_XMODEM1K))
    {
      strcpy(filename, "unknown.$$$");
    }
  }


  /* Clean up */

  if (path)
  {
    set_fdt(fd, &x.date); /* Set file date */
    close(fd);

    if (ret && *x.filename && *x.filename != '*')
    {
      byte try_name[PATHLEN];
      int fd, dev=is_devicename(x.filename);

      strcpy(try_name, path);
      strcat(try_name, x.filename);


      if (!dev && (fd=sopen(try_name, O_CREAT | O_WRONLY | O_BINARY,
                            SH_DENYNO, S_IREAD | S_IWRITE)) != -1)
      {
        long size;

        dlogit(("@XmRx - file `%s' successfully opened", x.filename));

        dev=is_device(fd);
        size=lseek(fd, 0L, SEEK_END);
        close(fd);

        if (!size)
          unlink(try_name);

        dlogit(("@XmRx - is_device=%d", dev));
      }

      if (dev)
      {
        logit(log_cant_ul_device, x.filename);
        unlink(tempname);
        *filename=*x.filename='\0';
      }
      else
      {
        uniqrename(tempname, try_name, x.filename, lcopy);

        dlogit(("@XmRx - '%s' renamed to '%s'", tempname, x.filename));
        strcpy(filename, x.filename);
      }
    }
    else
    {
      dlogit(("@XmRx - unlinked '%s'", tempname));
      unlink(tempname);
      *filename='\0';
    }
  }

  if (path && ret && *x.filename && *x.filename != '*')
  {
    if (x.size==-1L)
      x.size=x.received;

    ThruLog(x.size);
    logit(log_ul, Get_Protocol_Letter(x.protocol), x.filename, "");
  }

  free(buf);

  dlogit(("@XmRx - rc=%d", ret));

  if (*x.filename=='*')
  {
    strcat(filename, x.filename);
    ret=TRUE;
  }

  /* Return TRUE even if we got a dupe file */

  return ret;
}

