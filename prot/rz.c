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

#pragma off(unreferenced)
static char rcs_id[]="$Id: rz.c,v 1.2 2003/09/12 23:18:24 paltas Exp $";
#pragma on(unreferenced)

#define MAX_LANG_protocols

#include <stdio.h>
#include <io.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include "zsjd.h"
#include "zmodem.h"
#include "pdata.h"
#include "f_up.h"

static FILE *fout=NULL;
static int Eofseen;                    /* indicates cpm eof (^Z) has been received */
static long Bytesleft;                 /* number of bytes of incoming file left */
static long Modtime;                   /* Unix style mod time for incoming file */
static int Filemode;                   /* Unix style mode for incoming file */
static char *Pathname; /*[PATHLEN];*/
static int Rxclob;                     /* Clobber existing file */
static int Rxbinary;                   /* receive all files in bin mode */
static int Rxascii;                    /* receive files in ascii (translate) mode */
static int Thisbinary;                 /* current file is to be received in bin mode */
static char *secbuf; /*[1025];*/
static char zconv;                     /* ZMODEM file conversion request */
static char zmanag;                    /* ZMODEM file management request */
static int tryzhdrtype;       /* Header type to send corresponding to Last rx close */



/* Routine to allocate the static variables used by the rz.c Zmodem routines */

int ZmRzInitStatics(void)
{
  secbuf=malloc(1025);
  Pathname=malloc(PATHLEN);

  return secbuf && Pathname;
}




/* Routine to free the memory allocated by ZmRzInitStatics */

void ZmRzDeinitStatics(void)
{
  if (secbuf)
  {
    free(secbuf);
    secbuf=NULL;
  }

  if (Pathname)
  {
    free(Pathname);
    Pathname=NULL;
  }
}




/*
 * Routine to calculate the free bytes on the current file system
 *  ~0 means many free bytes (unknown)
 */

static long near
getfree(char *path)
{
  return zfree(path);
}







/*
 * Process incoming file information header
 */

static int near
procheader(char *path, char *name)
{
  char *openmode, *p;
  char szShortName[PATHLEN];


  /* Get the short version of the filename */

  strnncpy(szShortName, name, PATHLEN-2);

  /* Convert spaces to underscores */

  for (p=name; *p; p++)
    if (*p==' ')
      *p='_';


  /* set default parameters and overrides */

  Thisbinary = (!Rxascii) || Rxbinary;

  openmode=fopen_write;

  /*
   *  Process ZMODEM remote file management requests
   */

  if (!Rxbinary && zconv == ZCNL)	/* Remote ASCII override */
    Thisbinary = 0;

  if (zconv == ZCBIN)		/* Remote Binary override */
    Thisbinary = TRUE;
  else if (zmanag == ZMAPND)
    openmode = fopen_append;

  if (Thisbinary)
    openmode=zmanag==ZMAPND ? "ab" : fopen_writeb;


  /* Make sure that we're not uploading to a device */

  if (is_devicename(szShortName))
  {
    logit(log_ignoring_file, szShortName);
    return ERROR;
  }


  /* Make sure that it's not a duplicate file */

  if ((prm.flags2 & FLAG2_CHECKDUPE) && FileIsDupe(No_Path(szShortName)))
  {
    logit(log_dupe_file_recd, szShortName);

    /* Make filename begin with a '*' so that Max knows that the
     * file is a dupe.
     */

    strcpy(Pathname, "*");
    strcat(Pathname, szShortName);
    return ERROR;
  }


  /* Find the full name of this file */

  strcpy(Pathname, path);
  strcat(Pathname, No_Path(szShortName));


  /* Make sure that it is unique */

  if (fexist(Pathname))
    unique_name(Pathname);


  Bytesleft = DEFBYTL;
  Filemode = 0;
  Modtime = 0L;

  p = name + strlen(name) + 1;

  if (*p)
  {				/* file coming from Unix or DOS system */
    sscanf(p, "%ld%lo%o", &Bytesleft, &Modtime, &Filemode);

    if (Filemode & UNIXFILE)
      ++Thisbinary;

// Oh well, gotta a problem
#ifndef UNIX
    if (Bytesleft > (zfree(Pathname) - (long)prm.k_free*1000L))
    {
      logit(log_no_space_to_rec, Pathname);
      return ERROR;
    }
#endif
  }

  /* Indicate that we are receiving a file */

  ZmStatFile(Pathname, Bytesleft, Crc32);

  dlogit(("@Opening file '%s'", Pathname));
  fout = fopen(Pathname, openmode);

  if (!fout)
  {
    logit(cantopen, Pathname);
    *Pathname=0;
    return ERROR;
  }

  return OK;
}


