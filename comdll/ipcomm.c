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
 *  @version	$Id: ipcomm.c,v 1.13 2004/01/14 16:09:54 paltas Exp $
 *
 * $Log: ipcomm.c,v $
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

#define TELNET

static char rcs_id[]="$Id: ipcomm.c,v 1.13 2004/01/14 16:09:54 paltas Exp $";

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
#ifdef TELNET
# include "telnet.h"
#endif

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
#ifdef TELNET
  telnet_moption_t	telnetPendingOptions;	/**< Unprocessed option requests from remote */
  telnet_moption_t	telnetOptions;		/**< Current telnet options (bitmask) */
#endif
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
char sockpath[128];
char lockpath[128];

BOOL COMMAPI IpComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  int			fd = -1; 	/**< file descriptor */
  struct sockaddr_un 	serv_addr;
  struct servent 	*se;  
  int			junk;
  short			portnum = 0;
  COMMHANDLE		h = NULL;
  h = CommHandle_fromFileHandle(h, -1);

  if (strncasecmp(pszDevice, "Com", 3) == 0)
    pszDevice += 3;

  if (pszDevice[0] == ':')
    pszDevice++;

  portnum=atoi(pszDevice);

  sprintf(sockpath, "%s%d", SOCKPATH, portnum);
  sprintf(lockpath, "%s%d.lck", SOCKPATH, portnum);

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

#ifdef TELNET
/** Similar to write(), but is aware of NVT and automagically
 *  performs IAC-escape sequences when needed. The output to
 *  the remote NVT should be the same as if it were a physical
 *  terminal, conneted to a serial port, written to with write().
 *
 *  This routine tries to perform a single blocking write of the
 *  buffer up to and including the first character to escape.
 *  This means it may or may not write the entire buffer --
 *  but that's okay, because write() doesn't guarantee it will
 *  write the entire buffer, either.
 *
 *  @note	In incredibly unfortunate circumstances, it is
 *		theoretically possible for this routine to miss
 *		the second character in an IAC-IAC escape
 *		sequence. This occurs when the first write exactly
 *		fills up the buffer AND there is insufficient room
 *		to write even one more character to the TCP buffer
 *		at least one second later.
 *
 *  @param	HCOMM	Maximus communications handle to write to
 *  @param	buf	Data to write
 *  @param	count	Number of bytes to write
 *
 *  @returns	Number of bytes from the original buffer which
 *		were consumed for output, or the error value
 *		from read() -- -1 on POSIX systems.
 */
