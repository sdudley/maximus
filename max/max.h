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

/*# name=Main Maximus header file
*/

#ifndef __MAX_H_DEFINED
#define __MAX_H_DEFINED

/***************************************************************************
 *    Conditional flags for the entire program, and portability stuff      *
 ***************************************************************************/

#define CANENCRYPT          /* Do we support encryption of passwords? */
#define MUSTENCRYPT         /* Should we always encrypt the user's password
                             * at logon, even if they don't try to change it?
                             * (This requires CANENCRYPT.)
                             */
#define CHANGEENCRYPT       /* Passwords are always encrypted when the user
                             * changes it or logs on for the first time */
#define TTYVIDEO            /* Local video supported via fputc() et al */
#define MAXIMUS             /* Who are we? */
#define MEX                 /* Include MEX support */
#define MAX_TRACKER         /* Use message tracking subsystem */
/*#define FLUSH_LOG*/
#define INTERNAL_PROTOCOLS
#define SHORT_MDM_CMD

#if !defined(__FLAT__) && !defined(ORACLE)
  /*#define KEY*/               /* Use key file */
#endif

#ifdef OS_2
  #define MCP

  #ifndef ORACLE
    #define MCP_VIDEO
  #endif

  #ifndef __FLAT__
    /*#define MAXSNOOP*/
  #endif
#endif


/***************************************************************************
                          Initialization of variables
 ***************************************************************************/

#ifdef NOVARS
  #ifndef NOINIT
    #define INITIALIZE_STATIC
  #endif
#endif

#ifdef MAX_INITIALIZE
  #define extrn
  #define IS(x) =x
  #define LEN(x) x
#else
  #define extrn extern
  #define IS(x)
  #define LEN(x)
#endif

#include <time.h>

#ifndef __COMPILER_H_DEFINED
  #include "compiler.h"
#endif

#ifndef __TYPEDEFS_H_DEFINED
  #include "typedefs.h"
#endif

#include "utime.h"

#ifndef __PROG_H_DEFINED
  #include "prog.h"
#endif

#ifndef __SQAPI_H_DEFINED
  #include "msgapi.h"
#endif

#ifndef __EVENTS_H_DEFINED
  #include "events.h"
#endif

#ifndef __DV_H_DEFINED
  #include "dv.h"
#endif

#ifndef __WIN_H_DEFINED
#include "win.h"
#endif

#ifdef MAX_INCL_COMMS
#include "modem.h"
#endif

#ifdef PATHLEN
#undef PATHLEN
#endif

#ifdef BLINK
#undef BLINK
#endif

#ifdef DLE
#undef DLE
#endif

#ifdef lputs
#undef lputs
#endif


/***************************************************************************
                          Miscellanious Macros
 ***************************************************************************/

#ifdef __TURBOC__
#define isupsp(c) (_ctype[(c) + 1] & (_IS_UPP | _IS_SP))
#else
#define isupsp(c) (isupper(c) || isspace(c))
#endif

#define MNU(m,o)            (((m).menuheap)+(m).o)
#define Mdm_keyp()          (Mdm_kpeek() != -1)
#define Clear_KBuffer()     *linebuf='\0'
#define Save_Directory(path) Save_Dir(&orig_disk,orig_path,path)
#define Restore_Directories() Restore_Dir(&orig_disk,orig_path)
#define Save_Directory2(path) Save_Dir(&orig_disk2,orig_path2,path)
#define Restore_Directories2() Restore_Dir(&orig_disk2,orig_path2)
#define Save_Directory3(path) Save_Dir(&orig_disk3,orig_path3,path)
#define Restore_Directories3() Restore_Dir(&orig_disk3,orig_path3)
#define Yes_or_No(expr)     ((expr) ? yes : no)
#define Sysop_Yes_or_No(expr) ((expr) ? sysop_yes : sysop_no)
#define loc_kbhit()         (loc_peek() != -1)
#define Input(d,t,c,m,p)    Inputf(d,t,c,m,"%s",p)
#define Input_Char(t,x)     Input_Charf(t,"%s",x)
#define InputGetsWNH InputGets
#define InputGetseNH InputGetse
#define InputGetsNH  InputGetsL
#define mdm_getsp(d,m)      Inputf(d,INPUT_MSGENTER | INPUT_NLB_LINE,0,m,NULL)
#define mdm_getspnc(d,m)    Inputf(d,INPUT_MSGENTER | INPUT_NLB_LINE  | INPUT_NOCLEOL,0,m,NULL)
#define KeyGetRNP(p)        Input_Charf(CINPUT_DISPLAY | CINPUT_PROMPT | CINPUT_NOXLT | CINPUT_DUMP, p)
#define GetYnAnswer(p,t)    GetListAnswer(CYn,NULL,useyforyes,t,percent_s,p)
#define GetyNAnswer(p,t)    GetListAnswer(yCN,NULL,useyforyes,t,percent_s,p)
#define GetYnnsAnswer(p,t)  GetListAnswer(Yne,NULL,useyforyesns,t,percent_s,p)
#define GetYnhAnswer(h,p,t) GetListAnswer(CYnq,h,useyforyes,t,percent_s,p)
#define GetyNhAnswer(h,p,t) GetListAnswer(yCNq,h,useyforyes,t,percent_s,p)
#define UserHasKey(k)       (usr.xkeys & (1L << (dword)(k)))
#define UserKeyOff(k)       (usr.xkeys &= ~(1L << (dword)(k)))
#define UserKeyOn(k)        (usr.xkeys |= (1L << (dword)(k)))
#define MsgAreaHasLock(a,l) ((a).msglock & (1L << (dword)(l)))
#define FileAreaHasLock(a,l)((a).filelock & (1L << (dword)(l)))
#define GEPriv(p1,p2)       ((word)(p1) >= (word)(p2))
#define LEPriv(p1,p2)       ((word)(p1) <= (word)(p2))
#define Goto(xx,yy)         Printf(goto_str,(char)(xx),(char)(yy))
#define lputc(c)            (*local_putc)(c)
#define lputs(s)            (*local_puts)(s)
#define isalnumpunct(ch)    (! is_wd(ch))
#define UIDnum(n)           ((prm.flags2 & FLAG2_UMSGID) ? MsgMsgnToUid(sq, (n)) : (n))
#define hasRIP()            (!local && (usr.bits & BITS_RIP))

/***************************************************************************
                            Constant #define's
 ***************************************************************************/


#define RO_NEWMENU        0x0001  /* Display a new menu */
#define RO_QUIT           0x0002  /* Quit to the prior menu */
#define RO_SAVE           0x0004  /* Quit to the prior menu (and save msg) */

