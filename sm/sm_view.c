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

#define INCL_PM
#define INCL_DOS
#define INCL_VIO
#define INCL_AVIO

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "pos2.h"
#include "max.h"
#include "mcp.h"
#include "smrc.h"
#include "sm.h"
#include "pmdebug.h"

int AvioPrintf(LINEINFO *pli, char *format,...);
extern HPIPE hp;
extern char szViewClass[];


static void near AvioCls(HVPS hvps, int attr)
{
  byte cell[2];

  cell[0]=' ';
  cell[1]=(byte)attr;

  VioScrollDn(0, 0, VIO_HEIGHT-1, VIO_WIDTH-1, VIO_HEIGHT, &cell, hvps);
}



static void near loc_putc(LINEINFO *pli, int ch)
{
  byte b=ch;

  if (ch==8)
  {
    if (pli->usCurCol)
      pli->usCurCol--;

    return;
  }
  else if (ch==13)
  {
    pli->usCurCol=0;
    return;
  }

  if (ch==10)
    pli->usCurRow++;
  else
  {
    VioWrtCharStrAtt(&b, 1, pli->usCurRow, pli->usCurCol++,
                   &pli->avp.curattr, pli->hvps);
  }

  if (pli->usCurCol==VIO_WIDTH)
  {
    pli->usCurCol=0;
    pli->usCurRow++;
  }


  if (pli->usCurRow >= VIO_HEIGHT)
  {
    byte cell[2];

    cell[0]=' ';
    cell[1]=pli->avp.curattr;

    VioScrollUp(0, 0, VIO_HEIGHT-1, VIO_WIDTH-1, 1, &cell, pli->hvps);
    pli->usCurRow--;
  }
}


/* Output one AVIO character */

