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
#define INCL_GPI
#define INCL_DOS

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include "pos2.h"
#include "max.h"
#include "mcp.h"
#include "smrc.h"
#include "ezfont.h"
#include "pmdebug.h"
#include "msg.h"
#include "sm.h"
#include "prmapi.h"

/* Class names for the various windows */

static char szTermTxt[]="Sent termination signal...";
static char szStartTxt[]="Sent start signal...";
static char szTitle[]="Maximus Session Monitor";
static char szClientClass[]="ClientWin";
static HAB hab;
static HWND hwndFrame;            /* handle for the main application window */



HPIPE hp;                         /* Handle for pipe */
HWND hwndClient;                  /* Main client window */
char szViewClass[]="ViewWin";     /* Class name for the viewer windows */
int iMaxLines;                    /* Max number of lines to support at one time */
int fPipeBusy;                    /* TRUE if waiting for pipe to open */



/* Create the listbox window */

static HWND near InitListbox(HWND hwnd, LINEINFO *li)
{
  HWND hwndList;
  int i;

  hwndList=WinCreateWindow(hwnd, WC_LISTBOX, "",
                           LS_NOADJUSTPOS | LS_OWNERDRAW,
                           0, 0,
                           0, 0,
                           hwnd, HWND_TOP, ID_LISTBOX, NULL, NULL);

  /* Insert enough blank items to cover all of our lines */

  for (i=0; i < iMaxLines; i++)
  {
    li[i].tid=i+1;

    /* Insert an item for each line that we expect to handle */

    WinSendMsg(hwndList, LM_INSERTITEM, MPFROMSHORT(i), MPFROMP("-foo-"));


    /* Set the item handle for each item appropriately */

    WinSendMsg(hwndList, LM_SETITEMHANDLE, MPFROMSHORT(i),
               MPFROMP(li+i));
  }

  return hwndList;
}



/* Update the listbox status with the new status in li[iNode] */

static void near UpdateListbox(HWND hwndList, int iNode)
{
  /* Set a dummy setitemtext message so that this item gets redrawn */

  WinSendMsg(hwndList, LM_SETITEMTEXT, MPFROMSHORT(iNode), MPFROMP("(blank)"));
}



/* Draw a listbox item */

