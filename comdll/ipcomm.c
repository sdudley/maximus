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

/** 
 *  @file   	ipcomm.c   TCP/IP communications driver for Maximus, with NVT (-DTELNET) support.
 *  @note		   Eventually, this file will be split into at least four files:
 *			   stub, misc, nvt, raw. Then we should be able to add another .so 
 *                         for another comm type -- e.g. serial. Stub will call either
 *			   the nvt/raw/other_so routine of the right name, if that fails,
 *                         fall through to misc. Maximus will talk to stub.
 *			  
 *  @author 	Wes Garland
 *  @date   	May 24 2003
 *  @version	$Id: ipcomm.c,v 1.18 2004/06/06 21:46:58 paltas Exp $
 *
 * $Log: ipcomm.c,v $
 * Revision 1.18  2004/06/06 21:46:58  paltas
 * Fixed "carrier detech" in modemcom, and removed delay in ipcomm.
 *
 * Revision 1.17  2004/04/09 21:55:39  paltas
 * New IPcom.c without iac parsing and things, this is done in maxcomm.c
 *
 * Revision 1.16  2004/01/27 21:05:02  paltas
 * Fixed IAC parsing
 *
 * Revision 1.14  2004/01/19 23:37:03  paltas
 * Added some to get freebsd work, and improved maxcomm a bit..
 *
 * Revision 1.13  2004/01/14 16:09:54  paltas
 * Used a better way to switch between modem and ip..
 *
 * Revision 1.12  2004/01/13 00:47:31  paltas
 * Fixed comm module you can both run modem and telnet.
 *
 * Revision 1.11  2003/12/16 12:31:34  paltas
 * Fixed keys in local mode
 *
 * Revision 1.10  2003/11/23 13:14:45  paltas
 * Forking still doesn't work properly, but ModemIO works, just without filetransfer
 *
 * Revision 1.9  2003/11/15 23:24:57  paltas
 * Changed a bit to gaim comb. with modemio
 *
 * Revision 1.8  2003/11/15 23:20:19  paltas
 * Speed up telnet, and made Zmodem transfer working.. Dunno about X/Y
 * modem..
 *
 * Revision 1.7  2003/11/08 15:19:17  paltas
 * Fixed segfault problem in commdll
 *
 * Revision 1.6  2003/06/29 20:49:00  wesgarland
 * Changes made to allow pluggable communications module. Code in not currently
 * pluggable, but "guts" will be identical to pluggable version of telnet
 * and raw IP plugins.
 *
 * Changed representation of COMMHANDLE (and deprecated OSCOMMHANDLE) in wincomm
 * code (pseudo Win32 communications API), to allow the potential for multiple
 * comm handles, better support UNIX comm plug-ins, etc.
 *
 * Added functions to "Windows" communications API which are normally handled by
 * the assignment operator -- those are possible under Win32 because the
 * underlying representation by which all comm data is accessed (e.g. word
 * length, parity, buffer sizes) is an integer (not unlike a UNIX file
 * descriptor), but under UNIX a COMMHANDLE is an incomplete struct pointer.
 * Using these routines instead of "inside" knowledge should allow new code
 * written for UNIX to be backported to Windows (and maybe other OSes) easily.
 *
 * This check-in is "barely tested" and expected to have bugs.
 *
 * Revision 1.5  2003/06/13 04:24:43  wesgarland
 * Corrected tv_usec delay in bound socket select to valid value (0.5s); now accepts 
 * calls under Solaris
 *
 * Revision 1.4  2003/06/11 17:21:48  wesgarland
 * Andrew Clarke: Minor changes for building under FreeBSD + general touchup
 *
 * Revision 1.3  2003/06/11 02:12:01  wesgarland
 * Modified API-visible routines to check for a valid comm handle before using it.
 *
 * Revision 1.2  2003/06/06 00:58:21  wesgarland
 * Update to COMMAPI_VER=2 interface, new 8-bit capable telnet driver, support for
 * better performance in raw IP or telnet modes by toggling Nagle algorythm,
 * better detection for sockets disconnecte at the remote end.
 *
 */

#ifndef UNIX
# error UNIX only!
#endif

