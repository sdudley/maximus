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

#define DEBUG

#include <io.h>
#include <fcntl.h>

#ifdef PM

#include <string.h>
#include <stdlib.h>
#include "tui.h"

#ifdef DEBUG
#include "pmdebug.h"
#endif

HAB hab;
HWND hwndFrame, hwndClient;
HMQ hmq;
static char szClientClass[]="ClientClass";
static MRESULT EXPENTRY ClientProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2);
static MRESULT EXPENTRY DlgProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2);

static HVMENU ghvm;  /* menu currently being processed */

/* Initialize the PM subsystem */

void TuiPmInit(void)
{
  SWP swp;
  LONG cx, cy;
  static ULONG flFrameFlags=FCF_SIZEBORDER | FCF_TITLEBAR | FCF_SYSMENU |
                            FCF_MINMAX | FCF_TASKLIST;

  hab=WinInitialize(0);
  hmq=WinCreateMsgQueue(hab, 0);

  WinRegisterClass(hab, szClientClass, (PFNWP)ClientProc,
                   (ULONG)CS_SIZEREDRAW, 0);

  hwndFrame=WinCreateStdWindow(HWND_DESKTOP, (ULONG)0,
                               (PULONG)&flFrameFlags, (PSZ)szClientClass,
                               (PSZ)NULL, (ULONG)0L, (HMODULE)0,
                               (USHORT)ID_RESOURCE, &hwndClient);

#ifdef DEBUG
  DbgBegin(HWND_DESKTOP, hab, 200);
#endif

  /* Get size of entire screen (in pels) */

  swp.cx=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
  swp.cy=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

  cx=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);
  cy=(USHORT)WinQuerySysValue(HWND_DESKTOP, SV_CXSIZEBORDER);


  WinSetWindowPos(hwndFrame, (HWND)0,
                  0, 0,
                  0, 0,
                  SWP_MAXIMIZE);

  WinSetWindowPos(hwndFrame, (HWND)0,
                  (SHORT)-cx,                /* x position */
                  (SHORT)(swp.cy*1/10+cy),   /* y position */
                  (SHORT)(swp.cx+cx+cx),     /* x size */
                  (SHORT)(swp.cy*9/10),      /* y size */
                  (USHORT)(SWP_MOVE | SWP_SIZE | SWP_ACTIVATE));


  WinShowWindow(hwndFrame, TRUE);
}


/* Terminate the PM subsystem */

void TuiPmTerm(void)
{
#ifdef DEBUG
  DbgEnd();
#endif

  if (hwndFrame)
  {
    WinDestroyWindow(hwndFrame);
    hwndFrame=0;
  }

  if (hmq)
  {
    WinDestroyMsgQueue(hmq);
    hmq=0;
  }

  if (hab)
  {
    WinTerminate(hab);
    hab=0;
  }
}



static char * near _TuiPmConstructMenu(HVMENU vmenu, char *template, USHORT id, USHORT usMul)
{
  MENUTEMPLATE *pmt;
  MTI *pmti;
  BOOL fSubMenu;
  char *p;
  USHORT i;

  pmt=(MENUTEMPLATE *)template;

  pmt->cb=sizeof(MENUTEMPLATE);
  pmt->version=0;
  pmt->codepage=437;
  pmt->iInputSize=0;
  pmt->cMti=0;

  pmti=(MTI *)(pmt+1);
  p=(char *)pmti;

  for (i=0; i < vmenu->num_opt; i++)
  {
    fSubMenu=(vmenu->opt[i].menu &&
              (vmenu->opt[i].menu->type & MENU_DIALOG)==0);

    pmti->afStyle=MIS_TEXT | (fSubMenu ? MIS_SUBMENU : 0);
    pmti->afAttribute=0 /*MIA_CHECKED*/;
    pmti->idItem=id;

    id += usMul;

    p=(char *)(pmti+1);
    strcpy(p, vmenu->opt[i].name);
    p += strlen(p)+1;

    if (fSubMenu)
      p=_TuiPmConstructMenu(vmenu->opt[i].menu, p, id, usMul/PM_BLOCK_DIV);

    pmti=(MTI *)p;

    pmt->cMti++;
  }

  return p;
}


