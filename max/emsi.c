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
static char rcs_id[]="$Id: emsi.c,v 1.1.1.1 2002/10/01 17:50:54 sdudley Exp $";
#pragma on(unreferenced)

/*# name=IEMSI-processing module for Max 2.0
*/

#ifdef EMSI

#include <stdarg.h>
#include <string.h>
#include "mm.h"
#include "crc.h"
#include "modem.h"
#include "emsi.h"
#include "emsip.h"

static char location[]="Kingston, Ontario";
static byte emsi_buf[EMSI_MAX+20];
static char *emsi_max=emsi_buf+EMSI_MAX;
static word emsi_len;
static char px[]="%x";
static char plx[]="%lx";

unsigned fGotICI, fAbort, fEMSI;

/* Initialize the EMSI handshaking mechanism */

void EmsiInitHandshake(void)
{
  fGotICI=FALSE;
  fAbort=FALSE;
  fEMSI=FALSE;
}


/* Handle the initial **EMSI_xxx sequence and call the appropriate          *
 * dispatcher.                                                              */

word EmsiCheck(int ch)
{
  struct _eft *ep;
  char frame[5];


  if (fAbort || !EmsiGotInit(ch))
    return FALSE;

  /* Read frame and rest of EMSI line */

  if (!EmsiGetHeader(frame) ||
      !EmsiReadLine())
  {
    EmsiTxFrame(EMSI_NAK);
    return TRUE;
  }

  /* Now call the appropriate handler function for this frame */

  for (ep=eft; *ep->type; ep++)
    if (! *ep->type || strncmp(ep->type, frame, 3)==0)
    {
      if (!EmsiCrcOK(ep->crc_len) || ! (*ep->handler)())
        EmsiTxFrame(EMSI_NAK);

      break;
    }

  return TRUE;
}


/* Check to see if we've matched the initial **EMSI_ sequence */

static word near EmsiGotInit(int ch)
{
  static word estate=0;

  /* If character is not equal to the next in sequence, it's not an         *
   * EMSI frame.                                                            */

  if ((char)ch != emsi_seq[estate])
  {
    estate=0;
    return FALSE;
  }

  /* If we haven't reached the end of the sequence, return to caller.       */

  if (emsi_seq[++estate])
    return FALSE;

  estate=0;

  return TRUE;
}



/* Receive the three characters following the initial "**EMSI_" sequence */

static word near EmsiGetHeader(char *frame)
{
  long t;
  char *fp;

  /* Give three seconds to receive rest of header type */

  t=timerset(300);

  /* Read in the header */

  for (fp=frame; !timeup(t) && fp < frame+3; )
    if (Mdm_keyp())
      *fp++=Mdm_getcw() & 0x7f;

  *fp='\0';

  strcpy(emsi_buf, emsi_seq);
  strcat(emsi_buf, frame);

  logit("^EmsiGetHeader - Got %s", frame);

  /* If we didn't get the whole thing... */

  return (!timeup(t));
}


/* Read a line in from remote, terminating when 20 seconds have             *
 * expired (or until we get a \r).                                          */

static word near EmsiReadLine(void)
{
  long t;
  char *p;

  logit("^EmsiReadLine");

  /* Read the line into our EMSI buffer */

  t=timerset(2000);

  for (p=emsi_buf+strlen(emsi_buf); !timeup(t) && p < emsi_max; )
    if (Mdm_keyp())
      if ((*p=((byte)Mdm_getcw() & 0x7f))=='\r')
        break;
      else p++;

  *p='\0';

  emsi_len=p-emsi_buf;

  return (!timeup(t));
}


static word near EmsiCalcCrc16(word len)
{
  word crc16;
  char *s, *e;

  char temp[180];
  strncpy(temp, emsi_buf+2, 179);
  temp[179]='\0';

/*  logit("^Line=`%s' (len=%d)", temp, len);*/

  crc16=0;

  /* Start CRCing at the "EMSI_xxx", skipping over the ** */

  for (s=emsi_buf+2, e=s+len; s < e; s++)
    crc16=crc16fn(*s, crc16);

  logit("@EmsiCalcCrc16 - %04x", crc16);
  return crc16;
}


