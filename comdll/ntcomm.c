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

/*
   NTComm - Multithreaded asynchronous communications routines for Windows NT.

   Copyright 1993, 1994 by Lanius Corporation.  All rights reserved.
*/

#pragma off(unreferenced)
static char rcs_id[]="$Id: ntcomm.c,v 1.1.1.1 2002/10/01 17:49:34 sdudley Exp $";
#pragma on(unreferenced)

#define NO_RUNTIME /* also copy this include to ntcommdl.c! */
/*#define DEBUG_PIPE*/
/*#define DEBUG_TX*/
/*#define DEBUG_MN*/
/*#define DEBUG_FILE*/

#ifdef NT

#define YESWINERROR

#ifndef NO_RUNTIME
#include <stdio.h>
#include <stdlib.h>
#endif

#include "ntcomm.h"

#ifdef DEBUG_PIPE
#include "pdclient.h"
#endif

static BOOL volatile fTxPurged, fRxPurged;

#ifdef DEBUG_FILE
static FILE *fp_out;
#endif

void * memmove(unsigned char *to, unsigned char *from, unsigned int cnt)
{
  void *ofrom=from;

  while (cnt--)
    *to++=*from++;

  return ofrom;
}

void * memset(void *buffer, int ch, size_t size)
{
  char *pszDest;

  for (pszDest=buffer; size--; )
    *pszDest++ = ch;

  return buffer;
}

#ifdef NO_RUNTIME
void printf(char *fmt, ...)
{
  (void)fmt;
}

static int malloc_inited=0;
static HANDLE hh;

int malloc_init(void)
{
  if ((hh=HeapCreate(0, 4096, 32768))==NULL)
    return 0;

  return 1;
}

void *malloc(int size)
{
  char *p;

  if (!malloc_inited)
  {
    if (!malloc_init())
      return NULL;

    malloc_inited=1;
  }

  if ((p=HeapAlloc(hh, 0, size))==NULL)
    return NULL;

  return p;
}


int free(void *p)
{
  return HeapFree(hh, 0, p);
}
#endif /* NO_RUNTIME */

//HJK 98/3/23 - Don't need this function anymore, seperate threads do their
//own cleanup now:
//int cdie(HCOMM hc)
//{
//   --hc->cThreads;
   //printf("Thread dying!  Count is now %d\n", hc->cThreads);
//   return 0;
//}


/* Transmit-character thread */

