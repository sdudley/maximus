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

/*# name=Maximus variable definitions and declarations
*/

#ifndef __MAXV_H_DEFINED_
#define __MAXV_H_DEFINED_

#ifdef MAX_INCL_VARS  /* if we need to include external variables */

#include "arc_def.h"

struct _proto_str
{
  sword num;
  char *name;
};


#ifdef MAX_INITIALIZE
  /* Xlation table for IBM hibit characters to straight ASCII characters */

  unsigned char nohibit[256]={
                 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
                 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
                 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44,
                 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,
                 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74,
                 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
                 90, 91, 92, 93, 94, 95, 96, 97, 98, 99,100,101,102,103,104,
                105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,
                120,121,122,123,124,125,126,'^','C','u','e','a','a','a','a',
                'c','e','e','e','i','i','i','A','A','E','a','A','o','o','o',
                'u','u','y','O','U','c','L','Y','P','f','a','i','o','u','n',
                'N','a','o','?','+','+','2','4','!','<','>','X','X','X','|',
                '+','+','+','+','+','+','|','+','+','+','+','+','+','+','+',
                '+','-','+','+','+','+','+','+','+','+','-','+','+','+','+',
                '+','+','+','+','+','+','+','+','+','X','X','X','X','X','a',
                'b','g','p','E','o','u','T','O','O','O','q','o','o','e','U',
                '=','+','>','<','(',')','%','=','o','.','.','/','n','2','X',
                ' '};

  struct _proto_str intern_proto[]=
  { {PROTOCOL_NONE,     ""},
    {PROTOCOL_XMODEM,   "Xmodem"},
    {PROTOCOL_XMODEM1K, "1K-Xmodem"},
    {PROTOCOL_ZMODEM,   "Zmodem"},
    {PROTOCOL_SEALINK,  "SEAlink"},
    {PROTOCOL_YMODEM,   "Ymodem"},
    {PROTOCOL_YMODEMG,  "GYmodem-G"},
    {-9999,             NULL}
  };


#else

  extern unsigned char nohibit[];
  extern struct _proto_str intern_proto[];

#endif


#ifdef BINK_PROTOCOLS
  extern byte *Txbuf,                       /* File-transfer TRANSMIT buffer */
              *Secbuf;                      /* File-transfer RECEIVE buffer */
#endif

extern unsigned cdecl _stklen;
extern int cdecl brk_trapped;   /* XTRNL variable to hold local ^C count!  */

/* Routines that puts chars and strings on the local screen.  Varies        *
 * by prm.video, and it's called using the macro lputc().                   */

typedef void (_stdc *std_arg_int)(int);
typedef void (_stdc *std_arg_charstar)(char *);

extrn void (_stdc *local_putc)(int ch) IS((std_arg_int)fputchar);
extrn void (_stdc *local_puts)(char *s) IS((std_arg_charstar)putss);

extrn struct m_pointers prm;      /* Everything in the .CTL/PRM file        */
extrn struct _arcinfo *ari;       /* Archiving programs                     */
extrn struct _usr usr;            /* Current user!                          */
extrn struct _usr origusr;        /* User record, as read when we started   */
extrn struct _bbs_stats bstats;   /* BBS statistics file                    */
extrn union stamp_combo new_date; /* Date for F*, L* new files listing      */
extrn union stamp_combo date_newfile; /* User's original new-files-date value*/
extrn union stamp_combo scRestrict;/* Only DL msgs newer than this date     */

#ifndef BINK_PROTOCOLS
extrn word *crctab;               /* 16-bit CRC table                       */
extrn dword *cr3tab;              /* 32-bit CRC table                       */
#endif

extrn FILE *chatlog;

extrn VWIN *win IS(NULL);       /* The main screen window                   */
extrn VWIN *dspwin IS(NULL);    /* Display window ("above" normal screen)   */
extrn long dspwin_time IS(0L);  /* Timeout for the display window           */


