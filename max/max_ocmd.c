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
static char rcs_id[]="$Id: max_ocmd.c,v 1.1.1.1 2002/10/01 17:51:55 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Interpreter for the "%" outside command characters
*/

#define MAX_INCL_COMMS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "prog.h"
#include "dr.h"
#include "mm.h"
#include "max_area.h"
#include "max_file.h"
#include "max_msg.h"

/*  Parse_Outside_Cmd() -- An translator for outside commands.  This       *
 *                         function takes a printf()-style string, and     *
 *                         converts the values contained within to         *
 *                         various system parameters, which may be useful  *
 *                         to the program we're calling.  The currently-   *
 *                         supported translations are:                     *
 *                                                                         *
 *  %A -- The user's first name, all in caps                               *
 *  %a -- Number of previous calls to this system                          *
 *  %b -- Baud rate.  If the user is LOCAL, then 0 will be used.           *
 *  %B -- The user's last name, all in caps                                *
 *  %c -- User's city                                                      *
 *  %C -- The response to the user's last ^oR ("[menu]") command.          *
|*  %d -- The area number of the current message area                      *
|*  %D -- The area number of the current file area                         *
|*  %e -- The user's password                                              *
|*  %E -- The user's screen length, in rows                                *
 *  %f -- User's first name, in mixed case                                 *
 *  %F -- Path to the current file area, with trailing backslash           *
 *  %g -- Whether or not the user has graphics support:                    *
 *          A '0' will be used if the user is TTY,                         *
 *          a '1' will be used if the user has ANSI,                       *
 *          and a '2' will be used if the user has AVATAR.                 *
|*  %G -- Daily DL limit, in kilobytes                                     *
 *  %h -- The user's phone number                                          *
|*  %H -- Number of kilobytes downloaded today                             *
|*  %i -- Total downloads                                                  *
|*  %I -- Total uploads                                                    *
|*  %j -- Minutes on-line, this call                                       *
|*  %J -- Last string user entered for [readln]                            *
 *  %k -- The current node's task number (0 for no task number)            *
@*  %K -- The current task number (padded with zeroes, in hex)             *
 *  %l -- User's last name, in mixed case                                  *
 *  %L -- If the user is LOCAL, then this becomes the string "-k".         *
 *        Otherwise, it becomes the string "-px -by", where 'x' is the     *
 *        port number (1==COM1:, 2==COM2:, etc.), and 'y' is the baud      *
 *        rate.                                                            *
@*  %m -- Name of first file to xfer in xternal transfer                   *
 *  %M -- Path to the current message area, with trailing backlash         *
 *  %n -- User's full name, in mixed case                                  *
 *  %N -- The name of your BBS, as defined in your .CTL file.              *
 *  %o -- User's privilege level
 *  %p -- Port number (0=COM1, 1=COM2, etc.)                               *
 *  %P -- Port number (1=COM1, 2=COM2, etc.)                               *
|*  %q -- Path to the current msg area (NO trailing backslash)             *
|*  %Q -- Path to the current file area (NO trailing backslash)            *
 *  %r -- The user's real name, if applicable                              *
|*  %R -- The last command entered at the menu                             *
 *  %s -- The Sysop's last name                                            *
 *  %S -- The Sysop's first name                                           *
 *  %t -- User's time left in minutes, rounded to nearest minute           *
 *  %T -- User's time left, in seconds                                     *
 *  %u -- User's user number                                               *
 *  %U -- Translates to an underscore ("_").                               *
|*  %v -- Path to the current upload area (with trailing backslash)        *
|*  %V -- Path to the current upload area (NO trailing backslash)          *
|*  %w -- Full path and filename to current FILES.BBS file                 *
|*  %W -- Steady baud rate (as passed via -s)                              *
@*  %x -- Letter of current drive, in uppercase                            *
@*  %X -- Number of last message read in current area                      *
@*  %y -- Date of last call                                                *
@*  %Y -- User's current language number, with 0 being the first, etc      *
@*  %z -- Name of first file to transfer                                   *
 *  %Z -- Translates to user's full name, in caps                          *
|*  %! -- Translates to a '\n' newline                                     *
 *  %% -- Translates to a percent sign                                     */

struct _outstr
{
  char ch;
  int type;  /* 0==string, 1==digit, 2==area number, 3==uppercase string */
  void *p;   /* 4==path (no trailing '\')                                */
};

