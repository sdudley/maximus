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
/*º                                                                        */
/*º         DW-TEXT: DOS Services Library                                  */
/*                    File name management routines                        */
/*                                                                         */
/*           Created: 17/Jun/92                                            */
/*           Updated: 10/Jan/99                                            */
/*                                                                         */
/*           Copyright (c) 1990-1999 JV DIALOGUE                           */
/*           Written by Pete I. Kvitek                                     */
/*                                                                         */
/***************************************************************************/

#ifdef __OS2__
#define INCL_NOCOMMON
#define INCL_NOPM
#define INCL_DOSMISC
#define INCL_DOSFILEMGR
#define INCL_DOSDEVIOCTL
#define INCL_DOSDEVICES
#define INCL_DOSERRORS
#include <os2.h>
#elif UNIX
#include <time.h>
#define FTIME time_t
#else
#include <dos.h>
#include <direct.h>
#endif

#include "dw-def.h"
#include "dw-str.h"
#ifdef __DOS__
//#include "dw-dos.h"
#endif
#ifdef __W32__
#include "dw-w32.h"
#endif

/////////////////////////////////////////////////////////////////////////////
// M o d u l e   s p e c i f i c   d e c l a r a t i o n s                 //
/////////////////////////////////////////////////////////////////////////////

  // Platform specific filefind structure

  typedef struct _FFIND {   // ffind
#if   defined(__OS2__)
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    USHORT attrFile;
    UCHAR  cchName;
    CHAR   achName[CCHMAXPATHCOMP];
    HDIR   hdirFFind;
#elif defined(__W32__)
    DWORD  dwFileAttributes;
    DWORD  ftCreationTime[2];
    DWORD  ftLastAccessTime[2];
    DWORD  ftLastWriteTime[2];
    DWORD  nFileSizeHigh;
    DWORD  nFileSizeLow;
    DWORD  dwReserved0;
    DWORD  dwReserved1;
    CHAR   cFileName[ 260 ];
    CHAR   cAlternateFileName[ 14 ];
    PVOID  hffind;
#else //      __DOS__
    BYTE   reserved[21];    
    BYTE   fattr;           
    FTIME  ftime;           
    FDATE  fdate;           
    ULONG  fsize;           
    CHAR   fname[13];       
#endif
  } FFIND, FAR * PFFIND;

  // Helper routine prototypes from DOS-HLP.ASM

#ifdef __DOS__
  BOOL HLPENTRY DosHlpFindFirst(PSZ pszPath, PFFIND pffind, USHORT attr);
  BOOL HLPENTRY DosHlpFindNext(PFFIND pffind);
#endif

/////////////////////////////////////////////////////////////////////////////
// M i s c e l l a n e o u s   s u b r o u t i n e s                       //
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************
* This subroutine converts string to upper case
*/

  static VOID SUBENTRY DoStrUpr(PSZ psz)
  {
    for (; *psz; psz++) *psz = TOUPPER(*psz);
  }

/***************************************************************************
* This subroutine performs a case insensitive string comparison
*/

  static BOOL SUBENTRY DoStrCmpI(PSZ psz1, PSZ psz2)
  {
    while (*psz1 && *psz2 && (TOUPPER(*psz1++) == TOUPPER(*psz2++)));
    return *psz1 - *psz2;
  }

/***************************************************************************
* This subroutine checks if the specified file name is a device name
*/

  static BOOL SUBENTRY DoCheckDevName(PSZ pszFileName)
  {
    USHORT iDevName;

    static PSZ apszDevName[] = {
#ifdef __DOS__
      "nul",  "con",  "aux",  "prn",
      "com1", "com2", "com3", "com4",
      "lpt1", "lpt2", "lpt3", "lpt4",
#endif

#ifdef __OS2__
      "clock$", "kbd$", "mouse$", "pointer$", "screen$",
      "com1", "com2", "com3", "com4",
      "lpt1", "lpt2", "lpt3",
      "con", "nul", "prn",
#endif

#ifdef __W32__
      "con", "nul", "prn",
#endif
    };

    // Scan through all the devices table items looking for a match

    for (iDevName = 0; iDevName < numbof(apszDevName); iDevName++)
      if (!DoStrCmpI(apszDevName[iDevName], pszFileName))
        return TRUE;

    return FALSE;
  }

/***************************************************************************
* This subroutine copies one string to another, returns past the end of dest
*/

  static PCH SUBENTRY DoCopyStr(PSZ pszDest, PSZ pszSrc)
  {
    USHORT cch = xstrlen(pszSrc);

    xmemcpy(pszDest, pszSrc, cch + 1);

    return pszDest + cch;
  }

/***************************************************************************
* This subroutine copies one string to another
*/

  static VOID SUBENTRY DoCopyStrMax(PSZ pszDest, PSZ pszSrc, USHORT cchMax)
  {
    if (pszDest == NULL)
      return;

    if(xstrlen(pszSrc) >= cchMax) {
      xmemcpy(pszDest, pszSrc, cchMax);
      pszDest[cchMax] = 0;
    }
    else
      xstrcpy(pszDest, pszSrc);
  }

