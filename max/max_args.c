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
static char rcs_id[]="$Id: max_args.c,v 1.7 2004/01/15 01:09:09 paltas Exp $";
#pragma on(unreferenced)

/*# name=Command-line argument processing code
*/

#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "mm.h"
#include "caller.h"
#ifdef UNIX
#include "prm.h"
#include <errno.h>
#endif

#ifndef ORACLE

static void near Unknown_CmdParam(char *param);
static void near ReadCallerInfo(char *path);
static void near Parse_Single_Arg(char *arg);

#if defined(OS_2) && !defined(__FLAT__)
  #define INCL_DOS
  #include <os2.h>

  #include "semtrig.h"

  unsigned trigger_wait=FALSE;
  HSYSSEM hssmBBS;
  SEL selBBS;

  void pascal far MaxSemTriggerClose(USHORT usTermCode)
  {
    NW(usTermCode);
    /* Allow Binkley to run again */

    DosSemClear(hssmBBS);
    DosCloseSem(hssmBBS);
    DosExitList(EXLST_EXIT, 0);
  }


  static void near MaxSemTriggerOpen(SEL sel)
  {
    struct _semtrigger far *pst;

    trigger_wait=TRUE;

    if (DosGetSeg(sel))
    {
      Lprintf("Error!  Invalid shared segment %d!\n", sel);
      quit(ERROR_PARAM);
    }

    selBBS=sel;
    pst=MAKEP(sel, 0);

    if (DosOpenSem(&hssmBBS, pst->szSemName))
    {
      Lprintf("Error!  Can't open system semaphore %s!\n", pst->szSemName);
      quit(ERROR_PARAM);
    }
  }

  void MaxSemTriggerWait(void)
  {
    struct _semtrigger far *pst;

    if (!trigger_wait)
      return;

    pst=MAKEP(selBBS, 0);

    /* We must clear the semaphore so Bink can proceed with its initialization */

    DosSemClear(hssmBBS);

    /* Wait a bit, and then request the semaphore.  This will be cleared      *
     * when Bink has a BBS caller.                                            */

    DosSleep(1000);
    DosSemRequest(hssmBBS, SEM_INDEFINITE_WAIT);

    /* If the mailer wants us to simply quit */

    if (*pst->szSemName=='\0')
      quit(ERROR_PARAM);

    /* Now sert our variables based on the info passed by Bink */

    local=FALSE;
    baud=pst->ulRateDCE;
    steady_baud_l=pst->ulRateDTE;
    port=pst->hfPort-1;
    task_num=pst->usTask;
    fSetTask = TRUE;
    strcpy(arq_info, pst->szARQinfo);

    current_baud=Decimal_Baud_To_Mask((unsigned int)baud);
    steady_baud=Decimal_Baud_To_Mask((unsigned int)steady_baud_l);

    max_time=pst->ulMaxTime;
    getoff=timestart+(max_time*60L);

    /* Clean-up processing for our semaphore */

    DosExitList(EXLST_ADD, MaxSemTriggerClose);
  }
#endif


void Parse_Args(char *ctlname,int argc,char *argv[])
{
  int x;

  char *p1,
       *p2;

  for (x=1;x < argc;x++)
    Parse_Single_Arg(argv[x]);

  p1=strrchr(prmname,'.');
  p2=strrchr(prmname,'\\');

  if (p2==NULL)
  {
    p2=strrchr(prmname,'/');
  }
  
  if (p1)             /* There might be an extension */
  {
    if (p2)           /* There IS a path */
    {
      if (p2 < p1)    /* Path delim was BEFORE '.', so must be extension */
        *p1='\0';
    }
    else
    { 
	*p1='\0';    /* No path, so chop it off */
    }
  }
  strcpy(ctlname, cfancy_fn(prmname));
#ifndef UNIX
  strcat(ctlname, ".Ctl");
  strcat(prmname, ".Prm");
#else
  strcat(ctlname, ".ctl");
  strcat(prmname, ".prm");
#endif
}

#if 0

#pragma on(check_stack);

/* Test for the STK stack overflow handler replacement */

void Recurse(void)
{
  Recurse();
}

#pragma off(check_stack);

#endif

#ifdef UNIX
/** Dynamically choose a task number, based on (*ugh*)
 *  filesystem semaphores.
 */
byte getDynamicTaskNumber(int cleanup)
{
  char *path;
  int  i, fd;
  static char filename[FILENAME_MAX];

  if (cleanup)
  {
    if (filename[0])
      unlink(filename);
    return 0;
  }

  if (offsets && PRM(ipc_path))
    path = PRM(ipc_path);
  else
    path = "/tmp";

  /* First scan for files that don't exist at all */
  for (i=1; i <= 0xff; i++)
  {
    snprintf(filename, sizeof(filename), "%s/max_dtn.%i", /* path */ "/tmp", i);
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd >= 0)
    {
      if (flock(fd, LOCK_EX | LOCK_NB) == 0)
      {
        char buf[32];

	snprintf(buf, sizeof(buf), "%i\n", (int)getpid());
        write(fd, buf, strlen(buf));
	logit("# PID %i running as task %i", (int)getpid(), i);
        return i;
      }
      else
        close(fd);
    }
  }

  filename[0] = (char)0;
  return 0;
}

