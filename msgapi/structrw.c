#include <stdlib.h>
#include <string.h>
#include <assert.h>   
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>   
#include <errno.h>
#include <unistd.h>
#include "msgapi.h"
#include "typedefs.h"

/* Bo: this file is stolen from SMAPI (http://husky.sf.net)
 * And it ensures that the XMSG structure is written correctly,
 * which it's not with write()
 */

/* Wes: putword.c and putword.h are pulled from xmsgapi.
 * Will figure out how to integrate this nicely later.
 */

#undef __LITTLE_ENDIAN__
#undef __BIG_ENDIAN__

#if defined(BIG_ENDIAN)
# define __BIG_ENDIAN__
#elif defined(LITTLE_ENDIAN)
# define __LITTLE_ENDIAN__
#endif

#include "putword.h"
#include "putword.c"

int read_xmsg(int handle, XMSG *pxmsg)
{
    byte buf[XMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;
    int i;

    if (farread(handle, (byte far *)buf, XMSG_SIZE) != XMSG_SIZE)
    {
        return 0;
    }

                                /* 04 bytes "attr" */
    pxmsg->attr = get_dword(pbuf);
    pbuf += 4;
                                /* 36 bytes "from" */
    memmove(pxmsg->from, pbuf, XMSG_FROM_SIZE);
    pbuf += XMSG_FROM_SIZE;

                                /* 36 bytes "to"   */
    memmove(pxmsg->to, pbuf, XMSG_TO_SIZE);
    pbuf += XMSG_TO_SIZE;

                                /* 72 bytes "subj" */
    memmove(pxmsg->subj, pbuf, XMSG_SUBJ_SIZE);
    pbuf += XMSG_SUBJ_SIZE;

                                /* 8 bytes "orig"  */
                                /* 8 bytes "orig"  */
    pxmsg->orig.zone = get_word(pbuf); pbuf += 2;
    pxmsg->orig.net  = get_word(pbuf); pbuf += 2;
    pxmsg->orig.node = get_word(pbuf); pbuf += 2;
    pxmsg->orig.point= get_word(pbuf); pbuf += 2;

                                /* 8 bytes "dest"  */
    pxmsg->dest.zone = get_word(pbuf); pbuf += 2;
    pxmsg->dest.net  = get_word(pbuf); pbuf += 2;
    pxmsg->dest.node = get_word(pbuf); pbuf += 2;
    pxmsg->dest.point= get_word(pbuf); pbuf += 2;

                               /* 4 bytes "date_written" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pxmsg->date_written.date.da = rawdate & 31;
    pxmsg->date_written.date.mo = (rawdate >> 5) & 15;
    pxmsg->date_written.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_written.time.ss = rawtime & 31;
    pxmsg->date_written.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_written.time.hh = (rawtime >> 11) & 31;

                                /* 4 bytes "date_arrived" */
    rawdate = get_word(pbuf); pbuf += 2;
    rawtime = get_word(pbuf); pbuf += 2;
    pxmsg->date_arrived.date.da = rawdate & 31;
    pxmsg->date_arrived.date.mo = (rawdate >> 5) & 15;

    pxmsg->date_arrived.date.yr = (rawdate >> 9) & 127;
    pxmsg->date_arrived.time.ss = rawtime & 31;
    pxmsg->date_arrived.time.mm = (rawtime >> 5) & 63;
    pxmsg->date_arrived.time.hh = (rawtime >> 11) & 31;

                                /* 2 byte "utc_ofs" */
    pxmsg->utc_ofs = get_word(pbuf);
    pbuf += 2;

                                /* 4 bytes "replyto" */
    pxmsg->replyto = get_dword(pbuf);
    pbuf += 4;
                                /* 10 times 4 bytes "replies" */
    for (i = 0; i < MAX_REPLY; i++)
    {
        pxmsg->replies[i] = get_dword(pbuf);
        pbuf += 4;
    }

                                /* 4 bytes "umsgid" */
    pxmsg->umsgid = get_dword(pbuf);
    pbuf += 4;

                                /* 20 times FTSC date stamp */
    memmove(pxmsg->__ftsc_date, pbuf, 20);
    pbuf += 20;

    assert(pbuf - buf == XMSG_SIZE);
    return 1;
}

int write_xmsg(int handle, XMSG *pxmsg)
{
    byte buf[XMSG_SIZE], *pbuf = buf;
    word rawdate, rawtime;
    int i;

                                /* 04 bytes "attr" */
    put_dword(pbuf, pxmsg->attr);
    pbuf += 4;

                                /* 36 bytes "from" */
    memmove(pbuf, pxmsg->from, XMSG_FROM_SIZE);
    pbuf += XMSG_FROM_SIZE;

                                /* 36 bytes "to"   */
    memmove(pbuf, pxmsg->to, XMSG_TO_SIZE);
    pbuf += XMSG_TO_SIZE;

                                /* 72 bytes "subj" */
    memmove(pbuf, pxmsg->subj, XMSG_SUBJ_SIZE);
    pbuf += XMSG_SUBJ_SIZE;

                                /* 8 bytes "orig"  */
    put_word(pbuf, pxmsg->orig.zone);  pbuf += 2;
    put_word(pbuf, pxmsg->orig.net);   pbuf += 2;
    put_word(pbuf, pxmsg->orig.node);  pbuf += 2;
    put_word(pbuf, pxmsg->orig.point); pbuf += 2;

                                    /* 8 bytes "dest"  */
    put_word(pbuf, pxmsg->dest.zone);  pbuf += 2;
    put_word(pbuf, pxmsg->dest.net);   pbuf += 2;
    put_word(pbuf, pxmsg->dest.node);  pbuf += 2;
    put_word(pbuf, pxmsg->dest.point); pbuf += 2;


                                /* 4 bytes "date_written" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pxmsg->date_written.date.da) & 31);
    rawdate |= (((word)pxmsg->date_written.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_written.date.yr) & 127) << 9;

    rawtime |= (((word)pxmsg->date_written.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_written.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_written.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;


                                /* 4 bytes "date_arrvied" */
    rawdate = rawtime = 0;

    rawdate |= (((word)pxmsg->date_arrived.date.da) & 31);
    rawdate |= (((word)pxmsg->date_arrived.date.mo) & 15) << 5;
    rawdate |= (((word)pxmsg->date_arrived.date.yr) & 127) << 9;

    rawtime |= (((word)pxmsg->date_arrived.time.ss) & 31);
    rawtime |= (((word)pxmsg->date_arrived.time.mm) & 63) << 5;
    rawtime |= (((word)pxmsg->date_arrived.time.hh) & 31) << 11;

    put_word(pbuf, rawdate); pbuf += 2;
    put_word(pbuf, rawtime); pbuf += 2;
                                /* 2 byte "utc_ofs" */
    put_word(pbuf, pxmsg->utc_ofs);
    pbuf += 2;

                                /* 4 bytes "replyto" */
    put_dword(pbuf, pxmsg->replyto);
    pbuf += 4;

                                /* 10 times 4 bytes "replies" */
    for (i = 0; i < MAX_REPLY; i++)
    {
        put_dword(pbuf, pxmsg->replies[i]);
        pbuf += 4;
    }
                                /* 4 bytes "umsgid" */
    put_dword(pbuf, pxmsg->umsgid);
    pbuf += 4;


                                /* 20 times FTSC date stamp */
    memmove(pbuf, pxmsg->__ftsc_date, 20);
    pbuf += 20;

    assert(pbuf - buf == XMSG_SIZE);
    return (farwrite(handle, (byte far *)buf, XMSG_SIZE) == XMSG_SIZE);
}

