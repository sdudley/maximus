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

/*#define VALIDATION_SUITE_1*/
/*#define VALIDATION_SUITE_2*/

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include "uni.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dos.h>
#include <errno.h>
#include "nopen.h"

/* If _nopen_cheat != 0, the DOS version of nopen() will not do             *
 * the creat/close/open trick when creating a new file.  Instead, it will   *
 * just use creat.  Although this does not allow our sharing code to        *
 * come into force, it does make things a lot faster when file access       *
 * time is the bottleneck.                                                  */

char _stdc _nopen_cheat=0;
extern int __NFiles;


/* Open a file without sharing attributes - simply a shell for sopen */

int _stdc nopen(const char *name, int mode, ...)
{
  int permiss=S_IREAD | S_IWRITE;

  if (mode & O_CREAT)
  {
    va_list val;

    va_start(val, mode);
    permiss=va_arg(val, int);
    va_end(val);
  }

  return nsopen(name, mode, SH_DENYNO, permiss);
}

long _stdc ntell(int fd)
{
  return (nlseek(fd, 0, SEEK_CUR));
}


#if defined(__WATCOMC__)

#if __WCVER__ >= 950
  extern __IOMode(int fd);
  extern __SetIOMode(int fd, int mode);
#else
  #error check this!
  #define __IOMode(fd) _iomode[fd]
  #define __SetIOMode(fd, mode) _iomode[fd]=mode
#endif

#define __OrIoMode(fd, flag) __SetIOMode(fd, __IOMode(fd) | (flag))

#ifndef NT  /* not required for NT build */

static void near _wcclearflags(int fd)
{
  if (fd==-1 || fd >= __NFiles)
    return;

  __SetIOMode(fd, 0);
}

static void near _wcsetiomode(int fd, int mode)
{
  if (fd==-1 || fd >= __NFiles)
    return;

  __SetIOMode(fd, 0);   /* initialize i/o mode to "no bits set" */

/*  if (mode & (O_RDONLY|O_RDWR)) */
  if ((mode & O_WRONLY)==0)   /* WC uses "#define O_RDONLY 0"! */
    __OrIoMode(fd, _READ);

  if (mode & (O_WRONLY|O_RDWR))
    __OrIoMode(fd, _WRITE);

  if (mode & O_APPEND)
    __OrIoMode(fd, _APPEND);

  if (mode & O_BINARY)
    __OrIoMode(fd, _BINARY);

  __OrIoMode(fd, _IONBF);

  if (isatty(fd))
    __OrIoMode(fd, _ISTTY);
}
#endif /* !NT */

#endif /* __WATCOMC__ */


