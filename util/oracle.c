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
static char rcs_id[]="$Id: oracle.c,v 1.1.1.1 2002/10/01 17:57:34 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Oracle, the Maximus external .BBS file viewer
*/

#define MAX_INITIALIZE
#define MAX_INCL_COMMS
#define MAX_INCL_VER
#define MAX_DEFINE_VERSION

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <dos.h>
#include <ctype.h>
#include "prog.h"
#include "mm.h"
#include "max_msg.h"

int unum=0;
char *filenames[30];

static void Proc_Arg(char *a);
static void Oracle_Help(void);
static void Oracle_More(void);

static char oracle_name[PATHLEN];
char waitforcaller=0;

char privch=-1;

void GetLocalSize()
{
  extern int loc_cols, loc_rows;

  loc_cols = usr.width;
  loc_rows = usr.len;
}

void Oracle_Parse_Args(char *ctlname,int argc,char *argv[])
{
  struct _fossil_info finfo;
  char *pszPrm;
  int x;
  
  char env_args[PATHLEN],
       *s;

  NW(ctlname);
  
  fprintf(stderr, "\nORACLE " VERSION "  External *.BBS Display File Viewer\n");
  fprintf(stderr, "Copyright 1990-" THIS_YEAR " by Lanius Corporation.  All rights reserved.\n\n");
  
  timeon=time(NULL);
  timeoff=timeon+(60L*60L);
  max_time=60;

  pszPrm = GetMaximus(argc, argv, 1);
  strcpy(prmname, pszPrm);

  strcpy(firstname,"Joe");

  Blank_User(&usr);

  // Do this 'portably'

  fossil_inf((struct _fossil_info far *)&finfo);
  usr.len = finfo.height;
  usr.width = finfo.width;


  strcpy(usr.name,  "Joe SysOp");
  strcpy(usrname, usr.name);

  strcpy(usr.city,  "Anytown, Canada");
  strcpy(usr.alias, "John Doe");
  strcpy(usr.phone, "(123) 456-7890");
  strcpy(usr.pwd,   "Mypass");

  usr.ludate.msg_st.date.yr=11;
  usr.ludate.msg_st.date.mo=8;
  usr.ludate.msg_st.date.da=15;

  usr.ludate.msg_st.time.hh=1;
  usr.ludate.msg_st.time.mm=23;
  usr.ludate.msg_st.time.ss=45/2;

  usr.priv=(word)-1;
  usr.times=20;
  usr.up=250;
  usr.down=1000;
  usr.downtoday=100;
  usr.credit=1500;
  usr.debit=345;
  usr.video=GRAPH_ANSI;
  bstats.num_callers=10000;
  strcpy(bstats.lastuser,"Joe User");
  task_num=1;


  mah.heap = "";
  mah.heap_size = 1;

  fah.heap = "";
  fah.heap_size = 1;

  
  *oracle_name='\0';
     
  if ((s=getenv("ORACLE")) != NULL)
  {
    strcpy(env_args, s);
    
    for (s=strtok(env_args," "); s && *s; s=strtok(NULL," "))
      Proc_Arg(s);
  }

  if (argc <= 1)
    Oracle_Help();
  
  for (x=1; x < argc; x++)
    Proc_Arg(argv[x]);
}