#ifndef __GNUC__
static char rcs_id[]="$Id: ipcomm.c,v 1.18 2004/06/06 21:46:58 paltas Exp $";
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <errno.h>
#include "prog.h"
#include "ntcomm.h"

#include "comprots.h"
#include "comstruct.h"

/** Syntactic sugar for UNIX only */
#define unixfd(hc)	FileHandle_fromCommHandle(ComGetHandle(hc))

/** Communications handle. Completes foward declaration in ntcomm.h.
 *  Fields below handleType are private to the module. Fields above
 *  (and including) handleType are "public" to the com module stub.
 */
#if 0
struct _hcomm
{
  /****** Public members available to *any* COMMAPI driver */
  COMMHANDLE		h;              	/**< "Windows" COMMHANDLE -- contains UNIX fd */
  BOOL			burstMode;		/**< FALSE = Nagle algorithm enabled (TCP_NODELAY) */
  const char		*device;		/**< Name of tcp service or port number (ASCIZ) */
  BOOL			fDCD;           	/**< Current status of DCD */
  commHandle_t		handleType;		/**< Type of handle / dl_open stub */
  COMMTIMEOUTS 		ct;          		/**< Timeout values */

  /****** Private members for this file below */
  struct sockaddr_un 	*saddr_p;  		/**< Address we're bound to and listening on or NULL */
  int			listenfd;		/**< File descriptor for saddr_p or -1 */
  signed int		peekHack;		/**< Character we've ComPeek()ed but not ComRead(); or -1 */
  BOOL			burstModePending; 	/**< Next write's burst mode */
  size_t		txBufSize;		/**< Size of the transmit buffer */
} _hcomm;

#endif

void logit(char *format,...);

/** Set up communications timeouts. Values directly from sjd. Note that
 *  there are 16 timeout ticks per ms in some of these fields.
 *
 *  @param hc Maximus Communications Handle.
 */
void _SetTimeoutBlock(HCOMM hc)
{
  DCB dcb;                        /* Device Control Block info for com port */

  if (!hc)
    return;

  memset(&hc->ct, 0, sizeof hc->ct);   /* Default to a zero timeout for everything */

  hc->ct.ReadIntervalTimeout=16;      /* Wait up to 1 msec between chars */

  GetCommState(ComGetHandle(hc), &dcb);

  if (dcb.BaudRate > 2400)
    hc->ct.ReadTotalTimeoutConstant=125; /* Wait a max of 150 msec for a char */
  else hc->ct.ReadTotalTimeoutConstant=25; /* Wait a max of 25 msec for a char */

  /* The following is to fix problems with GetOverlappedResult hanging      *
   * in TxThread.  I think that this is a bug in NT, but this makes         *
   * Max work correctly, and after a night of debugging, that's all         *
   * that matters. :-)                                                      */

  hc->ct.WriteTotalTimeoutConstant=250; /*SJD Thu  04-22-1993  18:47:04 */

  SetCommTimeouts(ComGetHandle(hc), &hc->ct);
}

/** Misc. port initialization. 
 *  @param	hc	Maximus communications handle for opened port we're initializing.
 */
void _InitPort(HCOMM hc)
{
  DCB dcb;

  if (!hc)
    return;

  /* Set communications timeouts */

  _SetTimeoutBlock(hc);

  /* Get device control block for comm port */

  GetCommState(ComGetHandle(hc), &dcb);

  /* Set the event character to ^C.  If we receive a ^C, an event will      *
   * be generated and handled by our event thread.                          */

  dcb.fOutxDsrFlow=0;
  dcb.fOutX=0;
  dcb.fInX=0;
  dcb.fRtsControl=RTS_CONTROL_ENABLE;
  dcb.fAbortOnError=0;
  dcb.EvtChar=0;
/*  dcb.EvtChar=0x03;*/

  SetCommState(ComGetHandle(hc), &dcb);

  /* Now enable communications events for this port - errors, DCD, and ^C */

  SetCommMask(ComGetHandle(hc), DEFAULT_COMM_MASK);
}

/** Open the named device as a communications port.
 *  This is currently a little ugly, because in
 *  certain modes, Maximus will actually return
 *  the -pXX argument, where XX should be a com
 *  port number, but winds up being a file
 *  descriptor in the UNIX model.
 *
 *  @param hfComm    UNIX: ??????. NT: Com Port number
 *  @param phc       [out] Handle we're populating
 *  @param dwTxBuf   Transmit buffer size (deprecated)
 *  @param dwRxBuf   Receive buffer size (deprecated)
 *  @returns         TRUE on success
 */