#define PA_NOPWD          0x0001    /* Don't ask for pwd in this area */

#define MAX_BUF_STROKE    64        /* Max keys in MCP typeahead buf        */
#define MAX_KEYS          32        /* Max number of keys (0..8, A..X)      */
#define MRL_QEXP          0x0001    /* Expand multi-line quotes             */

/*efine MAXEXPAND         30        // max wildcard expansion               */
#define MAX_INITIALS      10        /* Max # of intials for quoted text     */
#define HARD_SAFE         2         /* 4 chars from user's right margin     */
#define SOFT_SAFE         20        /* Use smaller margin for soft-CR text  */

#define EXP_NONE          0         /* Nothing was found                    */
#define EXP_YES           1         /* We found something; ready to DL.     */
#define EXP_ERR           2         /* We got an error, and warnded the usr */


#define MAX_OTEAR_LEN     256       /* Max length of origin + tear line     */
#define MAX_FN_LEN        12        /* Max length of a filename             */
#define DL_NOTFOUND       0x00      /* Return codes from IsInFilesBbs       */
#define DL_FOUND          0x01
#define DL_NOTIME         0x02
#define DL_NOBYTES        0x04

/* These constants must be kept current with the FFLAG_ definitions in
 * max.mh!
 */

#define FFLAG_TAG         0x0001    /* File was T)agged                     */
#define FFLAG_THIS1       0x0002    /* Selected during THIS wcard expansion */
#define FFLAG_NOTIME      0x0004    /* Don't debit time for this file       */
#define FFLAG_NOBYTES     0x0008    /* Don't debit DL bytes for this file   */
#define FFLAG_EXP         0x0010    /* Filename was expanded from wildcard  */
#define FFLAG_NOENT       0x0020    /* This file is not on disk             */
#define FFLAG_OK          0x0040    /* This file previously verified for DL */
#define FFLAG_NOLIST      0x0080    /* File is not in FILES.BBS             */
#define FFLAG_GOT         0x0100    /* This file was uploaded               */
#define FFLAG_SENT        0x0200    /* This file was downloaded             */
#define FFLAG_STAGE       0x0400    /* Copy file to staging area before xfer*/
#define FFLAG_SLOW        0x0800    /* File is from from an FA_SLOW area    */
#define FFLAG_DUPE        0x8000    /* File is a dupe                       */


#define SWFLAG_NOSCAN     0x01      /* Don't scan lr ptr when entering area *
                                     * (Switch_Msg_Area())                  */

#define VA_NOVAL          0x0000    /* Don't check if dir exists */
#define VA_VAL            0x0001    /* Check if dir exists */
#define VA_PWD            0x0002    /* Check barricade if necessary */
#define VA_OVRPRIV        0x0004    /* Check everything except priv/locks */
#define VA_EXTONLY        0x0008    /* Used only in conj with VA_PWD.  If   *
                                     * the area has a passworded barricade, *
                                     * validmsgarea returns false.  If the  *
                                     * area has an extended barricade,      *
                                     * return true or false depending on    *
                                     * whether or not the user is able to   *
                                     * access the area.                     */

#define REASON_TIME       0x00      /* Reasons for user expiration          */
#define REASON_DATE       0x01

#define INIT_TIME         60000u    /* Reinit modem every 10 mins           */
#define MAX_EVENT         48        /* Maximum # of events in EVENTS.BBS    */
#define ARQ_LEN           40        /* Length of the ARQ-string buffer      */

#define COL_POPUP_BOR     (CYELLOW | _BLUE)

#define MSGENTER_UP     0x7fff /* Return codes for mdm_gets() - item--      */
#define MSGENTER_DOWN   0x7ffe /* Ditto -                       item++      */

#define TAG_NONE             0 /* Don't tag anything - normal download      */
#define TAG_ONELINE          1 /* Try not to send '\n'. For file listings   */
#define TAG_VERBOSE          2 /* We can be vebose about it.                */

#define UL_TRUNC         8192L /* Max length of an uploaded message         */
#define MAX_FBBS_ENTRY 1024+13 /* Max length of a line in FILES.BBS         */

#define DSPWIN_TIME          4 /* Stay on-screen for up to 4 seconds        */
#define ROBO_TIME          240L/* Max # of secs Max can go w/o DCD          */
#define ROBO_WARN          220L/* And after how many secs do we warn sysop? */

#define STATUS_NORMAL     0x00
#define STATUS_REMOVE     0x01
#define STATUS_FORCE      0x02

#define IREADLEN            60
#define MAXED_SAVE_MEM   4000L /* How much mem to save while in MaxEd       */
#define BORED_SAVE_MEM   4000L /* How much mem to save in BORED             */
#define MAXLEN    85           /* size of primary line-input buffer         */

#define CMSG_PAGE       0x00   /* "You're being paged by another user!"     */
#define CMSG_ENQ        0x01   /* "Are you on this chat channel?"           */
#define CMSG_ACK        0x02   /* "Yes, I AM on this channel!"              */
#define CMSG_EOT        0x03   /* "I'm leaving this chat channel!"          */
#define CMSG_CDATA      0x04   /* Text typed by used while in chat          */
#define CMSG_HEY_DUDE   0x05   /* A normal messge.  Always displayed.       */
#define CMSG_DISPLAY    0x06   /* Display a file to the user                */

#define MAX_TASK  256           /* Maximus number of tasks                  */
#define MAX_TRIES 30            /* # of times to retry IPCxx.BBS operation  */
#define MAX_OVR            16   /* Maximum # of override privs per area     */
#define MAX_MENUNAME  PATHLEN   /* Max. length of menuname[].               */
#define MAX_QUOTEBUF       50   /* Maximum number of quote windows to page  */
#define COPY_SIZE       16384   /* Default buffer size for copy_file()      */
#define RST_VER             3   /* Version number of RESTARxx.BBS           */
#define MAX_AREAS        1296   /* Maximum number of total areas -- This #  *
                                 * can't be changed, but it is handy to     *
                                 * have a def. for internal calculations.   *
                                 * 1296==(26+10)^2, since each of the       *
                                 * two digits of the area number can be     *
                                 * 0-9 or A-Z.                              */
#define NUM_AIDX           80   /* Number of AREAS.IDX entries at once      */
#define NUM_NDI           160   /* Number of nodelist entries to read in    */
#define USR_SEARCH         32   /* Number of users to read in at a time     */
#define MAX_LINE          255   /* Max. length of a .CTL file line!         */
#define MAX_MSGDISPLAY     26   /* Max # of lines that Msg_Display() will   *
                                 * req. from Msg_Read_Lines at once.        */