static void Proc_Arg(char *a)
{
  
  char *orig,
       *s;

  orig=a;
  
  if (*a=='-' || *a=='/')
  {
    a++;
    
    switch (tolower(*a))
    {
      case 'h':
        a++;
        
        switch (tolower(*a))
        {
          case 'n':
            usr.help=NOVICE;
            break;
            
          case 'r':
            usr.help=REGULAR;
            break;
            
          case 'e':
            usr.help=EXPERT;
            break;
            
          default:
            printf("Error!  Invalid help level specified: `%c'!\n",*a);
        }
        break;
        
      case 'i':
        usr.bits2 &= ~BITS2_IBMCHARS;
        break;

      case 'k':             /* Set keys */
        for (s=a+1; *s; s++)
        {
          *s=(char)toupper(*s);

          if (*s >= '1' && *s <= '8')
            usr.xkeys ^= (1L << (*s-'1'));
          else if (*s >= 'A' && *s <= 'X')
            usr.xkeys ^= (1L << ((*s-'A')+8));
        }
        break;
        
      case 'm':
        dsp_set=TRUE;
        
        switch(tolower(a[1]))
        {
          case 'b':
            displaymode=VIDEO_BIOS;
            break;
#ifdef TTYVIDEO
          case 'd':
            displaymode=VIDEO_DOS;
            break;
            
          case 'f':
            displaymode=VIDEO_FAST;
            break;
#endif
          case 'i':
            displaymode=VIDEO_IBM;
            break;

          default:
            printf("Error!  Invalid local video mode: `%c'!\n",a[1]);
        }
        break;

      case 'p':             /* handled by GetMaximus */
        break;

      case 'q':
        usr.bits |= BITS_HOTKEYS;
        break;
        
      case 's':
        a++;
        
        switch(tolower(*a))
        {
          case 'l':
            usr.len=(char)atoi(++a);
            break;
            
          case 'w':
            usr.width=(char)atoi(++a);
            break;
            
          default:
            printf("Error!  Invalid screen option specified: `%c'!\n",*a);
        }
        break;

      case 't':             /* TTY mode */
        usr.video=GRAPH_TTY;
        break;
        
      case 'v':             /* Set priv level */
        a++;
        if (isdigit(*a))
          usr.priv=(word)atol(a);
        else privch=(char)toupper(*a);
        break;

      default:
        printf("Error!  Invalid command-line argument: `%s'!\n\n",orig);
        /* fall-through */
        
      case '?':
        Oracle_Help();
    }
  }
  else if (*a=='?')
    Oracle_Help();
  else strcpy(oracle_name,a);
}



void Oracle(void)
{

  /* Convert priv level to a proper priv code */

  if (usr.priv==(word)-1)
  {
    if (privch==(char)-1)   /* Default 'sysop' level */
      usr.priv=(word)0xfffe;
    else
    {
      if ((usr.priv=ClassKeyLevel(privch))==(word)-1)
      {
        printf("Error!  Unknown priv level: `%c'!\n",privch);
        usr.priv=(word)0xfffe;
      }
    }
  }
  cls=ClassLevelIndex(usr.priv);

  if (! *oracle_name)
  {
    Printf("\nError!  No filename specified to display!\n");
    exit(1);
  }

  if (Display_File(0, NULL, oracle_name)==-1)
  {
    Printf("Error!  Can't open .BBS file '%s' for display.\n", oracle_name);
    vbuf_flush();
    exit(1);
  }

  vbuf_flush();
  VidClose();
}


static void Oracle_Help(void)
{
  printf("Oracle allows the SysOp to view a Maximus .BBS file off-line,\n");
  printf("without the need to start a local BBS session.\n\n");

  printf("Command-line format:\n\n");
  
  printf("    ORACLE <bbsfile> [[-x]...]\n\n");
  
  printf("`-x' can be any of the following switches:\n\n");
  
  printf("-hX  - Set the current help level to X, where X is either of `N' (novice), 'R'\n");
  printf("       (regular), or `E' (expert).\n");
  printf("-i   - Disables high-bit IBM characters.\n");
  printf("-kX  - Set the user's keys to X. ie. Use `-k149' to set keys 1, 4 and 9.\n");
  printf("-mX  - Set the local video mode to X, where X is one of 'B' (BIOS)\n");
  printf("       or 'I' (IBM).\n");
  printf("-pX  - Read the Maximus PRM info from X.  ie. -pMAX.PRM\n");
  printf("-q   - Enable quick hotkeys.\n");
  printf("-slX - Set the virtual screen length to X rows.\n");
  printf("-swX - Set the virtual screen width to X columns.\n");

  Oracle_More();

  printf("-t   - Forces TTY mode (no colour/cursor movement)\n");
  printf("-vX  - Set the current priv level to X, where X is either the numeric priv\n");
  printf("       level or the privilege key (eg. -vs for SysOp, etc.\n\n");
  
  printf("You may also alter the default for these switches, by setting the ORACLE\n");
  printf("environment variable.  For example, issue the command:\n\n");
  
  printf("    SET ORACLE=-pD:\\MAX\\MAX.PRM -vS\n\n");
  
  printf("to set the default .PRM name to D:\\MAX\\MAX.PRM, and to set the default priv\n");
  printf("level to SysOp.  (Note that environment variable settings can be overriden on\n");
  printf("the command line.)\n");
  
  exit(1);
}