/***************************************************************************
* This subroutine checks for the directory spec character at the left
*/

  static BOOL SUBENTRY DoCheckDir(PCH pch)
  {
    // Check for double things like in \..

    if (*(pch - 1) == '.')
      pch--;

    // Check for the directory character. Note that leading zero

    switch (*--pch) {
#ifndef UNIX
      case ':' : if (*(pch - 2) != '\0')
                   return FALSE;
#endif
//      case '/' :
//      case '\\':
      case PATH_DELIM:
      case '\0': break;
    }

    return TRUE;
  }

/***************************************************************************
* This subroutine makes a full path using a DOS service
*/
#ifdef __DOS__
  static BOOL SUBENTRY DoBuildFullPath(PSZ pszDest, PSZ pszSrc)
  {
    struct SREGS sregs;
    union REGS regs;

    regs.h.ah = 0x60;
    sregs.ds  = FP_SEG(pszSrc);  regs.x.si = FP_OFF(pszSrc);    // ds:si= src
    sregs.es  = FP_SEG(pszDest); regs.x.di = FP_OFF(pszDest);   // es:di= dst
    intdosx(&regs, &regs, &sregs);

    return !regs.x.cflag;
  }
#endif
/***************************************************************************
* This subroutine retrieves application executable file name
*/

  static BOOL SUBENTRY DoQueryExePath(PSZ pszDest)
  {
    CHAR achDrive[FN_MAXDRIVE], achDir[FN_MAXDIR];

#ifdef __DOS__
    union REGS regs;
    PSZ pch;

    // Get psp segment address using dos services in bx

    regs.h.ah = 0x62; intdos(&regs, &regs);

    // Get the environment block address and scan through the
    // regular environment strings until the end reached

    pch = MAKEP(*((PUSHORT) MAKEP(regs.x.bx, 0x2c)), 0);
    while(*pch++ || *pch);

    // Check if there is an executable path appended to the
    // end of the environment string pool

    if (*((PUSHORT)(pch + 1)) == 0)
      return FALSE;

    // Copy the executable file name into the callers buffer

    xstrcpy(pszDest, pch + 3);
#endif

#ifdef __OS2__
    USHORT offCmd;              // command line offset -- dummy
    PCH pch = NULL;             // pointer to the name of executable file

    // Get the address of the environment

    if (DosGetEnv((PUSHORT)&pch + 1, &offCmd))
      return FALSE;

    // Adjust forward until we point to full path of exe file

    while(*pch++ || *pch);

    // Copy the executable file name into the callers buffer

    xstrcpy(pszDest, pch + 1);
#endif

#ifdef __W32__
    W32GetExeFileName(pszDest, FN_MAXPATH);
#endif

    // Normally we'll have the full exe path by now, however on some
    // networked drives environment path is relative. So split the exe
    // file name and check if it's UNC file name

    if (DosSplitFileName(pszDest, achDrive, achDir, NULL, NULL) & FN_UNC)
      return TRUE;

    // Check if drive and absolute directory specification

//    if (achDrive[0] && achDir[0] == '\\')
    if (achDrive[0] && achDir[0] == PATH_DELIM)
      return TRUE;

    // Build full path and check if ok

    return DosBuildFullPath(pszDest, pszDest);
  }

/***************************************************************************
* This subroutine makes the caller's file find data from the internal one
*/

  static VOID SUBENTRY DoFFblkFromFFind(PFFBLK pffblk, PFFIND pffind)
  {
#ifdef __DOS__

    // Copy file size and attributes
    
    pffblk->fsize = pffind->fsize;
    pffblk->fattr = pffind->fattr;

    // Copy file name    
    
    xstrncpy(pffblk->fname, pffind->fname, lengof(pffblk->fname));

    // Copy file modification time and date

    pffblk->ftime = pffind->ftime;
    pffblk->fdate = pffind->fdate;

#endif

#ifdef __OS2__

    // Copy file size and attributes
    
    pffblk->fsize = pffind->cbFile;
    pffblk->fattr = pffind->attrFile;

    // Copy file name    
    
    xstrncpy(pffblk->fname, pffind->achName, lengof(pffblk->fname));

    // Copy file modification time and date

    pffblk->ftime = pffind->ftimeLastWrite;
    pffblk->fdate = pffind->fdateLastWrite;

#endif

#ifdef __W32__
    DWORD ftLastWriteTime[2];
    
    // Copy file size and attributes
    
    pffblk->fsize = pffind->nFileSizeLow;
    pffblk->fattr = (USHORT) pffind->dwFileAttributes;

    // Copy file name    
    
    xstrncpy(pffblk->fname, pffind->cFileName, lengof(pffblk->fname));

    // Convert file time to local file time, then to dos file time and date

    W32FileTimeToLocalFileTime(pffind->ftLastWriteTime, ftLastWriteTime);
    W32FileTimeToDosDateTime(ftLastWriteTime, 
                            (unsigned short *)&pffblk->fdate, 
                            (unsigned short *)&pffblk->ftime);
#endif
  }