#define MAX_MSGLINES      256   /* Max # of lines you can req. from         *
                                 * Msg_Read_Lines at once.                  */
#define MAX_MSGWIDTH      129   /* Max. width of each message line          */
#define MAX_PRINTF       2048   /* Max. number of output ...printf() chars  */
#define SEEK_SIZE          64   /* # of bytes of ZIPfile to search for      *
                                 * header in at a time.  This MUST not      *
                                 * be higher than 64, 'bcuz we may get      *
                                 * into problems with NESTED ZIPfiles if    *
                                 * it is any larger.                        */
#define MAX_NEST            8   /* Max. number of .BBS files to nest.       *
                                 * We can go more, but we'd have to         *
                                 * increase the stack size quite a bit!     */
#define MAX_BUSYLOOP       35   /* Max. number of times we should loop      *
                                 * waiting for a file to become un-busy     */
#define FILEBUFSIZ        512   /* Size of display-file read-in buffer      */
#define FWD_SAFE          256   /* Max addition for origin line et al.      */
#define FWDBUFSIZE        512   /* #ofbytes to copy at a time for msg. fwd  */
#define FWD_OVERLAP        40   /* #ofbytes to overlap for msg. fwd         */
#define CHANGE_SCREENLEN   85   /* # of #'s to print when testing user's    *
                                 * screen length.                           */
#define RETIMEOUT          60   /* #of seconds until hangup after 1st       *
                                 * timeout warning (1 min.)                 */
#define MAX_EXECARGS       64   /* Max. number of parameters when invoking  *
                                 * an external program                      */
#define BMSGBUFLEN (MSGBUFLEN*3)/* Amount to allow for msgbuffer            */
#define MSGBUFLEN         512   /* Max. num of characters of message to     *
                                 * hold at any one time.  No less than 80   */
#define WORDBUFLEN         78   /* Length of wordwrap buffer                */
#define DEFAULT_ATTR        3   /* Cyan on black                            */
#define PATHLEN           120   /* MAXIMUM length of any path\filename.ext  */
#define BUFLEN             80   /* Size of primary line input buffer        */
#define PROMPTLEN          90   /* Max. length of any prompt                */
#define DLE                16   /* DLE character used in BBS file display   */
#define PREREGISTERED  0xfffe   /* Priv. value that means let no new        *
                                 * callers on the system.                   */

/***************************************************************************
                   Bit Masks and Function Behaviour Controls
 ***************************************************************************/

#define GRAPHICS_ENTRY_FUNCS 6  /* These are values to be passed to         */
                                /* Get_Msg_Attr()...                        */
#define ITEM_ATTR   0           /*                                          */
#define ITEM_FROM   1           /*                                          */
#define ITEM_ORIG   2           /*                                          */
#define ITEM_TO     3           /*                                          */
#define ITEM_DEST   4           /*                                          */
#define ITEM_SUBJ   5           /*                                          */


#define MSGLINE_NORMAL    0x00 /* Type of line currently being displayed    */
#define MSGLINE_SEENBY    0x01 /* in message...                             */
#define MSGLINE_KLUDGE    0x02 /*                                           */
#define MSGLINE_QUOTE     0x04 /*                                           */
#define MSGLINE_END       0x08 /*                                           */
#define MSGLINE_SOFT      0x10 /* THIS line wordwraps onto the next         */
#define MSGLINE_LASTWRAP  0x20 /* LAST line was wordwrapped onto this       */

#define DIRECTION_PREVIOUS 0x00/* Direction of message reading              */
#define DIRECTION_NEXT    0x01 /*                                           */

#define CTL_NONE          0x00 /* What type of .CTL to generate for         */
#define CTL_NORMAL        0x01 /* external programs.                        */
#define CTL_UPLOAD        0x02
#define CTL_DOWNLOAD      0x03

#define OUTSIDE_ERRORLEVEL 0x01/* Method to use for outside command!        */
#define OUTSIDE_CHAIN     0x02
#define OUTSIDE_RUN       0x03
#define OUTSIDE_DOS       0x04
#define OUTSIDE_CONCUR    0x05
#define OUTSIDE_NOFIX     0x20 /* Don't try to readjust lread ptr           */
#define OUTSIDE_REREAD    0x40 /* Re-read LASTUSxx.BBS after execution      */
#define OUTSIDE_STAY      0x80 /* Stay in current directory when going out! */

#define ERROR_MINFILE        2 /* The different errorlevel exits we use     */
#define ERROR_FILE           3 /* for various reasons.                      */
#define ERROR_NOFOSSIL       3
#define ERROR_PARAM          3
#define ERROR_CRITICAL       3
#define ERROR_RECYCLE        2
#define ERROR_KEXIT          1

/* Specifies type of local video, used for `displaymode'. */

#define VIDEO_DOS         0x00 /* Standard DOS output hooks */
#define VIDEO_FOSSIL      0x01 /* FOSSIL write-character function */
#define VIDEO_IBM         0x02 /* Direct screen writes */
#define VIDEO_FAST        0x03 /* Semi-fast undocumented DOS call */
#define VIDEO_BIOS        0x04 /* Semi-faster int 10h BIOS writes */

#define SKIP_NONE            0 /* For the ^Pxxx commands in .BBS files.   */
#define SKIP_FILE            1
#define SKIP_LINE            2



#define DO_EITHER     0x00
#define DO_THEN       0x01
#define DO_ELSE       0x02

#define VAR_RETCODE   0x00

#define ACTION_EQ     0x00
#define ACTION_NE     0x01
#define ACTION_GT     0x02
#define ACTION_LT     0x03
#define ACTION_GE     0x04
#define ACTION_LE     0x05



/* Definitions for the Input() function.  Remember to use no more than ONE
   of `INPUT_echo' or `INPUT_alreadych'!                                   */

#define INPUT_LB_LINE     0x0001 /* Input a line, utilizing linebuf         */
#define INPUT_NLB_LINE    0x0002 /* Input a line, NOT utilizing linebuf     */
#define INPUT_WORD        0x0004 /* Input a word, utilizing linebuf         */

#define INPUT_ECHO        0x0008 /* `ch' contains character to echo back.  */
#define INPUT_ALREADYCH   0x0010 /* `ch' contains a character that has     *
                                  * already been entered by the user.      */
#define INPUT_SCAN        0x0020 /* Allow scan codes to be returned        */
/*#define INPUT_NOHOTFLASH  0x0020*/ /* Don't go up to the top line of screen! */
#define INPUT_NOCTRLC     0x0040 /* Don't allow user to ^C to dump output  *
                                  * and redisplay prompt.                  */
#define INPUT_MSGENTER    0x0080 /* If we're currently entering message    *
                                  * header info, and the arrow keys are    *
                                  * allowed to do something...             */
