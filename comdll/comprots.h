void _SetTimeoutBlock(HCOMM hc);
void _InitPort(HCOMM hc);
BOOL COMMAPI ComOpenHandle(COMMHANDLE hfComm, HCOMM *phc, DWORD dwRxBuf, 
DWORD dwTxBuf);
BOOL COMMAPI IpComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, 
DWORD dwTxBuf);
BOOL COMMAPI IpComClose(HCOMM hc);
#ifdef TELNET
ssize_t telnet_write(HCOMM hc, const unsigned char *buf, size_t count);
ssize_t timeout_read(int fd, unsigned char *buf, size_t count, time_t timeout);
telnet_moption_t telnetOptionBit(telnet_option_t option);
void setTelnetOption(HCOMM hc, telnet_command_t command, telnet_option_t option);
static inline ssize_t telnet_read(HCOMM hc, unsigned char *buf, size_t count);
void negotiateTelnetOptions(HCOMM hc, int preferBinarySession);
#endif
USHORT COMMAPI IpComIsOnline(HCOMM hc);
BOOL COMMAPI IpComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount);
BOOL COMMAPI IpComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, 
PDWORD pdwBytes);
int COMMAPI IpComGetc(HCOMM hc);
int COMMAPI IpComPeek(HCOMM hc);
BOOL COMMAPI IpComPutc(HCOMM hc, int c);
BOOL COMMAPI IpComRxWait(HCOMM hc, DWORD dwTimeOut);
BOOL COMMAPI IpComTxWait(HCOMM hc, DWORD dwTimeOut);
DWORD COMMAPI IpComInCount(HCOMM hc);
DWORD COMMAPI IpComOutCount(HCOMM hc);
DWORD COMMAPI IpComOutSpace(HCOMM hc);
BOOL COMMAPI IpComPurge(HCOMM hc, DWORD fBuffer);
COMMHANDLE COMMAPI IpComGetHandle(HCOMM hc);
BOOL COMMAPI ComGetDCB(HCOMM hc, LPDCB pdcb);
USHORT COMMAPI ComSetDCB(HCOMM hc, LPDCB pdcb);
BOOL COMMAPI IpComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, BYTE 
bDataBits, BYTE bStopBits);
DWORD IpComGetBaudRate(HCOMM hc);
BOOL COMMAPI IpComPause(HCOMM hc);
BOOL COMMAPI IpComResume(HCOMM hc);
BOOL COMMAPI IpComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut);
BOOL COMMAPI IpComBurstMode(HCOMM hc, BOOL fEnable);

BOOL COMMAPI ModemComOpen(LPTSTR pszDevice, HCOMM *phc, DWORD dwRxBuf, 
DWORD dwTxBuf);
BOOL COMMAPI ModemComClose(HCOMM hc);
USHORT COMMAPI ModemComIsOnline(HCOMM hc);
BOOL COMMAPI ModemComWrite(HCOMM hc, PVOID pvBuf, DWORD dwCount);
BOOL COMMAPI ModemComRead(HCOMM hc, PVOID pvBuf, DWORD dwBytesToRead, 
PDWORD pdwBytesRead);
int COMMAPI ModemComGetc(HCOMM hc);
BOOL COMMAPI ModemComPutc(HCOMM hc, int c);
BOOL COMMAPI ModemComIsAModem(HCOMM hc);
BOOL COMMAPI ModemComWatchDog(HCOMM hc, BOOL fEnable, DWORD ulTimeOut);
int COMMAPI ModemComPeek(HCOMM hc);
COMMHANDLE COMMAPI ModemComGetHandle(HCOMM hc);
DWORD COMMAPI ModemComOutCount(HCOMM hc);
void ModemRaiseDTR (HCOMM hc);
void ModemRaiseDTR (HCOMM hc);
BOOL COMMAPI ModemComSetBaudRate(HCOMM hc, DWORD dwBps, BYTE bParity, 
BYTE bDataBits, BYTE bStopBits);
BOOL COMMAPI ModemComTxWait(HCOMM hc, DWORD dwTimeOut);
BOOL COMMAPI ModemComRxWait(HCOMM hc, DWORD dwTimeOut);
BOOL COMMAPI ModemComPurge(HCOMM hc, DWORD fBuffer);
BOOL COMMAPI ModemComBurstMode(HCOMM hc, BOOL fEnable);
DWORD COMMAPI ModemComInCount(HCOMM hc);
DWORD COMMAPI ModemComOutSpace(HCOMM hc);
BOOL COMMAPI ModemComPause(HCOMM hc);
BOOL COMMAPI ModemComResume(HCOMM hc);






