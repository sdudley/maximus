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
static char rcs_id[]="$Id: m_save.c,v 1.2 2003/06/04 23:46:21 wesgarland Exp $";
#pragma on(unreferenced)

/*# name=Message Section: Message saving routines
*/

#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "max_edit.h"
#include "m_for.h"
#include "m_save.h"
#ifdef UNIX
# include <errno.h>
#endif

static void near SaveMsgFromUpfile(HMSG msgh,FILE *upfile, long total_len,int local_msg, PMAH pmah);
static void near SaveMsgFromEditor(HMSG msgh, long total_len, PMAH pmah);

int SaveMsg(XMSG *msg, FILE *upfile, int local_msg,
            long msgnum, int chg, PMAH pmah, char *msgarea,
            HAREA marea, char *ctrl_buf, char *orig_area,
            int fSkipCC)
{
  HMSG mh;
  
  UMSGID lastuid;

  long total_len, save_to, kludge_len;
  
  int ret, cnt, tlen;
  int fResetFile;
  word found_tear;
  word line;

  char temp[PATHLEN];
  char orig[MAX_OTEAR_LEN];
  char *kludge;

  ret=FALSE;

  /* Tell the user what we're doing... */
  
  Printf(savedmsg1);
  display_line=display_col=1;

  /* Open the message handle */
  
  lastuid=MsgMsgnToUid(sq, last_msg);

  /* Try to open the message up to 10 times */

  for (cnt=10; cnt--; )
    if ((mh=MsgOpenMsg(marea, MOPEN_CREATE, msgnum ? msgnum : 0L)) != NULL)
      break;
    else Giveaway_Slice();

  if (!mh)
  {
    logit("!Can't write message (msgapierr=%d)", msgapierr);
    ret=WriteErr(TRUE);
    goto Done;
  }
  
  /* Now determine how much space we need to write the message... */
  
  if (upfile)  /* This is easy -- take size of input file, and add */
  {            /* a couple of bytes.                               */

    fseek(upfile, 0L, SEEK_END);
    total_len=ftell(upfile)+512;
  }
  else
  {
    total_len=0L;
    found_tear=0;
    
    for (line=1; line <= num_lines; line++)
    {
      /* See if it's a hard or soft carriage return, and output the     *
       * correct text.                                                  */

      Check_For_Origin(&found_tear, screen[line]+1);

      total_len += strlen(screen[line]+1);

      if (screen[line][0]==HARD_CR)
        total_len++;
    }


    if (found_tear != 2 && (pmah->ma.attribs & MA_ECHO))
    {
      GenerateOriginLine(orig, pmah);
      total_len += strlen(orig);
    }
    
    total_len += 10;
  }

  kludge=GenerateMessageKludges(msg, pmah, ctrl_buf);

  if (!kludge)
    kludge_len=0;
  else
  {
    /* Allow some more room in ctrl header (for efficiency) */

    kludge_len=strlen(kludge)+1;

    if (!chg && (msg->attr & MSGFILE) && (pmah->ma.attribs & MA_ATTACH))
      kludge_len += PATHLEN;
  }

  if (!orig_area || !*orig_area)
    tlen=0;
  else
    total_len +=(tlen=sprintf(temp, replying_to_area, orig_area));

  fResetFile = FALSE;

  /* If we are leaving a local attach, don't set the attach bit in
   * the physical base until we have updated the message with the
   * appropriate info.  (Security in case a user hangs up after
   * entering the msg but before entering the attach info.)
   */

  if (!chg && (msg->attr & MSGFILE) && (pmah->ma.attribs & MA_ATTACH) &&
      AllowAttribute(pmah, MSGKEY_LATTACH))
  {
    msg->attr &= ~MSGFILE;
    fResetFile = TRUE;
  }

  if (MsgWriteMsg(mh, FALSE, msg, temp, tlen, total_len, kludge_len, kludge) != 0)
  {
    if (kludge)
      free(kludge);

    ret=WriteErr(FALSE);
    goto Done;
  }

  if (fResetFile)
    msg->attr |= MSGFILE;

  found_tear=0;

  /* Now we write the actual text (contained in the array of arrays    *
   * screen[][]).  Make sure to differentiate between hard and soft    *
   * carriage returns!  Also, if the upfile parameter is NULL, then it *
   * means that we're writing the message from our internal fs or line *
   * editor.  If it is non-null, then it is a pointer to a file        *
   * (probably an uploaded or locally-entered message) that we should  *
   * use instead.                                                      */

  if (upfile)
    SaveMsgFromUpfile(mh, upfile, total_len, local_msg, pmah);
  else SaveMsgFromEditor(mh, total_len, pmah);
  
  /* And finish it off... */

  MsgCloseMsg(mh);
  mh=NULL;

  save_to=msgnum ? msgnum : MsgHighMsg(sq);
  if (orig_area && *orig_area)
    Printf(savedmsg3, PMAS(pmah,name), UIDnum(save_to));
  else
    Printf(savedmsg2, UIDnum(save_to));

  /* Place entries in log file, and "all that jazz" */

  CleanupAfterSave(chg, msg, save_to, pmah, msgarea, kludge, marea);

  if (kludge)
    free(kludge);

  if (!chg && (msg->attr & MSGFILE) && (pmah->ma.attribs & MA_ATTACH) &&
      AllowAttribute(pmah, MSGKEY_LATTACH))
  {
    Msg_AttachUpload(pmah, sq, msg, MsgMsgnToUid(sq, save_to));
  }

  if (!fSkipCC)
    Handle_Carbon_Copies(save_to, upfile, msg);

  /* If this is a local attach, then receive the file(s) now */

Done:

  if (mh)
    MsgCloseMsg(mh);
    
  last_msg=MsgUidToMsgn(sq, lastuid, UID_PREV);

  /* Free the message text, if we were using the internal editors */

  if (upfile==NULL) 
    Free_All();

  return ret;
}



