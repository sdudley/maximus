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

/* what about the new nodelist 7/fd types in max.ctl? */
/* installcopy200one does not rename the original file */
/* what about replacing latent copies of maxcomm.dll et al on
 * the libpath?
 */
/* do we need to search the entire hard drive to replace
 * old DLLs?
 */
//#define COMMERCIAL /* for disk-based installation */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>

#ifdef OS_2
#define INCL_DOS
#include "pos2.h"
#endif

#include "prog.h"
#include "dr.h"
#include "tui.h"
#include "wdearc.h"
#include "install.h"
#include "bfile.h"

#define MAX_DOSOS2_SIZE   5000000     /* Space required to do installs */
#define MAX_DOSOS2_SIZE_S "6.0"

#define MAX_DOS_SIZE      3000000
#define MAX_DOS_SIZE_S    "4.0"

#define MAX_OS2_SIZE      3000000
#define MAX_OS2_SIZE_S    "4.0"

struct _tag
{
  #define TID_1     251
  #define TID_2     95
  #define TID_3     93
  #define TID_4     16

  unsigned char id1;
  unsigned char id2;
  unsigned char id3;
  unsigned char id4;
  char str[24];
};

static word dupe_file(char *full, char *path, char *name);
static word near do_file(char *name);

FILE *ilog;                           /* Installation log file */

char szDirBase[PATHLEN];              /* Main Max directory */
char szCtlName[PATHLEN];              /* System control file */
static char szDirMex[PATHLEN];        /* MEX directory */
static char szDirRip[PATHLEN];        /* RIP directory */
static char szDirMisc[PATHLEN];       /* Misc dir */
static char szDirHlp[PATHLEN];        /* Help directory */
static char szDirLang[PATHLEN];       /* Language file directory */
static int iVerType;                  /* Target operating system */

static int iCurDisk;                  /* Current drive number (0=A:, 1=B:) */
static char cCurDisk;                 /* Current drive as a letter */
static int fDoUpgrade;                /* Do an upgrade or a new install? */
static char szDirDll[PATHLEN];        /* Directory for system .DLLs */
unsigned long ulBootDrive;            /* OS/2 boot drive (1=A, 2=B, etc) */
char cBootDrive;                      /* Character version of above */
static int f300Upgrade = FALSE;       /* Upgrading from 3.0 or above? */

int fExtractFiles;                    /* Flags for the upgrade install */
int fCvtCtl;
int fCvtEvt;
int fCvtTag;
int fCvtConfig;
int fRecompile;


/* Names of the files to be extracted during installation */

static char sysr_fiz[]="sysr.fiz";
static char sysp_fiz[]="sysp.fiz";
static char ctl_fiz[]="ctl.fiz";
static char lang_fiz[]="lang.fiz";
static char misc_fiz[]="misc.fiz";
static char hlp_fiz[]="hlp.fiz";
static char mex_fiz[]="mex.fiz";
static char rip_fiz[]="rip.fiz";

#ifdef COMMERCIAL
static char doc_fiz[]="doc.fiz";
#endif

static char szCfgName[PATHLEN];
static char szCfgSysop[PATHLEN];
static word iCfgPort;
static word iCfgSpeed;
static char szFidoAddr[PATHLEN];
static char szFidoPath[PATHLEN];
static word iFidoNodeVer;
static char szCfgPorts[20];       /* com port number */
static char iFidoDo5;
static char iFidoDo6;
static char iFidoDo7;
static char iFidoDoFD;
static char szMiscRep[PATHLEN];       /* Relative paths used when replacing  */
static char szHlpRep[PATHLEN];        /* strings in control files.           */
static char szLangRep[PATHLEN];

static struct _fiztab
{
  #define FIZ_ALL   0
  #define FIZ_DOS   1
  #define FIZ_OS2   2

  char *szName;
  char *szDir;
  word (*dupe_file)(char *, char *, char *);
  word (near *doit)(char *fn);
  int iOS;
  int fExtract;
} fizzes[]=
{
  #define FIZ_SYSR 0
  {sysr_fiz,      szDirBase,    dupe_file,    do_file,  FIZ_DOS, TRUE},
  #define FIZ_SYSP 1
  {sysp_fiz,      szDirBase,    dupe_file,    do_file,  FIZ_OS2, TRUE},
  #define FIZ_CTL 2
  {ctl_fiz,       szDirBase,    dupe_file,    do_file,  FIZ_ALL, TRUE},
  #define FIZ_LANG 3
  {lang_fiz,      szDirLang,    dupe_file,    do_file,  FIZ_ALL, TRUE},
  #define FIZ_MISC 4
  {misc_fiz,      szDirMisc,    dupe_file,    do_file,  FIZ_ALL, TRUE},
  #define FIZ_HLP 5
  {hlp_fiz,       szDirHlp,     dupe_file,    do_file,  FIZ_ALL, TRUE},
  #define FIZ_MEX 6
  {mex_fiz,       szDirMex,     dupe_file,    do_file,  FIZ_ALL, TRUE},
  #define FIZ_RIP 7
  {rip_fiz,       szDirRip,     dupe_file,    do_file,  FIZ_ALL, TRUE},
#ifdef COMMERCIAL
  #define FIZ_DOC 8
  {doc_fiz,       szDirBase,    dupe_file,    do_file,  FIZ_ALL, TRUE},
#endif
  {0,             0,            0,            0,        0}
};

#ifdef OS_2
static struct _dllinfo
{
  char *szDllName;
  char *szModName;
} maxdll[] =
{
  {"maxbt32.dll",   "maxbt32"},
  {"maxdb32.dll",   "maxdb32"},
  {"mcp32.dll",     "mcp32"},
  {"msgapi32.dll",  "msgapi32"},
  {"mexvm32.dll",   "mexvm32"},
  {"maxblast.dll",  "maxblast"},
  {"maxblas2.dll",  "maxblas2"},
  {"maxcomm.dll",   "maxcomm"},
  {"maxuapi.dll",   "maxuapi"},
  {NULL,            NULL}
};
#endif


static struct _updtab upd_new_max[]=
{
  {0,                "name",    NULL,      NULL,2, szCfgName,       0,NULL},
  {0,                "sysop",   NULL,      NULL,2, szCfgSysop,      0,NULL},
  {0,                "path",    "system",  NULL,3, szDirBase,       0,NULL},
  {FLAG_UCOM|FLAG_IC,"output",  "com1",    NULL,2, szCfgPorts,      0,NULL},
  {FLAG_W,           "baud",    "maximum", NULL,3, (char *)&iCfgSpeed,0,NULL},
  {0,                "address", NULL,      NULL,2, szFidoAddr,      0,NULL},
  {FLAG_CZ|FLAG_IC,  "path",    "netinfo", NULL,3, szFidoPath,      0,NULL},
  {FLAG_CZ|FLAG_IC,  "nodelist","version", "5", 0, &iFidoDo5,      0,NULL},
  {FLAG_CZ|FLAG_IC,  "nodelist","version", "6", 0, &iFidoDo6,      0,NULL},
  {FLAG_CZ|FLAG_IC,  "nodelist","version", "7", 0, &iFidoDo7,      0,NULL},
  {FLAG_CZ|FLAG_IC,  "nodelist","version", "FD",0, &iFidoDoFD,     0,NULL},
  {0,                NULL,      NULL,      NULL,0, NULL, 0,NULL}
};


static struct _xlattab xlat_new_max[]=
{
  {"Hlp\\",   szHlpRep},
  {"Misc\\",  szMiscRep},
  {"Lang\\",  szLangRep},
  {NULL, NULL}
};


#define stringize(x) #x

/* Table containing file sizes and versions of the various files
 * under Max 2.00 and 2.01wb.
 */

