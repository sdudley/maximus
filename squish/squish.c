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
static char rcs_id[]="$Id: squish.c,v 1.6 2003/11/28 21:48:07 paltas Exp $";
#pragma on(unreferenced)

/*#define TEST_VER*/
#define INITSQUISH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "alc.h"
#include "dr.h"
#include "prog.h"
#include "max.h"
#include "msgapi.h"
#include "squish.h"
#include "squishpv.h"
#include "smserial.h"

extern char __fd2n;   /* bring in nopen() routines */

struct _config config;
word mode;

static time_t start;
static word old_emax;
static int fRunZCore=FALSE;

static struct _args ar={"", "squish.cfg", "", "", "",
                        TRUE, ACTION_NONE, FALSE, {NULL}, 0};

#ifdef __TURBOC__
unsigned int __stklen=32767;
#endif

#ifdef DJ
FILE *dj;
#endif

int _stdc main(int argc, char *argv[])
{
  char *p;

  NW(__fd2n);

#if defined(__FLAT__) && !defined(UNIX)
  Hello("SQUISH/386", "SquishMail Conference Processor", version, "1990, " THIS_YEAR);
#elif defined(LINUX)
  Hello("SQUISH/LINUX", "SquishMail Conference Processor", version, "1990, " THIS_YEAR);
#elif defined(UNIX)
  Hello("SQUISH/UNIX", "SquishMail Conference Processor", version, "1990, " THIS_YEAR);
#else
  Hello("SQUISH", "SquishMail Conference Processor", version, "1990, " THIS_YEAR);
#endif

#ifdef OS_2 /* Serialize Squish's execution */
  BbsSemSerialize("/sem/squish/default", "BBSSEM");
#endif

#ifdef __WATCOMC__
  setbuf(stdout, NULL);
#endif

#ifdef DMALLOC
  /*dmalloc_on(TRUE);*/
#endif

#ifdef DJ
  dj=fopen("dj.log", "a");
#endif

#ifdef UNIX
  if (!getenv("SQUISH"))
    putenv("SQUISH=" INSTALL_PREFIX "/etc/squish.cfg");
#endif

  if (!fexist(ar.cfgname) && (p=getenv("SQUISH")) != NULL)
    (void)strcpy(ar.cfgname, p);

#if defined(__MSDOS__) && !defined(__FLAT__)
  install_24();
  (void)atexit(uninstall_24);
#endif

  if (argc < 2)
    usage();
    
  Initialize_Variables();

  /* Use the SQUISH environment variable to locate config file, if necessary */

  p=getenv("SQUISH");

  if (p && !fexist(ar.cfgname)) /* as long as there's no squish.cfg in cwd */
    (void)strcpy(ar.cfgname, p);

  ParseArgs(&ar, (byte **)argv);   /* Command-line arguments */
/*  strcpy(ar.cfgname,"Squish.Cfg");*/

  Parse_Config(ar.cfgname);  /* parse squish.cfg and areas.bbs */

/*
  {
    NETADDR bill={1,249,1,0};

    if (DestIsHereA(&bill))
      printf("!!! This copy of Squish is registered to Bill Cassidy !!!\n");
  }
*/

  Parse_Areas(*ar.areasbbs ? ar.areasbbs : config.areasbbs);

  /* If a log-file override was specified on the command line */

  if (*ar.logfile)
  {
    if (config.logfile)
      free(config.logfile);

    config.logfile=sstrdup(ar.logfile);
  }

  InitializeConfig();     /* Initialize configuration */

  if (ar.action==ACTION_GET || ar.action==ACTION_SEND ||
      ar.action==ACTION_UPDATE || ar.action==ACTION_POLL)
  {
    config.flag &= ~FLAG_ADDMODE;
    HandleAttReqPoll(ar.action, ar.toscan);
  }
  else SquishSquashCycle();

#ifdef DJ
  if (dj)
    fclose(dj);
#endif

  CleanupConfig();

  (void)printf("\n" SQNAME ": Done!\n");

  return (erl_max ? ERL_MAXMSGS : erl_echo ? ERL_TOSS_ECHO :
          erl_net ? ERL_TOSS_NET : erl_sent ? ERL_SENT_ECHO :
          ERL_NONE);
}


