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

#ifndef __WIN_H_DEFINED
  #include "win.h"
#endif

void ZmStatInit(void);
void ZmStatHdr(int fSend, int iHdr, char *szHdr, unsigned long ulHdr);
void ZmStatFile(char *szPath, unsigned long ulSize, unsigned fCrc32);
void ZmStatData(unsigned long ulPos, unsigned uiBlockLen, unsigned fCrc32);
unsigned XferWinOpen(sword protocol, unsigned doing_dl);
void XferWinClose(void);
void XferWinClear(void);
void XferTxtNewFile(byte *filename, long size, word protocol, unsigned doing_dl);
void XferWinNewFile(char *filename, long size);
void ZmStatus(unsigned crc32, unsigned block_size, long size, long sent, time_t start, char *status);
int mdm_getct(unsigned timeout);
void XmSendCAN(void);
void XmStatus(unsigned flag, unsigned do_crc, long sent,
              time_t start, word protocol, long size, long last_ack,
              unsigned n_err, long block);
void unique_name (char *fname);
void ThruStart(void);
void ThruSetStartOffset(long lStartOfs);
void ThruLog(long lTotalBytes);

/*#define DEBUGZ*/

#if defined(DEBUGX) || defined(DEBUGZ)
  #define dlogit(x) logit x
#else
  #define dlogit(x)
#endif