static dword near EmsiCalcCrc32(word len)
{
  dword crc32;
  char *s, *e;

  char temp[180];
  strncpy(temp, emsi_buf+2, 179);
  temp[179]='\0';

/*  logit("^Line=`%s' (len=%d)", temp, len);*/

  crc32=0xffffffffLu;

  for (s=emsi_buf+2, e=s+len; s < e; s++)
    crc32=crc32fn(*s, crc32);

  logit("@EmsiCalcCrc32 - %08lx", crc32);
  return crc32;
}



/* Calculate the CRC of the frame in EMSI_BUF and ensure that it's okay */

static word near EmsiCrcOK(word len)
{
  dword crc32, c32;
  word crc16, c16;
  char *e=emsi_buf+strlen(emsi_buf);


  /* Calculate the alleged CRC of our packet */

  if (len==16)
  {
    e -= 4;

    logit("@EmsiCrcOK16 - ASCII CRC=%s", e);

    if (sscanf(e, px, &crc16) != 1)
      return FALSE;
  }
  else
  {
    e -= 8;

    logit("@EmsiCrcOK32 - ASCII CRC=%s", e);

    if (sscanf(e, plx, &crc32) != 1)
      return FALSE;
  }


  /* Now calculate the actual CRC */

  if (len==16)
  {
    c16=EmsiCalcCrc16(e-emsi_buf-2);
    logit("@EmsiCrcOK16 - Calculated=%04x, Actual=%04x", c16, crc16);
  }
  else
  {
    c32=EmsiCalcCrc32(e-emsi_buf-2);
    logit("@EmsiCrcOK32 - Calculated=%08lx, Actual=%08lx", c32, crc32);
  }

  if ((len==16 && c16 != crc16) ||
      (len==32 && c32 != crc32))
    return FALSE;

  return TRUE;
}



/* EMSI Frame Type ACK - Acknowledgement of last packet */

static word near EmsiFtACK(void)
{
  logit("^EmsiFtACK");

  if (!fGotICI)
    return TRUE;

  return TRUE;
}


/* EMSI Frame Type NAK - Negative acknowledge; retransmit last packet */

static word near EmsiFtNAK(void)
{
  logit("^EmsiFtNAK");

  if (!fGotICI)
    return TRUE;

  /**/
  return TRUE;
}


/* EMSI Frame Type IIR - Request from remote to abort EMSI negotiation */

static word near EmsiFtIIR(void)
{
  logit("^EmsiFtIIR");
  fAbort=TRUE;
  return TRUE;
}


/* EMSI Frame Type ICI - Interactive Caller Information */
/* **EMSI_ICI<len><data><crc32><CR> */

static word near EmsiFtICI(void)
{
  long t1;
  struct _efield *ef;
  char buf[20];
  char *eb;
  word tries, len, frame;

  logit("^EmsiFtICI");

  /* Don't accept multiple EMSI_ICI packets */

  if (fGotICI || fLoggedOn)
    return TRUE;


  /* Extract the length from the packet header */

  memmove(buf, emsi_buf+10, 4);
  buf[4]='\0';
  eb=emsi_buf+14;


  /* Extract length from packet header */

  if (sscanf(buf, px, &len) != 1)
    return FALSE;

  if (len > emsi_len-4)
    len=emsi_len-4;


  /* Now parse all of the fields out of the header */

  for (ef=ici_fields; ef->where; ef++)
    if (!EmsiGetField(&eb, ef->where, ef->maxlen))
    {
      /* Couldn't decypher field in packet, so terminate negotiation */

      EmsiTxFrame(EMSI_IIR);
      fAbort=TRUE;
      return TRUE;
    }

  logit("^EmsiFtICI - Name: %s", eclr.ec_name);
  logit("^EmsiFtICI - Alias: %s", eclr.ec_alias);
  logit("^EmsiFtICI - City: %s", eclr.ec_city);
  logit("^EmsiFtICI - Phdata: %s", eclr.ec_phdata);
  logit("^EmsiFtICI - Phvoice: %s", eclr.ec_phvoice);
  logit("^EmsiFtICI - Pwd: %s", eclr.ec_pwd);
  logit("^EmsiFtICI - Dob: %s", eclr.ec_dob);
  logit("^EmsiFtICI - Crt: %s", eclr.ec_crt);
  logit("^EmsiFtICI - Proto: %s", eclr.ec_proto);
  logit("^EmsiFtICI - Cap: %s", eclr.ec_cap);
  logit("^EmsiFtICI - Req: %s", eclr.ec_req);
  logit("^EmsiFtICI - Sw: %s", eclr.ec_sw);
  logit("^EmsiFtICI - Xlat: %s", eclr.ec_xlat);

  fGotICI=TRUE;


  /* Now try to send the _ISI frame to the remote */

  tries=4;
  frame=EMSI_BAD;

  while (frame != EMSI_ACK && frame != EMSI_IIR && --tries)
  {
    /* Transmit the "Server Information" frame */

    EmsiTxFrame(EMSI_ISI);

    t1=timerset(2000);

GetNext:

    switch (frame=EmsiPollFrame(t1))
    {
      case EMSI_ACK:
        logit("^EmsiFtICI-ISI: Got ACK");
        fEMSI=TRUE;
        break;

      case EMSI_NAK:
        logit("^EmsiFtICI-ISI: Got NAK");
/*      EmsiFtNAK();*/
        break;

      case EMSI_IIR:
        logit("^EmsiFtICI-ISI: Got IIR");
        EmsiTxFrame(EMSI_IIR);  
        fAbort=TRUE;
        break;

      case EMSI_BAD:
        logit("^EmsiFtICI-ISI: Got BAD");
        break;

      default:
        logit("^EmsiFtICI-ISI: Got unknown frame %d", frame);
        goto GetNext;
    }
  }


  /* Abort session if we timed out */

  if (!tries)
  {
    EmsiTxFrame(EMSI_IIR);
    fAbort=TRUE;
  }

  return TRUE;
}


