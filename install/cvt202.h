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

#define MAX_FILE_STACK    16          /* Up to 16 nested include files */
#define MAX_LINE_WORDS    32          /* Up to 32 words on one line */
#define MAX_CTL_LINE      (PATHLEN*3) /* Max length of .ctl line */

typedef struct _filestk
{
  FILE *fpIn;
  FILE *fpOut;

  char szInName[PATHLEN];
  char szOutName[PATHLEN];

  int iLine;
} FILESTK;

/* Enumeration describing the states of the converter */

typedef enum
{
  CS_TOP,
  CS_SYSTEM,
  CS_MATRIX,
  CS_MENU,
  CS_AREA,
  CS_COLOUR,
  CS_SESSION,
  CS_EVENT,
  CS_LANGUAGE,
  CS_END_
} CVTSTATE;

#define MAX_TRANSLATE_TABLES    CS_END_

/* Keyword translation table entry for a state */

typedef struct
{
  char *szFrom;     /* Keyword to translate FROM */
  char *szTo;       /* Keyword to translate TO */
  int fOpt;
} XLATTABLE;

/* Arguments passed to keyword handlers */

typedef struct
{
  char *szLine;
  char *szStrippedLine;
  char *szKeyword;
  char *szWords[MAX_LINE_WORDS];
} CVTPARM;

/* Prototype for a keyword handler */

typedef CVTSTATE (*KWHANDLER)(CVTPARM *pcp);

/* Typedef for an entry in a keyword translator table */

typedef struct
{
  char *szKeyword;
  KWHANDLER kwh;
} TRANSLATOR;

typedef struct
{
  TRANSLATOR *ptt;        /* Keywords that require special processing */
  XLATTABLE *pxt;         /* Search-and-replace matching */
  int fHaltOutput;        /* Pass input to output file if not filtered? */
} TRANSLATESTATE;


typedef struct
{
  int fMsg;
  int fFile;
  char *szName;

  char *szMsgInfo;
  char *szMsgAccess;
  char *szMsgMenuName;
  char *szMsgName;
  char *szPath;
  char *szOrigin;
  char *szMsgOverride;
  char *szMsgBarricade;

  char szMsgStyle[PATHLEN];
  int iRenumDays;
  int iRenumMax;

  char *szFileInfo;
  char *szDownload;
  char *szUpload;
  char *szFileList;
  char *szFileAccess;
  char *szFileMenuName;
  char *szFileOverride;
  char *szFileBarricade;

} AREADATA;


typedef struct
{
  int fWroteMarea;
  int fWroteFarea;
} SESSIONDATA;

typedef struct
{
  char *szName;
} MENUDATA;


/* Struct for old MTAG.BBS */

struct _tagdata 
{
  word struct_len;
  char name[36];
  char areas[348];
};


char *fchar(char *str,char *delim,int wordno);
void Unknown(char *szWhat, char *szValue);