void dtnCleanUp()
{
  getDynamicTaskNumber(1);  
}
#endif

static void near Parse_Single_Arg(char *arg)
{
  if (*arg=='-' || *arg=='/')
  {
    cstrlwr(arg);

    switch(arg[1])
    {
#ifdef MCP
      case 'a': /* MCP pipe name */
        strcpy(szMcpPipe, arg+2);
        break;
#endif
      case 'b':             /* Baud rate select */
        if ((baud=atol(arg+2)) != 0L)
        {
          local=FALSE;
          current_baud=Decimal_Baud_To_Mask((unsigned int)baud);
        }
        break;

      case 'c':             /* Create USER.BBS if doesn't already exist */
        create_userbbs=TRUE;
        break;

      case 'd':
        switch(arg[2])
        {
          case 'l':
            debuglog=TRUE;
            break;

#ifndef __FLAT__
          case 'a':
          {
            extern int main();
            logit("#main at %04x:%04x", (unsigned short)((unsigned long)main >> 16),
                  (unsigned short)(unsigned long)main);
          }
          break;
#endif

#ifdef TEST_VER
          case 'o':           /* -do: debug overlays */
            debug_ovl=TRUE;
            break;
#endif

#ifdef DMALLOC
          case 'm':           /* -dm: debug malloc() */
          {
            #ifdef DMALLOC
              #ifdef __MSDOS__
                printf("PSP at %04x\n", _psp);
              #endif

              dmalloc_on(arg[3]=='q' ? TRUE : FALSE);
            #endif
            break;
          }
#endif
        }
        break;

      case 'e':
        event_num=(char)atoi(arg+2);
        break;

      case 'j':             /* JAM in command */
        strcpy(linebuf, arg+2);
        break;

      case 'i':
        ReadCallerInfo(arg+2);
        break;

      case 'k':             /* Local mode */
        baud=0;
        current_baud=0;
        local=TRUE;
        waitforcaller=FALSE;
	tcpip=1;
        break;

      case 'l':             /* Log file */
        strcpy(log_name,arg+2);
        break;

      case 'm':             /* Multitasker */
        {
          static struct _mtask
          {
            char letter;
            char mtask;
          } mtasks[]={{'d', MULTITASKER_doubledos},
                      {'u', MULTITASKER_unix},
                      {'q', MULTITASKER_desqview},
                      {'p', MULTITASKER_topview},
                      {'l', MULTITASKER_mlink},
                      {'w', MULTITASKER_mswindows},
                      {'m', MULTITASKER_pcmos},
                      {'a', MULTITASKER_auto},
                      {'n', MULTITASKER_none},
                      {0,0}};

          struct _mtask *m;

          for (m=mtasks; m->letter; m++)
            if (arg[2]==m->letter)
            {
              multitasker=m->mtask;
              break;

            }

          if (! m->letter)
            Unknown_CmdParam(arg);
        }
        break;

      case 'n':
        task_num=(byte)atoi(arg+2);
#ifdef UNIX
	if (task_num == 0)
	  task_num = getDynamicTaskNumber(0);
	if (task_num == 0)
        {
	  logit("!No free tasks!");
	  exit(0);
        }
        else
          atexit(dtnCleanUp);
#endif
        fSetTask = TRUE;
        break;

#if defined(OS_2) && !defined(__FLAT__)
      case 'o':
        MaxSemTriggerOpen((SEL)atoi(arg+2));
        break;
#endif

      case 'p':             /* COM port */
        if (arg[2] == 't')
	{
	    tcpip = TRUE;
	    port=atoi(arg+3)-1;
	}
        else if (arg[2] != 'd')
          port=atoi(arg+2)-1;
        else
        {
          port_is_device=TRUE;
          port=atoi(arg+3)-1;
        }
        break;
        
      case 'r':             /* Restart with logged-in caller still online! */
        restart_system=TRUE;
        break;
        
      case 's':
        steady_baud_l=atol(arg+2);
        steady_baud=Decimal_Baud_To_Mask((unsigned int)steady_baud_l);
        break;

      case 't':             /* Max. time left */
        max_time=atol(arg+2);
        getoff=timestart+(max_time*60L);
        break;

      case 'u':             /* Auto user-editor */
        do_useredit=TRUE;

        if (arg[2]=='q' || arg[2]=='h')
          usr.bits |= BITS_HOTKEYS;
        else
          usr.help=NOVICE;

        break;

      case 'v':
        dsp_set=TRUE;

        switch(arg[2])
        {
          case 'b':
            displaymode=VIDEO_BIOS;
            break;

          case 'i':
            displaymode=VIDEO_IBM;
            break;

#if 0 /* no longer supported */
           case 'd':
            displaymode=VIDEO_DOS;
            break;
            
          case 'f':
            displaymode=VIDEO_FAST;
            break;
            
          case 'o':
            displaymode=VIDEO_FOSSIL;
            break;
#endif
           default:
            Unknown_CmdParam(arg);
        }
        break;

      case 'w':
        waitforcaller=TRUE;
        port_is_device=TRUE;
        local=FALSE;
        break;

      case 'x':
        switch (arg[2])
        {
          case 'c':
            no_dcd_check=TRUE;
            break;

          case 'd':
            no_dtr_drop=TRUE;
            break;
            
         case 'j':
            no_shell=TRUE;
            break;

#ifdef OS_2
          case 't':
            no_traptrap=TRUE;
            break;
#endif

          case 'z':
            no_zmodem=TRUE;
            break;

           default:
            Unknown_CmdParam(arg);
        }
        break;
        
      case 'y':
        strcpy(arq_info, arg+2);
        break;

#ifdef MAXSNOOP
      case 'z':
        SnSetPipeName(arg+2);
        break;
#endif

      default:
        Unknown_CmdParam(arg);
        break;
    }
  }
  else if (eqstri(arg,qmark))
    Unknown_CmdParam(NULL);
  else strcpy(prmname,arg);
}