static struct _filetab
{
  char *szName;
  unsigned long ul200dos;   /* size of the file under Max version X */
  unsigned long ul201dos;
  unsigned long ul200os2;
  unsigned long ul201os2;
  unsigned long ul202dos;
  unsigned long ul202os2;
  unsigned long ul300dos;
  unsigned long ul300os2;
  int iType;
} ftab[]=
  {                 /* 2.00dos */ /* 2.01dos */ /* 2.00os2 */  /* 2.01os2 */  /* 2.02dos */   /* 2.02os2 */
    {"accem.exe",      20868Lu,    29270Lu,       20143Lu,     32660Lu,       29260Lu,        32656Lu,      -1, -1, -1},
    {"ans2bbs.exe",    13122Lu,    11788Lu,       12041Lu,     12006Lu,       11778Lu,        12002Lu,      -1, -1, -1},
    {"ans2mec.exe",    13896Lu,    12450Lu,       12815Lu,     12634Lu,       12440Lu,        12630Lu,      -1, -1, -1},
    {"ansi2bbs.exe",   13122Lu,    11788Lu,       12041Lu,     12006Lu,       11778Lu,        12002Lu,      -1, -1, -1},
    {"ansi2mec.exe",   13896Lu,    12450Lu,       12815Lu,     12634Lu,       12440Lu,        12630Lu,      -1, -1, -1},
    {"cvtusr.exe",     24548Lu,    24438Lu,       23373Lu,     23908Lu,       24422Lu,        24914Lu,      -1, -1, -1},
    {"editcall.exe",    8610Lu,     8814Lu,        7059Lu,      8938Lu,       8818Lu,         8948Lu,       -1, -1, -1},
    {"editcal.exe",     8610Lu,     8814Lu,        7059Lu,      8938Lu,       8818Lu,         8948Lu,       -1, -1, -1},
    {"fb.exe",         16966Lu,    23030Lu,       15921Lu,     22202Lu,       23052Lu,        22214Lu,      -1, -1, -1},
    {"maid.exe",       17808Lu,    18026Lu,       16897Lu,     18134Lu,       18030Lu,        18144Lu,      -1, -1, -1},
    {"max.exe",        98112Lu,   104032Lu,           0Lu,         0Lu,       104016,         0Lu,          252288, -1, -1},
    {"maxp.exe",           0Lu,        0Lu,      351449Lu,    362996Lu,       0Lu,            366812Lu,     -1, 425038, -1},
    {"max.ovl",       219216Lu,   223488Lu,           0Lu,         0Lu,       226544Lu,       0Lu,          392272, -1, -1},
    {"maxpipe.exe",        0Lu,        0Lu,       11925Lu,         0Lu,       0Lu,            10385Lu,      -1, -1, -1},
    {"mecca.exe",      29516Lu,    39746Lu,       33660Lu,     43276Lu,       39736Lu,        43272Lu,      -1, -1, -1},
    {"mr.exe",         23728Lu,    24634Lu,       26819Lu,     23672Lu,       23666Lu,        23710Lu,      -1, -1, -1},
    {"oracle.exe",     76684Lu,    67532Lu,       82714Lu,     65076Lu,       67740Lu,        65216Lu,      -1, -1, -1},
    {"scanbld.exe",    20270Lu,    20246Lu,       18115Lu,     20626Lu,       20230Lu,        20632Lu,      -1, -1, -1},
    {"silt.exe",      101806Lu,    90216Lu,       69249Lu,     61238Lu,       175580Lu,       77638Lu,      -1, -1, -1},
    {"accemp.exe",         0Lu,        0Lu,       20143Lu,     32660Lu,       0Lu,            32656Lu,      -1, -1, -1},
    {"ans2bbsp.exe",       0Lu,        0Lu,       12041Lu,     12006Lu,       0Lu,            12002Lu,      -1, -1, -1},
    {"ans2mecp.exe",       0Lu,        0Lu,       12815Lu,     12634Lu,       0Lu,            12440Lu,      -1, -1, -1},
    {"cvtusrp.exe",        0Lu,        0Lu,       23373Lu,     24908Lu,       0Lu,            24914Lu,      -1, -1, -1},
    {"editcalp.exe",       0Lu,        0Lu,        7059Lu,      8938Lu,       0Lu,            8948Lu,       -1, -1, -1},
    {"fbp.exe",            0Lu,        0Lu,       15921Lu,     22202Lu,       0Lu,            22214Lu,      -1, -1, -1},
    {"maidp.exe",          0Lu,        0Lu,       16897Lu,     18134Lu,       0Lu,            18144Lu,      -1, -1, -1},
    {"meccap.exe",         0Lu,        0Lu,       33660Lu,     43276Lu,       0Lu,            43272Lu,      -1, -1, -1},
    {"mrp.exe",            0Lu,        0Lu,       26819Lu,     23672Lu,       0Lu,            23710Lu,      -1, -1, -1},
    {"oraclep.exe",        0Lu,        0Lu,       82714Lu,     65076Lu,       0Lu,            65216Lu,      -1, -1, -1},
    {"pmsnoop.exe",        0Lu,        0Lu,       18160Lu,         0Lu,       0Lu,            18160Lu,      -1, -1, -1},
    {"scanbldp.exe",       0Lu,        0Lu,       18115Lu,     20626Lu,       0Lu,            20632Lu,      -1, -1, -1},
    {"siltp.exe",          0Lu,        0Lu,       69249Lu,     61238Lu,       0Lu,            77638Lu,      -1, -1, -1},
  };

  #define FTAB_SIZE (sizeof(ftab)/sizeof(ftab[0]))

  /* List of files in the above table */

  enum ftab_t
  {
    ACCEM_EXE, ANS2BBS_EXE, ANS2MEC_EXE, ANSI2BBS_EXE, ANSI2MEC_EXE,
    CVTUSR_EXE, EDITCALL_EXE, EDITCAL_EXE, FB_EXE, MAID_EXE, MAX_EXE,
    MAXP_EXE, MAX_OVL, MAXPIPE_EXE, MECCA_EXE, MR_EXE, ORACLE_EXE,
    SCANBLD_EXE, SILT_EXE, ACCEMP_EXE, ANS2BBSP_EXE, ANS2MECP_EXE,
    CVTUSRP_EXE, EDITCALP_EXE, FBP_EXE, MAIDP_EXE, MECCAP_EXE,
    MRP_EXE, ORACLEP_EXE, PMSNOOP_EXE, SCANBLDP_EXE, SILTP_EXE,
  };



enum exetype_t
{
  EXEABSENT=-1,
  EXE200DOS=0,
  EXE201DOS,
  EXE200OS2,
  EXE201OS2,
  EXE202DOS,
  EXE202OS2,
  EXE300DOS,
  EXE300OS2,
  EXEUNKNOWN
};


void _fast NoMem(void)
{
  WinInfo(" ERROR!  Ran out of memory! ");
  WinExit(1);
}


MenuFunction(DlgButNew)
{
  NW(opt);
  dlg_ok=1;
  fDoUpgrade=FALSE;
  return 1;
}

MenuFunction(DlgButUpgrade)
{
  NW(opt);
  dlg_ok=1;
  fDoUpgrade=TRUE;
  return 1;
}


/* Tidy up the name of a non-base directory */

MenuFunction(ValDir)
{
  NW(opt);

  fancy_str(opt->data);
  
  Add_Trailing(opt->data, '\\');
  _TuiMenuOptNormal(opt);
  
  return 0;
}


MenuFunction(ValLanguage)
{
  if (!fDoUpgrade)
  {
    char szNewLang[PATHLEN];

    strcpy(szNewLang, szDirBase);
    strcat(szNewLang, "Lang\\");

    if (!eqstri(szNewLang, szDirLang))
    {
      WinInfo(" The language directory cannot be changed for\r\n"
              " a new install.");

      strcpy(szDirLang, szNewLang);
    }
  }

  return ValDir(opt);
}


/* Validate the base directory for an installation */

MenuFunction(ValBase)
{
  NW(opt);
  
  if (opt->appdata==DLG_EDIT)
  {
    if (szDirBase[1] != ':')
    {
      strocpy(szDirBase+2, szDirBase);
      szDirBase[0] = cCurDisk;
      szDirBase[1] = ':';
    }

    ValDir(opt);
    strcpy(szDirMisc, szDirBase);
    strcpy(szDirHlp, szDirBase);
    strcpy(szDirLang, szDirBase);
    strcpy(szDirRip, szDirBase);
    strcpy(szDirMex, szDirBase);

    strcat(szDirMisc, "Misc\\");
    strcat(szDirHlp, "Hlp\\");
    strcat(szDirLang, "Lang\\");
    strcat(szDirMex, "M\\");
    strcat(szDirRip, "Rip\\");
  }

  ValDir(opt);
  ValDir(opt+1);
  ValDir(opt+2);
  ValDir(opt+3);

  return 0;
}