static void Oracle_More(void)
{
  printf("[More]");
  fflush(stdout);
  kgetch();
  printf("\r      \r");
}

void Fix_MagnEt(void) {}
int Display_Options(char *name,XMSG *msg) {NW(name); NW(msg); return 0;}
void File_Locate(void) {*linebuf='\0';}
void Msg_Checkmail(char *menuname) { NW(menuname); }
int ChatFindIndividual(byte tid,char *username,char *status,word *avail) {NW(tid);NW(username);NW(status);NW(avail);return 0;}
int ChatSendMsg(byte tid,int type,int len,char *msg) {NW(tid);NW(type);NW(len);NW(msg);return 0; }
int Header_Message(int entry,int silent) {NW(entry);NW(silent);return 0;}
int Header_File(int entry,int silent) {NW(entry);NW(silent);return 0;}
int Header_Change(int entry,int silent) {NW(entry);NW(silent);return 0;}
int Header_None(int entry,int silent) {NW(entry);NW(silent);return 0;}
int Goodbye_Comment(void) {return 0;}
void Who_Is_On(void) {}
void ChatCleanUp(void) {}
void Check_For_Message(char *s1,char *s2) {NW(s1);NW(s2);}
void Parse_Local(int ch) {NW(ch);}
void Keyboard_Off(void) {}
int Highbit_Allowed(void) {return TRUE;}
char mdm_dump(char buffer) {NW(buffer); return 0;}
void PleaseRespond(void) {return;}
void Time5Left(void) {return;}
void TimeAlmostUp(void) {return;}
void TimeLimit(void) {return;}
void Lost_Carrier(void) {return;}
void Shell_To_Dos(void) {}
void Got_A_Null_Pointer(char *type, char *where) {NW(type);NW(where);}
int File_Get_Download_Names(int do_tag,int protocol) {NW(do_tag); NW(protocol); return 0;}
#if defined(OS_2) || defined(NT)
word carrier(void) { return 1; }
#endif

void cdecl logit(char *format,...)
{
  va_list var_args;
  char temp[PATHLEN];

  if (*format != '!')
    return;

  va_start(var_args, format);
  vsprintf(temp, format, var_args);
  va_end(var_args);

  Printf("\r%s\n", temp);
}


#if defined(OS_2) || defined(NT)

  void mdm_dtr(char dtr) { NW(dtr); }
  int mdm_ctrlc(char mask) { NW(mask); return 0; }
  void _fast medfini(void) {}
#endif

int Outside(char *leaving,char *returning,int method,char *parm,
                   int display_slogan,int ctltype,char restart_type,
                   char *restart_name)
{
  NW(display_slogan); NW(leaving); NW(ctltype); NW(returning);
  NW(restart_name); NW(restart_type); NW(method);

  Printf(WHITE ">>> External program: '%s' <<<  ",parm);
  Press_ENTER();
  return 0;
}


int Mex(char *file)
{
  Printf(WHITE ">>> MEX program: '%s' <<<   ", file);
  return 0;
}


dword MAPIENTRY SquishHash(byte OS2FAR *f) { NW(f); return 0; }

char * Show_Pwd(char *pwd,char *ret,char echo)
{
  (void)ret;
  (void)echo;

  return pwd;
}


#if defined(OS_2) || defined(NT)
int mdm_avail()
{
  return 0;
}
#endif

