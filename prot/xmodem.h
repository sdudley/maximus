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

#ifndef __BFILE_H_DEFINED
  #include "bfile.h"
#endif

#define MAX_XMODEM_BUF  1100  /* 1024 bytes plus space */
#define NUM_CAN         4     /* 4 CANs required to cancel a file */
#define NUM_NAK         20    /* Max number of NAKs */

#define XM_SOH  (byte)0x01
#define XM_STX  (byte)0x02
#define XM_EOT  (byte)0x04
#define XM_ACK  (byte)0x06
#define XM_NAK  (byte)0x15
#define XM_CAN  (byte)0x18
#define XM_C    (byte)'C'
#define XM_G    (byte)'G'

/* Status codes for XmStatus */

#define XS_NAK    0x00  /* This block was NAKed */
#define XS_SENT   0x01  /* Sending this block */
#define XS_EOT    0x02  /* Sending EOT */
#define XS_GOTEOT 0x03  /* Sending EOT */
#define XS_RECV   0x04  /* Received this block */

/* Structure of the SEAlink block 0 */

struct _sealink
{
  dword file_size;
  dword file_time;
  byte name[17];
  byte program[15];
  byte overdrive;
  byte rsvd[87];
};


/* Xmodem send stuff - passed among all of the Xmodem functions */

struct _xmstuff
{
  /* This stuff is filled in by XmTx */

  sword protocol;                 /* Letter representing our protocol */

  byte *filename;                 /* Name of the file being sent */
  BFILE bf;                       /* File handle for this file */
  long size;                      /* Size of the file */

  byte *xm_buffer;                /* Buffer for reading/sending data */
  unsigned long ulTotalFiles;     /* Total # of files to send */
  unsigned long ulTotalBytes;     /* Total # of bytes to send */

  /* This stuff is filled in by XmTxInit */

  long n_blocks;                  /* Number of blocks in the file */
  long block;                     /* Current block */
  long last_ack;                  /* Last ACKed block */
  long window;                    /* Size of protocol window */
  unsigned blocksize;             /* Size of each block */
  unsigned do_crc;                /* Are we using CRC or checksum? */
  unsigned crc_set;               /* Has do_crc been set? */
  unsigned need_ack;              /* Do we need an ACK to continue? */
  unsigned ackless;               /* Is this an ACKless protocol? */

  time_t start;                   /* Starting time of the transfer */

  unsigned n_err;                 /* Number of errors in current block */
  unsigned n_can;                 /* Number of CANs received */
  int last_ch;                    /* Last character received */
};


/* Xmodem receive stuff - passed among all of the Xmodem functions */

struct _xmrx
{
  /* This stuff is filled in by XmRx */

  sword protocol;                 /* Letter representing our protocol */

  byte *path;                     /* Path for receiving file */
  byte filename[PATHLEN];         /* Filename (no path) of file */
  int fd;                         /* File handle for this file */
  long size;                      /* Size of the file (-1 if unknown) */
  long received;                  /* Amount of file we have successfully recd */
  SCOMBO date;                    /* File's date */

  byte *xm_buffer;                /* Buffer for receiving data */
  unsigned *pusEOB;               /* End of batch transfer? */

  /* This stuff is filled in by XmRxInit, if possible */

  long block;                     /* Current block */
  unsigned blocksize;             /* Size of current block */
  unsigned do_crc;                /* Are we using CRC or checksum? */
  unsigned crc_set;               /* Are we using CRC or checksum? */
  unsigned ackless;               /* Is this an ACKless protocol? */

  time_t start;                   /* Starting time of the transfer */
  time_t tSendNAK;                /* Send a NAK if no response by this time */

  unsigned n_err;                 /* Number of errors in current block */
  unsigned n_nak;                 /* Number of NKNs received */
  unsigned n_can;                 /* Number of CANs received */

  int ch;                         /* The character just received is */
  int last_ch;                    /* Last character received before 'ch' */
  int fGotEOT;                    /* Have we seen an EOT from the remote? */
};


/* Prototypes for the public Xmodem functions */

unsigned XmTx(char *filename, word protocol, unsigned long ulFiles, unsigned long ulBytes, char *szRealName);
unsigned XmRx(byte *path, byte *filename, word protocol, unsigned *pusEOB);

