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
static char rcs_id[]="$Id: max_main.c,v 1.3 2003/06/13 03:18:59 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Main menu functions and commands
*/

#define MAX_LANG_max_main
#define MAX_INCL_COMMS
#define INCL_NOPM
#define INCL_DOS

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <dos.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#ifdef UNIX
# include <sys/utsname.h>
#endif

#ifdef OS_2
#include <os2.h>
#endif

#include "prog.h"
#include "ffind.h"
#include "alc.h"
#include "mm.h"
#include "events.h"
#include "max_menu.h"
#include "max_area.h"
#include "max_main.h"
#include "userapi.h"

static void near Yell(void);
static void near Max_Version(void);
static void near Compilation_Data(void);
static void near Goodbye(void);


int Exec_Main(int type, char **result)
{
  *result=NULL;
  
  switch(type)
  {
/*  case statistics:    Statistics();         break;*/
    case o_yell:        Yell();               break;
    case userlist:      return (UserList());
    case o_version:     Max_Version();        break;
    case leave_comment: Goodbye_Comment();    break;
#ifdef CLIMAX
    case climax:        CLIMax();             break;
#endif

    case user_editor:   
      User_Edit(NULL);
      Putc('\n');
      break;

    case goodbye:
      Goodbye();
      break;

    default:
      logit(bad_menu_opt, type);
      return 0;
  }

  return 0;
}


/* Get a random number from 0 .. num_tunes-1 specifying the tune to play. */

static void near get_random_tune(char *file, char *tune)
{
  char *line;
  unsigned num_tune=0;
  FILE *fp;

  /* Open the tune file */

  if ((fp=fopen(file, fopen_read))==NULL)
  {
    char temp[PATHLEN];

    strcpy(temp, file);
    strcat(temp, ".bbs");

    if ((fp=fopen(temp, fopen_read))==NULL)
      return;
  }

  /* Count the number of lines starting with a '*' */

  if ((line=malloc(PATHLEN)) != NULL)
    while (fgets(line, PATHLEN, fp))
      if (*line=='*')
        num_tune++;

  fclose(fp);

  /* Select a tune by its number */

  sprintf(tune, "*%d", num_tune ? (rand() % num_tune) : 0);
}

      


/* This beeps the local speaker (or plays a tune) */

static int near Yell_Beep(struct _event *e)
{
  if (*e->tune && *PRM(tune_file) && prm.noise_ok && (usr.bits & BITS_NERD)==0)
  {
    char tune[32];

    strcpy(tune, e->tune);

    if (eqstri(tune, "random"))
      get_random_tune(PRM(tune_file), tune);

    play_tune(PRM(tune_file), tune, yellchk,
              multitasker==MULTITASKER_desqview);
  }
  else
  {
    if (loc_kbhit() || mdm_avail() || !chatreq)
      return -1;

    Mdm_putc('\x07');
    
    Delay(85);
    
    if (prm.noise_ok && (usr.bits & BITS_NERD)==0)
      Local_Beep(1);
  }
    
  return 0;
}



