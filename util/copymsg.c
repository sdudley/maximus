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

#pragma off(unreferenced)
static char rcs_id[]="$Id: copymsg.c,v 1.3 2003/06/11 14:03:07 wesgarland Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include "dr.h"
#include <time.h>
#include "msgapi.h"
#include "old_msg.h"

#define TRUE 1
#define FALSE 0

static void *zalloc(size_t bytes)
{
    void * ret = malloc(bytes);
    if(!ret){
        printf("Could not satisfy %u byte memory request\n", bytes);
        exit(3);
        return NULL;
    }
    else return(ret);
}

static char *_ctime(union _stampu *st)
{
    static char str[40] = "12/31/89 00:00";

    if( !st->date.da )
        return("NULL");
    sprintf(str, "%02d/%02d/%02d %02d:%02d",
        st->date.mo, st->date.da, st->date.yr + 1980,
        st->time.hh, st->time.mm);
    return(str);
}

static struct _msg_attrib{
    unsigned attrib;
    char *name;
}msg_attrib[] = {
    {MSGPRIVATE, "Priv "},
    {MSGCRASH  , "Crash "},
    {MSGREAD   , "Rcvd "},
    {MSGSENT   , "Sent "},
    {MSGFILE   , "File "},
    {MSGFWD    , "FWD "},
    {MSGORPHAN , "ORPHAN "},
    {MSGKILL   , "Kill "},
    {MSGLOCAL  , "Local "},
    {MSGHOLD   , "HOLD "},
    {MSGXX2    , "XX2 "},
    {MSGFRQ    , "FREQ "},
    {MSGRRQ    , "RRQ "},
    {MSGCPT    , "CPT "},
    {MSGARQ    , "ARQ "},
    {MSGURQ    , "URQ "},
    {0         , NULL}
};

#define MAXSIZ 0x8000

int copymsg(FILE *outfile, char *fname)
{
    word i, size;
    char *buf;
    int col = 0;
    char lastword[100];
    int j = 0;
    int first = TRUE;
    struct _omsg *msg;
    FILE *inf;

    fixPathMove(fname); 
    inf = fopen(fname, "rb");
    if(!inf)
        return FALSE;

    buf = zalloc(MAXSIZ);
    size = fread(buf, 1, MAXSIZ, inf);
    fclose(inf);
    msg = (struct _omsg *)buf;

    *lastword = 0;
    i = 0;
    while( msg_attrib[i].name ){
        if( msg->attr & msg_attrib[i].attrib )
            strcat(lastword, msg_attrib[i].name );
        i++;
    }
    fprintf(outfile, "----------------------------------------------------------------------------\r\n");
    fprintf(outfile, "Msg : %04d %42s Prev: %04u  Next: %04u\r\n", atoi(fname), lastword, msg->reply, msg->up);
    sprintf(lastword, "%d/%d", msg->dest_net, msg->dest);
    fprintf(outfile, "To  : %-36s %9s  Written:%s\r\n", msg->to, lastword, _ctime(&msg->date_written));
    sprintf(lastword, "%d/%d", msg->orig_net, msg->orig);
    fprintf(outfile, "From: %-36s %9s  Arrived:%s\r\n", msg->from, lastword, _ctime(&msg->date_arrived));
    fprintf(outfile, "Subj: %s\r\n", msg->subj);

    /* format message : */
    for(i=sizeof(struct _omsg); i<size && !ferror(outfile); i++){
        if( buf[i] == '\r' ){
            lastword[j] = 0;
            if(*lastword !=1 && strncmp(lastword, "SEEN-BY:", sizeof("SEEN-BY:")-1)){
                fputs(lastword, outfile);
                fputs("\r\n", outfile);
            }
            else{
                for(;i<size && buf[i]!='\r'; i++); /* skip to end of line */
            }
            col = 0;
            j = 0;
            first = FALSE;
        }
        else if( (buf[i]&0x7f) !='\n' && (buf[i]&0x7f) !='\r' ){
            lastword[j++] = buf[i];
            if(buf[i]==' ' || j>79){
                lastword[j] = 0;
                if(first){
                    first = FALSE;  /* make sure there is at least one blank line after the subject. */
                    fputs("\r\n", outfile);
                }
                if(*lastword !=1 && strncmp(lastword, "SEEN-BY:", sizeof("SEEN-BY:")-1)){
                    fputs(lastword, outfile);
                }
                else{
                    for(;i<size && buf[i]!='\r'; i++); /* skip to end of line */
                    j = 0;  /*PLF Fri  09-15-1989  21:35:43 */
                }
                col += j;
                j = 0;
            }
            else if(col + j > 78){ /* wrap to next line */
                fputs("\r\n", outfile);
                col = 0;
            }
        }
    }
    if(ferror(outfile)){
        printf("error %s", _strerror(NULL));
        exit(1);    /*PLF Wed  10-02-1991  02:37:19 */
    }
    free(buf);
    return(!ferror(outfile));
}

#if 0
int cdecl main(int argc, char **argv)
{
    int i;
    FILE *outf = fopen("out", "ab");
    for(i=1; i<argc; i++)
        copymsg(outf, argv[i]);
}
#endif

