/* Modem communication modules for Maximus, by Bo Simonsen */
/* Distributed along with Maximus under the GPL license */

/* Bits "stolen" from BTXE (http://btxe.sourceforge.net) */
/* Might not work well yet.. */

#include <termios.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "prog.h"
#include "ntcomm.h"
#include "telnet.h"
#include "comstruct.h"
#include "comprots.h"

static char filename[128];
static char lockname[128];


BOOL COMMAPI ModemComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
  int                   fd = -1;        /**< file descriptor */
  int pid;
  int tmp;
  FILE* lockfp;
  COMMHANDLE    h = NULL;
  struct termios tios;

  if(strstr(pszDevice, "com"))
    pszDevice += 3;
    
  sprintf(filename, "/dev/ttyS%01d", (unsigned) (atoi(pszDevice)-1));
  sprintf(lockname, "/var/lock/LCK..ttyS%01d", (unsigned) (atoi(pszDevice)-1));

    if(fexist(lockname))
    {
        logit("!Lock file does allready exist! (%s)", lockname);
	exit(0);
    }

  fd = open(filename, O_RDWR | O_NDELAY);
  
  if(fd < 0)
  {
    logit("!Could not open device! (%s)", filename);
    return(FALSE);
  }
  else
  {
    lockfp = fopen(lockname, "w");
    fprintf(lockfp, "%08d", getpid());
    fclose(lockfp);    
  }
    
  h = CommHandle_fromFileHandle(h, -1);

  if (!ComOpenHandle(h, phc, dwRxBuf, dwTxBuf))
  {
    close(fd);
    return FALSE;
  }

  CommHandle_setFileHandle(h, fd);

  _InitPort(*phc);

  if(fd)
  {
     (*phc)->listenfd = fd; 
  }
  
  (*phc)->device        = strdup(filename);
  (*phc)->peekHack	= -1;
  (*phc)->fDCD		= 0;

  return TRUE;
}

BOOL COMMAPI ModemComClose(HCOMM hc)
{
    if(!hc)
	return FALSE;
    
    close(hc->listenfd);
    unlink(lockname);
    
    return TRUE;
} 

USHORT COMMAPI ModemComIsOnline(HCOMM hc)
{
    int tmp=0;

    if(!hc)
       return FALSE;

    if(hc->fDCD == 1)
    {
	if(ioctl(hc->listenfd, TIOCMGET, &tmp) < 0)
	{
	    hc->fDCD = 2;
    	    return(FALSE);
	}
        if((tmp & TIOCM_CD) == FALSE)
        {	
	    hc->fDCD = 2;
    	    logit("!Carrier lost");
	    return (FALSE);
	}
    }
    else if(hc->fDCD == 2)
	return(FALSE);

    return(TRUE);
}

BOOL COMMAPI ModemComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
    if(!ModemComIsOnline(hc))
	return FALSE;

    if(write(hc->listenfd, pvBuf, dwCount) != -1)
	return TRUE;
    else 
	return FALSE;
}

BOOL COMMAPI ModemComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
    fd_set fdrx;
    struct timeval tv;
    int sresult = 0;    
    int BytesRead = 0;
    int tmp = 0;
    
    if(!ModemComIsOnline(hc))
	return FALSE;
    
    tv.tv_usec = 5;
    tv.tv_sec = 0;
    
    FD_ZERO(&fdrx);
    FD_SET(hc->listenfd, &fdrx);

    *pdwBytesRead = 0;
    
    if(select(hc->listenfd + 1, &fdrx, 0, 0, &tv) >= 0)
    {
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
    else
    {
	hc->fDCD = 2;
	return(FALSE);
    }

    return FALSE;	
}

int COMMAPI ModemComGetc(HCOMM hc)
{
    DWORD dwBytesRead;
    char b = 0;

    if(!ModemComIsOnline(hc))
	return -1;
        
    return (ComRead(hc, &b, 1, &dwBytesRead) == 1) ? b : -1;
}

