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
static char rcs_id[]="$Id: max.c,v 1.2 2003/06/13 03:25:37 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "alc.h"
#include "prog.h"
#include "mm.h"

#ifdef __TURBOC__
  extern unsigned cdecl _stklen=16656;
#endif

#if defined(OS_2) && defined(__FLAT__)
  #define INCL_DOS
  #include "pos2.h"
#endif

int _stdc c_main(int argc,char *argv[])
{
  char *ctlname;
  char *key_info;

  /* First thing to do is trap the ^brk interrupt */

#if defined(__FLAT__) && defined(OS_2)
  EXCEPTIONREGISTRATIONRECORD err;

  brktrapos2(&err, TRUE);
#else
  brktrap();
#endif
  maximus_atexit(brkuntrap);

  #if defined(__WATCOMC__)
    #ifndef __386__
      #include <malloc.h>

      /* Bump the near heap up to 64K, since we're using a mixed memory model */

      _heapgrow();

      /* Allocate memory in chunks of 64 bytes -- don't waste anything! */
      _amblksiz=64;
    #endif

    grow_handles(40);
  #endif
    
  /*dmalloc_on(1);*/

  Init_Variables();

#if defined(UNIX)
  chdir(INSTALL_PREFIX);
  if (!getenv("MAXIMUS"))
    putenv("MAXIMUS=" INSTALL_PREFIX "/etc/max.prm");
#endif

  if ((ctlname=(char *)malloc(PATHLEN))==NULL)
    maximus_exit(2);

#ifdef ORACLE
  Oracle_Parse_Args(ctlname,argc,argv);
#else
  Parse_Args(ctlname,argc,argv);
#endif

  Read_Prm(ctlname);

  free(ctlname); /* force the real free() to be called instead of macro */

  Read_Access();  /* Read after PRM to conserve memory */

  key_info=Startup();

#ifdef ORACLE
  Oracle();
  return 0;       /* To shut TC up */
#else
  Read_Stats(&bstats);

  maximus_atexit(FinishUp);

  fflush(stdout);

  gkey_info=key_info;

#ifdef __MSDOS__
  if (restart_system)
    Sys_Rst();
  else
#endif
  {
    if (waitforcaller)
      Wait_For_Caller();

    Login(key_info);

    Display_Options(main_menu, NULL);
  }

  return quit(0);

#endif
}

