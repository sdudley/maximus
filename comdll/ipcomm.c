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

/* $Id: ipcomm.c,v 1.1 2003/06/05 23:33:19 wesgarland Exp $
 *
 * $Log: ipcomm.c,v $
 * Revision 1.1  2003/06/05 23:33:19  wesgarland
 * Updated to support 7-bit NVT (telnet); massive rewrite of fdcomm.c into
 * ipcomm.c (formerly known as rawcomm.c)
 *
 */

/** 
 *  @file   tcpcomm.c      TCP/IP communications driver for Maximus
 *  @author Wes Garland
 *  @date   May 24 2003
 */

#ifndef UNIX
# error UNIX only!
#endif

#define WATCHDOG				/**< use alarm() to make sure max doesn't die */
#define WATCHDOG_LISTEN_TIMEOUT		0	/**< how long to wait between listen->accept */
#define WATCHDOG_ACTIVITY_TIMEOUT	300	/**< how long to wait between ComRead activity */

static char rcs_id[]="$Id: ipcomm.c,v 1.1 2003/06/05 23:33:19 wesgarland Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include "prog.h"
#include "ntcomm.h"

#ifdef TELNET
# include "telnet.h"
#endif

void logit(char *format,...);

#ifdef WATCHDOG
/** Signal handler */
static HCOMM WD_hc = NULL;
void WD_dropCarrier(int dummy)
{
  if (!WD_hc)
    return;

  shutdown(WD_hc->h, 2);
  close(WD_hc->h);      
  WD_hc->fDCD = 0;

  logit("!" __FILE__ ": WATCHDOG Dropping Carrier..");
}
#endif

/** Open the named device as a communications port.
 *  This is currently a little ugly, because in
 *  certain modes, Maximus will actually return
 *  the -pXX argument, where XX should be a com
 *  port number, but winds up being a file
 *  descriptor in the UNIX model.
 *
 *  @param hfComm    UNIX: File Descriptor. NT: Com Port number
 *  @param phc       [out] Handle we're populating
 *  @param dwTxBuf   Transmit buffer size (deprecated)
 *  @param dwRxBuf   Receive buffer size (deprecated)
 *  @returns         TRUE on success
 */

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

  hc->cThreads = 0;
  hc->fDie = FALSE;

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
 *  @param	dwTxBuf		Transmit buffer size (unused)
 *  @param	dwRxBuf		Receive buffer size (unused)
 *  @returns			TRUE on success
 * 
 */ 
BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  int			h = -1; 	/**< file descriptor */
  struct sockaddr_in 	serv_addr;
  struct servent 	*se;  
  short			portnum = 0;
  int			junk;

  if (strncasecmp(pszDevice, "Com", 3) == 0)
    pszDevice += 3;

  if (pszDevice[0] == ':')
    pszDevice++;

  if ((portnum = (short)atoi(pszDevice)) == 0)
  {
    se = getservbyname(pszDevice, "tcp");
    if (se)
      portnum = se->s_port;
  }
      
  if (!portnum)
    portnum = 2001;

  if ((h = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    logit("!Unable to create AF_INET socket! (%s)", strerror(errno));
    return FALSE;
  }

  serv_addr.sin_family=AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port=htons(portnum);

  junk = AF_INET;
  setsockopt(h, SOL_SOCKET, SO_REUSEADDR, (char *)&junk, sizeof(junk));

  if (bind(h, (struct sockaddr *) &serv_addr, sizeof(serv_addr)))
  {
    logit("!Unable to bind to port %i! (%s)", (int)portnum, strerror(errno));
    close(h);
    return FALSE;
  }

  /* No listen backlog. See ComIsOnline() */
  if (listen(h, 1))
  {
    logit("!Unable to listen on port %i! (%s)", (int)portnum, strerror(errno));
    close(h);
    return FALSE;
  }

  /* Now attempt to register this handle to get our threads going */
  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    close(h);
    return FALSE;
  }

  (*phc)->device = strdup(pszDevice);
  (*phc)->saddr_p = malloc(sizeof(serv_addr));
  if ((!(*phc)->saddr_p) || (!(*phc)->device))
    NoMem();
  *((*phc)->saddr_p) = serv_addr;

