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

#include "bfile.h"

#define VERSION       "3.00"
#define VERSIONNODOT  "300"
#define DOSARCHIVE    "MAX" VERSIONNODOT "R.ZIP"
#define OS2ARCHIVE    "MAX" VERSIONNODOT "R.ZIP"

struct _updtab
{
  #define FLAG_W    0x01    /* "replace" points to a word, not a string */
  #define FLAG_CZ   0x02    /* Comment out if *replace=='\0' */
  #define FLAG_UCOM 0x04    /* Uncomment this field always */
  #define FLAG_IC   0x08    /* Ignore comment strings */
    
  word flag;
  char *w1, *w2, *w3;
  word updword;
  char *replace;
  word wdata;
  word (*linefn)(char *line, BFILE in, BFILE out, word data);
};

struct _xlattab
{
  char *find;
  char *replace;
};

extern char szDirBase[];    /* Main Max directory */
extern char szCtlName[];    /* System control file */
extern struct _vmenu dlgCvt[];/* Upgrade dialog */
extern int fExtractFiles;
extern int fCvtCtl;
extern int fCvtEvt;
extern int fCvtTag;
extern int fCvtConfig;
extern int fRecompile;
extern char cBootDrive;


void ArcWinOpen(void);
void ArcWinClose(void);
void _stdc WinErr(char *format, ...);
void PerformConversion(void);
void set_files_base(char *szBase);
int DoUpgradeExtract(void);
void _fast NoMem(void);
void recompile_system(void);
void CvtConfig(char *szCfg, char *szPrm);

