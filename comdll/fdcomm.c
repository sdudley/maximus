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
   fdcomm, by wes -- hackish way to use a pair of file descriptors to
   replace comm.dll...  might be able to use this right from inetd with
   STDIN/STDOUT, but that may cause problems, confusing the local console
   with remote user input. So we will probably use a bind/listen/accept
   loop to replace the serial port. Maybe we'll tie STDIN/STDOUT to 
   /dev/console or the starting pty... More notes when I figure this out.
   Oh yeah, and just using a straight socket means problems with telnet
   codes. But that can be fixed later..

   Copyright 1993, 1994 by Lanius Corporation.  All rights reserved.
*/

#ifndef UNIX
# error UNIX only!
#endif

static char rcs_id[]="$Id: fdcomm.c,v 1.2 2003/06/05 23:26:49 wesgarland Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#include "prog.h"
#include "ntcomm.h"

#ifdef DEBUG_FILE
static FILE *fp_out;
#endif

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

BOOL COMMAPI ComOpenHandle(int hfComm, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  HCOMM hc;

  /* Verify that this is a valid comm handle */
  if (!phc)
    return FALSE;

  /* Allocate memory for the handle structure */
  if ((hc=malloc(sizeof(*hc)))==NULL)
    return FALSE;

  /* Store the passed handle (file descriptor)*/
  hc->h=hfComm;

  /* Create event semaphores for controlling our three threads */

#if 0 /* later */
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
#endif

  hc->cThreads = 0;
  hc->fDie = FALSE;

#if 0 /* later */
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
#endif /* later */

  /* Store the comm handle in the caller's variable */
  *phc=hc;
  return TRUE;
}


/* Open a com port.  This takes a device filename, opens it as a            *
 * communications resource, and then passes the handle to the               *
 * ComOpenHandle function to get our threads rolling.                       */

BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  /* dwRxBuf and dwTxBuf appear to be minimum serial buffer sizes. */
  
  int h = -1; /* fd */
  struct sockaddr_in serv_addr;

#if 0
  /* Attempt to open the specified communications port */
  if ((h=CreateFile(pszDevice,
                    GENERIC_READ | GENERIC_WRITE,
                    0,
                    &sa,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                    (OSCOMMHANDLE)0))==INVALID_HANDLE_VALUE)
  {
    return FALSE;
  }
#endif

  serv_addr.sin_port = 0;

  if ((pszDevice[0] == '/') || (strncasecmp(pszDevice, "Com/", 4) == 0))
  {
    /* modem or pipe */
    h = open(pszDevice, O_RDWR, 0666);
    if (h < 1)
      return FALSE;
  }
  else
  {
    /* port number to listen on */
    int			port = atoi(pszDevice);
    int32              junk;

    if (!port && (strncasecmp(pszDevice, "com", 3) == 0))
      port = atoi(pszDevice + 3);

    if (port == 1)
    {
      /* Special port: stdin */
      h = STDIN_FILENO;
      serv_addr.sin_port = 0;
      goto openIt;
    }
      
    if (!port)
      port = 2001;

    if ((h = socket(AF_INET, SOCK_STREAM, 0)) < 0)
      return FALSE;

    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port=htons((short)port);

    junk = AF_INET;
    setsockopt(h, SOL_SOCKET, SO_REUSEADDR, (char *)&junk, sizeof(junk));

    if (bind(h, (struct sockaddr *) &serv_addr, sizeof(serv_addr)))
    {
      close(h);
      return FALSE;
    }

    if (listen(h, 1))
    {
      close(h);
      return FALSE;
    }
  } 

openIt:
  
  /* Now attempt to register this handle to get our threads going */
  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    close(h);
    return FALSE;
  }

  (*phc)->device = strdup(pszDevice);
  if (serv_addr.sin_port)
  {
    (*phc)->saddr_p = malloc(sizeof(serv_addr));
    if (!(*phc)->saddr_p)
      NoMem();

    *((*phc)->saddr_p) = serv_addr;
  }
  else
    (*phc)->saddr_p = NULL;

  return TRUE;
}


/* Close the specified communications device */

BOOL COMMAPI ComClose(HCOMM hc)
{
    hc->fDie = TRUE;

#if 0  /* later */
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
#endif /* later */

/* not yet 
  free(hc->cqTx.pbBuf);
  free(hc->cqRx.pbBuf);
*/

  if (hc->saddr_p)
  {
    shutdown(hc->h, 2);
    hc->fDCD = 0;
  }

  close(hc->h);
  free((char *)hc->device);
  free(hc);

#ifdef DEBUG_PIPE
  PDDeinit();
#endif

  return TRUE;
}