static void near Yell(void)
{
  struct _event ye;
  char temp[PATHLEN];
  unsigned gotit;
  unsigned yells;

  chatreq=TRUE;
  ci_paged();

  logit(log_user_yelling, usr.name);


  /* Randomize the random number genertor */

  srand(time(NULL));

  /* If we can find a yell event encompassing the current time... */

  gotit=GetEvent(EFLAG_YELL, 0, &ye, TRUE);
  

  /* Don't let user yell more than 'num_yells' times, without the           *
   * sysop intervening with <Alt-C>.                                        */

  if (gotit && ye.data2 && num_yells >= ye.data2)
  {
    ci_paged_ah();
    gotit=FALSE;
  }


  if (gotit)
  {
    void *sbinfo; /* soundblaster information */

    num_yells++;

    Printf(user_yelling, usr.name);
    vbuf_flush();

    yells=ye.data1;

    sbinfo=YellSblastInit();
    
    while (yells--)
    {
      Yell_Beep(&ye);

      if (Mdm_keyp())
      {
        Mdm_getcw();
        break;
      }

      if (loc_kbhit() || !chatreq)
        break;
    }

    YellSblastTerm(sbinfo);

    /* If Sysop didn't press <sp>, and didn't press Alt-C (which sets    *
     * chatreq to FALSE)                                                 */

    if (!loc_kbhit() && chatreq)
    {
      sprintf(temp, "%sNOTIN.?BS", PRM(misc_path));

      if (fexist(temp))
      {
        *strrchr(temp,'.')='\0';
        Display_File(0, NULL, temp);
      }
      else
      {
        Puts(il_nest_pas_ici);
        Press_ENTER();
      }
    }
  }
  else /* sysop is not available */
  {
    sprintf(temp, "%sYELL.?BS", PRM(misc_path));

    if (fexist(temp))
    {
      *strrchr(temp, '.')='\0';
      Display_File(0, NULL, temp);
    }
    else
    {
      Puts(yell_is_off);
      Press_ENTER();
    }
  }
}



static void near Max_Version(void)
{
#if !defined(UNIX) && !defined(NT)
  byte id, subid;
#endif

  Puts(CLS);

  Printf(slogan, LMAGENTA, version, test);
  Puts(copyright);
  Puts(dev_info);

#ifdef KEY
  Printf(LRED "System: %s\n"
              " SysOp: %s\n\n",
         gkey_info+strlen(gkey_info)+1,
         gkey_info);
#endif

  /* Now tell user when we were compiled */

  Compilation_Data();

#ifdef HCC
  Printf(YELLOW "Licensed for use by members of the Dutch HCC.\n\n");
#endif

#if !defined(NT) && !defined(UNIX)
  #if defined(OS_2)
    #ifdef __FLAT__
      DosDevConfig(&id, DEVINFO_MODEL);
      DosDevConfig(&subid, DEVINFO_SUBMODEL);
    #else
      DosDevConfig(&id, DEVINFO_MODEL, 0);
      DosDevConfig(&subid, DEVINFO_SUBMODEL, 0);
    #endif
  #elif defined(__MSDOS__)
    id=*(char far *)MK_FP(0xffffu, 0x000e);
    subid=0;
  #else
    #error Unknown OS
  #endif


    Puts(computer_type);

    switch (id)
    {
      case 0xff:
        Puts(class_pc);
        break;

      case 0xfe:
        Puts(class_xt);
        break;

      case 0xfd:
        Puts(class_pcjr);
        break;

      case 0xfc:
        switch(subid)
        {
          case 0x02:
            Puts(class_ps2_xt);
            break;

          case 0x04:
            Printf(class_ps2_mod, 50);
            break;

          case 0x05:
            Printf(class_ps2_mod, 60);
            break;

          case 0x01: /* many clones have AT/339 - fall thru to "Generic AT" */
          default:
            Puts(class_at);
            break;
        }
        break;

      case 0xfa:
        Printf(class_ps2_mod, 30);
        break;

      case 0xf9:
        Puts(class_pc_conv);
        break;

      case 0xf8:
        switch (subid)
        {
          case 0x2e: case 0x14: case 0x16: case 0x2a: case 0x2c: case 0x58:
          case 0x5a: case 0x5c: case 0x40: case 0x28:
            Printf(class_ps2_mod, 95);
            subid=0;
            break;

          case 0x2f: case 0x11: case 0x13: case 0x2b: case 0x2d: case 0x57:
          case 0x59: case 0x5b: case 0x3f: case 0x29:
            Printf(class_ps2_mod, 90);
            subid=0;
            break;

          default:
            Printf(class_ps2_mod, 80);
        }


        break;

      case 0x9a:
        Puts(class_compaq_p);
        break;

      case 0x2d:
        Puts(class_compaq);
        break;

      default:
        Printf(class_generic, (int)id);
        break;
    }

    #ifdef OS_2
    if (subid)
      Printf(minor_revision, subid+1);
    #endif
#endif

  Puts(oper_sys);

#if defined(NT)
  {
#if 1 /* NT 3.1 or below */
    int ver=GetVersion();

    Printf("Windows%s Version %u.%02u\n",
           (ver & 0xff) >= 4 ? "95" : " NT",
           ver & 0xff, (ver >> 8) & 0xff);
#else
    OSVERSIONINFO vi;
    int ver;

    GetVersionExA(&vi);

    Printf("Windows%s Version %u.%02u (build %d)\n",
           vi.dwPlatformId==VER_PLATFORM_WIN32_WINDOWS ? "95" : " NT",
           vi.dwMajorVersion,
           vi.dwMinorVersion,
           vi.dwBuildNumber);
#endif
  }
#elif defined(OS_2)
  {
    int osmajor = _osmajor;
    int osminor = _osminor;

    if (_osmajor==20 && _osminor >= 30)
    {
      osmajor = 30;
      osminor = (_osminor - 30) * 10;
    }

    Printf(os2_ver, osmajor/10, osminor);
  }
#elif defined(__MSDOS__)

  {
    struct _fossil_info finfo;

    if (_osmajor > 9)
      Printf(os2_dosbox,_osmajor/10,_osminor);
    else Printf(dos_ver,_osmajor,_osminor);

    fossil_inf(&finfo);
    Printf(fossil_ver,finfo.id);
  }

#elif defined(UNIX)
  {
    struct utsname name;

    if (uname(&name) != -1) /* Don't change this */
      Printf("%s %s, running on %s hardware (%s)\n", name.sysname, name.release, name.machine, name.nodename);
    else
      Printf("Unknown UNIX-type platform\n");
  }
#else
  #error Unknown OS
#endif

  {
    char temp[40];
    Printf(heap_mem, commaize((long)coreleft(), temp));
  }

  Puts(WHITE);
  Press_ENTER();
}



