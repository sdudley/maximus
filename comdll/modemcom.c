/* Bits "stolen" from BTXE (http://btxe.sourceforge.net) */
/* Might not work well yet.. */

#include <termios.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include "prog.h"
#include "ntcomm.h"

typedef void * commHandle_t;

struct _hcomm
{
  /****** Public members available to *any* COMMAPI driver */
  COMMHANDLE            h;                      /**< "Windows" COMMHANDLE -- con
tains UNIX fd */
  BOOL                  burstMode;              /**< FALSE = Nagle algorithm ena
bled (TCP_NODELAY) */
  const char            *device;                /**< Name of tcp service or port
 number (ASCIZ) */
  BOOL                  fDCD;                   /**< Current status of DCD */
  commHandle_t          handleType;             /**< Type of handle / dl_open st
ub */
  COMMTIMEOUTS          ct;                     /**< Timeout values */
 /********* Private for this struct **********/
  int                   listenfd;
  signed int		peekHack;
  BOOL                  burstModePending;       /**< Next write's burstmode */
 
} _hcomm;

static void _SetTimeoutBlock(HCOMM hc)
{
  DCB dcb;                        /* Device Control Block info for com port */

  if (!hc)
    return;

  memset(&hc->ct, 0, sizeof hc->ct);   /* Default to a zero timeout for everything */

  hc->ct.ReadIntervalTimeout=16;      /* Wait up to 1 msec between chars */
  if (dcb.BaudRate > 2400)
    hc->ct.ReadTotalTimeoutConstant=125; /* Wait a max of 150 msec for a char */
  else 
    hc->ct.ReadTotalTimeoutConstant=25; /* Wait a max of 25 msec for a char *  
    hc->ct.WriteTotalTimeoutConstant=250; /*SJD Thu  04-22-1993  18:47:04 */
}

static void _InitPort(HCOMM hc)
{
  DCB dcb;

    return;

  /* Set communications timeouts */

  _SetTimeoutBlock(hc);
  dcb.fOutxDsrFlow=0;
  dcb.fOutX=0;
  dcb.fInX=0;
  dcb.fRtsControl=RTS_CONTROL_ENABLE;
  dcb.fAbortOnError=0;
  dcb.EvtChar=0;
}



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
//  SetupComm(hc->h, dwRxBuf, dwTxBuf);
//  hc->txBufSize = dwTxBuf;

  /* Store the passed handle (may contain file descriptor)*/
  hc->h = hfComm;
  _InitPort(hc);

  /* Store the comm handle in the caller's variable */
  *phc=hc;
  return TRUE;
}


BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  int                   fd = -1;        /**< file descriptor */
  char filename[128];
  COMMHANDLE    h = NULL;
  struct termios tios;

  memset(filename, 0, 128);

/*  if(!strncasecmp(pszDevice, "Com", 3))
  {
    pszDevice += 3;
    sprintf(filename, "/dev/ttyS%d", atoi(pszDevice)-1);
  }
  else
  {*/
    strcpy(filename, "/dev/modem");
//  }

  fd = open(filename, O_RDWR | O_NDELAY);
  
  if(fd == -1)
  {
    exit(0);
  }
  
  fcntl (fd, F_SETFL, FASYNC);

  tcgetattr(fd, &tios);
  
  tios.c_iflag = 0;
  tios.c_oflag = 0;  
  tios.c_lflag = 0;

  tios.c_cflag = B0 | CS8 | CREAD | CLOCAL | CRTSCTS;

  tcsetattr(fd, TCSANOW, &tios);

  h = CommHandle_fromFileHandle(h, -1);

  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    close(fd);
    return FALSE;
  }

  _InitPort(phc);

  if(fd)
  {
     (*phc)->listenfd = fd; 
  }
  
  (*phc)->device        = strdup(filename);
  (*phc)->peekHack	= -1;
  return TRUE;
}

BOOL COMMAPI ComClose(HCOMM hc)
{
    if(!hc)
	return FALSE;
    
    close(hc->listenfd);
    
    return TRUE;
} 

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
    if(!hc)
       return 0;
       
    return TRUE;
}

BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
    if(write(hc->listenfd, pvBuf, dwCount) != -1)
	return TRUE;
    else 
	return FALSE;
}

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
    fd_set fdrx;
    struct timeval tv;
    int sresult = 0;    
    int BytesRead = 0;
    int tmp = 0;

    if(hc->peekHack >= 0)
    {
	* (char*) pvBuf = hc->peekHack;
	*pvBuf++;
	hc->peekHack = -1;
	*pdwBytesRead = 1;
	
	if(--dwBytesToRead == 0)
	    return TRUE;
    }
    else
	*pdwBytesRead = 0;

    ioctl(hc->listenfd, TIOCMGET, &tmp);

    FD_ZERO(&fdrx);
    FD_SET(hc->listenfd, &fdrx);
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    
    if(sresult = select(hc->listenfd + 1, &fdrx, NULL, NULL, &tv))
    {
	switch(sresult)
	{
	case -1:
	case 0:
	    break;    
	
	default:
	    BytesRead = read(hc->listenfd, pvBuf, dwBytesToRead);
	    
	    if(BytesRead <= 0)
	    {
		return FALSE;
	    }
	    else	    	    
	    {
		*pdwBytesRead += BytesRead;
		return TRUE;
	    }
	}
    }
    return FALSE;	
}

int COMMAPI ComGetc(HCOMM hc)
{
    DWORD dwBytesRead;
    char b = 0;
        
    return (ComRead(hc, &b, 1, &dwBytesRead) == 1) ? b : -1;
}

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
    char b = c;
    
    return (ComWrite(hc, &b, 1));
}

BOOL COMMAPI ComIsAModem(HCOMM hc)
{
  return TRUE;
}

BOOL COMMAPI ComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
{
#ifndef __GNUC__ 
  (void)hc; (void)fEnable; (void)ulTimeOut;
#endif
  return FALSE;
}

int COMMAPI ComPeek(HCOMM hc)
{
  if (!ComIsOnline(hc))
    return -1;

  if (hc->peekHack == -1)
    hc->peekHack = ComGetc(hc);

  return hc->peekHack;
}

COMMHANDLE COMMAPI ComGetHandle(HCOMM hc)  
{
  return (COMMHANDLE)hc->h;
}

DWORD COMMAPI ComOutCount(HCOMM hc)
{
  ComIsOnline(hc);
  return 0;
}

void
LOWER_DTR (HCOMM hc)
{
  struct termios tty;

  tcgetattr (hc->listenfd, &tty);
  cfsetospeed (&tty, B0);
  cfsetispeed (&tty, B0);
  tcsetattr (hc->listenfd, TCSANOW, &tty);
}

void
RAISE_DTR (HCOMM hc)
{
  struct termios tty;

  LOWER_DTR(hc);
  tcgetattr (hc->listenfd, &tty);
  cfsetospeed (&tty, B115200);
  cfsetispeed (&tty, B115200);
  tcsetattr (hc->listenfd, TCSANOW, &tty);
}


BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE
bDataBits,
 BYTE bStopBits)
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

BOOL COMMAPI ComTxWait(HCOMM hc, DWORD dwTimeOut)
{  
	return TRUE;
}
BOOL COMMAPI ComRxWait(HCOMM hc, DWORD dwTimeOut)
{  
	return TRUE;
}

BOOL COMMAPI ComPurge(HCOMM hc, DWORD fBuffer)
{  
	return TRUE;
}

BOOL COMMAPI ComBurstMode(HCOMM hc, BOOL fEnable)
{
  BOOL lastState;          

  if (!hc)
    return FALSE;

  lastState = hc->burstModePending;
  hc->burstModePending = fEnable;

  return lastState;
}

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

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
  if (!ComIsOnline(hc))
    return 0;

  return 0;
}
BOOL COMMAPI ComPause(HCOMM hc)
{
  return FALSE;
}
BOOL COMMAPI ComResume(HCOMM hc)
{
  return FALSE;
}

