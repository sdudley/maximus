/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/***************************************************************************/
/*                                                                         */
/*            DW-TEXT: Win32 Services Library                              */
/*                     Header file                                         */
/*                                                                         */
/*            Created: 05/Jan/99                                           */
/*            Updated: 12/Jan/99                                           */
/*                                                                         */
/*            Copyright (c) 1990-1999 JV DIALOGUE                          */
/*            Written by Pete I. Kvitek                                    */
/*                                                                         */
/***************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifndef DW_W32_DEFS
#define DW_W32_DEFS

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   d e c l a r a t i o n s                     //
/////////////////////////////////////////////////////////////////////////////

  // System time structure used by W32GetSystemTime()

  typedef struct _W32SYSTEMTIME {   // w32st
    unsigned short wYear;
    unsigned short wMonth;
    unsigned short wDayOfWeek;
    unsigned short wDay;
    unsigned short wHour;
    unsigned short wMinute;
    unsigned short wSecond;
    unsigned short wMilliseconds;
  } W32SYSTEMTIME, * PW32SYSTEMTIME;

  // Memory status structure used by W32GlobalMemoryStatus()

  typedef struct _W32MEMORYSTATUS { // w32mst
    unsigned long dwLength;         // sizeof(MEMORYSTATUS)
    unsigned long dwMemoryLoad;     // percent of memory in use
    unsigned long dwTotalPhys;      // bytes of physical memory
    unsigned long dwAvailPhys;      // free physical memory bytes
    unsigned long dwTotalPageFile;  // bytes of paging file
    unsigned long dwAvailPageFile;  // free bytes of paging file
    unsigned long dwTotalVirtual;   // user bytes of address space
    unsigned long dwAvailVirtual;   // free user bytes
  } W32MEMORYSTATUS, * PW32MEMORYSTATUS;

  // Flags used by W32MessageBeep()

#define W32MB_WARNING               0x00000000L
#define W32MB_ERROR                 0x00000010L
#define W32MB_NOTE                  0x00000030L

/////////////////////////////////////////////////////////////////////////////
// H e l p e r   f u n c t i o n   p r o t o t y p e s                     //
/////////////////////////////////////////////////////////////////////////////

  // Miscellaneous routines

  void __stdcall W32Beep(unsigned long dwFreq, unsigned long dwDur);
  void __stdcall W32MessageBeep(unsigned int uType);
  void __stdcall W32Sleep(unsigned long dwMilliseconds);
  unsigned long __stdcall W32GetTickCount(void);
  void __stdcall W32GetSystemTime(PW32SYSTEMTIME pw32st);
  void __stdcall W32OutputDebugString(char * lpString);

  void __stdcall W32SetFileApisToOEM(VOID);
  void __stdcall W32SetFileApisToANSI(VOID);
  void __stdcall W32SetErrorMode(int fFailCriticalErrors);

  void __stdcall W32GlobalMemoryStatus(PW32MEMORYSTATUS pw32mst);

  // Console related routines

  int __stdcall W32SetConsoleTitle(char * pszTitle);
  int __stdcall W32GetConsoleTitle(char * pszTitle, int cchTitle);
  int __stdcall W32GetConsoleSize(short * pCX, short * pCY);
  int __stdcall W32GetCursorInfo(unsigned long * pdwSize, int * pbVisible);
  int __stdcall W32SetCursorInfo(unsigned long dwSize, int bVisible);
  int __stdcall W32GetCursorPos(short * pX, short * pY);
  int __stdcall W32SetCursorPos(short X, short Y);
  int __stdcall W32GetNumberOfConsoleMouseButtons(void);
  int __stdcall W32ShowVbuf(unsigned short * pvbuf, short CX, short CY);
  int __stdcall W32GrabVbuf(unsigned short * pvbuf, short CX, short CY);

  // File system related routines

  unsigned long __stdcall W32GetExeFileName(char * lpFileName, unsigned long nSize);
  void * __stdcall W32FindFirstFile(char * lpFileName, void * lpFindFileData);
  int __stdcall W32FindNextFile(void * hffile, void * lpFindFileData);
  int __stdcall W32FindClose(void * hffile);
  int __stdcall W32FileTimeToLocalFileTime(void * lpFileTime, void * lpLocalFileTime);

  int __stdcall W32FileTimeToDosDateTime(void * lpFileTime,
                                         unsigned short * lpFatDate,
                                         unsigned short * lpFatTime);

  unsigned long __stdcall W32GetFullPathName(char * lpFileName,
                                             unsigned long nBufferLength,
                                             char * lpBuffer);

  unsigned long __stdcall W32GetLogicalDrives(void);
  unsigned int __stdcall W32GetDriveType(char * lpRootDir);
  int __stdcall W32GetVolumeInformation(char * lpRootDir, char * lpLabel,
                                        char * lpFileSystem);

  // Clipboard related routines

  int __stdcall W32OpenClipboard(void);
  int __stdcall W32CloseClipboard(void);
  int __stdcall W32EmptyClipboard(void);
  int __stdcall W32SetClipboardData(int fmt, char * pb, int cb);
  void * __stdcall W32GetClipboardData(int fmt);
  int __stdcall W32GetClipboardDataSize(int fmt);

#endif /* DW_W32_DEFS */

#ifdef __cplusplus
}
#endif

/***************************************************************************
* End of DW-W32.H                                                          *
****************************************************************************/
