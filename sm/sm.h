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

/* Window messages sent to the main client window */

#define SM_SETSTATUS  WM_USER
#define SM_PING       (WM_USER+1)
#define SM_VIO        (WM_USER+2)
#define SM_VIODUMP    (WM_USER+3)
#define SM_PIPEWAIT   (WM_USER+4)


#define THREAD_STK_SIZE     16384


#ifdef far
#define far
#endif


/* Identifier for the main window listbox */

#define ID_LISTBOX      512


typedef struct
{
  char curattr;
  int last_cc;
  char str2[25];
  char state;
  char newattr;

  word s2, s3;

  byte uch;
  byte save_cx;

  word x, y, z, a;
} AVP;

/* Structure used to hold information about each line */

typedef struct
{
  byte tid;                       /* Task ID for this node */
  OS2UINT fOnline;                /* Is this node on-line? */
  struct _cstat *pcs;             /* Status of this node */
  long lPingTime;                 /* Time of the last ping */
  HWND hwndMain;                  /* Main client window */

  /* VIEW WINDOW: */

  HWND hwndFrame;                 /* Frame window handle */
  HWND hwndClient;                /* Client window handle */
  HVPS hvps;                      /* VIO pres space for View window */
  HDC hdc;                        /* Window device context for View window */

  USHORT usCellWidth;
  USHORT usCellHeight;

  USHORT usCurRow, usCurCol;      /* Current cursor position */
  AVP avp;                        /* AVIO output parameters */


} LINEINFO;


typedef struct
{
  USHORT cbGot;
  byte *pb;
} SMDISPATCH;


/* Width (in terms on # of avg chars) of each column in the listbox */

#define LB_COL1     8
#define LB_COL2     20
#define LB_COL3     32
#define LB_COL4     8

#define LB_SUM (LB_COL1+LB_COL2+LB_COL3+LB_COL4+3)


/* Parameters for the video i/o functions */

#define VIO_WIDTH   80
#define VIO_HEIGHT  25

#define VIO_PELWIDTH(ptr)     (VIO_WIDTH * (ptr)->usCellWidth)
#define VIO_PELHEIGHT(ptr)    (VIO_HEIGHT * (ptr)->usCellHeight)


MRESULT EXPENTRY ViewProc(HWND hwnd, MSGDEF msg, MPARAM mp1, MPARAM mp2);
void MainMenuView(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId);
TID StartMonitor(char *szPipeRoot);
void MainMenuMessage(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId);
void MainMenuGMessage(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId);