static int near DrawItem(MPARAM mp2, FONTMETRICS *pfm, LINEINFO *liInfo)
{
  char text[120];
  POWNERITEM poi;
  POINTL ptl;

  /* Get information for this item */

  poi=PVOIDFROMMP(mp2);

  /* Start drawing text at the specified location */

  ptl.x=poi->rclItem.xLeft;
  ptl.y=poi->rclItem.yBottom + pfm->lMaxDescender;

  WinFillRect(poi->hps, &poi->rclItem,
              poi->fsState ? CLR_BLACK : SYSCLR_ENTRYFIELD);
              //GpiQueryBackColor(poi->hps));

  /* Draw the node number */

  ptl.x += pfm->lAveCharWidth;

  GpiSetColor(poi->hps, CLR_RED);
  sprintf(text, "%3d", poi->idItem+1);
  GpiCharStringAt(poi->hps, &ptl, strlen(text), text);
  GpiSetColor(poi->hps, poi->fsState ? CLR_WHITE : CLR_BLACK);
  ptl.x += pfm->lAveCharWidth * LB_COL1;

  /* Draw the status field */


  if (fPipeBusy || liInfo[poi->idItem].fOnline==0)
  {
    strcpy(text, "(off-line)");
    GpiCharStringAt(poi->hps, &ptl, strlen(text), text);
  }
  else
  {
    time_t *ptt;
    struct tm *ptm;

    strcpy(text, liInfo[poi->idItem].pcs->username);
    GpiCharStringAt(poi->hps, &ptl, strlen(text), text);
    ptl.x += pfm->lAveCharWidth * LB_COL2;

    strcpy(text, liInfo[poi->idItem].pcs->status);
    GpiCharStringAt(poi->hps, &ptl, strlen(text), text);
    ptl.x += pfm->lAveCharWidth * LB_COL3;

    ptt=(time_t *)&liInfo[poi->idItem].lPingTime;

    if (!*ptt)
      strcpy(text, "(none)");
    else
    {
      ptm=localtime(ptt);

      sprintf(text, "%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
    }

    GpiCharStringAt(poi->hps, &ptl, strlen(text), text);
  }

  poi->fsStateOld=poi->fsState=0;

  return TRUE;
}


/* Handle the WM_CREATE message */

static HWND near MainCreate(HWND hwnd, FONTMETRICS *pfm, LINEINFO *liInfo)
{
  HPS hps;

  /* Now figure out how big our status bar will be */

  hps=WinGetPS(hwnd);
  GpiQueryFontMetrics(hps, sizeof(FONTMETRICS), pfm);
  WinReleasePS(hps);

  return InitListbox(hwnd, liInfo);
}


/* Display the main application window */

static void near MainPaint(HWND hwnd, USHORT cxClient, USHORT cyClient, FONTMETRICS *pfm)
{
  HPS hps;
  POINTL ptl;
  RECTL rcl;

  hps=WinBeginPaint(hwnd, 0, NULL);

  rcl.xLeft=rcl.yBottom=0;
  rcl.xRight=cxClient;
  rcl.yTop=cyClient;

//  WinFillRect(hps, &rcl, SYSCLR_ENTRYFIELD);
  WinFillRect(hps, &rcl, SYSCLR_FIELDBACKGROUND);

  /* Move to the title area */

  ptl.x = pfm->lAveCharWidth + WinQuerySysValue(HWND_DESKTOP, SV_CXDLGFRAME);
  ptl.y = cyClient - pfm->lMaxAscender;
  GpiMove(hps, &ptl);

  /* Place the headings in the appropriate spot */

  GpiCharString(hps, 4, "Line");
  ptl.x += pfm->lAveCharWidth * LB_COL1;

  GpiCharStringAt(hps, &ptl, 8, "Username");
  ptl.x += pfm->lAveCharWidth * LB_COL2;

  GpiCharStringAt(hps, &ptl, 6, "Status");
  ptl.x += pfm->lAveCharWidth * LB_COL3;

  GpiCharStringAt(hps, &ptl, 4, "Ping");

  WinEndPaint(hps);
}


/* Set the status text (only) for a specified node */

static void near SetStatusText(LINEINFO *pli, char *txt)
{
  /* Allocate memory for the node status */

  if (!pli->pcs)
  {
    pli->pcs=malloc(sizeof *pli->pcs);
    memset(pli->pcs, 0, sizeof *pli->pcs);
  }

  /* Set the status of this node appropriately */

  if (pli->pcs)
    strcpy(pli->pcs->status, txt);
}



/* The user pressed button 2, so display the pop-up menu in an appropriate  *
 * place.                                                                   */

static void near MainButton2(HWND hwnd, HWND hwndList, HWND hwndMenu,
                             LINEINFO *liInfo, MPARAM mp1, FONTMETRICS *pfm)
{
  USHORT usTop=SHORT1FROMMR(WinSendMsg(hwndList, LM_QUERYTOPINDEX, 0, 0));
  USHORT usId;
  SWP swp;

  WinQueryWindowPos(hwndList, &swp);

  usId=usTop+(long)(swp.y + swp.cy - (long)SHORT2FROMMP(mp1)) / (long)(pfm->lMaxAscender + pfm->lMaxDescender);
  WinSendMsg(hwndList, LM_SELECTITEM, MPFROMSHORT(usId), MPFROMSHORT(1));

  /* Set the enabled/disabled status of the menu items appropriately: */

  /* start task */

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_START, TRUE),
             MPFROM2SHORT(MIA_DISABLED,
             liInfo[usId].fOnline==1 ? MIA_DISABLED : 0));

  /* end task */

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_STOP, TRUE),
             MPFROM2SHORT(MIA_DISABLED,
             liInfo[usId].fOnline==1 ? 0 : MIA_DISABLED));

  /* view */

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_VIEW, TRUE),
             MPFROM2SHORT(MIA_DISABLED,
             liInfo[usId].fOnline==1 ? 0 : MIA_DISABLED));

  /* log file */

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_LOG, TRUE),
             MPFROM2SHORT(MIA_DISABLED,
             liInfo[usId].fOnline==1 ? 0 : MIA_DISABLED));

  /* message */

  WinSendMsg(hwndMenu, MM_SETITEMATTR, MPFROM2SHORT(IDM_MESSAGE, TRUE),
             MPFROM2SHORT(MIA_DISABLED,
             liInfo[usId].fOnline==1 ? 0 : MIA_DISABLED));

  /* Now display the pop-up menu itself */

  WinPopupMenu(hwnd, hwnd, hwndMenu,
               SHORT1FROMMP(mp1), SHORT2FROMMP(mp1),
               liInfo[usId].fOnline==1 ? IDM_VIEW : IDM_START,
               PU_NONE | PU_MOUSEBUTTON1 | PU_MOUSEBUTTON2 |
                 PU_KEYBOARD | PU_HCONSTRAIN | PU_VCONSTRAIN |
                 PU_POSITIONONITEM);
}



