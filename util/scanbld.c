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
static char rcs_id[]="$Id: scanbld.c,v 1.2 2003/06/11 02:13:06 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=SCANBLD main module
*/

#define MAX_INCL_VER
#define SCANBLD_COMPILE

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include <ctype.h>
#include "max.h"
#include "msgapi.h"
#include "newarea.h"
#include "areaapi.h"
#include "scanbld.h"
#include "alc.h"
#include "userapi.h"
#include "prmapi.h"



/* Private data structure used for config info */

struct _sbcfg
{
  HPRM hp;                              /* Handle to .prm file */
  char userindex[PATHLEN];              /* Path to user index file */
  char mareafile[PATHLEN];              /* Path to message area data files */

  int num_names;                        /* Explicit msg areas to scan */
  char *names_to_scan[MAX_SCANAREA];

  int num_hash;                         /* Hash of user names to store */
  dword far *hashes;

  int flags;                            /* Miscellaneous bitflags */
};



/* Hash function used for calculating the hashes in the .sqi file */

static dword near UserHash(byte OS2FAR *f)
{
  dword hash=0, g;
  char OS2FAR *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + (dword)tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }
  

  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}


static void near Scan_More(void)
{

  printf("[Press Enter] ");
  fflush(stdout);
#ifdef UNIX
  getchar();
#else
  getch();
#endif

  printf("\x1b[A\r              \r");
}

void _fast NoMem(void)
{
  printf("Ran out of memory!\n");
  exit(1);
}

/* Display the SCANBLD help screen */

static void near Format(void)
{
  static char *help[]=
  {
    "Error in command-line!  Format:\n\n",

    "SCANBLD [<area_name>...] [@<tosslog>] [[-x]...]\n\n",

    "[area_type] is an optional list of names or types of areas to be scanned.\n",
    "You may specify the names of one or more message areas on your system.  In\n",
    "addition, the following special keywords can be used to select sets of areas:\n\n",

    "   All - Scan all areas.\n",
    "  Echo - Scan only EchoMail areas.\n",
    "  Conf - Scan only Conference areas.\n",
    " Local - Scan only local message areas.\n",
    "Matrix - Scan only Matrix/NetMail areas.\n\n",

#ifdef OS_2
    "For normal operation, SCANBLD should be invoked as \"scanbldp all\".\n\n"
#else
    "For normal operation, SCANBLD should be invoked as \"scanbld all\".\n\n"
#endif

    "When using the special area types above, you may also exclude certain areas\n",
    "from the scan.  To do this, add a '!' in front of the area name.\n\n",

    "@",

    "For example, 'scanbld all !comments !foobar' would scan all message areas on\n"
    "the system except for 'comments' and 'foobar'.\n\n"

    "If specified, <tosslog> is a file containing a list of areas tags\n",
    "to be scanned, one per line.  Each line in the file will be compared\n",
    "with the 'tag' keyword for each area in MSGAREA.CTL.  Areas with tags\n",
    "listed in <tosslog> will be scanned, in addition to any areas specified\n",
    "on the command-line.\n\n",

    "You can also specify any of the following switches to change SCANBLD's\n",
    "operation:\n\n",

    "  -c       - Forced compile of all areas.  Instead of scan each area for\n",
    "             changes and conditionally compiling the area, using -c forces\n"
    "             SCANBLD to rebuild all areas.\n",
    "  -m<file> - Use msg area data from <file> instead of default from PRM file.\n"
    "  -nd      - Tells SCANBLD not to delete the @tosslog file after processing.\n",
    "  -p<file> - Use PRM file <file> instead of the MAXIMUS environment variable.\n",
    "  -q       - Forces quiet operation.  SCANBLD will display a `#' for each area\n",
    "             processed, instead of the usual area statistics.\n",
    "  -u<file> - Use user file <file> instead of the default from the PRM file.\n",
    NULL
  };

  char **pp;

  for (pp=help; *pp; pp++)
    if (eqstri(*pp, "@"))
      Scan_More();
    else printf("%s", *pp);

  exit(1);
}

static void near TooManyAreas(void)
{
  printf("Error!  Too many areas specified on command line!\n");
  exit(1);
}

