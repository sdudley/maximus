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
static char rcs_id[]="$Id: s_sessio.c,v 1.2 2003/06/05 03:18:58 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Session' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"
#include "dr.h"
#include "l_attach.h"

static void Init_Attach_Db(char * szAttDbName);

int Parse_Session(FILE *ctlfile)
{
  int x /*,y */;

/*long tl; */

  char temp[MAX_LINE],  /* Contains entire line */
  /*   temp2[MAX_LINE], // First word on line */
       p[MAX_LINE],     /* First word on line */
       *sp,
       *s2;

  linenum++;

  while (fgets(line,MAX_LINE,ctlfile) != NULL)
  {
    Strip_Comment(line);

    if (! *line)
    {
      linenum++;
      continue;
    }
    
    strcpy(temp,line);

    getword(line,p,ctl_delim,1);

    if (! *p)
      ;
    else if (eqstri(p,"end"))
      break;
    else if (eqstri(p,"min"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"logon"))
      {
        getword(line,p,ctl_delim,4);
        prm.min_baud=atoi(p);
      }
      else if (eqstri(p,"nontty"))
      {
        getword(line,p,ctl_delim,4);
        prm.speed_graphics=atoi(p);
        if (prm.speed_rip==0)
          prm.speed_rip=prm.speed_graphics;
      }
      else if (eqstri(p,"rip"))
      {
        getword(line,p,ctl_delim,4);
        prm.speed_rip=atoi(p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p, "track"))
    {
      getword(line, p, ctl_delim, 2);
      s2=fchar(line, ctl_delim, 3);

      if (eqstri(p, "base"))
        Make_Filename(prm.track_base, s2)
      else if (eqstri(p, "exclude"))
        Make_Filename(prm.track_exclude, s2)
      else if (eqstri(p, "view"))
        Make_String(prm.track_privview, s2)
      else if (eqstri(p, "modify"))
        Make_String(prm.track_privmod, s2)
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p, "attach"))
    {
      getword(line, p, ctl_delim, 2);
      s2=fchar(line, ctl_delim, 3);

      if (eqstri(p, "base"))
      {
        Make_Filename(prm.attach_base, s2)
        Init_Attach_Db(s2);
      }
      else if (eqstri(p, "path"))
      {
        Make_Path(prm.attach_path, s2);
        if (! direxist(s2))
          makedir(s2);
      }
      else if (eqstri(p, "archiver"))
        Make_String(prm.attach_archiver, s2)
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"logon"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"level"))
      {
        getword(line,p,ctl_delim,3);
        prm.logon_priv=Deduce_Priv(p);
      }
      else if (eqstri(p,"preregistered"))
        prm.logon_priv=PREREGISTERED;
      else if (eqstri(p,"timelimit"))
      {
        getword(line,p,ctl_delim,3);
        prm.logon_time=atoi(p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if ((x=1,eqstri(p,"uses")) || (x=2,eqstri(p,"file")))
    {
      getword(line,p,ctl_delim,2);

      if (x==2 && eqstri(p,"date"))
      {
        getword(line,p,ctl_delim,3);

        if ((x=1,eqstri(p,"automatic")) ||
            (x=2,(eqstri(p,"none") || eqstri(p,"manual"))))
        {
          if (x==1)
            prm.flags |= FLAG_autodate;
          else prm.flags &= ~FLAG_autodate;

          getword(line,p,ctl_delim,4);

          if (! *p)
          {
            if (x==1)
              prm.date_style=0;
            else prm.date_style=-1;
          }
          else if (eqstri(p,"mm-dd-yy"))
            prm.date_style=0;
          else if (eqstri(p,"dd-mm-yy"))
            prm.date_style=1;
          else if (eqstri(p,"yy-mm-dd"))
            prm.date_style=2;
          else if (eqstri(p,"yymmdd"))
            prm.date_style=3;
          else Unknown_Ctl(linenum,p);
        }
        else if (eqstri(p,"yymmdd")   || eqstri(p,"mm/dd/yy") ||
                 eqstri(p,"mm-dd-yy") || eqstri(p,"dd") ||
                 eqstri(p,"none"))
          ;
        else Unknown_Ctl(linenum,p);
      }
      else
      {
        s2=fchar(line,ctl_delim,3);

        Add_Filename(s2);

        if (eqstri(p,"welcome"))
        {
          Make_Filename(prm.welcome,s2)

          if (! prm.rookie)
            prm.rookie=prm.welcome;

          if (! prm.newuser2)
            prm.newuser2=prm.welcome;
        }
        else
        {
          static struct _filelist
          {
            char *name;
            OFS *where;
          } fl[] =
            {
              {"logo",            &prm.logo},
              {"application",     &prm.application},
              {"newuser1",        &prm.newuser1},
              {"newuser2",        &prm.newuser2},
              {"rookie",          &prm.rookie},
              {"quote",           &prm.quote},
              {"daylimit",        &prm.daylimit},
              {"timewarn",        &prm.timewarn},
              {"tooslow",         &prm.tooslow},
              {"locatehelp",      &prm.hlp_locate},
              {"contentshelp",    &prm.hlp_contents},
              {"xferbaud",        &prm.xferbaud},
              {"fileareas",       &prm.file_area_list},
              {"msgareas",        &prm.msgarea_list},
              {"opedhelp",        &prm.oped_help},
              {"magnethelp",      &prm.oped_help},
              {"maxedhelp",       &prm.oped_help},
              {"lorehelp",        &prm.hlp_editor},
              {"boredhelp",       &prm.hlp_editor},
              {"replacehelp",     &prm.hlp_replace},
              {"inquirehelp",     &prm.msg_inquire},
              {"leaving",         &prm.out_leaving},
              {"returning",       &prm.out_return},
              {"byebye",          &prm.byebye},
              {"barricade",       &prm.barricade},
              {"badlogon",        &prm.bad_logon},
              {"nospace",         &prm.no_space},
              {"beginchat",       &prm.chat_fbegin},
              {"endchat",         &prm.chat_fend},
              {"shell_leaving",   &prm.shelltodos},
              {"shell_returning", &prm.backfromdos},
              {"nomail",          &prm.nomail},
              {"notfound",        &prm.notfound},
              {"cant_enter_area", &prm.areanotexist},
              {"filename_format", &prm.fname_format},
              {"scanhelp",        &prm.hlp_scan},
              {"protocoldump",    &prm.proto_dump},
              {"tunes",           &prm.tune_file},
              {"listhelp",        &prm.hlp_list},
              {"headerhelp",      &prm.hlp_hdrentry},
              {"entryhelp",       &prm.hlp_msgentry},
              {"configure",       &prm.not_configured},
              {NULL,NULL}
            };

          struct _filelist *f;

          for (f=fl;f->name;f++)
          {
            if (eqstri(p,f->name))
            {
              Make_Filename(*f->where,s2);
              break;
            }
          }

          if (! f->name)
            Unknown_Ctl(linenum,p);
        }

        if (x==1 && do_prm)           /* USES statement */
        {                             /* Allow mecca or mex vm */
          int n=(*s2==':')?1:0;
#ifndef UNIX
          static char * ext[] = { ".BBS", ".VM" };
#else
          static char * ext[] = { ".bbs", ".vm" };
#endif
          if (! fexist(s2+n))
          {
            strcat(s2,ext[n]);

            if (! fexist(s2+n))
            {
              *strrchr(s2,'.')='\0';
              printf(cantfind_file_ctl,s2,linenum);
              Compiling(-1,NULL,NULL);
            }
          }
        }
      }
    }
    else if (eqstri(p,"kill"))
    {
      byte * b=NULL;
      word * c=NULL;

      getword(line,p,ctl_delim,2);

      if (eqstri(p,"private"))
        b = &prm.auto_kill;
      else if (eqstri(p,"attach"))
      {
        b = &prm.kill_attach;
        c = &prm.kill_attach_priv;
      }

      if (b)
      {
        getword(line,p,ctl_delim,3);

        if (eqstri(p,"never"))
          *b=0;
        else if (eqstri(p,"ask"))
          *b=1;
        else if (eqstri(p,"always"))
          *b=2;
        else b=0;

        if (b && c)
        {
          getword(line,p,ctl_delim,4);
          *c = (!p || !*p) ? 0 : Deduce_Priv(p);
        }
      }
      if (!b)
        Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"filelist"))
    {
      getword(line,p,ctl_delim,3);
      prm.fbbs_margin=(byte)atoi(p);
    }
    else if (eqstri(p,"after"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"call"))
      {
        getword(line,p,ctl_delim,4);
        prm.exit_val=(byte)atoi(p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"edit"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"disable"))
      {
        getword(line,p,ctl_delim,3);

        if (eqstri(p,"userlist"))
          prm.flags |= FLAG_no_ulist;
        else if (eqstri(p,"magnet") || eqstri(p,"maxed"))
          prm.flags |= FLAG_no_magnet;
        else Unknown_Ctl(linenum,p);
      }
      else if (eqstri(p,"menu"))
      {
        getword(line,p,ctl_delim,3);
        Make_String(prm.edit_menu,p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"format"))
    {
      getword(line,p,ctl_delim,2);
      s2=fchar(line,ctl_delim,3);

      if (eqstri(p,"time"))
        Make_String(prm.timeformat,s2)
      else if (eqstri(p,"date"))
        Make_String(prm.dateformat,s2)
      else if (eqstri(p,"msgheader"))
        Make_String(prm.msg_header,s2)
      else if (eqstri(p,"msgformat"))
        Make_String(prm.msg_format,s2)
      else if (eqstri(p,"msgfooter"))
        Make_String(prm.msg_footer,s2)
      else if (eqstri(p,"fileheader"))
        Make_String(prm.file_header,s2)
      else if (eqstri(p,"fileformat"))
        Make_String(prm.file_format,s2)
      else if (eqstri(p,"filefooter"))
        Make_String(prm.file_footer,s2)
      else Unknown_Ctl(linenum,p);
    }
#if 0
    else if (eqstri(p,"define"))
    {
      getword(line,p,ctl_delim,2);

      y=Deduce_Class(Deduce_Priv(p));

      getword(line,p,ctl_delim,3);
      getword(line,temp2,ctl_delim,4);

      if (eqstri(p,"cume"))
        prm.cls[y].max_time=atoi(temp2);
      else if (eqstri(p,"time"))
        prm.cls[y].max_call=atoi(temp2);
      else if (eqstri(p,"logon"))
      {
        getword(line,p,ctl_delim,5);
        prm.cls[y].min_baud=atoi(p);
      }
      else if (eqstri(p,"file"))
      {
        getword(line,p,ctl_delim,5);

        if (eqstri(temp2,"limit"))
        {
          tl=atol(p);

          if (tl > 32767L)
          {
            tl=32767L;

            printf("\n\aError on line %d of CTL file!  Max File "
                   "Limit is 32767.\n",linenum);
            Compiling(-1,NULL,NULL);
          }

          prm.cls[y].max_dl=(int)tl;
        }
        else if (eqstri(temp2,"baud"))
          prm.cls[y].min_file_baud=atoi(p);
        else if (eqstri(temp2,"ratio"))
          prm.cls[y].ratio=atoi(p);
        else Unknown_Ctl(linenum,p);
      }
      else Unknown_Ctl(linenum,p);
    }
#endif
#if 0
    else if (eqstri(p,"ratio"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"threshold"))
      {
        getword(line,p,ctl_delim,3);
        prm.ratio_threshold=atoi(p);
      }
      else Unknown_Ctl(linenum,p);
    }