#define INPUT_NOLF        0x0100 /* Don't send LF at end of input function */
#define INPUT_MSGREAD     0x0200 /* Allow Prev/Next arrow keys             */
#define INPUT_WORDWRAP    0x0400 /* Word-wrap this string                  */
#define INPUT_NOCLEOL     0x0800 /* Don't ever issue CLEOL codes           */
#define INPUT_DEFAULT     0x1000 /* Default response is in string 's'.     */
#define INPUT_NOECHO      0x2000 /* Don't echo output at all               */


/* Definitions for the Input_Char() function.  Remember to use no more than
   ONE of `CINPUT_acceptable' or `CINPUT_prompt'!                          */


#define CINPUT_DISPLAY    0x0001 /* If we display the character entered,if  *
                                  * we're in hotkey mode.  If we're not, we *
                                  * *always* display the characters entered */
#define CINPUT_RTNHELP    0x0001 /* For GetListAnswer() only - returns the  *
                                  * `help' key as well after displaying the *
                                  * help file to allow screen redraw etc.   */
#define CINPUT_ACCEPTABLE 0x0002 /* `extra' specifies a list that the       *
                                  * entered character must be one of.       *
                                  * Otherwise, we re-prompt and ask again.  */
#define CINPUT_PROMPT     0x0004 /* Display the prompt `extra' before       *
                                  * trying to get the character.            */
#define CINPUT_LASTMENU   0x0008 /* If set, we set `lastmenu' to be the     *
                                  * character the user pressed.  Otherwise, *
                                  * we simply return() it.                  */
#define CINPUT_SCAN       0x0010 /* Return scan codes if necessary */
#define CINPUT_NOXLT      0x0020 /* Don't translate special characters,     *
                                  * such as \r and \n, into their ASCII     *
                                  * equivilents.  (ie. '\r'=='|')           */
#define CINPUT_NOCTRLC    0x0040 /* Don't allow user to ^C to dump output   *
                                  * and redisplay prompt.                   */
#define CINPUT_P_CTRLC    0x0080 /* Only display prompt if user ^C's...     */
#define CINPUT_NOLF       0x0100 /* Don't display LF */
#define CINPUT_FULLPROMPT 0x0200 /* The GetListAnswer() o_prompt parm does  *
                                  * include the choices                     */
#define CINPUT_ALLANSWERS 0x0400 /* If we need an answer or not             */
#define CINPUT_DUMP       0x0800 /* DUMP output once we get a character     */
#define CINPUT_MSGREAD    0x1000 /* Arrow keys for msg.reading              */
#define CINPUT_NOUPPER    0x2000 /* Don't do a toupper() on the character   */
#define CINPUT_AUTOP      0x4000 /* For hotkeys, display prompt anyway      */
#define CINPUT_ANY        0x8000 /* For hotkeys, display prompt anyway      */


#define RESTART_MENU      0x00  /* If restarting at a menu                */
#define RESTART_DOTBBS    0x01  /* If restarting in middle of .BBS file   */

#define NUM_MENU           256  /* Max. # of options in a menu file       */


/* Defines for *.MNU: */

#define AREATYPE_LOCAL    0x01  /* Tells which type (and which type ONLY)  */
#define AREATYPE_MATRIX   0x02  /* that command can be used in.            */
#define AREATYPE_ECHO     0x04
#define AREATYPE_CONF     0x08

#define AREATYPE_ALL      (AREATYPE_LOCAL | AREATYPE_MATRIX |   \
                           AREATYPE_ECHO | AREATYPE_CONF)

#define HEADER_NONE       0x00  /* The header type for each menu.          */
#define HEADER_MESSAGE    0x01
#define HEADER_FILE       0x02
#define HEADER_CHANGE     0x03
#define HEADER_CHAT       0x04

#define OFLAG_NODSP       0x0001 /* Don't display menu option on MENU, but */
                                 /* accept it as a command.                */
#define OFLAG_CTL         0x0002 /* Produce a .CTL file for this xtern cmd */
#define OFLAG_NOCLS       0x0004 /* Don't do a CLS for this display_menu   */
#define OFLAG_THEN        0x0008 /* Do only if last IF equation was true   */
#define OFLAG_ELSE        0x0010 /* Do only if last IF equation was FALSE  */
#define OFLAG_ULOCAL      0x0020 /* Only display for local users           */
#define OFLAG_UREMOTE     0x0040 /* Only display for remote useres         */
#define OFLAG_REREAD      0x0080 /* Re-read LASTUSER.BBS upon re-entry     */
#define OFLAG_STAY        0x0100 /* Don't perform menu clean-up operations */
#define OFLAG_RIP         0x0200 /* Available for RIP callers only         */
#define OFLAG_NORIP       0x0400 /* Available for non-RIP callers only     */

/* A few colours to make things easier on ME, the programmer.  These are   *
 * used all over the place, and made AVATAR commands a helluvalot easier   *
 * to deal with.  Unfortunately, if you don't have an ANSI compiler,       *
 * you're pretty well out of luck!                                         */

/* Standard colours: */

#define BLACK        "\x16\x01\x00"
#define BLUE         "\x16\x01\x01"
#define GREEN        "\x16\x01\x02"
#define CYAN         "\x16\x01\x03"
#define RED          "\x16\x01\x04"
#define MAGENTA      "\x16\x01\x05"
#define BROWN        "\x16\x01\x06"
#define GRAY         "\x16\x01\x07"
#define GREY         "\x16\x01\x07"
#define LBLACK       "\x16\x01\x08"
#define LBLUE        "\x16\x01\x09"
#define LGREEN       "\x16\x01\x0a"
#define LCYAN        "\x16\x01\x0b"
#define LRED         "\x16\x01\x0c"
#define LMAGENTA     "\x16\x01\x0d"
#define YELLOW       "\x16\x01\x0e"
#define WHITE        "\x16\x01\x0f"
#define ATTR         "\x16\x01%c"

/* Special AVATAR sequences: */

#define BLINK        "\x16\x02"
#define CLS          "\x0c"
#define CLEOL        "\x16\x07"
#define CLEOS        "\x16\x0f"     /* Not real AVATAR but a macro */
#define UP           "\x16\x03"
#define DOWN         "\x16\x04"
#define LEFT         "\x16\x05"
#define RIGHT        "\x16\x06"

/* Now some special colours that we use: */

