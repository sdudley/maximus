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
#include "prog.h"

#if defined(OS_2) && defined(__FLAT__)      /* 32-bit OS/2 */

  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  /* Set file date/time -- OS/2 version */

  int _fast set_fdt(int handle, union stamp_combo *filestamp)
  {
    FILESTATUS3 fs3;
    int stat;

    if ((stat=DosQueryFileInfo(handle, FIL_STANDARD, &fs3, sizeof fs3))==0)
    {
      *(USHORT *)&fs3.fdateLastWrite = filestamp->dos_st.date;
      *(USHORT *)&fs3.ftimeLastWrite = filestamp->dos_st.time;

      stat = DosSetFileInfo(handle, FIL_STANDARD, (void *)&fs3, sizeof fs3);
    }

    return(stat);
  }

#elif defined(OS_2) && !defined(__FLAT__)  /* 16-bit OS/2 */

  #define INCL_NOPM
  #define INCL_DOS
  #include <os2.h>

  struct FileInfoBuf
  {
    unsigned short cre_date, cre_time;  /* not supported yet */
    unsigned short acc_date, acc_time;  /* not supported yet */
    unsigned short write_date;  /* date/time of last write to file */
    unsigned short write_time;
  };


  /* set file date/time. OS/2 version */

  int _fast set_fdt(int handle, union stamp_combo *filestamp)
  {
     struct FileInfoBuf fib;
     int stat;

     fib.cre_date = fib.cre_time = 0;
     fib.acc_date = fib.acc_time = 0;

     fib.write_date = filestamp->dos_st.date;
     fib.write_time = filestamp->dos_st.time;

     stat = DosSetFileInfo( handle, 1, (void *)&fib, sizeof(fib));
     return(stat);
  }

#elif defined(__MSDOS__)

  /* set date and time of file, using int 21h (MS-DOS)  */

  int _fast set_fdt(int handle, union stamp_combo *filestamp)
  {
    union REGS regs;

  #ifdef __386__
    regs.h.ah=0x57;                         /* get/set date/time call */
    regs.h.al=0x01;                         /* set date/time */
    regs.x.ebx=handle;
    regs.x.edx=(unsigned)filestamp->dos_st.date;
    regs.x.ecx=(unsigned)filestamp->dos_st.time;

    int386(0x21, &regs,&regs);

    return (regs.x.cflag ? regs.x.eax : 0);
  #else
    regs.h.ah=0x57;                         /* get/set date/time call */
    regs.h.al=0x01;                         /* set date/time */
    regs.x.bx= handle;
    regs.x.dx=filestamp->dos_st.date;
    regs.x.cx=filestamp->dos_st.time;

    int86(0x21, &regs,&regs);

    return (regs.x.cflag ? regs.x.ax : 0);
  #endif
  }

#elif defined(NT)

  #include "pwin.h"

  int _fast set_fdt(int handle, union stamp_combo *psc)
  {
    FILETIME ft;

    DosDateTimeToFileTime(psc->dos_st.date, psc->dos_st.time, &ft);
    return !SetFileTime((HANDLE)handle, NULL, NULL, &ft);
  }

#elif defined(UNIX)
#include <sys/types.h>
#include <utime.h>

int set_fdt(int fd, union stamp_combo *psc)
{
#if 0
  struct utimbuf touchBuf;
  struct timeval tmdate, *tmdate_p;

  if ((fd < 0) || (!psc))
    return 1;

  memset(&touchBuf, 0, sizeof(touchBuf));
  memset(&tmdate, 0, sizeof(tmdate));

  tmdate_p = DosDate_to_TmDate(psc, &tmdate);
  if (!tmdate_p)
    return 1;

  touchBuf.actime = touchBuf.modtime = tmdate_p->tv_sec;

  
  return 0;
#endif
  return 1; /* No equivalent UNIX call -- can't get filename reliably from fd */
}
#else
  #error Unknown OS
#endif