BOOL COMMAPI ComOpenHandle(COMMHANDLE hfComm, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  HCOMM hc;

  /* Verify that this is a valid comm handle */
  if (!phc)
    return FALSE;

  /* Allocate memory for the handle structure */
  hc = calloc(sizeof(*hc), 1);
  if (hc == NULL)
    return FALSE;

  hc->h = CommHandle_fromFileHandle(NULL, -1);
  SetupComm(hc->h, dwRxBuf, dwTxBuf);
  hc->txBufSize = dwTxBuf;

  /* Store the passed handle (may contain file descriptor)*/
  hc->h = hfComm;

  _InitPort(hc);

  /* Store the comm handle in the caller's variable */
  *phc=hc;
  return TRUE;
}

/**
 *  Open a com port.  This takes a device filename, opens it as a
 *  communications resource, and then passes the handle to the
 *  ComOpenHandle function.
 *
 *  TCP/IP interpretation of the functionality: Take a port number,
 *  listen on it. Do not accept (yet)
 *
 *  @param	pszDevice	Name of the device; since this is a tcp/ip
 *                       	driver, we will assume it is a port name or
 *                       	number. Leading "COM" or "COM:" strings will
 *                       	ignored. This means the service name either
 *                       	cannot start with com, or it will have to
 *                       	be passed as comcom.
 *  @param 	phc		[out] Handle we're populating
 *  @param	dwTxBuf		Transmit buffer size
 *  @param	dwRxBuf		Receive buffer size
 *  @returns			TRUE on success
 */ 
 
#define SOCKPATH "maxipc" 
static char sockpath[PATH_MAX];
static char lockpath[PATH_MAX];

BOOL COMMAPI IpComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  int			fd = -1; 	/**< file descriptor */
  struct sockaddr_un 	serv_addr;
  short			portnum = 0;
  COMMHANDLE		h = NULL;
  char			tmpPath[PATH_MAX];
  h = CommHandle_fromFileHandle(h, -1);

  if (strncasecmp(pszDevice, "Com", 3) == 0)
    pszDevice += 3;

  if (pszDevice[0] == ':')
    pszDevice++;

  portnum=atoi(pszDevice);

  sprintf(sockpath, "%s/%s%d", getcwd(tmpPath, PATH_MAX), SOCKPATH, portnum);
  sprintf(lockpath, "%s/%s%d.lck", getcwd(tmpPath, PATH_MAX), SOCKPATH, portnum);

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    logit("!Unable to create AF_INET socket! (%s)", strerror(errno));
    return FALSE;
  }

  serv_addr.sun_family = AF_UNIX;
  strcpy(serv_addr.sun_path, sockpath);
  unlink(serv_addr.sun_path);
  unlink(lockpath);

  if (bind(fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)))
  {
    logit("!Unable to bind to port %i! (%s)", (int)portnum, strerror(errno));
    close(fd);
    return FALSE;
  }

  /* No listen backlog. See ComIsOnline() */
  if (listen(fd, 1))
  {
    logit("!Unable to listen on port %i! (%s)", (int)portnum, strerror(errno));
    close(fd);
    return FALSE;
  }

  /* Now attempt to initialize this handle */
  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    close(fd);
    return FALSE;
  }

  (*phc)->listenfd	= fd;
  (*phc)->device 	= strdup(pszDevice);
  (*phc)->saddr_p 	= malloc(sizeof(serv_addr));
  if ((!(*phc)->saddr_p) || (!(*phc)->device))
    NoMem();
  *((*phc)->saddr_p) = serv_addr;

  return TRUE;
}

/** Close the specified communications device.
 *  TCP/IP Interpretation: Close the socket
 *
 *  @param	hc	The handle to close
 *  @returns		TRUE on success
 */