MenuFunction(FidoValAddr)
{
  NW(opt);
  return 0;
}


#include "install.dlg"


/* Change to a specific drive and path */

static void near chpath(char *dir)
{
  char to[PATHLEN];

  strcpy(to, dir);

  if (strlen(to) != 3)
    Strip_Trailing(to, '\\');
  
  setdisk(toupper(*to)-'A');
  chdir(to);
}


/* Examine the files in an existing Maximus directory and determine
 * how to install the new files.
 */

static void near EnumerateFiles(void)
{
  int i;
  FFIND *ff;

  /* Scan through each file in the table and see if it exists.  If
   * so, try to identify it as a 2.0dos/os2 or 2.01dos/os2 file.
   */

  for (i=0; i < FTAB_SIZE; i++)
  {
    char szFile[PATHLEN];

    sprintf(szFile, "%s%s", szDirBase, ftab[i].szName);

    if ((ff=FindOpen(szFile, 0)) != NULL)
    {
      if (ff->ulSize==ftab[i].ul300dos)
        ftab[i].iType=EXE300DOS;
      else if (ff->ulSize==ftab[i].ul300os2)
        ftab[i].iType=EXE300OS2;
      else if (ff->ulSize==ftab[i].ul200dos)
        ftab[i].iType=EXE200DOS;
      else if (ff->ulSize==ftab[i].ul201dos)
        ftab[i].iType=EXE201DOS;
      else if (ff->ulSize==ftab[i].ul200os2)
        ftab[i].iType=EXE200OS2;
      else if (ff->ulSize==ftab[i].ul201os2)
        ftab[i].iType=EXE201OS2;
      else if (ff->ulSize==ftab[i].ul202dos)
        ftab[i].iType=EXE202DOS;
      else if (ff->ulSize==ftab[i].ul202os2)
        ftab[i].iType=EXE202OS2;
      else
      {
        int fd;

        ftab[i].iType=EXEUNKNOWN;

        if ((fd=open(szFile, O_RDONLY | O_BINARY)) != -1)
        {
          struct _tag tag;
          long ofs = lseek(fd, -(long)sizeof(struct _tag), SEEK_END);

          if (ofs > 0L && read(fd, (char *)&tag, sizeof tag)==sizeof tag)
          {
            if (tag.id1==TID_1 && tag.id2==TID_2 && tag.id3==TID_3 &&
                tag.id4==TID_4)
            {
              if (strncmp(tag.str, "Max30", 5)==0)
              {
                /* Max301DOS, Max301OS2, Max301W32, Max301GEN */

                if (strncmp(tag.str+6, "OS2",3)==0)
                  ftab[i].iType = EXE300OS2;
                else
                  ftab[i].iType = EXE300DOS;
              }
            }

          }

          close(fd);
        }
      }

      FindClose(ff);
    }

/*    printf("%s: %s\n",
           ftab[i].szName,
           ftab[i].iType==EXE200DOS ? "200dos"
         : ftab[i].iType==EXE201DOS ? "201dos"
         : ftab[i].iType==EXE200OS2 ? "200os2"
         : ftab[i].iType==EXE201OS2 ? "201os2"
         : ftab[i].iType==EXEABSENT ? "absent"
                                    : "unknown");*/
  }

/*  getch();*/
}






#ifndef COMMERCIAL /* not required for commercial install */

/* Copy file szName from the directory szFromDir to szToDir. */

static int near InstallCopyNoQuery(char *szFromDir, char *szToDir, char *szName)
{
  char szFullFrom[PATHLEN];
  char szFullTo[PATHLEN];
  char szFullFromCanon[PATHLEN];
  char szFullToCanon[PATHLEN];

  /* Create fully-pathed filenames */

  sprintf(szFullFrom, "%s%s", szFromDir, szName);
  sprintf(szFullTo, "%s%s", szToDir, szName);

  if (ilog)
    fprintf(ilog, "Copying %s -> %s\n", szFullFrom, szFullTo);

  canon(szFullFrom, szFullFromCanon);
  canon(szFullTo, szFullToCanon);

  /* If the source and destination paths are the same, do nothing. */

  if (eqstri(szFullFromCanon, szFullToCanon))
  {
    fprintf(ilog, "  ! Cannot copy file to itself.  %s skipped.\n",
            szFullFrom);

    return TRUE;
  }

  return lcopy(szFullFrom, szFullTo)==0;
}

#endif


/* Copy from szFrom to szTo, renaming the szTo file if necessary */

static int near InstallCopy200One(char *szDir, char *szFrom, char *szTo)
{
  char szFullFrom[PATHLEN];
  char szFullTo[PATHLEN];

  /* Create fully-pathed filenames */

  sprintf(szFullFrom, "%s%s", szDir, szFrom);
  sprintf(szFullTo, "%s%s", szDir, szTo);

  if (ilog)
    fprintf(ilog, "Copying %s -> %s\n", szFullFrom, szFullTo);

  return lcopy(szFullFrom, szFullTo)==0;
}


/* Check the file type of a possible 2.00 file.  If it matches an OS/2
 * executable type, regardless of the name, copy over the new
 * OS/2 executable.
 */

static int near InstallCopy200File(char *szDir, int iNewFile, int i200File)
{
  if (ftab[i200File].iType==EXE200OS2 || ftab[i200File].iType==EXE201OS2 ||
      ftab[i200File].iType==EXE202OS2)
  {
    return InstallCopy200One(szDir, ftab[iNewFile].szName,
                             ftab[i200File].szName);
  }

  return TRUE;
}


#ifdef OS_2

/* Replace a copy of a .DLL that was previously placed in
 * c:\os2\dll.
 */

static void near InstallReplaceDll(char *szName)
{
  char szTo[PATHLEN];
  char szFrom[PATHLEN];

  sprintf(szFrom, "%s%s", szDirBase, szName);

  sprintf(szTo,
          "%c:\\Os2\\Dll\\%s",
          (int)ulBootDrive+'A'-1,
          szName);

  if (fexist(szTo))
  {
    if (ilog)
      fprintf(ilog, "Copying %s -> %s\n", szFrom, szTo);

    lcopy(szFrom, szTo);
  }
}

#endif


/* This function handles the name conversions that took place between
 * Max 2.00 and Max 2.01.
 *
 * Under Max 2.00, all of the executable files had the same name in
 * the dos and os/2 versions, except for max.exe/maxp.exe.
 *
 * In 2.01, the suffix "p" was added to all of the OS/2 filenames.
 * In addition, ansi2bbs was renamed to ans2bbs, ansi2mec was
 * renamed to ans2mec, and editcall was renamed to editcal.
 * (This was so that the added "p" could be fit in the 8.3 dos filename
 * format.)
 *
 * Unfortunately, this presents a number of problems when trying to
 * upgrade an existing system.  To compensate for these complexities,
 * we first extract all of the executables using the preferred
 * names (with the trailing "p"'s for os/2).
 *
 * We have already enumerated all of the files in the Max directory
 * and determined their OS type and version.  If we find a file
 * with one of the Max 2.00 type names, there are four possibilities:
 *
 * 1) the file is from 2.00 dos
 * 2) the file is from 2.00 os/2
 * 3) the file is from 2.01 dos
 * 4) the file is from 2.01 os/2 (which the user has renamed to remove
 *    the "p")
 * 5) the file is from 2.02. Treat as if from 2.01.
 *
 * In any case, we must ensure that the user's batch files do not
 * break, so we have to update the file with the appropriate
 * OS version.
 *
 * Finally, there are a couple of separate cases for our installation.
 * If we are installing the DOS version, then the ANSI2BBS, ANSI2MEC
 * and EDITCALL files should be replaced by the DOS versions of
 * ANS2BBS, ANS2MEC and EDITCAL.  (This is because the executables
 * were just renamed, meaning that we can just copy them without
 * worrying about OS dependencies.)
 *
 * The second install case is when we are installing OS/2 and not
 * DOS.  In this case, we need to upgrade all of the old 2.00-style
 * files to the OS/2 version, regardless of whether or not they
 * have the trailing "p".  This is what is done by the
 * majority of the code in this function.
 */