static void near InitializeConfig(void)
{
  struct _minf minf;

  /* Turn secure mode on/off */

  if (ar.action==ACTION_RESCAN)
  {
    (void)SblistToNetaddr(&config.def, &ar.n);

    ParseNN(*ar.toscan, &ar.n.zone, &ar.n.net, &ar.n.node,
            &ar.n.point, FALSE);
  }

  #ifdef NEVER
    if (config.max_talist > 4)
      relocate(config.max_handles);
  #endif
  
  minf.req_version=MSGAPI_VERSION;
  minf.def_zone=config.addr->zone;

  minf.palloc=sq_palloc;
  minf.pfree=sq_pfree;
  minf.repalloc=sq_repalloc;

  minf.farpalloc=sq_farpalloc;
  minf.farpfree=sq_farpfree;
  minf.farrepalloc=sq_farrepalloc;

  if (MsgOpenApi(&minf) != 0)
  {
    (void)printf("Error initializing MsgAPI.  Aborting...\n");
    exit(ERL_ERROR);
  }

  if ((mode & MODE_tosscan)==MODE_tosscan)
    config.flag |= FLAG_ONEPASS;

  start=time(NULL);
}


static void near SquishSquashCycle(void)
{
  time_t now=time(NULL);

  S_LogOpen(config.logfile);

  S_LogMsg("+Begin, " SQNAME " v%s (mem=%ldK - main=%p)", version,
           (long)coreleft()/1024L, (void *)main);

  StatsOpen();

#ifdef __MSDOS__
  {
    long coreneeded;
    long left=coreleft() / 1024L;

    coreneeded = outbufmax/1024L + writebufmax/1024L + maxmsglen/1024L;

    #ifdef __FLAT__
      #define MIN_CORE  1000L
      #define ADDED     200L
    #else
      #define MIN_CORE  200L
      #define ADDED     20L
    #endif

    coreneeded=max(MIN_CORE, coreneeded+ADDED);

    if (left < (long)coreneeded && !fRunZCore)
    {
      S_LogMsg("!> Squish may not have enough free memory to");
      S_LogMsg("!> operate properly.  Current heap memory is %ldK,", left);
      S_LogMsg("!> but the suggested value is %ldK.  You can use the", coreneeded);
      S_LogMsg("!> -m switch to force Squish to run anyway using");
      S_LogMsg("!> the current amount of memory, but this is not");
      S_LogMsg("!> suggested.  (To reduce memory requirements, try");
      S_LogMsg("!> using a lower 'Buffers' setting, disabling");
      S_LogMsg("!> Statistics mode, and removing unnecessary areas.)");
      S_LogClose();

      exit(ERL_ERROR);
    }
  }
#endif

/*  S_LogMsg("@SquishSquashCycle Begin - mode=%04xh", mode);*/

  do
  {
    HoleDeinitHole();
    HoleInitHole();

    if (mode & (MODE_scan | MODE_pack))
      HoleScanHole();

    old_emax=erl_max;
    
    erl_max=FALSE;  /* we haven't reached max_msgs on this run (yet) */
  

    /* Toss messages as necessary */

    if (mode==0 || (mode & MODE_toss))
      Toss_Messages(ar.echotoss, old_emax, start);
  
    /* Now handle any separate scanning */

    if ((mode & MODE_scan) && !erl_max)
    {
      word cflag;
      
      /* Remove the ONEPASS bit from the general flags, since even though   *
       * we may have been doing a toss/scan before, now we're just in       *
       * straight scan mode to pick off leftover areas.                     */

      cflag=config.flag;
      config.flag &= ~FLAG_ONEPASS;
      
      Scan_Messages(*ar.echotoss ? ar.echotoss : NULL,
                    ar.action==ACTION_RESCAN ? &ar.n : NULL,
                    start);

/*      S_LogMsg("@AfterScan - mode=%04xh, erlmax=%d, scan_ctr=%ld, max_msgs=%ld",
               (int)mode, (int)erl_max, (long)scan_ctr, (long)config->max_msgs);*/
                  
      config.flag=cflag;
    }

    /* If we've reached a max_msgs situation and we're in a one-pass        *
     * scan/pack environment, pack it here.                                 */

    if (erl_max && (mode & MODE_sp)==MODE_sp)
    {
/*      S_LogMsg("@BeforePack - erlmax=%d", erlmax);*/
      Munge_Outbound_Area(config.routing, ar.sched);
    }
  } while (erl_max && (mode & MODE_sp)==MODE_sp);

  if (mode & MODE_pack)
  {
    if (ar.do_pack)
      Pack_Messages(config.netmail);

    Munge_Outbound_Area(config.routing, ar.sched);
  }

  if (! ar.leave_packets)
    HoleMoveOut();

  HoleDeinitHole();

  if ((mode & MODE_link) && !erl_max)
    Link_Messages(*ar.echotoss ? ar.echotoss : NULL);
  
  StatsClose();

  now=time(NULL)-now;

  if (now==0)
    now=1;

  S_LogMsg("+End.  Toss=%ld (%ld/s), sent=%ld (%ld/s), mem=%ldK",
           nmsg_tossed, nmsg_tossed/now,
           nmsg_sent, nmsg_sent/now,
           (long)coreleft()/1024L);

  S_LogClose();
}