static int near WriteErr(int opened)
{
  logit("!Msg%sMsg rc=%d, errno=%d", opened ? "Open" : "Write",
        msgapierr, errno);

  Printf(errwriting);
  return TRUE;
}



static void near SaveMsgFromUpfile(HMSG msgh,FILE *upfile, long total_len,int local_msg, PMAH pmah)
{
  char orig[MAX_OTEAR_LEN];
  char temp[PATHLEN*2+5];
  byte *s;
  
  int first=TRUE;
  int line_len=0;
  word found_tear=0;
  int last_char=0;
  int last_len=0;
  int last_isblstr=FALSE;
  int was_trimmed=FALSE;
  int is_trimmed;
  unsigned char ch=HARD_CR;
  char c;
    
  fseek(upfile, 0L, SEEK_SET);
  
  while (fbgetsc(temp, PATHLEN, upfile, &is_trimmed))
  {
    size_t this_linelen;

    /* Make sure the message isn't too long */

    if (!local_msg && ftell(upfile) > UL_TRUNC)
      break;

    /* Strip out any control characters */

    for (s=temp; *s; s++)
    {
      /* Expand tabs */

      if (*s=='\t')
      {
        size_t add=8-((((char *)s-(char *)temp)+line_len)%8);

        if (((char *)s-(char *)temp)+add >= PATHLEN-5)
          *s=' ';
        else
        {
          strocpy(s+add, s+1);
          memset(s, ' ', add);
          s += add;
        }
      }
      else if (*s < ' ')
        *s=' ';
      else if (*s == 0x8d && (prm.charset & CHARSET_CHINESE)==0)
      {
        if (s[1] && !isspace(s[1]) &&
            ((((char *)s > (char *)temp) && s[-1]!=' ') || (((char *)s== (char *)temp) && !first && last_char!=' ')))
          *s=' '; /* Replace with space if required */
        else
        {         /* Otherwise just eliminate it */
          strocpy(s,s+1);
          --s;
        }
      }
      else
        *s=((mah.ma.attribs & MA_HIBIT) ? *s : nohibit[(unsigned char)*s]);
    }
    
    Check_For_Origin(&found_tear, temp);

    /* Now insert the hard/soft CR for the previous line, based on   *
     * this line's attributes...                                     */

    if (first)
      first=FALSE;
    else if (was_trimmed)      /* Ensures that long lines are left alone */
    {
      /* Shift it over one space, so we can put in the CR (or nothing) */
      
      strocpy(temp+1,temp);

      /* If last line had a trailing space, or if the first char     *
       * on this line is a letter.                                   */

      if (temp[1] > 32 && !last_isblstr && last_len >= 60)
      {
        if (! is_wd((char)last_char)) /* Make sure wrapped line has space */
          *temp=' ';
        else strocpy(temp, temp+1); /* shift back one space */

        ch=SOFT_CR;
      }
      else
      {
        *temp=HARD_CR;
        ch=HARD_CR;
      }
    }

    this_linelen=strlen(temp);

    MsgWriteMsg(msgh, TRUE, NULL, temp, this_linelen, total_len, 0L, NULL);

    if (*temp)
      last_char=temp[this_linelen-1];
    else last_char='\0';

    last_len=this_linelen;
    last_isblstr=isblstr(temp);
    if ((was_trimmed=is_trimmed)==0)
      line_len=0;
    else
      line_len+=this_linelen;
  }
  

  /* If that last CR was a soft CR, add another one */

  if (ch==SOFT_CR)
    strcpy(temp, "\r");
  else *temp='\0';


  
  /* Add the final CR */

  strcat(temp, "\r");
  MsgWriteMsg(msgh, TRUE, NULL, temp, strlen(temp), total_len, 0L, NULL);


  
  /* Add the origin line */

  if (found_tear != 2 && (pmah->ma.attribs & MA_ECHO))
  {
    GenerateOriginLine(orig, pmah);
    MsgWriteMsg(msgh, TRUE, NULL, orig, strlen(orig), total_len, 0L, NULL);
  }
  
  /* Stick on the trailing NUL */
  
  c='\0';
  MsgWriteMsg(msgh, TRUE, NULL, &c, sizeof(char), total_len, 0L, NULL);

  
  /* If we truncated a msg, say this to the user */

  if (upfile && !local_msg && ftell(upfile) >= UL_TRUNC)
    Puts(msg_tlong);
}