static void near AvioPutc(LINEINFO *pli, int ch)
{
  if (pli->avp.state==-1)
  {
    switch (ch)
    {
      case 9:
        /* So tabs will work reliably in lputs() */

        if (pli->avp.last_cc==-1)
        {
          pli->avp.save_cx=pli->usCurRow+1;
          pli->avp.last_cc=pli->usCurCol+1;
        }

        for (pli->avp.x=0;
             pli->avp.x < (word)(9-(pli->avp.last_cc % 8));
             pli->avp.x++)
        {
          loc_putc(pli, ' ');
        }

        pli->avp.last_cc=-1;
        break;

      case 10:
        pli->usCurCol=0;

        if (++pli->usCurRow >= VIO_HEIGHT)
        {
          byte cell[2];

          cell[0]=' ';
          cell[1]=pli->avp.curattr;

          VioScrollUp(0, 0, VIO_HEIGHT-1, VIO_WIDTH-1, 1, &cell, pli->hvps);
          pli->usCurRow--;
        }
        break;

      case 12:
        if (1)/*(usr.bits2 & BITS2_CLS)*/
        {
          AvioCls(pli->hvps, CGRAY);
          pli->usCurCol=pli->usCurRow=0;
          pli->avp.curattr=3;
        }
        else
        {
          AvioPutc(pli, 13);
          AvioPutc(pli, 10);
        }
        break;

      case 13:
        pli->usCurCol=0;
        break;

      case 22:
        pli->avp.state=0;
        break;

      case 25:
        pli->avp.state=25;
        break;

      case 7:   /* Strip out any beeps intended for the user */
        break;

      default:
        loc_putc(pli, ch);
        break;
    }
  }
  else switch (pli->avp.state)
  {
    case 0:
      switch (ch)
      {
        case 1:                   /* Attribute, get another character */
          pli->avp.state=1;
          break;

        case 2:
          pli->avp.curattr |= _BLINK;
          pli->avp.state=-1;
          break;

        case 3:
          if (pli->usCurRow)
            pli->usCurRow--;

          pli->avp.state=-1;
          break;

        case 4:
          if (pli->usCurRow < VIO_HEIGHT-1)
            pli->usCurRow++;

          pli->avp.state=-1;
          break;

        case 5:
          if (pli->usCurCol)
            pli->usCurCol--;

          pli->avp.state=-1;
          break;

        case 6:
          if (pli->usCurCol < VIO_WIDTH-1)
            pli->usCurCol++;

          pli->avp.state=-1;
          break;

        case 7:
        {
          byte cell[2];
          cell[0]=' ';
          cell[1]=pli->avp.curattr;

          VioScrollDn(pli->usCurRow, pli->usCurCol, pli->usCurRow,
                      VIO_WIDTH-1, 1, &cell, pli->hvps);

          pli->avp.state=-1;
          break;
        }

        case 8:                   /* Goto, get another character */
          pli->avp.state=8;
          break;

        case 9:                   /* Insert mode on -- not supported */
        case 10:                  /* Scroll area up */
        case 11:                  /* Scroll area down */
          break;

        case 12:                  /* Clear area */
          pli->avp.state=12;
          break;

        case 13:                  /* Fill area */
          pli->avp.state=15;
          break;

        case 14:                  /* Delete character at cursor */
          break;

        case 25:                  /* Repeat pattern */
          pli->avp.state=30;
          break;

        default:
          pli->avp.state=-1;
      }
      break;

    case 1:
      if (ch != DLE)              /* Attribute */
      {
        pli->avp.curattr=(char)ch;
        pli->avp.state=-1;
      }
      else pli->avp.state=2;
      break;

    case 2:                         /* Attribute DLE */
      pli->avp.curattr=ch & 0x7f;
      pli->avp.state=-1;
      break;

    case 8:                         /* Goto1 */
      pli->avp.save_cx=(unsigned char)ch;
      pli->avp.state=10;
      break;

    case 10:                        /* Goto2 */
      pli->avp.state=-1;

      pli->usCurRow=pli->avp.save_cx-1;
      pli->usCurCol=ch-1;
      break;

    case 12:                        /* Clear 1 */
      pli->avp.save_cx=(unsigned char)ch;
      pli->avp.state=13;
      break;

    case 13:                        /* Clear 2 */
      pli->avp.s2=ch+1;
      pli->avp.state=14;
      break;

    case 14:                        /* Clear 3 */
      pli->avp.y=pli->usCurRow+1;
      pli->avp.z=pli->usCurCol+1;
      pli->avp.a=pli->avp.save_cx;
      pli->avp.state=-1;

      AvioPrintf(pli, "\x16\x01%c", pli->avp.a);

      for (pli->avp.x=0; pli->avp.x < pli->avp.s2; pli->avp.x++)
        AvioPrintf(pli, "\x16\x08%c%c\x19 %c", pli->avp.y+pli->avp.x, pli->avp.z,
                   (char)ch+1);

      AvioPrintf(pli, "\x16\x08%c%c", pli->avp.y, pli->avp.z);
      break;

    case 15:                        /* Fill 1 */
      pli->avp.save_cx=(unsigned char)ch;
      pli->avp.state=16;
      break;

    case 16:                        /* Fill 2 */
      pli->avp.s2=ch;
      pli->avp.state=17;
      break;

    case 17:                        /* Fill 3 */
      pli->avp.s3=ch+1;
      pli->avp.state=18;
      break;

    case 18:                        /* Fill 4 */
      pli->avp.y=pli->usCurRow+1;
      pli->avp.z=pli->usCurCol+1;
      pli->avp.a=pli->avp.save_cx;
      pli->avp.state=-1;

      AvioPrintf(pli, "\x16\x01%c", pli->avp.a);

      for (pli->avp.x=0; pli->avp.x < pli->avp.s3; pli->avp.x++)
        AvioPrintf(pli, "\x16\x08%c%c\x19%c%c", pli->avp.y+pli->avp.x,
                pli->avp.z, pli->avp.s2, ch+1);

        AvioPrintf(pli, "\x16\x08%c%c", pli->avp.y, pli->avp.z);
      break;

    case 25:                      /* RLE1 */
      pli->avp.save_cx=(byte)ch;
      pli->avp.state=27;
      break;

    case 27:                      /* RLE2 */
      {
        word x;
        byte c;
        int uch;

        c=(byte)pli->avp.save_cx;

        uch=(byte)ch;

        for (x=0; x < uch; x++)
          loc_putc(pli, c);

        pli->avp.state=-1;
      }
      break;

    case 30:
      pli->avp.save_cx=(unsigned char)ch;
      pli->avp.x=0;
      pli->avp.state=31;
      break;

    case 31:
      if (pli->avp.x < 24 && pli->avp.x < pli->avp.save_cx)
        pli->avp.str2[pli->avp.x++]=(char)ch;
      else
      {
        word y;

        pli->avp.str2[pli->avp.x]='\0';
        pli->avp.state=-1;

        pli->avp.uch=(byte)ch;

        for (y=0; y < pli->avp.uch; y++)
          AvioPrintf(pli, "%s", pli->avp.str2);
      }
      break;

    default:
      pli->avp.state=-1;
      break;
  }
}

