#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "prog.h"
#include "ntcomm.h"

extern int tcpip;

BOOL COMMAPI ComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, DWORD dwTxBuf)
{
    if(tcpip)
	return IpComOpen(pszDevice, phc, dwRxBuf, dwTxBuf);
    else
	return ModemComOpen(pszDevice, phc, dwRxBuf, dwTxBuf);
}

BOOL COMMAPI ComClose(HCOMM hc)
{
    if(tcpip)
	return IpComClose(hc);
    else
	return ModemComClose(hc);
} 

USHORT COMMAPI ComIsOnline(HCOMM hc)
{
    if(tcpip)
	return IpComIsOnline(hc);
    else
	return TRUE;
}

BOOL COMMAPI ComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount)
{
    if(tcpip)
	return IpComWrite(hc, pvBuf, dwCount);
    else
	return ModemComWrite(hc, pvBuf, dwCount);
}

BOOL COMMAPI ComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, PDWORD pdwBytesRead)
{
    if(tcpip)
	return IpComRead(hc, pvBuf, dwBytesToRead, pdwBytesRead);
    else
	return ModemComRead(hc, pvBuf, dwBytesToRead, pdwBytesRead);    
}

int COMMAPI ComGetc(HCOMM hc)
{
    if(tcpip)
	return IpComGetc(hc);
    else
	return ModemComGetc(hc);
}

BOOL COMMAPI ComPutc(HCOMM hc, int c)
{
    if(tcpip)
	return IpComPutc(hc, c);
    else
	return ModemComPutc(hc, c);
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
    if(tcpip)
	return IpComPeek(hc);
    else
	return ModemComPeek(hc);
}

COMMHANDLE COMMAPI ComGetHandle(HCOMM hc)  
{
    if(tcpip)
	return IpComGetHandle(hc);
    else
	return ModemComGetHandle(hc);
}

DWORD COMMAPI ComOutCount(HCOMM hc)
{
  ComIsOnline(hc);
  return 0;
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
    if(tcpip)
	return IpComBurstMode(hc, fEnable);
    else
	return ModemComBurstMode(hc, fEnable);
}

DWORD COMMAPI ComInCount(HCOMM hc)
{
    if(tcpip)
	IpComInCount(hc);
    else
	ModemComInCount(hc);
}

DWORD COMMAPI ComOutSpace(HCOMM hc)
{
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