#endif
    else if (eqstri(p,"upload"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"check"))
      {
        getword(line, p, ctl_delim, 3);
        
        if (eqstri(p, "dupes") || eqstri(p, "dupe"))
        {
          prm.flags2 |= FLAG2_CHECKDUPE;
          
          getword(line, p, ctl_delim, 4);
          
          if (eqstri(p, "extension"))
            prm.flags2 |= FLAG2_CHECKEXT;
        }
        else if (eqstri(p, "virus"))
        {
          getword(line, p, ctl_delim, 4);
          Make_Filename(prm.viruschk, p);
        }
        else Unknown_Ctl(linenum, p);
      }
#if 0
      else if (eqstri(p,"reward"))
      {
        getword(line,p,ctl_delim,3);

        if (! *p || p[strlen(p)-1] != '%')
        {
          printf("\n\aError!  The `Upload Reward' value must have a "
                 "trailing percentage sign!\n");
          exit(1);
        }
        else prm.ul_reward=atoi(p);
      }
#endif
      else if (eqstri(p,"log"))
      {
        s2=fchar(line,ctl_delim,3);
        Make_Filename(prm.ul_log,s2);
      }
#if 0
      else if (eqstri(p,".bbs"))
      {
        getword(line,p,ctl_delim,4);
        prm.ulbbs_priv=Deduce_Priv(p);
      }
