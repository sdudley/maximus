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

#include <stdlib.h>
#include <string.h>
#include "pos2.h"
#include "max.h"
#include "mcp.h"
#include "smrc.h"
#include "msg.h"
#include "sm.h"


#define MAX_MAXMSG_LEN  (PATHLEN-sizeof(struct _cdat)-2)

extern HPIPE hp;


/* Dialog procedure for the "send a message" dialog box */

MRESULT APIENTRY MsgDlgProc(HWND hwnd, OS2UINT msg, MPARAM mp1, MPARAM mp2)
{
  static char *pszOutput;

  switch (msg)
  {
    case WM_INITDLG:
      pszOutput=PVOIDFROMMP(mp2);

      /* Set the max text limit for this entryfield */

      WinSendDlgItemMsg(hwnd, IDD_MSG_ENTRY, EM_SETTEXTLIMIT,
                        MPFROMSHORT(MAX_MAXMSG_LEN), 0);
      break;

    case WM_COMMAND:
      /* Process pushbutton msgs */

      if (SHORT1FROMMP(mp2)==CMDSRC_PUSHBUTTON)
      {
        /* If we got a pushbutton, terminate with the right code */

        switch (SHORT1FROMMP(mp1))
        {
          case IDD_MSG_SEND:
            /* Copy the entered text into the buffer */

            WinQueryWindowText(WinWindowFromID(hwnd, IDD_MSG_ENTRY),
                               MAX_MAXMSG_LEN, pszOutput);

            WinDismissDlg(hwnd, DID_OK);
            break;

          case DID_CANCEL:
            WinDismissDlg(hwnd, DID_CANCEL);
            break;
        }
      }
      return 0;
  }

  return WinDefDlgProc(hwnd, msg, mp1, mp2);
}


/* Send the message in szMsg to task 'tid' */

static int near MessageSendOne(byte tid, char *szMsg)
{
  struct _cdat *pcd;
  OS2UINT rc;

  if ((pcd=malloc(sizeof(*pcd) + strlen(szMsg) + 1))==NULL)
    return FALSE;

  /* Fill out the chat message header */

  pcd->tid=0;
  pcd->dest_tid=tid;
  pcd->type=CMSG_HEY_DUDE;
  pcd->len=strlen(szMsg)+1;

  /* Copy in the message after it */

  strcpy((char *)(pcd+1), szMsg);

  /* Tell the MCP to display this message */

  rc=McpSendMsg(hp, CLMSG_MAX_SEND_MSG, (void *)pcd,
                sizeof(*pcd)+strlen(szMsg)+1);
  free(pcd);

  return rc==0;
}


/* Menu option: send a message to an on-line node */

void MainMenuMessage(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  char szMsg[PATHLEN];

  NW(pli); NW(usId);

  /* Display the "send a message" dialog box */

  if (WinDlgBox(hwndList, hwnd, MsgDlgProc, 0, IDD_MSG, (PVOID)szMsg)==DID_OK)
  {
    if (MessageSendOne(pli[usId].tid, szMsg))
    {
      /* Tell the user that everything went okay */

      sprintf(szMsg, "Message sent to node %d", pli[usId].tid);

      WinMessageBox(HWND_DESKTOP, hwnd, szMsg, "", 0,
                    MB_OK | MB_ICONASTERISK);
    }
  }
}

void MainMenuGMessage(HWND hwnd, HWND hwndList, LINEINFO *pli, USHORT usId)
{
  extern int iMaxLines;
  char szMsg[PATHLEN];
  int i;

  NW(usId);

  /* Display the "send a message" dialog box */

  if (WinDlgBox(hwndList, hwnd, MsgDlgProc, 0, IDD_MSG, (PVOID)szMsg)==DID_OK)
  {
    for (i=0; i < iMaxLines; i++)
      if (pli[i].fOnline)
        MessageSendOne(i+1, szMsg);

    /* Tell the user that everything went okay */

    sprintf(szMsg, "Message sent to all nodes");

    WinMessageBox(HWND_DESKTOP, hwnd, szMsg, "", 0,
                  MB_OK | MB_ICONASTERISK);
  }
}


