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

#ifndef __DISPLAY_H_DEFINED
#define __DISPLAY_H_DEFINED

/* Quick macro to get the next character from the .BBS file */

#define DispGetChar() (d->bufp < d->highp \
                         ? (word)*d->bufp++ \
                         : (word)_DispGetChar(d, 1))

#define DispPeekChar()(d->bufp < d->highp \
                         ? (word)*d->bufp   \
                         : (word)_DispGetChar(d, 0))

#define DispGetPos()  (tell(d->bbsfile)-(long)(d->highp-d->bufp))

#define DISPLAY_NONE      0x00 /* Nothing special for display               */
#define DISPLAY_FILESBBS  0x01 /* If displaying a FILES.BBS                 */
#define DISPLAY_NEWFILES  0x02 /* If only displaying new files              */
#define DISPLAY_SEARCH    0x04 /* If we're searching FILES.BBS for text     */
#define DISPLAY_AREANAME  0x08 /* If we display area name for each area     */
#define DISPLAY_HOTMENU   0x10 /* If we're to support hotkeys               */
#define DISPLAY_PCALL     0x20 /* Increment the #-of-callers count          */
#define DISPLAY_MENUHELP  0x40 /* If we should use menuhelp for usr.help    */
#define DISPLAY_NOLOCAL   0x80 /* Don't display locally (.RBS file)         */

#define DISP_EOF 257           /* end of file indicator for DispGetChar()   */

#define DRET_NOMEM       -2     /* Not enough memory */
#define DRET_NOTFOUND    -1     /* File was not found */
#define DRET_OK           0     /* File found OK */
#define DRET_BREAK        2     /* User ^c'ed or gave 'N' to More prompt */
#define DRET_HOT          3     /* User hotkeyed out of menu */
#define DRET_EXIT         4     /* Cancel display of all via [exit] token */


typedef struct _dstk
{
  int bbsfile;    /* file we're currently reading from */
  FILE *questfile;
  
  byte onexit[PATHLEN];     /* File to display after exiting */
  byte lastfile[PATHLEN];   /* File currently being displayed */
  byte filename[PATHLEN];   /* Filename to use next */
  byte *nonstop;
  
  word chcount;             /* char counter when displaying hotflash */
  word chcount_n;           /* char counter for a normal file */
  word type;
  int ret;

  byte intern_nonstop;      /* if calling func had no 'nonstop' variable */
  byte skipcr;
  byte recd_chars;
  byte automore;
  byte ck_abort;
  byte doing_filesbbs;
  byte allanswers;
  byte break_loop;


  byte scratch[MAX_FBBS_ENTRY+10];
  byte temp[PATHLEN*2];

  byte *filebufr;           /* buffer for reading file */
  byte *bufp;               /* current location in buffer */
  byte *highp;              /* highest location in buffer */
  
  word beginline;           /* If we're at beginning of a line */

} DSTK;


word _DispGetChar(DSTK *d, word inc);
void Add_Full_Path(char *src,char *dest);
word DisplayDatacode(DSTK *d);
word DisplayQuestionnaire(DSTK *d);
word DisplayMaxCode(DSTK *d);
void DispSkipLine(DSTK *d);
word Priv_Code(int ch,int action);
char * DispGetString(DSTK *d, char *str, word maxlen);
void DispGetToBlank(DSTK *d, char *str);
int Process_Files_Entry(DSTK *d, char *orig_entry);
word DispSlowGetChar(DSTK *d);

#endif /* __DISPLAY_H_DEFINED */