/*
 * Putsec writes the n characters of buf to receive file fout.
 *  If not in binary mode, carriage returns, and all characters
 *  starting with CPMEOF are discarded.
 */

static int near
putsec(char *buf, int n)
{
  char *p;

  if (n == 0)
    return OK;

  if (Thisbinary)
  {
    if (fwrite(buf, 1, n, fout) != n)
    {
      logit(log_no_space, errno);
      return ERROR;
    }
  }
  else
  {
    if (Eofseen)
      return OK;

    for (p = buf; --n >= 0; ++p)
    {
      if (*p == '\r')
	continue;

      if (*p == CPMEOF)
      {
	Eofseen = TRUE;
	return OK;
      }

      /* Return to caller if disk full */

      if (putc(*p, fout)==EOF)
      {
        logit(log_no_space, errno);
        return ERROR;
      }
    }
  }

  return OK;
}





#if 0 /* unused */
/*
 * IsAnyLower returns TRUE if string s has lower case letters.
 */

int
IsAnyLower(char *s)
{
  for (; *s; ++s)
    if (islower(*s))
      return TRUE;

  return FALSE;
}
#endif




/*
 * Close the receive dataset, return OK or ERROR
 */


static int near
closeit(void)
{
  time_t time();
  int rc=0;

  if (fout)
  {
    dlogit(("@Closing receive file"));
    rc=fclose(fout);
    fout=NULL;
  }

  if (rc != 0)
    return ERROR;

  if (Modtime)
  {
    SCOMBO sc;
    time_t t=Modtime;
    struct tm *ptm;

    ptm=localtime(&t);
    TmDate_to_DosDate(ptm, &sc);
    SetFileDate(Pathname, &sc);
  }

  return OK;
}


/*
 * Ack a ZFIN packet, let byegones be byegones
 */

static void near
ackbibi(void)
{
  int n;

  dlogit(("@ackbibi:"));

  stohdr(0L);

  for (n = 3; --n >= 0;)
  {
    purgeline();
    zshhdr(ZFIN, Txhdr);

    switch (readline(100))
    {
      case 'O':
	readline(1);		/* Discard 2nd 'O' */
        dlogit(("@ackbibi complete"));
	return;

      case RCDO:
	return;

      case TIMEOUT:
      default:
	break;
    }
  }
}



/*
 * Receive a file with ZMODEM protocol
 *  Assumes file name frame is in secbuf
 */

