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
static char rcs_id[]="$Id: max_init.c,v 1.3 2004/01/15 01:09:09 paltas Exp $";
#pragma on(unreferenced)

/*# tname=Initialization code
*/

#define MAX_LANG_max_init
#define MAX_LANG_max_chat
#define MAX_INCL_COMMS

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <ctype.h>
#include <string.h>
#include <share.h>
#include "dr.h"
#include "prog.h"
#include "mm.h"
#include "max_msg.h"
#include "max_file.h"
#include "max_edit.h"
#include "emsi.h"

#ifdef KEY
#include "makekey.h"
#endif


static void near OpenAreas(void);
static void near install_handlers(void);
static void near StartUpVideo(void);
static void near Initialize_Colours(void);


void Init_Variables(void)
{
  int x;
  char *s;

  install_handlers(); /* ISRs for ^c and int 24h */

  timestart=time(NULL);

  InputAllocStr();
  OutputAllocStr();

  /* Fix up global and initialization data */

  multitasker=-2;
  strcpy(log_name,c123);

  local_putc=(void (_stdc *)(int))fputchar;
  local_puts=(void (_stdc *)(char *))putss;

#ifndef ORACLE
  Lprintf(slogan,"\n",version,test);
  Lputs(copyright);
#endif

  *firstname=*linebuf=*searchfor=*fix_menupath=*last_readln=*arq_info='\0';

  display_line=display_col=current_line=current_col=1;

  isareply=isachange=FALSE;

  port=-1;
  cls=-1;
  orig_disk2=orig_disk3=-1;
  local=-2;
  task_num=255;
  fSetTask = FALSE;
  event_num=(byte)-2;

  baud=0;
  current_baud=0;
  steady_baud=0;
  steady_baud_l=0;
  ultoday=0;
  brk_trapped=0;
  erl=0;
  
  #ifndef ORACLE
  max_lines=0;
  #endif
    
  num_yells=0;
  rst_offset=-1L;
  getoff=0x7fffffffL;

  fFlow=FALSE;

  #ifdef UNIX
  memset(&CommApi, 0, sizeof(struct CommApi_));
  #endif

  menu_lines=1;

  if ((s=getenv("PROMPT")) != NULL)
  {
    /* extra parens for dmalloc() kludge - see max.c */

    if ((original_prompt=(malloc)(strlen(s)+1)) != NULL) 
      strcpy(original_prompt,s);
    else original_prompt=NULL;
  }
  else original_prompt=NULL;



  do_caller_vanished=TRUE;

  snoop=keyboard=caller_online=do_timecheck=fossil_initd=
    written_echomail=written_matrix=sent_time_5left=sent_time_almostup=
               wrap=inmagnet=restart_system=first_search=
                 barricade_ok=create_userbbs=this_logon_bad=inchat=
    locked=chatreq=do_useredit=fthru_yuhu=shut_up=debug_ovl=
    no_dcd_check=FALSE;
    fLoggedOn=FALSE;

#ifdef __MSDOS__
  port_is_device=TRUE;
#else
  port_is_device=FALSE; /* port is a file handle */
#endif

  nowrite_lastuser=in_file_xfer=written_local=mn_dirty=no_zmodem=
    in_mcheck=no_shell=dsp_set=in_node_chat=chkmail_reply=
    waitforcaller=in_wfc=log_wfc=in_msghibit=FALSE;

  direction=DIRECTION_NEXT;

  offsets=NULL;

  chatlog=NULL;
#ifndef ORACLE
  sq=NULL;
#endif
  dspwin=NULL;
  dspwin_time=0L;

  #ifndef ORACLE
  Init_File_Buffer();
  #endif

  max_time=0xffffL;
  last_bps=0;

  displaymode=VIDEO_DOS;  /* So that our output will work for Lprintf. */
                          /* This hopefully gets reset later! */

  prm.logon_priv=0;
  Blank_User(&usr);

/*InitTag(&tfa);*/

  for (x=0;x < MAX_DRIVES;x++)
    orig_path2[x]=orig_path3[x]=NULL;

/*  memset(echo_written_in, '\x00', sizeof(echo_written_in));*/

#ifdef MCP
  *szMcpPipe = 0;
#endif


#ifdef EMSI
  EmsiInitHandshake();
#endif
}