DWORD __stdcall TxThread(LPVOID arg)
{
  HCOMM hc;                     /* Local thread information */
  DWORD dwBytesWritten;         /* Number of bytes written in this pass */
  OVERLAPPED ol;                /* Structure used for overlapped I/O info */

  hc=(HCOMM)arg;                /* Make local copy of thread info */
  ++hc->cThreads;

  /* Create a do-nothing semaphore used by NT to indicating I/O completion */

  ol.hEvent=hc->hevTxDone;
  ol.Offset=ol.OffsetHigh=0L;

#ifdef DEBUG_FILE
  if ((fp_out=fopen("\\ntcomm.out", "ab"))==NULL)
  {
    printf("Can't open \\ntcomm.out!\n");
    exit(1);
  }
#endif

  for (;;)
  {
    /* Wait until we have something to transmit */

#ifndef NO_RUNTIME
//    printf("1"); fflush(stdout);
#endif
    WaitForSingleObject(hc->hevTx, INFINITE);
#ifndef NO_RUNTIME
//    printf("2"); fflush(stdout);
#endif

    if (hc->fDie)
      break;

    do
    {
      DWORD cChars;

      fTxPurged=FALSE;

      cChars = QueueGetSizeContig(&hc->cqTx);

      if (cChars==0)
      {
        QueueWrapPointersRemove(&hc->cqTx);
        cChars = QueueGetSizeContig(&hc->cqTx);
      }

      /* Write this stuff to the comms device */

      SetEvent(ol.hEvent);
      SetCommTimeouts(hc->h, &hc->ct);

      WaitForSingleObject(hc->hevTxPause, INFINITE);

      if (hc->fDie)
        break;

      if (!WriteFile(hc->h, hc->cqTx.pbHead, cChars,
                     &dwBytesWritten, &ol))
      {
        if (GetLastError()==ERROR_IO_PENDING)
          GetOverlappedResult(hc->h, &ol, &dwBytesWritten, TRUE);
        else
        {
          #ifdef DEBUG_TX
          printf("!!GetLastError is %d\n", GetLastError());
          #endif
        }
      }

      if (hc->fDie)
        break;


      /* If we purged the transmit buffer while in the middle of            *
       * a tx, don't try to update the buffers.                             */

      if (fTxPurged)
        break;

      /* Move the new head of the ring buffer up to the last byte             *
       * we transmitted.                                                      */

      if (dwBytesWritten > 0)
        QueueRemoveContig(&hc->cqTx, dwBytesWritten);
      else
      {
        DWORD dwStat;
        COMSTAT cs;

        ClearCommError(hc->h, &dwStat, &cs);

        #ifdef DEBUG_TX
        printf("@Tx: GOT ZERO! (%d) - to write: %d\n", dwStat,
               pbTxBot-hc->pbTxHead);

        printf("(%d %d %d %d %d %d %d %d %d %d)\n",
               cs.fCtsHold, cs.fDsrHold, cs.fRlsdHold, cs.fXoffHold,
               cs.fXoffSent, cs.fEof, cs.fTxim, cs.fReserved, cs.cbInQue,
               cs.cbOutQue);

        fflush(stdout);
        #endif
      }
    }
    while (hc->cqTx.pbHead != hc->cqTx.pbTail);

    if (hc->fDie)
      break;

    /* Repeat while there is stuff to transmit */
  }

  //HJK 98/03/22 Thread exits now regularly, in stead of committing suicide :-)
  --hc->cThreads;
  ExitThread(1);
  return 0;
}



/* Receive-character thread */

DWORD __stdcall RxThread(LPVOID arg)
{
  HCOMM hc;                     /* Local thread information */
  DWORD dwBytesRead;            /* Number of bytes actually read */
  OVERLAPPED ol;                /* Structure for overlapped file i/o */
  BOOL rc;

  hc=(HCOMM)arg;                /* Make local copy of thread info */
  ++hc->cThreads;

  /* Create a semaphore which indicates that reading is complete */

  ol.hEvent=hc->hevRxDone;
  ol.Offset=ol.OffsetHigh=0L;

  for (;;)                  /* Repeat forever */
  {
    DWORD cChars;

    if (hc->fDie)
      break;

    cChars = QueueGetFreeContig(&hc->cqRx);

    if (cChars == 0)
    {
      QueueWrapPointersInsert(&hc->cqRx);
      cChars = QueueGetFreeContig(&hc->cqRx);
    }

    /* Wait until there are some bytes to read */

    if (cChars)
    {
      fRxPurged=FALSE;

      WaitForSingleObject(hc->hevRxPause, INFINITE);

      if (hc->fDie)
        break;

      if ((rc=ReadFile(hc->h, hc->cqRx.pbTail, cChars,
                       &dwBytesRead, &ol)) != FALSE ||
        GetLastError()==ERROR_IO_PENDING)
      {
        /* The read was valid, so wait until all bytes are read */

        if (!rc)
          GetOverlappedResult(hc->h, &ol, &dwBytesRead, TRUE);

        if (hc->fDie)
          break;

        #ifdef DEBUG_PIPE
        /*if (dwBytesRead)
          PDPrintf("rx: %d '%-*.*s'\n", dwBytesRead,
                 dwBytesRead, dwBytesRead, hc->pbRxTail);*/
        #endif

        /* If the receive buffer was purged, don't update anything */

        if (fRxPurged)
          continue;

        QueueInsertContig(&hc->cqRx, dwBytesRead);

        /* Inform ComGetc or ComRead that it's okay to continue */

        /*SetEvent(hc->hevRx);*/ /* not used */
      }
      else
      {
        #ifdef DEBUG_PIPE
        PDPrintf("@ReadFile error - rc=%d, lasterror=%d\n",
               rc, GetLastError());
        #endif
      }

      if (hc->fDie)
        break;
    }
    else /* !cChars */
    {
      Sleep(1);
    }

    if (hc->fDie)
      break;
  }

  //HJK 98/03/22 Thread exits now regularly, in stead of committing suicide :-)
  --hc->cThreads;
  ExitThread(1);
  return 0;
}