int near TuiPmClientSelect(USHORT id);

int TuiPmEvent(HVMENU vmenu)
{
  HWND hwndMenu;
  QMSG qmsg;
  char *template, *p;


  template=malloc(20000);

  ghvm=vmenu;

  p=_TuiPmConstructMenu(vmenu, template, PM_BLOCK_START, PM_BLOCK_SIZE);

  hwndMenu=WinCreateMenu(hwndFrame, template);

  WinSendMsg(hwndFrame, WM_UPDATEFRAME, MPFROMSHORT(FCF_MENU), 0);

  free(template);

  while (WinGetMsg(hab, &qmsg, (HWND)0, 0, 0))
    WinDispatchMsg(hab, &qmsg);

  return 0;
}

/* Create a dialog box based on the TUI menu/dialog */

static int near TuiPmCreateDlg(HVMENU hd)
{
  HWND hwndDlg;
  char *template, *txt;
  DLGTEMPLATE *pdt;
  DLGTITEM *pdi;
  USHORT cxDlg, cyDlg;
  USHORT cDlgt;
  SHORT i;

  template=malloc(20000);
  pdt=(DLGTEMPLATE *)template;

  /* Fill in the thing which describes this template */

#define DLT_SIZE 14
  pdt->type=0;
  pdt->codepage=437;
  pdt->offadlgti=DLT_SIZE;
  pdt->fsTemplateStatus=1;
  pdt->iItemFocus=-1;
  pdt->coffPresParams=0;

/*  DbgPrintf("pdt->type=%d", pdt->type);
  DbgPrintf("pdt->codepage=%d", pdt->codepage);
  DbgPrintf("pdt->offadlgti=%d", pdt->offadlgti);
  DbgPrintf("pdt->fsTemplateStatus=%d", pdt->fsTemplateStatus);
  DbgPrintf("pdt->iItemFocus=%d", pdt->iItemFocus);
  DbgPrintf("pdt->coffPresParams=%d", pdt->coffPresParams);*/

  /* Count the number of dialog template items.  One item for               *
   * each, except we need two items to map one entryfield.                  */

  for (i=0, cDlgt=0; i < (SHORT)hd->num_opt; i++, cDlgt++)
    if (hd->opt[i].regist==DlgStrReg)
      cDlgt++;


  pdi=(DLGTITEM *)((char *)pdt+DLT_SIZE);

  txt=(char *)pdi + (cDlgt+1)*sizeof(DLGTITEM);

  pdi->fsItemStatus=0;
  pdi->cChildren=cDlgt;
  pdi->cchClassName=0;
  pdi->offClassName=1;
  pdi->cchText=0;
  pdi->offText=(char *)txt-(char *)pdt;
  *txt++='\0';
  pdi->flStyle=/*WS_VISIBLE |*/ WS_CLIPSIBLINGS | WS_SAVEBITS | FS_NOBYTEALIGN |
               FS_DLGBORDER;
  cxDlg=pdi->cx=hd->sizex * 4;
  cyDlg=pdi->cy=(hd->sizey-3) * 8;

  pdi->x=0;
  pdi->y=0;

  pdi->id=1;
  pdi->offPresParams=-1;
  pdi->offCtlData=(char *)txt-(char *)pdt;
  *(ULONG *)txt=0;
  txt += sizeof(ULONG);

/*    DbgPrintf("** first");
    DbgPrintf("pdi->fsItemStatus=%d", pdi->fsItemStatus);
    DbgPrintf("pdi->cChildren=%d", pdi->cChildren);
    DbgPrintf("pdi->cchClassName=%d", pdi->cchClassName);
    DbgPrintf("pdi->offClassName=%d", pdi->offClassName);
    DbgPrintf("pdi->cchText=%d", pdi->cchText);
    DbgPrintf("pdi->offText=%d", pdi->offText);
    DbgPrintf("pdi->flStyle=0x%lx", pdi->flStyle);
    DbgPrintf("pdi->x=%d", pdi->x);
    DbgPrintf("pdi->y=%d", pdi->y);
    DbgPrintf("pdi->cx=%d", pdi->cx);
    DbgPrintf("pdi->cy=%d", pdi->cy);
    DbgPrintf("pdi->id=%d", pdi->id);
    DbgPrintf("pdi->offPresParams=%d", pdi->offPresParams);
    DbgPrintf("pdi->offCtlData=%d", pdi->offCtlData);*/

  pdi++;

  for (i=0; i < (SHORT)hd->num_opt; i++, pdi++)
  {
    char *oname;

    pdi->fsItemStatus=0;
    pdi->cChildren=0;

    pdi->cchClassName=0;

    if ((oname=strchr(hd->opt[i].name, ';')) != NULL)
      oname++;
    else oname=hd->opt[i].name;

    pdi->cchText=oname ? strlen(oname) : 0;
    pdi->offText=(char *)txt-(char *)pdt;

    /* Remove the tilde if we're using an info field */

    if (hd->opt[i].regist==DlgStrReg)
    {
      char *p;

      if ((p=strchr(oname, '~')) != NULL)
        memmove(p, p+1, strlen(p+1)+1);
    }

    strcpy(txt, oname ? oname : "");
    txt += strlen(txt) + 1;

    pdi->cx= pdi->cchText * 9 / 2;

    if (hd->opt[i].regist==DlgButReg)
      pdi->cy=13;
    else pdi->cy=8;

    pdi->x = hd->opt[i].cx * 4;
    pdi->y = cyDlg - (hd->opt[i].cy * 17 / 2) - pdi->cy; /* use 1st quad instead of 4th quad */

    pdi->id=i+3;
    pdi->offPresParams=-1;
    pdi->offCtlData=-1;


    if (hd->opt[i].regist==DlgStrReg)
    {
      /* For an entryfield, create two items: one is the label the the      *
       * left, which describes this field, and the other is the             *
       * entryfield itself.                                                 */

      /* Text */

      pdi->offClassName=5;
      pdi->flStyle=WS_VISIBLE | SS_TEXT;

      /* Create a copy of this item */

      pdi[1]=*pdi;
      pdi++;

      /* Copy contains the entryfield */

      pdi->offClassName=6;
      pdi->flStyle=WS_VISIBLE | WS_TABSTOP | ES_LEFT | ES_AUTOSCROLL;

      /* Length of field is contained in the original option text */

      pdi->cx = atoi(hd->opt[i].name) * 9 / 2;

      /* Move field right by length of label */

      pdi->x += (strlen(oname)+2) * 9 / 2;

      /* Default to an empty field */

      pdi->cchText=0;
      pdi->offText = (char *)txt-(char *)pdt;
      *txt++='\0';

      /* Assign a new ID number */

      pdi->id += 1024;
    }
    else if (hd->opt[i].regist==DlgButReg)
    {
      pdi->offClassName=3;
      pdi->flStyle=WS_VISIBLE | WS_GROUP | WS_TABSTOP |
                   BS_PUSHBUTTON;
    }
    else if (hd->opt[i].regist==DlgRadReg)
    {
      pdi->offClassName=3;
      pdi->flStyle=WS_VISIBLE | WS_GROUP | WS_TABSTOP |
                   BS_AUTORADIOBUTTON;
    }
    else if (hd->opt[i].regist==DlgChkReg)
    {
      pdi->offClassName=3;
      pdi->flStyle=WS_VISIBLE | WS_GROUP | WS_TABSTOP |
                   BS_AUTOCHECKBOX;
    }
    else /*if (hd->opt[i].regist==DlgInfReg)*/
    {
      pdi->offClassName=5;
      pdi->flStyle=WS_VISIBLE | SS_TEXT;
    }


/*    DbgPrintf("** next");
    DbgPrintf("pdi->fsItemStatus=%d", pdi->fsItemStatus);
    DbgPrintf("pdi->cChildren=%d", pdi->cChildren);
    DbgPrintf("pdi->cchClassName=%d", pdi->cchClassName);
    DbgPrintf("pdi->offClassName=%d", pdi->offClassName);
    DbgPrintf("pdi->cchText=%d", pdi->cchText);
    DbgPrintf("pdi->offText=%d", pdi->offText);
    DbgPrintf("pdi->flStyle=0x%lx", pdi->flStyle);
    DbgPrintf("pdi->x=%d", pdi->x);
    DbgPrintf("pdi->y=%d", pdi->y);
    DbgPrintf("pdi->cx=%d", pdi->cx);
    DbgPrintf("pdi->cy=%d", pdi->cy);
    DbgPrintf("pdi->id=%d", pdi->id);
    DbgPrintf("pdi->offPresParams=%d", pdi->offPresParams);
    DbgPrintf("pdi->offCtlData=%d", pdi->offCtlData);*/
  }

  pdt->cbTemplate=(char *)txt-(char *)pdt;

  hwndDlg=WinCreateDlg(HWND_DESKTOP, hwndClient, DlgProc, pdt,
                       NULL);
  WinProcessDlg(hwndDlg);
  WinDestroyWindow(hwndDlg);

  free(template);

  return 0;
}


