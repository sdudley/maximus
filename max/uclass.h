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

/*
 * Describes user privilege/class information API
 *
 */

#ifndef __UCLASS_H_DEFINED
#define __UCLASS_H_DEFINED

cpp_begin()

#define CLS_ID  0x8f7c9321L

  /* File header structure (internal) */

typedef struct _clshdr
{
  dword ulclhid;                                         /* ID signature */
  word  usclfirst;                             /* Offset of first record */
  word  usn;                     /* Number of user class records present */
  word  ussize;                      /* Size of individual class records */
  word  usstr;                                    /* Size of all strings */
} CLSHDR;

  /* This structure describes the priv file
   * structure and is subject to change
   */

typedef struct _clsrec
{
  word  usLevel;                               /* Numeric security level */
  word  usKey;              /* Keyletter for MECCA [?below] [?above] etc */

  zstr  zAbbrev;                          /* Abbreviation for this class */
  zstr  zDesc;                                        /* Long class name */
  zstr  zAlias;                            /* Alias for the abbreviation */

  word  usTimeDay;                    /* Daily time allowance in minutes */
  word  usTimeCall;                    /* Call time allowance in minutes */
  word  usCallsDay;                             /* Max calls/day -1=none */
  word  usMinBaud;                           /* Minimum logon baud / 100 */
  word  usFileBaud;                       /* Minimum download baud / 100 */
  word  usFileRatio;                       /* File upload:download ratio */
  word  usFreeRatio;     /* Download allowance before ratio takes effect */
  dword ulFileLimit;          /* Maximum download allowance in kilobytes */
  word  usUploadReward;                               /* % upload reward */

  zstr  zLoginFile;       /* Show this file (in misc directory) on login */

  dword ulAccFlags;                        /* Access related class flags */
  dword ulMailFlags;                         /* Mail related class flags */
  dword ulUsrFlags;                          /* User-defined class flags */

  word  usOldPriv;      /* 'old' user priv equivalent, for compatibility */

} CLSREC;

  /* Mail-related class access flags */

#define CFLAGM_PVT      0x01000000L             /* Show private messages */
#define CFLAGM_EDITOR   0x02000000L            /* External editor access */
#define CFLAGM_LEDITOR  0x04000000L      /* Local external editor access */
#define CFLAGM_NETFREE  0x08000000L                 /* Netmail is 'free' */
#define CFLAGM_ATTRANY  0x10000000L /* Allow 'sysop' attribute overrides */
#define CFLAGM_RDONLYOK 0x40000000L /* Allow write priv in readonly area */
#define CFLAGM_NOREALNM 0x80000000L      /* Don't add ^aREALNAME to msgs */

  /* General purpose access flags */

#define CFLAGA_ULBBSOK  0x00000001L        /* Uploading .?bs files is ok */
#define CFLAGA_FLIST    0x00000002L /* Allow file dloads not in filelist */
#define CFLAGA_FHIDDEN  0x00000004L       /* See/download 'hidden' files */
#define CFLAGA_UHIDDEN  0x00000008L /* Allow 'not in userlist' user view */
#define CFLAGA_HIDDEN   0x00000010L          /* Always hide in user list */
#define CFLAGA_HANGUP   0x00000020L                /* Hangup immediately */
#define CFLAGA_NOLIMIT  0x00000040L          /* Don't check dload limits */
#define CFLAGA_NOTIME   0x00000080L           /* Don't check time limits */

#define mailflag(b)   (!!(ClassGetInfo(cls,CIT_MAILFLAGS) & (b)))
#define acsflag(b)    (!!(ClassGetInfo(cls,CIT_ACCESSFLAGS) & (b)))

 /* The following determine the type of information
  * returned by ClassGetInfo()
  */

#define CIT_NUMCLASSES    -1
#define CIT_DAY_TIME      0
#define CIT_CALL_TIME     1
#define CIT_DL_LIMIT      2
#define CIT_RATIO         3
#define CIT_MIN_BAUD      4
#define CIT_MIN_XFER_BAUD 5
#define CIT_MAX_CALLS     6
#define CIT_FREE_RATIO    7
#define CIT_UPLOAD_REWARD 8
#define CIT_ACCESSFLAGS   9
#define CIT_MAILFLAGS     10
#define CIT_USERFLAGS     11
#define CIT_LEVEL         12
#define CIT_KEY           13
#define CIT_INDEX         14
#define CIT_OLDPRIV       15

  /* This structure is used in memory to
   * manage the API interface
   */

typedef struct _uclassh
{
  word    usn;                            /* Number of classes available */
  word    ussize;                                         /* Record size */
  CLSREC  *pcInfo;                 /* Pointer to class information array */
  char    *pHeap;                       /* Pointer to class strings heap */
} CLH, *PCLH;

  /* Privilege comparison operators */

enum _privcmp
{
  privGE  =0,   /* >= */
  privLE  =1,   /* <= */
  privGT  =2,   /* >  */
  privLT  =3,   /* <  */
  privEQ  =4,   /* == */
  privNE  =5    /* != */
};


  /* Class information API */

int ClassReadFile(char *pszName);        /* Read class information file */
void ClassDispose();

CLSREC *ClassRec(int idx);

int ClassKeyIndex(word key);
word ClassKeyLevel(word key);
int ClassAbbrevIndex(char *pszAbbrev);            /* Cvt priv sym to idx */
word ClassAbbrevLevel(char *pszAbbrev);            /* C'vt abbrev to idx */
int ClassLevelIndex(word usLevel);             /* Convert level to index */
dword ClassGetInfo(int idx, int itype);             /* class information */
word ClassLevel(char *pszAbbrev);                   /* Level from access */
void ClassFlag(int idx, int which, dword fSet, dword fReset);
char *privstr(word priv, char *buf);

#define ClassSetMailFlag(i,f)   ClassFlag(i,CIT_MAILFLAGS,f,0)
#define ClassResetMailFlag(i,f) ClassFlag(i,CIT_MAILFLAGS,0,f)
#define ClassSetAcsFlag(i,f)    ClassFlag(i,CIT_ACCESSFLAGS,f,0)
#define ClassResetAcsFlag(i,f)  ClassFlag(i,CIT_ACCESSFLAGS,0,f)

#define Priv_Level(l)         ClassDesc(ClassLevelIndex(l))

int max2priv(word usLevel);
word max3priv(int iPriv);

cpp_end()

#endif