/* Monitor thread - This keeps tabs on the communications port,             *
 * monitoring for certain events (such as a change in DCD, a                *
 * ^C received from the user, and so on).                                   */

DWORD __stdcall MnThread(LPVOID arg)
{
  HCOMM hc;                     /* Local thread information */
  DWORD dwCond;                 /* Condition causing this event */
  DWORD dwErrors;               /* Communication errors detected */
  DWORD dwJunk;                 /* Junk dword for GetOverlappedResult */
  OVERLAPPED ol;                /* Structure used for overlapped I/O info */
  DWORD rc;

  hc=(HCOMM)arg;                /* Make local copy of thread info */
  ++hc->cThreads;

  /* Create a do-nothing semaphore used by NT to indicating I/O completion */

  ol.hEvent=hc->hevMonDone;
  ol.Offset=ol.OffsetHigh=0L;


  /* Find out the initial state of DCD */

  GetCommModemStatus(hc->h, &dwCond);
  hc->fDCD=!!(dwCond & MS_RLSD_ON);

  for (;;)
  {
    /* Wait for something to happen */

    dwCond=0;
    rc=WaitCommEvent(hc->h, &dwCond, &ol);

    if (!rc)
    {
      if (GetLastError()==ERROR_IO_PENDING)
        GetOverlappedResult(hc->h, &ol, &dwJunk, TRUE);
    #ifdef DEBUG_PIPE
      else PDPrintf("@WaitCommEvent error - rc=%d, error=%d\n", 0, GetLastError());
    #endif
    }

    if (hc->fDie)
      break;

#ifdef DEBUG_PIPE
  #ifdef DEBUG_MN
      PDPrintf("@Mn: dwcond=%d\n", dwCond);
  #endif
#endif
    if (dwCond & EV_RLSD)       /* Toggle the DCD flag */
      hc->fDCD=!hc->fDCD;

    if (dwCond & EV_RXFLAG)     /* We got a ^C from the user */
      hc->dwCtrlC++;

    if (dwCond & EV_ERR)        /* Line error (ie. parity, framing) */
    {
      #ifdef DEBUG_PIPE
        PDPrintf("@Mn: Got line error; dwErrors=%08lx\n", dwErrors);
      #endif

      ClearCommError(hc->h, &dwErrors, NULL);
    }

    if (dwCond & EV_TXEMPTY)    /* Transmit queue is empty */
      SetEvent(hc->hevTxWait);

    if (dwCond & EV_RXCHAR)     /* Received one character */
      SetEvent(hc->hevRxWait);

    if (hc->fDie)
      break;
  }

  //HJK 98/03/22 Thread exits now regularly, in stead of committing suicide :-)
  --hc->cThreads;
  ExitThread(1);
  return 0;
}


static void _SetTimeoutBlock(HCOMM hc)
{
  DCB dcb;                        /* Device Control Block info for com port */

  memset(&hc->ct, 0, sizeof hc->ct);   /* Default to a zero timeout for everything */

  hc->ct.ReadIntervalTimeout=16;      /* Wait up to 1 msec between chars */

  GetCommState(hc->h, &dcb);

  if (dcb.BaudRate > 2400)
    hc->ct.ReadTotalTimeoutConstant=125; /* Wait a max of 150 msec for a char */
  else hc->ct.ReadTotalTimeoutConstant=25; /* Wait a max of 25 msec for a char */

  /* The following is to fix problems with GetOverlappedResult hanging      *
   * in TxThread.  I think that this is a bug in NT, but this makes         *
   * Max work correctly, and after a night of debugging, that's all         *
   * that matters. :-)                                                      */

  hc->ct.WriteTotalTimeoutConstant=250; /*SJD Thu  04-22-1993  18:47:04 */

  SetCommTimeouts(hc->h, &hc->ct);
}