static int near InstallCopyOver200(char *szDir, int iInstDos, int iInstOs2)
{
  /* Rename files for doing a dos install only */

  if (iInstDos)
  {
    if (ftab[ANSI2BBS_EXE].iType != EXEABSENT)
      if (!InstallCopy200One(szDir, ftab[ANS2BBS_EXE].szName,
                             ftab[ANSI2BBS_EXE].szName))
        return FALSE;

    if (ftab[ANSI2MEC_EXE].iType != EXEABSENT)
      if (!InstallCopy200One(szDir, ftab[ANS2MEC_EXE].szName,
                             ftab[ANSI2MEC_EXE].szName))
        return FALSE;

    if (ftab[EDITCALL_EXE].iType != EXEABSENT)
      if (!InstallCopy200One(szDir, ftab[EDITCAL_EXE].szName,
                             ftab[EDITCALL_EXE].szName))
        return FALSE;
  }

#ifdef OS_2
  /* Try to replace errant copies of *.dll files that may not
   * get properly updated.
   */

  if (iInstOs2)
  {
    struct _dllinfo *p;

    for (p = maxdll; p->szDllName; p++)
      InstallReplaceDll(p->szDllName);
  }
#endif

  /* only continue if installing os/2 and not dos */

  if (!iInstOs2 || iInstDos)
    return TRUE;

  if (! InstallCopy200File(szDir, ACCEMP_EXE,   ACCEM_EXE))     return FALSE;
  if (! InstallCopy200File(szDir, ANS2BBSP_EXE, ANS2BBS_EXE))   return FALSE;
  if (! InstallCopy200File(szDir, ANS2MECP_EXE, ANS2MEC_EXE))   return FALSE;
  if (! InstallCopy200File(szDir, ANS2BBSP_EXE, ANSI2BBS_EXE))  return FALSE;
  if (! InstallCopy200File(szDir, ANS2MECP_EXE, ANSI2MEC_EXE))  return FALSE;
  if (! InstallCopy200File(szDir, CVTUSRP_EXE,  CVTUSR_EXE))    return FALSE;
  if (! InstallCopy200File(szDir, EDITCALP_EXE, EDITCALL_EXE))  return FALSE;
  if (! InstallCopy200File(szDir, EDITCALP_EXE, EDITCAL_EXE))   return FALSE;
  if (! InstallCopy200File(szDir, FBP_EXE,      FB_EXE))        return FALSE;
  if (! InstallCopy200File(szDir, MAIDP_EXE,    MAID_EXE))      return FALSE;
  if (! InstallCopy200File(szDir, MECCAP_EXE,   MECCA_EXE))     return FALSE;
  if (! InstallCopy200File(szDir, MRP_EXE,      MR_EXE))        return FALSE;
  if (! InstallCopy200File(szDir, ORACLEP_EXE,  ORACLE_EXE))    return FALSE;
  if (! InstallCopy200File(szDir, SCANBLDP_EXE, SCANBLD_EXE))   return FALSE;
  if (! InstallCopy200File(szDir, SILTP_EXE,    SILT_EXE))      return FALSE;

  return TRUE;
}


/* Called for each file in the archive.  Returns TRUE if we are to
 * extract the file; FALSE otherwise.
 */

static word near do_file(char *name)
{
  NW(name);
  return TRUE;
}


/* Returns TRUE if we should ship extracting the file.
 * full is the full path+filespec.  This can be modified to change
 *      the destination of the extraction.
 * path is the extraction path.
 * name is the filename w/o path
 */

static word dupe_file(char *full, char *path, char *name)
{
  int doRen=FALSE;
  char new[PATHLEN];
  char *p;

  NW(path);

  /* Replace all .EXE files and certain selected .mec files
   * that need to be updated.  However, only replace the
   * .mec files if their size is the same as in the
   * 2.00 distribution.
   */

  if (stristr(name, ".exe") ||
      stristr(name, ".ovl") ||
      stristr(name, ".dll") ||
      eqstri(name, "callinfo.mec") && fsize(full)==2476 ||
      eqstri(name, "shellbye.mec") && fsize(full)==  65 ||
      eqstri(name, "wwiv.mec")     && fsize(full)==1650 ||
      eqstri(name, "filehelp.mec") && fsize(full)==6324 ||
      eqstri(name, "olr.mec")      && fsize(full)==3004)
  {
    doRen=TRUE;
  }

  /* Don't try to upgrade mcp32.dll (mostly because it may
   * already be in use).
   */

  if ((stristr(name, "mcp32.dll") || stristr(name, "mcp.exe")) && f300Upgrade)
    doRen = FALSE;

  if (eqstri(name, "english.mad") && !f300Upgrade)
  {
    long size = fsize(full);

    if (size != 63620 && size != 67435 && size != 67603 && size != 67619)
    {
      WinInfo(" INSTALL needs to install a new\r\n"
              " version of the system language\r\n"
              " file.  The old version will be\r\n"
              " renamed to ENGLISH.200.");
    }

    doRen=TRUE;
  }

  if (doRen)
  {
    strcpy(new, full);
    
    /* Rename old *.exe files to *.200 */

    if ((p=strrchr(new, '.')) != NULL)
      strcpy(p, f300Upgrade ? ".300" : ".202");
    
    if (uniqrename(full, new, NULL, NULL) != 0)
    {
      WinErr("Error renaming %s to %s", full, new);
      return TRUE;
    }
    
    return FALSE;
  }

  return TRUE;
}



/* Check to ensure that the appropriate archive files exist */

static int InstallCheckExist(int fDos, int fOs2)
{
  struct _fiztab *pf;
  static char cant_find[]="Fatal error!  Cannot find archive file %s";
  int rc=TRUE;

  /* Ensure that all of the .fiz files exist */

  for (pf=fizzes; pf->szName; pf++)
  {
    if (!fexist(pf->szName))
    {
      /* A file is allowed to be absent only if we don't need it to
       * install this OS version of Maximus.
       */

      if (
#ifndef COMMERCIAL  /* the "all" files are on disk 2 for commercial version */
      pf->iOS==FIZ_ALL ||
#endif
          (pf->iOS==FIZ_DOS && fDos) ||
          (pf->iOS==FIZ_OS2 && fOs2))
      {
        WinErr(cant_find, pf->szName);
        rc=FALSE;
      }
    }
  }

  if (!rc)
    WinInfo(" INSTALL could not find one or more of the required\r\n"
            " archive files.  The installation is aborted.");

  return rc;
}




/* Ensure that one individual directory exists */

static int near InstallAssertDir(char *path)
{
  if (direxist(path))
    return TRUE;

  if (make_dir(path)==-1)
  {
    WinErr("Error creating %s", path);
    return FALSE;
  }

  return TRUE;
}



/* Check to ensure that all of the destination directories exist */

static int InstallCheckDirs(void)
{
  if (!InstallAssertDir(szDirBase))
    return FALSE;

  if (!InstallAssertDir(szDirLang))
    return FALSE;

  if (!InstallAssertDir(szDirMisc))
    return FALSE;

  if (!InstallAssertDir(szDirMex))
    return FALSE;

  if (!InstallAssertDir(szDirRip))
    return FALSE;

  return InstallAssertDir(szDirHlp);
}


/* Extract all of the files from the .fiz archives */

