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

static char emsi_seq[]="**EMSI_";

static struct _emsi_caller eclr;

static word near EmsiGotInit(int ch);
static word near EmsiGetHeader(char *frame);
static word near EmsiReadLine(void);
static word near EmsiCalcCrc16(word len);
static dword near EmsiCalcCrc32(word len);
static word near EmsiCrcOK(word len);
static word near EmsiFtACK(void);
static word near EmsiFtNAK(void);
static word near EmsiFtIIR(void);
static word near EmsiFtICI(void);
static word near EmsiPollFrame(long t);
static word near EmsiFtBAD(void);
static word near EmsiGetField(char **buf, char *field, word len);
static word near EmsiTxBytes(char *buf, word len);
static void near EmsiBldA1(char *fmt, ...);
static void near EmsiBldISI(void);
static void near EmsiBldISM(void);
static void near EmsiBldBAD(void);


static struct _eft eft[]=
{
  {"ACK", EMSI_ACK, EmsiFtACK, NULL,       16},
  {"NAK", EMSI_NAK, EmsiFtNAK, NULL,       16},
  {"IRQ", EMSI_IRQ, EmsiFtBAD, NULL,       16}, /* used by server only */
  {"IIR", EMSI_IIR, EmsiFtIIR, NULL,       16},
  {"ICI", EMSI_ICI, EmsiFtICI, EmsiBldBAD, 32}, /* used by client only */
  {"ISI", EMSI_ISI, EmsiFtBAD, EmsiBldISI, 32}, /* used by server only */
  {"ISM", EMSI_ISM, EmsiFtBAD, EmsiBldISM, 32}, /* used by server only */
  {"CHT", EMSI_CHT, EmsiFtBAD, NULL,       16}, /* used by server only */
  {"TCH", EMSI_TCH, EmsiFtBAD, NULL,       16}, /* used by server only */
  {""   , EMSI_BAD, EmsiFtBAD, EmsiBldBAD}
};


static struct _efield ici_fields[]=
{
  {eclr.ec_name,    EC_NAME_LEN},
  {eclr.ec_alias,   EC_ALIAS_LEN},
  {eclr.ec_city,    EC_CITY_LEN},
  {eclr.ec_phdata,  EC_PHDATA_LEN},
  {eclr.ec_phvoice, EC_PHVOICE_LEN},
  {eclr.ec_pwd,     EC_PWD_LEN},
  {eclr.ec_dob,     EC_DOB_LEN},
  {eclr.ec_crt,     EC_CRT_LEN},
  {eclr.ec_proto,   EC_PROTO_LEN},
  {eclr.ec_cap,     EC_CAP_LEN},
  {eclr.ec_req,     EC_REQ_LEN},
  {eclr.ec_sw,      EC_SW_LEN},
  {eclr.ec_xlat,    EC_XLAT_LEN},
  {NULL,            0},
};