BOOL COMMAPI IpComClose(HCOMM hc)
{
  if (!hc)
    return FALSE;

  if (hc->saddr_p)
  {
    if (hc->listenfd == -1)
    {
      shutdown(unixfd(hc), 2);
      close(unixfd(hc));
    }
    else
      close(hc->listenfd);
  }

  hc->fDCD = FALSE;		/* TCP/IP shutdown: remove "carrier" signal */

  if(hc->device)
     free((char *)hc->device);
  if(hc)
     free(hc);

  unlink(sockpath);
  unlink(lockpath);

  return TRUE;
}

ssize_t timeout_read(int fd, unsigned char *buf, size_t count, time_t timeout)
{
  /* for used by telnet_read to read more buffer w/o blocking */
 
  struct timeval	tv;
  int			i;
  fd_set		rfds;

  tv.tv_sec = timeout;
  tv.tv_usec = 5;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  do
  {
    /* Note: on some unices, tv will be decremented to report the
     * amount slept; on others it will not. That means that some
     * unices will wait longer than others in the case that a
     * signal has been received.
     */
    i = select(fd + 1, &rfds, NULL, NULL, &tv);
  } while(i < 0 && (errno == EINTR));

  if (i > 0)
    return read(fd, buf, count);

  if (i == 0)
    return 0;

  return -1;
}


/** Return the current status of DCD on this line
 *  TCP/IP Interpretation: If carrier has not been set true yet,
 *  try and accept. If that succeeds, raise carrier. This is 
 *  analogous to a modem auto-answering and raising the DCD
 *  signal on the serial port.
 *
 *  @note	This routine gets polled while Maximus is idle.
 *		This is how we recognize that a caller is online.
 *
 *  @warning	We pull a super-sneaky trick here to avoid the
 *		(normal) requirement of a listen->accept->fork
 *		daemon. We assume that max -w is being run in
 *		respawn mode (e.g. from inittab). When a call 
 *		comes in we fork and exit the parent, letting the
 *		child continue along its merry way. Respawing
 *		kicks up a new parent, and by then this process
 *		isn't listening for inbound connections anymore.
 *		This also makes it necessary for us to be running
 *		in max -n0 mode, so that each max instance has
 *		a unique node (task) number.
 * 
 *  @param	hc 	Communications handle
 *  @returns		0 when we're offline
 */