/* Return the current status of DCD on this line */

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
  /* I'll bet this routine gets "polled" every
   * now and then when max is idle. So we'll
   * use this spot to take a bound/listening
   * socket, and see if it's ready to accept.
   * Assuming we're not already online, that is..
   */

  if (!hc)
    return 0;

  if (!hc->fDCD && hc->saddr_p) /* No "carrier", is a socket: look for inbound */
  {
    fd_set rfds;
    struct timeval tv;
    
    FD_ZERO(&rfds);
    FD_SET(hc->h, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = (0.25 * 1000000); /* one-quarter second wait */

    if (select(hc->h + 1, &rfds, NULL, NULL, &tv) > 0)
    {
      int addrSize = sizeof(*hc->saddr_p);
      int fd;

      fd = accept(hc->h, (struct sockaddr *)&hc->saddr_p, &addrSize);
      if (fd >= 0)
      {	
	int optval;
	int optlen;
    
        hc->listenfd = hc->h;
	hc->h = fd;
	hc->fDCD = 1;

	/* Now, disable the nagle algorithm to try and reduce
         * character-mode latency.
	 */
	
	optlen = sizeof(optval);
	optval = 1;
    
	setsockopt(hc->h, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen);
      }
    }
  }
  else
    hc->fDCD = 1;
  return hc->fDCD;
}


BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
  /* In the MT version, this would write into the transmit
   * ring. This version just loops until all the bytes are
   * written out.
   */

  DWORD bytesWritten;
  DWORD totalBytesWritten;

  if (!hc)
    return FALSE;

  if (!ComIsOnline(hc)) /* which is unbound */
    return FALSE;

  totalBytesWritten = 0;
  do
  {
    bytesWritten = write(hc->h, (char *)pvBuf + totalBytesWritten, dwCount - totalBytesWritten);
    if ((bytesWritten < 0) && (errno != EINTR))
      return FALSE;

    if (bytesWritten != (dwCount - totalBytesWritten))
      sleep(0);

    totalBytesWritten += bytesWritten;
  } while(totalBytesWritten < dwCount);

  return TRUE;
}

#if 0 /* not yet */
BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
  /* Append multiple bytes to the transmit ring buffer */

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
#endif /* not yet */

static int peekHack = -1;

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
  fd_set rfds;
  struct timeval tv;
  BOOL retval = FALSE;

  if (!ComIsOnline(hc))
    return FALSE;

  if (dwBytesToRead == 0)
    return TRUE;

  if (peekHack >= 0)
  {
    *(char *)pvBuf = peekHack;
    pvBuf++;
    peekHack = -1;
    *pdwBytesRead = 1;

    if (dwBytesToRead-- == 0)	/* Optimization for one-byte, post-peek read */
      return TRUE;

    retval = TRUE;
  }
  else
    *pdwBytesRead = 0;

  FD_ZERO(&rfds);
  FD_SET(hc->h, &rfds);

  tv.tv_sec = 0;
  tv.tv_usec = (0.01 * 1000000); /* one-one hundredth of a second wait */
  
  if (select(hc->h + 1, &rfds, NULL, NULL, &tv) != 0)
  {
    int bytesRead = read(hc->h, pvBuf, dwBytesToRead);

    if (bytesRead >= 0)
    {
      *pdwBytesRead += bytesRead;
      retval = TRUE;
    }
    else
    {
      if (hc->saddr_p)
      { 
        shutdown(hc->h, 2);
        hc->fDCD = 0;
      }
  
      retval = FALSE;
    }
  }

#ifdef DEBUG_COMM
  if (*pdwBytesRead)
  {
    char buf[*pdwBytesRead + 1];
    strcpy(buf, pvBuf);
    buf[*pdwBytesRead] = 0;
    syslog(LOG_ERR, "Read %i bytes: %s", buf, *pdwBytesRead);
  }
#endif

  return retval;
}

#if 0 /* not yet */
BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
  /* Read many bytes from the com port */
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
#endif

/* Read a single character from the com port */

int COMMAPI ComGetc(HCOMM hc)
{
  DWORD dwBytesRead;
  BYTE b;

  if (!ComIsOnline(hc))
    return -1;

  return (ComRead(hc, &b, 1, &dwBytesRead) == 1) ? b : -1;
}

