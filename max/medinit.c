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
static char rcs_id[]="$Id: medinit.c,v 1.1.1.1 2002/10/01 17:52:18 sdudley Exp $";
#pragma on(unreferenced)

#ifdef OS_2

#include <stdio.h>
#include <string.h>
#define INCL_NOPM
#define INCL_DOS
#define INCL_VIO
#include <os2.h>
#include "snserver.h"
#include "ffind.h"
#include "compiler.h"
#include "typedefs.h"
#include "modem.h"

extern  void near _loadds StackOvl(int err);



extern void far pascal InitializeMED(void); /* for MultiScope postmortem debugger */
extern  int pascal far ZeroDiv(void );
extern  int LogOpen(void );
extern  void LogClose(void );
extern  void LogFlush(void );
extern  int LogWrite(char *fmt);

static void near medinit2(void)
{
#ifndef __FLAT__
    PFN oldvect;
    extern void ( near _loadds * _aexit_rtn)(int errorlevel);

  #ifdef MULTISCOPE
    InitializeMED();
  #endif

    DosSetVec(VECTOR_DIVIDE_BY_ZERO, ZeroDiv, &oldvect);

  /* _aexit_rtn = StackOvl;*/
#endif
}

#ifndef __FLAT__
int pascal far ZeroDiv(void)
{
    char *msg = "\r\nDivide by zero\r\n";
    char far *null = 0L;
    VioWrtTTY(msg, strlen(msg), 0);
    *null = 0;                  /* cause GP fault */
    return(0);
}
#endif

#ifdef NEVER
#pragma check_stack(off)
void near _loadds StackOvl(int err)
{
    int far *null = 0L;
    *null = err;                /* cuase GP fault */
}
#pragma check_stack()
#endif


static void _loadds near textify(void);

void pascal far _loadds CleanUp(USHORT flag)
{
    HFILE hand;
    char *msg;
    char str[200];
#ifdef __FLAT__
    ULONG bytes;
#else
    USHORT bytes;
#endif

    setvbuf(stdout, NULL, _IONBF, 0);
    hand = ComGetFH(hcModem);
    if( hand == -1 ) hand = 0;
    if(!ComIsOnline(hcModem)) hand = 0;
    switch(flag){
        case TC_EXIT       :  /* Normal exit */
            msg = "normally[0m";
            break;
        case TC_HARDERROR  :  /* Hard-error abort */
            msg = "due to a hardware error";
            break;
        case TC_KILLPROCESS:  /* Unintercepted DosKillProcess */
            msg = "because it was killed by another process";
#if 0
            if(check4pmd())
                strcat(msg, " (MED kicked in too)");
#endif
            break;
        case TC_TRAP       :  /* Trap operation */
            msg = "because of an exception trap";
            break;
        default:
            msg = "for unknown reasons";
            break;
    }
    sprintf(str, "\n[0m[1mThe BBS has ended %s.\r\n", msg);
    printf(str);
    if(hand)
        DosWrite(hand, str, strlen(str), &bytes);
    if(flag != TC_EXIT){
        sprintf(str, "Please wait while an attempt is made to record this bad news...\r\n[0m");
        printf(str);
        if(hand){
            DosWrite(hand, str, strlen(str), &bytes);
            DosBufReset(hand);
        }
        LogWrite("!!!! Uh oh.\n");
        textify();
        /*DosSleep(3000L);*/
        sprintf(str,"[0mThank you.\r\n");
        printf(str);
        if(hand){
            DosWrite(hand, str, strlen(str), &bytes);
            DosBufReset(hand);
            com_DTR_off();
        }
    }
    LogClose();
    fflush(stdout);
    DosExitList(EXLST_EXIT, 0L);
}

void medinit(void)
{
    extern byte no_traptrap;

    medinit2();

    if (!no_traptrap)
    {
      #ifdef __FLAT__
        DosError(FERR_DISABLEHARDERR | FERR_DISABLEEXCEPTION);
      #else
        DosError(EXCEPTION_DISABLE | HARDERROR_DISABLE);
      #endif

      DosExitList(EXLST_ADD, (PFNEXITLIST)CleanUp);
    }
}

void medfini(void)
{
  extern byte no_traptrap;

  if (!no_traptrap)
    DosExitList(EXLST_REMOVE, (PFNEXITLIST)CleanUp);
}

#define GRAPHICS_MODE 0x02

static void _loadds near textify(void)
{
    VIOMODEINFO viomi;
    int col, blanklines;
    USHORT row;
    char buf[200], str[200];

    viomi.cb = sizeof(viomi);
    VioGetMode(&viomi, 0);
    if(viomi.col > sizeof(buf) || (viomi.fbType & GRAPHICS_MODE))
        return;  /* won't work in graphics mode */
    buf[viomi.col] = 0;
    blanklines = 0;
    for(row=0; row < viomi.row; row++){
        VioReadCharStr(buf, &viomi.col, row, 0, 0);
        for(col = viomi.col-1; col>=0; col--){ /* find last blank (eol) */
            if( buf[col] && buf[col] !=' ')
                break;
        }
        if(col == -1){
            blanklines++;           /* blank line */
        }
        else{
            buf[col+1] = 0;
            while(blanklines){
                LogWrite("! CRASH ³\n");
#ifdef MAXSNOOP
                SnWrite("! CRASH ³\n");
#endif
                blanklines--;
            }
            sprintf(str, "! CRASH ³%s\n", buf);
            LogWrite(str);
#ifdef MAXSNOOP
            SnWrite(str);
#endif
        }
    }
}

#endif /* OS_2 */