/////////////////////////////////////////////////////////////////////////////
// P u b l i c   r o u t i n e s                                           //
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************
* This routine makes a file name from its components
*/

  VOID APIENTRY DosMergeFileName(PSZ pszPath, PSZ pszDrive, PSZ pszDir,
                                 PSZ pszFile, PSZ pszExt)
  {
    // Copy the drive if any

    if (pszDrive && *pszDrive) {
      *pszPath++ = *pszDrive++;
      *pszPath++ = ':';
    }

    // Copy the directory if any and append trailing back slash if
    // it's not already there

    if (pszDir && *pszDir) {
      pszPath = DoCopyStr(pszPath, pszDir);
//      if (*(pszPath - 1) != '\\' && *(pszPath - 1) != '/')
      if (*(pszPath - 1) != PATH_DELIM )
//        *pszPath++ = '\\';
        *pszPath++ = PATH_DELIM;
    }

    // Copy the file name and extension if any

    if (pszFile) pszPath = DoCopyStr(pszPath, pszFile);
    if (pszExt)  pszPath = DoCopyStr(pszPath, pszExt);

    // Fix the trailing null

    *pszPath = 0;
  }

/***************************************************************************
* This routine splits a file name to its components
*/

  USHORT APIENTRY DosSplitFileName(PSZ pszPath, PSZ pszDrive, PSZ pszDir,
                                   PSZ pszFile, PSZ pszExt)
  {
    CHAR ach[FN_MAXPATH + 2];
    USHORT cch, iDir, fs = 0;
    PCH pch;

    // Initialize all the supplied strings to default values

    if (pszDrive) *pszDrive = '\0';
    if (pszDir)   *pszDir   = '\0';
    if (pszFile)  *pszFile  = '\0';
    if (pszExt)   *pszExt   = '\0';

    // Skip over the leading spaces if any

    while (*pszPath == ' ')
      pszPath++;

    // Limit the length of the file name

    if ((cch = xstrlen(pszPath)) > FN_MAXPATH)
      cch = FN_MAXPATH;

    // Set in the leading zero terminator and copy the file name past it

    pch = ach; *pch++ = 0;
    xmemcpy(pch, pszPath, cch);
    *(pch += cch) = 0;

    // Split the filename and fill corresponding nonzero pointers

    iDir = 0;
    loop {
      switch (*--pch) {
        case '.'  : // If not dir and if this is a dot at the end check
                    // if we got a dir char at the left. This is important
                    // for README. specification

                    if (!iDir && (*(pch + 1) == '\0'))
                      iDir = DoCheckDir(pch) ? 0 : 1;

                    // If not dir and if we don't have the extension yet
                    // set it in and cut it off

                    if (!iDir && ((fs & FN_EXTENSION) == 0)) {
                      fs |= FN_EXTENSION;
                      DoCopyStrMax(pszExt, pch, FN_MAXEXT - 1);
                      *pch = 0;
                    }
                    continue;

        case ':'  : // Check if this a second char in the spec and
                    // loop it if not thus skipping the inbetween colons

                    if (pch != &ach[2])
                      continue;
                    else
                      ; // fall through

        case '\0' : // If we have a dir and there is something
                    // past here and set it in then cut it off

                    if (iDir) {
                      if (*++pch) fs |= FN_DIRECTORY;
                      DoCopyStrMax(pszDir, pch, FN_MAXDIR - 1);
//                      if (iDir > 2 && pch[0] == '\\' && pch[1] == '\\') fs|= FN_UNC;
                      if (iDir > 2 && pch[0] == PATH_DELIM && pch[1] == PATH_DELIM) fs|= FN_UNC;
                      *pch-- = 0;
                      break;
                    } else
                      ; // fall through

//        case '/'  : // If we don't have a dir yet and there is something
//        case '\\' : // past here set the file in and cut it off. Then check
        case PATH_DELIM : 
                    // for the line beginning or drive delimiter

                    if (!iDir) {
                      iDir++;
                      if (*++pch) fs |= FN_FILENAME;
                      DoCopyStrMax(pszFile, pch, FN_MAXFILE - 1);
                      *pch-- = 0;
                      if (*pch == 0 || (*pch == ':' && pch == &ach[2]))
                        break;
                    } else
                      iDir++;
                    continue;

        case '*'  : // If not dir then set the wildcard flag
        case '?'  : if (!iDir) fs |= FN_WILDCARDS;

        default   : continue;
      }
      break;
    }

    // Check if we ended up at the drive spec and set it in

    if (*pch == ':') {
      if (ach[1]) fs |= FN_DRIVE;
      DoCopyStrMax(pszDrive, &ach[1], FN_MAXDRIVE - 1);
    }

    return fs;
  }

/////////////////////////////////////////////////////////////////////////////
// Logical disk drive related functions                                    //
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************
* This routine sets the current directory on the specified drive
*/

  BOOL APIENTRY DosSetCurDir(PSZ pszPath)
  {
#ifdef __DOS__
    struct SREGS sregs;
    union REGS regs;

    regs.h.ah = 0x3b;
    regs.x.dx = FP_OFF(pszPath);
    sregs.ds  = FP_SEG(pszPath);
    sregs.es  = sregs.ds;       // load valid selector for protected mode
    intdosx(&regs, &regs, &sregs);

    return !regs.x.cflag;
#endif

#ifdef __OS2__
    return DosChDir(pszPath, 0) ? FALSE : TRUE;
#endif

#ifdef __W32__
    return chdir(pszPath) ? FALSE : TRUE;
#endif
  }