USHORT COMMAPI IpComIsOnline(HCOMM hc)
{
  fd_set 		rfds, wfds;
  struct timeval 	tv;
  DCB			dcb;

  if (!hc)
    return 0;

  if (hc->fDCD)
  {
    static byte tries = 0;
    int		rready;

    tries++; tries %= 15;
    if (tries != 0)
      goto skipCheck;	/* Only check once in a while */

    /* "Carrier"? Let's make sure the socket is okay. */
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(unixfd(hc), &rfds);
    FD_SET(unixfd(hc), &wfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (((rready = select(unixfd(hc) + 1, &rfds, NULL, NULL, &tv)) < 0) || (select(unixfd(hc) + 1, NULL, &wfds, NULL, &tv) < 0))
    {
      hc->fDCD = FALSE;
      shutdown(unixfd(hc), 2);
      close(unixfd(hc));      
      unlink(lockpath);
    }

    if ((rready == 1) && hc->fDCD && (hc->peekHack == -1))
    {
      unsigned char 	buf[1];
      ssize_t		i;

      i = read(unixfd(hc), &buf, 1);
      switch(i)
      {
	case 0:
	case -1:
	  hc->fDCD = FALSE;
	  shutdown(unixfd(hc), 20);
	  close(unixfd(hc));
	  break;
	case 1:
	  hc->peekHack = buf[0];
	  break;
      }

      if (hc->fDCD == FALSE)
      {
	logit("!Caller closed TCP/IP connection (Dropped Carrier)");
      }	
    }

    skipCheck:
    return hc->fDCD ? 1 : 0;
  }

  if(hc->listenfd == -1)
    return 0;

  /* No "Carrier"? See if we can accept a connection */
  FD_ZERO(&rfds);
  FD_SET(hc->listenfd, &rfds);

  /* Will longish delay cause problems with console? */
  tv.tv_sec = 0;
  tv.tv_usec = 500000;

  if (select(hc->listenfd + 1, &rfds, NULL, NULL, &tv) > 0)
  {
    int addrSize = sizeof(*hc->saddr_p);
    int fd = -1;

    fd = accept(hc->listenfd, (struct sockaddr *)&hc->saddr_p, &addrSize);
    if (fd >= 0)
    {	
      FILE* 	f = NULL;

      /* Have accepted a socket. Close the bound socket and dump
       * the parent, so that we init can swing open a new task.
       * This technique probably won't cause us much grief, except
       * maybe on a very busy system.
       */
      /* Set accepted descriptor and other misc com parameters */
      
      f = fopen(lockpath, "w");
      fclose(f);
      
      CommHandle_setFileHandle(hc->h, fd);
      hc->fDCD = TRUE;
      hc->listenfd = -1;
      memset(&dcb, 0, sizeof(dcb));
      dcb.isTerminal = FALSE;

      dcb.fBinary	= TRUE;

      SetCommState(ComGetHandle(hc), &dcb);
      ComSetBaudRate(hc, 38400, NOPARITY, 8, ONESTOPBIT);
      _SetTimeoutBlock(hc);

      hc->burstMode = TRUE; 
      hc->burstModePending = FALSE; /* turn off nagle by default */
 
      /* Stuff a fake LF into the input stream to try and kick max 
       * into waking up faster.
       */
      hc->peekHack = '\n';

    }
  }

  return hc->fDCD ? 1 : 0;
}

/** Write a string to the comm device. In the NT version,
 *  this just writes into the transmit ring. This version 
 *  just loops until all the bytes are written out, or
 *  something horrible happens.
 *
 *  @param	hc	Comm handle to write to
 *  @param	pvBuf	Buffer to write
 *  @param	dwCount	How many bytes to write
 *  @returns		TRUE if all bytes are written.
 */
BOOL COMMAPI IpComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
  DWORD bytesWritten;
  DWORD totalBytesWritten;

  if (!hc)
    return FALSE;

  if (!IpComIsOnline(hc)) /* Don't write to the listen fd! */
    return FALSE;

  if (hc->burstMode != hc->burstModePending) /** see ComBurstMode() */
  {
    int	retval;
    int optval;
    int	optlen = sizeof(optval);

    optval = !hc->burstModePending;

    retval = setsockopt(unixfd(hc), IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen);
    if (retval == 0)
      hc->burstMode = hc->burstModePending;
  }

  totalBytesWritten = 0;
  do
  {
    bytesWritten = write(unixfd(hc), (char *)pvBuf + totalBytesWritten, dwCount - totalBytesWritten);
    if ((bytesWritten < 0) && (errno != EINTR))
    {
      logit("!Unable to write to socket (%s)", strerror(errno));
      hc->fDCD = FALSE;
      return FALSE;
    }

    if (bytesWritten != (dwCount - totalBytesWritten))
      sleep(0);

    totalBytesWritten += bytesWritten;
  } while(totalBytesWritten < dwCount);

  return TRUE;
}

/** Read data from the communications device. If we previously
 *  called ComPeek(), peekHack will not be -1. If that's the case,
 *  that byte is prepended to the buffer before further processing.
 *  Routine blocks for whatever was last specified by SetCommTimeouts().
 *
 *  @param	hc			Communications handle
 *  @param	pvBuf			Buffer to populate with data
 *  @param	dwBytesToRead		max number of bytes to read
 *  @param	pdwBytesRead [out]	Number of bytes which were actually read
 *  @returns				TRUE if some data was read
 */