/* Start the specified task */

void MainMenuStart(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  NW(hwnd);

  /* We can't start the node if it is already on-line! */

  if (pli[usId].fOnline==1)
  {
    WinAlarm(HWND_DESKTOP, WA_ERROR);
    return;
  }

  McpSendMsg(hp, CLMSG_START, &pli[usId].tid, 1);

  SetStatusText(pli+usId, szStartTxt);

  pli[usId].fOnline=-1;

  /* Redraw the client window */

  UpdateListbox(hwndList, usId);
}


/* Tell a specified Max task to stop running */

void MainMenuStop(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  char szPrompt[PATHLEN];

  /* Make sure that we are on-line */

  if (pli[usId].fOnline != 1)
  {
    WinAlarm(HWND_DESKTOP, WA_ERROR);
    return;
  }

  sprintf(szPrompt, "Warning!  Task %d will be stopped!", pli[usId].tid);

  if (WinMessageBox(HWND_DESKTOP, hwnd, szPrompt, "Kill Task",
                    0, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1 |
                       MB_APPLMODAL | MB_MOVEABLE)==MBID_OK)
  {
    McpSendMsg(hp, CLMSG_HAPPY_DAGGER, &pli[usId].tid, 1);
    SetStatusText(pli+usId, szTermTxt);
    UpdateListbox(hwndList, usId);
  }
}


void MainMenuLog(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  NW(hwnd); NW(hwndList); NW(pli); NW(usId);
}



/* Start all active nodes */

void MainMenuGStart(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  int i;

  NW(hwnd); NW(hwndList); NW(usId);

  for (i=0; i < iMaxLines; i++)
    if (pli[i].fOnline != 1)
    {
      McpSendMsg(hp, CLMSG_START, &pli[i].tid, 1);
      SetStatusText(pli+i, szStartTxt);
      pli[i].fOnline=-1;
      UpdateListbox(hwndList, i);
      DosSleep(1);
    }
}


/* Stop all active nodes */

void MainMenuGStop(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  int i;

  NW(hwnd); NW(hwndList); NW(usId);

  if (WinMessageBox(HWND_DESKTOP, hwnd, "Warning!  All tasks will be stopped!",
                    "Kill All Tasks",
                    0, MB_OKCANCEL | MB_ICONEXCLAMATION | MB_DEFBUTTON1 |
                       MB_APPLMODAL | MB_MOVEABLE)==MBID_OK)
  {
    for (i=0; i < iMaxLines; i++)
      if (pli[i].fOnline != 0)
      {
        McpSendMsg(hp, CLMSG_HAPPY_DAGGER, &pli[i].tid, 1);
        SetStatusText(pli+i, szTermTxt);
        UpdateListbox(hwndList, i);
        DosSleep(1);
      }
  }
}



/* Handle menu options from the main menu */

