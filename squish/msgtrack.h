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

#ifndef __TYPEDEFS_H_DEFINED
#define __TYPEDEFS_H_DEFINED

/* Handle the special defines for 32-bit flat model compilers */

#if defined(__386__) || defined(__FLAT__)
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned short word;
  typedef signed short   sword;

  typedef unsigned int  dword;
  typedef signed int    sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#else
  typedef unsigned      bit;

  typedef unsigned char byte;
  typedef signed char   sbyte;

  typedef unsigned short word;
  typedef signed short  sword;

  typedef unsigned long dword;
  typedef signed long   sdword;

  typedef unsigned short ushort;
  typedef   signed short sshort;

  typedef unsigned long  ulong;
  typedef   signed long  slong;
#endif

#endif



/*--------------------------------------------------------------------------*/
/* nodex.ndx                                                                */
/*                                                                          */
/* Version 7 Nodelist Index structure.  This is a 512-byte record, which    */
/* is defined by three structures:  Record 0 is the Control Record, then    */
/* some number of Leaf Node (LNode) Records, then the Index Node (INode)    */
/* Records.  This defines an unbalanced binary tree.                        */
/*                                                                          */
/* This description is based on Scott Samet's CBTREE.PAS program.           */
/*                                                                          */
/*--------------------------------------------------------------------------*/

struct _ndx {
    union
    {
        struct _CtlBlk {
            word    CtlBlkSize; /* Blocksize of Index Blocks   */
            sdword  CtlRoot;    /* Block number of Root        */
            sdword  CtlHiBlk;   /* Block number of last block  */
            sdword  CtlLoLeaf;  /* Block number of first leaf  */
            sdword  CtlHiLeaf;  /* Block number of last leaf   */
            sdword  CtlFree;    /* Head of freelist            */
            sdword  CtlLvls;    /* Number of index levels      */
            word    CtlParity;  /* XOR of above fields         */
        } CtlBlk;

        struct _INodeBlk {
            sdword  IndxFirst;  /* Pointer to next lower level */
            sdword  IndxBLink;  /* Pointer to previous link    */
            sdword  IndxFLink;  /* Pointer to next link        */
            sword   IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            /* If IndxFirst is NOT -1, this is INode:          */
            struct _IndxRef {
                word   IndxOfs; /* Offset of string into block */
                word   IndxLen; /* Length of string            */
                sdword IndxData;/* Record number of string     */
                sdword IndxPtr; /* Block number of lower index */
            } IndxRef[20];
        } INodeBlk;

        struct _LNodeBlk {
                                /* IndxFirst is -1 in LNodes   */
            sdword  IndxFirst;  /* Pointer to next lower level */
            sdword  IndxBLink;  /* Pointer to previous link    */
            sdword  IndxFLink;  /* Pointer to next link        */
            sword   IndxCnt;    /* Count of Items in block     */
            word    IndxStr;    /* Offset in block of 1st str  */
            struct _LeafRef {
                word   KeyOfs;  /* Offset of string into block */
                word   KeyLen;  /* Length of string            */
                sdword KeyVal;  /* Pointer to data block       */
            } LeafRef[30];
        } LNodeBlk;

        char RawNdx[512];

    } ndx;
};

/*--------------------------------------------------------------------------*/
/*                                                                          */
/* OPUS 1.20 Version 7 Nodelist structure. Copyright 1991 Wynn Wagner III   */
/* and Doug Boone. Used by permission.                                      */
/*                                                                          */
/*--------------------------------------------------------------------------*/

struct _vers7 {
        sword  Zone;
        sword  Net;
        sword  Node;
        sword  HubNode;        /* If node is a point, this is point number. */
        word CallCost;         /* phone company's charge */
        word MsgFee;           /* Amount charged to user for a message */
        word NodeFlags;        /* set of flags (see below) */
        byte ModemType;        /* RESERVED for modem type */
        byte Phone_len;
        byte Password_len;
        byte Bname_len;
        byte Sname_len;
        byte Cname_len;
        byte pack_len;
        byte BaudRate;         /* baud rate divided by 300 */
};