#define BLKONWHIT    "\x16\x01\x70"
#define REDONWHIT    "\x16\x01\x74"
#define WHITONWHIT   "\x16\x01\x77"
#define MAGONBLUE    "\x16\x01\x1d"
#define WHITONBLUE   "\x16\x01\x1f"
#define YELONBLUE    "\x16\x01\x1e"
#define LREDONBLUE   "\x16\x01\x1c"
#define LMAGONBLUE   "\x16\x01\x1d"
#define LCYANONBLUE  "\x16\x01\x1b"
#define DKGRAYONWHIT "\x16\x01\x78"
#define CHATTR       "\x16\x01%c"

/* Priv level definitions */

#define priv_twit         (_privs[0].name)
#define priv_disgrace     (_privs[1].name)
#define priv_limited      (_privs[2].name)
#define priv_normal       (_privs[3].name)
#define priv_worthy       (_privs[4].name)
#define priv_privil       (_privs[5].name)
#define priv_favored      (_privs[6].name)
#define priv_extra        (_privs[7].name)
#define priv_clerk        (_privs[8].name)
#define priv_asstsysop    (_privs[9].name)
#define priv_sysop        (_privs[10].name)
#define priv_hidden       (_privs[11].name)


/***************************************************************************
                       Definitions for MaxEd and Bored
 ***************************************************************************/

/* #define UPDATE_POSITION*/     /* Whether or not we want to have a        *
                                 * display in the lower-right hand corner  *
                                 * of the screen, telling the current      *
                                 * cursor position.  (Default OFF, too     *
                                 * slow from remote!)                      */

#define SAVE               0x00 /* Return codes from the editor...         */
#define ABORT              0x01
#define NOTHING            0x02

#define LOCAL_EDIT         0x03 /* Returned ONLY by Editor() func, passed  *
                                 * back through Local_Editor().            */

#define HARD_CR           '\x0d'/* User pressed C/R here, insert into      *
                                 * the actual message.                     */
#define SOFT_CR ((byte)'\x8d')  /* Local wordwrap, don't bother storing    *
                                 * it in the messages.                     */

#define MODE_UPDATE           0 /* Update position after wordwrap          */
#define MODE_NOUPDATE         1 /* Don't update position after wordwrap    */
#define MODE_SCROLL           2 /* Update position, and scroll if          *
                                 * necessary!                              */

#define QUOTELINES            4 /* Number of lines to quote at a time       */

#ifdef __FARDATA__ /* If we have the memory to spare, use it! */
  #ifdef __MSDOS__
    #define MAX_LINES           250
  #else
    #define MAX_LINES           1000
  #endif
#else
  #define MAX_LINES           100 /* Maximum number of lines in file          */
#endif

#define MAX_LINELEN (LINELEN*2) /* Double the length of line which can      *
                                 * possibly be entered in editor.           */
#define LINELEN              80 /* Max. length of line to use               */
#define MAX_WRAPLEN          35 /* Length of largest word to wordwrap       */
#define UPDATEBUF_LEN (MAX_LINES+1)  /* # of update_table entries           */

#define SCROLL_LINES (usrlen-4) /* Number of lines to scroll down or up     */
                                /* for any given command which goes past    */
                                /* screen boundaries.                       */

#define SCROLL_CASUAL (usrlen >> 1)  /* Half of screen for casual scroll    */

/* Go left XX characters */
#define Cursor_LeftM(n) {Goto(current_line,current_col-(n)); cursor_y -= min(cursor_y-1,n);}

/*--------------------------------------------------------------------------*/
/* Access levels                                                            */
/*--------------------------------------------------------------------------*/

#if 0
   #define  TWIT        -2 /* 0xFFFE */
   #define  DISGRACE    0x0000
   #define  LIMITED     0x0001  /**/
   #define  NORMAL      0x0002
   #define  WORTHY      0x0003  /**/
   #define  PRIVIL      0x0004
   #define  FAVORED     0x0005  /**/
   #define  EXTRA       0x0006
   #define  CLERK       0x0007  /**/
   #define  ASSTSYSOP   0x0008
   #define  SYSOP       0x000A

#ifdef NUMBER_PRIVS
   #define  HIDDEN      0x7fff
#else
   #define  HIDDEN      0x000b
#endif
#endif


/***************************************************************************
                       Structure Definitions
 ***************************************************************************/

#ifndef _NDI_DEFINED
  #define _NDI_DEFINED

  struct _ndi
  {
    word node;          /* node number  */
    word net;           /* net number   */
  };
#endif

#include "colour.h"


/* Typedefs */

typedef word zstr;      /* Offset of string within area heap */

#ifndef _SYS_DEFINED
#define _SYS_DEFINED

struct _sys
{
    word ls_caller;        /* Used in SYSTEM.BBS only: number of callers    */
    sword priv;            /* Minimum privs required to get to the area     */
    byte msgpath[40];      /* Path to messages                              */
    byte bbspath[40];      /* Path to BBS/GBS files *or* the barricade file */
    byte hlppath[40];      /* Path to the user-help system                  */
    byte uppath[40];       /* Path to the file upload sub-directory         */
    byte filepath[40];     /* Path to the file download sub-directory       */
    word attrib;           /* Area attribute (see below)                    */
    byte lock;             /*                                               */
    byte filler;           /* Free space for another kludge...              */
    long quote_pos;        /* Position of next usable byte in Quote file    */
};

#endif

#include "uclass.h"       /* User class API */
#include "max_u.h"        /* USER.BBS structure */
#include "option.h"       /* Menu option enumeration */
#include "tagapi.h"
#include "callinfo.h"     /* Caller information log */

/* An individual menu option.  There are many of these contained in one    *
 * _menu file, following the _menu data header, optionally with some       *
 * NULL-terminated strings between each _opt structure, for the argument.  */

struct _opt
{
#ifdef __FLAT__
  /* force enum to be 16 bits */
  word type;
#else
  option type;  /* What this menu option does                              */
#endif

  zstr priv;    /* Priv level required to execute this command             */
  dword rsvd;   /* Bit-field locks for this particular menu option         */
  word flag;    /* See the OFLAG_xxx contants for more info.               */
  zstr name;    /* The menu option, as it appears to user                  */
  zstr keypoke; /* Auto-keypoke string                                     */
  zstr arg;     /* The argument for this menu option                       */
  byte areatype;/* If this particular option can only be used if the user  *
                 * is in a certain message-area type.                      */
  byte fill1;   /* Reserved by Maximus for future use                      */

  byte rsvd2[8]; /* Reserved for future uses */
};

#define DEFAULT_OPT_WIDTH 20

struct _menu
{
  word header,      /* What to display when the user enters menu, such as  *
                     * "The MESSAGE Section", "The CHG SETUP Section", etc */
       num_options, /* Total number of options (struct _opt's) in menu     */
       menu_length, /* Number of lines long the .?BS menu file is!         */
       opt_width;   /* Option width override                               */