#ifdef UNIX
extrn char slogan[] IS(CLS "%sMAXIMUS Version %s%s\n");
#else
extrn char slogan[] IS(CLS "%sMAXIMUS Version %s%s\n");
#endif
                    
extrn char copyright[] IS("Copyright 1989, " THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");

extrn char dev_info[] IS(LMAGENTA "Design by Scott Dudley.\n"
                                  "Development by Scott Dudley, Peter Fitzsimmons and David Nugent.\n"
				  "UNIX port by Wes Garland.\n\n"

                         LRED "For technical support, send mail to tech@lanius.com or \"Tech\" at 1:249/106.\n"
                         LRED "For more information on Maximus, send mail to sales@lanius.com, or write to:\n\n"

                         YELLOW "    Lanius Corporation\n"
                                "    777 Downing St.\n"
                                "    Kingston, Ont.\n"
                                "    CANADA  K7M 5N3\n\n");

                       
extrn char fopen_read[] IS("r");     /* Flags for using fopen() calls */
extrn char fopen_readb[] IS("rb");   /* ... */
extrn char fopen_write[] IS("w");    /* ... */
extrn char fopen_writep[] IS("w+");  /* ... */
extrn char fopen_readpb[] IS("r+b");  /* ... */
extrn char fopen_writepb[] IS("w+b");  /* ... */
extrn char fopen_writeb[] IS("wb");  /* ... */
extrn char fopen_append[] IS("a");   /* ... */
extrn char blank_str[] IS("");
extrn char fn_format[] IS("%s%0.*s");
extrn char ansi_cls[] IS("\x1b[H\x1b[0;30;36m\x1b[2J\x1b[J");
extrn char ansi_blink[] IS("\x1b[5m");
extrn char ansi_up[] IS("\x1b[A");
extrn char ansi_down[] IS("\x1b[B");
extrn char ansi_left[] IS("\x1b[D");
extrn char ansi_right[] IS("\x1b[C");
extrn char ansi_cleol[] IS("\x1b[K");
extrn char ansi_goto[] IS("\x1b[%d;%dH");
extrn char ansi_goto1[] IS("\x1b[%dH");
extrn char ansi_gray[] IS("\x1b[0;37m");
extrn char log_mem_nolheap[] IS("!NOT ENOUGH MEMORY FOR LANGUAGE FILE HEAP!");
extrn char log_err_lang[] IS("!Language file err for '%s'");
extrn char tearline[] IS("\r--- %s %s\r"
                        " * Origin: %s (%s)\r"
                        "SEEN-BY: %u/%u\r\r");
extrn char quotes_misunderstood[] IS("quotes_m");
extrn char old_prm_file[] IS("OLD PRM FILE\n\n");
extrn char cant_find_file[] IS("Can't access %s file %s!");
extrn char prm_txt[] IS("PRM");
extrn char access_txt[] IS("ACCESS");
extrn char inval_prm_ver[] IS("Invalid .PRM file!\n");
extrn char huh[] IS("'%s'?\n\n");
extrn char numsign_misunderstood[] IS("num_m");
extrn char files_poo[] IS("%s$files%02x.poo");
extrn char files_bak[] IS("files.bak");
extrn char filesdat_name[] IS("files.dat");
extrn char filesdmp_name[] IS("files.dmp");
extrn char filesidx_name[] IS("files.idx");
extrn char master_idx[] IS("maxfiles.idx");


extrn char user_poo[] IS("$$$user");
extrn char user_poo_bbs[] IS("$$$user.bbs");
extrn char user_poo_idx[] IS("$$$user.idx");
extrn char user_bak[] IS("user.bak");
extrn char mem_nmsgb[] IS("!NOT ENOUGH MEMORY FOR MESSAGE BUFFER");
extrn char mem_nrmb[] IS("!NOT ENOUGH MEMORY FOR READ MESSAGE BUFFER");
extrn char mem_none[] IS("!NOT ENOUGH FREE MEMORY");
extrn char cmd_delim[] IS(" ;,\t\r\n");
extrn char path_delim[] IS(":\\/ \r\n\t");
extrn char pdel_only[] IS("\\/:");
extrn char ctl_delim[] IS(" \t\n");
extrn char dl_delim[] IS(" \t,;+");
extrn char zero[] IS("0");
extrn char one[] IS("1");
extrn char scanfile_name[] IS("%sscanfile.dat");
extrn char c123[] IS("\x01\x02\x03");
extrn char *percent_s IS("%s");

extrn char lastuser_bbs[] IS("%slastuser.bbs");
extrn char lastusxx_bbs[] IS("%slastus%02x.bbs");

extrn char restarxx_bbs[] IS("%srestar%02x.bbs");

/*         active_bbs[]   IS("%sactive.bbs"), */
extrn char activexx_bbs[] IS("%sactive%02x.bbs");
extrn char active_star[]  IS("%sactive*.bbs");

extrn char logformat[] IS("%c %02d %s %02d:%02d:%02d %s %s\n");
extrn char nameabbr[] IS("MAX ");
/*extrn char colon[] IS(":");*/
extrn char dot[] IS(".");
extrn char asterisk[] IS("*");
/*extrn char slash[] IS("/");*/
extrn char ps_lastread[] IS("%slastread.bbs");
extrn char ps_lastread_single[] IS("%slastread");
extrn char sq_lastread[] IS("%s.sql");
extrn char ss[] IS("%s%s");
extrn char goto_str[] IS("\x16\x08%c%c");
/*extrn char msg_intl[] IS("\x01INTL %d:%d/%d %d:%d/%d\r");*/
/*extrn char msg_fmpt[] IS("\x01""FMPT %d\r");*/
/*extrn char msg_topt[] IS("\x01TOPT %d\r");*/
extrn char msgtemp_name[] IS("%smsgtmp%02x.$$$");
/*extrn char white_n[] IS(WHITE "\n");*/
extrn char bbs_stats[] IS("%sbbstat%02x.bbs");
extrn char sp_bs[] IS(" \x08");
extrn char clear_string[] IS("\x16\x08%c%c\x19 %c");
extrn char fill_string[] IS("\x16\x08%c%c\x19%c%c");
extrn char attr_string[] IS("\x16\x01%c");
extrn char pd[] IS("%d");
extrn char pu[] IS("%u");
extrn char pl[] IS("%ld");
extrn char plu[] IS("%lu");
extrn char dotbbs[] IS(".bbs");
extrn char dotidx[] IS(".idx");
extrn char dotgbs[] IS(".gbs");
extrn char dotrbs[] IS(".rbs");
extrn char dotdat[] IS(".dat");
/*extrn char dotlog[] IS(".log");
extrn char dotctl[] IS(".ctl");*/
/*extrn char noaccess[] IS("NoAccess");*/
extrn char all[] IS("All");
extrn char active_x_bbs[] IS("active%x.bbs");
extrn char avt_home[] IS("\x16\x08\x01\x01");
extrn char bs_sp_bs[] IS("\x08 \x08");
extrn char n_n[] IS("\n\n");
extrn char qmark[] IS("?");
extrn char p2s[] IS("%2s");
extrn char eq[] IS("=");
/*extrn char comma[] IS(",");*/
extrn char opusxfer_name[] IS("%smaxxfer.%03x");
/*extrn char maketime_fmt[] IS("%2d %s %d %02d:%02d:%02d");*/
extrn char dot_dmp[] IS(".dmp");
extrn char dot_dat[] IS(".dat");
extrn char dot_star[] IS(".*");
extrn char dot_rep[] IS(".rep");
extrn char dot_msg[] IS(".msg");
extrn char rle_str[] IS("\x19%c%c");
extrn char mtag_dat[] IS("%smtag.dat");
extrn char mtag_idx[] IS("%smtag.idx");
extrn char mtag_fre[] IS("%smtag.fre");
/*extrn char *ftag_name IS("%sftag.bbs");*/
#ifndef UNIX
extrn char ipcxx_bbs[] IS("%sIPC%02x.BBS");
extrn char ipc_star[] IS("%sIPC*.BBS");
extrn char ipc_x[] IS("IPC%x.BBS");
#else
extrn char ipcxx_bbs[] IS("%sipc%02x.bbs");
extrn char ipc_star[] IS("%sipc*.bbs");
extrn char ipc_x[] IS("ipc%x.bbs");
#endif
extrn char xctl_keyboard[] IS("Keyboard\n");
extrn char xctl_port_baud[] IS("Port %u\nBaud %ld\n");
extrn char xctl_modem[] IS("Modem %x %lx %x %x %lx\n");
extrn char xctl_time[] IS("Time %u\n");
extrn char xctl_log[] IS("Log %s\n");
extrn char xctl_msgs[] IS("Messages %s\n");
extrn char xctl_uploads[] IS("Uploads %s\n"); 
extrn char xctl_downloads[] IS("Downloads %s\n");
extrn char xctl_help[] IS("Help %sHlp\n");
extrn char xctl_filesbbs[] IS("FilesBbs %s\n");
extrn char mnu_file[] IS("FILE"); /* default name of the file menu */
extrn char actrack_colon[] IS("ACTRACK:");
#ifdef KEY
extrn char err_no_key[] IS("Error!  Key file MAXIMUS.KEY does not exist or is invalid!\n");
#endif

/**/
extrn char *gkey_info;                  /* Pointer to key info */
extrn byte menuhelp;
extrn byte finished IS(FALSE);
extrn char *offsets IS(NULL);           /* Pointer to PRM info.            */
extrn char *menuname IS(NULL);          /* Pointer to mname[] in MAX_MENU  */
extrn char *original_prompt IS(NULL);
extrn char *orig_path2[LEN(MAX_DRIVES)];/* What dir we started in, alt1    */
extrn char *orig_path3[LEN(MAX_DRIVES)];/* What dir we started in, alt2    */
extrn char main_menu[MAX_MENUNAME] IS("MAIN"); /* unless overriden in max.prm */
extrn char files_bbs[13] IS ("files.bbs");
extrn char sfiles[13] IS ("files");
extrn char last_onexit[LEN(PATHLEN)] IS("");
extrn char orig_disk2;                  /* Disk drive we started from, alt1*/
extrn char orig_disk3;                  /* Disk drive we started from, alt2*/
extrn char original_path[PATHLEN];      /* The DRIVE/PATH we started from  */
#ifdef UNIX
extrn char prmname[LEN(80)] IS("etc/max.prm");/* Name of current .PRM file     */
#else
extrn char prmname[LEN(80)] IS("max.prm");/* Name of current .PRM file     */
#endif
extrn char searchfor[LEN(BUFLEN)];      /* Text to search FILES.BBS for    */
extrn char log_name[LEN(80)];           /* Name of log file                */
extrn char menupath[LEN(80)];           /* Path to the current menu files. */
                                        /* Can be mod'ed by barricades.    */
extrn char rippath[LEN(80)];            /* Path to the current rip files.  */

#ifdef MCP
extrn char szMcpPipe[LEN(80)];          /* Pipe for MCP                    */
#endif

extrn char fix_menupath[LEN(MAX_MENUNAME)]; /* Name to adjust menupath to  */
extrn char firstname[LEN(36)];          /* User's first name.              */
extrn union stamp_combo next_ludate;    /* Ludate to insert at logoff      */
extrn char last_readln[MAXLEN+1];
extrn byte linebuf[LEN(BUFLEN+1)];      /* Line input bfr for stacked cmds */
extrn char arq_info[ARQ_LEN];           /* MNP/V42b info                   */
extrn char usrname[LEN(sizeof(usr.name))]; /* Name/alias of current user   */


extrn char local;                       /* If we're in local mode          */
extrn char displaymode;                 /* Local display mode              */
extrn char snoop;                       /* If we see what's on the screen. */
extrn char keyboard;                    /* If we can type.  Implies snoop. */
extrn byte task_num;                    /* What our task number is         */
extrn byte fSetTask;                    /* True if -n spec'd on cmd line   */
extrn char no_local_output;             /* Local output suppressed         */
extrn char no_remote_output;            /* Remove output suppressed        */
extrn byte event_num;                   /* Which event file to use?        */
extrn char caller_online;               /* If caller currently on-line     */
extrn char do_timecheck;                /* If we check the user's time     */
extrn char fossil_initd;                /* If the FOSSIl has been started  */
extrn char curattr IS(DEFAULT_ATTR);    /* Current local screen attribute  */
extrn char mdm_attr IS(DEFAULT_ATTR);   /* Current remote screen attribute */
extrn byte lastmenu;                    /* The last ^OR response           */
extrn char protocol_letter;             /* First letter of xtrnl protocol  */
extrn char written_echomail;            /* User wrote echomail message?    */
extrn char written_matrix;              /* User wrote matrix message?      */
extrn char written_local;               /* User wrote local message?       */
extrn char written_conf;                /* User wrote conference message?  */
extrn char sent_time_5left;             /* If issued a "5minleft" warn     */
extrn char sent_time_almostup;          /* If issued a "t.almostup" warn   */
extrn char barricade_ok;                /* If  barricade pwd was asked     */
extrn char wrap;                        /* If the current line has wrapped */
extrn char first_search;                /* 1st area srched with L)ocate    */
extrn char inmagnet;                    /* If we're in the MagnEt editor   */
extrn char multitasker;                 /* Which Mtask we're under         */
extrn char restart_system;              /* If restarting the current user  */
extrn char erl;                         /* Errorlevel exited with          */
extrn char in_file_xfer;                /* Force logit() calls to display  */
extrn char in_mcheck;                   /* If we're in the mailchecker     */
extrn char create_userbbs;              /* Create USER.BBS?                */
extrn char this_logon_bad;              /* If user failed current logon    */
extrn char inchat;                      /* If we're in sysop chat mode     */
extrn char locked;                      /* If priv level is locked         */
extrn char chatreq;                     /* If user req'd chat this call    */
extrn char do_useredit;                 /* Run user-editor at startup      */
extrn char fthru_yuhu;                  /* If YooHoo has fallen through    */
extrn char shut_up;                      /* If timelimit should be shut up  */
extrn char nowrite_lastuser;            /* Don't write LASTUSxx.BBS at exit*/
extrn char ul_no_space;                 /* Ran out of space during upload  */
extrn char dsp_set;                     /* If 'displaymode' has been set   */
extrn char mn_dirty;                    /* If menuname var has changed     */
extrn char no_zmodem;                   /* Turn off internal Zmodem        */
extrn char no_shell;                    /* Turn off local Alt-J command    */
extrn char no_video;                    /* No local video                  */
extrn char mcp_video;                   /* Send video via pipes            */
extrn char last_maxed;                  /* If last editor was MaxEd        */
extrn char in_node_chat;                /* If user is in multi-node chat   */
extrn char chkmail_reply;               /* If we're doing a CheckMail reply*/
extrn char do_caller_vanished;          /* If caller hung up               */
extrn char waitforcaller;               /* Wait and grab caller ourselves  */
extrn char in_wfc;                      /* Currently waiting for a caller  */
extrn char in_msghibit;                 /* In a msg action that might permit hibits */
extrn char log_wfc;                     /* Put log msgs in WFC window      */
extrn char no_dcd_check;                /* Disable DCD detection           */
extrn char port_is_device;              /* True if 'port' contains a       *
                                         * COMx port number, not a file    *
                                         * handle.                         */
#ifdef BINK_PROTOCOLS
extrn char got_dupe;                    /* If dupe file rec'd via xm/ym/sea*/
#endif
extrn char debug_ovl;                   /* Debug overlays?                 */
extrn byte cYes;                        /* Static vars for y/n/= prompts   */
extrn byte cNo;
extrn byte cNonStop;
extrn char *szHeyDude;                  /* [apb] template */
extrn char *szPageMsg;                  /* chat page template */

extrn int cdecl port;                   /* Actual com port we're using     */

extrn int fLoggedOn;                    /* Has user logged on yet?         */
extrn int cls;                          /* Class num. of current user      */
extrn int matches;                      /* # of matches in FILES.BBS       */
extrn int menu_lines;                   /* #of lines menu options take up  */
extrn int next_menu_char;               /* Execute this char as the next   *
                                         * menu option (from MoreYnns_Read)*/


/*extrn int last IS(MENU_OTHER);*/      /* Last menu-file displayed        */
extrn int lockpriv;                     /* The current user's locked priv  */
extrn byte debuglog IS(FALSE);          /* debug statements in log         */
extrn word num_yells;                    /* Number of yells                 */
extrn int datelen;                      /* Max length of MsgDate() output  */


extrn unsigned long timestart;          /* When Max execution started      */
extrn unsigned long timeon;             /* Timestmp whn usr got on systm   */
extrn unsigned long timeoff;            /* Timestmp whn usr mst be off sys */
extrn unsigned long getoff;             /* timestart+(max_time*60L)        */
extrn unsigned long max_time;           /* -t parameter                    */
extrn unsigned long baud;               /* User's current baud rate        */
extrn unsigned long last_bps;           /* Transfer rate of last file sent */
extrn unsigned long input_timeout;      /* Timer for caller sleep timeout  */
extrn word          timeout_tics;       /* Default # of csecs before t'out */

extrn long ultoday;                     /* Kbytes uploaded on this call    */
extrn dword last_lastread;              /* Initial val of lread ptr        */

extrn long rst_offset;                  /* Used with -r and .BBS file      */

extrn unsigned int current_baud;        /* Current baud, in format         */
                                        /* acceptable by mdm_baud call.    */
                                        /* Use the normal `baud' if you    */
                                        /* need the numeric value.         */

extrn dword steady_baud_l;              /* Baud rate (below is mask)       */
extrn unsigned int steady_baud;         /* steady_baud is the baud mask    */
                                        /* to lock the COM port at, if     */
                                        /* any.                            */

extrn unsigned char display_line;       /* # of lines since last More[Y,n] */
extrn unsigned char display_col;        /* Column# since last More[Y,n]    */
extrn unsigned char current_line;       /* Actual line of screen we're on  */
extrn unsigned char current_col;        /* Actual col of screen we're on   */

extrn struct _maxcol col;               /* Max colour information          */
extrn int last_protocol;                /* Last protocol download          */
extrn byte fFlow IS(FALSE);             /* If flow control restriction     */
extrn byte no_dtr_drop IS(FALSE);       /* If we're not to drop DTR        */
extrn byte no_traptrap IS(FALSE);       /* Don't trap traps (OS/2)         */

extrn PCLH pclh;                        /* Class information               */

extrn struct callinfo sci;              /* Caller information              */

#ifndef OS_2
extrn void (far pascal *sleeper)(void) IS(NULL); /* Give away timeslice f()*/
#endif

#ifdef DEBUG_OUT
extrn char dout_log IS(FALSE);
#endif

#include "language.h"

#include "max_con.h" /* constants */

extern char comp_date[];
extern char comp_time[];
extern LLPUSH lam, laf; /* Current message/file area handles */

#endif /* MAX_INCLUDE_VARS */

#endif /* __MAXV_H_DEFINED_ */