static void near CleanupConfig(void)
{
  (void)MsgCloseApi();
  Cleanup();
}




void usage(void)
{
  char temp[50];

  putss("SquishMail can be started with any of the following commands:\n\n"

        "   SQUISH [IN] [OUT] [SQUASH] [LINK] [switches...]\n\n"

        "       Any or all of the above options may be specified on one command-line.\n"
        "       `IN' instructs Squish to toss (import) messages.  `OUT' instructs\n"
        "       Squish to scan (export) messages.  `SQUASH' instructs Squish to\n"
        "       pack/mash messages in the netmail area.  `LINK' instructs Squish to\n"
        "       relink reply chains.  Specifying `IN' and `OUT' together turns on\n"
        "       Squish's one-pass mode.\n\n"

        "   SQUISH RESCAN <area_tag> <node>              - Rescan area to node\n"
        "   SQUISH SEND <file> [TO] <node> [flavour]     - Attach file to node\n"
        "   SQUISH GET <file> [FROM] <node> [flavour]    - Request file from node\n"
        "   SQUISH UPDATE <file> [FROM] <node> [flavour] - Upd. request file from node\n"
        "   SQUISH POLL <node> [flavour]                 - Poll node\n\n");

  (void)printf("Press <enter> to continue: ");
  (void)fgets(temp, 50, stdin);
  putss("\n");

  putss("Command-line arguments supported by Squish are:\n\n"

        "   -a<areas_bbs>         - Use <areas_bbs> instead of the default AREAS.BBS\n"
        "   -c<config_file>       - Use <config_file> instead of SQUISH.CFG\n"
        "   -f<echotoss>          - Use <echotoss> to log areas (toss) or to link/scan\n"
        "   -l                    - Leave uncompressed packets in OUTBOUND.SQ\n"
        "   -n<log_file>          - Override the log file given in SQUISH.CFG\n"
        "   -o                    - When doing a `squash', process outbound area only\n"
        "   -q                    - Quiet mode.  Suppresses most informational displays\n"
        "   -s<tag>               - Override default scheduling, and run schedule <tag>\n"
        "   -t                    - Toggle secure mode\n"
        "   -u                    - Toggle TossBadMsgs mode\n"
        "   -v                    - Toggle statistics mode\n"
        "   -z                    - Only scan non-passthrough areas\n");

  exit(ERL_ERROR);
}