static void near quitfile()
{
  vbuf_flush();
  Local_Beep(3);
  Delay(300);
  quit(ERROR_FILE);
}


char * Startup(void)
{
#ifdef KEY
  int key_fd, key_ok=TRUE;
  char *key_buf, *key_outbuf;
  int key_size, key_strip_ofs;
#endif

  union stamp_combo now;
  char temp[PATHLEN];

#ifndef ORACLE
  struct _usr user;
  struct _minf mi;
  int fd;
#endif



  if (getcwd(original_path, PATHLEN)==NULL)
  {
    Lputs(err_startup_tlong);
    Local_Beep(3);
    maximus_exit(ERROR_FILE);
  }
  else if (strlen(cfancy_fn(original_path)) > 3)
    strcat(cfancy_fn(original_path), PATH_DELIMS);

  /* Install the critical error handler */
  
  if ((prm.flags2 & FLAG2_NOCRIT)==0)
  {
    install_24();
    maximus_atexit(uninstall_24);
  }

  Initialize_Languages();
  Initialize_Colours();

  /* Determine maximum length of string returned by MsgDte() */
  
  Get_Dos_Date(&now);
  MsgDte(&now, temp);
  datelen=strlen(temp);

  if (!dsp_set)
    displaymode=prm.video;
  
  /* Make sure that the version string is ok */

  if (~version[0]+~version[2]+~version[3] != VER_CHECKSUM ||
      ~tear_ver[0]+~tear_ver[2]+~tear_ver[3] != VER_CHECKSUM ||
      ~xfer_id[8]+~xfer_id[10]+~xfer_id[11] != VER_CHECKSUM ||
      ~tear_ver[0]+~tear_ver[2]+~tear_ver[3] != VER_CHECKSUM ||
      ~us_short[0]+~toupper(us_short[1])+
                   ~toupper(us_short[2]) != NAME_CHEKSUM ||
      ~name[0]+~name[1]+~name[2] != NAME_CHEKSUM ||
      ~xfer_id[0]+~toupper(xfer_id[1])+~toupper(xfer_id[2]) != NAME_CHEKSUM)
  {
    logit(grunged_exe);
    quit(ERROR_CRITICAL);
  }
  
  /* Open a couple of files to prepare for the caller */

  if (!do_useredit)
    OpenAreas();

  Load_Archivers();

#ifdef KEY
  if ((key_fd=key_open("MAXIMUS.KEY"))==-1)
    key_ok=FALSE;
#endif

  /* Wait for the caller to come on-line, if using the os/2 version */

#if defined(OS_2) && !defined(__FLAT__) && !defined(ORACLE)
  MaxSemTriggerWait();
#endif

  /* Turn off the status line for local mode.  If restart_system is set,    *
   * then we won't know if the user is local or remote until we've read     *
   * in RESTARxx.BBS, so we'll take care of it in the System_Restart()      *
   * function in that case.                                                 */

  if (local && !restart_system)
    prm.flags &= ~FLAG_statusline;

  /* Open the video display */

  switch (displaymode)
  {
#if 0
    case VIDEO_FOSSIL:

#ifndef ORACLE       

      /* ORACLE doesn't use the fossil -- just fall thru to DOS video, if   *
       * that's the mode we've specified.                                   */

      local_putc=(void (_stdc *)(int))fossil_putc;
      local_puts=(void (_stdc *)(char *))fossil_puts;
      break;
#endif

    case VIDEO_DOS:
      local_putc=(void (_stdc *)(int))fputchar;
      local_puts=(void (_stdc *)(char *))putss;
      break;

    case VIDEO_FAST:
#ifdef __MSDOS__
      local_putc=(void (_stdc *)(int))xputch;
      local_puts=(void (_stdc *)(char *))xputs;
      break;
#endif  /* else fall through...*/
#endif /* TTYVIDEO */

    default:
      displaymode = VIDEO_IBM;

    case VIDEO_IBM:
    case VIDEO_BIOS:
      StartUpVideo();
      
#ifndef ORACLE
      /* Don't display copyright window if we're restarting from before */
      
      if (restart_system)
        break;

  #ifdef MCP_VIDEO
        if (no_video)
        {
          dspwin=NULL;
          dspwin_time=0L;
        }
        else
        {
  #endif
          dspwin=WinMsg(BORDER_DOUBLE, col.pop_text, col.pop_border,
                        logo1, logo2, NULL);

          dspwin_time=timerset(DSPWIN_TIME*100);
  #ifdef MCP_VIDEO
        }
  #endif
#endif
      break;
  }

#ifdef KEY
  if (key_ok && key_load(key_fd, &key_size, &key_buf) != 0)
    key_ok=FALSE;
#endif

  /* Use default, unless specified otherwise on command-line */

  if (!fSetTask)
    task_num=prm.task_num;

  /* Unless specifically overridden, use the task num for the event file */

  if (event_num==(byte)-2)
    event_num=task_num;

#ifndef ORACLE
  Read_Events();
#endif

  if (!restart_system)
    ChatCleanUp();

  if (port==-1)
    port=prm.com_port;

  if (local==-2)
    local=TRUE;

  if (local)
    port=0xff;
  else if (port==-1 || port==0xff)
    port=0;

  snoop=(char)(prm.flags & FLAG_snoop);

#ifdef __MSDOS__
  (void)zfree("");/* Calculate free disk space here, so we don't */
                  /* have a long pause later down the road. Not  */
                  /* needed for OS/2, unless we're doing the     */
                  /* fancy logo bit.                             */
#endif


  Lputs(GRAY);

  if (*PRM(menupath))
    strcpy(menupath, PRM(menupath));
  else strcpy(menupath, PRM(sys_path));

  if (*PRM(rippath))
    strcpy(rippath, PRM(rippath));
  else strcpy(rippath, menupath);

  timeon=time(NULL);              /* Initalize time on/off counters */

#ifdef KEY
  if (key_ok)
    key_close(key_fd);
#endif


  /* Start up the input-processing threads */

  /*KickThreads();*/

#ifndef ORACLE

  if (do_useredit)
  {
    strcpy(usr.name, "\xff ");
    Config_Multitasker(FALSE);

    usr.video=GRAPH_ANSI;
    usr.bits |= BITS_TABS;
    usr.bits2 |= BITS2_CLS | BITS2_IBMCHARS;
    usr.width=80;
    usr.len=25;

    timeoff=timeon+(1440*60L);

    local=TRUE;

    OpenAreas();
    
    *log_name='\0';

    Fossil_Install(TRUE);
    User_Edit(NULL);

    AreaFileClose(ham);
    AreaFileClose(haf);

    ShutDownVideo();

    quit(0);
  }
  
#endif /* !ORACLE */

  timeoff=timestart+(unsigned long)(max_time*60L);
  /* Calc_Timeoff(); */

  do_timecheck=TRUE;


#ifndef ORACLE
  Blank_User(&usr);

  if (eqstr(log_name, c123)) /* No cmd-line param */
  {
    if (restart_system)
      *log_name='\0';
    else strcpy(log_name, PRM(log_name));
  }

  if (log_name && *log_name)  /* If we have a valid log name */
  {
    if (! LogOpen())
      quit(ERROR_CRITICAL);
    else
    {
      if (!restart_system)
        LogWrite("\n");
    }
  }

#ifdef KEY
  if (key_ok && (key_outbuf=malloc(key_size))==NULL)
    key_ok=FALSE;
#endif

  if (!restart_system)
  {
    extern int _stdc main();

    if (task_num)
      logit(log_begin_mt, version, task_num);
    else
      logit(log_begin_1t, version);
  }
  
#ifdef KEY
  if (key_ok)
  {
    key_strip_ofs=key_unpack1(key_buf, key_outbuf, key_size);
    key_free(key_buf);
  }
#endif

  /* Only do a log msg if we're NOT restarting the system */

  Config_Multitasker(!restart_system);

  if (!restart_system)
  {
    char szUserFile[PATHLEN];

    logit(" %s", PRM(system_name));

    strcpy(szUserFile, PRM(user_file));
    strcat(szUserFile, dotbbs);

    if ((fd=shopen(szUserFile, O_RDONLY | O_BINARY)) != -1)
    {
      struct _usr u;

      if (read(fd, (char *)&u, 240)==240)
      {
        if (u.struct_len != sizeof(u)/20)
        {
          logit("!FATAL!  User file record length in USER.BBS is invalid.");
          logit("!(If upgrading from Max 2.x, did you run \"CVTUSR -P\"?)");
          close(fd);
          quit(ERROR_CRITICAL);
        }
      }

      close(fd);
    }

    sprintf(temp, activexx_bbs, original_path, task_num);

    if (fexist(temp))
    {
      unlink(temp);

      if (task_num)
        sprintf(temp, lastusxx_bbs, original_path, task_num);
      else sprintf(temp, lastuser_bbs, original_path);

      if ((fd=shopen(temp, O_RDONLY | O_BINARY)) != -1)
      {
        read(fd,(char *)&user, sizeof(struct _usr));
        close(fd);

        logit(log_syscrash1, task_num);
        logit(log_syscrash2, user.name);
      }
    }

    Fossil_Install(TRUE);
  }

#ifdef KEY
  if (key_ok && do_checksum(key_outbuf, key_size) != 0)
    key_ok=FALSE;
#endif

  mi.req_version=MSGAPI_VERSION;
  mi.def_zone=prm.address[0].zone;

  mi.palloc=max_palloc;
  mi.pfree=max_pfree;
  mi.repalloc=max_repalloc;

  mi.farpalloc=max_farpalloc;
  mi.farpfree=max_farpfree;
  mi.farrepalloc=max_farrepalloc;

  
  if (MsgOpenApi(&mi)==-1)
  {
    logit(log_err_msgapi);
    quit(ERROR_CRITICAL);
  }

#ifdef KEY
  if (!key_ok)
  {
    Lputs(err_no_key);
    quitfile();
  }
#endif


  /* Connect with the MCP server */

#ifdef MCP
  ChatOpenMCP();
#endif

  /* This must happen AFTER fossil_install and logopen! */

  OS2Init();

#ifdef KEY
  key_unpack2(key_outbuf, key_size, key_strip_ofs);
#endif

#endif /* !ORACLE */

#if defined(KEY) && !defined(ORACLE)
  return key_outbuf + CODE_1_SIZE;
#else
  return NULL;
#endif
}