void AvioPuts(LINEINFO *pli, char *psz)
{
  while (*psz)
    AvioPutc(pli, *psz++);
}

int AvioPrintf(LINEINFO *pli, char *format,...)
{
  va_list var_args;
  int x;

  char string[MAX_PRINTF];

  if (strlen(format) >= MAX_PRINTF)
    return -1;

  va_start(var_args,format);
  x=vsprintf(string,format,var_args);
  va_end(var_args);

  AvioPuts(pli, string);
  return x;
}



/* We got a video i/o dump from the kids */

static void near ViewVio(LINEINFO *pli, byte *pb, USHORT cbGot)
{
  /* Write all of the characters to the window */

  if (cbGot)
    while (--cbGot)
      AvioPutc(pli, *++pb);

  VioSetCurPos(pli->usCurRow, pli->usCurCol, pli->hvps);
//  WinInvalidateRect(hwnd, NULL, FALSE);
}



/* We got a video i/o dump from the kids */

static void near ViewVioDump(HWND hwnd, LINEINFO *pli, byte *pb)
{
  VIO_DUMP_HDR *pdh;
  USHORT usHeight, usWidth;

  pb++;                     /* skip over task number */
  pdh=(VIO_DUMP_HDR *)pb;
  usHeight=pdh->bHeight;
  usWidth=pdh->bWidth;
  pli->usCurRow=pdh->bCurRow;
  pli->usCurCol=pdh->bCurCol;
  pli->avp.curattr=pdh->bCurAttr;

  pb=(char *)(pdh+1);


  /* Copy the cells to the screen */

  VioWrtCellStr(pb, usHeight * usWidth * sizeof(USHORT), 0, 0, pli->hvps);

  /* Set the virtual cursor position */

  VioSetCurPos(pli->usCurRow, pli->usCurCol, pli->hvps);

  /* Redraw the window */

  WinInvalidateRect(hwnd, NULL, FALSE);

  pli->avp.last_cc=-1;
  pli->avp.state=-1;
}



/* Window procedure for the viewer window */

