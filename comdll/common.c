#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prog.h"
#include "ntcomm.h"
#include "telnet.h"
#include "comprots.h"
#include "comstruct.h"

extern int tcpip;

extern struct CommApi_ CommApi;

void SetCommApi()
{
    memset(&CommApi, 0, sizeof(struct CommApi_));
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
	CommApi.fComGetc = &ModemComGetc;
	CommApi.fComPutc = &ModemComPutc;	
	CommApi.fComPeek = &ModemComPeek;
	CommApi.fComGetHandle = &ModemComGetHandle;
	CommApi.fComSetBaudRate = &ModemComSetBaudRate;
	CommApi.fComBurstMode = &ModemComBurstMode;
        CommApi.fComInCount = &ModemComInCount;
	CommApi.fComOutSpace = &ModemComOutSpace;
	CommApi.fComOutCount = &ModemComOutCount;
    }

}

BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
    SetCommApi();
    return ((*CommApi.fComOpen)(pszDevice, phc, dwRxBuf, dwTxBuf));
}

BOOL COMMAPI ComClose(HCOMM hc)
{
    if(!CommApi.fComClose)
	SetCommApi();
	
    return ((*CommApi.fComClose)(hc));
} 

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
    if(!CommApi.fComIsOnline)
	SetCommApi();

    return ((*CommApi.fComIsOnline)(hc));
}

BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
    if(!CommApi.fComWrite)
	SetCommApi();

    return ((*CommApi.fComWrite)(hc, pvBuf, dwCount));
}

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
    if(!CommApi.fComRead)
	SetCommApi();

    return ((*CommApi.fComRead)(hc, pvBuf, dwBytesToRead, pdwBytesRead));
}

int COMMAPI ComGetc(HCOMM hc)
{
    if(!CommApi.fComGetc)
	SetCommApi();

    return ((*CommApi.fComGetc)(hc));
}

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
    if(!CommApi.fComPutc)
	SetCommApi();

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
    if(!CommApi.fComPeek)
	SetCommApi();

    return ((*CommApi.fComPeek)(hc));
}

COMMHANDLE COMMAPI ComGetHandle(HCOMM hc)  
{
    if(!CommApi.fComGetHandle)
	SetCommApi();

    return ((*CommApi.fComGetHandle)(hc));
}

DWORD COMMAPI ComOutCount(HCOMM hc)
{
    if(!CommApi.fComOutCount)
	SetCommApi();

    return ((*CommApi.fComOutCount)(hc));
}


BOOL COMMAPI ComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE
bDataBits,
 BYTE bStopBits)
{
    if(!CommApi.fComSetBaudRate)
	SetCommApi();

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
    if(!CommApi.fComBurstMode)
	SetCommApi();

    return ((*CommApi.fComBurstMode)(hc, fEnable));
}

DWORD COMMAPI ComInCount(HCOMM hc)
{
    if(!CommApi.fComInCount)
	SetCommApi();

    return ((*CommApi.fComInCount)(hc));
}

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
    if(!CommApi.fComOutSpace)
	SetCommApi();

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


void RAISE_DTR(HCOMM hc)
{
    if(!tcpip)
	ModemRaiseDTR(hc);
}

void LOWER_DTR(HCOMM hc)
{
    if(!tcpip)
	ModemLowerDTR(hc);
}