#if defined(WATCHDOG)
  WD_hc = *phc;
  signal(SIGALRM, WD_dropCarrier);
  alarm(WATCHDOG_LISTEN_TIMEOUT);
#endif
  
  return TRUE;
}


/** Close the specified communications device.
 *  TCP/IP Interpretation: Close the socket
 *
 *  @param	hc	The handle to close
 *  @returns		TRUE on success
 */
BOOL COMMAPI ComClose(HCOMM hc)
{
  hc->fDie = TRUE;

  if (hc->saddr_p)
    shutdown(hc->h, 2);

  hc->fDCD = 0;			/* TCP/IP shutdown: remove "carrier" signal */

  close(hc->h);
  free((char *)hc->device);
  free(hc);

  return TRUE;
}

#ifdef TELNET
ssize_t telnet_write(int fd, const unsigned char *buf, size_t count)
{
  unsigned char	*iac;

  iac = memchr(buf, cmd_IAC, count);
  if (!iac)
    return write(fd, buf, count);
  else
  {
    ssize_t		bytesWritten;

    /* Write up to the IAC */
    bytesWritten = write(fd, buf, (iac - buf) + 1);
    if (bytesWritten != ((iac - buf) + 1))
      return bytesWritten;

    /* Write an extra IAC, and hope to hell it goes */
    if (write(fd, iac, 1) != 1)
    {
      sleep(1);
      write(fd, iac, 1);
    }

    /* Report that we weren't "able" to write the whole buffer */
    return bytesWritten;
  }
}
#else
# define telnet_write(a, b, c) write(a,b,c)
#endif

static int peekHack = -1;

