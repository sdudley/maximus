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

#include "mexall.h"
#include "modem.h"

#ifdef MEX

  /* Display a string to the user */

  word EXPENTRY intrin_printstring(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);

    if ((s=MexArgGetString(&ma, FALSE)) != 0)
    {
      Printf(percent_s, s);
      free(s);
    }

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }


  /* Display a long integer to the user */

  word EXPENTRY intrin_printunsignedlong(void)
  {
    MA ma;

    MexArgBegin(&ma);

    Printf("%lu", (unsigned long)(dword)MexArgGetDword(&ma));

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_printlong(void)
  {
    MA ma;

    MexArgBegin(&ma);

    Printf("%ld", (long)(sdword)MexArgGetDword(&ma));

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }


  /* Display an integer to the user */

  word EXPENTRY intrin_printunsignedint(void)
  {
    MA ma;

    MexArgBegin(&ma);

    Printf("%u", (unsigned int)(word)MexArgGetWord(&ma));

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_printint(void)
  {
    MA ma;

    MexArgBegin(&ma);

    Printf("%d", (int)(sword)MexArgGetWord(&ma));

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }


  /* Display a single character to the user */

  word EXPENTRY intrin_printchar(void)
  {
    MA ma;

    MexArgBegin(&ma);

    Printf("%c", MexArgGetByte(&ma));

    if (pmisThis->pmid->instant_video)
      vbuf_flush();

    return MexArgEnd(&ma);
  }

  /* Flush the local video buffer */

  word EXPENTRY intrin_vidsync(void)
  {
    vbuf_flush();
    return 0;
  }

  word EXPENTRY intrin_snoop(void)
  {
    MA ma;

    MexArgBegin(&ma);
    regs_2[0]=!!snoop;
    snoop=!!MexArgGetWord(&ma);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_issnoop(void)
  {
    regs_2[0]=!!snoop;
    return 0;
  }


  word EXPENTRY intrin_mdm_command(void)
  {
    MA ma;
    char *psz;
    int rc;

    MexArgBegin(&ma);
    psz=MexArgGetString(&ma,FALSE);
    rc=(word)( psz ? mdm_cmd(psz) : -1);

    regs_2[0] = (rc==-1 ? FALSE : TRUE);

    if (psz)
      free(psz);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_mdm_flow(void)
  {
    MA ma;

    MexArgBegin(&ma);
    if (MexArgGetWord(&ma))
      Mdm_Flow_On();
    else
      Mdm_Flow_Off();
    return MexArgEnd(&ma);
  }

  /* Enable/disable local and/or remote output
   * This works independantly from the snoop setting
   */

  word EXPENTRY intrin_set_output(void)
  {
    MA ma;
    word flag = 0;
    
    MexArgBegin(&ma);

    /* Return current state */

    if (no_local_output)
      flag |= 1;
    if (no_remote_output)
      flag |= 2;
    regs_2[0]=flag;

    /* Set output state from argument */

    flag=MexArgGetWord(&ma);
    no_local_output=(flag&1)?1:0;
    no_remote_output=(flag&2)?1:0;

    return MexArgEnd(&ma);
  }

#endif /* MEX */