#endif
      else if (eqstri(p,"space"))
      {
        getword(line,p,ctl_delim,3);

        if (eqstri(p,"free"))
        {
          getword(line,p,ctl_delim,4);
          prm.k_free=atol(p);
        }
        else Unknown_Ctl(linenum,p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"mailchecker"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"kill"))
      {
        getword(line,p,ctl_delim,3);
        prm.mc_kill_priv=Deduce_Priv(p);
      }
      else if (eqstri(p,"reply"))
      {
        getword(line,p,ctl_delim,3);
        prm.mc_reply_priv=Deduce_Priv(p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"local"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"editor"))
      {
        s2=fchar(line,ctl_delim,3);
        Make_String(prm.local_editor,s2);
      }
      else if (eqstri(p,"input"))
        prm.flags2 |= FLAG2_ltimeout;
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"comment"))
    {
      getword(line,p,ctl_delim,3);
      Make_String(prm.cmtarea,p);
    }
#if 0
    else if (eqstri(p,"userlist"))
    {
      getword(line,p,ctl_delim,2);
      s2=fchar(line,ctl_delim,3);

      if (eqstri(p,"minimum"))
        prm.min_ulist=Deduce_Priv(s2);
      else if (eqstri(p,"maximum"))
        prm.max_ulist=Deduce_Priv(s2);
      else Unknown_Ctl(linenum,p);
    }