#if defined(__MSDOS__)

  #include "fpseg.h"

  /* Open a file with sharing attributes */

  int _stdc nsopen(const char *name, int mode, int shacc, ...)
  {
    union REGS save, r;
    int permiss=S_IREAD | S_IWRITE;
    int fd;

    #ifndef __FLAT__
    struct SREGS sr;
    #endif


    if (mode & O_CREAT)
    {
      va_list val;

      va_start(val, shacc);
      permiss=va_arg(val, int);
      va_end(val);
    }

    /* Access mode: read, write, read/write */

    r.h.al=(char)((mode & O_RDWR)==O_RDWR      ?  0x02 :
                  (mode & O_WRONLY)==O_WRONLY  ?  0x01 :
                                                  0x00);

    r.h.al |= ((shacc==SH_DENYNO) ? 0x40 :
               (shacc==SH_DENYWR) ? 0x20 :
               (shacc==SH_DENYRD) ? 0x30 :
               (shacc==SH_DENYRW) ? 0x10 :
               0) | /* "compatibility" mode */
               ((mode & O_NOINHERIT) ? 0x80u : 0); /* noinherit is default */

    /* read-only unless S_IWRITE */

  #ifdef __FLAT__
    r.x.ecx = (permiss & S_IWRITE)==S_IWRITE ? 0x00 : 0x01;
    r.x.edx = (unsigned)name;
  #else
    r.x.cx = (permiss & S_IWRITE)==S_IWRITE ? 0x00 : 0x01;
    sr.ds=FP_SEG(((char far *)name));
    r.x.dx=FP_OFF(((char far *)name));
  #endif


    save=r;

    /* If we're not using exclusive access, try to open the file.  If it    *
     * is exclusive access, skip this since we need to call creat() anyway. */

    if (mode & O_EXCL)
      r.x.cflag=1;
    else
    {
      /* Function 3d = open file */

      r.h.ah=0x3d;

  #ifdef __FLAT__
      int386(0x21, &r, &r);
  #else
      int86x(0x21, &r, &r, &sr);
  #endif
/*      printf("int 21 ah=3d rc=%d\n", r.x.cflag);*/

      if (r.x.cflag==0 && (mode & O_TRUNC))
      {
#ifdef __FLAT__
        int tfd=r.x.eax;
#else
        int tfd=r.x.ax;
#endif

        nlseek(tfd, 0L, SEEK_SET);
        nwrite(tfd, "", 0);
      }
    }


    /* If it failed, or if using exclusive access, use creat() */

    if (r.x.cflag && (mode & O_CREAT))
    {
      r=save;
      r.h.ah=(char)((mode & O_EXCL) ? 0x5b : 0x3c);  /* create new file */

  #ifdef __FLAT__
      int386(0x21, &r, &r);
  #else
      int86x(0x21, &r, &r, &sr);
  #endif

/*      printf("int 21 ah=%x rc=%d\n", (mode & O_EXCL) ? 0x5b : 0x3c, r.x.cflag);*/

      /* If the open succeeded... */

      if (r.x.cflag==0 && !_nopen_cheat)
      {
        /* The file is open, but creat() doesn't honour file sharing modes.  *
         * Consequently, we have to close and reopen using the mode we want. */

        if (shacc != SH_COMPAT)
        {
  #ifdef __FLAT__
          nclose(r.x.eax);
  #else
          nclose(r.x.ax);
  #endif

/*          printf("int 21 ah=close rc=%d\n", r.x.cflag);*/

          r=save;
          r.h.ah=0x3d;

  #ifdef __FLAT__
          int386(0x21, &r, &r);
  #else
          int86x(0x21, &r, &r, &sr);
  #endif

/*          printf("int 21 ah=3d rc=%d\n", r.x.cflag);*/
        }
      }
    }

    if (r.x.cflag)
    {
  #ifdef __FLAT__
      errno=r.x.eax;
  #else
      errno=r.x.ax;
  #endif
      return -1;
    }

  #ifdef __FLAT__
    fd=r.x.eax;
  #else
    fd=r.x.ax;
  #endif

    if (fd != -1 && (mode & O_APPEND))
      nlseek(fd, 0L, SEEK_END);

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    _wcsetiomode(fd, mode);
  #endif

    return fd;
  }


  int _stdc nread(int fd, char *buf, unsigned len)
  {
    union REGS r;

  #ifdef __FLAT__
    r.h.ah=0x3f;
    r.x.ebx=fd;
    r.x.ecx=len;
    r.x.edx=(unsigned)buf;
    int386(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.eax;
      return -1;
    }

    return r.x.eax;
  #else
    struct SREGS sr;

    r.h.ah=0x3f;
    r.x.bx=fd;
    r.x.cx=len;
    sr.ds=FP_SEG(((char far *)buf));
    r.x.dx=FP_OFF(((char far *)buf));
    int86x(0x21, &r, &r, &sr);

    if (r.x.cflag)
    {
      errno=r.x.ax;
      return -1;
    }

    return r.x.ax;
  #endif
  }

  int _stdc nwrite(int fd, char *buf, unsigned len)
  {
    union REGS r;

  #ifdef __FLAT__
    r.h.ah=0x40;
    r.x.ebx=fd;
    r.x.ecx=len;
    r.x.edx=(unsigned)buf;
    int386(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.eax;
      return -1;
    }

    return r.x.eax;
  #else
    struct SREGS sr;
    r.h.ah=0x40;
    r.x.bx=fd;
    r.x.cx=len;
    sr.ds=FP_SEG(((char far *)buf));
    r.x.dx=FP_OFF(((char far *)buf));
    int86x(0x21, &r, &r, &sr);

    if (r.x.cflag)
    {
      errno=r.x.ax;
      return -1;
    }

    return r.x.ax;
  #endif
  }

  long _stdc nlseek(int fd, long ofs, int pos)
  {
    union REGS r;

    r.h.ah=0x42;

    r.h.al=(char)((pos==SEEK_CUR) ? 1 :
                  (pos==SEEK_END) ? 2 :
                                    0);
  #ifdef __FLAT__
    r.x.ebx=fd;
    r.w.cx=(int)(ofs >> 16);
    r.w.dx=(int)(ofs & 0xffffuL);
/*    r.x.edx=ofs;*/
    int386(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.eax;
      return -1;
    }

    return ((long)(r.w.dx << 16) | (long)r.w.ax);
  #else
    r.x.bx=fd;
    r.x.cx=(int)(ofs >> 16);
    r.x.dx=(int)(ofs & 0xffffuL);
    int86(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.ax;
      return -1;
    }

    return ((long)r.x.dx << 16) | (long)r.x.ax;
  #endif

  }

  int _stdc nclose(int fd)
  {
    union REGS r;

    r.h.ah=0x3e;

  #ifdef __FLAT__
    r.x.ebx=fd;
    int386(0x21, &r, &r);
  #else
    r.x.bx=fd;
    int86(0x21, &r, &r);
  #endif

    /* Now clear out flags for this handle */

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
  _wcclearflags(fd);
  #endif

    return (r.x.cflag ? -1 : 0);
  }

  int ndup(int fd)
  {
    union REGS r;

  #ifdef __FLAT__
    r.h.ah=0x45;
    r.x.ebx=fd;

    int386(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.eax;
      return -1;
    }

    #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
      if (r.x.eax < __NFiles)
        __SetIOMode((int)r.x.eax, __IOMode(fd));
    #endif

    return r.x.eax;
  #else
    r.h.ah=0x45;
    r.x.bx=fd;

    int86(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.ax;
      return -1;
    }

    #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
      if (r.x.ax < __NFiles)
        __SetIOMode((int)r.x.ax, __IOMode(fd));
    #endif

    return r.x.ax;
  #endif
  }

  int ndup2(int fd1, int fd2)
  {
    union REGS r;

    r.h.ah=0x46;

  #ifdef __FLAT__
    r.x.ebx=fd1;
    r.x.ecx=fd2;

    int386(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.eax;
      return -1;
    }
  #else
    r.x.bx=fd1;
    r.x.cx=fd2;

    int86(0x21, &r, &r);

    if (r.x.cflag)
    {
      errno=r.x.ax;
      return -1;
    }
  #endif

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    if (fd2 < __NFiles)
      __SetIOMode(fd2, __IOMode(fd1));
  #endif

    return 0;
  }