ssize_t telnet_write(HCOMM hc, const unsigned char *buf, size_t count)
{
  unsigned char	*iac;
  int		fd = unixfd(hc);

  if (!hc)
    return -1;

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
# define telnet_write(a, b, c) write(unixfd(a),b,c)
#endif

#ifdef TELNET
/** Read, blocking for (at most) the specified timeout.
 *  @see	telnet_read()
 *
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

/** Return the bitmask version of a telnet option; suitable
 *  for setting/querying a telnet_moption_t via bitwise
 *  operations.
 *
 *  @param	The telnet option (byte code)
 *  @returns	The telnet option (bitmask code)
 */
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

/** Set the values of the telnet option bitmasks associated with the
 *  current Maximus file handle. Used to process IAC WILL, WONT, DO,
 *  and DONT requests and acknowlegements from the remote NVT.
 *
 *  @param	hc	The Maximus communications handle associate with the NVT session.
 *  @param	command	The telnet command byte (WILL, WONT, DO, DONT)
 *  @param	option	The telnet option byte (SGA, ECHO, BINARY-TRANSFER, etc).
 */
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
 *  @returns	Number of translated bytes read, or -1 on error.
 */
static inline ssize_t telnet_read(HCOMM hc, unsigned char *buf, size_t count)
{
  unsigned char	*iac, *ch, arg, arg2;
  int		fd = unixfd(hc);
  ssize_t	bytesRead = read(fd, buf, count);	/* Select()ed for read already */
  
 
  if (!hc)
    return -1;

  if (hc->telnetOptions & mopt_TRANSMIT_BINARY)
    goto parse_iac;

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
	timeout_read(unixfd(hc), (char *)&(hc->peekHack), 1, 1);
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

    ch = memchr(buf, '\r', bytesRead);
    if(ch)
	do
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
	    ch = memchr(ch, '\r', bytesRead - (ch - buf));
	} while(ch && bytesRead);
    

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
  
  parse_iac:
  /* Code below here assumes bytesRead, count >= 1 */
  if(count <= 0)
    return bytesRead;
  
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
# define telnet_read(a, b, c) read(unixfd(a),b,c)
#endif

#ifdef TELNET
/** This simple driver doesn't negotiate, it makes demands!
 *  We write with write() to enable us to write telnet data
 *  unmolested. We receive with ComPeek() because it will
 *  consume telnet data, and change the comm parameters
 *  as needed.
 *
 *  @note	The calls to ComPeek() aren't strictly
 *              necessary, as the control codes will get
 *              get consumed as soon as Maximus tries to
 *		read from the comm handle. However, it is
 *		best to process the return codes as quickly
 *		as possible, so as to allow the first ComWrite()
 *		from Maximus to be in the correct mode whenever
 *		possible.
 *
 *  @param	hc			Communications handle
 *  @param	preferBinarySession	TRUE if we'd prefer a binary session with the NVT.
 */
void negotiateTelnetOptions(HCOMM hc, int preferBinarySession)
{
  unsigned char command[3];
  int 		ch;

  ch = IpComPeek(hc);	/* Get the ball rolling */

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_ENVIRON);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_SGA);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_ECHO);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_SGA);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DONT, opt_NAWS);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  if (!preferBinarySession)
    return;

  sprintf(command, "%c%c%c", cmd_IAC, cmd_DO, opt_TRANSMIT_BINARY);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  sprintf(command, "%c%c%c", cmd_IAC, cmd_WILL, opt_TRANSMIT_BINARY);
  write(unixfd(hc), command, 3);
  if (ch == -1)
    ch = IpComPeek(hc);

  return;
}
#endif

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
    tv.tv_usec = 1;

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
      int	optval;
      int	optlen = sizeof(optval);
      pid_t	parentPID = getpid();
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
//      hc->listenfd = -1;
      hc->fDCD = TRUE;
      memset(&dcb, 0, sizeof(dcb));
      dcb.isTerminal = FALSE;

#ifdef TELNET
      dcb.fBinary	= FALSE;	/* Set true after negotiating this */
      logit("#Negotiating Telnet Options");
      negotiateTelnetOptions(hc, TRUE);
#else
      dcb.fBinary	= TRUE;
#endif

      SetCommState(ComGetHandle(hc), &dcb);
      ComSetBaudRate(hc, 38400, NOPARITY, 8, ONESTOPBIT);
      _SetTimeoutBlock(hc);

      /* Setup the suggested buffer sizes as the TCP buffer sizes */
#ifdef BROKEN /* Makes max VERY slow under linux 2.0.30 -- WTF? -- Wes */
      if ((optval = hc->txBufSize))
	setsockopt(unixfd(hc), SOL_SOCKET, SO_SNDBUF, (char *)&optval, optlen);
      if ((optval = hc->rxBufSize))
	setsockopt(unixfd(hc), SOL_SOCKET, SO_RCVBUF, (char *)&optval, optlen);
#endif
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
#if defined(COMMAPI_DEBUG)
    logit("!%sabled nagle algorithm", optval ? "dis" : "en");
#endif

    retval = setsockopt(unixfd(hc), IPPROTO_TCP, TCP_NODELAY, (char *)&optval, optlen);
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
  FD_SET(unixfd(hc), &rfds);

  if (select(unixfd(hc)+1, &rfds, NULL, NULL, &tv))
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