static void near ParseArgs(struct _args *ags, byte *argv[])
{
  static char unknown_opt[]="Unknown command-line option: `%s'\n";
  byte **arg;

  for (mode=0, arg=argv+1; *arg; arg++)
  {
    if (**arg != '-')
    {
      if (eqstri(*arg, "in") || eqstri(*arg, "toss"))
        mode |= MODE_toss;
      else if (eqstri(*arg, "out") || eqstri(*arg, "scan"))
        mode |= MODE_scan;
      else if (eqstri(*arg, "squash") || eqstri(*arg, "pack"))
        mode |= MODE_pack;
      else if (eqstri(*arg, "link"))
        mode |= MODE_link;
      else if (eqstri(*arg, "rescan"))
      {
        mode|=MODE_scan;
       
        if (arg[1]==NULL || arg[2]==NULL)
        {
          (void)printf("Error!  Format for RESCAN command is:\n\n"
                       "    SQUISH RESCAN <area_tag or echotoss.log> <node>\n");

          exit(ERL_ERROR);
        }
        
        (void)strcpy(ags->echotoss, arg[1]);
        
        ags->action=ACTION_RESCAN;
        ags->toscan=arg+2;
        //return;
	arg += 2;
      }
#if 0
      else if (eqstri(*arg, "test"))
      {
          test_harness(arg[1]);
          exit(ERL_NONE);
      }
#endif
      else
      {
        ags->toscan=arg+1;
        mode=MODE_pack;
        ags->do_pack=FALSE;

        if (eqstri(*arg, "get") || eqstri(*arg, "request"))
        {
          ags->action=ACTION_GET;
          return;
        }
        else if (eqstri(*arg, "update"))
        {
          ags->action=ACTION_UPDATE;
          return;
        }
        else if (eqstri(*arg, "send") || eqstri(*arg, "attach"))
        {
          ags->action=ACTION_SEND;
          return;
        }
        else if (eqstri(*arg, "poll"))
        {
          ags->action=ACTION_POLL;
          return;
        }
        else
        {
          (void)printf(unknown_opt, *arg);
          exit(ERL_ERROR);
        }
      }
    }
    else switch(tolower((*arg)[1]))
    {
      case 'a':
        (void)strcpy(ags->areasbbs, *arg+2);
        break;

      case 'c':
        (void)strcpy(ags->cfgname, *arg+2);
        break;

#if defined(DMALLOC) && defined(__MSDOS__)
      case 'd':
      {
        extern int _stdc shutup;

        (void)printf("PSP at %04x\n", _psp);
        shutup=FALSE;
        break;
      }
#endif

      case 'f':
        (void)strcpy(ags->echotoss, *arg+2);
        break;
        
      case 'l':
        ags->leave_packets=TRUE;
        break;

      case 'm':
        fRunZCore=TRUE;
        break;

      case 'n': /* new log file */
        (void)strcpy(ags->logfile, *arg+2);
        break;

      case 's':
        (void)strcpy(ags->sched, *arg+2);
        mode |= MODE_pack;
        break;

      case 't':
        config.flag ^= FLAG_SECURE;
        break;

      case 'u':
        config.flag ^= FLAG_TOSSBAD;
        break;

      case 'o':
        ags->do_pack=FALSE;
        break;

      case 'q':
        config.flag2 |= FLAG2_QUIET;
        break;

#if 0
      case 'x':
        mode=MODE_pack;

        switch(tolower((*arg)[2]))
        {
          case 'f':
            convert=CVT_FLO;
            break;
            
          case 'a':
            convert=CVT_ARC;
            break;
            
          case 'k':
            convert=CVT_KILL;
            break;

          case 's':
            convert=CVT_SFLO;
            break;
        }
        break;
#endif

      case 'v':
        config.flag ^= FLAG_STATS;
        break;

      case 'z':
        config.flag2 |= FLAG2_NPTHRU;
        break;

      default:
        (void)printf(unknown_opt, *arg);
        exit(ERL_ERROR);
    }
  }
}