/***************************************************************************
* This routine returns the current directory on the specified drive
*/

  BOOL APIENTRY DosQueryCurDir(USHORT iDrive, PSZ pszPath)
  {
#ifdef __DOS__
    struct SREGS sregs;
    union REGS regs;

//    *pszPath++ = '\\';
    *pszPath++ = PATH_DELIM;

    regs.h.ah = 0x47;
    regs.h.dl = LOBYTE(iDrive);
    regs.x.si = FP_OFF(pszPath);
    sregs.ds  = FP_SEG(pszPath);
    sregs.es  = sregs.ds;       // load valid selector for protected mode
    intdosx(&regs, &regs, &sregs);

    return !regs.x.cflag;
#endif

#ifdef __OS2__
    USHORT cbPath = FN_MAXPATH - 1;

//    *pszPath++ = '\\';
    *pszPath++ = PATH_DELIM;

    return DosQCurDir(iDrive, pszPath, &cbPath) ? FALSE : TRUE;
#endif

#ifdef __W32__
    CHAR achPath[FN_MAXPATH];

    // Get current working directory and check if ok
    
    if (!_getdcwd(iDrive, achPath, FN_MAXPATH)) {
//      xstrcpy(pszPath, "\\");
      xstrcpy(pszPath, PATH_DELIMS);
      return FALSE;
    }

    // getcwd returns working directory with the drive specification
    // so copy the directory past it

    xstrcpy(pszPath, achPath + 2);

    return TRUE;
#endif
  }

/***************************************************************************
* This routine sets the current drive given its index
*/

  BOOL APIENTRY DosSetCurDrive(USHORT iDrive)
  {
#ifdef __DOS__
    union REGS regs;

    regs.h.ah = 0x0e;
    regs.h.dl = LOBYTE(iDrive - 1);
    intdos(&regs, &regs);

    return DosQueryCurDrive() == iDrive;
#endif

#ifdef __OS2__
    return DosSelectDisk(iDrive) ? FALSE : TRUE;
#endif

#ifdef __W32__
    return _chdrive(iDrive) ? FALSE : TRUE;
#endif
  }

/***************************************************************************
* This routine returns the current drive index
*/

  USHORT APIENTRY DosQueryCurDrive(VOID)
  {
#ifdef __DOS__
    union REGS regs;

    regs.h.ah = 0x19;
    intdos(&regs, &regs);

    return regs.h.al + 1;
#endif

#ifdef __OS2__
    USHORT iDrive;
    ULONG flDriveMap;

    return DosQCurDisk(&iDrive, &flDriveMap) ? FALSE : iDrive;
#endif

#ifdef __W32__
    return _getdrive(); 
#endif
  }

/***************************************************************************
* This routine returns number of drives in system
*/

  USHORT APIENTRY DosQueryDriveCount(VOID)
  {
#ifdef __DOS__
    union REGS regs;

    regs.h.ah = 0x0e;
    regs.h.dl = DosQueryCurDrive();
    intdos(&regs, &regs);

    return regs.h.al;
#endif

#ifdef __OS2__
    return (USHORT)('Z' - 'A' + 1);
#endif

#ifdef __W32__
    return (USHORT)('Z' - 'A' + 1);
#endif
  }


/***************************************************************************
* This routine returns logical drives map
*/

  APIDECL ULONG APIENTRY DosQueryLogicalDrives(VOID)
  {
    // The return value is a bitmask representing the currently available 
    // disk drives. Bit position 0 (the least-significant bit) is drive A, 
    // bit position 1 is drive B, bit position 2 is drive C, and so on.   
  
#ifdef __DOS__
    ULONG flDriveMap, flDriveMask;
    USHORT iCurDrive, iDrive;
    
    // Preserve current drive

    iCurDrive = DosQueryCurDrive();

    // Loop through all possible drives tryingto set the current drive to 
    // the one under question and check if successful

    for (iDrive = 0, flDriveMap = 0, flDriveMask = 1; 
         iDrive < (USHORT)('Z' - 'A' + 1);
         iDrive++, flDriveMask<<= 1) {
      if (DosSetCurDrive(iDrive + 1)) 
        flDriveMap|= flDriveMask;
    }

    // Restore current drive

    DosSetCurDrive(iCurDrive);

    return flDriveMap;
#endif

#ifdef __OS2__
    USHORT iDrive;
    ULONG flDriveMap;

    // Call the service and check if ok
    
    if (DosQCurDisk(&iDrive, &flDriveMap))
      return 0L;

    return flDriveMap;
#endif

#ifdef __W32__
    return W32GetLogicalDrives();
#endif
  }
  