static int near
rzfile(char *path)
{
  int c, n;

  Eofseen = FALSE;

  if (procheader(path, secbuf) == ERROR)
    return (tryzhdrtype = ZSKIP);

  n = 20;
  Rxbytes = 0L;

  for (;;)
  {
    stohdr(Rxbytes);
    dlogit(("@rzfile1: sending zrpos %lx", rclhdr(Txhdr)));
    zshhdr(ZRPOS, Txhdr);

  nxthdr:

    if (ZmQueryLocalAbort())  /* Don't use ZmDoLocalAbort because the */
      return ERROR;           /* canit() will be done by ZmodemRecvFile */

    switch (c = zgethdr(Rxhdr, 0))
    {
      default:
        dlogit(("@rzfile1: zgethdr returned %d", c));
	return ERROR;

      case ZNAK:
      case TIMEOUT:
        dlogit(("@rzfile2: zgethdr returned %d", c));

        if (--n < 0)
	  return ERROR;

        continue;

      case ZFILE:
      {
        int temp;

        temp=zrdata(secbuf, 1024);

        dlogit(("@rzfile - zfile(zrdata)=%d", temp));
        continue;
      }

      case ZEOF:
        if (rclhdr(Rxhdr) != Rxbytes)
	{
	  /*
	     * Ignore eof if it's at wrong place - force
	     *  a timeout because the eof might have gone
	     *  out before we sent our zrpos.
	   */
          z_errors = 0;
	  goto nxthdr;
	}

	if (closeit())
	{
	  tryzhdrtype = ZFERR;
          dlogit(("@rzfile: closeit returned <> 0"));
	  return ERROR;
	}

        dlogit(("@rzfile: normal EOF"));
	return c;

      case ERROR:		/* Too much garbage in header search error */
        dlogit(("@rzfile3: zgethdr returned %d", c));

        if (--n < 0)
	  return ERROR;

        dlogit(("@rzfile1 sending attn='%s'", Attn));
        zmputs(Attn);
	continue;

      case ZSKIP:
	closeit();
        dlogit(("@rzfile: Sender SKIPPED file"));
	return c;

      case ZDATA:
        dlogit(("@rzfile: found a zdata header"));

        if (rclhdr(Rxhdr) != Rxbytes)
	{
          dlogit(("@rzfile: zdata hdr at wrong offset - %lx != %lx",
                  (long)rclhdr(Rxhdr), (long)Rxbytes));

          if (--n < 0)
	    return ERROR;

          dlogit(("@rzfile2 sending attn='%s'", Attn));
          zmputs(Attn);
	  continue;
	}

      moredata:

        if (ZmQueryLocalAbort())  /* Don't use ZmDoLocalAbort because the */
          return ERROR;           /* canit() will be done by ZmodemRecvFile */

        ZmStatData(Rxbytes, Rxcount, Crc32);

        /*if (Verbose > 1)
        {
          Lprintf("\r%7ld ZMODEM%s    ",
                  Rxbytes, Crc32 ? " CRC-32" : "");
          vbuf_flush();
        }*/

	switch (c = zrdata(secbuf, 1024))
	{
	  case ZCAN:
            dlogit(("@rzfile4: zgethdr returned %d", c));
	    return ERROR;

	  case ERROR:		/* CRC error */
            dlogit(("@rzfile5: zgethdr returned %d", c));

            if (--n < 0)
	      return ERROR;

            dlogit(("@rzfile3 sending attn='%s'", Attn));
            zmputs(Attn);
	    continue;

	  case TIMEOUT:
	    if (--n < 0)
	    {
              dlogit(("@rzfile6: zgethdr returned %d", c));
	      return ERROR;
	    }
	    continue;

	  case GOTCRCW:
	    n = 20;

            if (putsec(secbuf, Rxcount) != OK)
              return ERROR;

            Rxbytes += Rxcount;
            stohdr(Rxbytes);
	    zshhdr(ZACK, Txhdr);
            zmdm_pputcw(XON);
            flushmo();
	    goto nxthdr;

	  case GOTCRCQ:
	    n = 20;

            if (putsec(secbuf, Rxcount) != OK)
              return ERROR;

            Rxbytes += Rxcount;
            stohdr(Rxbytes);
	    zshhdr(ZACK, Txhdr);
	    goto moredata;

	  case GOTCRCG:
	    n = 20;

            if (putsec(secbuf, Rxcount) != OK)
              return ERROR;

            Rxbytes += Rxcount;
	    goto moredata;

	  case GOTCRCE:
	    n = 20;

            if (putsec(secbuf, Rxcount) != OK)
              return ERROR;

            Rxbytes += Rxcount;
	    goto nxthdr;
	}
	break;

    }
  }
}





/*
 * Initialize for Zmodem receive attempt, try to activate Zmodem sender
 *  Handles ZSINIT frame
 *  Return ZFILE if Zmodem filename received, -1 on error,
 *   ZCOMPL if transaction finished,  else 0
 */

