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

/*# name=^c/^break trap functions
*/

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include "compiler.h"
#include "typedefs.h"

int brk_trapped=0;
static byte brk_is_trapped=0;

#if defined(__MSDOS__)

  #include <dos.h>
  #include <conio.h>
  #include "prog.h"

  static void _intr new_9(void);
  static void _intr trap_break(void);

  static void (_intr *old_9)(void);
  static void (_intr *old_23)(void);
  static void (_intr *old_1b)(void);


  void _fast brktrap(void)
  {
    if (brk_is_trapped)
      return;

    brk_is_trapped=TRUE;

    old_23=(_intcast)getvect(0x23);    /* Dos ^C int */
    old_1b=(_intcast)getvect(0x1b);    /* Bios ^break int */
    old_9=(_intcast)getvect(0x09);     /* 8259 keyboard ISR */

    setvect(0x23, (_veccast)trap_break);
    setvect(0x1b, (_veccast)trap_break);
    setvect(0x09, (_veccast)new_9);
  }


  void _stdc brkuntrap(void)
  {
    if (!brk_is_trapped)
      return;

    brk_is_trapped=FALSE;

    setvect(0x23, (_veccast)old_23);     /* replace old ints */
    setvect(0x1b, (_veccast)old_1b);
    setvect(0x09, (_veccast)old_9);
  }


  static void _intr new_9(void)
  {
    static char far *shift_state=(char far *)0x417;

    static int scancode,
               kcode;

    if (*shift_state & 4)     /* CTRL pressed? */
    {
      scancode=inportb(0x60);

      if (scancode==46 || scancode==70)     /* 'C' or <break> key? */
      {
        brk_trapped++;

        /* ACK key */
        kcode=inportb(0x61);
        outportb(0x61, kcode | 0x80);
        outportb(0x61, kcode);

        /* signal EOI (end of interrupt) to 8259 */
        outportb(0x20, 0x20);
        return;         /* IRET */
      }
    }

    (*old_9)();         /* Chain to next interrupt handler */
  }


  static void _intr trap_break(void)
  {
    brk_trapped++;                                         /* inc every time user hits ^break */
  }

