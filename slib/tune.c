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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include "prog.h"

#if defined(__MSDOS__) && defined(__386__)
  int _fast do_tune(FILE *tunefile,int (_stdc *chkfunc)(void),int dv)
  {
    NW(tunefile); NW(chkfunc); NW(dv);
    printf("fatal: do_tune not implemented\n");
    exit(1);
    return 0;
  }

  int _fast play_tune(char *filespec,char *name,int (_stdc *chkfunc)(void),int dv)
  {
    NW(filespec); NW(name); NW(chkfunc); NW(dv);

    printf("fatal: play_tune not implemented\n");
    exit(1);
    return 0;
  }

#else /* !(MSDOS && 386) */

  static char *delims=" \t\n";

  #if defined(OS_2) || defined(NT)
    void (_fast *noisefunc)(int, int) = noise;
  #elif defined(__MSDOS__)
    void (_fast *noisefunc)(int, int);
    volatile unsigned long far *biostick=(unsigned long far *)0x000046c;
  #else
    #error Unknown OS
  #endif

  int _fast do_tune(FILE *tunefile,int (_stdc *chkfunc)(void),int dv)
  {
    char temp[PATHLEN];
    char *s;

    long cum_dur=0L;
    unsigned last_freq=(unsigned)-1;
    unsigned freq, duration;
    unsigned ticks;

  #ifdef __MSDOS__
    unsigned long start;

    start=*biostick;
  #else
    NW(dv);
  #endif

    while (fgets(temp, PATHLEN, tunefile))
    {
      Strip_Trailing(temp, '\n');

      s=temp;

      while (*s==' ' || *s=='\t')
        s++;

      if (*s=='#' || *s=='\0')
        break;

      s=strtok(s, delims);

      while (s)
      {
        if (last_freq==(unsigned)-1)
        {
          freq=atoi(s);
          s=strtok(NULL, delims);
        }
        else
        {
          freq=last_freq;
          last_freq=(unsigned)-1;
        }

        if (!s)
        {
          last_freq=freq;
          break;
        }

        duration=atoi(s);

        ticks=duration/(1000/18);

        if (duration != 0)
        {
          if (ticks==0)
            ticks++;

          cum_dur += ticks;
        }

        s=strtok(NULL, delims);
        (*noisefunc)(freq, duration);

        if (chkfunc && (*chkfunc)())
        {
          (*noisefunc)(0, 0);
          return -1;
        }
      }
    }

  #ifdef __MSDOS__
    if (dv)
      while (*biostick < start+cum_dur)
        if (chkfunc && (*chkfunc)())
        {
          (*noisefunc)(0,0);
          return -1;
        }
  #endif

    (*noisefunc)(0,0);
    return 0;
  }


  int _fast play_tune(char *filespec,char *name,int (_stdc *chkfunc)(void),int dv)
  {
    FILE *tunefile;

    char temp[PATHLEN];
    char *ln;

    int play_tune=-1;
    unsigned got_it=FALSE;
    unsigned tune_num=0;

    /* Play a tune by ordinal? */

    if (*name=='*')
      play_tune=atoi(name+1);

    #ifdef __MSDOS__
    noisefunc=(dv ? dv_noise : noise);
    #endif

    if ((tunefile=fopen(filespec, "r"))==NULL)
    {
      strcpy(temp, filespec);
      strcat(temp, ".bbs");

      if ((tunefile=fopen(temp, "r"))==NULL)
        return -1;
    }

    /* Read until EOF */

    while (fgets(temp, PATHLEN, tunefile))
    {
      ln=temp;

      while (*ln==' ' || *ln=='\t')
        ln++;

      if (*ln != '*')
        continue;

      Strip_Trailing(ln, '\n');

      for (++ln; *ln==' ' || *ln=='\t'; ln++)
        ;

      if (eqstri(ln, name) || tune_num==(unsigned)play_tune)
      {
        got_it=TRUE;

        if (do_tune(tunefile, chkfunc, dv))
          break;
      }

      tune_num++;
    }

    fclose(tunefile);

    return got_it;
  }
#endif /* !(MSDOS && 386) */