static int near
tryz(char *path)
{
  int c, n;
  int cmdzack1flg;
  int hdr;

  for (n = 15; --n >= 0;)
  {
    /* Set buffer length (0) and capability flags */

    stohdr(0L);

    Txhdr[ZF0] = CANFC32 | CANFDX | CANOVIO | CANBRK;

    if (Zctlesc)
      Txhdr[ZF0] |= TESCCTL;

    dlogit(("@sending the tryz header"));
    zshhdr(tryzhdrtype, Txhdr);
    dlogit(("@done the tryz header"));

    if (tryzhdrtype == ZSKIP)	/* Don't skip too far */
      tryzhdrtype = ZRINIT;	/* CAF 8-21-87 */

  again:

    /* Check if sysop hit <esc> */

    if (ZmDoLocalAbort())
      return ERROR;

    hdr = zgethdr(Rxhdr, 0);
    dlogit(("@got tryz response %d", hdr));

    switch (hdr)
    {
      case ZRQINIT:
	continue;

      case ZEOF:
	continue;

      case TIMEOUT:
	continue;

      case ZFILE:
	zconv = Rxhdr[ZF0];
	zmanag = Rxhdr[ZF1];
/*        ztrans = Rxhdr[ZF2];*/
	tryzhdrtype = ZRINIT;
	c = zrdata(secbuf, 1024);

        dlogit(("@tryz - zrdata=%d", c));

        mode(2);

	if (c == GOTCRCW)
	  return ZFILE;

	zshhdr(ZNAK, Txhdr);
	goto again;

      case ZSINIT:
        dlogit(("@tryz: receiving ZSINIT = %08lx", Rxhdr[ZF0]));
        Zctlesc = TESCCTL & Rxhdr[ZF0];

	if (zrdata(Attn, ZATTNLEN) == GOTCRCW)
	{
	  stohdr(1L);
	  zshhdr(ZACK, Txhdr);
	  goto again;
	}

	zshhdr(ZNAK, Txhdr);
	goto again;

      case ZFREECNT:
        stohdr(getfree(path));
	zshhdr(ZACK, Txhdr);
	goto again;

      case ZCOMMAND:
	cmdzack1flg = Rxhdr[ZF0];

	if (zrdata(secbuf, 1024) == GOTCRCW)
	{
	  if (cmdzack1flg & ZCACK1)
	    purgeline();	/* dump impatient questions */

	  do
	  {
	    zshhdr(ZCOMPL, Txhdr);
	  }
          while (++z_errors < 20 && zgethdr(Rxhdr, 1) != ZFIN);

	  ackbibi();
	  return ZCOMPL;
	}

	zshhdr(ZNAK, Txhdr);
	goto again;

      case ZCOMPL:
	goto again;

      default:
	continue;

      case ZFIN:
	ackbibi();
	return ZCOMPL;

      case ZCAN:
	return ERROR;
    }
  }

  return 0;
}



/*
 * Receive one file with ZMODEM protocol
 */

int ZmodemRecvFile(char *path, char *name, int fInit)
{
  int c;

  fSending = FALSE;

  ThruStart();

  /* If we are running over a telnet connection, escape all control chars */

#ifdef OS_2
  if (GetConnectionType()==CTYPE_TELNET)
    Zctlesc = TRUE;
#endif

  if (!fInit)
    dlogit(("@ZmodemRecvFileNext"));
  else
  {
    dlogit(("@ZmodemRecvFileFirst"));

    mode(1);
    ZmStatInit();
    ZmVarsInit();
    fout=NULL;

    Rxtimeout = 100;
    Rxclob=FALSE;
    Rxbinary=FALSE;
    Rxascii=FALSE;
    tryzhdrtype = ZRINIT;
    Zctlesc=FALSE;
  }

  *name=0;

  if ((c=tryz(path)) == 0 || c == ZCOMPL || c == ERROR)
  {
    dlogit(("@Tryz returned %d", c));
    return c;
  }

  if (c != ZFILE)
    dlogit(("@ZMODEM - %d is not ZFILE!", c));

  *Pathname=0;
  c = rzfile(path);

  dlogit(("@Rzfile returned %d", c));

  switch (c)
  {
    case ZEOF:
      dlogit(("@Successfully recv %s (%d)", Pathname, c));
      strcpy(name, Pathname);
      break;

    case ZSKIP:
      dlogit(("@Skipped file %s", Pathname));
      strcpy(name, Pathname);
      break;

    case ZCOMPL:
      dlogit(("@Zmodem session complete"));
      mode(0);
      break;

    default:
      closeit();

      if (c != ZCOMPL)
        canit();

      dlogit(("@Zmodem error return %c", c));
      break;
  }

  if (*Pathname && c != ZEOF && c != ZCOMPL && *Pathname != '*')
  {
    int rc;

    dlogit(("@Unlinking partial file %s", Pathname));

    rc=unlink(Pathname);

    if (rc != 0)
      dlogit(("!Unlink returned %d", rc));
  }

  return c;
}