static int near InstallExtractFiles(int fDos, int fOs2)
{
  long lSpaceFree=zfree(szDirBase);
  struct _fiztab *pf;

  /* Make sure that we have enough free space to do the
   * install.
   */

  if ((fDos && fOs2 && lSpaceFree < MAX_DOSOS2_SIZE) ||
      (fDos && lSpaceFree < MAX_DOS_SIZE) ||
      (fOs2 && lSpaceFree < MAX_OS2_SIZE))
  {
    int rc;

    rc=WinGetYN(" WARNING!  The install program needs at\r\n"
                " least %s megabytes of free space to install\r\n"
                " the Maximus configuration that you have\r\n"
                " selected, but only %ld bytes are\r\n"
                " available.\r\n\r\n"

                " Continue installation anyway?",

                fDos && fOs2 ? MAX_DOSOS2_SIZE_S :
                fDos ? MAX_DOS_SIZE_S :
                       MAX_OS2_SIZE_S,
                lSpaceFree);

    if (!rc)
      return FALSE;
  }

  if (!InstallCheckExist(fDos, fOs2))
    return FALSE;

  if (!InstallCheckDirs())
  {
    WinInfo(" The entire Maximus directory tree must\r\n"
            " be present for the INSTALL program to\r\n"
            " work correctly.  The installation is\r\n"
            " aborted.");
    return FALSE;
  }

  ArcWinOpen();

  for (pf=fizzes; pf->szName; pf++)
  {
    if (pf->iOS==FIZ_ALL ||
        (pf->iOS==FIZ_DOS && fDos) ||
        (pf->iOS==FIZ_OS2 && fOs2))
    {
      if (pf->fExtract)
      {
        #ifdef COMMERCIAL
          while (!fexist(pf->szName))
            WinInfo(" Please insert program disk #2.");
        #endif

        dearcit(pf->szName, pf->szDir, pf->dupe_file, pf->doit);
      }
    }
  }

#ifndef COMMERCIAL
  /* Copy the doc files to the right directory */

  InstallCopyNoQuery(".\\", szDirBase, "readme.1st");
  InstallCopyNoQuery(".\\", szDirBase, "license.doc");
  InstallCopyNoQuery(".\\", szDirBase, "max.doc");
  InstallCopyNoQuery(".\\", szDirBase, "whatsnew." VERSIONNODOT);
  InstallCopyNoQuery(".\\", szDirBase, "order.frm");
  InstallCopyNoQuery(".\\", szDirBase, "usa.frm");
  InstallCopyNoQuery(".\\", szDirBase, "deutsch.frm");
#endif

  ArcWinClose();

  return TRUE;
}


/* Update the Max control file to reflect all of the necessary changes */

static void near UpdateCtl(char *from, char *to, struct _updtab *upd, struct _xlattab *xlat, void (*endfn)(BFILE out))
{
  VWIN *ctlwin;
  BFILE in, out;
  struct _xlattab *x;
  struct _updtab *u;
  SCOMBO sc;

  char line[PATHLEN];
  char w1[PATHLEN];
  char w2[PATHLEN];
  char w3[PATHLEN];
  word lc=0;
  word nochg=FALSE;

  static char *delim="% \t\n";

  #define CTL_IBUF  8192
  #define CTL_OBUF  8192
    
  ctlwin=WinOpen(0, 0, 6, 40, BORDER_DOUBLE, CWHITE | _BLUE, CYELLOW | _BLUE,
                 WIN_CENTRE | WIN_NOCSYNC);
    
  WinCursorHide(ctlwin);

  if ((in=Bopen(from, BO_RDONLY, BSH_DENYNO, CTL_IBUF))==NULL)
  {
    WinErr("Can't open control file `%s' for read! (%d)", from, errno);

    WinExit(1);
  }

  if ((out=Bopen(to, BO_WRONLY | BO_CREAT | BO_TRUNC, BSH_DENYNO, CTL_OBUF))==NULL)
  {
    WinErr("Can't open scratch file `%s' for write! (%d)", to, errno);
    WinExit(1);
  }

  Get_Dos_Date(&sc);
  
  sprintf(w1, "%% Updated by INSTALL v" VERSION " on %02d-%02d-%02d.\r\n"
              "%%\r\n",
          sc.msg_st.date.mo, sc.msg_st.date.da, (sc.msg_st.date.yr+80) % 100);

  Bwrite(out, w1, strlen(w1));

  if (ctlwin)
  {
    sprintf(w1, "Updating %s", from);
    
    WinCenter(ctlwin, 1, w1);
    WinSync(ctlwin, TRUE);
  }

  /* Read in each line of the control file and make modifications */

  while (Bgets(line, PATHLEN, in))
  {
    char *srch="% Updated by INSTALL";

    if (lc==0 && eqstrin(line, srch, strlen(srch)))
    {
      WinInfo(" WARNING!  This control file was already modified for\r\n"
              " use with Maximus " VERSION".  Since this control file was\r\n"
              " already configured, INSTALL will not attempt to do\r\n"
              " so again.");
            
      nochg=TRUE;
    }

    Strip_Trailing(line, '\n');

    if ((++lc % 25)==0 && ctlwin)
    {
      sprintf(w1, "Line %u", lc);
      WinCenter(ctlwin, 2, w1);
      WinSync(ctlwin, TRUE);
    }

    if (!nochg)
    {
      /* Grab the first three words of each line */

      getword(line, w1, delim, 1);
      getword(line, w2, delim, 2);
      getword(line, w3, delim, 3);

      /* Now process any updates which need to be made */

      for (u=upd; u && u->w1; u++)
      {
        char *wrd, *p;

        if (!eqstri(w1, u->w1))
          continue;

        if (u->w2 && !eqstri(w2, u->w2))
          continue;

        if (u->w3 && !eqstri(w3, u->w3))
          continue;

        wrd=firstchar(line, delim, u->updword);

        if (!wrd)
          if ((wrd=firstchar(line, delim, 1))==NULL)
            wrd=line+strlen(line);

        if ((p=strchr(line, '%'))==NULL || p > wrd || (u->flag & FLAG_IC))
        {
          if (u->linefn)
            if ((*u->linefn)(line, in, out, u->wdata))
            {
              line[0]='\0';
              line[1]='\0';
            }

          /* Comment out if zero */

          if (u->flag & FLAG_CZ)
            *line=(byte)((*u->replace=='\0') ? '%' : ' ');

          if (u->flag & FLAG_UCOM)
            *line=' ';

          /* Update a word in the line, if necessary */

          if (u->updword)
          {
            if (u->flag & FLAG_W)
              sprintf(wrd, "%u", *(word *)u->replace);
            else strcpy(wrd, u->replace);
          }
        }
      }


      /* Perform textual substitutions for each line of the control file */

      if (xlat)
        for (x=xlat; x->find; x++)
        {
          char *fnd;

          if ((fnd=strstr(line, x->find)) != NULL)
          {
            strocpy(fnd+strlen(x->replace), fnd+strlen(x->find));
            memmove(fnd, x->replace, strlen(x->replace));
          }
        }
    }

    strcat(line, "\r\n");
    Bwrite(out, line, strlen(line));
  }

  if (endfn)
    (*endfn)(out);
  
  Bclose(in);
  Bclose(out);

  if (ctlwin)
    WinClose(ctlwin);
}



/* Fix up the strings used when updating the control file */

static void near InstallConfigUpdate(void)
{
  int slbase;

  sprintf(szCfgPorts, "Com%u", iCfgPort+1);
  iFidoDo5=(byte)(iFidoNodeVer==5);
  iFidoDo6=(byte)(iFidoNodeVer==6);
  iFidoDo7=(byte)(iFidoNodeVer==7);
  iFidoDoFD=(byte)(iFidoNodeVer==42);

  /* Now convert the help/misc stuff to relative paths, if possible */

  strcpy(szHlpRep, szDirHlp);
  strcpy(szMiscRep, szDirMisc);
  strcpy(szLangRep, szDirLang);

  slbase=strlen(szDirBase);

  if (strnicmp(szHlpRep, szDirBase, slbase)==0)
    strocpy(szHlpRep, szHlpRep+slbase);

  if (strnicmp(szLangRep, szDirBase, slbase)==0)
    strocpy(szLangRep, szLangRep+slbase);

  if (strnicmp(szMiscRep, szDirBase, slbase)==0)
    strocpy(szMiscRep, szMiscRep+slbase);
}