static void near ScanOneMessage(struct _sbcfg *psc, int fd, HMSG hmsg, long mn)
{
  SBREC sr;
  dword OS2FAR *ph;
  dword OS2FAR *eh;
  dword hash;
  XMSG xmsg;

  if ((mn % 10)==0 && (psc->flags & SFLAG_QUIET)==0)
  {
    printf("\b\b\b\b%04d", mn);
    fflush(stdout);
  }

  MsgReadMsg(hmsg, &xmsg, 0L, 0L, NULL, 0L, NULL);

  hash=UserHash(xmsg.to);

  /* Now scan for the "To:" name in the table of hashes */

  for (ph=psc->hashes, eh=psc->hashes + psc->num_hash; ph < eh; ph++)
    if (*ph==hash)
    {
      /* Got it!  Fill out the SBREC for this message */

      memset(&sr, 0, sizeof sr);
      sr.msgnum=mn;
      strcpy(sr.to, xmsg.to);
      sr.attr=(word)xmsg.attr;

      /* Now write this SBREC entry to disk */

      if (write(fd, (char *)&sr, sizeof sr) != sizeof sr)
      {
        printf("\nError!  Cannot write to SCANFILE.DAT!\n");
        exit(1);
      }

      break;
    }

  /* If we couldn't find it, there was no harm done */

  return;
}


static void near ScanOneArea(struct _sbcfg *psc, PMAH pmah)
{
  SBHDR sbhdr;
  char sfname[PATHLEN];
  char pszMsgpath[PATHLEN];
  int do_partial, do_full;
  long mn;
  HAREA ha;
  HMSG hmsg;
  int fd;

  /* Do a full build if requested by the user */

  do_full=!!(psc->flags & SFLAG_FORCE);
  do_partial=FALSE;

  if ((pmah->ma.type & MSGTYPE_SDM)==0)
    return;

  PrmRelativeString(psc->hp, PMAS(pmah, path), pszMsgpath);

  if (psc->flags & SFLAG_QUIET)
    printf("#");
  else
    printf("Area %-36s - Scanning", pszMsgpath);

  fflush(stdout);

  if ((ha=MsgOpenArea(pszMsgpath, MSGAREA_NORMAL, pmah->ma.type))==NULL)
  {
    printf("\nCan't open area '%s'.  Skipping!\n", pszMsgpath);
    return;
  }

  strcpy(sfname, pszMsgpath);
  strcat(sfname, "scanfile.dat");

  if ((fd=sopen(sfname, O_CREAT | O_RDWR | O_BINARY,
                SH_DENYNO, S_IREAD | S_IWRITE))==-1)
  {
    printf("\nError!  Cannot open scanfile %s for write!\n", sfname);
    exit(1);
  }

  /* Read the number of messages scanned for this area in the past.  If     *
   * we can't read it, assume that it is a new file and just blank it       *
   * out.                                                                   */

  if (read(fd, (char *)&sbhdr, sizeof sbhdr) != sizeof sbhdr)
    memset(&sbhdr, 0, sizeof sbhdr);

  /* If both the number of messages and the highest message have been       *
   * incremented by the same amount, it probably means that one or more     *
   * messages were entered and nothing else happened, so we can do          *
   * just a partial build.  Otherwise, do a full build!                     */

  if (sbhdr.num_msg &&
      MsgGetNumMsg(ha)-sbhdr.num_msg == MsgGetHighMsg(ha)-sbhdr.high_msg &&
      MsgGetHighMsg(ha) >= sbhdr.high_msg)
    do_partial=TRUE;
  else do_full=TRUE;

  if (do_full)
  {
    /* If we're doing a full build, truncate the scanfile */

    setfsize(fd, 0L);
    lseek(fd, sizeof(SBHDR), SEEK_SET);

    if ((psc->flags & SFLAG_QUIET)==0)
    {
      printf(" - Building 0001");
      fflush(stdout);
    }

    mn=1L;
  }
  else
  {
    /* Otherwise, just append to the file */

    mn=sbhdr.high_msg+1L;
    lseek(fd, 0L, SEEK_END);

    if ((psc->flags & SFLAG_QUIET)==0)
    {
      printf(" - Updating %04d", (int)sbhdr.high_msg);
      fflush(stdout);
    }
  }

  for (; mn <= MsgGetHighMsg(ha); mn++)
    if ((hmsg=MsgOpenMsg(ha, MOPEN_READ, mn)) != NULL)
    {
      ScanOneMessage(psc, fd, hmsg, mn);
      MsgCloseMsg(hmsg);
    }

  /* Now write the updated scanfile header back to this file */

  sbhdr.num_msg=MsgGetNumMsg(ha);
  sbhdr.high_msg=MsgGetHighMsg(ha);

  lseek(fd, 0L, SEEK_SET);

  if (write(fd, (char *)&sbhdr, sizeof sbhdr) != sizeof sbhdr)
    printf("\nError writing to scanfile %s!\n", sfname);

  close(fd);
  MsgCloseArea(ha);

  if ((psc->flags & SFLAG_QUIET)==0)
    printf("\b\b\b\b%04d - Done\n", mn-1);
}