BOOL COMMAPI ModemComPutc(HCOMM hc, int c)
{
    if(!ModemComIsOnline(hc))
	return -1;

    return (ComWrite(hc, &c, 1));
}

BOOL COMMAPI ModemComIsAModem(HCOMM hc)
{
  return TRUE;
}

BOOL COMMAPI ModemComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut)
{
#ifndef __GNUC__ 
  (void)hc; (void)fEnable; (void)ulTimeOut;
#endif
  return FALSE;
}

int COMMAPI ModemComPeek(HCOMM hc)
{
  return FALSE;
}

COMMHANDLE COMMAPI ModemComGetHandle(HCOMM hc)  
{
  if(!hc)
    return NULL;

  return (COMMHANDLE)hc->h;
}

DWORD COMMAPI ModemComOutCount(HCOMM hc)
{
    return(FALSE);
}

void ModemLowerDTR (int fd)
{
  struct termios tty;

  tcgetattr (fd, &tty);
  cfsetospeed (&tty, B0);
  cfsetispeed (&tty, B0);
  tcsetattr (fd, TCSANOW, &tty);
}

void ModemRaiseDTR (int fd, int baud, int cts)
{
  struct termios tty;
  int ttybaud=0;

  fcntl (fd, F_SETFL, FASYNC);

  tcgetattr (fd, &tty);

  tty.c_iflag = 0;
  tty.c_oflag = 0;  
  tty.c_lflag = 0;

  tty.c_cflag = B0 | CS8 | CREAD | CLOCAL;
  if(cts)
     tty.c_cflag |= CRTSCTS;

  switch(baud)
  {
    case CBR_110:
	ttybaud = B110;
	break;
    case CBR_300:
	ttybaud = B300;
	break;
    case CBR_600:
	ttybaud = B600;
	break;
    case CBR_1200:
	ttybaud = B1200;
	break;
    case CBR_2400:
	ttybaud = B2400;
	break;
    case CBR_4800:
	ttybaud = B4800;
	break;
    case CBR_9600:
	ttybaud = B9600;
	break;
    case CBR_14400:
    case CBR_19200:
	ttybaud = B19200;
	break;
    case CBR_38400:
	ttybaud = B38400;
	break;
    case CBR_56000:
    case CBR_57600:
	ttybaud = B57600;
	break;
    case CBR_128000:
    case CBR_115200:
	ttybaud = B115200;
	break;
    case CBR_256000:
	ttybaud = B230400;
	break;	
    default:
	ttybaud = B115200;
	break;
  }
  
  cfsetospeed (&tty, ttybaud);
  cfsetispeed (&tty, ttybaud);
  
  tcsetattr (fd, TCSANOW, &tty);
}


BOOL COMMAPI ModemComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE
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

BOOL COMMAPI ModemComTxWait(HCOMM hc, DWORD dwTimeOut)
{  
    return ModemComIsOnline(hc);
}
BOOL COMMAPI ModemComRxWait(HCOMM hc, DWORD dwTimeOut)
{  
    return ModemComIsOnline(hc);
}

BOOL COMMAPI ModemComPurge(HCOMM hc, DWORD fBuffer)
{  
    return ModemComIsOnline(hc);
}

BOOL COMMAPI ModemComBurstMode(HCOMM hc, BOOL fEnable)
{
  if (!hc)
    return FALSE;

  return 1;
}

DWORD COMMAPI ModemComInCount(HCOMM hc)
{
  int tmp = 0;
  
  if(!ModemComIsOnline(hc))
    return(FALSE);
  
  ioctl(hc->listenfd, FIONREAD, &tmp);
  return tmp;
}

DWORD COMMAPI ModemComOutSpace(HCOMM hc)
{
  return TRUE;
}
BOOL COMMAPI ModemComPause(HCOMM hc)
{
  return FALSE;
}
BOOL COMMAPI ModemComResume(HCOMM hc)
{
  return FALSE;
}

int ModemComIsOnlineNow(HCOMM hc)
{
    hc->fDCD = 1;
    return(TRUE);
}