/* Get the next frame from the remote, not echoing back any characters.     *
 * Return the frame type to teh caller for processing.                      */

static word near EmsiPollFrame(long t)
{
  struct _eft *ep;
  char frame[5];


  logit("^EmsiPollFrame");

  /* Get the next EMSI frame or die trying */

  for (;;)
  {
    if (timeup(t) || !carrier())
      return EMSI_BAD;

    if (Mdm_keyp() && EmsiGotInit(Mdm_getcw()))
      break;
  }

  logit("^EmsiPollFrame - Call GetHeader");

  /* Read frame and rest of EMSI line */

  if (!EmsiGetHeader(frame) ||
      !EmsiReadLine())
  {
    logit("^EmsiPollFrame - NAK");
    EmsiTxFrame(EMSI_NAK);
    return EMSI_BAD;
  }

  logit("^EmsiPollFrame - Got `%s'", emsi_buf);

  /* Now call the appropriate handler function for this frame */

  for (ep=eft; *ep->type; ep++)
    if (! *ep->type || strncmp(ep->type, frame, 3)==0)
    {
      logit("^EmsiPollFrame - Call `%s' handler", ep->type);
      return ep->ft;
    }

  logit("^EmsiPollFrame - Return EMSI_BAD");
  return EMSI_BAD;
}



/* If we received a bad frame, return TRUE (meaning to do nothing). */

static word near EmsiFtBAD(void)
{
  logit("^EmsiFtBAD");
  return TRUE;
}




/* Parse a single ASCII1 field out of an EMSI packet */

static word near EmsiGetField(char **buf, char *field, word len)
{
  char *p=*buf;
  char *o, *omax;
  char temp[20];
  int ch;

  /* Make sure that field starts with a '}' */

  if (*p++ != '{')
    return FALSE;

  /* Loop around until we receive the entire field */

  for (o=field, omax=o+len; *p != '}' && o < omax-1; )
  {
    /* Parse an escaped eight-bit character */

    if (*p=='\\')
    {
      /* Two backslashes in a row means a single backslash */

      if (*++p=='\\')
        *o++=*p++;
      else
      {
        memmove(temp, p, 2);

        temp[2]='\0';

        if (sscanf(temp, px, &ch) != 1)
          return FALSE;

        *o++=(byte)ch;
        p += 2;
      }
    }
    else
    {
      *o++=*p++;
    }
  }


  /* Now skip to find the '}', in case our buffer overflowed */

  if (*p != '}')
    while (p < omax-1 && *p != '}')
      p++;

  *o='\0';
  *buf=p+1;
}








/****************************************************************************/
/*                    TURNAROUND:  Transmit IEMSI Frames                    */
/****************************************************************************/


/* Transmit an EMSI frame to the client */

