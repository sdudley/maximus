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
static char rcs_id[]="$Id: async.c,v 1.1 2002/10/01 17:50:45 sdudley Exp $";
#pragma on(unreferenced)

#ifdef OS_2

#define MAX_INCL_COMMS
#include <stdio.h>
#include <ctype.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#define INCL_NOPM
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include "pos2.h"
#undef CONTROL
#include "comm.h"
#include "mm.h"

HCOMM hcModem=0;            /* comm.dll handle */

/* 0 - not a virtual modem port, or vmodem not running
 * 1 - no virtual modem connection
 * 2 - active virtual modem connection
 * 3 - active telnet connection
 */

int GetConnectionType(void)
{
  HFILE hf;
  int rc;

  typedef struct
  {
    USHORT cbSize;
    BYTE bStatus;
  } VMPORTDATA;

  VMPORTDATA vpd = {sizeof(VMPORTDATA), 0};

  if (local)
    return CTYPE_LOCAL;

  #ifdef __FLAT__
  {
    ULONG dlen, plen;

    hf = ComGetFH(hcModem);

    plen=0;
    dlen=sizeof vpd;

    rc = DosDevIOCtl(hf, IOCTL_ASYNC, 0xe8, NULL, 0L, &plen,
                     &vpd, sizeof vpd, &dlen);
  }
  #else
    hf = ComGetFH(hcModem);

    rc = DosDevIOCtl2(&vpd, sizeof vpd, (PVOID)0, 0,
                      0xe8, IOCTL_ASYNC, hf);
  #endif

  return rc ? CTYPE_ASYNC : (int)vpd.bStatus;
}

void com_DTR_on(void)
{
    DCBINFO sDCB;
    USHORT rc;

    if(!(rc=ComGetDCB(hcModem, &sDCB))){
        sDCB.fbCtlHndShake |= MODE_DTR_CONTROL;   /* raise DTR */
        ComSetDCB(hcModem, &sDCB);
    }
    else
        logit("!SYS%04u: ComGetDCB()", rc);
}

void com_DTR_off(void)
{
    DCBINFO sDCB;
    USHORT rc;

    if(!(rc=ComGetDCB(hcModem, &sDCB))){
        sDCB.fbCtlHndShake &= ~MODE_DTR_CONTROL; /* lower DTR */
        ComSetDCB(hcModem, &sDCB);
    }
    else
        logit("!SYS%04u: ComGetDCB()", rc);
}

void com_XON_disable(void)
{
    DCBINFO sDCB;
    USHORT rc;

    if(!(rc=ComGetDCB(hcModem, &sDCB))){
        /* disable auto Xmit and recv flow control */
        sDCB.fbFlowReplace &= ~(MODE_AUTO_TRANSMIT | MODE_AUTO_RECEIVE);
        ComSetDCB(hcModem, &sDCB);
    }
    else
        logit("!SYS%04u: ComGetDCB()", rc);
}

void com_XON_enable(void)
{
    DCBINFO sDCB;
    USHORT rc;

    if(!(rc=ComGetDCB(hcModem, &sDCB))){
        /* enable auto Xmit and recv flow control */
        sDCB.fbFlowReplace |= MODE_AUTO_TRANSMIT;   /*PLF Wed  04-04-1990  02:35:41 */
        ComSetDCB(hcModem, &sDCB);
    }
    else
        logit("!SYS%04u: ComGetDCB()", rc);
}

/* com_break() : start break if on==TRUE, stop break if on==FALSE */
void com_break(int on)
{
  OS2UINT cmd;
  OS2UINT comerr;
  OS2UINT rc;
  HFILE hf;

  cmd = on ? ASYNC_SETBREAKON : ASYNC_SETBREAKOFF;
  hf = ComGetFH(hcModem);

  if (hf)
  {
    #ifdef __FLAT__
      ULONG dlen, plen;

      plen=0;
      dlen=sizeof(comerr);

      rc=DosDevIOCtl(hf, IOCTL_ASYNC, cmd, NULL, 0L, &plen,
                     &comerr, sizeof(comerr), &dlen);
    #else
      rc=DosDevIOCtl(&comerr, 0L, cmd, IOCTL_ASYNC, hf);
    #endif

    if (rc)
      logit("!SYS%04u: ASYNC_SETBREAK", rc);
  }
}