#ifdef OS_2

  static void near CopyDll(char *szName)
  {
    char szFrom[PATHLEN];
    char szTo[PATHLEN];
    int rc;

    sprintf(szFrom, "%s%s", szDirBase, szName);
    sprintf(szTo, "%s%s", szDirDll, szName);

#ifdef __FLAT__
    rc = DosCopy(szFrom, szTo, DCPY_EXISTING);
#else
    rc = DosCopy(szFrom, szTo, DCPY_EXISTING, 0L);
#endif

    if (rc)
    {
      sprintf(szFrom, " Error!  INSTALL could not write to\r\n"
                      " %s. (error %d)", szTo, (int)rc);
      WinInfo(szFrom);
    }
  }


  /* Routine to determine whether or not a given .DLL
   * can be loaded by the operating system.
   */

  static int near CanFindDll(char *szModule)
  {
    HMODULE hmod;

    if (DosLoadModule(NULL, 0, szModule, &hmod)==0)
    {
      DosFreeModule(hmod);
      return TRUE;
    }

    return FALSE;
  }


  int CanFindDlls(void)
  {
    int fFind = TRUE;
    struct _dllinfo *pd;

    for (pd=maxdll; pd->szDllName; pd++)
    {
      if (!CanFindDll(pd->szModName))
      {
        fFind=FALSE;
        break;
      }
    }

    return fFind;
  }

  /* Check to ensure that the DLLs required to run SILT are installed */

  static int near InstallCheckDlls(void)
  {
    HVMENU hDlg;
    VWIN *wWorking;
    int fFirst=TRUE;
    struct _dllinfo *pd;

    while (!CanFindDlls())
    {
      int ok;

      if (fFirst)
      {
        WinInfo(" INSTALL needs to place the Maximus .DLL files\r\n"
                " into a directory that is on your system's LIBPATH.\r\n\r\n"

                " \"C:\\OS2\\DLL\" is the normal location for .DLL\r\n"
                " files, but an alternate directory can be selected\r\n"
                " from the following dialog box.");

        fFirst=FALSE;
      }
      else
      {
        WinInfo(" INSTALL still could not find the proper Maximus\r\n"
                " .DLL files on your LIBPATH.  If you are unsure\r\n"
                " of your LIBPATH, please cancel the installation\r\n"
                " and examine CONFIG.SYS to find out which\r\n"
                " directories are on your LIBPATH.");
      }

      /* Default to the OS2 directory on the boot drive */

      sprintf(szDirDll, "%c:\\Os2\\Dll\\", (int)ulBootDrive+'A'-1);

      hDlg=TuiRegisterMenu(dlgCopyPath);
      ok=TuiExecMenu(hDlg);
      TuiDestroyMenu(hDlg);

      if (!ok)
        return FALSE;

      wWorking=WinStatus("Copying .DLL files...");

      for (pd=maxdll; pd->szDllName; pd++)
        CopyDll(pd->szDllName);

      WinClose(wWorking);
    }

    return TRUE;
  }

#endif



/* Update the main Maximus control files with the necessary changes */

static void near InstallDoControlFiles(void)
{
  char szFrom[PATHLEN];
  char szTo[PATHLEN];

  /* Configure the parameters to be used when updating the control file */

  InstallConfigUpdate();

  /* Now write the new control file */

  sprintf(szFrom, "%smax.ctl", szDirBase);
  sprintf(szTo,   "%smax.$$$", szDirBase);

  UpdateCtl(szFrom, szTo, upd_new_max, xlat_new_max, NULL);

  /* Now copy max.poo back to max.ctl */

  unlink(szFrom);
  rename(szTo, szFrom);

  /* Now translate the misc/help paths in the menus control file */

  sprintf(szFrom, "%smenus.ctl", szDirBase);
  sprintf(szTo,   "%smenus.$$$", szDirBase);

  UpdateCtl(szFrom, szTo, NULL, xlat_new_max, NULL);
  
  unlink(szFrom);
  rename(szTo, szFrom);
}

/* recompile_system
 *
 * Recompile all of the system control files, mecca scripts and MEX programs.
 */

void recompile_system(void)
{
  char temp[PATHLEN];
  char szOptP[PATHLEN];
  char szConfigName[PATHLEN];

  /* Figure out whether or not to add a "p" to the names of the programs
   * to be run.
   */

#ifdef OS_2
  if (iVerType & 2)
    strcpy(szOptP, "p");
  else
#endif
    strcpy(szOptP, "");


  WinInfo(" The install program will now compile:\r\n\r\n"

          "  - the Maximus language file,\r\n"
          "  - the MEX program source files,\r\n"
          "  - the MECCA script files, and\r\n"
          "  - the system control file.");

  /* Add the MAXIMUS environment var to config.sys/autoexec.bat */

  #ifdef OS_2
    sprintf(szConfigName, "%c:\\Config.Sys", (char)cBootDrive);
  #else
    sprintf(szConfigName, "%c:\\Autoexec.Bat", (char)cBootDrive);
  #endif

  sprintf(temp, "%smax.prm", szDirBase);
  CvtConfig(szConfigName, temp);

  /* Clear the screen and run SILT to compile the new system */

  MouseHide();
  VidCls(CGRAY);

  /* Now also set the MAXIMUS variable for this session. */

  sprintf(temp, "MAXIMUS=%smax.prm", szDirBase);
  putenv(strdup(temp));

  /* Compile language files */

  sprintf(temp, "%smaid%s %senglish", szDirBase, szOptP, szDirLang);
  printf("\n>>> %s\n", temp);
  system(temp);

  /* Compile MECCA files */

  chpath(szDirMisc);
  sprintf(temp, "%smecca%s %s*.me? -t", szDirBase, szOptP, szDirMisc);
  printf("\n>>> %s\n", temp);
  system(temp);

  chpath(szDirHlp);
  sprintf(temp, "%smecca%s %s*.me? -t", szDirBase, szOptP, szDirHlp);
  printf("\n>>> %s\n", temp);
  system(temp);

  chpath(szDirRip);
  sprintf(temp, "%smecca%s %s*.me? -t", szDirBase, szOptP, szDirRip);
  printf("\n>>> %s\n", temp);
  system(temp);

  /* Compile MEX files */

  chpath(szDirMex);
  sprintf(temp, "for %%f in (*.mex) do %smex%s %s%%f", szDirBase, szOptP, szDirMex);
  printf("\n>>> %s\n", temp);
  system(temp);

  /* Compile main control file */

  chpath(szDirBase);
  sprintf(temp, "%ssilt%s max -u", szDirBase, szOptP);
  printf("\n>>> %s\n", temp);
  system(temp);

  /* Redraw the screen */

  WinSyncAll();
  MouseShow();
}


/* Install a new version of Maximus */

static int near InstallNew(void)
{
  HVMENU hDlg;
  int ok;

  /* See if the destination directory exists.  If it does,
   * present a warning to the user.
   */

  if (direxist(szDirBase))
  {
    if (!WinGetYN(" WARNING!  The directory:\r\n"
                  "    %s\r\n"
                  " already exists.  Installing to this directory will\r\n"
                  " destroy any existing Maximus configuration in that\r\n"
                  " existing Maximus configuration\r\n"
                  " location.\r\n\n"

                  " Are you sure that you want to install Maximus in the\r\n"
                  " specified directory?",
                  szDirBase))
    {
      return FALSE;
    }
  }

  do
  {
    if (iVerType==0)
    {
      WinInfo(" Error!  INSTALL could not find the archives for\r\n"
              " either the DOS or the OS/2 version of the program.\r\n"
              " Please place either SYSP.FIZ or SYSR.FIZ in the\r\n"
              " current directory before running INSTALL again.");
      return FALSE;
    }

#ifdef __MSDOS__
    /* When installing from DOS, we *need* the dos executables, since
     * DOS can only run DOS programs.  (On the other hand, if we install
     * from OS/2, we don't necessarily need the OS/2 executables, since
     * we can always run DOS programs.
     */

    if ((iVerType & 1)==0)
    {
      WinInfo(" Error!  You need the DOS installation\r\n"
              " file (SYSR.FIZ) to install Maximus\r\n"
              " under DOS.");
      return FALSE;
    }
#endif

    /* If we have both sysr and sysp, ask the user about the target os(es) */

    if (iVerType==3)
    {
      hDlg=TuiRegisterMenu(dlgVersion);
      ok=TuiExecMenu(hDlg);
      TuiDestroyMenu(hDlg);

      if (!ok)
        return FALSE;

      if (iVerType==0)
        WinInfo(" You must select at least one target\r\n"
                " operating system.");
    }
  }
  while (iVerType==0);

  /* Set up the default configuration options */

  strcpy(szCfgName,  "New Maximus System");
  strcpy(szCfgSysop, "New Sysop");
  iCfgPort=0;
  iCfgSpeed=38400u;

  /* Get perhaps-modified config options from the user */

  hDlg=TuiRegisterMenu(dlgConfig);
  ok=TuiExecMenu(hDlg);
  TuiDestroyMenu(hDlg);

  if (!ok)
    return FALSE;

  /* Ask network-type questions */

  if (WinGetYN(" Are you a member of a FidoNet-technology\r\n"
               "   network?  (Select \"no\" if unsure.)"))
  {
    hDlg=TuiRegisterMenu(dlgFido);
    ok=TuiExecMenu(hDlg);
    TuiDestroyMenu(hDlg);

    if (!ok)
      return FALSE;
  }
  else
  {
    strcpy(szFidoAddr, "0:0/0");
  }

  /* Decompress the files into the right directory */

  if (!InstallExtractFiles(iVerType & 1, iVerType & 2))
    return FALSE;

  /* Update control files */

  InstallDoControlFiles();


  #ifdef OS_2
  /* Make sure that the DLLs are in the right place to run SILT */

  if (iVerType & 2)
  {
    if (!InstallCheckDlls())
      return FALSE;
  }
  #endif

  /* Figure out whether to use meccap or mecca */

  /* Recompile system control files */

  recompile_system();

#ifdef OS_2
  if (iVerType & 2)
    WinInfo(" Installation complete.  Please reboot your\r\n"
            " system to complete the installation.\r\n\r\n"

            " After rebooting, you may change to your new\r\n"
            " Maximus directory and type \"maxp -c\" to start\r\n"
            " Maximus and create a new user file.");
  else
#endif
    WinInfo(" Installation complete.  Please reboot your\r\n"
            " system to complete the installation.\r\n\r\n"

            " After rebooting, you may change to your new\r\n"
            " Maximus directory and type \"max -c\" to start\r\n"
            " Maximus and create a new user file.");

  return TRUE;
}


