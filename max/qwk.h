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

#ifndef __QWK_H_DEFINED
#define __QWK_H_DEFINED

/* Structure for MESSAGES.DAT:

 "Produced by Qmail...Copyright (c) 1987 by Sparkware.  All Rights Reserved."
   (Pad this string to 128 chars / rec #1)
     
   {
     struct _qmhdr;
     char msgblocks[n][128];
   }
 }

*/


#define QWK_RECSIZE   128       /* 128 bytes per record */
#define QWK_EOL       (byte)'\xe3'    /* End-of-line char for PCBored systems */
#define QWK_EOL_STR   "\xe3"    /* Above, as a string */
#define MAX_QIDX        128
#define MAX_LEN_CHAIN   128
#define MAX_QWK_AREAS   254     /* Max 254 areas in QWK format */

#define QWK_ACTIVE  225
#define QWK_KILLED  226


struct _qmhdr
{
  byte status;  /* '*' or '+' for private, ' ' otherwise */
  byte msgn[7];   
  byte date[8];   /* MM-DD-YY */
  byte time[5];   /* HH:MM */
  byte to[25];
  byte from[25];
  byte subj[25];
  byte pwd[12];
  byte replyto[8];
  byte len[6];
  byte msgstat;   /* See QWK_xxx, above */
  byte confLSB;
  byte confMSB;
/*byte wasread;*/
  byte rsvd[3];
};


/* Structure of the ###.NDX records */

struct _qmndx
{
  byte mks_rec[4];
  byte conf;
};




/* Structure of a ########.DAT kludge file:

   _akh                       (Header file)
   _akdat[0..akh.num_areas]   (One per area)
*/

struct _akh
{
  char name[36];  /* This user's name */
  word num_areas;
};

struct _akd
{
  byte name[MAX_ALEN];      /* Area name, as known to Max */
  union stamp_combo used;   /* Date that this area was last used */
};



int QWK_Begin(BROWSE *b);
int QWK_Display(BROWSE *b);
int QWK_After(BROWSE *b);
int QWK_Status(BROWSE *b,char *aname,int colour);
int QWK_Idle(BROWSE *b);
int QWK_End(BROWSE *b);

int Read_Kludge_File(struct _akh *akh, struct _akd **akd);
int InsertAkh(char *aname, int tossto);
int Write_Kludge_File(struct _akh *akh, struct _akd **akd);

#endif /* __QWK_H_DEFINED */

