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

/* Attempt at a clean 32-bit rewrite of maxpipe.c.
 *
 * Still a bit too slow to use.  Needs to have some buffers added
 * for the input and output threads, so that we don't end up
 * calling the device driver for every single byte.
 */

/*#define MXP_DEBUG*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <process.h>
#include <conio.h>
#define MAX_INCL_VER
#define INCL_SUB
#define INCL_BSE
#define INCL_DOS
#include "pos2.h"
#include "max.h"

typedef struct
{
  HFILE hpRead;
  HFILE hpWrite;
} REDIRPIPE, *PREDIRPIPE;

typedef struct
{
  REDIRPIPE rpStdin;
  REDIRPIPE rpStdout;
  HFILE hfModem;
  char szArgString[PATHLEN*2];
  HFILE hfStdin;
  HFILE hfStdout;
  HFILE hfStderr;
  TID tStdin;
  TID tStdout;

} PIPES;

#define PIPE_SIZE_STDIN   256
#define PIPE_SIZE_STDOUT  8192

#define HANDLE_STDIN      0
#define HANDLE_STDOUT     1
#define HANDLE_STDERR     2

#define STDIN_STK_SIZE    16384
#define STDOUT_STK_SIZE   16384

static char abStdinStk[STDIN_STK_SIZE];
static char abStdoutStk[STDOUT_STK_SIZE];


void dprintf(PIPES *pp, char *fmt, ...)
{
  char sz[PATHLEN];
  ULONG ulWritten;
  va_list va;

  va_start(va, fmt);
  vsprintf(sz, fmt, va);
  va_end(va);

  DosWrite(pp->hfStdout,
           sz,
           strlen(sz),
           &ulWritten);
}

/* Create pipes to be used for input and output */

static int CreatePipes(PIPES *pp)
{
  ULONG rc;

#ifdef MXP_DEBUG
  printf("create pipes\n");
#endif

  rc = DosCreatePipe(&pp->rpStdin.hpRead,
                     &pp->rpStdin.hpWrite,
                     PIPE_SIZE_STDIN);

  if (rc)
    return FALSE;

  rc = DosCreatePipe(&pp->rpStdout.hpRead,
                     &pp->rpStdout.hpWrite,
                     PIPE_SIZE_STDOUT);

  if (rc)
    return FALSE;

  return TRUE;
}


/* Twiddle the output for our client so that it get sent down
 * the pipe instead of to the screen.
 */

static int FiddleOutput(PIPES *pp)
{
  ULONG rc;
  HFILE hfDup;

#ifdef MXP_DEBUG
  printf("fiddle output\n");
#endif

  /* Duplicate our existing file handles */

  pp->hfStdin = -1L;
  rc = DosDupHandle(HANDLE_STDIN, &pp->hfStdin);

  if (rc)
    return FALSE;

  pp->hfStdout = -1L;
  rc = DosDupHandle(HANDLE_STDOUT, &pp->hfStdout);

  if (rc)
    return FALSE;

  pp->hfStderr = -1L;
  rc = DosDupHandle(HANDLE_STDERR, &pp->hfStderr);

  if (rc)
    return FALSE;


  /* Now map the pipes onto stdin/stdout */

  hfDup = HANDLE_STDIN;
  rc = DosDupHandle(pp->rpStdin.hpRead, &hfDup);

  if (rc)
    return FALSE;

  hfDup = HANDLE_STDOUT;
  rc = DosDupHandle(pp->rpStdout.hpWrite, &hfDup);

  if (rc)
    return FALSE;

  hfDup = HANDLE_STDERR;
  rc = DosDupHandle(pp->rpStdout.hpWrite, &hfDup);

  if (rc)
    return FALSE;

  return TRUE;
}


/* Parse the command-line */

static int ParseArgs(PIPES *pp, int argc, char **argv)
{
  char *psz;

#ifdef MXP_DEBUG
  printf("parsing args\n");
#endif

  if (argc < 3)
  {
    printf("Usage:\n\n");

    printf("%s <pipe_handle> <program> [<args>...]\n",
           argv[0]);

    return FALSE;
  }

  pp->hfModem = atoi(*++argv);

  /* Now create the argument string for DosExecPgm */

  psz=pp->szArgString;

  strcpy(psz, *++argv);

  if (!stristr(psz, ".exe"))
    strcat(psz, ".exe");

  psz += strlen(psz)+1;

  while (*++argv)
  {
    strcat(psz, *argv);

    if (argv[1])
      strcat(psz, " ");
  }

  psz[strlen(psz)+1] = 0;

  return TRUE;
}