  sword hot_colour; /* What colour to display if a user uses hotkeys to    *
                     * bypass a .?BS menu display, before displaying the   *
                     * key.  -1 == display nothing.                        */

  word title;       /* Length of the title string, not counting \0.        */
  word headfile;    /* Length of the header filename, not counting \0      */
  word dspfile;     /* Name of file to display for menu, instead of        *
                     * generating menu from .Mnu file.                     */
  word flag;        /* See MFLAG_XXX in MAX.H.                             */
};



/* Menu structure used INTERNALLY within Max itself */

typedef struct _amenu
{
  struct _menu m;
  struct _opt *opt;
  char *menuheap;
} AMENU, *PAMENU;


  #define MFLAG_MF_NOVICE   0x0001u /* MenuFile for these levels only */
  #define MFLAG_MF_REGULAR  0x0002u
  #define MFLAG_MF_EXPERT   0x0004u
/*  #define MFLAG_MF_HOTFLASH 0x0008u*/
  #define MFLAG_MF_RIP      0x0400u
  #define MFLAG_MF_ALL      (MFLAG_MF_NOVICE | MFLAG_MF_REGULAR | \
                             MFLAG_MF_EXPERT /*| MFLAG_MF_HOTFLASH*/)

  #define MFLAG_HF_NOVICE   0x0010u /* HeaderFile for these levels only */
  #define MFLAG_HF_REGULAR  0x0020u
  #define MFLAG_HF_EXPERT   0x0040u
  #define MFLAG_HF_RIP      0x0800u
/*  #define MFLAG_HF_HOTFLASH 0x0080u*/

  #define MFLAG_HF_ALL      (MFLAG_HF_NOVICE | MFLAG_HF_REGULAR | \
                             MFLAG_HF_EXPERT /*| MFLAG_HF_HOTFLASH*/)

  #define MFLAG_SILENT      0x0100u /* Silent menuheader option */
  #define MFLAG_RESET       0x0200u /* Reset term size on display */




#define STATS_VER           1   /* Version number of the BBSTATxx.BBS file */

struct _bbs_stats
{
  byte    version;      /* Version number of BBSTATxx.BBS file */
  dword   num_callers;
  dword   quote_pos;
  dword   msgs_written;
  time_t  online_date;
  dword   total_dl;
  dword   total_ul;
  sword   today_callers;
  union stamp_combo date;
  byte    lastuser[36];
};



/* Structure for entries in PROTOCOL.MAX */

/*
Protocol Lynx
        Description     LYNX - Fast transfer protocol
        Type            Batch
        Type            Opus

        LogFile         D:\Max\Dszlog*.Log
        ControlFile     D:\Max\Lynx.Ctl
        DownloadCmd     Lynx.Exe S /%p /%b /S /H @lynx.ctl
        UploadCmd       Lynx.Exe R /%p /%b /S /D /H #
        DownloadString  %s
        UploadString    
        DownloadKeyword x
        UploadKeyword   X
        FilenameWord    10
        DescriptWord    0
End Protocol
*/

struct _proto
{
  #define P_ISPROTO 0x01  /* This bit always set                            */
  #define P_BATCH   0x02  /* Can handle batch transfers                     */
  #define P_OPUS    0x04  /* Write an Opus-style .CTL file                  */
  #define P_ERL     0x08  /* Exit with xtern_erlvl                          */
  #define P_BI      0x10  /* Bidirectional transfer                         */

  word flag;

  char desc[40];
  char log[PATHLEN];
  char ctl[PATHLEN];
  char dlcmd[PATHLEN];
  char ulcmd[PATHLEN];
  char dlstr[40];
  char ulstr[40];
  char dlkey[40];
  char ulkey[40];
    
  word fnamword;
  word descword;
};



#include "prm.h"    /* MAX.PRM structure */

/*#include "areadat.h"*/ /* area.dat/idx/ndx structure */
#include "newarea.h"


/* Handle for holding current message/file area stack */

typedef struct _llpush
{
  union
  {
    MAH mah;
    FAH fah;
  } ah;

  BARINFO biOldPriv;
  dword last_msg;             /* Last message for area, for msg types only */

  struct _llpush *next;
} *LLPUSH;




/* ChatFind...() handle.  Only used internally. */

struct _cgs
{
  int ptr;
  char num_tid;
  char rsvd;

  int tids[255];
};





/* IPCxx.BBS header structure (see MAX_CHAT.C) */

struct _cstat
{
  word avail;

  byte username[36];
  byte status[80];

#ifndef MCP
  word msgs_waiting;

  dword next_msgofs;
  dword new_msgofs;
#endif
};



/* Data element in IPCxx.BBS file (see MAX_CHAT.C) */

struct _cdat
{
  word tid;
  word type;
  word len;

#ifdef MCP
  word dest_tid;
  dword rsvd1;
#else
  dword rsvd1;
  word  rsvd2;
#endif
};

/* Handle for saving CHAT status.  Mainly used internally, but also        *
 * in RESTARxx.BBS.                                                        */

struct _css
{
  word avail;
  byte status[80];
};


/* NOTE: The following structure is not completely stable.  Unless         *
 * rst.rst_ver is equal to RST_VER, then the ONLY items you're guaranteed  *
 * to be able to read are those marked with "*STABLE*".  Those items       *
 * are guaranteed to be stored at those offsets for all future versions    *
 * of Maximus, regardless of the version number.  However, everything      *
 * else is likely to change at a moment's notice.                          */

struct _restart
{
  byte rst_ver; /* Version number of restart data                 *STABLE* */

  sdword timeon;  /* Date user got on system, seconds since 1970  *STABLE* */
  sdword timeoff; /* Date user must be OFF system, secs since '70 *STABLE* */
  sdword restart_offset; /* Offset in .BBS file to restart at     *STABLE* */

  dword baud;             /* User's baud rate                   *STABLE*   */
  dword max_time;         /* Max time, as given by '-t' param   *STABLE*   */

  sword port;             /* Current COM port, 0=COM1, 1=COM2,  *STABLE*   */

  char written_echomail;  /* 0=user HASN'T written echomail     *STABLE*   */
  char written_matrix;    /* 0=user HASN'T entered matrix msg   *STABLE*   */
  char local;             /* 0=NOT local                        *STABLE*   */

  struct _stamp laston;   /* Time the user was last on system   *STABLE*   */
  
  word steady_baud;       /* Locked baud rate of user           *STABLE*   */

  sdword starttime;       /* Start time, for external protocol             */
  sdword timestart;       /* Time when MAX.EXE was started                 */
  sdword ultoday;         /* KB's the user has uploaded today              */

