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
static char rcs_id[]="$Id: maxpipe.c,v 1.1.1.1 2002/10/01 17:57:27 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <process.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#define INCL_NOPM
#define INCL_DOS
#define INCL_VIO
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include "pos2.h"

#include "comm.h"

#ifdef __FLAT__
  #define DosMakePipe           DosCreatePipe
  #define DosBufReset           DosResetBuffer
  #define DosSendSignal(p, s)   DosSendSignalException(p, 4)

  #ifdef _loadds
  #undef _loadds
  #endif

  #ifdef FAR
  #undef FAR
  #endif

  #ifdef far
  #undef far
  #endif

  #ifdef VOID
  #undef VOID
  #endif

  #define VOID void
  #define far
  #define FAR
  #define _loadds
#endif

extern  int DupHandles(void );
extern  int StartThreads(void );
extern  void CloseHandles(void );
static  void far _loadds WatchDogThread(void *pv);
static  int CHAR_AVAIL(void );
static  BYTE GET_CHAR(void );
static  void far _loadds StdinThread(void *pv);
static  void far _loadds StdoutThread(void *pv);

#define STACK1 16384
#define STACK2 16384
#define STACK3 16384

static HCOMM hfCom;         /* modem */
static HFILE hfStdoutR;
static HFILE hfStdoutW;
static HFILE hfStdinR;
static HFILE hfStdinW;
static HFILE oldStdin, oldStdout, oldStderr;
static PID pidProc;
int alive =  TRUE;
int large = 0;


int main(int argc, char **argv)
{
  int rc1, rc2;
  PID pidd;
  RESULTCODES rescResults;
  int hf;

  if (argc < 3 )
  {
    printf("\nMAXPIPE  Maximus Remote Pipe Facility  Version 1.20\n");
    printf("(c) 1995 by Peter Fitzsimmons and Lanius Corporation.  All rights reserved.\n\n");

    printf("Usage:  MAXPIPE <comm handle> <program.exe> [arguments]\n");
    return 255;
  }

  hf=(HFILE)atoi(argv[1]);

  if (hf <= 0 || hf > 255)
  {
    if (ComOpen(argv[1], &hfCom, 0, 0))
      hfCom = 0;
  }
  else ComHRegister(hf, &hfCom, 0, 0);

  if (!DupHandles() || !StartThreads())
    return 255;
  else
  {
    char szFailName[120];
    char szCmdLine[255];
    char *p;
    int i;

    for (i=2, p=szCmdLine; i < argc; i++)
    {
      strcpy(p, argv[i]);

      if (i==2 && !strchr(p, '.'))
        strcat(p, ".exe");

      if (i==2)
        p += strlen(p)+1;
      else
      {
        strcat(p, " ");
        p += strlen(p);
      }
    }

    p[-1]='\0';
    p[0]='\0';

    rc1=DosExecPgm(szFailName, 120, EXEC_ASYNCRESULT, szCmdLine, NULL, &rescResults,
                   szCmdLine);

    pidProc=rescResults.codeTerminate;

    if (pidProc != -1)
      rc2=DosCwait(DCWA_PROCESSTREE, DCWW_WAIT, &rescResults, &pidd, pidProc);
  }

  if (hfCom)
    ComTxWait(hfCom, 2000L);

  CloseHandles();

  if (pidProc==-1)
    fprintf(stderr, "MAXPIPE: Couldn't spawn %s\n", argv[2]);

  if (rc1 != 0)
    printf("MAXPIPE:  SYS%04d: DosExecPgm()\n", rc1);
  else if (rc2 != 0)
    printf("MAXPIPE:  SYS%04d: DosCwait()\n", rc2);

  return (pidProc==-1 ? 255 : rescResults.codeResult);
}

int DupHandles(void)
{
  HFILE sin = fileno(stdin);
  HFILE sout = fileno(stdout);
  HFILE serr = fileno(stderr);

  oldStdin = oldStdout = oldStderr = 0xffff;
  DosDupHandle(sin,  &oldStdin );
  DosDupHandle(sout, &oldStdout);
  DosDupHandle(serr, &oldStderr);


  if (DosMakePipe(&hfStdoutR, &hfStdoutW, 150) ||
      DosMakePipe(&hfStdinR, &hfStdinW, 150) ||
      DosDupHandle(hfStdinR, &sin) ||
      DosDupHandle(hfStdoutW, &serr) ||
      DosDupHandle(hfStdoutW, &sout))
  {
    fprintf(stderr, "MAXPIPE: Couldn't dup stdin/stdout handles\n");
    return(FALSE);
  }

/*printf("pipe>Std handles duped successfully\n");*/
  return TRUE;
}

void * zalloc(size_t bytes)
{
  void *ret=malloc(bytes);

  if (!ret)
  {
    fprintf(stderr, "MAXPIPE: Couldn't allocate %u bytes\n", bytes);
    exit(3);
  }

  return ret;
}