/* Reads in the xx.PRM file, compares dates between that and the .CTL, etc */

void Read_Prm(char *ctlname)
{
  int prmfile;
  long end;
  sword size;

#ifdef ORACLE
  NW(ctlname);
#else
  Compare_Dates(ctlname, prmname);
#endif

  if ((prmfile=shopen(prmname, O_RDONLY | O_BINARY))==-1)
  {
    Lprintf(cant_find_file, prm_txt, prmname);
    quitfile();
  }

  if (read(prmfile, (char *)&prm, sizeof(prm)) != sizeof(prm) ||
      prm.id != 'M' || prm.version != CTL_VER ||
      prm.heap_offset != sizeof(struct m_pointers))
  {
    Lputs(inval_prm_ver);
    quitfile();
  }

  /* Find out how long the file is */
  
  end=lseek(prmfile, 0L, SEEK_END);
  
  /* Now seek to the beginning of the variable-length heap */
  
  lseek(prmfile, (long)prm.heap_offset, SEEK_SET);

  size=(sword)(end-(long)prm.heap_offset);

  offsets=(char *)(malloc)(size);
  
  if (read(prmfile, offsets, size) != size)
  {
    Lprintf(cant_find_file, prm_txt, prmname);
    quitfile();
  }
  
  close(prmfile);

  /* Now figure out which main menu to display */

  if (*PRM(first_menu))
    strnncpy(main_menu, PRM(first_menu), MAX_MENUNAME-1);

  /* Also grab the current MCP pipe, if not overridden on the command-line */

#ifdef MCP
  if (! *szMcpPipe)
    strcpy(szMcpPipe, PRM(mcp_pipe));
#endif

  /* Set the timeout counter... */
 
  if (prm.input_timeout > 10) /* more than 60000 tics overflows a word */
    prm.input_timeout = 10;

  timeout_tics=((word)prm.input_timeout*60)*100;
  
  /* If it's less than one minute, default to four mins */

  if (timeout_tics < 6000)
    timeout_tics=4*60*100;

}