#elif defined(OS_2) && defined(__FLAT__) /* OS/2 2.0 */

  #define INCL_DOS
  #include <os2.h>

  #include <stdio.h>


  static EXCEPTIONREGISTRATIONRECORD *perr;
  static FILE *fpOut;
  static int fTrapTrap = FALSE;

  static void outfn(char *fmt, ...)
  {
    va_list va;


    if (fpOut)
    {
      va_start(va, fmt);
      vfprintf(fpOut, fmt, va);
      va_end(va);
    }

    va_start(va, fmt);
    vfprintf(stdout, fmt, va);
    va_end(va);
  }

  /* Determine if a specific memory address can be read w/o
   * an access violation */

  static int MemAddrOk(unsigned long ulAddr)
  {
    ULONG rc;
    ULONG ulRegSize=1;
    ULONG ulAllocFlags;

    rc=DosQueryMem((PVOID)ulAddr, &ulRegSize, &ulAllocFlags);

    return (rc==0 && (ulAllocFlags & PAG_READ));
  }

  static void WalkStack(unsigned long eip, unsigned long ebp)
  {
    static int cnt=0;

    outfn("Stk at %08lx:\n", eip);

    while (MemAddrOk(ebp))
    {
      ULONG ulAddr=*(unsigned long *)(ebp+4);

      if (ulAddr==0xffffffffLu)
        break;

      outfn("%08lx%c", ulAddr, (++cnt % 5)==0 ? '\n' : ' ');

      /* Traverse the next link */

      ebp=*(unsigned long *)ebp;
    }

    outfn("\n");
  }



  /* Handler for ^c's and ^brk's */

  static ULONG _System ExceptionHandler(PEXCEPTIONREPORTRECORD perepr,
                                        struct _EXCEPTIONREGISTRATIONRECORD *peregr,
                                        PCONTEXTRECORD pcr,
                                        PVOID pv)
  {
    static int i;
    static PTIB ptib;
    static PPIB ppib;
    static char szFile[30];
    static ULONG ulTime;
    static ULONG ulMustCount=0;

    NW(peregr);
    NW(pv);

    switch (perepr->ExceptionNum)
    {
      case XCPT_SIGNAL:
        if (perepr->ExceptionInfo[0]==XCPT_SIGNAL_INTR ||
            perepr->ExceptionInfo[0]==XCPT_SIGNAL_BREAK)
        {
          /*DosBeep(500, 10L);*/
          DosAcknowledgeSignalException(perepr->ExceptionInfo[0]);
          brk_trapped++;
          return XCPT_CONTINUE_EXECUTION;
        }
        break;

      case XCPT_GUARD_PAGE_VIOLATION:
      case XCPT_UNABLE_TO_GROW_STACK:
      case XCPT_ACCESS_VIOLATION:
      case XCPT_INTEGER_DIVIDE_BY_ZERO:
      case XCPT_FLOAT_DIVIDE_BY_ZERO:
      case XCPT_ILLEGAL_INSTRUCTION:
      case XCPT_PRIVILEGED_INSTRUCTION:
      case XCPT_BAD_STACK:
      case XCPT_INVALID_UNWIND_TARGET:
        if (!fTrapTrap)
          break;

        DosEnterMustComplete(&ulMustCount);
        DosGetInfoBlocks(&ptib, &ppib);
        sprintf(szFile, "%d.trp", ppib->pib_ulpid);
        fpOut=fopen(szFile, "a");

        outfn("ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ\n");
        outfn("FATAL ERROR.  The program encountered an exception violation.\n");
        outfn("Please record the following information and report it to the author.\n");

        if (fpOut)
          outfn("--> This report is being saved to file '%s' in the current directory.\n",
                szFile);

        outfn("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n");

        outfn("type: %08lx address: %08lx time: %08lx\n",
              (long)perepr->ExceptionNum,
              (long)perepr->ExceptionAddress,
              (long)time(&ulTime));

        outfn("flags: %08lx params: ", perepr->fHandlerFlags);

        for (i=0; i < perepr->cParameters; i++)
          outfn("[%d]=%08lx ", i, perepr->ExceptionInfo[i]);

        outfn("\n");

        if (pcr->ContextFlags & CONTEXT_CONTROL)
        {
          outfn("cs:eip=%04x:%08lx  ss:esp=%04x:%08lx\n",
                 pcr->ctx_SegCs, pcr->ctx_RegEip,
                 pcr->ctx_SegSs, pcr->ctx_RegEsp);
        }

        if (pcr->ContextFlags & CONTEXT_SEGMENTS)
        {
          if (pcr->ContextFlags & CONTEXT_CONTROL)
            outfn("cs=%04x  ", pcr->ctx_SegCs);

          outfn("ds=%04x  es=%04x  fs=%04x  gs=%04x",
                 pcr->ctx_SegDs, pcr->ctx_SegEs,
                 pcr->ctx_SegFs, pcr->ctx_SegGs);

          if (pcr->ContextFlags & CONTEXT_CONTROL)
            outfn(" ss=%04x", pcr->ctx_SegSs);

          outfn("\n");
        }

        if (pcr->ContextFlags & CONTEXT_INTEGER)
        {
          outfn("eax=%08lx  ebx=%08lx  ecx=%08lx  edx=%08lx\n",
                pcr->ctx_RegEax, pcr->ctx_RegEbx,
                pcr->ctx_RegEcx, pcr->ctx_RegEdx);

          outfn("esi=%08lx  edi=%08lx",
                 pcr->ctx_RegEsi, pcr->ctx_RegEdi);

          if (pcr->ContextFlags & CONTEXT_CONTROL)
            outfn(" ebp=%08lx  efl=%08lx",
                   pcr->ctx_RegEbp, pcr->ctx_EFlags);

          outfn("\n");
        }

        if (pcr->ContextFlags & CONTEXT_CONTROL)
        {
          outfn("ESPd: ");

          for (i=0; i < 256; i++)
          {
            if (MemAddrOk(pcr->ctx_RegEsp+i))
              outfn("%02x", *(unsigned char *)(pcr->ctx_RegEsp+i));
            else
              break;

            if (((i+1) % 32)==0 && i != 255)
              outfn("\n      ");
          }

          outfn("\nEBPd: ");

          for (i=0; i < 32; i++)
          {
            if (MemAddrOk(pcr->ctx_RegEbp+i))
              outfn("%02x", *(unsigned char *)(pcr->ctx_RegEbp+i));
            else
              break;
          }

          outfn("\nEIPd: ");

          for (i=0; i < 32; i++)
          {
            if (MemAddrOk(pcr->ctx_RegEip+i))
              outfn("%02x", *(unsigned char *)(pcr->ctx_RegEip+i));
            else
              break;
          }

          outfn("\n");

        }

        DosBeep(500, 10L);

        if (pcr->ContextFlags & CONTEXT_CONTROL)
          WalkStack((unsigned long)perepr->ExceptionAddress, pcr->ctx_RegEbp);

        outfn("ÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ\n");
        fclose(fpOut);

        DosExitMustComplete(&ulMustCount);
        DosUnsetExceptionHandler(peregr);
        return XCPT_CONTINUE_EXECUTION;
    }

    return XCPT_CONTINUE_SEARCH;
  }


  void _fast brktrapos2(void *err, int fDoTrap)
  {
    ULONG ulSigCount;
    ULONG rc;

    if (brk_is_trapped)
      return;

    fTrapTrap = fDoTrap;

    perr = err;
    perr->prev_structure=0;
    perr->ExceptionHandler=ExceptionHandler;

    if ((rc=DosError(FERR_DISABLEEXCEPTION | FERR_DISABLEHARDERR)) != 0)
      printf("DosError rc=%d\n", rc);

    if ((rc=DosSetExceptionHandler(perr)) != 0)
    {
      printf("Can't register exception handler - %lu\n", rc);
      return;
    }

    if ((rc=DosSetSignalExceptionFocus(SIG_SETFOCUS, &ulSigCount)) != 0)
    {
      printf("DosSetSignalExceptionFocus rc=%d\n", rc);
    }

    brk_is_trapped=TRUE;
    brk_trapped=0;
  }

  void _stdc brkuntrap(void)
  {
    ULONG ulSigCount;
    int rc;

    if (!brk_is_trapped)
      return;

    if ((rc=DosSetSignalExceptionFocus(SIG_UNSETFOCUS, &ulSigCount)) != 0)
    {
      printf("DosSetSignalExceptionFocus rc=%d\n", rc);
    }

    if ((rc=DosUnsetExceptionHandler(perr)) != 0)
      printf("DosUnsetExceptionHandle rc=%d\n", rc);

    brk_is_trapped=FALSE;
  }