int StartThreads(void)
{
#ifdef __FLAT__
  static char stack1[STACK1];
  static char stack2[STACK2];
  static char stack3[STACK3];
#else
  BYTE *stack1, *stack2, *stack3;

  stack1=zalloc(STACK1);
  stack2=zalloc(STACK2);
#endif

#ifdef __FLAT__
  if (_beginthread(StdinThread, stack1, STACK1, NULL)==-1 ||
      _beginthread(StdoutThread, stack2, STACK2, NULL)==-1)
  {
    fprintf(stderr, "MAXPIPE: Couldn't create threads\n");
    return FALSE;
  }
#else
  if (DosCreateThread((PFNTHREAD)StdinThread, &tid1, stack1+STACK1) ||
      DosCreateThread((PFNTHREAD)StdoutThread, &tid2, stack2+STACK2))
  {
    fprintf(stderr, "MAXPIPE: Couldn't create threads\n");
    return FALSE;
  }
#endif

/*printf("pipe>Stdin/Stdout threads created successfully\n");*/

  if (hfCom)
  {
#ifdef __FLAT__
    if (_beginthread(WatchDogThread, stack3, STACK3, NULL)==-1)
    {
      fprintf(stderr, "MAXPIPE: Couldn't create Watchdog thread\n");
      return FALSE;
    }
#else
    stack3=zalloc(STACK3);

    if (DosCreateThread((PFNTHREAD)WatchDogThread, &tid3, stack3+STACK3))
    {
      fprintf(stderr, "MAXPIPE: Couldn't create Watchdog thread\n");
      return FALSE;
    }
#endif

/*  printf("pipe>WatchDog thread created successfully\n");*/
  }

  return TRUE;
}

void CloseHandles(void)
{
  HFILE sin=fileno(stdin);
  HFILE sout=fileno(stdout);
  HFILE serr=fileno(stderr);

  DosClose(hfStdoutR);
  DosClose(hfStdoutW);
  DosClose(hfStdinR);
  DosClose(hfStdinW);
  DosDupHandle(oldStderr, &serr);
  DosDupHandle(oldStdout, &sout);
  DosDupHandle(oldStdin, &sin);
  DosClose(oldStderr);
  DosClose(oldStdout);
  DosClose(oldStdin);
}


#pragma check_stack(off)


/* wake up every 15 seconds and check the carrier */

static VOID FAR _loadds WatchDogThread(void *pv)
{
  (void)pv;

  while (alive)
  {
    DosSleep(1000L);

    if (!ComIsOnline(hfCom))
      DosKillProcess(DKP_PROCESSTREE, pidProc);
    else
      DosBufReset(fileno(stdout));
  }

  DosExit(EXIT_THREAD, 0);
}


static int CHAR_AVAIL(void)
{
  if (kbhit())
    return TRUE;

  if (hfCom)
    return (ComPeek(hfCom) != -1);
  else
    return FALSE;
}


static BYTE GET_CHAR(void)
{
  BYTE c;

  if (kbhit())
    return (BYTE)getch();

  ComRxWait(hfCom, 10000L);

  c = (BYTE)ComGetc(hfCom);

  if (c == 3)
    if (!DosSendSignal(pidProc, SIG_CTRLBREAK))
      ComPurge(hfCom, COMM_PURGE_ALL);

  return c;
}

static VOID FAR _loadds StdinThread(void *pv)
{
  static char linebuf[100];
  OS2UINT bytes, bo;
  BYTE c;

  (void)pv;
  bytes = 0;

  while (alive)
  {
    if (!CHAR_AVAIL())
      DosSleep(1L);
    else
    {
      while (CHAR_AVAIL() && bytes<sizeof(linebuf))
      {
        c=GET_CHAR();

        if (c=='\r')
          c='\n';

        if (c==8) /* backspace */
        {    
          if (bytes > 0)
          {
            bytes--;
            DosWrite(hfStdoutW, "\x8 \x8", 3, &bo);
          }
        }
        else
        {
          linebuf[bytes++] = (char)c;
          DosWrite(hfStdoutW, &c, 1, &bo);
        }
      }

      if (c == '\n')
      {
        DosWrite(hfStdinW, linebuf, bytes, &bo);
        bytes = 0;
      }
    }
  }

  DosExit(EXIT_THREAD, 0);
}


static VOID FAR _loadds StdoutThread(void *pv)
{
  static char bufout[200];
  static char buf[150];
  OS2UINT bytes, i, j;

  (void)pv;

  while (alive)
  {
    DosRead(hfStdoutR, buf, sizeof(buf), &bytes);

    for(i=j=0; i < bytes; i++)
    {
      if (buf[i] == '\n')
        bufout[j++] = '\r';

      bufout[j++] = buf[i];
    }

    if (hfCom)
      ComWrite(hfCom, bufout, j);

    VioWrtTTY(bufout, j, 0);
  }

  DosExit(EXIT_THREAD, 0);
}