static void near SaveMsgFromEditor(HMSG msgh, long total_len, PMAH pmah)
{
  word found_tear;
  word line;

  char orig[MAX_OTEAR_LEN];
  char c;
  
  for (line=1; line <= num_lines; line++)
  {
    /* See if it's a hard or soft carriage return, and output the     *
     * correct text.                                                  */

    Check_For_Origin(&found_tear, screen[line]+1);

    /* Append a hard carriage return, if desired */
    
    if (screen[line][0]==HARD_CR)
      strcat(screen[line]+1, "\r");
    
    MsgWriteMsg(msgh, TRUE, NULL, screen[line]+1,
                strlen(screen[line]+1), total_len, 0L, NULL);
  }

  if (num_lines && screen[num_lines] && screen[num_lines][0]==SOFT_CR)
  {
    c='\r';

    MsgWriteMsg(msgh, TRUE, NULL, &c, (long)sizeof(char),
                total_len, 0L, NULL);
  }
  
  /* Add the origin line */

  if (found_tear != 2 && (pmah->ma.attribs & MA_ECHO))
  {
    GenerateOriginLine(orig, pmah);
    MsgWriteMsg(msgh, TRUE, NULL, orig, strlen(orig), total_len, 0L, NULL);
  }
  
  /* Stick on the trailing NUL */
  
  c='\0';
  MsgWriteMsg(msgh, TRUE, NULL, &c, sizeof(char), total_len, 0L, NULL);
}
 