#ifdef TELNET
ssize_t timeout_read(int fd, unsigned char *buf, size_t count, time_t timeout)
{
  /* for used by telnet_read to read more buffer w/o blocking */
  
  struct timeval	tv;
  int			i;
  fd_set		rfds;

  tv.tv_sec = timeout;
  tv.tv_usec = 500;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  do
  {
    /* Note: on some unices, tv will be decremented to report the
     * amount read; on others it will not. That means that some
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

ssize_t telnet_read(int fd, unsigned char *buf, size_t count)
{
  unsigned char	*iac, *ch, arg, arg2;
  ssize_t	bytesRead = read(fd, buf, count);	/* Select()ed for read already */

  telnet_read_reread:
  if (bytesRead <= 0)
    return bytesRead;

  if (count == 1) 
  {
    /* Asked for a single byte. Make sure the returned byte
     * was nothing special. 
     *
     * Note: According to my docs, NULs are to be ignored
     *       during telnet. So how the heck do we do ZMODEM? 
     *	     Probably with IAC WILL TRANSMIT-BINARY.
     */

    switch(buf[0])
    {
      case '\0':
	count = timeout_read(fd, buf, 1, 0);
	goto telnet_read_reread;
      case cmd_IAC:
	break;				/* Fall through to IAC processing below */
      case '\r':
	peekHack = -1;
	timeout_read(fd, (char *)&peekHack, 1, 1);
	switch(peekHack)
	{
	  case '\n':			/* Telnet end-of-line, give max \r\n */
	    return bytesRead;
	    break;
	  case '\0':			/* Telnet carriage return, give max \r (consume \0) */
	    peekHack = -1;
	    break;
	}
	break;
      default:
	return bytesRead;
    }
  }
  else
  {
    /* Modify in-buffer eol, or cr sequences to look right */

    for (ch = memchr(buf, '\r', bytesRead);
	 ch && bytesRead;
	 ch = memchr(ch, '\r', bytesRead - (ch - buf)))
    {
      switch(ch[1])
      {
	case '\n':			/* Telnet end-of-line, give max \r\n */ 
	  ch++; 
	  break;
	case '\0':			/* Telnet carriage return, give max \r (consume \0) */
	  memmove(ch, ch + 1, --bytesRead);
	  break;
      }
    }

    /* Pull \0 out of the buffer */
    for (ch = memchr(buf, '\0', bytesRead);
	 ch && bytesRead;
	 ch = memchr(ch + 1, '\0', bytesRead - (ch - buf)))
    {
      memmove(ch, ch + 1, --bytesRead);
    }

    /* All characters consumed?? Try again */ 
    if (bytesRead == 0)
    {
      bytesRead = timeout_read(fd, buf, count, 0);
      if (bytesRead > 0)
	goto telnet_read_reread;
      return bytesRead;
    }
  }

  /* Code below here assumes bytesRead, count >= 1 */
  for (iac = memchr(buf, cmd_IAC, bytesRead);
       bytesRead > 0 && iac && (iac < (buf + bytesRead));
       iac = memchr(iac + 1, cmd_IAC, bytesRead))
  {
    /* There was an embedded IAC. This means we need 
     * to locate the telnet data and "consume" it.
     */

    if (iac == (buf + bytesRead - 1))		/* IAC last char in buf? */
    {
      if (timeout_read(fd, &arg, 1, 5) != 1)
      {
	sleep(1);
	if (timeout_read(fd, &arg, 1, 5) != 1)
	  return bytesRead - 1;			/* Can't find the argument.. drop the IAC */
      }

      if (arg == cmd_IAC)			/* Escaped (real) IAC (0xff) @ end of buf */
	return bytesRead;

      bytesRead -= 1;
    }
    else
    {
      arg = iac[1];
      if (arg == cmd_IAC || (arg == cmd_EC))	/* Escaped (real) IAC (0xff), or erase char command */
      {
	bytesRead -= 1;
	memmove(iac, iac + 1, bytesRead - (iac - buf));
	if (arg == cmd_EC)
	  *iac = 0x08; 				/* erase char command: shove a backspace in the stream */
	continue;
      }
      else
      {
	bytesRead -= 2;
	memmove(iac, iac + 2, bytesRead - (iac - buf));
      }
    }

    /* Now the IAC and a single argument have been consumed. iac
     * is now a pointer to arbitrary telnet data 
     */

    switch(arg)
    {
      case cmd_WILL:				/* These each have one additional argument. */
      case cmd_WONT: 
      case cmd_DO:
      case cmd_DONT:
	if (iac == (buf + bytesRead))		/* Extra argument not in buffer */
	{
	  if (timeout_read(fd, &arg, 1, 2) != 1)
	    return bytesRead;			/* Can't find the last argument.. drat. */
	}
	else
	{
	  arg = *iac;
	  bytesRead -= 1;
	  memmove(iac, iac + 1, bytesRead - (iac - buf));
	}
	break; /* continue loop */

      case cmd_SE:				/* These commands have no arguments */
      case cmd_NOP:
      case cmd_DM:
      case cmd_BRK:
      case cmd_IP:
      case cmd_AO:
      case cmd_AYT:
      case cmd_GA:
	break; /* continue loop */
	
      case cmd_SB:				/* Telnet subneg'n data coming; ignore all data 'till IAC/SE */
      {
	unsigned char 	*niac; /* next IAC */
	int		found = 0;

	for (niac = memchr(iac, cmd_IAC, bytesRead - (iac - buf));
	     niac && (niac < (buf + bytesRead));
	     niac = memchr(niac + 1, cmd_IAC, bytesRead - (niac -buf)))
	{
	  if (niac[1] == cmd_SE)
	  {
	    found = 1;
	    break;
	  }
	}
	
	if (found)				/* IAC/SE sequence was in buffer. Consume it */
	{
	  bytesRead -= ((niac + 1) - iac);
	  memmove(iac, niac + 1, bytesRead - ((niac + 1) - buf));
	}
	else					/* IAC/SE seq not in current buffer. Read 'till we find it */
	{
	  unsigned char	c, lastC;
	  ssize_t	i;

	  c = buf[bytesRead - 1];		/* Prime the loop with the last char read */
	  bytesRead = iac - buf;		/* Report back we only read up to the IAC */

	  do
	  {
	    lastC = c;
	    i = timeout_read(fd, &c, 1, 20);
	  } while ((i = 1) && !(c == cmd_SE && lastC == cmd_IAC));
	}
	break; /* continue loop */
      }
      default:
	logit("!Found unknown IAC argument %c", arg);
	break; /* continue loop */
    } /* esac */
  } /* end for */

  return bytesRead;
}
#else
# define telnet_read(a, b, c) read(a,b,c)
#endif

#ifdef TELNET
/** This simple driver doesn't negotiate, it makes demands!
 *  We write with write() to enable us to send telnet data
 *  unmolested. We receive with read() because it will
 *  consume telnet data, and change the comm parameters
 *  as (not) needed.
 */
void negotiateTelnetOptions(HCOMM hc)
{
  unsigned char command[3];
  int 		ch = ComPeek(hc);	/* Get the ball rolling */

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_ENVIRON);
  write(hc->h, command, 3);
  if (ch == -1)
    ch = ComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_SGA);
  write(hc->h, command, 3);
  if (ch == -1)
    ch = ComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_ECHO);
  write(hc->h, command, 3);
  if (ch == -1)
    ch = ComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_SGA);
  write(hc->h, command, 3);
  if (ch == -1)
    ch = ComPeek(hc);


  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_NAWS);
  write(hc->h, command, 3);
  if (ch == -1)
    ch = ComPeek(hc);

  return;
}
#endif