  union stamp_combo next_ludate;
  
  byte restart_type;      /* 1 if started via .BBS file, 0 otherwise       */
  char restart_name[PATHLEN]; /* Name of .BBS file to restart in           */
  char menupath[PATHLEN]; /* The current menu path                         */
  char firstname[36];     /* The user's first name                         */
  char last_onexit[PATHLEN]; /* The 'onexit' filename for current .BBS file*/
  char parm[PATHLEN];     /* Parms for external program, if any            */
  char fix_menupath[PATHLEN]; /* Readjust menu name                        */

  char lastmenu;          /* Last ^oR menu choice                          */
  char snoop;             /* If snoop is currently on or off               */

  char locked;            /* If priv is locked via keyboard 'L' command    */

  char keyboard;          /* If the Sysop's keyboard is turned on          */
  char protocol_letter;   /* Letter representing current protocol choice   */

  char chatreq;           /* If user wanted to chat with SysOp             */
  char mn_dirty;          /* If menuname buf is dirty                      */

  char barricade_ok;      /* If current barricade area is OK               */
  char no_zmodem;         /* If zmodem not allowed                         */

  sword usr_time;         /* User's usr.time value                         */
  sword rsvdxx1;          /* reserved for future use                       */
  sword lockpriv;         /* If rst.locked (above), then this is real priv */
  sword ctltype;          /* Control-file type (for xternal protocol)      */

  word current_baud;      /* User's baud rate, as a mask for mdm_baud() */

  /* Bit flags for ECHOTOSS.LOG */
  char rsvd[(1296/CHAR_BITS)+1];
  char rsvd2[1120]; /* old area data structure */
  struct _css css;
  
  char log_name[80];

  struct _usr origusr;    /* User record as it was at log-on */

#if 0
  word  fnames;
  char  filenames[MAXEXPAND][PATHLEN];
  dword filesizes[MAXEXPAND];
  word  fileflags[MAXEXPAND];
#endif
  
  char event_num;
  char rsvd3;
  
  sword last_protocol;
  long getoff;
  char returning[PATHLEN];
  long steady_baud_l;             /* Locked baud rate (as integer) */
  SCOMBO date_newfile;            /* User's last newfiles date */
  char menuname[PATHLEN];         /* Name of current menu */
};


/*#include "dmalloc.h"*/


/***************************************************************************
                      Global Variables and Constant Strings
 ***************************************************************************/

#ifdef MAX_INCL_VER
#include "max_vr.h"  /* Version information */
#endif

#include "max_p.h"   /* Protocol definitions */

#ifdef MAX_INCL_VARS
#include "max_v.h"   /* All of our external variables */
#endif

#include "m_for.h"

/*char *zalloc();*/
char *receive_file (char *fpath, char *fname, char protocol);


/* The rest of this junk is from OPUS.H: */

/*--------------------------------------------------------------------------*/
/*               The Opus Computer-Based Conversation System                */
/*       (c) Copyright 1986, Wynn Wagner III, All Rights Reserved           */
/*                                                                          */
/*                                                                          */
/*                   YOOHOO is a trademark of Wynn Wagner III               */
/*                                                                          */
/*                        YOOHOO-YOOHOO/2U2 & WaZOO are                     */
/*           Copyright 1987, Wynn Wagner III, All Rights Reserved           */
/*                                                                          */
/*                                                                          */
/*                                                                          */
/* This material is available for use by anybody with no strings and        */
/* no guarantees.                                                           */
/*                                                                          */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* NodeList.Sys                                                             */
/*                                                                          */
/*    NET > 0 and NODE > 0    Normal node                                   */
/*                                                                          */
/*    NET > 0 and NODE <= 0   Host node                                     */
/*                               Net host........node== 0                   */
/*                               Regional host...node==-1                   */
/*                               Country host....node==-2                   */
/*                                                                          */
/*    NET == -1      Nodelist.Sys revision                                  */
/*                                                                          */
/*    NET == -2      Nodelist statement                                     */
/*                                                                          */
/*--------------------------------------------------------------------------*/


#ifndef _NODE_DEFINED
  #define _NODE_DEFINED
    
  struct _node  /* NODELIST.SYS */
  {
     sword number;        /* node number                                   */
     sword net;           /* net number                                    */
     word  cost;          /* cost of a message to this node                */
     word  rate;          /* baud rate                                     */
     byte  name[20];      /* node name                                     */
     byte  phone[40];     /* phone number                                  */
     byte  city[40];      /* city and state                                */
  };
#endif

#ifndef _NEWNODE_DEFINED
  #define _NEWNODE_DEFINED

  struct _newnode /* NODELIST.DAT */
  {
     word NetNumber;
     word NodeNumber;
     word Cost;                                 /* cost to user for a
                                                 * message */
     byte SystemName[34];                       /* node name */
     byte PhoneNumber[40];                      /* phone number */
     byte MiscInfo[30];                         /* city and state */
     byte Password[8];                          /* WARNING: not necessarily
                                                 * null-terminated */
     word RealCost;                             /* phone company's charge */
     word HubNode;                              /* node # of this node's hub
                                                 * or 0 if none */
     byte BaudRate;                             /* baud rate divided by 300 */
     byte ModemType;                            /* RESERVED for modem type */
     word NodeFlags;                            /* set of flags (see below) */
     word NodeFiller;
  };
#endif

/*------------------------------------------------------------------------*/
/* Values for the `NodeFlags' field                                       */
/*------------------------------------------------------------------------*/
#define B_hub     0x0001
#define B_host    0x0002
#define B_region  0x0004
#define B_zone    0x0008
#define B_CM      0x0010
#define B_res1    0x0020
#define B_res2    0x0040
#define B_res3    0x0080
#define B_res4    0x0100
#define B_res5    0x0200
#define B_res6    0x0400
#define B_res7    0x0800
#define B_point   0x1000
#define B_res9    0x2000
#define B_resa    0x4000
#define B_resb    0x8000

/* Generic node structure, as used internally */

struct _maxnode
{
  word zone, net, node, point;
  word cost;
  char name[40];
  char phone[40];
  char city[40];
  word flag;
};


#define CMDLEN    60          /* size of the command typeahead buffer       */
#define CARRYLEN  20          /* LORE editor's carry buffer                 */


/*--------------------------------------------------------------------------*/
/* User help levels                                                         */
/*--------------------------------------------------------------------------*/
#define  EXPERT      (byte)0x02  /* grizzled veteran, no menus at all       */
#define  REGULAR     (byte)0x04  /* experienced user, brief menus           */
#define  NOVICE      (byte)0x06  /* Full menus plus additional hand-holding */
/*#define  HOTFLASH    (byte)0x20*/  /* Hotkey, full-screen interface           */


