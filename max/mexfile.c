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

#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mexall.h"
#include "ffind.h"

#ifdef MEX

  int MexAddFHandle(struct _mex_instance_stack *pmis, int fd)
  {
    int i;

    for (i=0; i < MAX_MEXFH; ++i)
    {
      if (pmis->fht[i]==(word)-1)
      {
        pmis->fht[i]=(word)fd;
        return TRUE;
      }
    }

    return FALSE;
  }

  int MexDelFHandle(struct _mex_instance_stack *pmis, int fd)
  {
    int i;

    for (i=0; i < MAX_MEXFH; ++i)
    {
      if (pmis->fht[i]==(word)fd)
      {
        close(fd);
        pmis->fht[i]=(word)-1;
        return TRUE;
      }
    }

    return FALSE;
  }

  /* Open a file for read/write */

  word EXPENTRY intrin_open(void)
  {
    MA ma;
    char *s;
    word wMode;
    int mode;

    MexArgBegin(&ma);

    s=MexArgGetString(&ma, FALSE);
    wMode=MexArgGetWord(&ma);

/*    Printf("@MAX: open file '%s' mode %d\n", s ? s : "(null)", wMode);*/

    if (!s)
      mode=-1;
    else
    {
      mode=0;

      if (wMode & IOPEN_CREATE)
        mode |= O_CREAT | O_TRUNC;

      if ((wMode & IOPEN_RW)==IOPEN_RW)
        mode |= O_RDWR;
      else if (wMode & IOPEN_WRITE)
        mode |= O_WRONLY;
      else
        mode |= O_RDONLY;

      if (wMode & IOPEN_APPEND)
        mode |= O_APPEND;

      if (wMode & IOPEN_BINARY)
        mode |= O_BINARY;

      mode=sopen(s, mode, SH_DENYNO, S_IREAD | S_IWRITE);
      free(s);

      if (mode!=-1)
      {
        if (! MexAddFHandle(pmisThis, mode))
        {
          close(mode);
          mode = -1;
        }
      }
    }

    regs_2[0]=mode;
    return MexArgEnd(&ma);
  }



  /* Read from a file */

  word EXPENTRY intrin_read(void)
  {
    MA ma;
    IADDR where;
    word wLen;
    char *s;
    int fd, len;

    MexArgBegin(&ma);

    /* Get the file descriptor, the string, and the length */

    fd=MexArgGetWord(&ma);
    MexArgGetRefString(&ma, &where, &wLen);
    len=MexArgGetWord(&ma);

    if ((s=malloc(len)) != NULL)
    {
      regs_2[0]=read(fd, s, len);
      MexKillString(&where);

      MexStoreByteStringAt(MexIaddrToVM(&where), s,
                           (int)regs_2[0] > 0 ? regs_2[0] : 0);
      free(s);
/*      Printf("@MAX: Read string '%-*.*s'\n", len, len, s);*/
    }

    return MexArgEnd(&ma);
  }


  /* Read one line from a file */

  word EXPENTRY intrin_readln(void)
  {
    MA ma;
    IADDR where;
    word wLen;
    char *s;
    int fd;

    MexArgBegin(&ma);

    fd=MexArgGetWord(&ma);
    MexArgGetRefString(&ma, &where, &wLen);

    #define MEX_MAX_LINE  512

    /* Allocate memory for reading in one line */

    if ((s=malloc(MEX_MAX_LINE+1))==NULL)
      regs_2[0]=-1;
    else
    {
      char *p;
      long lPosn = tell(fd);
      int got=read(fd, s, MEX_MAX_LINE);
      int len;

      /* Make sure that line is NUL-terminated */

      s[MEX_MAX_LINE]=0;

      if (got > 0)
        s[got]=0;

      /* Cap it off at the \n, if necessary */

      p=strchr(s, '\n');

      if (p != 0)
      {
        if (p > s && p[-1]=='\r')
        {
          p[-1] = 0;
          lPosn += 2;
        }
        else
        {
          p[0]=0;
          ++lPosn;
        }
      }

      len=strlen(s);

      /* Now seek to the beginning of the next line */

      lseek(fd, lPosn + (long)len, SEEK_SET);

      MexKillString(&where);
      MexStoreByteStringAt(MexIaddrToVM(&where), s, len);
/*      Printf("@MAX: Read line '%s'\n", s);*/
      free(s);

      regs_2[0]=(got <= 0) ? -1 : len;
    }

    return MexArgEnd(&ma);
  }


  /* Write to a file */

  word EXPENTRY intrin_write(void)
  {
    MA ma;
    IADDR where;
    word wLen;
    char *s;
    int fd, len;

    MexArgBegin(&ma);
    fd=MexArgGetWord(&ma);
    s=MexArgGetRefString(&ma, &where, &wLen);
    len=MexArgGetWord(&ma);

    len=min(len, wLen);

    regs_2[0]=write(fd, s, len);

/*    Printf("@MAX: Write text '%-*.*s' to file, len=%d\n", wLen, wLen,
           s ? s : "(null)", wLen);*/

    return MexArgEnd(&ma);
  }


  /* Write to a file */

  word EXPENTRY intrin_writeln(void)
  {
    MA ma;
    IADDR where;
    word wLen;
    char *s;
    int fd;

    MexArgBegin(&ma);
    fd=MexArgGetWord(&ma);
    s=MexArgGetNonRefString(&ma, &where, &wLen);

    regs_2[0]=write(fd, s, wLen);
    write(fd, "\r\n", 2);

    MexKillString(&where);

/*    Printf("@MAX: Wrote line '%s' to file\n", s);*/

    return MexArgEnd(&ma);
  }


  /* Seek to a specific byte in a file */

  word EXPENTRY intrin_seek(void)
  {
    MA ma;
    int fd, iWhere;
    long lPos;

    MexArgBegin(&ma);
    fd=MexArgGetWord(&ma);
    lPos=MexArgGetDword(&ma);
    iWhere=MexArgGetWord(&ma);

    regs_4[0]=lseek(fd, lPos, iWhere);

    return MexArgEnd(&ma);
  }



  /* Return current file pointer */

  word EXPENTRY intrin_tell(void)
  {
    MA ma;
    int fd;

    MexArgBegin(&ma);
    fd=MexArgGetWord(&ma);

    regs_4[0]=tell(fd);

    return MexArgEnd(&ma);
  }


  /* Close a file */

  word EXPENTRY intrin_close(void)
  {
    MA ma;
    int fd;

    MexArgBegin(&ma);
    fd=MexArgGetWord(&ma);

    regs_2[0] = MexDelFHandle(pmisThis,fd);
/*    Printf("@MAX: close file %d\n", fd);*/

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_rename(void)
  {
    MA ma;
    char *oldname, *newname;

    MexArgBegin(&ma);
    oldname=MexArgGetString(&ma,FALSE);
    newname=MexArgGetString(&ma,FALSE);
    if (!oldname || !newname)
      regs_2[0]=FALSE;
    else
      regs_2[0]=(rename(oldname,newname)==0);
    if (oldname)
      free(oldname);
    if (newname)
      free(newname);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_remove(void)
  {
    MA ma;
    char *filename;

    MexArgBegin(&ma);
    filename=MexArgGetString(&ma,FALSE);
    if (!filename)
      regs_2[0]=FALSE;
    else
    {
      regs_2[0]=(remove(filename)==0);
      free(filename);
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_filecopy(void)
  {
    MA ma;
    char *oldname, *newname;
    int ret;

    MexArgBegin(&ma);
    oldname=MexArgGetString(&ma,FALSE);
    newname=MexArgGetString(&ma,FALSE);
    if (!oldname || !newname)
      ret=-1;
    else
      ret=lcopy(oldname,newname);
    if (oldname)
      free(oldname);
    if (newname)
      free(newname);

    regs_2[0] = (ret==0);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_fileexists(void)
  {
    MA ma;
    char *filename;

    MexArgBegin(&ma);
    filename=MexArgGetString(&ma,FALSE);
    if (!filename)
      regs_2[0]=0;
    else
    {
      regs_2[0]=fexist(filename);
      free(filename);
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_filesize(void)
  {
    MA ma;
    char *filename;

    MexArgBegin(&ma);
    filename=MexArgGetString(&ma,FALSE);
    if (!filename)
      regs_4[0]=-1;
    else
    {
      regs_4[0]=fsize(filename);
      free(filename);
    }
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_filedate(void)
  {
    MA ma;
    char *filename;
    struct mex_stamp *ms;

    MexArgBegin(&ma);
    filename=MexArgGetString(&ma,FALSE);
    ms=MexArgGetRef(&ma);
    regs_4[0]=FALSE;
    if (filename)
    {
      FFIND *ff=FindOpen(filename,ATTR_READONLY|ATTR_SUBDIR);
      if (ff)
      {
        StampToMexStamp(&ff->scWdate, ms);
        FindClose(ff);
        regs_4[0]=TRUE;
      }
      free(filename);
    }
    return MexArgEnd(&ma);
  }

#endif /* MEX */