static void near ShowMdmSettings(void)
{
#if 1
    return;
#else
    DCBINFO dcb;
    char *On = "On";
    char *Off = "Off";
    char *dtr;
    char *rts;
    char *buffer;
    int Rx=0, Tx;

    if(!ComGetDCB(hcModem, &dcb)){
        logit(" Modem: TO=%s,XON(Rx)=%s,XON(Tx)=%s",
            (dcb.fbTimeout & MODE_NO_WRITE_TIMEOUT)? On : Off,
            (dcb.fbFlowReplace & MODE_AUTO_RECEIVE)? On : Off,
            (dcb.fbFlowReplace & MODE_AUTO_TRANSMIT)? On : Off);
        logit(" Modem: IDSR=%s,ODSR=%s,OCTS=%s",
            (dcb.fbCtlHndShake & MODE_DSR_SENSITIVITY)? On : Off,
            (dcb.fbCtlHndShake & MODE_DSR_HANDSHAKE)? On : Off,
            (dcb.fbCtlHndShake & MODE_CTS_HANDSHAKE)? On : Off);
        switch(dcb.fbCtlHndShake & (MODE_DTR_CONTROL | MODE_DTR_HANDSHAKE)){
            case 0: dtr = Off; break;
            case MODE_DTR_CONTROL: dtr = On; break;
            case MODE_DTR_HANDSHAKE: dtr = "IHS"; break;     /* input handshaking */
            default:
                dtr = "??"; break;
        }
        switch(dcb.fbFlowReplace & (MODE_RTS_CONTROL | MODE_RTS_HANDSHAKE | MODE_TRANSMIT_TOGGLE)){
            case 0: rts = Off;  break;
            case MODE_RTS_CONTROL: rts = On; break;
            case MODE_RTS_HANDSHAKE: rts = "IHS"; break;
            case MODE_TRANSMIT_TOGGLE: rts = "TOG"; break;
            default:
                rts = "??"; break;
        }
        switch(dcb.fbTimeout & 0x18){
            case 0x08: buffer = Off; break;
            case 0x10: buffer = On; break;
            case 0x18: buffer = "Auto"; break;
            default: buffer = "??"; break;
        }
        switch(dcb.fbTimeout & 0x60){
                case 0:    Rx = 1;  break;
                case 0x20: Rx = 4;  break;
                case 0x40: Rx = 8;  break;
                case 0x60: Rx = 14; break;
        }
        Tx = (dcb.fbTimeout & 0x80)? 16 : 1;
        logit(" Modem: DTR=%s,RTS=%s,BUFFER=%s (Rx=%d, Tx=%d)",
            dtr,
            rts,
            buffer,
            Rx, Tx);
        /*logit(" Modem: dcb.FbTimeout  =%04X", dcb.fbTimeout);*/
    }
#endif
}

int Cominit(int port)
{
    char tmp[20];
    USHORT rc;
    HFILE hf;

    if (hcModem)
    {
      ComResume(hcModem);
      return 0x1954;
    }

    if (port_is_device)
    {
      sprintf(tmp, "com%1u", port+1);
      rc=ComOpen(tmp, &hcModem, 8200, 8200);
    }
    else
    {
      sprintf(tmp, "handle %d", port+1);
      hf=(HFILE)port+1;
      rc=ComHRegister(hf, &hcModem, 8200, 8200);
    }

    if (rc)
    {
      logit("!SYS%04u:  Cannot open com port (%s)", rc, tmp);
      return 0;
    }

    DosSetPrty(PRTYS_THREAD, PRTYC_NOCHANGE, 2, 0);
    ShowMdmSettings();

    return 0x1954;
}

#endif /* OS_2 */

