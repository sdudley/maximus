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

#ifdef NEVER

/*  NOTE:  The _area structure has a dynamic length!  To access this file, *
 *         you should read the first _area structure from the file, and    *
 *         check the struct_len byte.  Then, to access the file, you seek  *
 *         to each new location, instead of reading straight through.      *
 *                                                                         *
 *         For example, to read all of the _area file into an array, you   *
 *         MUST do it like this, for upward compatiblity:                  */

  {
    struct _area area[NUM_AREAS];

    int x,
        slen;

    if ((areafile=open(area_name, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
      Error();

    /* Read the first record of the file, to grab the structure-length     *
     * byte.                                                               */

    read(areafile,&area[0],sizeof(struct _area));
    slen=area[0].struct_len;

    for (x=0;! eof(area_data);x++)
    {
      /* Note this lseek() call, which positions the pointer to the        *
       * start of the next record, no matter how long the previous         *
       * record was.                                                       */

      lseek(areafile,x*(long)struct_len,SEEK_SET);
      read(areafile,&area[x],sizeof(struct _area));
    }

    close(areafile);
  }

#endif

#define  SYSMAIL   0x0001 /* is a mail area                                */
#define  NOPUBLIC  0x0004 /* OPUS: Disallow public messages                */
#define  NOPRIVATE 0x0008 /* OPUS: Disallow private messages               */
#define  ANON_OK   0x0010 /* OPUS: Enable anonymous messages               */
#define  ECHO      0x0020 /* OPUS: Set=Echomail Clear=Not Echomail         */
#define  HIGHBIT   0x0040 /* MAX:  Allow high-bit chars in this area       */
#define  NREALNAME 0x0200 /* MAX:  Don't use ^aREALNAME for this area      */
#define  UREALNAME 0x0400 /* MAX:  Use usr.name instead of alias (if alsys)*/
#define  CONF      0x0800 /* MAX:  Conference-type area (no origin/sb's)   */
#define  UALIAS    0x1000 /* MAX:  Use usr.alias instead of usr.name       */

#define  SHARED     (CONF | ECHO)
#define  NOPVTORPUB (NOPRIVATE | NOPUBLIC)

struct _override
{
  sword priv;   /* Override priv level */
  dword lock;   /* Override lock setting */

  byte ch;      /* First letter of menu option to apply override to */
  byte fill;    /* Reserved by Maximus */
};



#define AREA_ID   0x54414441L /* "ADAT" */
#define AREA_id   AREA_ID

struct _area
{
  long id;              /* Unique identifier for AREA.DAT structure.       *
                         * Should be AREA_id, above.                       */

  word struct_len;      /* Length of _area structure -- this needs only    *
                         * to be read from the first record in an area     *
                         * data file, since it can be assumed to remain    *
                         * the same throughout the entire file.  This is   *
                         * GUARANTEED to be at offset four for this and    *
                         * all future versions of this structure.          */

  word areano;          /* OBSOLETE.  Two-byte integer representation of   *
                         * this area's name.  Use area.name instead.       */

  byte name[40];        /* String format of area's name.  USE THIS!        */

  /*************************************************************************/
  /**                        Message Area Information                     **/
  /*************************************************************************/

  word type;            /* Message base type.  MSGTYPE_SDM = *.MSG.        *
                         * MSGTYPE_SQUISH = SquishMail.  (Constants are    *
                         * in MSGAPI.H)                                    */

  byte msgpath[80];     /* Path to messages                                */
  byte msgname[40];     /* The 'tag' of the area, for use in ECHOTOSS.LOG  */
  byte msginfo[80];     /* The DIR.BBS-like description for msg section    */
  byte msgbar[80];      /* Barricade file for message area                 */
  byte origin[62];      /* The ORIGIN line for this area                   */

  sword msgpriv;        /* This is the priv required to access the msg     *
                         * section of this area.                           */
  byte fill0;           /* The lock for the message area (obsolete)        */

  byte fill1;

  sword origin_aka;     /* This is the AKA number to use on the origin     *
                         * line.  See the normal SysOp documentation on    *
                         * the "Origin" statement, for info on how this    *
                         * number is used.                                 */

  /*************************************************************************/
  /**                        File Area Information                        **/
  /*************************************************************************/


  byte filepath[80];    /* Path for downloads                              */
  byte uppath[80];      /* Path for uploads                                */
  byte filebar[80];     /* Barricade file for file areas                   */
  byte filesbbs[80];    /* Path to FILES.BBS-like catalog for this area    */
  byte fileinfo[80];    /* The DIR.BBS-like description for file section   */

  sword filepriv;       /* This is the priv required to access the file    *
                         * section of this area.                           */
  byte fill15;          /* The locks for the file area (obsolete)          */
  byte fill2;

  /*************************************************************************/
  /**                      Miscellaneous Information                      **/
  /*************************************************************************/


  byte msgmenuname[13]; /* Alternate *.MNU name to use for this msg.area   */
  byte filemenuname[13];/* Alternate *.MNU name to use for this file area  */

  word attrib[12];      /* This is an array of attributes for the          *
                         * msg/file areas.  These are dependant on PRIV    *
                         * level.  Once you have the CLASS number for a    *
                         * particular user (via Find_Class_Number()), you  *
                         * can find the attributes for that particular     *
                         * priv level like this: "area.attrib[class]"      *
                         * ...which will get you the attribute for that    *
                         * priv level.                                     */

  /*************************************************************************/
  /**                      Stuff hacked on later                          **/
  /*************************************************************************/

  struct _override movr[16]; /* Override privs for msg/file areas */
  struct _override fovr[16];
  
  dword msglock;        /* 32-bit locks for message areas                  */
  dword filelock;       /* 32-bit locks for file areas                     */

  word killbyage;       /* MAXREN: max # of days to keep msgs in this area */
                        /*         (use 0 for no deletion by age)          */
  word killbynum;       /* MAXREN: max # of msgs to keep in area (use 0    */
                        /*         for no deletion by #msgs.)              */

};



/* New Max 2.xx format for area.ndx.  The file is simply an array of        *
 * these structures.                                                        */

struct _aidx
{
  dword offset;
  byte name[10];
};


/* This is the old, Max 1.02 format for area.idx.  This is obsolete, but    *
 * it is still written by SILT for backwards compatibility.                 */

struct _102aidx
{
  word  area;       /* Same format as area.areano */
  dword offset;
  dword rsvd;
};






