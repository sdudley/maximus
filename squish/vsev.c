/* Modified by Bo Simonsen */

/*
 *  VSEV.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Version7 nodelist processor.
 *
 *  The version 7 nodelist is normally comprised as follows:
 *
 *  1. NODEX.DAT - the complete nodelist data file.
 *  2. NODEX.NDX - an index into nodex.dat.  The index entries are all
 *  NETADDR-based.  After using the index, you will obtain an offset,
 *  which can be used to access nodex.dat.
 *  3. SYSOP.NDX - another index into nodex.dat, this time by sysop name.
 *  The name used for the index is "surname, firstname".  Although the
 *  names are stored in mixed case, they are ordered case-insensitive.
 */

/*
 *  This module has only one external function - vsevGetInfo.  It
 *  requires a pointer to a VSEV structure, which should be empty.  The
 *  second parameter is the name of the V7 data file.  The third
 *  parameter is the offset.  You are  expected to have used one of the
 *  indexes to have retrieved the proper offset before retrieving the
 *  data.
 *
 *  The function returns 0 on success, non-zero on failure.  If the call
 *  was successful, then via the passed VSEV data structure, you will now
 *  have access to the following information:
 *
 *
 *  zone, net, node, point, hub, cost, fee, flags, modem, phone, password,
 *  board, sysop, misc, baud.
 *
 *  Note that on return from this function, the board name, sysop name
 *  and misc info will be both unpacked and converted into mixed case.
 *  Internally, the data in the v7 nodelist data file is packed by
 *  converting everything to uppercase and then compressing it by
 *  compressing 3 bytes into 2.  Presumably someone thought that the
 *  space advantages of making an abortion like this was worth it, and
 *  not just a gigantic wank.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "squish.h"
#include "vsev.h"

static void mixstr(unsigned char *str);
static void mxbtInit(MXBT * mxbt);
static void mxbtOpen(MXBT * mxbt, char *indexFile);
static void mxbtClose(MXBT * mxbt);
static void mxbtFetchControl(MXBT * mxbt);
static void mxbtSetKey(MXBT * mxbt, void *searchKey);
static void mxbtSetCompare(MXBT * mxbt, int (*compare) (void *testKey, void *searchKey, int len));
static void mxbtReadRec(MXBT * mxbt);
static void mxbtFindLeaf(MXBT * mxbt);
static void mxbtSearchLeaf(MXBT * mxbt);
static int compareNode(void *a, void *b, int len);
int v7lookupnode(NETADDR * faddr, char * indexname);

/*
 *  MXBT.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  BTree functions that operate on an index file compatible with that
 *  used by Mix Database Toolchest (or something like that).
 *
 *  The btree index file is comprised of three different record types,
 *
 *  stored as a fixed length (e.g. 512 bytes), even though the record
 *
 *  types may not require that full length.
 *
 *  1. A control record.  This is the first record in the file, and
 *  contains important information such as the fixed length that all
 *  the other records are.
 *
 *  2. Leaf nodes.  These are identified by having "-1" in the first
 *  field (a "long").  Every search key will be found in one of these
 *  records.  The leaf nodes appear after the control record.
 *
 *  3. Index nodes.  These are identified by not having "-1" in the
 *  first field.  Some, but not all, of the keys will be found in
 *  one of these fields.  The index nodes appear after the last leaf
 *  node.
 *
 *  Note that if you wish to update records in this index file, you
 *  should update the leaf nodes before the index nodes.
 *
 *  The objective of all this is to find a matching key, which will
 *  then have a corresponding "long" value.  This long value can
 *  then be used as an offset into an appropriate data file.
 *
 *  The basic structure of the index records goes like this.  Each
 *  index record has several keys stored in it.  If your key is
 *  lower than the first key, then the recType field doubles up as
 *  the *record number* of the left branch.  Multiply that by the
 *  record size and you have your offset.  Otherwise, go through
 *  the index entries until you find the an entry with a key greater
 *  than yours, or you reach the end.  The previous entry to that
 *  one will have the left node for you to follow, in the "lower"
 *  variable.  Again, this is a record number.  The idea is that
 *  you keep following the appropriate left branch until you hit
 *  a leaf node, then you do a sequential search.  Crikey, if
 *  you're a real loser you could even do a binary search of the
 *  leaf node.
 *
 */

/*
 *  The algorithm we employ is:
 *
 *  1. fetch the control record
 *
 *  2. Search the index nodes for our key until we reach a leaf
 *  node.
 *
 *  3. Search the leaf node for our key.
 */

long mxbtOneSearch(MXBT * mxbt, char *indexFile, void *searchKey, int (*compare) (void *testKey, void *searchKey, int len))
{
    mxbt->error = 0;
    mxbtInit(mxbt);
    if (!mxbt->error)
    {
        mxbtOpen(mxbt, indexFile);
        if (!mxbt->error)
        {
            mxbtFetchControl(mxbt);
            if (!mxbt->error)
            {
                mxbtSetKey(mxbt, searchKey);
                mxbtSetCompare(mxbt, compare);
                mxbtFindLeaf(mxbt);
                if (!mxbt->error)
                {
                    mxbtSearchLeaf(mxbt);
                }
            }
            mxbtClose(mxbt);
        }
    }
    if (mxbt->error)
    {
        return -1L;
    }
    else
    {
        return mxbt->value;
    }
}

static void mxbtInit(MXBT * mxbt)
{
    mxbt->buf = mxbt->myunion.intbuf;
    mxbt->index = (struct mxbt_indexrec *)mxbt->buf;
    mxbt->leaf = (struct mxbt_leafrec *)mxbt->buf;
}