/* Extract the files for doing an upgrade */

int DoUpgradeExtract(void)
{
  VWIN *wWorking;
  int iDosVer;
  int iOs2Ver;
  int ok;

  switch (ftab[MAXP_EXE].iType)
  {
    case EXE200OS2: iOs2Ver=200;  break;
    case EXE201OS2: iOs2Ver=201;  break;
    case EXE202OS2: iOs2Ver=202;  break;
    case EXE300OS2: iOs2Ver=300;  break;
    default:        iOs2Ver=0;
  }

  switch (ftab[MAX_EXE].iType)
  {
    case EXE200DOS: iDosVer=200;  break;
    case EXE201DOS: iDosVer=201;  break;
    case EXE202DOS: iDosVer=202;  break;
    case EXE300DOS: iDosVer=300;  break;
    default:        iDosVer=0;
  }

  if (!iDosVer && !iOs2Ver)
  {
    WinInfo(" Fatal error!  The install program examined\r\n"
            " the specified directory but could not locate\r\n"
            " a valid copy of Maximus (either MAX.EXE or\r\n"
            " MAXP.EXE).  Note that this program can only\r\n"
            " upgrade from Maximus 2.x or 3.x\r\n\r\n"

            " Please ensure that your existing Maximus\r\n"
            " installation is in the correct location and\r\n"
            " run the install program again.");

    return FALSE;
  }

  ok=WinGetYN(" The install program found the following\r\n"
              " Maximus installation(s) in %s:\r\n\r\n"

              " %s%s"
              " %s%s\r\n"

              " Is this correct?",

              szDirBase,
              iDosVer ? "Maximus (DOS) " : "",
              iDosVer==200     ? "v2.00\r\n"
                : iDosVer==201 ? "v2.01wb\r\n"
                : iDosVer==202 ? "v2.02\r\n"
                : iDosVer==300 ? "v3.00\r\n"
                : iDosVer==-1  ? " - unknown version!\r\n"
                               : "",
              iOs2Ver ? "Maximus (OS/2) " : "",
              iOs2Ver==200     ? "v2.00\r\n"
                : iOs2Ver==201 ? "v2.01wb\r\n"
                : iOs2Ver==202 ? "v2.02\r\n"
                : iOs2Ver==300 ? "v3.00\r\n"
                : iOs2Ver==-1  ? "- unknown version!\r\n"
                : "");

  if (iOs2Ver && !fexist(sysp_fiz))
  {
    WinInfo(" Error!  You have instructed INSTALL to\r\n"
            " upgrade an existing Maximus-OS/2 system,\r\n"
            " but the new OS/2 executables could not be\r\n"
            " found.  Please obtain a copy of SYSP.FIZ\r\n"
            " (from " OS2ARCHIVE ") and run INSTALL again.");

    return FALSE;
  }


  if (iDosVer && !fexist(sysr_fiz))
  {
    WinInfo(" Error!  You have instructed INSTALL to\r\n"
            " upgrade an existing Maximus-DOS system,\r\n"
            " but the new DOS executables could not be\r\n"
            " found.  Please obtain a copy of SYSR.FIZ\r\n"
            " (from " DOSARCHIVE ") and run INSTALL again.");

    return FALSE;
  }

  if (!ok)
  {
    WinInfo(" The install program tries to identify your Maximus installation\r\n"
            " by checking the file sizes of MAX.EXE and MAXP.EXE.\r\n\r\n"

            " If these two files have been corrupted or modified, the install\r\n"
            " program cannot correctly identify your existing Maximus\r\n"
            " installation.\r\n\r\n"

            " Please reinstall the executables from your original Maximus\r\n"
            " distribution before continuing with the upgrade.  Note that\r\n"
            " users who are using Maximus 1.02 will need to upgrade to\r\n"
            " version 2.0 or above BEFORE running this install program.");

    return FALSE;
  }

  if (!InstallExtractFiles(iDosVer, iOs2Ver))
    return FALSE;

  wWorking=WinStatus("Updating existing Maximus executables...");

  InstallCopyOver200(szDirBase, iDosVer, iOs2Ver);

  WinClose(wWorking);

  return TRUE;
}



/* Upgrade an existing copy of Maximus */

static int near InstallUpgrade(void)
{
  VWIN *wWorking;
  HVMENU hDlg;
  int fOk;

  char szStripLang[PATHLEN];

  /* Test the location of the language directory and ensure that it
   * is where we expect it to be.
   */

  strcpy(szStripLang, szDirBase);
  strcat(szStripLang, "Lang\\");

  if (!eqstri(szStripLang, szDirLang))
  {
    int rc;

    strcpy(szStripLang, szDirBase);
    strcat(szStripLang, "Lang\\");

    rc=WinGetYN(" WARNING!  The install program has detected that your language\r\n"
                " file directory is not located in:\r\n\r\n"

                "   %s\r\n\r\n"

                " Although the upgrade can proceed, you must edit the following\r\n"
                " files to reflect the correct paths before trying to recompile\r\n"
                " your system control files:\r\n\r\n"

                "   lang\\user.lh\r\n"
                "   m\\language.mh\r\n\r\n"

                " Do you wish to continue?",
                szStripLang);

    if (!rc)
      return FALSE;
  }

  if (!WinGetYN(" Have you made a backup of your existing\r\n"
                " Maximus installation?"))
  {
    WinInfo(" We suggest that you make a backup\r\n"
            " of your Maximus directory.  While\r\n"
            " this release has been extensively\r\n"
            " tested, there is a chance that the\r\n"
            " upgrade may not work smoothly.\r\n\r\n"

            " Please re-run the install program\r\n"
            " after you have backed up your system.");

    return FALSE;
  }

  /* See what version of Max we have here */

  wWorking=WinStatus("Scanning directory...");
  EnumerateFiles();
  WinClose(wWorking);

  if (ftab[MAXP_EXE].iType==EXEUNKNOWN ||
      ftab[MAX_EXE].iType==EXEUNKNOWN)
  {
    WinInfo(" WARNING!  INSTALL could not determine the type of\r\n"
            " Maximus system installed in the specified directory.\r\n\r\n"

            " INSTALL will assume that you are upgrading from\r\n"
            " Maximus 2.x.  IF YOU ARE UPGRADING FROM MAXIMUS 3.0,\r\n"
            " cancel the installation, ensure that the release copy\r\n"
            " of MAX.EXE or MAXP.EXE is in your Maximus directory,\r\n"
            " and run the installation again.");

    if (ftab[MAX_EXE].iType==EXEUNKNOWN)
      ftab[MAX_EXE].iType = EXE202DOS;

    if (ftab[MAXP_EXE].iType==EXEUNKNOWN)
      ftab[MAXP_EXE].iType = EXE202OS2;

  }

  if (ftab[MAXP_EXE].iType==EXE300OS2 ||
      ftab[MAX_EXE].iType==EXE300DOS)
  {
    f300Upgrade = TRUE;

    /* Skip over some files that shouldn't be updated. */

    fizzes[FIZ_MISC].fExtract = FALSE;
    fizzes[FIZ_HLP].fExtract = FALSE;
    fizzes[FIZ_MEX].fExtract = FALSE;
    fizzes[FIZ_RIP].fExtract = FALSE;

    if (!DoUpgradeExtract())
      WinExit(1);

    WinInfo(" Conversion complete.");
  }
  else
  {
    /* Set default options for the install */

    sprintf(szCtlName, "%smax.ctl", szDirBase);
    set_files_base(szDirBase);

    fExtractFiles = fCvtCtl = fCvtEvt =
                    fCvtTag = fCvtConfig = fRecompile = TRUE;

    /* Register the menu and prompt the user. */

    hDlg = TuiRegisterMenu(&dlgCvt);
    fOk = TuiExecMenu(hDlg);
    TuiDestroyMenu(hDlg);

    if (fOk)
      PerformConversion();
  }

  return TRUE;
}