/***************************************************************************
* This routine returns logical drive info
*/

  APIDECL USHORT APIENTRY DosQueryDriveInfo(USHORT iDrive, PSZ pszLabel, 
                                            PSZ pszText)
  {
#ifdef __DOS__
    struct SREGS sregs;
    union REGS regs;
    CHAR achNet[128];
    CHAR achDev[16];
    USHORT idev;
    FFBLK ffblk;
    PCH pch;

    // Check if device is remote or local

    regs.x.ax = 0x4409;
    regs.h.bl = iDrive;
    intdos(&regs, &regs);

    if (regs.x.cflag)
      return DRIVE_UNKNOWN;

    // If it's remote then try to get the \\server\share name by
    // scanning through the network redirector list

    if (regs.x.dx & 0x1000) {
      for (idev = 0; ; idev++) {
        regs.x.ax = 0x5f02;
        regs.x.bx = idev;
        sregs.ds  = FP_SEG(achDev); regs.x.si = FP_OFF(achDev); // ds:si= src
        sregs.es  = FP_SEG(achNet); regs.x.di = FP_OFF(achNet); // es:di= dst
        intdosx(&regs, &regs, &sregs);
        if (regs.x.cflag)
          break;

        // Check if this is a remote disk which local name matches our drive

        if (regs.h.bl == 0x04 && achDev[0] == 'A' + iDrive - 1) {
          if (pszText) xstrcpy(pszText, achNet);
          break;
        }
      }

      // CD-ROM drives also fall into this cathegory so filter them out
      
      regs.x.ax = 0x150b;
      regs.x.cx = iDrive - 1;
      int86(0x2f, &regs, &regs);

      return !regs.x.ax ? DRIVE_REMOTE : DRIVE_CDROM;
    }

    // Check if device is removable media

    regs.x.ax = 0x4408;
    regs.h.bl = iDrive;
    intdos(&regs, &regs);
    if (regs.x.cflag)
      return DRIVE_UNKNOWN;
    if (regs.h.al == 0x00)
      return DRIVE_REMOVABLE;
    if (regs.h.al != 0x01)
      return DRIVE_UNKNOWN;

    // For fixed disk drives attempt to read the volume label,
    // and if one exists, remove the delemiter dot if any

    if (DosFindFirstFile("\\*.*", &ffblk, FA_LABEL)) {
      if ((pch = xstrchr(ffblk.fname, '.')) != NULL)
        xstrcpy(pch, pch + 1);
      if (pszLabel) xstrcpy(pszLabel, ffblk.fname);
      DosFindFileClose(&ffblk);
    }

    // Set in the file system type 

    if (pszText) xstrcpy(pszText, "");

    return DRIVE_FIXED;
#endif

#ifdef __OS2__
    static CHAR achDrive[] = "C:";
    struct {USHORT iType; USHORT cbName; CHAR ach[60];} info;
    BYTE fNonRemovable = TRUE, cmd = 0;
    USHORT code, cb = sizeof(info);
    FSINFO fsinfo;
    HFILE hfile;
    PBYTE pb;

    // Query attached file system for the specified drive

    achDrive[0] = 'A' + iDrive - 1; xmemset(&info, 0, sizeof(info));
    if (DosQFSAttach(achDrive, 0, FSAIL_QUERYNAME, (PBYTE) &info, &cb, 0l))
      return DRIVE_UNKNOWN;

    // Check if the drive is remote or local

    if (info.iType == FSAT_REMOTEDRV) {
      pb = (PBYTE)(&info) + 2 * sizeof(USHORT) + info.cbName + 1;
      pb+= sizeof(USHORT) + *((PUSHORT) pb) + 1;
      if (*((PUSHORT) pb) > 0) 
        if (pszText) xstrcpy(pszText, pb + sizeof(USHORT));
      return DRIVE_REMOTE;
    } else
    if (info.iType != FSAT_LOCALDRV)
      return DRIVE_UNKNOWN;

    // Now open the device and check if it's removable

    if (!DosOpen(achDrive, &hfile, &code, 0l, 0, FILE_OPEN,
                 OPEN_ACCESS_READONLY  | OPEN_SHARE_DENYNONE |
                 OPEN_FLAGS_DASD, 0l)) {
      if (DosDevIOCtl(&fNonRemovable, &cmd, DSK_BLOCKREMOVABLE,
                      IOCTL_DISK, hfile) == ERROR_NOT_READY) fNonRemovable = 0;
      DosClose(hfile);
    }

    // Check if drive is removable 

    if (!fNonRemovable)
      return DRIVE_REMOVABLE;

    // For fixed disk drives attempt to read the volume label

    if (!DosQFSInfo(iDrive, FSIL_VOLSER, (PBYTE) &fsinfo, sizeof(fsinfo)) && 
        fsinfo.vol.cch)
      if (pszLabel) xstrcpy(pszLabel, fsinfo.vol.szVolLabel);

    return DRIVE_FIXED;
#endif

#ifdef __W32__
    static CHAR achRootDir[] = "A:\\";
    CHAR achLabel[FN_MAXPATH] = "";
    CHAR achFileSystem[FN_MAXPATH] = "";
    PSZ pszRootDir;
    USHORT type;

    // Make up the root directory specification. Zero drive index specifies
    // root of the current directory

    if (iDrive) {
      achRootDir[0] = (CHAR)('A' + iDrive - 1);
      pszRootDir = achRootDir;
    } else {
      pszRootDir = NULL;
    }

    // Get the drive type 

    type = W32GetDriveType(pszRootDir);

    // Get the file info for certain drive types

    if (type == DRIVE_FIXED || type == DRIVE_REMOTE ||
        type == DRIVE_CDROM || type == DRIVE_RAMDISK)
      W32GetVolumeInformation(pszRootDir, achLabel, achFileSystem);

    // Copy the info strings

    if (pszLabel) xstrcpy(pszLabel, achLabel);
    if (pszText) xstrcpy(pszText, achFileSystem);
    
    return type;
#endif
  }