#define OUT_STR   0
#define OUT_INT   1
#define OUT_BYTE  2
#define OUT_UPSTR 3
#define OUT_NBACKS 4
#define OUT_UINT  5
#define OUT_DWORD 6

static struct _outstr outstr[]=
{
  {'f',OUT_STR    ,(void *)firstname},
  {'a',OUT_UINT   ,(void *)&usr.times},
  {'c',OUT_STR    ,(void *)usr.city},
  {'d',OUT_STR    ,(void *)usr.msg},
  {'h',OUT_STR    ,(void *)usr.phone},
  {'i',OUT_DWORD  ,(void *)&usr.down},
  {'k',OUT_BYTE   ,(void *)&task_num},
  {'n',OUT_STR    ,(void *)usrname},
/*{'q',OUT_NBACKS ,(void *)area.msgpath},*/
  {'r',OUT_STR    ,(void *)usr.name},
  {'u',OUT_UINT   ,(void *)&usr.lastread_ptr},
  {'o',OUT_UINT   ,(void *)&usr.priv},
  {'A',OUT_UPSTR  ,(void *)firstname},
  {'D',OUT_STR    ,(void *)usr.files},
  {'E',OUT_BYTE   ,(void *)&usr.len},
/*{'F',OUT_STR    ,(void *)area.filepath},*/
  {'H',OUT_DWORD  ,(void *)&usr.downtoday},
  {'I',OUT_DWORD  ,(void *)&usr.up},
  {'J',OUT_STR    ,(void *)last_readln},
/*{'M',OUT_STR    ,(void *)area.msgpath},*/
/*{'Q',OUT_NBACKS ,(void *)area.filepath},*/
  {'U',OUT_STR    ,(void *)"_"},
/*{'v',OUT_STR    ,(void *)area.uppath},*/
/*{'V',OUT_NBACKS ,(void *)area.uppath},*/
  {'X',OUT_DWORD  ,(void *)&last_msg},
  {'Y',OUT_BYTE   ,(void *)&usr.lang},
  {'%',OUT_STR    ,(void *)"%"},
  {'\0',0         ,NULL}
};