/** Return the current status of DCD on this line
 *  TCP/IP Interpretation: If carrier has not been set true yet,
 *  try and accept. If that succeeds, raise carrier. This is 
 *  analogous to a modem auto-answering and raising the DCD
 *  signal on the serial port.
 *
 *  @note	I'm betting that this routine gets polled while
 *		Maximus is idle. This is how we'll recognize that
 *		a caller is online.
 *
 *  @warning	We pull a super-sneaky trick here to avoid the
 *		(normal) requirement of a listen->accept->fork
 *		daemon. We assume that max -wfc is being run
 *		in respawn mode from inittab. When a call comes
 *		in we fork and exit the parent, letting the
 *		child continue along its merry way. init then
 *		kicks up a new parent, and by then this process
 *		isn't listening for inbound connections anymore.
 *		This also makes it necessary for us to be running
 *		in max -n0 mode, so that each max instance has
 *		a unique node (task) number.
 * 
 *  @returns	0 when we're offline
 *
 */
USHORT COMMAPI ComIsOnline(HCOMM hc)
{
  fd_set rfds, wfds;
  struct timeval tv;

  if (!hc)
    return 0;

  if (hc->fDCD)
  {
    /* "Carrier"? Let's make sure the socket is okay. */
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    FD_SET(hc->h, &rfds);
    FD_SET(hc->h, &wfds);

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if ((select(hc->h + 1, &rfds, NULL, NULL, &tv) < 0) || (select(hc->h + 1, NULL, &wfds, NULL, &tv) < 0))
      hc->fDCD = 0;

    if (hc->fDCD == 0)
    {
      shutdown(hc->h, 2);
      close(hc->h);      
    }

#ifdef WATCHDOG
    alarm(0);
#endif

    return hc->fDCD;
  }

  /* No "Carrier"? See if we can accept a connection */
  FD_ZERO(&rfds);
  FD_SET(hc->h, &rfds);

  /* Will longish delay cause problems with console? */
  tv.tv_sec = 1;
  tv.tv_usec = 5000;

  if (select(hc->h + 1, &rfds, NULL, NULL, &tv) > 0)
  {
    int addrSize = sizeof(*hc->saddr_p);
    int fd;

    fd = accept(hc->h, (struct sockaddr *)&hc->saddr_p, &addrSize);
    if (fd >= 0)
    {	
      int 	optval;
      int 	optlen;
      pid_t	parentPID = getpid();

#if defined(WATCHDOG)
      alarm(0);
      WD_hc = hc;
      signal(SIGALRM, WD_dropCarrier);
#endif

      /* Have accepted a socket. Close the bound socket and dump
       * the parent, so that we init can swing open a new task.
       * This technique probably won't cause us any grief, except
       * maybe on a very busy system.
       */

      close(hc->h);
      if (fork())
	_exit(0);	/* _exit -> no atexit cleanups! */

      logit("#pid %i accepted incoming connection and became pid %i", (int)parentPID, (int)getpid());

      hc->h = fd;
      hc->fDCD = 1;

      /* Now, disable the nagle algorithm to try and reduce
       * character-mode latency.
       */
	
      optlen = sizeof(optval);
      optval = 1;
    
/*      setsockopt(hc->h, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen); */
#ifdef TELNET
      logit("#Negotiating Telnet Options");
      negotiateTelnetOptions(hc);
#endif
    }
  }

  return hc->fDCD;
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
BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
  DWORD bytesWritten;
  DWORD totalBytesWritten;

  if (!hc)
    return FALSE;

  if (!ComIsOnline(hc)) /* Don't write to the listen fd! */
    return FALSE;

  totalBytesWritten = 0;
  do
  {
    bytesWritten = telnet_write(hc->h, (char *)pvBuf + totalBytesWritten, dwCount - totalBytesWritten);
    if ((bytesWritten < 0) && (errno != EINTR))
    {
      logit("!Unable to write to socket (%s)", strerror(errno));
      hc->fDCD = 0;
      return FALSE;
    }
#if defined(WATCHDOG)
    else
      alarm(WATCHDOG_ACTIVITY_TIMEOUT);
#endif
    
    if (bytesWritten != (dwCount - totalBytesWritten))
      sleep(0);

    totalBytesWritten += bytesWritten;
  } while(totalBytesWritten < dwCount);

  return TRUE;
}

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
    /* This is okay in telnet mode, because ComPeek()
     * is implemented with ComRead, which is guaranteed
     * not to return TELNET data
     */
    *(char *)pvBuf = peekHack;
    pvBuf++;
    peekHack = -1;
    *pdwBytesRead = 1;

    if (--dwBytesToRead == 0)	/* Optimization for one-byte, post-peek read */
    {
#if defined(WATCHDOG)
      alarm(WATCHDOG_ACTIVITY_TIMEOUT);
#endif
      return TRUE;
    }

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
    int bytesRead = telnet_read(hc->h, pvBuf, dwBytesToRead);

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