static void near Compilation_Data(void)
{
#if defined(__WATCOMC__)
  #ifdef __FLAT__
    #define COMPILER "WATCOM C/32"
  #else
    #define COMPILER "WATCOM C/16"
  #endif
#elif defined(__TURBOC__)
  #define COMPILER "Turbo C"
#elif defined(_QC)
  #define COMPILER "Microsoft Quick C"
#elif defined(__MSC__)
  #define COMPILER "Microsoft C"
#elif defined(__ZTC__)
  #define COMPILER "Zortech C"
#elif defined(__GNUC__)
  #define COMPILER "GNU C"
#else
#error Unknown compiler type!
#endif


#if defined(_MSC_VER)
  #if _MSC_VER > 0
    #define COMPSTR       " v%d.%d"
    #define COMPVERMAJ    (_MSC_VER/100)
    #define COMPVERMIN    (_MSC_VER % 100)
  #endif
#elif defined(__WATCOMC__)
  #define COMPSTR       " v%d.%d"
  #define COMPVERMAJ    (__WATCOMC__/100)
  #define COMPVERMIN    (__WATCOMC__%100)
#elif defined(__TURBOC__)
  #define COMPSTR       " v%d.%d"
  #define COMPVERMAJ    (__TURBOC__/0x100)
  #define COMPVERMIN    (__TURBOC__ % 0x100)
#else
  #define COMPSTR
  #define COMPVERMAJ    0
  #define COMPVERMIN    0
#endif

  Printf(compiled_on,
         comp_date, comp_time);

  Printf(COMPILER COMPSTR ")\n", COMPVERMAJ, COMPVERMIN);
}



