#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prog.h"
#include "ntcomm.h"
#include "telnet.h"
#include "comprots.h"
#include "comstruct.h"

struct CommApi_
{
    BOOL COMMAPI (*fComOpen)(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf);
    BOOL COMMAPI (*fComClose)(HCOMM hc);
    USHORT COMMAPI (*fComIsOnline)(HCOMM hc);
    BOOL COMMAPI (*fComRead)(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead);
    BOOL COMMAPI (*fComWrite)(HCOMM hc, PVOID pvBuf, DWORD dwCount);
    int COMMAPI (*fComGetc)(HCOMM hc);
    BOOL COMMAPI (*fComPutc)(HCOMM hc, int c);
    int COMMAPI (*fComPeek)(HCOMM hc);
    COMMHANDLE COMMAPI (*fComGetHandle)(HCOMM hc);
    BOOL COMMAPI (*fComSetBaudRate)(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE bDataBits, BYTE bStopBits);
    BOOL COMMAPI (*fComBurstMode)(HCOMM hc, BOOL fEnable);
    DWORD COMMAPI (*fComInCount)(HCOMM hc);
    DWORD COMMAPI (*fComOutSpace)(HCOMM hc);
    DWORD COMMAPI (*fComOutCount)(HCOMM hc);    
};

extern int tcpip;

struct CommApi_ CommApi;

BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
    if(tcpip)
    {
	CommApi.fComOpen = &IpComOpen;
	CommApi.fComClose = &IpComClose;
	CommApi.fComIsOnline = &IpComIsOnline;
	CommApi.fComRead = &IpComRead;
	CommApi.fComWrite = &IpComWrite;
	CommApi.fComGetc = &IpComGetc;
	CommApi.fComPutc = &IpComPutc;	
	CommApi.fComPeek = &IpComPeek;
	CommApi.fComGetHandle = &IpComGetHandle;
	CommApi.fComSetBaudRate = &IpComSetBaudRate;
	CommApi.fComBurstMode = &IpComBurstMode;
        CommApi.fComInCount = &IpComInCount;
	CommApi.fComOutSpace = &IpComOutSpace;
	CommApi.fComOutCount = &IpComOutCount;
    }
    else
    {
	CommApi.fComOpen = &ModemComOpen;
	CommApi.fComClose = &ModemComClose;
	CommApi.fComIsOnline = &ModemComIsOnline;
	CommApi.fComRead = &ModemComRead;
	CommApi.fComWrite = &ModemComWrite;
    }
    
    return ((*CommApi.fComOpen)(pszDevice, phc, dwRxBuf, dwTxBuf));
}

BOOL COMMAPI ComClose(HCOMM hc)
{
    return ((*CommApi.fComClose)(hc));
} 

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
    return ((*CommApi.fComIsOnline)(hc));
}

BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
    return ((*CommApi.fComWrite)(hc, pvBuf, dwCount));
}

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
    return ((*CommApi.fComRead)(hc, pvBuf, dwBytesToRead, pdwBytesRead));
}

int COMMAPI ComGetc(HCOMM hc)
{
    return ((*CommApi.fComGetc)(hc));
}

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
    return ((*CommApi.fComPutc)(hc, c));
}

BOOL COMMAPI ComIsAModem(HCOMM hc)
{
  if (tcpip)
    return FALSE;
  else
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
    return ((*CommApi.fComPeek)(hc));
}

COMMHANDLE COMMAPI ComGetHandle(HCOMM hc)  
{
    return ((*CommApi.fComGetHandle)(hc));
}

DWORD COMMAPI ComOutCount(HCOMM hc)
{
    return ((*CommApi.fComOutCount)(hc));
}


BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE
bDataBits,
 BYTE bStopBits)
{
    return ((*CommApi.fComSetBaudRate)(hc, dwBps, bParity, bDataBits, bStopBits));
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
    return ((*CommApi.fComBurstMode)(hc, fEnable));
}

DWORD COMMAPI ComInCount(HCOMM hc)
{
    return ((*CommApi.fComInCount)(hc));
}

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
    return ((*CommApi.fComOutSpace)(hc));
}

BOOL COMMAPI ComPause(HCOMM hc)
{
  return FALSE;
}
BOOL COMMAPI ComResume(HCOMM hc)
{
  return FALSE;
}