int COMMAPI ComPeek(HCOMM hc)
{
  /* "peek" by reading, and setting peekHack to the value
   * read. ComRead will return this value as the first
   * character in the buffer on the next read.
   */

  if (!ComIsOnline(hc))
    return -1;

  peekHack = ComGetc(hc);

  return peekHack;
}

#if 0 /* not yet */
int COMMAPI ComPeek(HCOMM hc)
{
  /* Peek - non-destructive read of first character from com port */
  /* If there is nothing to get, return -1 */

  if (QueueEmpty(&hc->cqRx))
    return -1;

  QueueWrapPointersRemove(&hc->cqRx);

  /* Else return the first character available */

  return (*hc->cqRx.pbHead);
}
#endif
/* Write a single character to the com port */

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
  BYTE b=(BYTE)c;

  if (!ComIsOnline(hc))
    return -1;

  return ComWrite(hc, &b, 1);
}



/* Wait for a character to be placed in the input queue */

BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut)
{
  if (!hc)
    return FALSE;

  if (dwTimeOut == -1)
  {
    while(!ComIsOnline(hc))
      sleep(0);
  }
  else
  {
    /* comisonline takes about 250 ms to run */
    while (!ComIsOnline(hc) && dwTimeOut)
    {
      sleep(0);
      if (dwTimeOut > 275)
        dwTimeOut -= 275;
      else
        dwTimeOut = 0;
    }
  }    
#if 0 /* not yet */
  /* I think this is sort of like select(), but I'm handling that right
   * against read() to avoid wierd-assed blocking problems.
   */

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
#else /* not yet */
  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_RXCHAR);
  return TRUE;
#endif
}


/* Wait for the transmit queue to empty */

BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut)
{
  time_t timeleft = dwTimeOut > 1000 ? dwTimeOut/1050 : 1;

  while (!ComIsOnline(hc) && timeleft--)
    sleep(1); 

#if 0 /* not yet */
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
#else /* not yet */
  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_TXEMPTY);
  return TRUE;
#endif
}


/* Returns the number of characters in the receive ring buffer */

DWORD COMMAPI ComInCount(HCOMM hc)
{
  /* Okay, we need to fake this so that mdm_avail() will
   * work. Let's peek, if that works, return that there is
   * one byte available for read..
   */

  int ch;

  if (!ComIsOnline(hc))
    return 0;

  ch = ComPeek(hc);

  return (ch >= 0 ? 1 : 0);
#if 0
  return QueueGetSize(&hc->cqRx);
#endif
}


/* Returns the number of bytes present in the transmit ring buffer */

DWORD COMMAPI ComOutCount(HCOMM hc)
{
  /* Okay, we need to fake this too, so we'll just always
   * say the buffer is empty.
   */

  ComIsOnline(hc);
  return 0;

#if 0 /* not yet */
  return QueueGetSize(&hc->cqTx);
#endif
}


/* Returns the number of free bytes in the transmit ring buffer */

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
  ComIsOnline(hc);
  return QueueGetFree(&hc->cqTx);
}


/* Purge either or all of the tx and rx buffers */

BOOL COMMAPI ComPurge(HCOMM hc, DWORD fBuffer)
{
  /* can't really fake a socket buffer purge, let's just
   * read until we can't, and assume the out buf is empty.
   */

#if (COMM_PURGE_TX + COMM_PURGE_RX) != (COMM_PURGE_ALL)
# error COMM_PURGE_* values in ntcomm.h are incorrect
#endif

  if (fBuffer & COMM_PURGE_RX)
    while (ComIsOnline(hc) && (ComGetc(hc) >= 0));

#if 0 /* not yet */
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

#endif
  return TRUE;
}


/* Return the file handle associated with this com port */

OSCOMMHANDLE COMMAPI ComGetHandle(HCOMM hc)
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

#if 0 /* not yet */
  ResetEvent(hc->hevRxPause);
  ResetEvent(hc->hevTxPause);
#endif 
  return FALSE;
}


BOOL COMMAPI ComResume(HCOMM hc)
{
#if 0 /* not yet */
  SetEvent(hc->hevRxPause);
  SetEvent(hc->hevTxPause);
#endif
  return FALSE;
}


BOOL COMMAPI ComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
{
#ifndef __GNUC__
  (void)hc; (void)fEnable; (void)ulTimeOut;
#endif
  return FALSE;
}