static void near MainMenu(HWND hwnd, USHORT usCmd, LINEINFO *liInfo, HWND hwndList)
{
  static struct
  {
    USHORT usCmd;
    void (*pfn)(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId);
  }
  cmdtable[]=
  {
    {IDM_START, MainMenuStart},
    {IDM_STOP, MainMenuStop},
    {IDM_VIEW, MainMenuView},
    {IDM_LOG, MainMenuLog},
    {IDM_MESSAGE, MainMenuMessage},
    {IDM_GLOBALSTART, MainMenuGStart},
    {IDM_GLOBALSTOP, MainMenuGStop},
    {IDM_GLOBALMESSAGE, MainMenuGMessage}
  };

  USHORT usId, i;

  /* Figure out which item is selected */

  usId=SHORT1FROMMP(WinSendMsg(hwndList, LM_QUERYSELECTION,
                               MPFROMSHORT(LIT_FIRST), 0));

  /* Make sure that the item number is valid */

  if (usId >= iMaxLines)
    return;

  /* Scan the command table for the function to call */

  for (i=0; i < sizeof(cmdtable)/sizeof(*cmdtable); i++)
    if (usCmd==cmdtable[i].usCmd)
    {
      (*cmdtable[i].pfn)(hwnd, hwndList, liInfo, usId);
      break;
    }
}


/* Main client window procedure */

static FONTMETRICS fm;

MRESULT EXPENTRY ClientProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2)
{
  static LINEINFO *liInfo;
  static USHORT cxClient, cyClient;
  static HWND hwndList;
  static HWND hwndMenu;
  int fOnlineOld, i;

  switch(msg)
  {
    case WM_CREATE:
      liInfo = malloc(iMaxLines * sizeof(LINEINFO));
      /* this malloc had better work, or else we're in trouble anyway */

      memset(liInfo, 0, iMaxLines * sizeof(LINEINFO));
      hwndList=MainCreate(hwnd, &fm, liInfo);
      hwndMenu=WinLoadMenu(HWND_OBJECT, 0, ID_POPUP);
      liInfo->hwndMain = hwnd;
      break;

    case WM_SIZE:
      /* Store new size of window */

      cxClient=SHORT1FROMMP(mp2);
      cyClient=SHORT2FROMMP(mp2);

      /* Resize the child list window */

      if (hwndList)
        WinSetWindowPos(hwndList, (HWND)0,
                        0, 0,
                        cxClient, cyClient - fm.lMaxAscender - fm.lMaxDescender,
                        SWP_SIZE | SWP_MOVE);

      WinShowWindow(hwndList, TRUE);
      break;

    case WM_PAINT:  /* Draw the status text */
      MainPaint(hwnd, cxClient, cyClient, &fm);
      break;

    case WM_DESTROY:
    {
      SWP swp;
      FILE *fp;

      /* Record our window position */

      WinQueryWindowPos(hwndFrame, &swp);

      if ((fp=fopen("sm.ini", "w")) != NULL)
      {
        fprintf(fp, "%ld %ld %ld %ld\n",
                swp.x, swp.y, swp.cx, swp.cy);
        fclose(fp);
      }

      WinDestroyWindow(hwndList);
      free(liInfo);
      break;
    }

    case WM_COMMAND:
      if (SHORT1FROMMP(mp2)==CMDSRC_MENU)
        MainMenu(hwnd, SHORT1FROMMP(mp1), liInfo, hwndList);
      break;

    case WM_BUTTON2DOWN:
      /* Display the pop-up menu */

      MainButton2(hwnd, hwndList, hwndMenu, liInfo, mp1, &fm);
      return 0;

    case WM_DRAWITEM:
      if (DrawItem(mp2, &fm, liInfo))
        return MPFROMSHORT(TRUE);
      break;

    /* Each item in the listbox is the same height */

    case WM_MEASUREITEM:
      return MRFROMSHORT(fm.lMaxAscender + fm.lMaxDescender);


    case SM_PIPEWAIT:
      fPipeBusy = LONGFROMMP(mp1);
      break;

    /* Set the status of a particular node */

    case SM_SETSTATUS:
      i=(int)(byte)SHORT1FROMMP(mp1);

      /* Make sure that the line number is okay */

      if (i==0 || i > iMaxLines)
        break;

      /* Make the node number zero-based */

      i--;

      /* Free any existing status text */

      if (liInfo[i].pcs)
        free(liInfo[i].pcs);

      liInfo[i].pcs=PVOIDFROMMP(mp2);

      /* Set the on-line flag for this node */

      fOnlineOld=liInfo[i].fOnline;

      liInfo[i].fOnline=!!*liInfo[i].pcs->username;

      /* If a session with an active snoop has ended, kill the              *
       * snoop window.                                                      */

      if (fOnlineOld && !liInfo[i].fOnline)
        if (liInfo[i].hwndClient)
          WinDestroyWindow(liInfo[i].hwndFrame);

      /* Redraw the client window */

      UpdateListbox(hwndList, i);
      break;


    /* Received a ping from a particular node */

    case SM_PING:
      i=(int)(byte)SHORT1FROMMP(mp1);

      if (i==0 || i > iMaxLines)
        break;

      i--;

      liInfo[i].lPingTime=LONGFROMMP(mp2);
      UpdateListbox(hwndList, i);
      break;

    case SM_VIO:          /* Video stuff - to be passed to AVIO window */
    case SM_VIODUMP:

      /* Validate the task number */

      i=(int)(byte)SHORT1FROMMP(mp1);

      if (i==0 || i > iMaxLines)
        break;

      i--;

      /* Pass this message off to the AVIO child window */

      if (liInfo[i].hwndClient)
        WinSendMsg(liInfo[i].hwndClient, msg, mp1, mp2);
      break;
  }

  return WinDefWindowProc(hwnd, msg, mp1, mp2);
}


