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

#include <dos.h>
#include <io.h>
#include "prog.h"

#if defined(OS_2) && defined(__FLAT__) /* 32-bit OS/2 */

  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  int _fast get_fdt(int handle, union stamp_combo *filestamp)
  {
    FILESTATUS3 fs3;
    int stat;

    stat = DosQueryFileInfo(handle, FIL_STANDARD, (void *)&fs3, sizeof fs3);

    if (stat==0)
    {
      filestamp->dos_st.date = *(USHORT *)&fs3.fdateLastWrite;
      filestamp->dos_st.time = *(USHORT *)&fs3.ftimeLastWrite;
    }

    return stat;
  }

#elif defined(OS_2) && !defined(__FLAT__)     /* 16-bit OS/2 */

  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  struct FileInfoBuf
  {
    unsigned short cre_date, cre_time;  /* not supported yet */
    unsigned short acc_date, acc_time;  /* not supported yet */
    unsigned short write_date;          /* date/time of last write to file */
    unsigned short write_time;
    unsigned long EndOfData;
    unsigned long FileAllocation;
    unsigned short attr;
  };

  int _fast get_fdt(int handle, union stamp_combo *filestamp)
  {
     struct FileInfoBuf fib;
     int stat;

     stat = DosQFileInfo( handle, 1, (void *)&fib, sizeof(fib));
     if(!stat){
        filestamp->dos_st.date = fib.write_date;
        filestamp->dos_st.time = fib.write_time;
     }
     return(stat);
  }

#elif defined(__MSDOS__)

  int _fast get_fdt(int handle, union stamp_combo *filestamp)
  {
     union REGS regs;

  #ifdef __386__

    regs.h.ah = 0x57;                         /* get/set date/time call */
    regs.h.al = 0x00;                         /* get date/time */
    regs.x.ebx = handle;

    int386(0x21, &regs, &regs);

    if (regs.x.cflag)
      return(regs.x.eax);

    filestamp->dos_st.date = (word)(regs.x.edx & 0xffff);
    filestamp->dos_st.time = (word)(regs.x.ecx & 0xffff);

    return 0;

  #else

    regs.h.ah = 0x57;                         /* get/set date/time call */
    regs.h.al = 0x00;                         /* get date/time */
    regs.x.bx = handle;

    int86(0x21, &regs, &regs);

    if (regs.x.cflag)
      return regs.x.ax;

    filestamp->dos_st.date=regs.x.dx;
    filestamp->dos_st.time=regs.x.cx;

    return 0;
  #endif
  }

#elif defined(NT)

  #include "pwin.h"

  int _fast get_fdt(int handle, union stamp_combo *psc)
  {
    extern HANDLE *__FileHandleIDs;
    BY_HANDLE_FILE_INFORMATION fi;
    int id;

#ifdef __WATCOMC__
  #if __WATCOMC__ >= 1000
    id = _os_handle(handle);
  #else
    id = __FileHandleIDs[handle];
  #endif
#else
  #error "Don't know how to handle this!"
#endif

    if (GetFileInformationByHandle((HANDLE)id, &fi))
    {
      FileTimeToDosDateTime(&fi.ftLastWriteTime,
                            &psc->dos_st.date,
                            &psc->dos_st.time);
      return 0;
    }

    return 1;
  }
#elif defined(UNIX)
/* Quick hack by wes. Looks get_fdt populates a stamp combo
 * with the file's timestamp, and returns 0 if okay. I
 * wonder which timestamp that is? I think I'll use mtime.
 */

#include <sys/stat.h>

int get_fdt(int fd, union stamp_combo *psc)
{
  struct stat sb;
  struct tm   timebuf;

  if (fstat(fd, &sb))
    return 1;

  if (TmDate_to_DosDate(localtime_r(&sb.st_mtime, &timebuf), psc))
    return 1;

  return 0;
}
#else
  #error Unknown OS
#endif