BOOL COMMAPI IpComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
  fd_set 	rfds;
  struct 	timeval tv;
  BOOL 		retval = FALSE;
  ssize_t	bytesRead = 0;

  if (!IpComIsOnline(hc))
    return FALSE;

  if (dwBytesToRead == 0)
    return TRUE;

  if (hc->peekHack >= 0)
  {
    /* This is okay in telnet mode, because ComPeek()
     * is implemented with ComRead, which is guaranteed
     * not to return TELNET data
     */
    *(char *)pvBuf = hc->peekHack;
    pvBuf++;
    hc->peekHack = -1;
    *pdwBytesRead = 1;

    if (--dwBytesToRead == 0)	/* Optimization for one-byte, post-peek read */
      return TRUE;

    retval = TRUE;
  }
  else
    *pdwBytesRead = 0;

  if (hc->burstMode)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
  }
  else
  {
    if (hc->ct.ReadTotalTimeoutConstant)
    {
      /* Initial wait for characters */
      tv.tv_sec  = (hc->ct.ReadTotalTimeoutConstant * 1000) / 1000000;
      tv.tv_usec = (hc->ct.ReadTotalTimeoutConstant * 1000) % 1000000;
    }
    else
    {
      tv.tv_sec = 0;
      tv.tv_usec = (0.1 * 1000000); 
    }
  }

  FD_ZERO(&rfds);
  FD_SET(unixfd(hc), &rfds);

  if (select(unixfd(hc)+1, &rfds, NULL, NULL, &tv))
  {
    bytesRead = read(unixfd(hc), pvBuf, dwBytesToRead);

    if (bytesRead >= 0)
    {
      *pdwBytesRead += bytesRead;
      retval = TRUE;
    }
    else
    {
      if (hc->saddr_p)
      { 
	logit("!Unable to read from socket (%s)", strerror(errno));
        shutdown(unixfd(hc), 2);
        hc->fDCD = FALSE;
      }
  
      retval = FALSE;
    }
  }

  if ((bytesRead != dwBytesToRead) && (bytesRead > 0) && (retval == TRUE) && hc->ct.ReadIntervalTimeout)
  {
    /* Jump back for more data, after fiddling with the buffer. */
    pvBuf += bytesRead;
    dwBytesToRead -= bytesRead;

    tv.tv_usec = (hc->ct.ReadIntervalTimeout * 1000) / 16; /* inter char timeout is in 1/16 of a ms */
    tv.tv_sec  = 0;
  }

  return retval;
}

/** Read a single character from the com port.
 *  @param	hc	Communications handle
 *  @returns		The character on success; -1 when there
 *  			was nothing to read or there was an error.
 *  @see		ComRead()
 */
int COMMAPI IpComGetc(HCOMM hc)
{
  DWORD dwBytesRead;
  BYTE b;

  if (!IpComIsOnline(hc))
    return -1;

  return (IpComRead(hc, &b, 1, &dwBytesRead) == 1) ? b : -1;
}


/** "peek" by reading, and setting peekHack to the value
 *  read. ComRead will return this value as the first
 *  character in the buffer on the next read.
 */
int COMMAPI IpComPeek(HCOMM hc)
{
  if (!IpComIsOnline(hc))
    return -1;

  if (hc->peekHack == -1)
    hc->peekHack = IpComGetc(hc);

  return hc->peekHack;
}

/** Write a single character to the com port */
BOOL COMMAPI IpComPutc(HCOMM hc, int c)
{
  BYTE b=(BYTE)c;

  if (!IpComIsOnline(hc))
    return -1;

  return IpComWrite(hc, &b, 1);
}

/** Wait for a character to be placed in the input queue.
 *  UNIX interpretation: this seems like some kind of select(),
 *  so that's exactly what we'll do.
 *
 *  @note			This routine might select against either
 *				a listening or accepted socket. If 
 *				"carrier" is low, we call ComIsOnline() on
 *				success to accept the incoming connection.
 *  @param	hc		Communications Handle
 *  @param	dwTimeOut	Time to wait, in msec
 *  @returns			TRUE if we can read without blocking.
 */
BOOL COMMAPI IpComRxWait(HCOMM hc, DWORD dwTimeOut)
{
  fd_set fds;
  struct timeval tv;

  if (!hc)
    return FALSE;

  if (hc->peekHack != -1)
    return TRUE;

  FD_ZERO(&fds);
  FD_SET(unixfd(hc), &fds);

  tv.tv_sec = dwTimeOut / 1000;
  tv.tv_usec = dwTimeOut % 1000;

  if (select(unixfd(hc) + 1, &fds, NULL, NULL, &tv) == 1)
  {
    if (hc->fDCD == FALSE)
      (void)IpComIsOnline(hc);
    return TRUE;
  }

  return FALSE;
}


/** Wait for the transmit queue to empty.
 *  UNIX Interpretation: this seems like some kind of
 *  select(). So, if we have "carrier", that's exactly
 *  what we'll do. Otherwise, we'll select for *read*
 *  on the listening descriptor. When this returns
 *  successful, we make a quick call to ComIsOnline()
 *  to get it to do the accept() ... fork().
 *
 *  @param	hc		Communication handle to check
 *  @param	dwTimeOut	Time in milliseconds to wait
 */