/* Add the title for the session monitor window */

static void near AddTitle(HWND hwndFrame)
{
  SWCNTRL swctl;

  WinSetWindowText(hwndFrame, szTitle);

  memset(&swctl, '\0', sizeof swctl);

  swctl.hwnd=hwndFrame;               /* our window handle */
  swctl.uchVisibility=SWL_VISIBLE;    /* should be visible on task list */
  swctl.fbJump=SWL_JUMPABLE;          /* user can select us on task list */

  strcpy(swctl.szSwtitle, szTitle);   /* copy in the application title */

  WinQueryWindowProcess(hwndFrame, &swctl.idProcess, NULL);
  WinAddSwitchEntry(&swctl);
  WinShowWindow(hwndFrame, TRUE);
}


/* Create the main application window */

static void near CreateStdWindow(void)
{
  /* Frame flags for the application - use a title bar, system menu,        *
   * sizaable window border, min/max icon, menu bar, an application         *
   * icon, and an accelerator (keystroke) table.                            */

  static ULONG flFrameFlags=FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER |
                            FCF_MINMAX | FCF_SHELLPOSITION | /*FCF_MENU |*/
                            FCF_ICON;
  int fSetPosn = FALSE;
  FILE *fp;
  SWP swp;

  /* Try to restore our window position */

  if ((fp=fopen("sm.ini", "r")) != NULL)
  {
    char temp[64];

    if (fgets(temp, sizeof temp, fp))
    {
      if (sscanf(temp, "%ld %ld %ld %ld",
                 &swp.x, &swp.y, &swp.cx, &swp.cy)==4)
      {
        fSetPosn = TRUE;
      }
    }

    fclose(fp);
  }


  /* Register the main window class */

  WinRegisterClass(hab, szClientClass, (PFNWP)ClientProc,
                   (ULONG)CS_SIZEREDRAW, 0);


  /* Register the viewer window class */

  WinRegisterClass(hab, szViewClass, (PFNWP)ViewProc,
                   (ULONG)CS_SIZEREDRAW, sizeof(LINEINFO *));


  /* Create an instance of the main window */

  hwndFrame=WinCreateStdWindow((HWND)HWND_DESKTOP, (ULONG)0,
                               (PULONG)&flFrameFlags, (PSZ)szClientClass,
                               (PSZ)NULL, (ULONG)0L, (HMODULE)0,
                               (USHORT)ID_RESOURCE, &hwndClient);

  /* Set the width of this window appropriately */

  if (!fSetPosn)
    WinQueryWindowPos(hwndFrame, &swp);

  swp.hwnd=hwndFrame;
  swp.hwndInsertBehind = HWND_TOP;

  if (!fSetPosn)
  {
    /* Fix the window size so that it is just big enough to hold all of our   *
     * information.                                                           */

    swp.cx=LB_SUM * fm.lAveCharWidth +
           WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL) +
           WinQuerySysValue(HWND_DESKTOP, SV_CXBORDER)*2 + 4;

    swp.cy=(iMaxLines+1) * (fm.lMaxAscender+fm.lMaxDescender) +
           WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR) +
           WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER)*2 +
           WinQuerySysValue(HWND_DESKTOP, SV_CYMENU) + 16;
  }

  #ifdef __FLAT__
    swp.fl=SWP_SIZE | SWP_SHOW | SWP_MOVE | SWP_ZORDER;
  #else
    swp.fs=SWP_SIZE | SWP_SHOW | SWP_MOVE | SWP_ZORDER;
  #endif

  WinSetMultWindowPos(hab, &swp, 1);
}



