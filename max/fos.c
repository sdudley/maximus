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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: fos.c,v 1.5 2004/01/27 21:00:29 paltas Exp $";
#pragma on(unreferenced)
#endif

/*# name=FOSSIL interface routines
*/

#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include "prog.h"
#include "mm.h"
#include "modem.h"
#include "keys.h"

static int waslocal =0;
static int waskeyboard =0;

int Mdm_WasLocalKey(void)
{
  return waslocal;
}

int Mdm_getcw(void) /* Get a character from either the modem or console,    */
{                   /* on current settings, and checking for carrier, etc.  */
  signed int x;
  unsigned int y;
  int timer2;
  
#ifdef DEBUG_OUT
  int c;
#endif

  timer2=FALSE;
  input_timeout=timerset(timeout_tics);

  for (;;)
  {

    if (waskeyboard)
    {
      waslocal=1;
      --waskeyboard;
      return 0;
    }

    /* Loop until something happens */

    while (! loc_kbhit() && (local || !mdm_avail()))
    {
      Check_Time_Limit(&input_timeout,&timer2);
      Check_For_Message(NULL,NULL);
      Giveaway_Slice();
    }

    timer2=FALSE;

    if (loc_kbhit())
    {
      if (local || keyboard)
      {
        if ((x=loc_getch())=='\x1b' && !local)
        {
          Keyboard_Off();
          waskeyboard=2;
          return 0; /* Return something that will be disguarded */
        }
        else
        {
          x &= (Highbit_Allowed() ? 0xff : 0x7f);

          if (x==3)
            brk_trapped=TRUE;

          waslocal=1;
#ifdef DEBUG_OUT
          c=(x & (Highbit_Allowed() ? 0xff : 0x7f));
          break;
#else
          return (x);
#endif
        }
      }
      else
      {
        Parse_Local(loc_getch());

        timer2=FALSE;
        input_timeout=timerset(timeout_tics);
      }
    }
    else
    {
      y=mdm_ggetcw();

      if (y==241 && !fLoggedOn && !fthru_yuhu)
      {
        fthru_yuhu=TRUE;
        logit(log_ft_yuhu);
      }

      waslocal=0;
#ifdef DEBUG_OUT
      c=(y & (Highbit_Allowed() ? 0xff : 0x7f));
      break;
#else
      return (y & (Highbit_Allowed() ? 0xff : 0x7f));
#endif
    }
  }
  
#ifdef DEBUG_OUT
  if (dout_log)
    DebOutGotChar(c);

  return c;
#endif
}


/* Check for a character from either the keyboard or the modem */

int Mdm_kpeek(void) 
{
  return Mdm_kpeek_tic(0);
}

/* Check for a character from either the keyboard or the modem
 * Wait for 'tics' time or until there's a character available
 */

int Mdm_kpeek_tic(word tics)
{
  static int x;
  long end;

  end=tics ? timerset(tics) : tics;

  do
  {
    if (loc_kbhit())
    {
      if (local || keyboard)
      {
#if K_ESC != K_ONEMORE	/* !UNIX (unless we change our minds about the implementation) */
        if ((x=loc_peek()) == K_ESC && !local)
        {
          loc_getch(); /* Throw away character */
          Keyboard_Off();
          waskeyboard=2;
          return 0;
        }
        else
#endif
        {
          if (x == K_CTRLC)
          {
            /* Throw away the character we just got */

            loc_getch();
            brk_trapped=TRUE;
          }

          waslocal=1;
          return x;
        }
      }
      else Parse_Local(loc_getch());
    }
    else if (!local && mdm_avail())
    {
      waslocal=0;
      return mdm_peek();
    }
    else if (tics)
    {
      Check_Time_Limit(NULL,NULL);
      Check_For_Message(NULL,NULL);
      Giveaway_Slice();
    }
  }
  while(tics && !timeup(end));
  return -1;
}



void Mdm_check(void)    /* Check for carrier loss, time limit, etc. */
{
  Check_Time_Limit(NULL,NULL);

  if (loc_kbhit())
  {
    if (local || keyboard)
    {
#if K_ESC != K_ONEMORE /* ! UNIX */
      if (loc_peek()== K_ESC && !local)
      {
        loc_getch(); /* Throw away character */
        Keyboard_Off();
        waskeyboard=2;
      }
#else
      ;
#endif
    }
    else Parse_Local(loc_getch());
  }

  Check_For_Message(NULL,NULL);
}

/* Flush output buffer, but don't let user ^C! */

void Mdm_flush(void)
{
  Mdm_flush_ck_tic(0, FALSE, TRUE);
}

int Mdm_flush_ck(void)
{
  return (Mdm_flush_ck_tic(0, TRUE, TRUE));
}

/* Flush output buffer, but optionally let user ^C! */

word Mdm_flush_ck_tic(word tics, word checkcc, word checkcar)
{
  long end;

  end=timerset(tics);

  vbuf_flush();

  if (!local)
  {
    while (! out_empty()  && (tics==0 || !timeup(end)))
    {
      if (checkcc && halt())
      {
        mdm_dump(DUMP_ALL);
        return TRUE;
      }

      if (!in_wfc && checkcar)
        Mdm_check();

      Giveaway_Slice();
    }
  }

  return FALSE;
}



/* Turn on/off remote ^C/^K checking and XON (^s/^q) flow control */

void Mdm_Flow(int state)
{
  if (local)
    return;

  if (state==FLOW_ON || state==FLOW_NO_CCK)
  {
    (void)mdm_ctrlc(state==FLOW_ON);

    #if defined(OS_2) || defined(NT) || defined(UNIX)
      com_XON_enable();
    #elif defined(__MSDOS__)
      mdm_flow(prm.handshake_mask);
    #else
      #error Unknown OS!
    #endif

    #if defined(NT) || defined(UNIX)
    com_HHS_enable(prm.handshake_mask);
    #endif
  }
  else if (state==FLOW_PARTIAL_OFF)
  {
    /* Turn off control-C/K checking, and use all flow control except      *
     * for XON/XOFF!                                                       */

    (void)mdm_ctrlc(0);
#if defined(OS_2) || defined(NT) || defined(UNIX)
    com_XON_disable();
#elif defined(__MSDOS__)
    mdm_flow(prm.handshake_mask & ~(FLOW_TXOFF|FLOW_RXOFF));
#else
    #error Unknown OS!
#endif

#if defined(NT) || defined(UNIX)
    com_HHS_enable(prm.handshake_mask);
#endif
  }
  else if (state==FLOW_OFF)
  {
    (void)mdm_ctrlc(0);
#if defined(OS_2) || defined(NT) || defined(UNIX)
    com_XON_disable();
#elif defined(__MSDOS__)
    mdm_flow(0);
#else
    #error Unknown OS!
#endif

#if defined(NT) || defined(UNIX)
    com_HHS_disable(prm.handshake_mask);
#endif
  }
}