void Read_Access()
{
  char temp[PATHLEN];
#ifndef ORACLE
  int plevels;
  extern PLIST *pl_privs;
#endif

  sprintf(temp, ss, PRM(access), dotdat);
  if (!ClassReadFile(temp))
  {
    char temp2[PATHLEN];
    strcpy(temp2, PRM(sys_path));
    strcat(temp2, temp);

    if (!ClassReadFile(temp2))
    {
      Lprintf(cant_find_file, access_txt, temp);
      quitfile();
    }
  }

#ifndef ORACLE
  /* We allocate this here to avoid memory fragmentation */

  plevels=(int)ClassGetInfo(0,CIT_NUMCLASSES);
  if ((pl_privs=malloc((plevels+1)*sizeof(PLIST)))!=NULL)
  {
    int i;

    for (i=0; i < plevels; ++i)
    {
      pl_privs[i].name=ClassDesc(i);
      pl_privs[i].value=(int)ClassGetInfo(i,CIT_LEVEL);
    }
    pl_privs[i].name=NULL;
    pl_privs[i].value=-999;
  }
#endif
}


static void near Initialize_Colours(void)
{
  char temp[PATHLEN];
  int fd;
  
  sprintf(temp, "%scolours.dat", PRM(sys_path));
  
  if ((fd=open(temp, O_RDONLY | O_BINARY))==-1)
  {
    cant_open(temp);
    quit(ERROR_CRITICAL);
  }
  
  if (read(fd, (char *)&col, sizeof(col)) != sizeof(col))
  {
    logit(cantread, temp);
    quit(ERROR_CRITICAL);
  }
  
  close(fd);
}