BOOL COMMAPI IpComTxWait(HCOMM hc, DWORD dwTimeOut)
{
  fd_set fds;
  struct timeval tv;

  if (!hc)
    return FALSE;

  tv.tv_sec = dwTimeOut / 1000;
  tv.tv_usec = dwTimeOut % 1000;

  if (IpComIsOnline(hc))
  {
    FD_ZERO(&fds);
    FD_SET(unixfd(hc), &fds);

    if (select(unixfd(hc) + 1, NULL, &fds, NULL, &tv) < 0)
      hc->fDCD = FALSE;
  }
  else
  {
    if(hc->listenfd == -1)
	return FALSE;
    FD_ZERO(&fds);
    FD_SET(hc->listenfd, &fds);

    if (select(hc->listenfd + 1, &fds, NULL, NULL, &tv) == 1)
      (void)IpComIsOnline(hc);
  }

  return TRUE;
}

/** Returns the number of characters in the receive ring buffer.
 *  We can't really tell this portably with TCP/IP without writing
 *  an intermediary buffer layer. Instead, we return either 0 or 1;
 *  when we return 0 there is no data pending. This isn't a big
 *  deal, as multiple one-byte reads are a billion times cheaper
 *  than muiltiple one-byte writes, even with Nagle on in the
 *  latter case.
 *
 *  @param	hc	Maximus communication handle to query
 *  @returns		0 if there is no data to be read; 1 otherwise. Always
 *                      returns 0 if no connection has been made.
 */
DWORD COMMAPI IpComInCount(HCOMM hc)
{
  /* Okay, we need to fake this so that mdm_avail() will
   * work. Let's peek, if that works, return that there is
   * one byte available for read..
   */

  int ch;

  if (!IpComIsOnline(hc))
    return 0;

  ch = IpComPeek(hc);

  return (ch >= 0 ? 1 : 0);
}

/** Returns the number of bytes present in the transmit ring buffer.
 *  @param	hc	Maximus communication handle to query
 *  @returns 0
 */
DWORD COMMAPI IpComOutCount(HCOMM hc)
{
  IpComIsOnline(hc);
  return 0;
}

/** Returns the number of free bytes in the transmit ring buffer.
 *  Under UNIX, we fudge this to be the size of the buffer. It
 *  doesn't really matter what the buffer size is, since ComWrite()
 *  won't return until we're done writing, but setting it too small
 *  will cause use to write too many packets (unless we enable nagle)
 *
 *  @param	hc	Maximus communication handle to query
 *  @returns		The transmit buffer size (if set) or 1KB
 */
DWORD COMMAPI IpComOutSpace(HCOMM hc)
{
  if (!IpComIsOnline(hc))
    return 0;

  return hc->txBufSize ? : 1024;
}

/** Purge either or all of the tx and rx buffers.
 *  Since this isn't portably possible for TCP/IP, we
 *  instead read until there is no more data pending, and
 *  hope the write buffer is empty after giving up a timeslice.
 *
 *  @param	hc	Maximus communication handle to purge
 *  @param	fBuffer Which buffer to purge (bitmask) -- legal
 *                      values are COMM_PURGE_RX, COMM_PURGE_TX.
 */
BOOL COMMAPI IpComPurge(HCOMM hc, DWORD fBuffer)
{
#if (COMM_PURGE_TX + COMM_PURGE_RX) != (COMM_PURGE_ALL)
# error COMM_PURGE_* values in ntcomm.h are incorrect
#endif

  if (fBuffer & COMM_PURGE_RX)
    while (IpComIsOnline(hc) && (ComGetc(hc) >= 0));

  if (fBuffer & COMM_PURGE_TX)
    sleep(0);

  return TRUE;
}

/** Return the "Windows" comm handle associated with the Maximus
 *  comm handle.
 *
 *  @param	hc	Maximus communication handle to query
 *  @returns		A "Windows" comm handle for the same
 *                      session.
 */
COMMHANDLE COMMAPI IpComGetHandle(HCOMM hc)
{
  return (COMMHANDLE)hc->h;
}