/* Figure out how many file handles we have available, and abort if less
 * than 15.
 */
   

static int near TestFiles(void)
{
#if !defined(OS_2) && !defined(COMMERCIAL)
  #define MIN_FD  13
    
  char *name="~~test.%d";
  char temp[PATHLEN];

  int fd[MIN_FD];
  int nf, i;
  
  for (nf=0; nf < MIN_FD; nf++)
  {
    sprintf(temp, name, nf);

    if ((fd[nf]=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                      SH_DENYNO, S_IREAD | S_IWRITE))==-1)
      break;
  }
    
  for (i=0; i < nf; i++)
  {
    close(fd[i]);
    sprintf(temp, name, i);
    unlink(temp);
  }
  
  if (nf != MIN_FD)
  {
    WinInfo(" Error!  Not enough file handles available.\r\n\r\n"
      
            " INSTALL needs more free 'file handles' than are\r\n"
            " currently available.  You must change C:\\CONFIG.SYS\r\n"
            " to allocate more files; if you don't have a CONFIG.SYS,\r\n"
            " you should create one in the root directory of drive C.\r\n\r\n"

            " In CONFIG.SYS, there should be a line that says:\r\n\r\n"
              
            "     FILES=30\r\n\r\n"
              
            " If not, add that line to the end of the file.  If the\r\n"
            " number following `FILES=' is LESS than 30, it should\r\n"
            " be increased to 30 or more.");

    return FALSE;
  }
#endif

  return TRUE;
}




/* Set up the break handler and video system.  Then open the main window */

static void near InstallStartup(void)
{
#ifdef OS_2
  int rc;
  #ifdef __FLAT__
  EXCEPTIONREGISTRATIONRECORD err;

  brktrapos2(&err, TRUE);
  #else
  brktrap();
  #endif
#else
  brktrap();
#endif
  atexit(brkuntrap);

  VidOpen(FALSE, TRUE, FALSE);
  TuiOpen();

  WinCursorHide(wscrn);
  WinFill(wscrn, '°', col(CCYAN | _BLUE));

  WinPutstra(wscrn, VidNumRows()-1, 0, col(CBLACK | _GRAY),
             " Maximus Installation ³ Copyright 1991, " THIS_YEAR
             " Lanius Corp ³ All rights reserved");

  WinCleol(wscrn, VidNumRows()-1, 78, col(CBLACK | _GRAY));
           
  WinSync(wscrn, FALSE);

#ifdef OS_2
    /* Now find the boot drive */

  #ifdef __FLAT__
    rc=DosQuerySysInfo(QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                       &ulBootDrive, sizeof ulBootDrive);
  #else
    /* QSV_BOOT_DRIVE is not defined in the os/2 1.x header
     * files, but this will work if we're running a 1.x
     * app under a 2.x system.
     */

    rc=DosQSysInfo(5, (PBYTE)&ulBootDrive, sizeof ulBootDrive);
  #endif

    if (rc)
      ulBootDrive=3;
#else
  ulBootDrive = 3;
#endif

  cBootDrive = (char)(ulBootDrive - 1 + 'A');

#ifndef COMMERCIAL
  ilog=fopen("install.log", "a");
#endif
}



/* Undo everything done by InstallStartup */

static void near InstallCleanup(void)
{
  if (ilog)
    fclose(ilog);

  MouseHide();
  WinCls(wscrn, CGRAY);
  TuiClose();

  VidCls(CGRAY);
  VidGotoXY(1, 1, FALSE);
  VidClose();
}


int _stdc main(int argc, char *argv[])
{
  HVMENU hDlg;             /* Dialog box handle */
  int ok;

  /* Init break handler and window subsystem */

  InstallStartup();

  /* Get the current disk */

  iCurDisk=get_disk();

#ifdef COMMERCIAL
  (void)argc;

  /* If current drive is not A: or B:, set the current drive to it now. */

  if (iCurDisk != 0 && iCurDisk != 1)
  {
    if (argv[0] && argv[0][1]==':')
      set_disk(toupper(argv[0][0]) - 'A');
    else
      set_disk(0);
  }

  /* Assume that we install to drive C, since the user is
   * probably on drive A right now.
   */
  iCurDisk = 2;
#else
  (void)argc;
  (void)argv;
#endif

  cCurDisk = iCurDisk + 'A';


#ifdef __MSDOS__
  if (coreleft() < 304000Lu)
  {
    WinInfo(" Error!  The install program needs to compile\r\n"
            " a number of files, but there is not enough free\r\n"
            " conventional memory to do so.  Please ensure\r\n"
            " that there is at least 560,000 bytes of free\r\n"
            " conventional memory before running INSTALL.");
    WinExit(1);
  }
#endif

#ifndef COMMERCIAL
  if (!fexist(ctl_fiz))
  {
#ifdef COMMERCIAL
    WinInfo(" Error!  The Maximus installation files must\r\n"
            " be in the current directory.  Please set the\r\n"
            " current drive to the install disk and restart\r\n"
            " INSTALL.EXE.");
    WinExit(1);
#else
    WinInfo(" Error!  The Maximus installation files (*.FIZ) must\r\n"
            " exist in the current directory.  Please change to\r\n"
            " the directory where you extracted the Maximus\r\n"
            " archives.");
    WinExit(1);
#endif
  }
#endif

  /* Now figure out which versions we are capable of installing */

  iVerType=0;

  if (fexist(sysr_fiz))
    iVerType |= 1;

  if (fexist(sysp_fiz))
    iVerType |= 2;

  /* Make sure that we have enough file handles available */

  if (TestFiles())
  {
    /* Display the "Welcome to Maximus" dialog */

    hDlg=TuiRegisterMenu(dlgWelcome);
    ok=TuiExecMenu(hDlg);
    TuiDestroyMenu(hDlg);

    if (ok)
    {
      /* Get the path to the user's Max dir.  Assume \max on the current
       * drive.
       */

      sprintf(szDirBase, "%c:\\Max\\", cCurDisk);
      sprintf(szDirMisc, "%c:\\Max\\Misc\\", cCurDisk);
      sprintf(szDirHlp,  "%c:\\Max\\Hlp\\", cCurDisk);
      sprintf(szDirLang, "%c:\\Max\\Lang\\", cCurDisk);
      sprintf(szDirMex,  "%c:\\Max\\M\\", cCurDisk);
      sprintf(szDirRip,  "%c:\\Max\\Rip\\", cCurDisk);

      hDlg=TuiRegisterMenu(dlgUpDir);
      ok=TuiExecMenu(hDlg);
      TuiDestroyMenu(hDlg);

      if (ok)
      {
        if (fDoUpgrade)
          ok=InstallUpgrade();
        else
          ok=InstallNew();

        if (!ok)
          WinInfo(" Installation aborted.  You must\r\n"
                  " run INSTALL again to install\r\n"
                  " Maximus on your system.");
      }
    }
  }

  InstallCleanup();

  return 0;
}