#endif
    else if (eqstri(p,"no"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"realname"))
        prm.flags |= FLAG_norname;
      else if (eqstri(p,"hogging"))   /* Just ignore, always active */
        ;
#if 0
      else if (eqstri(p,"filesbbs"))
      {
        getword(line,p,ctl_delim,3);

        if (eqstri(p,"download"))
        {
          getword(line,p,ctl_delim,4);
          prm.dlall_priv=Deduce_Priv(p);
        }
        else Unknown_Ctl(linenum,p);
      }
#endif
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"ask"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"phone"))
        prm.flags |= FLAG_ask_phone;
      else if (eqstri(p,"alias") || eqstri(p,"real"))
        prm.flags |= FLAG_ask_name;
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"alias"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"system"))
        prm.flags |= FLAG_alias;
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"chat"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"external"))
        Make_String(prm.chat_prog,fchar(line,ctl_delim,3))
      else if (eqstri(p, "capture"))
        prm.flags2 |= FLAG2_CAPTURE;
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"external"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"protocol") || eqstri(p,"batchprotocol") ||
          eqstri(p,"batchexitprotocol") || eqstri(p,"exitprotocol"))
      {
        getword(line,p,ctl_delim,3);

        if (eqstri(p,"errorlevel"))
        {
          getword(line,p,ctl_delim,4);
          prm.protoexit=atoi(p);
        }
#ifdef NEVER
        else
        {
          if (protocol_num==MAX_EXTERNP)
          {
            printf("\n\aError!  Too many external protocols, "
                   "line %d.\n",linenum);
            exit(1);
          }

          getword(line,p,ctl_delim,2);

          if (eqstri(p,"batchprotocol"))
            prm.protoflag[protocol_num] |= XTERNBATCH;
          else if (eqstri(p,"batchexitprotocol"))
            prm.protoflag[protocol_num] |= (XTERNBATCH | XTERNEXIT);
          else if (eqstri(p,"exitprotocol"))
            prm.protoflag[protocol_num] |= XTERNEXIT;

          getword(line,p,ctl_delim,3);
          s2=fchar(line,ctl_delim,4);

          Make_Filename(prm.protocols[protocol_num],p);

          if (s2 && *s2)
            Make_String(prm.protoname[protocol_num],s2)
          else prm.protoname[protocol_num]=prm.protocols[protocol_num];

          protocol_num++;
        }
#endif
        else Unknown_Ctl(linenum,p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"yell"))
    {
      getword(line, p, ctl_delim, 2);
      
      if (eqstri(p, "off"))
        prm.noise_ok=FALSE;
      else
      {
        printf("\n\aWarning!  Yell events have been moved from MAX.CTL to\n"
               "EVENTSxx.BBS.\n");
      }
    }
    else if (eqstri(p,"highest"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"msgarea"))
      {
        getword(line,p,ctl_delim,3);
        Make_String(prm.high_msgarea,p);
      }
      else if (eqstri(p,"filearea"))
      {
        getword(line,p,ctl_delim,3);
        Make_String(prm.high_filearea,p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p, "single"))
      prm.flags2 |= FLAG2_1NAME;
    else if (eqstri(p,"protocolctl"))
    {
      getword(line, p, ctl_delim, 2);
      Make_Filename(prm.protocol_max, p);
    }
    else if (eqstri(p, "messagedata"))
    {
      getword(line, p, ctl_delim, 2);
      Make_Filename(prm.marea_name, p);
    }
    else if (eqstri(p, "filedata"))
    {
      getword(line, p, ctl_delim, 2);
      Make_Filename(prm.farea_name, p);
    }
    else if (eqstri(p, "area"))
    {
      getword(line, p, ctl_delim, 2);

      if (eqstri(p, "change"))
      {
        getword(line,p,ctl_delim,4);
        Make_String(prm.achg_keys,strupr(p));
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p, "global"))
      prm.flags2 |= FLAG2_GLOBALHB;
    else if (eqstri(p,"rip"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p, "path"))
      {
        s2=fchar(line, ctl_delim, 3);
        Make_Path(prm.rippath, s2);

        if (! direxist(strings+prm.rippath))
          makedir(strings+prm.rippath);
      }
      else
        Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"menu"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p, "path"))
      {
        s2=fchar(line, ctl_delim, 3);
        Make_Path(prm.menupath, s2);

        if (! direxist(strings+prm.menupath))
          makedir(strings+prm.menupath);
      }
      else
        Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"first"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"menu"))
      {
        getword(line,p,ctl_delim,3);
        Make_String(prm.first_menu,p);
      }
      else if (eqstri(p,"file"))
      {
        getword(line,p,ctl_delim,4);
        Make_String(prm.begin_filearea,p);
      }
      else if (eqstri(p,"message"))
      {
        getword(line,p,ctl_delim,4);
        Make_String(prm.begin_msgarea,p);
      }
      else Unknown_Ctl(linenum,p);
    }
    else if (eqstri(p,"compatible"))
      prm.flags |= FLAG_lbaud96;
    else if (eqstri(p,"statusline"))
      prm.flags |= FLAG_statusline;
    else if (eqstri(p,"charset"))
    {
      getword(line, p, ctl_delim, 2);
      
      if (eqstri(p, "chinese"))
      {
        prm.charset=CHARSET_CHINESE;
        prm.flags2 |= FLAG2_GLOBALHB;
      }
      else if (eqstri(p, "swedish"))
        prm.charset=CHARSET_SWEDISH;
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"input"))
      prm.input_timeout=(byte)atoi(fchar(line,ctl_delim,3));
    else if (eqstri(p,"save"))
    {
      getword(line,p,ctl_delim,2);

      if (eqstri(p,"directories"))
      {
        getword(line,p,ctl_delim,3);

        for (sp=p;*sp;sp++)
        {
          if ((*sp=(char)toupper(*sp)) >= 'A' && *sp <= 'Z')
            BitOn(prm.drives_to_save,*sp-'A');
          else
          {
            printf("\n\aInvalid drive `%c' specified on line %d of "
                   "CTL file!\n",*p,linenum);
            Compiling(-1,NULL,NULL);
          }
        }
      }
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"arrow"))
      ;
    else if (eqstri(p, "strict"))
      prm.flags2 |= FLAG2_STRICTXFER;
    else if (eqstri(p, "stage"))
    {
      getword(line, p, ctl_delim,2);

      if (eqstri(p, "path"))
      {
        s2=fchar(line, ctl_delim, 3);
        Make_Path(prm.stagepath, s2);

        if (! direxist(strings+prm.stagepath))
          makedir(strings+prm.stagepath);
      }
      else
        Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"maxmsgsize"))
    {
      getword(line, p, ctl_delim, 2);

      prm.max_msgsize=atol(p);

      if (prm.max_msgsize < 32UL) /* Assume k */
        prm.max_msgsize*=1024L;

      if (prm.max_msgsize < 4096L)
        prm.max_msgsize=UL_TRUNC;
    }
    else if (eqstri(p, "use"))
    {
      getword(line, p, ctl_delim, 2);

      /* All references to message numbers will be by UMSGID */

      if (eqstri(p,"umsgids"))
        prm.flags2 |= FLAG2_UMSGID;
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"check"))
    {
      getword(line, p, ctl_delim, 2);

      if (eqstri(p, "ansi"))
        prm.flags2 |= FLAG2_CHKANSI;
      else if (eqstri(p, "rip"))
        prm.flags2 |= FLAG2_CHKRIP;
      else Unknown_Ctl(linenum, p);
    }
    else if (eqstri(p,"app") || eqstri(p,"application"))
      ;
    else Unknown_Ctl(linenum,p);

    linenum++;
  }

  linenum++;

  return 0;
}


static void Init_Attach_Db(char * szAttDbName)
{
  char temp[PATHLEN];

  strcpy(temp, szAttDbName);
  strcat(temp, ".db");
  if (! fexist(temp))
    LFAdbClose(LFAdbCreate(szAttDbName));
}


