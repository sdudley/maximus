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

/* $Id: ipcomm.c,v 1.3 2003/06/11 02:12:01 wesgarland Exp $
 *
 * $Log: ipcomm.c,v $
 * Revision 1.3  2003/06/11 02:12:01  wesgarland
 * Modified API-visible routines to check for a valid comm handle before using it.
 *
 * Revision 1.2  2003/06/06 00:58:21  wesgarland
 * Update to COMMAPI_VER=2 interface, new 8-bit capable telnet driver, support for
 * better performance in raw IP or telnet modes by toggling Nagle algorythm,
 * better detection for sockets disconnecte at the remote end.
 *
 */

/** 
 *  @file   ipcomm.c       TCP/IP communications driver for Maximus, with NVT (-DTELNET) support
 *  @note		   Eventually, this file will be split into at least four files:
 *			   stub, misc, nvt, raw. Then we should be able to add another .so 
 *                         for another comm type -- e.g. serial. Stub will call either
 *			   the nvt/raw/other_so routine of the right name, if that fails,
 *                         fall through to misc. Maximus will talk to stub.
 *			  
 *  @author Wes Garland
 *  @date   May 24 2003
 */

#ifndef UNIX
# error UNIX only!
#endif

#define WATCHDOG				/**< use alarm() to make sure max doesn't die */
#undef WATCHDOG
#define WATCHDOG_LISTEN_TIMEOUT		0	/**< how long to wait between listen->accept */
#define WATCHDOG_ACTIVITY_TIMEOUT	300	/**< how long to wait between ComRead activity */

static char rcs_id[]="$Id: ipcomm.c,v 1.3 2003/06/11 02:12:01 wesgarland Exp $";

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
#include <errno.h>
#include "prog.h"
#include "ntcomm.h"

#ifdef TELNET
# include "telnet.h"
#endif

typedef void * commHandle_t;		/**< Eventually, probably will be a dlopen() handle */

/** Communications handle. Completes foward declaration in ntcomm.h.
 *  Fields below handleType are private to the module. Fields above
 *  (and including) handleType are "public" to the com module stub.
 */
struct _hcomm
{
  int 			h;              /**< File Descriptor. For the listening socket (no carrier) or connected socket. */
  BOOL			burstMode;	/**< 0 = Nagle algorithm enabled (TCP_NODELAY) */
  const char		*device;	/**< Name of tcp service or port number (ASCIZ) */
  BOOL			fDCD;           /**< Current status of DCD */
  COMMTIMEOUTS		ct;		/**< Timeouts */
  commHandle_t		handleType;	/**< Description of handleType. Stub in case we ever get multiple drivers going. */

  /* Private members for this file below */
  struct sockaddr_in 	*saddr_p;  	/**< Address we're bound to and listening on. NULL after carrier. */
  size_t		txBufSize;	/**< Current value for SO_SNDBUF */
  size_t		rxBufSize;	/**< Current value for SO_RCVBUF */
  signed int		peekHack;	/**< Character we've ComPeek()ed but not ComRead(); -1 for none */
  BOOL			burstModePending; /**< Next write's burst mode */
  telnet_moption_t	telnetPendingOptions; /**< Unprocessed option requests from remote */
  telnet_moption_t	telnetOptions; /**< Current telnet options (bitmask) */
} _hcomm;

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

/** Setup communications timeouts. Values directly from sjd. Note tha
 *  there are 16 timeout ticks per ms in some of these fields.
 *
 *  @param hc Communications Handle.
 */