static int near TuiPmClientSelect(USHORT id)
{
  USHORT ix1, ix2, ix3;
  HVMENU hd;

  /*DbgPrintf("id=%d", id);*/

  ix1=id / PM_BLOCK_SIZE - 1;
  ix2=(id % PM_BLOCK_SIZE) / PM_BLOCK_DIV;
  ix3=(id % PM_BLOCK_SIZE) % PM_BLOCK_DIV;

  /*DbgPrintf("ix1=%d, 2=%d, 3=%d", ix1, ix2, ix3);*/

  /* Find the current menu option.  Handle up to three levels of            *
   * indirection.                                                           */

  if (ghvm->opt[ix1].menu->type & MENU_DIALOG)
    hd=ghvm->opt[ix1].menu;
  else if (ghvm->opt[ix1].menu->opt[ix2].menu->type & MENU_DIALOG)
    hd=ghvm->opt[ix1].menu->opt[ix2].menu;
  else hd=ghvm->opt[ix1].menu->opt[ix2].menu->opt[ix3].menu;

  return TuiPmCreateDlg(hd);
}


static MRESULT EXPENTRY ClientProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2)
{
  static USHORT cxClient, cyClient; /* Size of client window */
  HPS hps;

  switch (msg)
  {
    case WM_SIZE:
      cxClient=SHORT1FROMMP(mp2);
      cyClient=SHORT2FROMMP(mp2);
      break;

    case WM_PAINT:
      hps=WinBeginPaint(hwnd, (HPS)0, NULL);
      GpiErase(hps);
      WinEndPaint(hps);
      break;

    case WM_COMMAND:
      TuiPmClientSelect(SHORT1FROMMP(mp1));
      break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


static MRESULT EXPENTRY DlgProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2)
{
  POINTL p1;
  RECTL rcl;
  HPS hps;

  switch (msg)
  {
    case WM_INITDLG:
      p1.x=WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
      p1.y=WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);

      WinQueryWindowRect(hwnd, &rcl);
      WinSetWindowPos(hwnd, (HWND)0,
                      (p1.x - (rcl.xRight-rcl.xLeft)) / 2,
                      (p1.y - (rcl.yTop-rcl.yBottom)) / 2,
                      0, 0,
                      SWP_MOVE);

      WinShowWindow(WinQWindow(hwnd, QW_PARENT), TRUE);
      break;

    case WM_COMMAND:
      switch (SHORT1FROMMP(mp1))
      {
        case DID_OK:
        case DID_CANCEL:
          WinDismissDlg(hwnd, COMMANDMSG(&msg)->cmd==DID_OK);
          return 0;
      }
      break;
  }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


#endif

