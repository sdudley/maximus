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
static char rcs_id[]="$Id: max_mtsk.c,v 1.2 2003/06/04 23:46:22 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Multitasker autodetect logic
*/

#define MAX_LANG_max_init

#include "mm.h"
#include "mtask.h"

void Config_Multitasker(int log_it)
{
#if defined(OS_2)
  NW(log_it);

  multitasker=MULTITASKER_os2;
#elif defined(NT)
  NW(log_it);

  multitasker=MULTITASKER_nt;
#elif defined(UNIX)
multitasker=MULTITASKER_unix;
#elif defined(__MSDOS__)

  static struct _mtask
  {
    int (far pascal *checkfunc)(void);
    char val;
    void (far pascal *sleepfunc)(void);
    char *msg;
  } mtask[]={{dvloaded,     MULTITASKER_DESQVIEW, dvsleep,    "DESQview"},
              {tvloaded,    MULTITASKER_TOPVIEW,  dvsleep,    "TopView"},
              {winloaded,   MULTITASKER_MSWINDOWS,winsleep,   "Win/Enh386"},
              {mlinkloaded, MULTITASKER_MLINK,    mlinksleep, "Multilink"},
              {ddosloaded,  MULTITASKER_DOUBLEDOS,ddossleep,  "DoubleDOS"},
              {pcmosloaded, MULTITASKER_PCMOS,    pcmossleep, "PC-MOS"},
              {os2loaded,   MULTITASKER_OS2,      os2sleep,   "OS/2"},
	      (unixloaded,  MULTITASKER_UNIX,	  unixsleep,  "UNIX"),
              {NULL,        MULTITASKER_NONE,     spoolsleep, NULL}};
            
  struct _mtask *mt;

  /* If no m'task specified on command-line */

  if (multitasker==-2 || multitasker==MULTITASKER_auto)
  {
    if (multitasker==-2)
      multitasker=prm.multitasker;

    /* If no cmd-line arg, and no setting in CTL file, autodetect           *
     * the multitasker.                                                     */
    
    if (multitasker==MULTITASKER_AUTO)
      for (mt=mtask; mt->checkfunc; mt++)
        if (mt->checkfunc != mlinkloaded && mt->checkfunc != ddosloaded &&
            (*mt->checkfunc)())
        {
          multitasker=mt->val;
          break;
        }
  }

  if (multitasker != MULTITASKER_NONE)
    for (mt=mtask; mt->checkfunc; mt++)
      if (mt->val==multitasker && (*mt->checkfunc)())
      {
        sleeper=mt->sleepfunc;
        
        if (mt->msg && log_it)
          logit(mtask_detected, mt->msg);
  
        break;
      }
    
  if (multitasker==MULTITASKER_NONE || ! mt->checkfunc)
  {
    multitasker=MULTITASKER_none;
    sleeper=spoolsleep;
  }

#else
  #error Unknown OS!
#endif
}