char *GetMax(int argc, char **argv)
{
  char *pszMaximus = NULL;

  (void)argc;

  /* First search the command-line for an explicit '-p' override */

  if (argv)
  {
    while (*argv)
    {
      /* Ignore everything except -p options */

      if (**argv=='-' && (*argv)[1]=='p')
      {
        pszMaximus = *argv + 2;
        break;
      }

      argv++;
    }
  }

  /* If we couldn't get that, look for the environment variable */

  if (!pszMaximus && (pszMaximus = getenv("MAXIMUS"))==NULL)
  {
    WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
           "Error!  The 'MAXIMUS' environment variable could not be found. "
           "This variable must point to your main Maximus .PRM file.\n"
           "For example, the following can be added to your CONFIG.SYS:\n\n"
           "SET MAXIMUS=C:\\MAX\\MAX.PRM\n",
           "", 0,
           MB_OK | MB_ICONEXCLAMATION);
  }

  return pszMaximus;
}


static int ReadPrm(char *pszMax)
{
  HPRM hp;
  char szMsg[PATHLEN*3];

  if ((hp=PrmFileOpen(pszMax, FALSE)) != NULL)
  {
    iMaxLines = PrmFileValue(hp, mcp_sessions);

    if (iMaxLines > 255)
      iMaxLines = 255;
    else if (iMaxLines < 1)
      iMaxLines = 1;


    PrmFileClose(hp);
    return TRUE;
  }

  sprintf(szMsg,
          "Error!  The MAXIMUS parameter file, "
          "'%s', is invalid.  Please fix this file "
          "or set the MAXIMUS environment variable "
          "to point to a valid .PRM file.",
          pszMax);

  WinMessageBox(HWND_DESKTOP, HWND_DESKTOP,
         szMsg,
         "", 0,
         MB_OK | MB_ICONEXCLAMATION);

  return FALSE;
}



/* Main program */

int main(int argc, char *argv[])
{
  HMQ hmq;          /* message queue for PM messages */
  QMSG qmsg;        /* message retrieved from queue */
  char *pszMax;


  hab=WinInitialize(0);
  hmq=WinCreateMsgQueue(hab, 0);

  if ((pszMax = GetMax(argc, argv)) != NULL)
  {
    if (ReadPrm(pszMax))
    {
      CreateStdWindow();

      /* Give us a session title */

      AddTitle(hwndFrame);

      /* Start monitoring a server */

      StartMonitor(argc >= 2 ? argv[1] : "\\pipe\\maximus\\mcp");

      /*DbgBegin(hwndClient, hab, 100, TRUE);*/
      DbgPrintfp("*** Begin log!");

      /* Now retrieve and dispatch messages from the message queue */

      while (WinGetMsg(hab, &qmsg, (HWND)0, 0, 0))
        WinDispatchMsg(hab, &qmsg);

      DbgEnd();
      WinDestroyWindow(hwndFrame);
    }
  }

  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);

  return 0;
}