/////////////////////////////////////////////////////////////////////////////
// File related functions                                                  //
/////////////////////////////////////////////////////////////////////////////

/***************************************************************************
* This routine builds a complete path name for the specified file
*/

  BOOL APIENTRY DosBuildFullPath(PSZ pszDest, PSZ pszSrc)
  {
#ifdef __DOS__
    CHAR achDrive[FN_MAXDRIVE];
    CHAR achDir[FN_MAXDIR];
    CHAR achFile[FN_MAXFILE];
    CHAR achExt[FN_MAXEXT];
    CHAR achPath[FN_MAXDIR];
    USHORT iDrive, fs;
    BOOL fOk = FALSE;

    // Convert to upper case and decompose
    // the supplied path into it's components

    DoStrUpr(pszSrc);
    fs = DosSplitFileName(pszSrc, achDrive, achDir, achFile, achExt);

    // Check if this is a device name and do nothing if so

    if (DoCheckDevName(achFile)) {
      xstrcpy(pszDest, achFile);
      return TRUE;
    }

    // Check if this is a UNC file name and build the full path
    // using system services. Note that since dos replaces * wildcards
    // with '?' wildcards we split/merge the path specification once
    // again to place file/ext with original wildcards back.

    if ((fs & FN_UNC) != 0) {
      DosMergeFileName(achPath, achDrive, achDir, achFile, achExt);
      if (!DoBuildFullPath(pszDest, achPath))
        return FALSE;
      if (fs & FN_WILDCARDS) {
        DosSplitFileName(pszDest, achDrive, achDir, NULL, NULL);
        DosMergeFileName(pszDest, achDrive, achDir, achFile, achExt);
      }
      return TRUE;
    }

    // Get the current drive index

    iDrive = DosQueryCurDrive();

    // Check if drive is specified in the supplied path and if not,
    // assume the current one

    if (!achDrive[0]) {
      xstrcpy(achDrive, "A:");
      achDrive[0]+= (CHAR) iDrive - 1;
    }

    // Set the current drive to the requested one and check if ok

    if (!DosSetCurDrive(TOUPPER(achDrive[0]) - 'A' + 1))
      return FALSE;

    // Preserve the current directory on the reqested drive and
    // check if ok, otherwise restore initial current drive and return

    if (!DosQueryCurDir(0, achPath)) {
      DosSetCurDrive(iDrive);
      return FALSE;
    }

    // Check if directory specified and make it current

    if (achDir[0]) {

      // Kill trailing back slash if it's not the only character
      // of the directory specification

      if ((achDir[xstrlen(achDir) - 1] == '\\') && (xstrlen(achDir) > 1 ))
        achDir[xstrlen(achDir) - 1] = '\0';

      // Change to the specified directory and check if ok. If failed,
      // restore the inital directory on the requested drive and change
      // to the initial drive

      if (!DosSetCurDir(achDir))
        goto Out;
    }

    // So we managed to make a requested directory current on the
    // requested drive. Now get it's full specification and this
    // will be what we're after. If failed, just restore things back

    if (!DosQueryCurDir(0, achDir))
      goto Out;

    // Compose the fully qualified file name and restore
    // the initial directory on the requested drive and change
    // to the initial drive

    DosMergeFileName(pszDest, achDrive, achDir, achFile, achExt);
    fOk = TRUE;

Out:DosSetCurDir(achPath); DosSetCurDrive(iDrive);

    return fOk;
#endif

#if define(__OS2__) || define(UNIX)
    USHORT cch = xstrlen(pszSrc);
    CHAR achPath[FN_MAXPATH];
    BOOL fOk;

    // OS/2 service does not allow trailing backslashes when specifying
    // directory names unless at the root, so check if it is not there
    // and call service

# ifdef OS2
    if (cch == 0 || pszSrc[cch - 1] != '\\' ||
       (cch > 2 && pszSrc[cch - 2] == ':')) {
# else // UNIX
    if (cch == 0 || pszSrc[cch - 1] != PATH_DELIM ) {
#endif
      fOk = !DosQPathInfo(pszSrc, FIL_QUERYFULLNAME, pszDest, FN_MAXPATH, 0);
    } else {

      // Remove the trailing backslash, then call the service placing the
      // result into the local storage and put trailing backslash back

      pszSrc[cch - 1] = '\0';
      fOk = !DosQPathInfo(pszSrc, FIL_QUERYFULLNAME, achPath, FN_MAXPATH, 0);
//      pszSrc[cch - 1] = '\\';
      pszSrc[cch - 1] = PATH_DELIM;

      // Check if ok and if so, copy the result into the destination buffer
      // and append the trailing backslash

      if (fOk) {
        xstrcpy(pszDest, achPath);
//        xstrcat(pszDest, "\\");
        xstrcat(pszDest, PATH_DELIMS);
      }
    }

    return fOk;
#endif

#ifdef __W32__
    CHAR achPath[FN_MAXPATH];
    
    if (!W32GetFullPathName(pszSrc, lengof(achPath), achPath))
      return FALSE;

    xstrcpy(pszDest, achPath);
    
    return TRUE;
#endif
  }

/***************************************************************************
* This routine retrieves application executable file name
*/

  BOOL APIENTRY DosQueryExePath(PSZ pszDest)
  {
    static CHAR achPath[FN_MAXPATH];

    // Check if we don't have the executable file name query it

    if (!achPath[0] && !DoQueryExePath(achPath)) {
      achPath[0] = '\0';
      return FALSE;
    }

    // Copy executable file name into the callers buffer

    if (pszDest != NULL) xstrcpy(pszDest, achPath);

    return TRUE;
  }

/***************************************************************************
* This routine opens a stream in a given sharing mode
*/

// 
// Win32 port: sharing flags mix with new file mode flag O_TEMPORARY
//

#ifndef __W32__
  FILE * APIENTRY DosOpenFile(PSZ pszFile, PSZ pszType, USHORT fs)
  {
    USHORT fsMode = 0, fsOpen = fs;
    PCH pch = pszType;
    FILE * pfile;
    INT hfile;
    CHAR ch;

    ASSERT(pszFile != NULL);
    ASSERT(pszType != NULL);

    // Reset flags we're going to set according to the 'pszType'

    fsOpen&= ~(O_RDONLY | O_WRONLY | O_RDWR |
               O_CREAT | O_TRUNC | O_APPEND |
               O_TEXT | O_BINARY);

    // Scan the mode string and set the appropriate open flags

    ch = *pch++;
    if (ch == 'r') {
      fsOpen|= O_RDONLY;
      fsMode = 0;
    } else
    if (ch == 'w') {
      fsOpen|= O_WRONLY | O_CREAT | O_TRUNC;
      fsMode = S_IWRITE;
    } else
    if (ch == 'a') {
      fsOpen|= O_WRONLY | O_CREAT | O_APPEND;
      fsMode = S_IWRITE;
    } else
      return NULL;

    ch = *pch++;
    if (ch == '+' || (*pch == '+' && (ch == 't' || ch == 'b'))) {
      if (ch == '+') ch = *pch;
      fsOpen = (fsOpen & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
      fsMode = S_IREAD | S_IWRITE;
    }

    if (ch == 't')
      fsOpen|= O_TEXT;
    else
    if (ch == 'b')
      fsOpen|= O_BINARY;
    else
      fsOpen|= _fmode & (O_TEXT | O_BINARY);

    // Open file and check if ok

    if ((hfile = open(pszFile, fsOpen, fsMode)) == -1)
      return NULL;

    // Associate a stream with the file just opened and check if ok

    if ((pfile = fdopen(hfile, pszType)) == NULL) {
      close(hfile);
      return NULL;
    }

    // Return the pointer to stream

    return pfile;
  }
#else
  FILE * APIENTRY DosOpenFile(PSZ pszFile, PSZ pszType, USHORT fs)
  {
    USHORT fsOpen, fsMode;
    PCH pch = pszType;
    FILE * pfile;
    INT hfile;
    CHAR ch;

    ASSERT(pszFile != NULL);
    ASSERT(pszType != NULL);

    // Scan the mode string and set the appropriate open flags

    ch = *pch++;
    if (ch == 'r') {
      fsOpen = O_RDONLY;
      fsMode = 0;
    } else
    if (ch == 'w') {
      fsOpen = O_WRONLY | O_CREAT | O_TRUNC;
      fsMode = S_IWRITE;
    } else
    if (ch == 'a') {
      fsOpen = O_WRONLY | O_CREAT | O_APPEND;
      fsMode = S_IWRITE;
    } else
      return NULL;

    ch = *pch++;
    if (ch == '+' || (*pch == '+' && (ch == 't' || ch == 'b'))) {
      if (ch == '+') ch = *pch;
      fsOpen = (fsOpen & ~(O_RDONLY | O_WRONLY)) | O_RDWR;
      fsMode = S_IREAD | S_IWRITE;
    }

    if (ch == 't')
      fsOpen|= O_TEXT;
    else
    if (ch == 'b')
      fsOpen|= O_BINARY;
    else
      fsOpen|= _fmode & (O_TEXT | O_BINARY);

    // Open file and check if ok

    if ((hfile = sopen(pszFile, fsOpen, fs, fsMode)) == -1)
      return NULL;

    // Associate a stream with the file just opened and check if ok

    if ((pfile = fdopen(hfile, pszType)) == NULL) {
      close(hfile);
      return NULL;
    }

    // Return the pointer to stream

    return pfile;
  }
#endif
/***************************************************************************
* This routine initiates a file directory search
*/

  BOOL APIENTRY DosFindFirstFile(PSZ pszPath, PFFBLK pffblk, USHORT fs)
  {
    PFFIND pffind;

    // Clear the caller's structure

    xmemset(pffblk, 0, sizeof(FFBLK));

    // Allocate the find file structure and check if ok

    if ((pffind = MemAlloc(sizeof(FFIND), MA_CLEAR)) == NULL)
      return FALSE;

#ifdef __DOS__

    // Call the service and check if ok

    if (!DosHlpFindFirst(pszPath, pffind, fs)) {
      pffblk->pffind = NULL;
      MemFree(pffind);
      return FALSE;
    }
#endif

#ifdef __OS2__
{   USHORT cSearch = 1;

    // Create the new file find handle 

    pffind->hdirFFind = HDIR_CREATE;

    // Call the service and check if ok

    if (DosFindFirst(pszPath, &pffind->hdirFFind, fs, 
                    (PFILEFINDBUF)pffind, sizeof(FILEFINDBUF),
                    &cSearch, 0)) {
      pffind->hdirFFind = HDIR_CREATE;
      pffblk->pffind = NULL;
      MemFree(pffind);
      return FALSE;
    }
}
#endif

#ifdef __W32__
    
    // Call the wrapper routine and check if ok
    
    if ((pffind->hffind = W32FindFirstFile(pszPath, pffind)) == NULL) {
      pffblk->pffind = NULL;
      MemFree(pffind);
      return FALSE;
    } 
#endif
    
    // Set the file find control block into the caller's structure
    
    pffblk->pffind = pffind;

    // Make up the file info in the caller's structure

    DoFFblkFromFFind(pffblk, pffind);

    return TRUE;
  }

/***************************************************************************
* This routine continues a file directory search
*/

  BOOL APIENTRY DosFindNextFile(PFFBLK pffblk)
  {
    PFFIND pffind;

    // Check if we have the file find data block allocated

    if ((pffind = pffblk->pffind) == NULL)
      return FALSE;
    
#ifdef __DOS__

    // Call the service and check if ok

    if (!DosHlpFindNext(pffind)) 
      return FALSE;

#endif

#ifdef __OS2__
{   USHORT cSearch = 1;

    // Check if we have directory search handle

    if (pffind->hdirFFind == HDIR_CREATE) 
      return FALSE;

      // Call the service ad check if ok

    if (DosFindNext(pffind->hdirFFind,
                   (PFILEFINDBUF)pffind, sizeof(FILEFINDBUF),
                   &cSearch))
      return FALSE;
}
#endif

#ifdef __W32__

    // Check if we have the file find handle open 
    
    if (pffind->hffind == NULL) 
      return FALSE;

    // Call the wrapper routine and check if ok

    if (!W32FindNextFile(pffind->hffind, pffblk->pffind))
      return FALSE;

#endif

    // Make up the file info in the caller's structure

    DoFFblkFromFFind(pffblk, pffind);
    
    return TRUE;
  }

/***************************************************************************
* This routine closes a file directory search
*/

  APIDECL BOOL APIENTRY DosFindFileClose(PFFBLK pffblk)
  {
    PFFIND pffind;

    // Check if we have the file find data block allocated

    if ((pffind = pffblk->pffind) == NULL)
      return FALSE;
    
#ifdef __DOS__
    (void) pffind;
#endif

#ifdef __OS2__
    
    // Check if we have directory search handle

    if (pffind->hdirFFind != HDIR_CREATE) {
      DosFindClose(pffind->hdirFFind);
      pffind->hdirFFind = HDIR_CREATE;
    }
    
    return FALSE;
#endif

#ifdef __W32__

    // Check if we have the file find handle open and close it
    
    if (pffind->hffind != NULL) {
      W32FindClose(pffind->hffind);
      pffind->hffind = NULL;
    }
#endif

    // Free the file find data block allocated

    MemFree(pffblk->pffind);
    pffblk->pffind = NULL;

    return TRUE;
  }
  
/***************************************************************************
* This routine returns the given path information
*/

  APIDECL BOOL APIENTRY DosQueryPathInfo(PSZ pszPath, PFFBLK pffblk)
  {
#ifndef __OS2__
    FFBLK ffblk;
#else
    FILESTATUS fsts;
#endif

    // Clear the caller's structure

    xmemset(pffblk, 0, sizeof(FFBLK));

    // Make up the file info structure 

#ifndef __OS2__
    if (DosFindFirstFile(pszPath, &ffblk, FA_STANDARD | FA_DIRECTORY)) {
      pffblk->fattr = ffblk.fattr;
      pffblk->fdate = ffblk.fdate;
      pffblk->ftime = ffblk.ftime;
      pffblk->fsize = ffblk.fsize;
      DosFindFileClose(&ffblk);
    } else
      return FALSE;
#else
    if (!DosQPathInfo(pszPath, FIL_STANDARD, &fsts, sizeof(fsts), 0)) {
      pffblk->fattr = fsts.attrFile;
      pffblk->fdate = fsts.fdateLastWrite;
      pffblk->ftime = fsts.ftimeLastWrite;
      pffblk->fsize = fsts.cbFile;
    } else
      return FALSE;
#endif

    // Copy the file path 

    xstrcpy(pffblk->fname, pszPath);

    return TRUE;
  }

/***************************************************************************
* End of DOS-FILE.C                                                        *
****************************************************************************/
