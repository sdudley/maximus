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

#include "mcp.h"

#define VERSION               "1.00"

#define THREAD_STK_SIZE       4096    /* 4K thread stack size */
#define MAX_MCP_MONITORS      16      /* Allow up to 16 monitors on 1 session */

/* Information about all of the MCP/Maximus pipes */

typedef struct _pinfo
{
  USHORT session;                 /* This session number */
  HPIPE hp;                       /* Pipe handle */
  USHORT cbBufLen;                /* Size of buffer */
  byte *pbBuf;                    /* Buffer used for storing messages */

  USHORT cbMsg;                   /* Size of current message */
} PINFO;


/* Information about all of the client pipes */

typedef struct _cinfo
{
  HPIPE hp;                       /* Pipe handle */
  USHORT cbBufLen;                /* Size of buffer */
  byte *pbBuf;                    /* Buffer used for storing messages */

  USHORT cbMsg;                   /* Size of current message */
} CINFO;



/* Session info record - used for every Maximus session thread */

struct _ses_info
{
  BYTE tid;             /* Thread identifier */
  struct _cstat cs;     /* Chat status of this user */
  PINFO *ppi;           /* Pipe info for this session */
  HPIPE ahpMonitors[MAX_MCP_MONITORS];  /* List of monitoring pipes */
};

int CALLBACK MSetStatus(PINFO *ppi);
int CALLBACK MHello(PINFO *ppi);
int CALLBACK MEOT(PINFO *ppi);
int CALLBACK MMaxSendMsg(PINFO *ppi);
int CALLBACK MQueryTaskInfo(PINFO *ppi);
int CALLBACK MQueryTaskList(PINFO *ppi);
int CALLBACK MVio(PINFO *ppi);
int CALLBACK MVioDump(PINFO *ppi);
int CALLBACK MPing(PINFO *ppi);
int CALLBACK MLog(PINFO *ppi);

void MCPLogOpen(char *fname);
void MCPLog(char *fmt, ...);
void MCPLogClose(void);

void ClientIO(HPIPE hp);
void OverviewSendStatus(byte tid, struct _cstat *pcs);
void OverviewSendAllStatus(void);
void OverviewSendPing(byte tid);
void OverviewEnd(byte tid);