static void mxbtOpen(MXBT * mxbt, char *indexFile)
{
    mxbt->fp = fopen(indexFile, "rb");
    if (mxbt->fp == NULL)
    {
        mxbt->error = 1;
    }
}

static void mxbtClose(MXBT * mxbt)
{
    if (fclose(mxbt->fp) != 0)
    {
        mxbt->error = 1;
    }
}

static void mxbtFetchControl(MXBT * mxbt)
{
    if (fread(&mxbt->recSize, sizeof(unsigned short), 1, mxbt->fp) != 1)
    {
        mxbt->error = 1;
    }
    else if (fread(&mxbt->control, sizeof mxbt->control, 1, mxbt->fp) != 1)
    {
        mxbt->error = 1;
    }
}

static void mxbtSetKey(MXBT * mxbt, void *searchKey)
{
    mxbt->searchK = searchKey;
}

static void mxbtSetCompare(MXBT * mxbt, int (*compare) (void *testKey, void *searchKey, int len))
{
    mxbt->compareF = compare;
}

static void mxbtReadRec(MXBT * mxbt)
{
    size_t x;
    int y;
    y = fseek(mxbt->fp, mxbt->recordNum * mxbt->recSize, SEEK_SET);
    if (y != 0)
    {
        mxbt->error = 1;
    }
    else
    {
        x = fread(mxbt->buf, mxbt->recSize, 1, mxbt->fp);
        if (x != 1)
        {
            mxbt->error = 1;
        }
    }
}

static void mxbtFindLeaf(MXBT * mxbt)
{
    int cnt, x;

    mxbt->recordNum = mxbt->control.indexStart;
    mxbtReadRec(mxbt);
    while (!mxbt->error && (mxbt->index->recType != -1))
    {
        cnt = mxbt->index->keyCount;
        if (cnt < 0)
        {
            mxbt->error = 1;
        }
        else
        {
            for (x = 0; x < cnt; x++)
            {
                if (mxbt->compareF((char *)mxbt->index +
                  mxbt->index->keys[x].offset, mxbt->searchK,
                  mxbt->index->keys[x].len) > 0)
                {
                    break;
                }
            }
            if (x == 0)
            {
                mxbt->recordNum = mxbt->index->recType;
            }
            else
            {
                mxbt->recordNum = mxbt->index->keys[x - 1].lower;
            }
            mxbtReadRec(mxbt);
        }
    }
}

static void mxbtSearchLeaf(MXBT * mxbt)
{
    int cnt, x, ret;

    cnt = mxbt->leaf->keyCount;
    if (cnt <= 0)
    {
        mxbt->error = 1;
    }
    else
    {
        for (x = 0; x < cnt; x++)
        {
            ret = mxbt->compareF((char *)mxbt->leaf +
              mxbt->leaf->keys[x].offset, mxbt->searchK,
              mxbt->leaf->keys[x].len);
            if (ret > 0)
            {
                mxbt->error = 1;
                break;
            }
            else if (ret == 0)
            {
                mxbt->value = mxbt->leaf->keys[x].value;
                break;
            }
        }
        if (x == cnt)
        {
            mxbt->error = 1;
        }
    }
}
/*
 *  STREXTRA.C
 *
 *  Written on 30-Jul-90 by jim nutt.  Changes on 10-Jul-94 by John Dennis,
 *  Paul Edwards and Andrew Clarke.  Released to the public domain.
 *
 *  A few string handling routines for Msged.
 */

int strncmpi(const char *s, const char *t, size_t x)
{
    long n;

    n = (long)x;

    while (n-- && tolower(*s) == tolower(*t))
    {
        if (*s == '\0')
        {
            /* equal */
            return 0;
        }
        s++;
        t++;
    }

    if (n < 0)
    {
        /* maximum hit, equal */
        return 0;
    }

    /* fell through, not equal */
    if (tolower(*s) > tolower(*t))
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

/*
 *  VSEVOPS.C
 *
 *  Written by Paul Edwards and released to the public domain.
 *
 *  Perform Version7 nodelist lookup operations.
 */

static int compareNode(void *a, void *b, int len)
{
    struct
    {
        short zone;
        short net;
        short node;
        short point;
    }
    v7addr;
    NETADDR *addrp;

    addrp = (NETADDR *) b;
    memcpy(&v7addr, a, len);
    if (len <= 6)
    {
        v7addr.point = 0;
    }
    if (v7addr.zone < addrp->zone)
    {
        return -1;
    }
    else if (v7addr.zone > addrp->zone)
    {
        return 1;
    }
    else if (v7addr.net < addrp->net)
    {
        return -1;
    }
    else if (v7addr.net > addrp->net)
    {
        return 1;
    }
    else if (v7addr.node < addrp->node)
    {
        return -1;
    }
    else if (v7addr.node > addrp->node)
    {
        return 1;
    }
    else if (v7addr.point < addrp->point)
    {
        return -1;
    }
    else if (v7addr.point > addrp->point)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/*
 *  The following stuff was written by John Dennis (with mods by Paul
 *  Edwards) and was also released to the public domain.
 */

int v7lookupnode(NETADDR * faddr, char * indexname)
{
    long record;
    MXBT mxbt;

    if(faddr->point != 0)
	faddr->point = 0;

    record = mxbtOneSearch(&mxbt, indexname, (void *)faddr, compareNode);

    if (record == -1)
    {
        return FALSE;
    }

    else 
    {
	return TRUE;
    }

}