#if defined(WATCHDOG)
  alarm(WATCHDOG_ACTIVITY_TIMEOUT);
#endif

  return retval;
}

/** Read a single character from the com port */

int COMMAPI ComGetc(HCOMM hc)
{
  DWORD dwBytesRead;
  BYTE b;

  if (!ComIsOnline(hc))
    return -1;

  return (ComRead(hc, &b, 1, &dwBytesRead) == 1) ? b : -1;
}

/** "peek" by reading, and setting peekHack to the value
 *  read. ComRead will return this value as the first
 *  character in the buffer on the next read.
 */
int COMMAPI ComPeek(HCOMM hc)
{
  if (!ComIsOnline(hc))
    return -1;

  peekHack = ComGetc(hc);

  return peekHack;
}

/** Write a single character to the com port */
BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
  BYTE b=(BYTE)c;

  if (!ComIsOnline(hc))
    return -1;

  return ComWrite(hc, &b, 1);
}

/** Wait for a character to be placed in the input queue */
BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut)
{
  /* I think this is sort of like select(), but I'm handling that right
   * against read() to avoid blocking problems.
   */

  if (!hc)
    return FALSE;

  if (dwTimeOut == -1)
  {
    while(!ComIsOnline(hc))
      sleep(0);
  }
  else
  {
    /* comisonline takes about 250 ms to run -- guess timeslice is ~25 ms. accuracy is not important here */
    while (!ComIsOnline(hc) && dwTimeOut)
    {
      sleep(0);
      if (dwTimeOut > 275)
        dwTimeOut -= 275;
      else
        dwTimeOut = 0;
    }
  }    

  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_RXCHAR);
  return TRUE;
}


/** Wait for the transmit queue to empty */
BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut)
{
  time_t timeleft = dwTimeOut > 1000 ? dwTimeOut/1050 : 1;

  while (!ComIsOnline(hc) && timeleft--)
    sleep(1); 

  SetCommMask(hc->h, DEFAULT_COMM_MASK | EV_TXEMPTY);
  return TRUE;
}


/** Returns the number of characters in the receive ring buffer */
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
}


/** Returns the number of bytes present in the transmit ring buffer */
DWORD COMMAPI ComOutCount(HCOMM hc)
{
  /* Okay, we need to fake this too, so we'll just always
   * say the buffer is empty.
   */

  ComIsOnline(hc);
  return 0;
}


/** Returns the number of free bytes in the transmit ring buffer */
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

  return TRUE;
}


/** Return the file handle associated with this com port */
OSCOMMHANDLE COMMAPI ComGetHandle(HCOMM hc)
{
  return hc->h;
}


/** Get information specific to the serial driver device control block */
BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb)
{
  return GetCommState(hc->h, pdcb);
}



/** Set information specific to the serial driver device control block */
USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb)
{
  return SetCommState(hc->h, pdcb);
}

/** Set the baud rate of the com port, using the DCB functions */
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













