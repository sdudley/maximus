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

#ifdef MEX

  /* Return the number of timer ticks elapsed since Jan 1st 1970 */

  word EXPENTRY intrin_time(void)
  {
    regs_4[0]=time(NULL);
    return 0;
  }


  /* Get the current time, as a mex_stamp-type structure */

  word EXPENTRY intrin_timestamp(void)
  {
    MA ma;
    struct mex_stamp *pms;
    SCOMBO sc;
    word rc;

    /* Get pointer to the stamp structure */

    MexArgBegin(&ma);
    pms=MexArgGetRef(&ma);
    rc=MexArgEnd(&ma);

    /* Get the current time, then store in the Mex structure */

    Get_Dos_Date(&sc);
    StampToMexStamp(&sc, pms);

    regs_2[0]=0;
    return rc;
  }


  /* Function to convert a "ref struct _stamp" into a "long" date
   * long stamp_to_long(ref struct _stamp: st);
   */

  word EXPENTRY intrin_stamp_to_long(void)
  {
    MA ma;
    struct mex_stamp *pms;
    word rc;
    SCOMBO sc;
    struct tm t;

    MexArgBegin(&ma);
    pms = MexArgGetRef(&ma);
    rc = MexArgEnd(&ma);

    MexStampToStamp(pms, &sc);
    DosDate_to_TmDate(&sc, &t);

    regs_4[0] = mktime(&t);
    return rc;
  }


  /* Function to convert a "long" date into a "ref struct _stamp"
   *
   * void long_to_stamp(long: date, ref struct _stamp: st);
   */

  word EXPENTRY intrin_long_to_stamp(void)
  {
    MA ma;
    word rc;
    struct tm *ptm;
    struct mex_stamp *pms;
    SCOMBO sc;
    time_t l;

    MexArgBegin(&ma);
    l = MexArgGetDword(&ma);
    pms = MexArgGetRef(&ma);
    rc = MexArgEnd(&ma);

    ptm = localtime(&l);
    TmDate_to_DosDate(ptm, &sc);
    StampToMexStamp(&sc, pms);

    return rc;
  }



  /* Turn a MEX-type stamp structure into a date string */

  word EXPENTRY intrin_stamp_string(void)
  {
    char szDate[PATHLEN];
    MA ma;
    struct mex_stamp *pms;
    SCOMBO sc;
    word rc;

    /* Get pointer to the stamp structure */

    MexArgBegin(&ma);
    pms=MexArgGetRef(&ma);
    rc=MexArgEnd(&ma);

    /* Convert it to a real stamp structure */

    MexStampToStamp(pms, &sc);

    Timestamp_Format(PRM(dateformat), &sc, szDate);
    strcat(szDate, " ");
    Timestamp_Format(PRM(timeformat), &sc, szDate+strlen(szDate));

    MexReturnString(szDate);

    regs_2[0]=0;
    return rc;
  }



  /* Return the number of minutes remaining for the current call */

  word EXPENTRY intrin_timeleft(void)
  {
    regs_4[0]=timeleft();
    return 0;
  }


  /* Return the length of the current call */

  word EXPENTRY intrin_timeon(void)
  {
    regs_4[0]=timeonline();
    return 0;
  }

  /* Adjust user's time limit, keeping track of the "-t" parameter */

  word EXPENTRY intrin_timeadjustsoft(void)
  {
    MA ma;
    word was_added, added, was_xp_mins;
    long lDelta;

    MexArgBegin(&ma);
    lDelta=MexArgGetDword(&ma);

    /* Save these values */


    was_added=usr.time_added;
    was_xp_mins=usr.xp_mins;

    /* Adjust the time */

    regs_4[0]=Add_To_Time(lDelta);

    /* Fix adjusted data in the user record */

    added=usr.time_added-was_added;
    pmisThis->pmu->time_added+=added;
    ci_timeadd(added);
    if (usr.xp_flag & XFLAG_EXPMINS)
      pmisThis->pmu->xp_mins+=(usr.xp_mins-was_xp_mins);

/*    Printf("Tried to add %ld seconds to time; ended up adding %ld\n",
           lDelta, regs_4[0]);*/

    return MexArgEnd(&ma);
  }


  /* Perform an absolute modification of the user's time remaining */

  word EXPENTRY intrin_timeadjust(void)
  {
    MA ma;
    long lDelta;

    MexArgBegin(&ma);
    lDelta=MexArgGetDword(&ma);

    /* No fancy adjustments done here at all */

    regs_4[0]=(timeoff += lDelta);
    return MexArgEnd(&ma);
  }

#endif /* MEX */