int UserList(void)
{
  HUF huf;
  HUFF huff;
  HUFFS huffs;

  char string[BUFLEN];
  char search[BUFLEN];

  int found, ret;
  int searchname, searchalias;
  int go;

  char nonstop;


  ret=1;  /* Not found */

  WhiteN();

  if (! *linebuf)
    Puts(ul_1);

  InputGetsLL(search, BUFLEN, ul_2);

  if (! *linebuf)
    Putc('\n');

  if ((huf=UserFileOpen(PRM(user_file), 0))==NULL)
  {
    cant_open(PRM(user_file));
    return -1;
  }

  display_line=display_col=1;
  nonstop=FALSE;

  found=FALSE;

  /* If the user pressed enter, we want the whole list, but use
   * UserFileFindOpen so that it can be displayed in order.  But
   * if the user entered a search param, use FindSeq to find
   * it quickly.
   *
   */

  go=FALSE;

  if (*search)
  {
    if ((huffs=UserFileFindSeqOpen(huf)) != NULL)
      go=TRUE;
  }
  else
  {
    if ((huff=UserFileFindOpen(huf, NULL, NULL)) != NULL)
      go=TRUE;
  }

  if (go)
  {
    int showallusers=acsflag(CFLAGA_UHIDDEN);

    if (showallusers)
      searchname=searchalias=TRUE;
    else
    {
      searchname=(prm.flags & FLAG_alias)==0;
      searchalias=(prm.flags & FLAG_alias);
    }

    do
    {
      struct _usr user;

      if (*search)
        user=huffs->usr;
      else
        user=huff->usr;

      if (halt())
      {
        ret=-1;
        break;
      }

      /* If it's us, copy any extra info in our current user record */

      if (eqstri(user.name, usr.name))
        user=usr;

      if (! *search ||
          (searchname && stristr(user.name, search)) ||
          (*user.alias && searchalias && stristr(user.alias, search)))
      {
        int hiddenuser=(user.bits & BITS_NOULIST) || (ClassGetInfo(ClassLevelIndex(user.priv),CIT_ACCESSFLAGS) & CFLAGA_HIDDEN);

        if ((showallusers || !hiddenuser) && !(user.delflag & UFLAG_DEL))
        {
          found=TRUE;

          ret=0;

          if (MoreYnBreak(&nonstop, CYAN))
          {
            ret=-1;
            break;
          }

          Printf(ul_format,
                 ((prm.flags & FLAG_alias) && *user.alias)
                    ? user.alias : user.name,
                 sc_time(&user.ludate,string),
                 user.city);
        }
      }

      if (*search)
        go=UserFileFindSeqNext(huffs);
      else
        go=UserFileFindNext(huff, NULL, NULL);
    }
    while (go);

    if (*search)
      UserFileFindSeqClose(huffs);
    else
      UserFileFindClose(huff);
  }

  if (!found)
    Printf(ul_notfound, search);

  UserFileClose(huf);
  return ret;
}


#if 0 /* obsolete */

void Statistics(void)
{
  char temp[PATHLEN];
  union stamp_combo stamp;

  Timestamp_Format(PRM(dateformat),Get_Dos_Date(&stamp),temp);
  Printf(ustat1,temp);

  Timestamp_Format(PRM(timeformat),&stamp,temp);
  Puts(temp);
  Putc('\n');
  Putc('\n');

  Puts(ustat2);
  Printf(ustat3,timeonline());
  Printf(ustat4,timeleft());

  if (usr.time)
    Printf(ustat5,usr.time);

  Printf(ustat6,usr.times);

  Puts(ustat7);
  Printf(ustat8,usr.up);
  Printf(ustat9,usr.down);

  if (usr.downtoday)
    Printf(ustat10,usr.downtoday);
  
  Printf(ustat10_5, prm.cls[cls].max_dl - usr.downtoday);

  if (usr.credit || usr.debit)
  {
    Puts(ustat11);
    Printf(ustat12,usr.credit);
    Printf(ustat13,usr.debit);
  }
  
  if (usr.xp_flag & (XFLAG_EXPDATE | XFLAG_EXPMINS))
  {
    Puts(ustat14);

    if (usr.xp_flag & XFLAG_EXPMINS)
      Printf(ustat15, (long)usr.xp_mins-(long)timeonline());

    if (usr.xp_flag & XFLAG_EXPDATE)
      Printf(ustat16, FileDateFormat(&usr.xp_date,temp));
  }

  Putc('\n');
}