int Parse_Outside_Cmd(char *parm,char *outparm)
{
  int x;
  
  char temp[PATHLEN];
  char *in;
  char *out;
  char *p;

  struct _outstr *os;

  for (in=parm, out=outparm; *in; in++)
  {
    if (*in != '%')
      *out++=*in;
    else
    {
      /* Abort if a "%" at the end of a line */
      
      if (*++in=='\0')
        break;
      
      for (os=outstr; os->p; os++)
      {
        if (*in==os->ch)
        {
          switch(os->type)
          {
            case OUT_STR: /* String */
              strcpy(out, (byte *)os->p);
              break;

            case OUT_UINT:
            case OUT_INT: /* Int (dec) */
              sprintf(out, os->type==OUT_INT ? pd : pu, *(word *)os->p);
              break;
              
            case OUT_DWORD:
              sprintf(out, pl, *(dword *)os->p);
              break;
              
            case OUT_BYTE: /* Byte (dec) */
              sprintf(out, pd, (word)*(byte *)os->p);
              break;

            case OUT_UPSTR: /* Uppercase string */
              strcpy(out, (byte *)os->p);
              cstrupr(out);
              break;
              
            case OUT_NBACKS: /* String w/o backslash */
              strcpy(out,(char *)os->p);
              
              if ((x=strlen(out)) > 3)
                out[x-1]='\0';
          }
          
          break;
        }
      }
      
      if (! os->p)  /* If it wasn't found in the table, special case */
        switch (*in)
        {
          case 'b':
            if (local)
                strcpy(out, zero);
            else sprintf(out, pl, baud);
            break;

          case 'e':
#ifdef CANENCRYPT
            if (usr.bits & BITS_ENCRYPT)
              strcpy(out, brackets_encrypted);
            else
#endif
              strcpy(out, usr.pwd);
            break;

          case 'g':
            switch (usr.video)
            {
              case GRAPH_TTY:
                strcpy(out, zero);
                break;

              case GRAPH_ANSI:
                strcpy(out, one);
                break;

              case GRAPH_AVATAR:
                strcpy(out, "2");
                break;
            }
            break;

          case 'j':
            sprintf(out, pd, timeonline());
            break;
            
          case 'K':
            sprintf(out, "%02x", task_num);
            break;

          case 'l':
            strcpy(out,(p=firstchar(usrname, ctl_delim, 2))==NULL ?
                   no_last : p);
            break;

#ifndef ORACLE
          case 'm':
            {
              FENTRY fent;

              if (GetFileEntry(0, &fent))
                strcpy(out, fent.szName);
            }
            break;
#endif

          case 'p':
          case 'P':
            if (local)
              sprintf(out, pd, *in=='P' ? 0 : -1);
            else
            {
              #if defined(NT)
                sprintf(out, pd,   ComGetHandle(hcModem));
              #elif defined(OS_2)
                sprintf(out, pd,   ComGetFH(hcModem));
              #else
                sprintf(out, pd,  port+(*in=='P' ? 1 : 0));
              #endif
            }
            break;

          case 'q':
            strcpy(out, MAS(mah, path));

            if ((x=strlen(out)) > 3)
              out[x-1]='\0';
            break;

          case 's':
            strcpy(out,(p=firstchar(PRM(sysop),ctl_delim,2))==NULL ?
                       no_last : p);
            break;

          case 't':
            sprintf(out,pd,timeleft());
            break;
            
          case 'v':
            strcpy(out, FAS(fah, uppath));
            break;

          case 'w': /*ABK 1990-12-31 14:08:57 */
            p=FAS(fah, filesbbs);

            if (*p)
              strcpy(out, p);
            else sprintf(out, ss, FAS(fah, downpath), files_bbs);
            break;
            
          case 'x': /* current drive letter */
            *out=((byte)getdisk() + (byte)'A');
            out[1]='\0';
            break;

          case 'y':
            sc_time(&usr.ludate, out);
            break;
            
#ifndef ORACLE
          case 'z':
            {
              FENTRY fent;

              if (GetFileEntry(0, &fent))
                strcpy(out, fent.szName);
              else *out='\0';

            }
            break;
#endif

          case 'B':
            strcpy(out, (p=firstchar(usrname, ctl_delim, 2))==NULL ?
                   no_last : p);
            cstrupr(out);
            break;

          case 'C':
            sprintf(out, "%c", lastmenu);
            break;

          case 'F':
            strcpy(out, FAS(fah, downpath));
            break;

          case 'G':
            sprintf(out, pd, (word)ClassGetInfo(cls,CIT_DL_LIMIT));
            break;

          case 'L':
            if (local)
              strcpy(out,"-k");
            else sprintf(out, "-p%d -b%ld", port, baud);
            break;

          case 'M':
            strcpy(out, MAS(mah, path));
            break;

          case 'N':
            strcpy(out,PRM(system_name));
            break;

          case 'Q':
            strcpy(out, FAS(fah, downpath));

            if ((x=strlen(out)) > 3)
              out[x-1]='\0';
            break;

          case 'R': /*ABK 1990-12-31 */
            strcpy(out, linebuf);
            *linebuf='\0';
            break;

          case 'S':
            getword(PRM(sysop), temp, ctl_delim, 1);
            strcpy(out, temp);
            break;

          case 'T':
            sprintf(out, pl, timeoff-time(NULL));
            break;

          case 'V':
            strcpy(out, FAS(fah, uppath));

            if ((x=strlen(out)) > 3)
              out[x-1]='\0';
            break;

          case 'W':
            sprintf(out, pl, steady_baud_l ? steady_baud_l : baud);
            break;

          case 'Z':
            strcpy(out, usrname);
            cstrupr(out);
            break;

          case '!':
            out[0]='\n';
            out[1]='\0';
            break;
            
          default:        /* Invalid command */
            *out='\0';
            break;
        }

      out += strlen(out);
    }
  }

  *out='\0';

  return (strlen(outparm));
}


#ifndef ORACLE
#if 0 /* unused */
  void ParseOutsideCmd(char *txt)
  {
    char *buf;
    int maxlen=strlen(txt)+100;

    maxlen=max(maxlen, 256);

    if ((buf=malloc(maxlen))==NULL)
      logit(mem_none);
    else
    {
      Parse_Outside_Cmd(txt, buf);
      strcpy(txt, buf);
      free(buf);
    }
  }
#endif
#endif