static void near Check_For_Origin(word *found_tear, char *temp)
{
  if (*found_tear==0 && strncmp(temp, "---", 3)==0)
    *found_tear=1;
  else if (*found_tear==1 && strncmp(temp, " * Origin:", 10)==0)
    *found_tear=2;
  else if (*found_tear != 2)
    *found_tear=0;
}

  
static void near CleanupAfterSave(int chg, XMSG *msg, long save_to,
                                  PMAH pmah, char *msgarea, char *kludge,
                                  HAREA ha)
{
  char temp[PATHLEN];
  
  if (!eqstri(usrname, msg->from))
    logit(log_msgfrom, usr.name, msg->from);

  if (pmah->ma.attribs & MA_NET)
    sprintf(temp," (%d:%d/%d.%d)", msg->dest.zone, msg->dest.net,
            msg->dest.node, msg->dest.point);
  else *temp='\0';

  logit(chg ? chgdmsg : msgto,
        msg->to,
        temp,
        msgarea,
        save_to);

  Puts("\n" CLEOL);

  WroteMessage(pmah, msg, kludge, ha, chg);
}
  









static void near Handle_Carbon_Copies(long msgn, FILE *upfile, XMSG *msg)
{
  char temp[PATHLEN];
  word line;
  int first;
  UMSGID uid=MsgMsgnToUid(sq, msgn);

  first=TRUE;

  if (upfile)
  {
    fseek(upfile, 0L, SEEK_SET);

    while (fbgets(temp, PATHLEN, upfile) != NULL)
    {
      Strip_Trailing(temp,'\n');

      if (! ProcessCC(uid, temp, msg, first))
        break;

      first=FALSE;
    }

    return;
  }

  for (line=1; line <= num_lines; line++)
  {
    Strip_Trailing(screen[line]+1,'\r');

    if (! ProcessCC(uid, screen[line]+1, msg, first))
    {
      break;
    }

    first=FALSE;
  }
}


static int near ProcessCC(UMSGID uid, char *line, XMSG *msg, int first)
{
  NETADDR *d;
  struct _fwdp *fp;
  char name[PATHLEN];

  #define ADDR_LEN 30
  char addr[ADDR_LEN];

  char *p, *o;
  int gotone=FALSE;

  if ((fp=malloc(sizeof(struct _fwdp)))==NULL)
    return FALSE;

  memset(fp, 0, sizeof *fp);

  if ( (first && eqstrni(line, "cc:", 3)) ||
      (!first && *(byte *)line >= 32 && *(byte *)line < 127))
  {
    if (toupper(*line)=='C')
      p=line+3;
    else p=line;

    do
    {
      o=name;


      /* Skip over any leading spaces */

      while (*p==' ' || *p==',' || *p==';')
        p++;

      if (*p=='\0')
        break;

      /* Now copy the user's name */

      while (*p &&
             (!isdigit(*p) || (isdigit(*p) && p[-1] != ' ')) &&
             *p != ',' && *p != ';' && (*p != '.' || p[-1] != ' '))
      {
        *o++=*p++;
      }

      *o='\0';

      Strip_Trailing(name,' ');


      /* If there's a destination address... */

      if (isdigit(*p) || *p=='.')
      {
        o=addr;

        while (o < addr+ADDR_LEN && (isdigit(*p) || *p==':' ||
                                     *p=='/' || *p=='.'))
        {
          *o++=*p++;
        }

        *o='\0';
      }
      else *addr='\0';

      fp->tosq=sq;
      fp->fh=NULL;
      CopyMsgArea(&fp->toar, &mah);
      fp->fmsg=*msg;
      fp->tmsg=*msg;
      *fp->toname='\0';
      fp->msgnum=MsgUidToMsgn(sq, uid, UID_EXACT);
      fp->bomb=FALSE;
      fp->kill=FALSE;

      strnncpy(fp->tmsg.to, cfancy_str(name), sizeof(fp->tmsg.to)-1);

      fp->tmsg.attr |= MSGKILL; /* Delete after packing */



      d=&fp->tmsg.dest;

      /* Do a FIDOUSER.LST lookup, if we're in a matrix area */

      if (*addr=='\0' && (fp->toar.ma.attribs & MA_NET))
        Get_FidoList_Name(&fp->tmsg, addr, PRM(fidouser));

      /* Convert address to binary */

      MaxParseNN(addr, d);


      /* And cc the message... */

      Msg_Forward(fp);
      
      gotone=TRUE;
    }
    while (*p && *name);
  }

  free(fp);
  return (gotone);
}