static void _SetTimeoutBlock(HCOMM hc)
{
  DCB dcb;                        /* Device Control Block info for com port */

  if (!hc)
    return;

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

static void _InitPort(HCOMM hc)
{
  DCB dcb;

  if (!hc)
    return;

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
  if ((hc=calloc(sizeof(*hc), 1))==NULL)
    return FALSE;

  _InitPort(hc);

  /* Store the passed handle (file descriptor)*/
  hc->h=hfComm;

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

  (*phc)->txBufSize	= dwTxBuf;
  (*phc)->rxBufSize	= dwRxBuf;
  (*phc)->device 	= strdup(pszDevice);
  (*phc)->saddr_p 	= malloc(sizeof(serv_addr));
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
  if (!hc)
    return FALSE;

  if (hc->saddr_p)
    shutdown(hc->h, 2);

  hc->fDCD = 0;			/* TCP/IP shutdown: remove "carrier" signal */

  close(hc->h);
  free((char *)hc->device);
  free(hc);

  return TRUE;
}

#ifdef TELNET
ssize_t telnet_write(HCOMM hc, const unsigned char *buf, size_t count)
{
  unsigned char	*iac;
  int		fd = hc->h;

  if (!hc)
    return -1;

  if (hc->telnetOptions & mopt_TRANSMIT_BINARY)
    return write(fd, buf, count);

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
# define telnet_write(a, b, c) write(a->h,b,c)
#endif

#ifdef TELNET
/** Read, blocking for a specified timeout. Used to support telnet_read().
 *  @param	fd	file descriptor to read from
 *  @param	buf	buffer to populate
 *  @param	count	size of buffer
 *  @param	timeout	Number of seconds to wait for data.
 *
 *  @returns		Number of bytes read, or -1 on error.
 */
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

telnet_moption_t telnetOptionBit(telnet_option_t option)
{
  size_t i;

  for (i=0; (i < sizeof(telnet_OptionList) / sizeof(telnet_OptionList[0])); i++)
  {
    if (telnet_OptionList[i].optEnum == option)
      return telnet_OptionList[i].optBit;
  }

  return 0;
}

void setTelnetOption(HCOMM hc, telnet_command_t command, telnet_option_t option)
{
  BOOL			enable;
  telnet_moption_t	*optionMask;

  switch(command)
  {
    case cmd_WILL:
      enable = TRUE;
      optionMask = &(hc->telnetOptions);
      break;
    case cmd_WONT:
      enable = FALSE;
      optionMask = &(hc->telnetOptions);
      break;
    case cmd_DO:
      enable = TRUE;
      optionMask = &(hc->telnetPendingOptions);
      break;
    case cmd_DONT:
      enable = FALSE;
      optionMask = &(hc->telnetPendingOptions);
      break;
    default:
      logit("!Unregonized telnet option IAC %i %i", command, option);
      return;
  }

  switch(enable)
  {
    case TRUE:
      *optionMask |= telnetOptionBit(option);
      break;
    case FALSE:
      *optionMask &= telnetOptionBit(option);
  }

  return;
}

/** Read, consuming NVT control codes in as transparent a manner as possible.
 *  Processes IAC DO/DONT/WONT codes and adjusts hc as needed. Control codes
 *  are not passed to the caller.
 *
 *  @param	fd	file descriptor to read from
 *  @param	buf	buffer to populate
 *  @param	count	size of buffer
 *  @param	timeout	Number of seconds to wait for data.
 *
 *  @returns		Number of bytes read, or -1 on error.
 */
static inline ssize_t telnet_read(HCOMM hc, unsigned char *buf, size_t count)
{
  unsigned char	*iac, *ch, arg, arg2;
  int		fd = hc->h;
  ssize_t	bytesRead = read(fd, buf, count);	/* Select()ed for read already */

  if (!hc)
    return -1;

  if (hc->telnetOptions & mopt_TRANSMIT_BINARY)
    return bytesRead;

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
	hc->peekHack = -1;
	timeout_read(hc->h, (char *)&(hc->peekHack), 1, 1);
	switch(hc->peekHack)
	{
	  case '\n':			/* Telnet end-of-line, give max \r\n */
	    return bytesRead;
	    break;
	  case '\0':			/* Telnet carriage return, give max \r (consume \0) */
	    hc->peekHack = -1;
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
	  if (timeout_read(fd, &arg2, 1, 2) != 1)
	    return bytesRead;			/* Can't find the last argument.. drat. */
	}
	else
	{
	  arg2 = *iac;
	  bytesRead -= 1;
	  memmove(iac, iac + 1, bytesRead - (iac - buf));
	}
	setTelnetOption(hc, arg, arg2);
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
# define telnet_read(a, b, c) read(a->h,b,c)
#endif

#ifdef TELNET
/** This simple driver doesn't negotiate, it makes demands!
 *  We write with write() to enable us to send telnet data
 *  unmolested. We receive with read() because it will
 *  consume telnet data, and change the comm parameters
 *  as (not) needed.
 *
 *  @param	hc	Communications handle
 */
void negotiateTelnetOptions(HCOMM hc)
{
  unsigned char command[3];
  int 		ch;

  ch = ComPeek(hc);	/* Get the ball rolling -- ComPeek() will process/consume remote telnet commands */

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

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_TRANSMIT_BINARY);
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
 *  @param	hc 	Communications handle
 *  @returns		0 when we're offline
 */
USHORT COMMAPI ComIsOnline(HCOMM hc)
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
    FD_SET(hc->h, &rfds);
    FD_SET(hc->h, &wfds);

    tv.tv_sec = 0;
    tv.tv_usec = 1;

    if (((rready = select(hc->h + 1, &rfds, NULL, NULL, &tv)) < 0) || (select(hc->h + 1, NULL, &wfds, NULL, &tv) < 0))
    {
      hc->fDCD = 0;
      shutdown(hc->h, 2);
      close(hc->h);      
    }

    if ((rready == 1) && hc->fDCD && (hc->peekHack == -1))
    {
      unsigned char 	buf[1];
      ssize_t		i;

      i = read(hc->h, &buf, 1);
      switch(i)
      {
	case 0:
	case -1:
	  hc->fDCD = 0;
	  shutdown(hc->h, 20);
	  close(hc->h);
	  break;
	case 1:
	  hc->peekHack = buf[0];
	  break;
      }

      if (hc->fDCD == 0)
	logit("!Caller closed TCP/IP connection (Dropped Carrier)");
    }

    skipCheck:
#ifdef WATCHDOG
    alarm(0);
#endif

    return hc->fDCD;
  }

  /* No "Carrier"? See if we can accept a connection */
  FD_ZERO(&rfds);
  FD_SET(hc->h, &rfds);

  /* Will longish delay cause problems with console? */
  tv.tv_sec = 0;
  tv.tv_usec = 5000000;

  if (select(hc->h + 1, &rfds, NULL, NULL, &tv) > 0)
  {
    int addrSize = sizeof(*hc->saddr_p);
    int fd;

    fd = accept(hc->h, (struct sockaddr *)&hc->saddr_p, &addrSize);
    if (fd >= 0)
    {	
      int optval;
      int	optlen = sizeof(optval);
      pid_t	parentPID = getpid();

#if defined(WATCHDOG)
      alarm(0);
      WD_hc = hc;
      signal(SIGALRM, WD_dropCarrier);
#endif

      /* Have accepted a socket. Close the bound socket and dump
       * the parent, so that we init can swing open a new task.
       * This technique probably won't cause us much grief, except
       * maybe on a very busy system.
       */

      close(hc->h);
      if (fork())
	_exit(0);	/* _exit -> no atexit cleanups! */ /* This should NOT be changed to exec; fd limit - Wes */

      logit("#pid %i accepted incoming connection and became pid %i", (int)parentPID, (int)getpid());

      /* Setup misc com parameters */
      hc->h = fd;
      hc->fDCD = 1;
      memset(&dcb, 0, sizeof(dcb));

#ifdef TELNET
      dcb.fBinary	= FALSE;	/* Set true after negotiating this */
      logit("#Negotiating Telnet Options");
      negotiateTelnetOptions(hc);
#else
      dcb.fBinary	= TRUE;
#endif

      SetCommState(hc->h, &dcb);
      ComSetBaudRate(hc, 38400, NOPARITY, 8, ONESTOPBIT);
      _SetTimeoutBlock(hc);

      /* Setup the suggested buffer sizes as the TCP buffer sizes */
#ifdef BROKEN /* Makes max VERY slow under linux 2.0.30 -- WTF? -- Wes */
      if ((optval = hc->txBufSize))
	setsockopt(hc->h, SOL_SOCKET, SO_SNDBUF, (char *)&optval, optlen);
      if ((optval = hc->rxBufSize))
	setsockopt(hc->h, SOL_SOCKET, SO_RCVBUF, (char *)&optval, optlen);
#endif
      hc->burstMode = TRUE; 
      hc->burstModePending = FALSE; /* turn off nagle by default */
 
      /* Stuff a fake <esc> into the input stream to try and kick max 
       * into waking up faster.
       */
      hc->peekHack = '\n';
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

  if (hc->burstMode != hc->burstModePending) /** see ComBurstMode() */
  {
    int	retval;
    int optval;
    int	optlen = sizeof(optval);

    optval = !hc->burstModePending;
#if defined(COMMAPI_DEBUG)
    logit("!%sabled nagle algorithm", optval ? "dis" : "en");
#endif

    retval = setsockopt(hc->h, IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen);
    if (retval == 0)
      hc->burstMode = hc->burstModePending;
  }

  totalBytesWritten = 0;
  do
  {
    bytesWritten = telnet_write(hc, (char *)pvBuf + totalBytesWritten, dwCount - totalBytesWritten);
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
BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
  fd_set 	rfds;
  struct 	timeval tv;
  BOOL 		retval = FALSE;
  ssize_t	bytesRead = 0;

  if (!ComIsOnline(hc))
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

  if (hc->burstMode)
  {
    tv.tv_sec = 0;
    tv.tv_usec = 1;
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

  ComRead_getData:
  FD_ZERO(&rfds);
  FD_SET(hc->h, &rfds);

  if (select(hc->h + 1, &rfds, NULL, NULL, &tv) != 0)
  {
    bytesRead = telnet_read(hc, pvBuf, dwBytesToRead);

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

#if defined(WATCHDOG)
  alarm(WATCHDOG_ACTIVITY_TIMEOUT);
#endif

  if ((bytesRead != dwBytesToRead) && (bytesRead > 0) && (retval == TRUE) && hc->ct.ReadIntervalTimeout)
  {
    /* Jump back for more data, after fiddling with the buffer. */
    pvBuf += bytesRead;
    dwBytesToRead -= bytesRead;

    tv.tv_usec = (hc->ct.ReadIntervalTimeout * 1000) / 16; /* inter char timeout is in 1/16 of a ms */
    tv.tv_sec  = 0;
    goto ComRead_getData;
  }

  return retval;
}

/** Read a single character from the com port.
 *  @param	hc	Communications handle
 *  @returns		The character on success; -1 when there
 *  			was nothing to read or there was an error.
 *  @see		ComRead()
 */
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

  if (hc->peekHack == -1)
    hc->peekHack = ComGetc(hc);

  return hc->peekHack;
}

/** Write a single character to the com port */
BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
  BYTE b=(BYTE)c;

  if (!ComIsOnline(hc))
    return -1;

  return ComWrite(hc, &b, 1);
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
BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut)
{
  fd_set fds;
  struct timeval tv;

  if (!hc)
    return FALSE;

  if (hc->peekHack != -1)
    return TRUE;

  FD_ZERO(&fds);
  FD_SET(hc->h, &fds);

  tv.tv_sec = dwTimeOut / 1000;
  tv.tv_usec = dwTimeOut % 1000;

  if (select(hc->h + 1, &fds, NULL, NULL, &tv) == 1)
  {
    if (hc->fDCD == 0)
      (void)ComIsOnline(hc);
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
BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut)
{
  fd_set fds;
  struct timeval tv;

  if (!hc)
    return FALSE;

  FD_ZERO(&fds);
  FD_SET(hc->h, &fds);

  tv.tv_sec = dwTimeOut / 1000;
  tv.tv_usec = dwTimeOut % 1000;

  if (ComIsOnline(hc))
  {
    if (select(hc->h + 1, NULL, &fds, NULL, &tv) < 0)
      hc->fDCD = 0;
  }
  else
    if (select(hc->h + 1, &fds, NULL, NULL, &tv) == 1)
      (void)ComIsOnline(hc);

  return TRUE;
}


/** Returns the number of characters in the receive ring buffer.
 *  We can't really tell this with portably TCP/IP without writing
 *  an intermediary buffer layer. Instead, we return either 0 or 1;
 *  when we return 0 there is no data pending. This isn't as big
 *  a deal, as multiple one-byte reads are a billion times cheaper
 *  than muiltiple one-byte writes, even with Nagle on in the
 *  latter case.
 *
 *  @param	hc	Communication handle
 *  @returns		0 if there is no data to be read; 1 otherwise. Always
 *                      returns 0 if no connection has been made.
 */
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

/** Returns the number of bytes present in the transmit ring buffer.
 *  @returns 0
 */
DWORD COMMAPI ComOutCount(HCOMM hc)
{
  ComIsOnline(hc);
  return 0;
}

/** Returns the number of free bytes in the transmit ring buffer.
 *  Under UNIX, we fudge this to be the size of the buffer. It
 *  doesn't really matter what the buffer size is, since ComWrite()
 *  won't return until we're done writing, but setting it too small
 *  will cause use to send too many packets (unless we enable nagle)
 */
DWORD COMMAPI ComOutSpace(HCOMM hc)
{
  if (!ComIsOnline(hc))
    return 0;

  return hc->txBufSize ? : 1024;
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
  return hc ? hc->h : -1;
}


/** Get information specific to the serial driver device control block */
BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb)
{
  return hc ? GetCommState(hc->h, pdcb) : FALSE;
}



/** Set information specific to the serial driver device control block */
USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb)
{
  return hc ? SetCommState(hc->h, pdcb) : FALSE;
}

/** Set the baud rate of the com port, using the DCB functions */
BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE bDataBits, BYTE bStopBits)
{
  DCB dcb;
  BOOL rc;

  if (!hc)
    return FALSE;

  GetCommState(hc->h, &dcb);

  dcb.BaudRate=dwBps;

  dcb.ByteSize=bDataBits;
  dcb.Parity=bParity;
  dcb.StopBits=bStopBits;

  rc=SetCommState(hc->h, &dcb);
  _SetTimeoutBlock(hc);

  return rc;
}

DWORD ComGetBaudRate(HCOMM hc)
{
  DCB dcb;

  GetCommState(hc->h, &dcb);

  return dcb.BaudRate;
}

BOOL COMMAPI ComPause(HCOMM hc)
{
  /* Stop the RX thread from trying to look for a character.  The TX        *
   * thread doesn't really need to be paused, since it won't try            *
   * to send anything unless we give it a character, but this txpause       *
   * semaphore ensures that nothign is transmitted.                         */

  return FALSE;
}


BOOL COMMAPI ComResume(HCOMM hc)
{
  return FALSE;
}

BOOL COMMAPI ComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
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
BOOL COMMAPI ComBurstMode(HCOMM hc, BOOL fEnable)
{
  BOOL lastState;
  
  if (!hc)
    return FALSE;

  lastState = hc->burstModePending;
  hc->burstModePending = fEnable;

  return lastState;
}

/** Determine if the communications device is a modem,
 *  as opposed to something else, like a listening
 *  socket. Extension to Maximus COMM API by Wes.
 *
 *  @param	hc	The communications handle to check.
 *  @returns		TRUE if the device is a modem.
 *
 *  @note		Calls in main max sources should be guarded with 
 *			#if (COMMAPI_VER > 1)
 */
BOOL COMMAPI ComIsAModem(HCOMM hc)
{
  if (!hc)
    return TRUE;

  return FALSE;
}