#elif defined(OS_2)

  #include <stdio.h>
  #include <dos.h>
  #include <conio.h>
  #include "prog.h"


  #define INCL_DOSSIGNALS
  #include <os2.h>
  #include <stdio.h>

  static PFNSIGHANDLER pfnc;
  static PFNSIGHANDLER pfnb;
  static USHORT actc;
  static USHORT actb;


  static VOID PASCAL FAR BreakHandler(USHORT usSigArg, USHORT usSigNum)
  {
    NW(usSigArg);

    brk_trapped++;

    if (usSigNum==SIG_CTRLBREAK)
    {
      DosSetSigHandler(BreakHandler, &pfnb, &actb, SIGA_ACCEPT,
                       SIG_CTRLBREAK);
    }
    else
    {
      DosSetSigHandler(BreakHandler, &pfnc, &actc, SIGA_ACCEPT,
                       SIG_CTRLC);
    }


    return;
  }

  void _fast brktrap(void)
  {

    if (!brk_is_trapped)
    {
      brk_is_trapped=TRUE;

      DosSetSigHandler(BreakHandler, &pfnc, &actc, SIGA_ACCEPT,
                       SIG_CTRLC);

      DosSetSigHandler(BreakHandler, &pfnb, &actb, SIGA_ACCEPT,
                       SIG_CTRLBREAK);
    }
  }


  void _stdc brkuntrap(void)
  {
    if (brk_is_trapped)
    {
      brk_is_trapped=FALSE;

      DosSetSigHandler(pfnc, &pfnc, &actc, actc, SIG_CTRLC);
      DosSetSigHandler(pfnb, &pfnb, &actb, actb, SIG_CTRLBREAK);

      fflush(stdout);
    }
  }

#elif defined(NT)   /* Windows NT */

  #include "pwin.h"

  /* Handler for ^c/^break events */

  BOOL WINAPI BreakHandler(DWORD dwCtrlType)
  {
    if (dwCtrlType==CTRL_C_EVENT || dwCtrlType==CTRL_BREAK_EVENT)
    {
      brk_trapped++;
      return TRUE;
    }

    return FALSE;
  }

  /* Add the ^c/^brk handler */

  void _fast brktrap(void)
  {
    if (!brk_is_trapped)
    {
      brk_is_trapped=TRUE;
      SetConsoleCtrlHandler(BreakHandler, TRUE);
    }
  }

  /* Remove break handler */

  void _stdc brkuntrap(void)
  {
    if (brk_is_trapped)
    {
      brk_is_trapped=FALSE;
      SetConsoleCtrlHandler(BreakHandler, FALSE);
      fflush(stdout);
    }
  }

#elif defined(UNIX)

  void _fast brktrap(void)
  {
    int fix_me_later;
  }

  void _stdc brkuntrap(void)
  {
    int write_me_later;
  }

#else
  #error unknown operating system
#endif


#if 0
main()
{
  int i;

  brktrap();

  for (i=0; i < 500; i++)
    printf("cowabunga, dude!\n");

  brkuntrap();
}
#endif

