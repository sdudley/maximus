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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: fos_os2.c,v 1.11 2004/01/27 21:00:29 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=FOSSIL interface routines (OS/2)
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>

#if defined(UNIX)
# include "prog.h"
# include "mm.h"
# include "modem.h"
#endif

#if defined(OS_2) || defined(NT)
  #ifdef OS_2
    #define INCL_DOS
    #define INCL_VIO
    #define INCL_NOPM
    #include "pos2.h"
  #endif

  #include "prog.h"
  #include "mm.h"
  #include "modem.h"
#endif

#if defined(OS_2) || defined(NT) || defined(UNIX)
  static int fossil_installed=FALSE;

  int mdm_ggetcw(void)
  {
    ComRxWait(hcModem, -1L);
    return ComGetc(hcModem);
  }

  int mdm_deinit(void)          /* Returns 0 if closed, -1 if not open. */
  {
    extern int cdecl port;

    if (!local)
    {
      ComTxWait(hcModem, 10000L);
#if defined(NT) || defined(UNIX)
      ComClose(hcModem);
#endif
    }

    fossil_installed=FALSE;
    return 0;
  }



  void fossil_getxy(char *row, char *col)
  {
  #ifdef NT
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    *row=csbi.dwCursorPosition.Y;
    *col=csbi.dwCursorPosition.X;
  #else
    unsigned short r,c;

  #ifdef UNIX
    *row = (char)VidWhereY();
    *col = (char)VidWhereX();
  #else
    VioGetCurPos(&r, &c, 0);
    *row=(char)r;
    *col=(char)c;
  #endif

  #endif
  }

  #if 0 /* notused */
  void fossil_gotoxy(char row,char col)
  {
      VioSetCurPos((word)row, (word)col, 0);
  }
  #endif


  int fossil_inf(struct _fossil_info far *finfo)   /* Returns length of structure */
  {
      char far *p=(char *)finfo;

      while (p < (char *)(finfo+1))
        *p++='\0';

  #if defined(NT)
    {
      CONSOLE_SCREEN_BUFFER_INFO csbi;

      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

      finfo->width=csbi.dwSize.X;
      finfo->height=csbi.dwSize.Y;
    }
  #elif defined(UNIX)
    {
      finfo->width = VidNumCols();
      finfo->height = VidNumRows();
    }
  #else
    {
      VIOMODEINFO viomi;
      viomi.cb = sizeof(viomi);
      VioGetMode(&viomi, 0);

      finfo->width  = (char)viomi.col;
      finfo->height = (char)viomi.row;
    }
  #endif

      finfo->size = sizeof(struct _fossil_info);
      return(finfo->size);
  }


#ifdef TTYVIDEO
  void _stdc fossil_putc(char chr)
  {
    if (chr=='\n')
      putchar('\r');
    putchar(chr);
  }



  void _stdc fossil_puts(char *str)
  {
    #ifdef NT
      printf(percent_s, str);
      fflush(stdout);
    #else
      VioWrtTTY(str, strlen(str), 0);
    #endif
  }
#endif