/*--------------------------------------------------------------------------*/
/* Message bundle header                                                    */
/*--------------------------------------------------------------------------*/

#define PKTVER       2        /* Used for `ver' (below)                     */

#define pFido         0x00
#define pRover        0x01
#define pSEAdog       0x02
#define pSlick        0x04
#define pOpus         0x05
#define pDutchie      0x06
#define pTabby        0x08
#define pWolf68k      0x0A
#define pQMM          0x0B
#define pFrontDoor    0x0C
#define pMailMan      0x11
#define pOOPS         0x12
#define pGSPoint      0x13
#define pBGMail       0x14
#define pBinkScan     0x19
#define pDBridge      0x1A
#define pBinkleyTerm  0x1B
#define pYankee       0x1C
#define pDaisy        0x1E
#define pPolarBear    0x1F
#define pTheBox       0x20
#define pSTARgate2    0x21
#define pTMail        0x22
#define pTCOMMail     0x23
#define pBananna      0x24
#define pRBBSMail     0x25
#define pAppleNetmail 0x26
#define pChameleon    0x27
#define pMajikBoard   0x28
#define pQMail        0x29
#define pPointClick   0x2A
#define pA3Bundler    0x2B
#define pFourDog      0x2C
#define pMSGPACK      0x2D
#define pAMAX         0x2E
#define pDomComSys    0x2F
#define pLesRobot     0x30
#define pRose         0x31
#define pParagon      0x32
#define pBinkST       0x33
#define pStarNet      0x34
#define pZzyZx        0x35
#define pQEcho        0x36
#define pBOOM         0x37
#define pPBBS         0x38
#define pTrapDoor     0x39
#define pWelmat       0x3A
#define pNetGate      0x3B
#define pOdie         0x3C
#define pQuickGimme   0x3D
#define pdbLink       0x3E
#define pTosScan      0x3F
#define pBeagle       0x40
#define pIgor         0x41
#define pTIMS         0x42
#define pIsis         0x43
#define pAirMail      0x44
#define pXRS          0x45
#define pJuliet       0x46
#define pJabberwocky  0x47
#define pXST          0x48
#define pMailStorm    0x49
#define pBIXMail      0x4A
#define pIMAIL        0x4B
#define pFTNGate      0x4C
#define pRealMail     0x4D
#define pLora         0x4E
#define pTDCS         0x4F
#define pInterMail    0x50
#define pRFD          0x51
#define pYuppie       0x52
#define pEMMA         0x53
#define pQBoxMail     0x54
#define pNumber4      0x55
#define pNumber5      0x56
#define pGSBBS        0x57
#define pMerlin       0x58
#define pTPCS         0x59
#define pRaid         0x5A
#define pOutpost      0x5B
#define pNizze        0x5C
#define pArmadillo    0x5D
#define prfmail       0x5E
#define pMsgtoss      0x5F
#define pInfoTex      0x60
#define pGEcho        0x61
#define pCDEhost      0x62
#define pPktize       0x63
#define pMax          0x7a

#ifndef _PKTHDR_DEFINED
#define _PKTHDR_DEFINED

/* Packet header v2 and 2+ */

struct _pkthdr
{
  sword orig_node;        /* originating node                           */
  sword dest_node;        /* destination node                           */
  word  year;             /* 0..99  when packet was created             */
  word  month;            /* 1..12  when packet was created             */
  word  day;              /* 1..31  when packet was created             */
  word  hour;             /* 0..23  when packet was created             */
  word  minute;           /* 0..59  when packet was created             */
  word  second;           /* 0..59  when packet was created             */
  word  baud;             /* destination's baud rate                    */
  word  ver;              /* packet version                             */
  sword orig_net;         /* originating network number                 */
  sword dest_net;         /* destination network number                 */
  byte  product;          /* product type                               */
  byte  rev_maj;          /* Major revision number (see 'rev_min')      */

  byte password[8];       /* ONLY 6 CHARS ARE SIGNIFICANT!! */

  word qm_orig_zone;      /* Orig. and dest. zones used by QM           */
  word qm_dest_zone;

  word aux_net;           /* Aux net, used for point-capable packers    */
  word cw_validation;     /* Copy of 'cw', below                        */
  byte product_hi;        /* High byte of 'product', above              */
  byte rev_min;           /* Minor rev number (see 'rev_maj', above)    */
  word cw;                /* Capability word. See cREVxxx, below.       */

  word orig_zone;         /* Zone and point info used by TosScan et al  */
  word dest_zone;
  word orig_point;
  word dest_point;

  long prod_data;         /* Product-specific data                      */
};
   
#endif

#define cREV2P   0x0001
#define cREV2    0x0000

/* Packet header v2.2 - Henderson format, esoteric and used by few systems */

struct _pkthdr22
{
  sword orig_node;        /* originating node                           */
  sword dest_node;        /* destination node                           */
  word  orig_point;       /* (year) */
  word  dest_point;       /* (month) */
  word  zero;             /* rsvd - must be zero (day) */
  word  hour;             /* 0..23  when packet was created             */
  word  minute;           /* 0..59  when packet was created             */
  word  second;           /* 0..59  when packet was created             */
  word  pktsubver;        /* Packet subversion (baud)                   */
  word  ver;              /* packet version                             */
  sword orig_net;         /* originating network number                 */
  sword dest_net;         /* destination network number                 */
  byte  product;          /* product type                               */
  byte  rev_maj;          /* Major revision number (see 'rev_min')      */

  byte password[8];

  word orig_zone;         /* Orig. and dest. zones used by QM           */
  word dest_zone;

  byte orig_domain[8];
  byte dest_domain[8];

  long prod_data;         /* Product-specific data                      */
};



struct _pktprefix
{
  word  ver;
  sword orig_node;
  sword dest_node;
  sword orig_net;
  sword dest_net;
  word  attr;
  word  cost;
};


/*
      struct star_dot_pkt=
      {
        struct _pkthdr;

        struct
        {
          struct _pktprefix;

          null-terminated (date)
          null-terminated (to)
          null-terminated (from)
          null-terminated (subj)
          null-terminated (message!)
        };

        char null='\x00';
      }
*/

#ifdef MAX_INCL_PROTO
#include "proto.h"
#endif

#if defined(ORACLE) && defined(__TURBOC__)

/* Turn off warnings for "Unused parameter", "Defined var never used",     *
 * and "assigned value that is never used", bcuz they make things messy    *
 * when compiling Oracle.                                                  */

#pragma warn -par
#pragma warn -aus
#pragma warn -use

#endif

#endif  /* __MAX_H_DEFINED */