/* Initialize the communications port.  This sets up the default            *
 * read-byte timers for the lazy-read thread.                               */

static void _InitPort(HCOMM hc)
{
  DCB dcb;

  /* Set communications timeouts */

  _SetTimeoutBlock(hc);

  /* Get device control block for comm port */

  GetCommState(hc->h, &dcb);

  /* Set the event character to ^C.  If we receive a ^C, an event will      *
   * be generated and handled by our event thread.                          */

  dcb.fOutxDsrFlow=0;
  dcb.fOutX=0;
  dcb.fInX=0;
  dcb.fRtsControl=RTS_CONTROL_ENABLE;
  dcb.fAbortOnError=0;
  dcb.EvtChar=0;
/*  dcb.EvtChar=0x03;*/

  SetCommState(hc->h, &dcb);

  /* Now enable communications events for this port - errors, DCD, and ^C */

  SetCommMask(hc->h, DEFAULT_COMM_MASK);
}




/* Initialize the transmit/receive buffers and parameters */

static BOOL _InitBuffers(HCOMM hc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  /* Set up buffers to the appropriate size */

  SetupComm(hc->h, dwRxBuf, dwTxBuf);


  /* Allocate memory for our private tx/rx buffers */

  if ((hc->cqTx.pbBuf=malloc(dwRxBuf))==NULL ||
      (hc->cqRx.pbBuf=malloc(dwTxBuf))==NULL)
  {
    return FALSE;
  }


  /* Set up ring pointers for the transmit thread */

  hc->cqTx.pbHead = hc->cqTx.pbTail = hc->cqTx.pbBuf;
  hc->cqTx.pbEnd = hc->cqTx.pbBuf + dwTxBuf;


  /* Set up ring pointers for the receive thread */

  hc->cqRx.pbHead = hc->cqRx.pbTail = hc->cqRx.pbBuf;
  hc->cqRx.pbEnd = hc->cqRx.pbBuf + dwRxBuf;

  return TRUE;
}





/* Open the named device as a communications port */

BOOL COMMAPI ComOpenHandle(HANDLE hfComm, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  COMMPROP cp;
  HCOMM hc;
  DWORD dwJunk; /* Junk return value from CreateThread; not used */


#ifdef DEBUG_PIPE
  if (!PDInit("\\\\.\\pipe\\pipedeb"))
  {
    printf("Can't connect to \\\\.\\pipe\\pipedeb\n");
    exit(1);
  }
#endif

  /* Verify that this is a valid comm handle */

  if (!GetCommProperties(hfComm, &cp))
    return FALSE;


  /* Allocate memory for the handle structure */

  if ((hc=malloc(sizeof(*hc)))==NULL)
    return FALSE;

  /* Store the passed handle */

  hc->h=hfComm;

  /* Create event semaphores for controlling our three threads */

  if ((hc->hevTx=CreateEvent(NULL, FALSE, FALSE, NULL))==NULL ||
/*      (hc->hevRx=CreateEvent(NULL, FALSE, FALSE, NULL))==NULL ||*/
      (hc->hevTxDone=CreateEvent(NULL, FALSE, TRUE, NULL))==NULL ||
      (hc->hevRxDone=CreateEvent(NULL, FALSE, TRUE, NULL))==NULL ||
      (hc->hevMonDone=CreateEvent(NULL, FALSE, FALSE, NULL))==NULL ||
      (hc->hevTxWait=CreateEvent(NULL, FALSE, TRUE, NULL))==NULL ||
      (hc->hevRxWait=CreateEvent(NULL, FALSE, TRUE, NULL))==NULL ||
      (hc->hevTxPause=CreateEvent(NULL, TRUE, TRUE, NULL))==NULL ||
      (hc->hevRxPause=CreateEvent(NULL, TRUE, TRUE, NULL))==NULL)
  {
    return FALSE;
  }

  hc->cThreads = 0;
  hc->fDie = FALSE;

  /* Create the reading and writing threads */

  if ((hc->hTx=CreateThread(NULL, 0, TxThread, (LPVOID)hc, CREATE_SUSPENDED, &dwJunk))==NULL ||
      (hc->hRx=CreateThread(NULL, 0, RxThread, (LPVOID)hc, CREATE_SUSPENDED, &dwJunk))==NULL ||
      (hc->hMn=CreateThread(NULL, 0, MnThread, (LPVOID)hc, CREATE_SUSPENDED, &dwJunk))==NULL)
  {
    free(hc);
    return FALSE;
  }

/*  if (!SetThreadPriority(hc->hRx, THREAD_PRIORITY_TIME_CRITICAL))
    printf("@Couldn't set thread priority!\n");*/

  /* Set up the buffers for tx/rx */

  if (!_InitBuffers(hc, dwRxBuf, dwTxBuf))
  {
    free(hc);
    return FALSE;
  }

  _InitPort(hc);

  /* Start up the tx/rx threads */

  ResumeThread(hc->hMn);
  ResumeThread(hc->hRx);
  ResumeThread(hc->hTx);

  /* Store the comm handle in the caller's variable */

  *phc=hc;
  return TRUE;
}