/*  #ifndef ORACLE */

    int mdm_avail()
    {
      return !local && ComInCount(hcModem);
    }

    int mdm_baud(int bod)
    {
      static struct
      {
        int bodmask;
        long baudrate;
      } *pb, bodcvt[]=
      {
        {BAUD300,     300L},
        {BAUD600,     600L},
        {BAUD1200,    1200L},
        {BAUD2400,    2400L},
        {BAUD4800,    4800L},
        {BAUD9600,    9600L},
        {BAUD19200,   19200L},
        {BAUD38400,   38400L},
        {BAUD57600,   57600L},
        {BAUD115200,  115200L},
        {0,           0}
      };

      USHORT rc;
      long b;

      if (steady_baud)
        bod=steady_baud;

#if (COMMAPI_VER > 1)
if (ComIsAModem(hcModem))
{
#endif
      for (pb=bodcvt; pb->baudrate; pb++)
      {
        if (pb->bodmask==bod)
        {
          b=pb->baudrate;
          break;
        }
      }

      /* If unknown baud rate, use 19.2k */

      if (!pb->bodmask)
        b=19200L;
#if (COMMAPI_VER > 1)
} else b=38400L;
#endif

    #if defined(NT) || defined(UNIX)
      rc=!ComSetBaudRate(hcModem, b, NOPARITY, 8, ONESTOPBIT);
    #else
      /* If the speed is 115.2k, do nothing.  ComSetBaudRate doesn't
       * support the extended 115.2k ioctl, so don't touch the port
       * speed and hope that it was already set for 115.2k.
       */

      if (b==115200L)
        rc = 0;
      else
        rc = ComSetBaudRate(hcModem, b, 'N', 8, 1);
    #endif

      if (rc)
        logit("!SYS%04u:  ComSetBaudRate(%ld)", rc, b);

      return(0);
    }

    int mdm_blockwrite(int max_chars, char *offset)
    {
    
      //max_chars=min(ComOutSpace(hcModem), max_chars);

      if (!max_chars)
        return 0;

#if defined(NT) || defined(UNIX)
      return(ComWrite(hcModem, offset, max_chars) ? max_chars : 0);
#else
      return(ComWrite(hcModem, offset, max_chars) ? 0 : max_chars);
#endif
    }


    /* Read a block of data from the modem */

    int mdm_blockread(int max_chars, char *offset)
    {
      #if defined(NT) || defined(UNIX)
        DWORD cbBytesRead = 0;
      #else
        USHORT cbBytesRead = 0;
      #endif

      #if !defined(NT) && !defined(UNIX)
        char *p=offset;
        char *e=p+max_chars;
        int ch;

        while (p < e)
          if ((ch=ComGetc(hcModem)) != -1)
    	    *p++=(byte)ch;
          else break;

        cbBytesRead=p-offset;
      #else      
        ComRead(hcModem, offset, max_chars, &cbBytesRead);
      #endif
      return (int)cbBytesRead;
    }


    /*
     * break for 'hsecs' hundredths of seconds
     */

    void mdm_break(unsigned int hsecs)
    {
      com_break(TRUE);

      #ifdef NT
        Sleep(hsecs * 10L);
      #else
        DosSleep(hsecs * 10L);
      #endif

      com_break(FALSE);
    }




    int mdm_ctrlc(char mask)
    {
      int c;

      extern char local;

      /* This code is only required for OS/2.  Since the OS/2 keyboard cannot   *
       * (easily) send a signal for control-c when running in binary mode,      *
       * we must instead check for it at the beginning of the keyboard          *
       * buffer.                                                                */

      if (loc_peek()==3)
      {
        loc_getch(); /* throw away character */
        brk_trapped=TRUE;
        return TRUE;
      }

      /* This only works if ^C/^K is the first key that has not been serviced */

      if (local)
        return brk_trapped;

      if (! mask)
        return 0;

      c=ComPeek(hcModem);

      return (c==3 || c==11);
    }



    void mdm_dtr(char dtr)       /* No return value */
    {
#ifndef UNIX
      if (dtr)
        com_DTR_on();
      else com_DTR_off();
#else
      if (dtr)
        RAISE_DTR(hcModem);
      else LOWER_DTR(hcModem);
#endif      
    }


#ifndef ORACLE
    char mdm_dump(char buffer)
    {
      if (local)
        return 0;

      switch (buffer)
      {
        case DUMP_OUTPUT:
          ComPurge(hcModem, COMM_PURGE_TX);

          if (prm.flags & FLAG_break_clr)
            mdm_break(10);
          break;

        case DUMP_INPUT:
          ComPurge(hcModem, COMM_PURGE_RX);
          break;

        case DUMP_ALL:
          ComPurge(hcModem, COMM_PURGE_ALL);

          if (prm.flags & FLAG_break_clr)
            mdm_break(10);
          break;

        default:
            return -1;
      }

      return 0;
    }
#endif

    int mdm_init(int prt)   /* Returns 0 upon success, 2 on no FOSSIL installed */
    {
      port=prt;

      if (local)
        return INIT_OK;

      if (Cominit(prt) != 0x1954)
        return INIT_NOFOSSIL;

      if (waitforcaller)
        ComWatchDog(hcModem, FALSE, 0);     /* disable */
      else ComWatchDog(hcModem, TRUE, 5);   /* enable,  5 sec timeout */

      return INIT_OK;
    }

    void mdm_watchdog(int watch) /* No return value */
    {
      NW(watch);
    }

    word real_carrier(void)
    {
      return ComIsOnline(hcModem);
    }

    word carrier(void)
    {
      if (no_dcd_check || local)
        return TRUE;

      return real_carrier();
    }

/*  #endif */ /* ORACLE */

#endif /* OS_2 || NT */

/* works with output, but no input... */