#ifndef ORACLE
  unsigned int Decimal_Baud_To_Mask(unsigned int bd)
  {
    static struct _baudxlt
    {
      unsigned int decimal;
      word mask;
    } baud[]={{  300u, BAUD300},
              {  600u, BAUD600},
              { 1200u, BAUD1200},
              { 2400u, BAUD2400},
              { 4800u, BAUD4800},
              { 9600u, BAUD9600},
              {19200u, BAUD19200},
              {38400u, BAUD38400},
  #ifndef __MSDOS__
              {57600u, BAUD57600},
              {115200u,BAUD115200},
  #endif
              {0,0}
             };
    struct _baudxlt *b;

    for (b=baud; b->decimal; b++)
      if (bd==b->decimal)
        return b->mask;

    /* Not found, so default to 38.4k */
    return BAUD38400;
  }

#endif /* !ORACLE */


static void near OpenAreas(void)
{
#ifndef ORACLE
  if ((ham=AreaFileOpen(PRM(marea_name), TRUE))==NULL)
  {
    cant_open(PRM(marea_name));
    vbuf_flush();
    Local_Beep(3);
    maximus_exit(ERROR_FILE);
  }

  if ((haf=AreaFileOpen(PRM(farea_name), FALSE))==NULL)
  {
    cant_open(PRM(farea_name));
    vbuf_flush();
    Local_Beep(3);
    maximus_exit(ERROR_FILE);
  }
#endif
}







static void near install_handlers(void)
{
  /* Initialize the null-pointer check module */
  
  nullptr();  
}



static void near StartUpVideo(void)
{
  VidOpen(prm.flags2 & FLAG2_has_snow, multitasker==MULTITASKER_desqview,
          FALSE);

  if (!no_video)
    VidCls(CGREY);

  /* Turn on BIOS writes, but use same functions as the direct writes */

  if (displaymode==VIDEO_BIOS)
  {
#ifdef __MSDOS__
    VidBios(TRUE);
#endif
    displaymode=VIDEO_IBM;
  }

  WinApiOpen(FALSE);

  if ((win=WinOpen(0,
                   0,
                   VidNumRows()-((prm.flags & FLAG_statusline) ? 1 : 0),
                   VidNumCols(),
                   BORDER_NONE,
                   CGRAY,
                   CGRAY,
                   0))==NULL)
  {
    logit(mem_none);
    Local_Beep(3);
    maximus_exit(ERROR_CRITICAL);
  }

  local_putc=(void (_stdc *)(int))DoWinPutc;
  local_puts=(void (_stdc *)(char *))DoWinPuts;
}


void Load_Archivers(void)
{
  static int arcs_loaded=FALSE;
  
  if (!arcs_loaded)
  {
    #ifdef ORACLE
    ari=NULL;
    #else
    ari=Parse_Arc_Control_File(PRM(arc_ctl));
    #endif

    arcs_loaded=TRUE;
  }
}



void Local_Beep(int n)
{
  int n_beep;

  for (n_beep=n; n_beep--; )
  {
    #ifdef OS_2
      DosBeep(300, 250);
      DosSleep(100);
    #else
      fputc('\a', stdout);
    
      /* wc buffers stdout */
      fflush(stdout);
    #endif
  }
}