static void near Unknown_CmdParam(char *param)
{
  if (param)
    Lprintf(huh, param);

  Lprintf("Usage:\n\n");

  Lprintf("  max"
#ifdef OS_2
          "p"
#endif
          " [prm_name] [[args]...]\n\n");

#ifdef OS_2
  Lprintf("  -a<pipename> Name of MCP pipe           -s<speed>   Lock baud rate (bps)\n");
#else
  Lprintf("                                          -s<speed>   Lock baud rate (bps)\n");
#endif
  Lprintf("  -b<speed>    Connection speed (bps)     -t<minutes> Maximum time limit\n");
  Lprintf("  -c           Create 'user.bbs'          -u{q}       User edit (-uq=hotkeys)\n");
#ifdef OS_2
  Lprintf("  -dl          Debug logging              -vi         Select Video IBM\n");
#else
  Lprintf("  -dl          Debug logging              -v{bi}      Select Video BIOS/IBM\n");
#endif
  Lprintf("  -e<num>      Event file number          -w          Wait for caller\n");
  Lprintf("  -j<strokes>  Jam keystrokes into buffer -xc         No DCD checking\n");
  Lprintf("  -i<filename> Read FD caller info        -xd         Don't drop DTR\n");
  Lprintf("  -k           Local mode                 -xj         Don't allow <Alt-J>\n");
  Lprintf("  -m{dqplwman} Multitasker selection\n");
  Lprintf("  -n           Task number                -xz         No internal Zmodem\n");
#ifdef OS_2                                         
  Lprintf("  -p<handle>   Com handle                 -y<info>    Extra modem connect info\n");
  Lprintf("  -pd<port>    Com port number\n");
#elif UNIX
  Lprintf("  -p<port_num> Com port number (modem)    -y<info>    Extra modem connect info\n");  
  Lprintf("  -pt<ipcnum>  Start a TCP/IP task by using Unix sockets\n");
#else
  Lprintf("  -p<port_num> Com port number            -y<info>    Extra modem connect info\n");
#ifdef __MSDOS__
  Lprintf("  -r           Restart from Xtern_Erlvl\n");
#endif
#endif

  quit(ERROR_PARAM);
}

/* Handle FrontDoor's "CALLER.nnn" file */

static void near ReadCallerInfo(char *path)
{
  int fd;
  struct _crec crec;
  
  if ((fd=shopen(path, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
  {
    Lprintf(cantopen+1, path, errno);
    quit(ERROR_PARAM);
  }
  
  if (read(fd, (char *)&crec, sizeof(crec))==sizeof(crec) &&
      crec.iversion==CREC_VER)
  {
    waitforcaller=FALSE;

    if (crec.callrate && crec.comport)
    {
      local=FALSE;
      baud=crec.callrate;
      current_baud=Decimal_Baud_To_Mask((unsigned int)baud);
      
      steady_baud_l=crec.portrate;
      steady_baud=Decimal_Baud_To_Mask((unsigned int)steady_baud_l);
      port=crec.comport-1;
    }
    else
    {
      local=TRUE;
      baud=0L;
      current_baud=0;
    }
    
    max_time=crec.timeleft;
    getoff=timestart+(max_time*60L);
  }
  
  close(fd);
}

#endif /* !ORACLE */