#endif


static void near Goodbye(void)
{
  BARINFO bi;
  MAH ma;
  char *where;

  char string[PATHLEN];

  memset(&ma, 0, sizeof ma);

  if (!local)
  {
    WhiteN();

    sprintf(string, "%swhy_hu", PRM(misc_path));

    if (GetListAnswer(CYnq,string,useyforyes,0,disconnect)==NO)
      return;

    WhiteN();

    where=*PRM(cmtarea) ? PRM(cmtarea) : zero;
    
    if (ReadMsgArea(ham, where, &ma) &&
        ValidMsgArea(NULL, &ma, VA_VAL | VA_OVRPRIV, &bi) &&
        PushMsgArea(where, &bi))
    {
      sprintf(string, "%swhy_fb", PRM(misc_path));

      if (GetListAnswer(yCNq, string, useyforyes, 0, leave_msg, PRM(sysop))==YES)
        Goodbye_Comment();

      PopMsgArea();
    }
    
    do_caller_vanished=FALSE;

    Display_File(0, NULL, PRM(byebye));

    Printf(bibi, usrname);
    caller_online=FALSE;        /* To disable the "Caller dropped carrier" */
    Delay(200);                 /* message.                                */
  }

  mdm_hangup();
}


#ifdef OS_2
  /* Sound the given note for the specified duration on a 'blaster. */

  static void _fast sbnoise(int freq, int duration)
  {
    int f=freq;
    int oct=4;

    if (freq > 1)
    {
      while (f >= 524)
        f /= 2, oct++;

      while (f < 277)
        f *= 2, oct--;

      f=(int)((long)(f-277)*323L/246L+0x16bL);

      (*spsb->SblastFMOutAll)(0xb0, 0);
      (*spsb->SblastFMNoteAll)(oct, f);
    }

    /* Sleep until the tone ends */

    DosSleep(duration);
  }
#endif


