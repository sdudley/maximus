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
static char rcs_id[]="$Id: s_matrix.c,v 1.1.1.1 2002/10/01 17:57:46 sdudley Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Matrix and EchoMail' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"

static int alias_count=0;

int Parse_Matrix(FILE *ctlfile)
{
  int x,y;

  char temp[MAX_LINE],  /* Contains entire line */
       temp2[MAX_LINE], /* Temporary holding tank */
       p[MAX_LINE],     /* First word on line */
       *s2;

  linenum++;

  while (fgets(line,MAX_LINE,ctlfile))
  {
    Strip_Comment(line);

    if (*line)
    {
      strcpy(temp,line);

      getword(line,p,ctl_delim,1);

      if (! *p)
        ;
      else if (eqstri(p,"end"))
        break;
      else if (eqstri(p,"address"))
      {
        if (alias_count==ALIAS_CNT)
        {
          printf("\n\aToo many network addresses on line %d of CTL file!\n",
                 linenum);
          Compiling(-1,NULL,NULL);
        }
        else
        {
          getword(line,p,ctl_delim,2);

          prm.address[alias_count].zone=1;
          prm.address[alias_count].net=-1;
          prm.address[alias_count].node=-1;
          prm.address[alias_count].point=0;

          Parse_NetNode(p,&prm.address[alias_count].zone,
                        &prm.address[alias_count].net,
                        &prm.address[alias_count].node,
                        &prm.address[alias_count].point);

          alias_count++;
        }
      }
      else if (eqstri(p,"gate"))
        prm.flags2 |= FLAG2_gate;
      else if (eqstri(p,"path"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"netinfo"))
        {
          s2=fchar(line,ctl_delim,3);
          Make_Path(prm.net_info,s2);

          if (! direxist(strings+prm.net_info))
            makedir(strings+prm.net_info);
        }
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"fidouser"))
      {
        Make_Filename(prm.fidouser,fchar(line,ctl_delim,2));
      }
      else if (eqstri(p,"nodelist"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"version"))
        {
          getword(line,p,ctl_delim,3);

          if (eqstri(p, "5"))
            prm.nlver=NLVER_5;
          else if (eqstri(p, "6"))
            prm.nlver=NLVER_6;
          else if (eqstri(p, "7"))
            prm.nlver=NLVER_7;
          else if (eqstri(p, "fd"))
            prm.nlver=NLVER_FD;
          else Unknown_Ctl(linenum,p);
        }
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"log"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"echomail"))
        {
          prm.flags |= FLAG_log_echo;

          getword(line,p,ctl_delim,3);

          if (! *p)
          {
            printf("\n\aError!  No EchoToss filename specified on line %d of control file!\n",linenum);
            exit(1);
          }

          Make_Filename(prm.echotoss_name,p);
        }
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"after"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"edit"))
        {
          getword(line,p,ctl_delim,4);
          prm.edit_exit=(char)atoi(p);
        }
        else if (eqstri(p,"echomail"))
        {
          getword(line,p,ctl_delim,4);
          prm.echo_exit=(char)atoi(p);
        }
        else if (eqstri(p,"local"))
        {
          getword(line,p,ctl_delim,4);
          prm.local_exit=(char)atoi(p);
        }
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"message"))
      {
        getword(line,p,ctl_delim,2);

        if (eqstri(p,"show"))
        {
          getword(line,p,ctl_delim,3);
          getword(line,temp2,ctl_delim,5);

          if (eqstri(p,"ctl_a"))
            prm.ctla_priv=Deduce_Priv(temp2);
          else if (eqstri(p,"seenby"))
            prm.seenby_priv=Deduce_Priv(temp2);
          else if (eqstri(p,"private"))
            prm.pvt_priv=Deduce_Priv(temp2);
          else Unknown_Ctl(linenum,p);
        }
        else if (eqstri(p,"send"))
        {
          getword(line,p,ctl_delim,3);

          if (eqstri(p,"unlisted"))
          {
            getword(line,p,ctl_delim,4);
            prm.unlisted_priv=Deduce_Priv(p);

            getword(line,p,ctl_delim,5);
            prm.unlisted_cost=atoi(p);
          }
          else Unknown_Ctl(linenum,p);
        }
        else if (eqstri(p,"edit"))
        {
          getword(line,p,ctl_delim,3);

          if ((x=1,eqstri(p,"ask")) || (x=2,eqstri(p,"assume")))
          {
            getword(line,p,ctl_delim,4);

            if (x==1)   /* ASK */
            {
              x=Deduce_Attribute(p);
              getword(line,p,ctl_delim,5);
              if (x==-20 && (!p || !*p))
                prm.msg_localattach=0;
              else
              {
                y=Deduce_Priv(p);

                if (x==-22)      /* fromfile */
                  prm.msg_fromfile=y;
                else if (x==-20)
                  prm.msg_localattach=y;
                else
                  prm.msg_ask[x]=y;
              }
            }
            else        /* ASSUME */
            {
              x=Deduce_Attribute(p);

              getword(line,p,ctl_delim,5);
              if (x==-20 && (!p || !*p))
                prm.msg_localattach=0;
              else
              {
                y=Deduce_Priv(p);

                if (x==-22)     /* fromfile */
                  prm.msg_fromfile=y;
                else if (x==-20)
                  prm.msg_localattach=y;
                else
                  prm.msg_assume[x]=y;
              }
            }
          }
          else Unknown_Ctl(linenum,p);
        }
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"app") || eqstri(p,"application"))
        ;
      else Unknown_Ctl(linenum,p);
    }

    linenum++;
  }

  linenum++;

  return 0;
}