/** Get information specific to the serial driver device control block.
 *  @param	hc	Maximus communication handle to query.
 *  @pdcb [out]		The DCB associated with the underlying COMMHANDLE.
 *  @returns		TRUE on success.
 */
BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb)
{
  return hc ? GetCommState(ComGetHandle(hc), pdcb) : FALSE;
}

/** Set information specific to the serial driver device control block.
 *  @param	hc	Maximus communication handle to update
 *  @pdcb [out]		The DCB we're associating with the underlying COMMHANDLE.
 *  @returns		TRUE on success.
 */ 
USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb)
{
  return hc ? SetCommState(ComGetHandle(hc), pdcb) : FALSE;
}

/** Set the baud rate of the com port, using the DCB functions.
 *  @param	hc	Maximus communication handle to update
 *  @param	dwBps	Baud rate -- integer value, e.g. 19200 -- NOT "B19200",
 *                      which just "happens" to be the same under MOST (but
 *                      not all) Unices.
 *  @param	bParity		Parity
 *  @param	bDataBits	Data Bits (word length)
 *  @param	bStopBits	Stops Bits
 *  @see	SetCommState
 *  @returns	TRUE on success
 */
BOOL COMMAPI IpComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE bDataBits, BYTE bStopBits)
{
  DCB dcb;
  BOOL rc;

  if (!hc)
    return FALSE;

  GetCommState(ComGetHandle(hc), &dcb);

  dcb.BaudRate=dwBps;

  dcb.ByteSize=bDataBits;
  dcb.Parity=bParity;
  dcb.StopBits=bStopBits;

  rc=SetCommState(ComGetHandle(hc), &dcb);
  _SetTimeoutBlock(hc);

  return rc;
}

/** Get the baud rate of the com port, using the DCB functions.
 *  @param	hc	Maximus communication handle to query
 *  @returns	The baud rate, as a scalar value (not B19200, etc)
 *  @see ComSetBaudRate(), GetCommState()
 */
DWORD IpComGetBaudRate(HCOMM hc)
{
  DCB dcb;

  GetCommState(ComGetHandle(hc), &dcb);

  return dcb.BaudRate;
}

/** Stop the RX thread from trying to look for a character.  The TX       
 *  thread doesn't really need to be paused, since it won't try           
 *  to write anything unless we give it a character, but this txpause      
 *  semaphore ensures that nothign is transmitted.                        
 *
 *  @see	ComResume()
 *  @note	This is not implemented in the TCP/IP driver
 *  @returns	FALSE
 */
BOOL COMMAPI IpComPause(HCOMM hc)
{
  return FALSE;
}

/** Allow the RX thread to look for a character.
 *  @see	ComPause()
 *  @note	This is not implemented in the TCP/IP driver
 *  @returns	FALSE
 */
BOOL COMMAPI IpComResume(HCOMM hc)
{
  return FALSE;
}

BOOL COMMAPI IpComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
{
#ifndef __GNUC__
  (void)hc; (void)fEnable; (void)ulTimeOut;
#endif
  return FALSE;
}

/** Turn burst mode on/off. Extension to the original Maximus comdll
 *  API by Wes. Necessary because we want the option of running 
 *  sockets without the Nagle option, but some of Maximus' code
 *  requires looping over putc()-type calls. These calls are 
 *  non-trivial to change to puts()-type calls, because the looping
 *  does additional character-based processing, such as processing
 *  AVATAR code. So, we normally leave "burst mode" (nagle option)
 *  off, and enable it surrounding those places in the code which
 *  could benefit from packet assembly before transmission.
 *
 *  We actually set the socket option in ComWrite(), since that is
 *  where it matters most. Delaying the option setting means that
 *  successive writes between enable/disable/enable will become
 *  aggregated as one.
 *
 *  @param hc		The communications handle.
 *  @param fEnable	TRUE: Please assemble packets; FALSE: do not assemble packets
 *  returns		The previous state (last fEnable)
 *
 *  @note		Calls in main max sources should be guarded with 
 *			#if (COMMAPI_VER > 1)
 */
BOOL COMMAPI IpComBurstMode(HCOMM hc, BOOL fEnable)
{
  BOOL lastState;
  
  if (!hc)
    return FALSE;

  lastState = hc->burstModePending;
  hc->burstModePending = fEnable;

  return lastState;
}