/* Open a com port.  This takes a device filename, opens it as a            *
 * communications resource, and then passes the handle to the               *
 * ComOpenHandle function to get our threads rolling.                       */

BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  SECURITY_ATTRIBUTES sa;
  HANDLE h;

  /* Fix this handle so that it can be inherited by child processes */

  sa.nLength=sizeof sa;
  sa.lpSecurityDescriptor=NULL;
  sa.bInheritHandle=TRUE;

  /* Attempt to open the specified communications port */

  if ((h=CreateFile(pszDevice,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    &sa,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    (HANDLE)0))==INVALID_HANDLE_VALUE)
  {
    return FALSE;
  }

  /* Now attempt to register this handle to get our threads going */

  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    CloseHandle(h);
    return FALSE;
  }

  return TRUE;
}


/* Close the specified communications device */

BOOL COMMAPI ComClose(HCOMM hc)
{

    // HJK 98/3/23 : Bug in Windows95 can only fixed this way.
    // TerminateThreads is a rather obscure function and should not
    // be used, and not only because it does not work right under Win95.
    // Tried to fix instability problems under NT in threads.

    hc->fDie = TRUE;
    SetEvent(hc->hevTx);
    SetEvent(hc->hevTxPause);
    SetEvent(hc->hevTxDone);
    SetEvent(hc->hevTxWait);
    SetEvent(hc->hevRxPause);
    SetEvent(hc->hevRxDone);
    SetEvent(hc->hevRxWait);
    SetEvent(hc->hevMonDone);
    SetEvent(hc->h);

    while (hc->cThreads > 0)
        Sleep(10L);

  /* Destroy the semaphore handles */

  CloseHandle(hc->hevTx);
/*  CloseHandle(hc->hevRx);*/
  CloseHandle(hc->hevTxDone);
  CloseHandle(hc->hevRxDone);
  CloseHandle(hc->hevMonDone);
  CloseHandle(hc->hevTxWait);
  CloseHandle(hc->hevRxWait);
  CloseHandle(hc->hevTxPause);
  CloseHandle(hc->hevRxPause);
  CloseHandle(hc->h);

  /* Deallocate our local buffer memory */

  free(hc->cqTx.pbBuf);
  free(hc->cqRx.pbBuf);
  free(hc);

#ifdef DEBUG_PIPE
  PDDeinit();
#endif
  return TRUE;
}


/* Return the current status of DCD on this line */

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
#if 1
  return hc->fDCD;  /* NT Oct'92 is broken and EV_RLSD never occurs in MnThread! */
#else
  DWORD dwCond;

  GetCommModemStatus(hc->h, &dwCond);
  return !!(dwCond & MS_RLSD_ON);
#endif
}


/* Append multiple bytes to the transmit ring buffer */

BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
  DWORD dwMaxBytes;

#ifdef DEBUG_PIPE
  /*PDPrintf("@txmain: write %d\n", dwCount);*/
