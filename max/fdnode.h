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

#ifndef __FDNODE_H_DEFINED
#define __FDNODE_H_DEFINED

#include "prog.h"

#define PVT_NODELIST    0x10000000Lu
#define POINT_NODELIST  0x20000000Lu
#define MAX_NL_SIZE  256          /* Max size of raw def'n in NODELIST.### */

/****************************************************************************
                        Common Btree Definitions
 ****************************************************************************/

/* Structure used internally to hold a nodelist record */

struct _johonode
{
  char system[80];
  char city[30];
  char sysop[36];
  char phone[40];
  char flags[80];
  long baud;
};

struct _johofile
{
  int fdfd, nfd, pfd, ppfd, ufd;
};


/* Generic index */

struct _gdx
{
  dword nlofs;                    /* Index into raw nodelist.  If masked    *
                                   * with PVT_NODELIST, use FDNET.PVT       *
                                   * instead of NODELIST.###.               */

  word block_num;                 /* Block number (used in index recs only) */
  word rsvd1;
};

struct _hdr
{
  word rsvd1;
  byte rsvd2;
  word master_idx;
};


/* The four-byte header at the beginning of each Btree block */

struct _inf
{
  char nodes;
  word index;
  char rsvd[2];
};


/* Layout of first record */

struct _control
{
  struct _inf inf;
  struct _hdr hdr;
  char stuff[246];
  byte unknown;
  char nl_ext[3];
};


/****************************************************************************
                               PHONE.FDX
 ****************************************************************************/

/* PHONE.FDA structure: */

struct _pda
{
  byte rsvd1[5];
  byte phone[32];
  byte rsvd[8];
  word cost;
  byte rsvd2[1];
  byte sentinel;
};


/* A phone number record */

struct _pdx
{
  word rec_num;                   /* Record number in PHONE.FDA */
  byte shit[6];
  byte phone_len;
  byte phone[20];
};

/* Structure of a Btree data block in PHONE.FDX.  This structure            *
 * is always 928 bytes long, and the file length is always a multiple       *
 * of this.                                                                 */

struct _pdb
{
  struct _inf inf;

  union
  {
    struct _pdx pdx[32];
    struct _hdr hdr;
    char flat[928];
  } d;
};



/****************************************************************************
                            USERLIST.FDX
 ****************************************************************************/

/* A FrontDoor userlist record.  This is used in 99% of the blocks. */

struct _udx
{
  dword nlofs;                    /* Index into raw nodelist.  If masked    *
                                   * with PVT_NODELIST, use FDNET.PVT       *
                                   * instead of NODELIST.###.               */

  word block_num;                 /* Block number (used in index recs only) */

  word rsvd1;                     /* Unknown */
  byte sentinel;                  /* Unknown - always '\x18' */

  byte name[15];                  /* Space-padded, uppercase */

  byte zone_hi;
  byte zone_lo;

  byte net_hi;
  byte net_lo;

  byte node_hi;
  byte node_lo;

  byte point_hi;
  byte point_lo;

  byte type;                      /* See TYPE_XXX, below */
};


/* Structure of a Btree data block in USERLIST.FDX.  This structure         *
 * is always 1061 bytes long, and the file length is always a multiple      *
 * of this.                                                                 */

struct _udb
{
  struct _inf inf;

  union
  {
    struct _udx udx[32];
    struct _hdr hdr;
    char flat[1056];
  } d;
};


/****************************************************************************
                            NODELIST.FDX
 ****************************************************************************/




/* A FrontDoor nodelist record.  This is used in 99% of the blocks. */

struct _fdx
{
  dword nlofs;                    /* Index into raw nodelist.  If masked    *
                                   * with PVT_NODELIST, use FDNET.PVT       *
                                   * instead of NODELIST.###.               */

  word block_num;                 /* Block number (used in index recs only) */

  word rsvd1;                     /* Unknown */
  byte rsvd2;                     /* Unknown - always '\x0e' */

  byte zone_hi;                   /* MSB of zone */
  byte zone_lo;                   /* LSB of zone */

  byte net_hi;                    /* MSB of net */
  byte net_lo;                    /* LSB of net */

  byte node_hi;                   /* MSB of node */
  byte node_lo;                   /* LSB of node */

  byte point_hi;                  /* MSB of point */
  byte point_lo;                  /* LSB of point */

  word host_route;                /* Net# for host routng, or 0 if none     */
  word hub_route;                 /* Hub# for hub routing, or 0 if none     */

  byte type;                      /* See TYPE_XXX, below */

  byte sentinel;                  /* Always 0xff */
};

/* Definitions for the 'type' field, above */

#define TYPE_ZC     0x01
#define TYPE_RC     0x02
#define TYPE_NC     0x03
#define TYPE_HC     0x04
#define TYPE_PVT    0x05
#define TYPE_HOLD   0x06
#define TYPE_DOWN   0x07
#define TYPE_POINT  0x09


/* Structure of a Btree data block in NODELIST.FDX.  This structure         *
 * is always 741 bytes long, and the file length is always a multiple       *
 * of this.                                                                 */

struct _fdb
{
  struct _inf inf;

  union
  {
    struct _fdx fdx[32];
    struct _hdr hdr;
    char flat[736];
  } d;
};


int JoHoOpen(char *path, struct _johofile *jf, int userlist);
int JoHoLookup(int fd, size_t block_size, size_t rec_size, void *find, void *found, int (*compare)(void *, void *));
int JoHoFetch(void *f, struct _johofile *jf, struct _johonode *jn);
int JoHoClose(struct _johofile *jf);


#endif /* __FDNODE_H_DEFINED */