void EmsiTxFrame(word ft)
{
  char buf[30];
  struct _eft *ep;
  word crc16;
  long crc32;

  logit("@EmsiTxFrame1");

  /* Find the specified frame type */

  for (ep=eft; ep->ft; ep++)
    if (ep->ft==ft)
      break;

  /* Unknown frame type */

  if (!ep->ft)
    return;


  /* Build the beginning of the EMSI sequence */

  strcpy(emsi_buf, emsi_seq);
  strcat(emsi_buf, ep->type);


  /* Build the middle of the block */

  logit("@EmsiTxFrame2 - Send %s", ep->type);

  if (ep->bldfunc)
    (*ep->bldfunc)();

  logit("@EmsiTxFrame3");


  /* Calculate the CRC */

  if (ep->crc_len==16)
  {
    crc16=EmsiCalcCrc16(strlen(emsi_buf)-2);
    sprintf(buf, "%04X\r", crc16);

    logit("^EmsiTxFrame: **EMSI_%s (crc=%04x)", ep->type, crc16);
  }
  else
  {
    crc32=EmsiCalcCrc32(strlen(emsi_buf)-2);
    sprintf(buf, "%08lX\r", crc32);

    logit("^EmsiTxFrame: **EMSI_%s (crc=%08lx)", ep->type, crc32);
  }

  strcat(emsi_buf, buf);
  EmsiTxBytes(emsi_buf, strlen(emsi_buf));
  logit("@EmsiTxFrame4");
}


/* Transmit a sequence of bytes to the remote end.  Wait up to 30 seconds   *
 * for buffer to clear.                                                     */

static word near EmsiTxBytes(char *buf, word len)
{
  long txtimer;
  int got;
  char *s;

  if (local)
    return TRUE;

  txtimer=timerset(3000);

  logit("@EmsiTxBytes");

  for (s=buf; (got=mdm_blockwrite(len, s)) < len; )
  {
    if (!carrier() || timeup(txtimer))
    {
      logit("^EmsiTxBytes - Timeout");
      return FALSE;
    }

    s += got;
    len -= got;

    if (len > 100)
      Giveaway_Slice();
  }

  return TRUE;
}




/* Add an ASCII1 string to the current emsi_buf, translating as necessary */

static void near EmsiBldA1(char *fmt, ...)
{
  char string[180];
  char buf[20];
  byte *eb, *p;

  va_list var_args;


  /* Convert printf formatting into a single string */

  va_start(var_args, fmt);
  vsprintf(string, fmt, var_args);
  va_end(var_args);


  /* Start working from the end of the current EMSI buffer */

  eb=emsi_buf+strlen(emsi_buf);

  *eb++='{';


  /* Now copy the string into the buffer */

  for (p=string; *p; p++)
  {
    if (*p=='\\' || *p=='{' || *p=='}' || *p < ' ' || *p >= 0x7f)
    {
      /* Translate special characters into hex notation */

      sprintf(buf, "\\%02X", *p);

      strcpy(eb, buf);
      eb += strlen(eb);
    }
    else *eb++=*p;
  }

  *eb++='}';
  *eb='\0';
}




/* Build a server information block to transmit to client */

static void near EmsiBldISI(void)
{
  char buf[20];
  char *lp;

  /* Leave space for the len16 field */

  lp=emsi_buf+strlen(emsi_buf);
  strcpy(lp, "    ");

  EmsiBldA1("%s,%s", us_short, tear_ver);     /* {Maximus/2,2.0 b/1} */
  EmsiBldA1(percent_s, PRM(system_name));          /* {FWP} */
  EmsiBldA1(percent_s, location);                  /* {Kingston, Ontario} */
  EmsiBldA1(percent_s, PRM(sysop));                /* {Scott Dudley} */
  EmsiBldA1("%08lX", time(NULL));             /* {01234567} */
  EmsiBldA1("Maximus - Copyright " THIS_YEAR " by Scott J. Dudley.  All rights reserved.");
  EmsiBldA1("\x01");                          /* {} */
  EmsiBldA1("");                              /* {} */

  /* Now calculate the length of the frame and place it in the appropriate  *
   * spot.                                                                  */

  sprintf(buf, "%04X", strlen(emsi_buf)-14);
  memmove(lp, buf, 4);
}


/* Build an ASCII image to send to client - not implemented */

static void near EmsiBldISM(void)
{
  /* null */
}


/* Unknown frame - transmit nothing */

static void near EmsiBldBAD(void)
{
  /* null */
}

#endif /*EMSI*/