/* This function returns TRUE if we are to scan the area 'pmah' */

static int near ProcessThisArea(struct _sbcfg *psc, PMAH pmah)
{
  int i;

  /* First scan the explicit list of areas to scan/not scan */

  for (i=0; i < psc->num_names; i++)
    if (*psc->names_to_scan[i]=='#' &&
        eqstri(PMAS(pmah, echo_tag), psc->names_to_scan[i]+1))
      return TRUE;
    else if (*psc->names_to_scan[i]=='!' &&
             eqstri(PMAS(pmah, name), psc->names_to_scan[i]+1))
      return FALSE;
    else if (eqstri(PMAS(pmah, name), psc->names_to_scan[i]))
      return TRUE;

  /* Now check broad, area-type definitions */

  if ((pmah->ma.attribs & MA_ECHO) && (psc->flags & SFLAG_ECHO)==0)
    return FALSE;

  if ((pmah->ma.attribs & MA_NET) && (psc->flags & SFLAG_NET)==0)
    return FALSE;

  if ((pmah->ma.attribs & (MA_CONF|MA_NET|MA_ECHO))==0 &&
      (psc->flags & SFLAG_LOCAL)==0)
    return FALSE;

  if ((pmah->ma.attribs & MA_CONF) && (psc->flags & SFLAG_CONF)==0)
    return FALSE;

  /* If we've made it this far, assume that the user wants the areas        *
   * to be scanned.                                                         */

  return TRUE;
}


/* Read through the area file and scan each eligible area */

static void near ScanAreas(struct _sbcfg *psc)
{
  MAH ma={0};
  HAF ham;
  HAFF haff;

  if ((ham=AreaFileOpen(psc->mareafile, TRUE))==NULL)
  {
    printf("\nError opening area data file %s!\n", psc->mareafile);
    exit(1);
  }

  if ((haff=AreaFileFindOpen(ham, NULL, 0)) != NULL)
  {
    while (AreaFileFindNext(haff, &ma, FALSE)==0)
      if (ProcessThisArea(psc, &ma))
        ScanOneArea(psc, &ma);

    AreaFileFindClose(haff);

    DisposeMah(&ma);
  }

  AreaFileClose(ham);
}



/* Parse the command-line arguments for SCANBLD */