#endif

  /* Repeat while we still have bytes to transfer */

  while (dwCount)
  {
    dwMaxBytes = QueueGetFreeContig(&hc->cqTx);

    if (dwMaxBytes == 0)
    {
      QueueWrapPointersInsert(&hc->cqTx);
      dwMaxBytes = QueueGetFreeContig(&hc->cqTx);
    }

    /* Transmit no more than the requested number of bytes */

    if (dwMaxBytes > dwCount)
      dwMaxBytes = dwCount;

    /* If there is anything to write... */

    if (dwMaxBytes)
    {
      /* Move this stuff into the outgoing ring buffer */

      memmove(hc->cqTx.pbTail, pvBuf, dwMaxBytes);
      QueueInsertContig(&hc->cqTx, dwMaxBytes);


      /* Increment the input pointers so that we can output the rest of     *
       * the block.                                                         */

      dwCount -= dwMaxBytes;
      pvBuf=(char *)pvBuf+dwMaxBytes;

      /* Tell the TX thread to get to work */

      SetEvent(hc->hevTx);
    }

    if (!dwMaxBytes)
    {
      /* We couldn't empty out the ring buffer, so wait for last pending    *
       * write to complete.                                                 */

#ifdef DEBUG_PIPE
      PDPrintf("@Waiting to insert in buffer: (%d)\n"
             "head=%08lx, tail=%08lx, start=%08lx, end=%08lx\n",
             dwCount,
             hc->pbTxHead, hc->pbTxTail, hc->pbTxBuf, hc->pbTxEnd);
#endif

      SetEvent(hc->hevTxWait);

      Sleep(1L);
    }
  }

  return TRUE;
}


/* Read many bytes from the com port */

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
  DWORD dwMaxBytes;
  DWORD dwBytesRead=0;

  do
  {
    dwMaxBytes = QueueGetSizeContig(&hc->cqRx);

    if (dwMaxBytes == 0)
    {
      QueueWrapPointersRemove(&hc->cqRx);

      dwMaxBytes = QueueGetSizeContig(&hc->cqRx);
    }

    /* Make sure that we obtain no more bytes than the user asked for. */

    if (dwMaxBytes > dwBytesToRead)
      dwMaxBytes=dwBytesToRead;


    /* Copy the bytes to the user's data space */

    memmove(pvBuf, hc->cqRx.pbHead, dwMaxBytes);
    QueueRemoveContig(&hc->cqRx, dwMaxBytes);

    /* Increment the data pointers accordingly */

    pvBuf = (char *)pvBuf + dwMaxBytes;
    dwBytesToRead -= dwMaxBytes;
    dwBytesRead += dwMaxBytes;

  }
  while (dwMaxBytes && dwBytesToRead); /* Loop while we still have data */

  *pdwBytesRead=dwBytesRead;

  return !!dwBytesRead;
}


/* Read a single character from the com port */

int COMMAPI ComGetc(HCOMM hc)
{
  DWORD dwBytesRead;
  BYTE b;

  return ComRead(hc, &b, 1, &dwBytesRead) ? b : -1;
}



/* Peek - non-destructive read of first character from com port */

int COMMAPI ComPeek(HCOMM hc)
{
  /* If there is nothing to get, return -1 */

  if (QueueEmpty(&hc->cqRx))
    return -1;

  QueueWrapPointersRemove(&hc->cqRx);

  /* Else return the first character available */

  return (*hc->cqRx.pbHead);
}

/* Write a single character to the com port */

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
  BYTE b=(BYTE)c;

  return ComWrite(hc, &b, 1);
}



/* Wait for a character to be placed in the input queue */

BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut)
{
  DWORD rc;

  /* Set the event mask so that we watch for incoming characters */

  ResetEvent(hc->hevRxWait);
  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_RXCHAR);

  /* Wait for something to happen */

  if (QueueEmpty(&hc->cqRx))
  {
#ifdef DEBUG_PIPE
    PDPrintf("rx: wait\n");
#endif
    rc=WaitForSingleObject(hc->hevRxWait,
                           dwTimeOut==-1 ? INFINITE : dwTimeOut);
#ifdef DEBUG_PIPE
    PDPrintf("rx: end wait\n");
#endif
  }


  /* Disable received-character events */

  SetCommMask(hc->h, DEFAULT_COMM_MASK);

  return (rc != WAIT_TIMEOUT);
}