#if 0

#define RDLEN 8192

static void near test_harness(char *name)
{
  MSGH *msgh;
  long tl;

  char lastfrom[36]="";
  static char temp[100];
  static char ctext[1000];
  char *p;

  MSG *sq;

  XMSG msg,
       msg2,
       rmsg;

  char *rd;

  if ((rd=malloc(RDLEN))==NULL)
  {
    printf("no memory.\n");
    exit(ERL_ERROR);
  }

  memset(&msg,'\0',sizeof(XMSG));
  memset(&msg2,'\0',sizeof(XMSG));

  Get_Dos_Date((union stamp_combo *)&msg.date_arrived);
  Get_Dos_Date((union stamp_combo *)&msg.date_written);

  strcpy(msg.from,"Scott Dudley");
  strcpy(msg.to,"Test");
  strcpy(msg.subj,"This is a test!");

  strcpy(msg2.from,"Maximus-CBCS");
  strcpy(msg2.to,"Fido");
  strcpy(msg2.subj,"Bad dog!");

  msg.attr=MSGLOCAL;
  msg2.attr=MSGLOCAL;

  if ((sq=MsgOpenArea(*name=='!' ? name+1 : name,
                      MSGAREA_NORMAL,
                      *name=='!' ? MSGTYPE_SDM : MSGTYPE_SQUISH))==NULL)
  {
    printf("\aError opening SquishFile!\n");

    printf("Create? ");

    fgets(temp, 100, stdin);

    if (*temp=='Y' || *temp=='y')
    {
      if ((sq=MsgOpenArea(*name=='!' ? name+1 : name,
                      MSGAREA_CRIFNEC,
                      *name=='!' ? MSGTYPE_SDM : MSGTYPE_SQUISH))==NULL)
      {
        printf("\aError opening SquishFile!\n");
        exit(ERL_ERROR);
      }
    }
    else exit(ERL_ERROR);
  }

  MsgLock(sq);

  for (;;)
  {
    printf("SQ> ");

    fgets(temp,100,stdin);

    switch (tolower(*temp))
    {
      case 'r':
        if (temp[1]=='e')
          goto ReplyMsg;
        else if (temp[1]=='n')
          tl=MSGNUM_next;
        else tl=MSGNUM_previous;

        if ((msgh=MsgOpenMsg(sq,MOPEN_READ,tl))==NULL)
        {
          printf("Not found.\n");
          break;
        }

ReadText:
        if (MsgReadMsg(msgh,&rmsg,0L,RDLEN,rd,999L,ctext)==-1)
          printf("Error reading message.\n");
        else
        {
          printf("From: %-20s (%u:%d/%d.%u)  Date: %s\n"
                 "  To: %-20s (%u:%d/%d.%u)\n"
                 "Subj: %s\n"
                 "This: %ld (uid=%ld), Down: %ld (uid=%ld), Up: %ld (uid=%ld)\n\n",
                 rmsg.from,
                 rmsg.orig.zone,
                 rmsg.orig.net,
                 rmsg.orig.node,
                 rmsg.orig.point,
                 sc_time((union stamp_combo *)&rmsg.date_written,temp),
                 rmsg.to,
                 rmsg.dest.zone,
                 rmsg.dest.net,
                 rmsg.dest.node,
                 rmsg.dest.point,
                 rmsg.subj,
                 MsgCurMsg(sq),
                 MsgMsgnToUid(sq,MsgCurMsg(sq)),
                 MsgUidToMsgn(sq,rmsg.replyto,UID_EXACT),
                 rmsg.replyto,
                 MsgUidToMsgn(sq,rmsg.replies[0],UID_EXACT),
                 rmsg.replies[0]);

          strcpy(lastfrom,rmsg.from);
          p=rd;

          /* Remove all \n's and soft CR's */
          while ((p=strpbrk(p,"\n\x8d")) != NULL)
            memmove(p,p+1,strlen(p+1)+1);

          p=rd;

          /* Replace all returns with CR/LFs. */
          while ((p=strchr(p,'\r')) != NULL)
          {
            memmove(p+2,p+1,strlen(p+1)+1);
            p[1]='\n';

            p += 2;
          }

          fprintf(stdout, "%s", rd);

          printf("\n");
          
          printf("ctext:\n---%s---\n",ctext);
        }

        MsgCloseMsg(msgh);
        break;

ReplyMsg:
        msg.replyto=MsgMsgnToUid(sq,MsgCurMsg(sq));
        strcpy(msg.to,lastfrom);
        /* fall-thru */

      case 'w':
        printf("\nTEXT> ");

        fgets(temp,499,stdin);

        if ((msgh=MsgOpenMsg(sq,MOPEN_CREATE,0L))==NULL)
          printf("Can't open message!\n");
        else
        {
          if (MsgWriteMsg(msgh,FALSE,&msg,temp,strlen(temp)+1,
                          strlen(temp)+1,0L,NULL)==-1)
            printf("Can't write message!\n");
          else printf("Wrote msg#%d\n",MsgHighMsg(sq));

          MsgCloseMsg(msgh);
        }
       break;

       case 'n':  /* no private bit */

        if ((msgh=MsgOpenMsg(sq,MOPEN_RW,MsgCurMsg(sq)))==NULL)
          printf("Can't open message!\n");
        else
        {
           if (MsgReadMsg(msgh,&rmsg,0L,0L,NULL,0L,NULL) != -1)
           {
             rmsg.attr &= ~(MSGPRIVATE|MSGREAD);

             if (MsgWriteMsg(msgh,FALSE,&rmsg,NULL,0L,0L,0L,NULL)==-1)
               printf("Can't write message!\n");
             else printf("Done.\n",MsgHighMsg(sq));
           }

          MsgCloseMsg(msgh);
        }
       break;

     case 's':
        printf("Curmsg=%ld, Nummsg=%ld\n",MsgCurMsg(sq),MsgNumMsg(sq));
        break;

      case 'k':
        printf("Message #%d was %skilled okay.\n",atoi(temp+1),
               MsgKillMsg(sq,atoi(temp+1))==0 ? "" : "NOT ");
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if ((msgh=MsgOpenMsg(sq,MOPEN_READ,atol(temp)))==NULL)
        {
          printf("Goto was unsuccessful.\n");
          break;
        }
        goto ReadText;

      case 'q':
        goto done;

      case 'l':
        while ((msgh=MsgOpenMsg(sq,MOPEN_READ,MSGNUM_next)) != NULL)
        {
          if (MsgReadMsg(msgh,&msg,0L,0L,NULL,0L,NULL)==-1)
            printf("Error reading message.\n");
          else printf("#%-3ld Fm: %-15.15s  To: %-15.15s  Subj: %-25.25s\r\n",
                      MsgCurMsg(sq),msg.from,msg.to,msg.subj);

          MsgCloseMsg(msgh);
        }
        break;
    }
  }

done:

  MsgUnlock(sq);

  MsgCloseArea(sq);
  free(rd);
}



#ifdef DMALLOC
long core(void)
{
#ifdef __WATCOMC__
  unsigned long cdecl coreleft(void);
  printf("\r%ld\n",(long)coreleft());
  return ((long)coreleft());
#else
  return 0L;
#endif
}
#endif

#endif  /* TEST_VER */