static void near ParseArgs(int argc, char *argv[], struct _sbcfg *psc)
{
  int i;
  char *szTossName=NULL;
  int fGotArea = FALSE;

  if (argc < 2)
    Format();

  for (i=1; i < argc; i++)
  {
    if (eqstri(argv[i], "echo"))
    {
      psc->flags |= SFLAG_ECHO;
      fGotArea = TRUE;
    }
    else if (eqstri(argv[i], "net") || eqstri(argv[i], "matrix"))
    {
      psc->flags |= SFLAG_NET;
      fGotArea = TRUE;
    }
    else if (eqstri(argv[i], "conf"))
    {
      psc->flags |= SFLAG_CONF;
      fGotArea = TRUE;
    }
    else if (eqstri(argv[i], "local"))
    {
      psc->flags |= SFLAG_LOCAL;
      fGotArea = TRUE;
    }
    else if (eqstri(argv[i], "all"))
    {
      psc->flags |= SFLAG_ALL;
      fGotArea = TRUE;
    }
    else if (*argv[i]=='-' || *argv[i]=='/')
    {
      switch (tolower(argv[i][1]))
      {

        case 'c':   psc->flags |= SFLAG_FORCE;  break;
        case 'q':   psc->flags |= SFLAG_QUIET;  break;
        case 'n':   psc->flags |= SFLAG_NODEL;  break;
        case 'u':   strcpy(psc->userindex, argv[i]+2);  break;
        case 'm':   strcpy(psc->mareafile, argv[i]+2);  break;
        case 'p':   break; /* handled by GetMaximus */
        default:    printf("Unknown switch %s\n", argv[i]); exit(1);
      }
    }
    else if (*argv[i]=='@')
    {
      FILE *fp;
      char name[PATHLEN];

      fGotArea = TRUE;

      if ((fp=shfopen(argv[i]+1, "r", O_RDONLY))==NULL)
      {
        printf("Warning:  Can't read echotoss log `%s'.\n\n", argv[i]+1);
        continue;
      }

      szTossName=argv[i]+1;

      while (fgets(name, PATHLEN, fp))
      {
        Strip_Trailing(name, '\n');

        if (psc->num_names >= MAX_SCANAREA)
          TooManyAreas();
        
        if ((psc->names_to_scan[psc->num_names]=malloc(strlen(name)+2))==NULL)
          NoMem();

        *psc->names_to_scan[psc->num_names]='#';
        strcpy(psc->names_to_scan[psc->num_names++]+1, name);
      }

      fclose(fp);
    }
    else
    {
      if (psc->num_names >= MAX_SCANAREA)
        TooManyAreas();

      if ((psc->names_to_scan[psc->num_names]=malloc(PATHLEN))==NULL)
        NoMem();

      strcpy(psc->names_to_scan[psc->num_names++], argv[i]);
      fGotArea = TRUE;
    }
  }

  /* If no args specified, assume all areas */

  if (!fGotArea)
    psc->flags |= SFLAG_ALL;

  if (szTossName && (psc->flags & SFLAG_NODEL)==0)
    unlink(szTossName);
}


/* Read the user file index and use it as a hash of all user names */

static void near HashUserFile(struct _sbcfg *psc)
{
  HUF huf;
  HUFFS huffs;
  dword far *pdwHash;
  dword far *pdwEnd;
  long size;

  printf("(hashing user file)");
  fflush(stdout);

  if ((huf=UserFileOpen(psc->userindex, 0))==NULL)
  {
    printf("\nError opening user file %s for read!\n", psc->userindex);
    exit(1);
  }

  /* Determine size of hash table */

  size=UserFileSize(huf) * sizeof(dword) * 2;

  if ((psc->hashes=farmalloc(size))==NULL)
    NoMem();

  psc->num_hash=size / sizeof(dword);

  pdwHash=psc->hashes;
  pdwEnd=pdwHash + size;

  if ((huffs=UserFileFindSeqOpen(huf)) != NULL)
  {
    do
    {
      if (pdwHash < pdwEnd)
        *pdwHash++ = UserHash(huffs->usr.name);

      if (pdwHash < pdwEnd)
        *pdwHash++ = UserHash(huffs->usr.alias);
    }
    while (UserFileFindSeqNext(huffs));

    UserFileFindSeqClose(huffs);
  }

  UserFileClose(huf);

  printf("\r                   \r");
  fflush(stdout);
}


/* Initialize the MsgAPI */

static void near Init(struct _sbcfg *psc, int argc, char **argv)
{
  struct _minf mi;
  char *pszMaximus;

  mi.req_version=0;
  mi.def_zone=1;
  mi.haveshare=0;

  MsgOpenApi(&mi);

  pszMaximus = GetMaximus(argc, argv, 1);

  memset(psc, 0, sizeof *psc);
  psc->hp = PrmFileOpen(pszMaximus, 1);

  /* Get the default user index and message area filenames */

  PrmRelativeString(psc->hp, PrmFileString(psc->hp, user_file), psc->userindex);
  PrmRelativeString(psc->hp, PrmFileString(psc->hp, marea_name), psc->mareafile);
}

static void near Term(void)
{
  MsgCloseApi();
}

int c_main(int argc, char *argv[])
{
  struct _sbcfg sc;

  Hello("SCANBLD", "Mail database update utility", VERSION, "1990, " THIS_YEAR);

  Init(&sc, argc, argv);
  ParseArgs(argc, argv, &sc);
  HashUserFile(&sc);
  ScanAreas(&sc);
  Term();

  printf("\nDone!\n");

  return 0;
}