static void * near YellSblastInit(void)
{
#ifndef OS_2
  return 0;
#else
  struct _sbinfo *psb;
  HMODULE hmod;
  ULONG rc;

  if (getenv("OS2BLASTER")==NULL)
    return 0;

  /* Try to load the soundblaster .DLL */

#ifdef __FLAT__
  if (DosLoadModule(NULL, 0, "MAXBLAS2", &hmod) != 0)
#else
  if (DosLoadModule(NULL, 0, "MAXBLAST", &hmod) != 0)
#endif
    return NULL;

  /* Allocate memory for the struct */

  if ((psb=malloc(sizeof(struct _sbinfo)))==NULL)
    return NULL;

  psb->hmod=hmod;

#ifdef __FLAT__
  if ((rc=DosQueryProcAddr(psb->hmod, 0L, "SBLASTINIT32", (PFN *)(void *)&psb->SblastInit)) != 0 ||
      (rc=DosQueryProcAddr(psb->hmod, 0L, "SBLASTTERM32", (PFN *)(void *)&psb->SblastTerm)) != 0 ||
      (rc=DosQueryProcAddr(psb->hmod, 0L, "SBLASTFMOUTALL32", (PFN *)(void *)&psb->SblastFMOutAll)) != 0 ||
      (rc=DosQueryProcAddr(psb->hmod, 0L, "SBLASTFMNOTEALL32", (PFN *)(void *)&psb->SblastFMNoteAll)) != 0)
#else
  if ((rc=DosGetProcAddr(psb->hmod, "SBLASTINIT", (PFN far *)(void far *)&psb->SblastInit)) != 0 ||
      (rc=DosGetProcAddr(psb->hmod, "SBLASTTERM", (PFN far *)(void far *)&psb->SblastTerm)) != 0 ||
      (rc=DosGetProcAddr(psb->hmod, "SBLASTFMOUTALL", (PFN far *)(void far *)&psb->SblastFMOutAll)) != 0 ||
      (rc=DosGetProcAddr(psb->hmod, "SBLASTFMNOTEALL", (PFN far *)(void far *)&psb->SblastFMNoteAll)) != 0)
#endif
  {
    logit("!QueryProcAddr returned %d", rc);
    psb->SblastInit=0;
    psb->SblastTerm=0;
    psb->SblastFMOutAll=0;
    psb->SblastFMNoteAll=0;
  }

  psb->use_sb=FALSE;

#ifdef __FLAT__
  #define dothunk32to16(v) *(long *)&v = thunk32to16(*(long *)&v)

  dothunk32to16(psb->SblastInit);
  dothunk32to16(psb->SblastTerm);
  dothunk32to16(psb->SblastFMOutAll);
  dothunk32to16(psb->SblastFMNoteAll);
#endif

  if (psb->SblastInit && (*psb->SblastInit)(0x220, 7)==0)
  {
    psb->use_sb=TRUE;

    /* Initialize the SB registers */

    (*psb->SblastFMOutAll)(0x40, 0x10);
    (*psb->SblastFMOutAll)(0x60, 0xf0);
    (*psb->SblastFMOutAll)(0x80, 0x77);
    (*psb->SblastFMOutAll)(0xa0, 0x98);
    (*psb->SblastFMOutAll)(0x23, 0x01);
    (*psb->SblastFMOutAll)(0x43, 0x00);
    (*psb->SblastFMOutAll)(0x20, 0x42);
    (*psb->SblastFMOutAll)(0x60, 0xe6);
    (*psb->SblastFMOutAll)(0x63, 0xe5);
    (*psb->SblastFMOutAll)(0xe0, 0x02);
    (*psb->SblastFMOutAll)(0x80, 0xbc);
    (*psb->SblastFMOutAll)(0x83, 0x77);

    /* Bump our priority up to 'foreground' */

#ifndef __FLAT__
    DosGetPrty(PRTYS_PROCESS, &psb->usOldPrty, 0);
    DosSetPrty(PRTYS_PROCESS, PRTYC_FOREGROUNDSERVER, 0, 0);
#endif

    psb->pfnOldNoise=noisefunc;
    noisefunc=sbnoise;

    /* Set static variable to the same address */

    spsb=psb;
  }

  return psb;
#endif
}

static void near YellSblastTerm(void *v)
{
#ifndef OS_2
  (void)v;
#else
  struct _sbinfo *psb=(struct _sbinfo *)v;

  if (!psb)
    return;

  if (psb->use_sb)
  {
    /* Turn off the sound and exit */

    (*psb->SblastFMOutAll)(0xb0, 0);
    (*psb->SblastTerm)();

    /* Set us back to our old priority */

#ifndef __FLAT__
    DosSetPrty(PRTYS_PROCESS, psb->usOldPrty, 0, 0);
#endif

    /* Restore the noise function */

    noisefunc=psb->pfnOldNoise;
  }


  DosFreeModule(psb->hmod);
  free(psb);
  spsb=NULL;
#endif
}

/* Needs to stay in root because it's called by tune() */

int _stdc yellchk(void)
{
  static long lasttime=-1L;
  long curtime;
  
  curtime=time(NULL);
  
  if (curtime != lasttime)
  {
    Mdm_putc('\x07');
    
    lasttime=curtime;
  }
    
  if (loc_kbhit() || mdm_avail() || !chatreq)
    return -1;
  else return 0;
}