/* Wait for the transmit queue to empty */

BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut)
{
  DWORD rc;

  /* Set the event mask so that we watch for incoming characters */

  ResetEvent(hc->hevTxWait);
  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_TXEMPTY);

  /* Wait for something to happen */

  if (!QueueEmpty(&hc->cqTx))
  {
#ifdef DEBUG_PIPE
    PDPrintf("tx: wait\n");
#endif
    rc=WaitForSingleObject(hc->hevTxWait,
                           dwTimeOut==-1 ? INFINITE : dwTimeOut);
#ifdef DEBUG_PIPE
    PDPrintf("tx:endwait\n");
#endif
  }


  /* Disable received-character events */

  SetCommMask(hc->h, DEFAULT_COMM_MASK);

  return (rc != WAIT_TIMEOUT);
}


/* Returns the number of characters in the receive ring buffer */

DWORD COMMAPI ComInCount(HCOMM hc)
{
  return QueueGetSize(&hc->cqRx);
}


/* Returns the number of bytes present in the transmit ring buffer */

DWORD COMMAPI ComOutCount(HCOMM hc)
{
  return QueueGetSize(&hc->cqTx);
}


/* Returns the number of free bytes in the transmit ring buffer */

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
  return QueueGetFree(&hc->cqTx);
}


/* Purge either or all of the tx and rx buffers */

BOOL COMMAPI ComPurge(HCOMM hc, DWORD fBuffer)
{
  COMMTIMEOUTS ct;

#ifdef DEBUG_PIPE
  PDPrintf("tx: purge\n");
#endif

  if (fBuffer & COMM_PURGE_RX)
  {
    fRxPurged=TRUE;
    QueuePurge(&hc->cqRx);
    PurgeComm(hc->h, PURGE_RXCLEAR);
  }

  if (fBuffer & COMM_PURGE_TX)
  {
    fTxPurged=TRUE;
    QueuePurge(&hc->cqTx);
    PurgeComm(hc->h, PURGE_TXCLEAR);
  }

  /* Force the read and write functions to terminate by temporarily         *
   * setting the read and write timeouts to (almost) zero.                  */

  ct=hc->ct;

  ct.ReadTotalTimeoutConstant=1;
  ct.WriteTotalTimeoutConstant=1;

  SetCommTimeouts(hc->h, &ct);
  SetCommTimeouts(hc->h, &hc->ct);

  return TRUE;
}


/* Return the file handle associated with this com port */

HANDLE COMMAPI ComGetHandle(HCOMM hc)
{
  return hc->h;
}



/* Get information specific to the serial driver device control block */

BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb)
{
  return GetCommState(hc->h, pdcb);
}



/* Set information specific to the serial driver device control block */

USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb)
{
  return SetCommState(hc->h, pdcb);
}



/* Set the baud rate of the com port, using the DCB functions */

BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE bDataBits, BYTE bStopBits)
{
  DCB dcb;
  BOOL rc;

  GetCommState(hc->h, &dcb);

  dcb.BaudRate=dwBps;

  dcb.ByteSize=bDataBits;
  dcb.Parity=bParity;
  dcb.StopBits=bStopBits;

  rc=SetCommState(hc->h, &dcb);

  _SetTimeoutBlock(hc);

  return rc;
}


BOOL COMMAPI ComPause(HCOMM hc)
{
  /* Stop the RX thread from trying to look for a character.  The TX        *
   * thread doesn't really need to be paused, since it won't try            *
   * to send anything unless we give it a character, but this txpause       *
   * semaphore ensures that nothign is transmitted.                         */

  ResetEvent(hc->hevRxPause);
  ResetEvent(hc->hevTxPause);
  return FALSE;
}


BOOL COMMAPI ComResume(HCOMM hc)
{
  SetEvent(hc->hevRxPause);
  SetEvent(hc->hevTxPause);
  return FALSE;
}


BOOL COMMAPI ComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
{
  (void)hc; (void)fEnable; (void)ulTimeOut;
  return FALSE;
}


#endif /* NT */