MRESULT EXPENTRY ViewProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2)
{
  LINEINFO *pli;
  HPS hps;

  switch (msg)
  {
    case WM_CREATE:
    {
      HWND hwndSys;
      HWND hwndFrame;

      /* Get the system window handle */

      hwndFrame = WinQWindow(hwnd, QW_PARENT);
      hwndSys = WinWindowFromID(hwndFrame, FID_SYSMENU);

      /* Delete the unused menu items */

      WinSendMsg(hwndSys,
                 MM_DELETEITEM,
                 MPFROM2SHORT(SC_MAXIMIZE, TRUE),
                 MPFROMLONG(0));

      WinSendMsg(hwndSys,
                 MM_DELETEITEM,
                 MPFROM2SHORT(SC_SIZE, TRUE),
                 MPFROMLONG(0));

      WinSendMsg(hwndSys,
                 MM_DELETEITEM,
                 MPFROM2SHORT(SC_RESTORE, TRUE),
                 MPFROMLONG(0));
      break;
    }

    case WM_PAINT:
      pli=WinQueryWindowPtr(hwnd, 0);
      hps=WinBeginPaint(hwnd, 0, NULL);
      VioShowPS(VIO_HEIGHT, VIO_WIDTH, 0, pli->hvps);
      WinEndPaint(hps);
      break;

    case WM_SIZE:
      return WinDefAVioWindowProc(hwnd, msg, (ULONG)mp1, (ULONG)mp2);

    case WM_CLOSE:
      WinDestroyWindow(WinQWindow(hwnd, QW_PARENT));
      return 0;

    case SM_VIO:
      /* Handle standard video i/o */

      ViewVio((LINEINFO *)WinQueryWindowPtr(hwnd, 0),
              PVOIDFROMMP(mp2), SHORT2FROMMP(mp1));

      free(PVOIDFROMMP(mp2));
      break;

    case SM_VIODUMP:
      /* Handle a video screen dump */

      ViewVioDump(hwnd, (LINEINFO *)WinQueryWindowPtr(hwnd, 0),
                  PVOIDFROMMP(mp2));

      free(PVOIDFROMMP(mp2));
      break;

    case WM_CHAR:
#if 0
      /* Handle key i/o for the specified task */

      if (SHORT1FROMMP(mp1) & KC_KEYUP)
        break;

      if (CHAR1FROMMP(mp2) && (SHORT1FROMMP(mp1) & KC_CHAR))
      {
        byte msg[2];
        int i=CHAR3FROMMP(mp1);

        pli=WinQueryWindowPtr(hwnd, 0);
        msg[0]=pli->tid;
        msg[1]=CHAR1FROMMP(mp2);


        while (i--)
        {
          McpSendMsg(hp, CLMSG_KEY, msg, 2);
          DbgPrintf("Got key \"%c\" for tid %d", msg[1], msg[0]);
        }

      }
#endif
      break;

    case WM_DESTROY:    /* Clean-up processing */
      pli=WinQueryWindowPtr(hwnd, 0);

      /* Tell MCP to stop sending overview information */

      McpSendMsg(hp, CLMSG_ENDMONITOR, &pli->tid, 1);

      /* Delete the PS */

      VioAssociate(NULL, pli->hvps);
      VioDestroyPS(pli->hvps);

      /* Wipe out the fields in our struct */

      pli->hwndFrame=0;
      pli->hwndClient=0;
      pli->hvps=0;
      pli->hdc=0;

      break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


void MainMenuView(HWND hwnd, HWND hwndList, LINEINFO *ali, USHORT usId)
{
  ULONG flFrameFlags=FCF_DLGBORDER | FCF_MINBUTTON | FCF_SHELLPOSITION |
                     FCF_SYSMENU | FCF_TASKLIST | FCF_TITLEBAR | FCF_ICON;

  char szWindowTxt[PATHLEN];
  RECTL rcl;
  LINEINFO *pli=ali+usId;

  NW(hwndList);
  NW(hwnd);

  if (pli->fOnline != 1)
  {
    WinAlarm(HWND_DESKTOP, WA_ERROR);
    return;
  }

  if (! pli->hwndFrame)
  {
    pli->hwndFrame=WinCreateStdWindow(HWND_DESKTOP, (ULONG)WS_ANIMATE,
                                      (PULONG)&flFrameFlags, (PSZ)szViewClass,
                                      (PSZ)NULL, (ULONG)0L, (HMODULE)0,
                                      (USHORT)ID_RESOURCE, &pli->hwndClient);

    /* Set the reserved memory location to equal the LINEINFO data */

    WinSetWindowPtr(pli->hwndClient, 0, pli);

    VioCreatePS(&pli->hvps, VIO_HEIGHT, VIO_WIDTH, 0, 0x0001 /*FORMAT_CGA*/, 0);
    pli->hdc=WinOpenWindowDC(pli->hwndClient);
    VioAssociate(pli->hdc, pli->hvps);

    /* Clear the VIO screen */

    AvioCls(pli->hvps, 0x07);

    /* Try to get an 8x8 font */

    VioSetDeviceCellSize(8, 8, pli->hvps);

    /* Find out the font that was actually chosen */

    VioGetDeviceCellSize((PSHORT)&pli->usCellHeight,
                         (PSHORT)&pli->usCellWidth,
                         pli->hvps);

    /* Now figure out how big the frame needs to be */

    rcl.xLeft=0;
    rcl.yBottom=0;
    rcl.xRight=VIO_PELWIDTH(pli);
    rcl.yTop=VIO_PELHEIGHT(pli);

    WinCalcFrameRect(pli->hwndFrame, &rcl, FALSE);

    /* Move our window so that it is the appropriate size */

    WinSetWindowPos(pli->hwndFrame, HWND_TOP,
                    0, 0,
                    rcl.xRight-rcl.xLeft, rcl.yTop-rcl.yBottom,
                    SWP_SIZE | SWP_SHOW | SWP_ZORDER);

    /* Now set our window's text to indicate the view */

    sprintf(szWindowTxt, "MSM - Node %d", usId+1);
    WinSetWindowText(pli->hwndFrame, szWindowTxt);

    /* Tell MCP to start sending monitoring information */

    McpSendMsg(hp, CLMSG_BEGINMONITOR, &pli->tid, 1);
  }
  else
  {
    /* Redraw the window */

    //WinInvalidateRect(pli->hwndClient, NULL, FALSE);

    /* Move this window to the top */

    WinSetWindowPos(pli->hwndFrame, HWND_TOP,
                    0, 0, 0, 0,
                    SWP_ZORDER | SWP_SHOW);
  }

  /* Make it the focus window */

  WinSetFocus(HWND_DESKTOP, pli->hwndClient);
}