#elif defined(OS_2)

  #define INCL_DOSFILEMGR
  #include "pos2.h"

  int _stdc nsopen(const char *name, int mode, int shacc, ...)
  {
    HFILE hf;
  #ifdef __FLAT__
    ULONG rc, usAction;
  #else
    USHORT rc, usAction;
  #endif
    int permiss=S_IREAD | S_IWRITE;
    char *p;

    if (mode & O_CREAT)
    {
      va_list val;

      va_start(val, shacc);
      permiss=va_arg(val, int);
      va_end(val);
    }

    /* Now use the OS "open file" function */

    p=(char *)name;

  #ifdef __FLAT__ /* OS/2 2.0 */
    rc=DosOpen(p,
               &hf,
               &usAction,
               0,
               /****/
               (permiss & S_IWRITE) ? FILE_NORMAL : FILE_READONLY,
               /****/
               ((mode & O_CREAT) ? OPEN_ACTION_CREATE_IF_NEW :
                                   OPEN_ACTION_FAIL_IF_NEW) |
               ((mode & O_EXCL) ? OPEN_ACTION_FAIL_IF_EXISTS :
                (mode & O_TRUNC) ? OPEN_ACTION_REPLACE_IF_EXISTS :
                                   OPEN_ACTION_OPEN_IF_EXISTS),
               /****/
               ((mode & O_WRONLY)==O_WRONLY ? OPEN_ACCESS_WRITEONLY :
                (mode & O_RDWR)==O_RDWR     ? OPEN_ACCESS_READWRITE :
                                              OPEN_ACCESS_READONLY) |
               (shacc==SH_DENYWR ? OPEN_SHARE_DENYWRITE :
                shacc==SH_DENYRD ? OPEN_SHARE_DENYREAD :
                shacc==SH_DENYRW ? OPEN_SHARE_DENYREADWRITE :
                                   OPEN_SHARE_DENYNONE) |
               OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_NO_LOCALITY,
               /****/
               NULL);
  #else
    rc=DosOpen(p,
               &hf,
               &usAction,
               0,
               /****/
               (permiss & S_IWRITE) ? FILE_NORMAL : FILE_READONLY,
               /****/
               ((mode & O_EXCL) ? 0 :
                 ((mode & (O_CREAT|O_TRUNC))==(O_CREAT|O_TRUNC)
                 ? 0 : FILE_OPEN)) |
               ((mode & O_CREAT) ? FILE_CREATE : 0)   |
               ((mode & (O_TRUNC|O_EXCL))==O_TRUNC ? FILE_TRUNCATE : 0),
               /****/
               ((mode & O_WRONLY)==O_WRONLY ? OPEN_ACCESS_WRITEONLY :
                (mode & O_RDWR)==O_RDWR     ? OPEN_ACCESS_READWRITE :
                                              OPEN_ACCESS_READONLY) |
               (shacc==SH_DENYWR ? OPEN_SHARE_DENYWRITE :
                shacc==SH_DENYRD ? OPEN_SHARE_DENYREAD :
                shacc==SH_DENYRW ? OPEN_SHARE_DENYREADWRITE :
                                   OPEN_SHARE_DENYNONE) |
               OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_NO_LOCALITY,
               /****/
               0);
  #endif

    if (rc)
    {
      errno=rc;
      return -1;
    }

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    _wcsetiomode(hf, mode);
  #endif

    if (hf != -1 && (mode & O_APPEND))
      nlseek(hf, 0L, SEEK_END);

    return hf;
  }

  int _stdc nread(int fd, char *buf, unsigned len)
  {
    #ifdef __FLAT__
      ULONG rc, got;
    #else
      USHORT rc, got;
    #endif

    rc=DosRead((HFILE)fd, buf, len, &got);

    if (rc)
    {
      errno=rc;
      return -1;
    }

    return got;
  }

  int _stdc nwrite(int fd, char *buf, unsigned len)
  {
    #ifdef __FLAT__
      ULONG rc, got;
    #else
      USHORT rc, got;
    #endif

    rc=DosWrite((HFILE)fd, buf, len, &got);

    if (rc)
    {
      errno=rc;
      return -1;
    }

    return got;
  }

  long _stdc nlseek(int fd, long ofs, int pos)
  {
    #ifdef __FLAT__
      ULONG rc;
    #else
      USHORT rc;
    #endif

    ULONG newpos;

  #ifdef __FLAT__
    rc=DosSetFilePtr((HFILE)fd, ofs,
  #else
    rc=DosChgFilePtr((HFILE)fd, ofs,
  #endif
                     pos==SEEK_END ? FILE_END :
                     pos==SEEK_CUR ? FILE_CURRENT :
                     FILE_BEGIN,
                     &newpos);

    if (rc)
    {
      errno=rc;
      return -1;
    }

    return newpos;
  }

  int _stdc nclose(int fd)
  {
    #ifdef __FLAT__
      ULONG rc;
    #else
      USHORT rc;
    #endif

    rc=DosClose(fd);

    if (rc)
    {
      errno=rc;
      return -1;
    }

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    _wcclearflags(fd);
  #endif

    return 0;
  }

  int _stdc ndup(int fd)
  {
    HFILE hfNew=(HFILE)-1;
    OS2UINT rc;

    if ((rc=DosDupHandle((HFILE)fd, &hfNew)) != 0)
    {
      errno=rc;
      return -1;
    }

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    if ((int)hfNew < __NFiles)
      __SetIOMode((int)hfNew, __IOMode(fd));
  #endif

    return (int)hfNew;
  }

  int _stdc ndup2(int fd1, int fd2)
  {
    HFILE hfNew=(HFILE)fd2;
    OS2UINT rc;

    if ((rc=DosDupHandle((HFILE)fd1, &hfNew)) != 0)
    {
      errno=rc;
      return -1;
    }

  #if defined(__WATCOMC__) /* handle special declarations for WC's file mode array */
    if (fd2 < __NFiles)
      __SetIOMode(fd2, __IOMode(fd1));
  #endif

    return 0;
  }

#elif defined(NT) || defined(__POSIX__)

  /* We will assume for now that the Windows NT version of the MS RTL       *
   * are okay.  (A bad assumption, but...)  If we run into trouble          *
   * later, our own version of these functions can be written.              */

  int _stdc nsopen(const char *name, int mode, int shacc, ...)
  {
    return sopen(name, mode, shacc, S_IREAD | S_IWRITE);
  }

  int _stdc nread(int fd, char *buf, unsigned len)
  {
    return read(fd, buf, len);
  }

  int _stdc nwrite(int fd, char *buf, unsigned len)
  {
    return write(fd, buf, len);
  }

  long _stdc nlseek(int fd, long ofs, int pos)
  {
    return lseek(fd, ofs, pos);
  }

  int _stdc nclose(int fd)
  {
    return close(fd);
  }

  int _stdc ndup(int fd)
  {
    return dup(fd);
  }

  int _stdc ndup2(int fd1, int fd2)
  {
    return dup2(fd1, fd2);
  }

#else
  #error Unknown OS
#endif


#ifdef VALIDATTION_SUITE_1
main()
{
  int fd;
  char buf[2];

  if ((fd=nopen("open.obj", O_RDONLY | O_BINARY))==-1)
  {
    printf("err opening open.obj\n");
    return 1;
  }

  printf("ntell(fd)=%ld\n", ntell(fd));

  printf("nread(fd, buf, 2)=%d\n",
         nread(fd, buf, 2));

  printf("ntell(fd)=%ld\n", ntell(fd));

  printf("nlseek(fd, 1, SEEK_SET)=%ld\n", nlseek(fd, 1L, SEEK_SET));

  printf("ntell(fd)=%ld\n", ntell(fd));

  printf("hdr=%c%c\n", buf[0], buf[1]);

  printf("nclose(fd)=%d\n", nclose(fd));
}
#endif

#ifdef VALIDATION_SUITE_2

main()
{
  int fd;

  if ((fd=nopen("nopen.obj", O_CREAT | O_TRUNC | O_EXCL | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("  OK: O_EXCL1 (%d)\n", fd);
  }
  else
  {
    printf("FAIL: O_EXCL1\n");
    nclose(fd);
  }

  unlink("notexist.fil");

  if ((fd=nopen("notexist.fil", O_CREAT | O_TRUNC | O_EXCL | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_EXCL2 (%d)\n", fd);
  }
  else
  {
    printf("  OK: O_EXCL2\n");
    nclose(fd);
  }

  unlink("notexist.fil");

  if ((fd=nopen("notexist.fil", O_CREAT | O_EXCL | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_EXCL3 (%d)\n", fd);
  }
  else
  {
    printf("  OK: O_EXCL3\n");
    nclose(fd);
  }

  unlink("notexist.fil");


  if ((fd=nopen("notexist.fil", O_RDONLY | O_BINARY | O_EXCL, S_IREAD | S_IWRITE))==-1)
  {
    printf("  OK: O_RDONLY1\n");
  }
  else
  {
    printf("FAIL: O_RDONLY1\n");
    nclose(fd);
  }


  if ((fd=nopen("notexist.fil", O_RDONLY | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("  OK: O_RDONLY2\n");
  }
  else
  {
    printf("FAIL: O_RDONLY2\n");
    nclose(fd);
  }

  if ((fd=nopen("nopen.obj", O_RDONLY | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_RDONLY3 (%d)\n", fd);
  }
  else
  {
    if (nlseek(fd, 0, SEEK_END) > 1)
      printf("  OK: O_RDONLY3\n");
    else printf("FAIL: O_RDONLY3 (truncated)\n");

    nclose(fd);
  }

  if ((fd=nopen("nopen.obj", O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_CREAT1 (%d)\n", fd);
  }
  else
  {
    if (nlseek(fd, 0, SEEK_END) > 1)
      printf("  OK: O_CREAT1\n");
    else printf("FAIL: O_CREAT1 (truncated)\n");

    nclose(fd);
  }

  if ((fd=nopen("nopen.obj", O_CREAT | O_TRUNC | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_CREAT2 %d %d\n", fd, errno);
  }
  else
  {
    if (nlseek(fd, 0, SEEK_END) != 0)
      printf("FAIL: O_CREAT2 (not truncated)\n");
    else printf("  OK: O_CREAT2\n");

    nwrite(fd, "foo", 3);
    nclose(fd);
  }

  if ((fd=nopen("new.fil", O_CREAT | O_RDWR | O_BINARY, S_IREAD | S_IWRITE))==-1)
  {
    printf("FAIL: O_CREAT3 (%d)\n", fd);
  }
  else
  {
    if (nlseek(fd, 0, SEEK_END)==0)
    {
      long ofs;

      if ((ofs=nlseek(fd, 0x1234, SEEK_SET)) != 0x1234)
        printf("FAIL: O_CREAT3 (ofs=%lx)\n", ofs);
      else printf("  OK: O_CREAT3\n");

      nwrite(fd, "foo bar", 7);
    }
    else printf("FAIL: O_CREAT3 (not truncated)\n");

    nclose(fd);
  }

  unlink("new.fil");
  return 0;
}

#endif

