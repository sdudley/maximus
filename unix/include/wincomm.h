#ifndef _WINCOMM_H
#include <termios.h>

typedef int OSCOMMHANDLE;
typedef int hfComm;
typedef int HFILE;

typedef struct _DCB 
{ 
  DWORD DCBlength;
  DWORD BaudRate;  
  DWORD fBinary  :1;  
  DWORD fParity  :1;  
  DWORD fOutxCtsFlow :1;  
  DWORD fOutxDsrFlow  :1;  
  DWORD fDtrControl  :2;  
  DWORD fDsrSensitivity  :1;  
  DWORD fTXContinueOnXoff  :1;  
  DWORD fOutX  :1;  
  DWORD fInX  :1;  
  DWORD fErrorChar  :1;  
  DWORD fNull  :1;  
  DWORD fRtsControl  :2;  
  DWORD fAbortOnError  :1;
  DWORD fDummy2  :17;  
  WORD wReserved;  
  WORD XonLim;  
  WORD XoffLim;  
  BYTE ByteSize;  
  BYTE Parity;  
  BYTE StopBits;  
  char XonChar;  
  char XoffChar;  
  char ErrorChar;  
  char EofChar;  
  char EvtChar;  
  WORD wReserved1;
} DCB, *LPDCB;

/* Can't use UNIX Bxxx definitions because on some platforms
 * (e.g. Linux) they are not the same numbers as the baud rate
 * they represent -- but are defined that way under Windows.
 * Changing the definition might break some silly code like
 * baud=atoi(config_variable)
 */

#define CBR_110		110
#define CBR_19200	19200
#define CBR_300		300
#define CBR_38400	3400
#define CBR_600		600
#define CBR_56000	5600
#define CBR_1200	1200
#define CBR_57600	57600
#define CBR_2400	2400
#define CBR_115200	115200
#define CBR_4800	4500
#define CBR_128000	128000
#define CBR_9600	9600
#define CBR_256000	256000
#define CBR_14400	14400

#define DTR_CONTROL_ENABLED	1
#define DTR_CONTROL_DISABLED	2
#define DTR_CONTROL_HANDSHAKE	3

#define RTS_CONTROL_ENABLE	1
#define RTS_CONTROL_DISABLE	2
#define RTS_CONTROL_HANDSHAKE	3
#define RTS_CONTROL_TOGGLE	4

/* Not sure if these are the right values, but maximus relies on 'N' */
#define EVENPARITY		'E'
#define MARKPARITY		'M'
#define NOPARITY		'N'
#define ODDPARITY		'O'
#define SPACEPARITY		'S'

#define ONESTOPBIT		1
#define ONE5STOPBITS		15
#define TWOSTOPBITS		2

#define EV_BREAK 	1<<1/* A break was detected on input. */
#define EV_CTS 		1<<2/* The CTS (clear-to-send) signal changed state. */
#define EV_DSR 		1<<3/* The DSR (data-set-ready) signal changed state. */ 
#define EV_ERR 		1<<4/* A line-status error occurred. */
#define EV_RING 	1<<5/* A ring indicator was detected. */
#define EV_RLSD 	1<<6/* The RLSD (receive-line-signal-detect) signal changed state. */
#define EV_RXCHAR 	1<<7/* A character was received and placed in the input buffer. */
#define EV_RXFLAG 	1<<8/* The event character was received and placed in the input buffer. */
#define EV_TXEMPTY 	1<<9/* The last character in the output buffer was sent.  */

/* Line-status errors */
#define CE_FRAME	1
#define CE_OVERRUN	2
#define CE_RXPARITY	3

typedef struct _COMMTIMEOUTS
{  
  DWORD ReadIntervalTimeout;  
  DWORD ReadTotalTimeoutMultiplier;  
  DWORD ReadTotalTimeoutConstant;  
  DWORD WriteTotalTimeoutMultiplier;  
  DWORD WriteTotalTimeoutConstant;
} COMMTIMEOUTS, *LPCOMMTIMEOUTS;

BOOL SetCommState(hfComm hFile,LPDCB lpDCB);
BOOL GetCommState(hfComm hFile, LPDCB lpDCB);
BOOL SetCommMask(hfComm hFile, DWORD dwEvtMask);
BOOL SetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts);
BOOL GetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts);
BOOL SetupComm(OSCOMMHANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue);
BOOL SetCommBreak(OSCOMMHANDLE hFile);
BOOL ClearCommBreak(OSCOMMHANDLE hFile);

#endif /* _WINCOMM_H */