static void ThreadStdin(void *pv)
{
  PIPES *pp = pv;
  KBDKEYINFO kki;

#ifdef MXP_DEBUG
  dprintf(pp, "ThreadStdin starting\r\n");
#endif

/*  DosSleep(5000);*/

  for (;;)
  {
    int ch;
    ULONG rc;
    ULONG ulWritten;

    rc = KbdCharIn(&kki, IO_WAIT, 0);

    if (rc)
      dprintf(pp," kci rc=%d\r\n",rc);

    ch = kki.chChar;

    if (ch==0x0d)
      ch = 0x0a;

#ifdef MXP_DEBUG
    dprintf(pp, "ThreadStdin got '%c'\r\n", ch);
#endif

    rc = DosWrite(pp->rpStdin.hpWrite,
                  &ch,
                  1,
                  &ulWritten);

    rc = DosWrite(pp->hfStdout,
                  &ch,
                  1,
                  &ulWritten);

    if (ch==0x0a)
    {
      ch = 0x0d;

      rc = DosWrite(pp->hfStdout,
                    &ch,
                    1,
                    &ulWritten);
    }

    if (rc || ulWritten != 1)
    {
      dprintf(pp, "ThreadStdin rc=%d, written=%d\r\n", rc, ulWritten);
      exit(rc);
    }
  }
}


static void ThreadStdout(void *pv)
{
  PIPES *pp = pv;

#ifdef MXP_DEBUG
  dprintf(pp, "ThreadStdout starting\r\n");
#endif

/*  DosSleep(5000);*/

  for (;;)
  {
    int ch;
    ULONG rc;
    ULONG ulRead;
    ULONG ulWritten;

    rc = DosRead(pp->rpStdout.hpRead,
                 &ch,
                 1,
                 &ulRead);

#ifdef MXP_DEBUG
    dprintf(pp, "ThreadStdout got '%c'\r\n", ch);
#endif

    if (rc)
    {
      dprintf(pp, "ThreadStdout rc=%d\r\n", rc);
      exit(rc);
    }

    rc = DosWrite(pp->hfStdout,
                  &ch,
                  1,
                  &ulWritten);

    if (rc)
      exit(rc);

    if (pp->hfModem)
    {
      rc = DosWrite(pp->hfModem,
                    &ch,
                    1,
                    &ulWritten);

      if (rc)
      {
        dprintf(pp, "DosWrite rc=%d\r\n", rc);
        exit(rc);
      }
    }
  }
}


/* Create the two threads to handle I/O on the stdin and stdout pipes */

static int CreatePipeThreads(PIPES *pp)
{
#ifdef MXP_DEBUG
  dprintf(pp, "create pipe threads 1\r\n");
#endif

  pp->tStdin = _beginthread(ThreadStdin,
                            abStdinStk,
                            sizeof(abStdinStk),
                            pp);

#ifdef MXP_DEBUG
  dprintf(pp, "create pipe threads 2 rc=%d\r\n", pp->tStdin);
#endif

  pp->tStdout = _beginthread(ThreadStdout,
                             abStdoutStk,
                             sizeof(abStdoutStk),
                             pp);
#ifdef MXP_DEBUG
  dprintf(pp, "create pipe threads 3\r\n");
#endif

  return (pp->tStdin != -1 && pp->tStdout != -1);
}


static int SpawnProgram(PIPES *pp)
{
  ULONG rc;
  RESULTCODES rcd;
  char szFail[PATHLEN];

#ifdef MXP_DEBUG
  dprintf(pp, "calling DosExecPgm\r\n");
#endif

  rc = DosExecPgm(szFail,
                  sizeof(szFail),
                  EXEC_SYNC,
                  pp->szArgString,
                  NULL,
                  &rcd,
                  pp->szArgString);

  if (rc)
    dprintf(pp, "Could not exec program '%s' - rc=%d\r\n",
            pp->szArgString, rc);

  return TRUE;
}


int main(int argc, char **argv)
{
  PIPES p;

  Hello("MXPIPE32", "Remote I/O Redirection", VERSION, THIS_YEAR);

  if (!ParseArgs(&p, argc, argv))
    return 1;

  if (!CreatePipes(&p))
    return 1;

  if (!FiddleOutput(&p))
    return 1;

  if (!CreatePipeThreads(&p))
    return 1;

#ifdef MXP_DEBUG
  dprintf(&p,
          "@debug - prog='%s', arg='%s'\r\n",
          p.szArgString,
          p.szArgString + strlen(p.szArgString)+1);
#endif

  if (!SpawnProgram(&p))
    return 1;

#ifdef MXP_DEBUG
  dprintf(&p, "done!\r\n");
#endif

  return 0;
}

