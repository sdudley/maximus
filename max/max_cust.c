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
static char rcs_id[]="$Id: max_cust.c,v 1.1.1.1 2002/10/01 17:51:35 sdudley Exp $";
#pragma on(unreferenced)

/*# name=Custom message/file-area listings
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include <string.h>
#include "alc.h"
#include "prog.h"
#include "max_msg.h"


#ifndef ORACLE

/* This function parses the "Format MsgHeader xxx"-type junk into          *
 * something that is displayable.                                          */

int ParseCustomMsgAreaList(PMAH pmah, char *div, char *parm, char *outparm, int first, int ch)
{
  MAH save_mah;
  HAREA ta;
  UMSGID uid;
  
  dword highmsg, lrptr=0L;
  
  char temp[PATHLEN];
  char *in, *out, *p, *s;
       
  static word cnt;
  int lrfile, hex_val;

  word cvtit, tempint;
  word lalign;
  word to_skip;
  word size;
  sword max, min;

  memset(&save_mah, 0, sizeof save_mah);

  if (first)
    cnt=0;

  for (in=parm, out=outparm, to_skip=0; *in; in++)
  {
    if (*in=='%')
    {
      max=min=-1;
      lalign=TRUE;

Again:

      switch (*++in)
      {
        case '-':
          lalign=TRUE;

          if (isdigit(*(in+1)))
          {
            in++;
            goto Next;
          }
          else goto Again;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':

          lalign=FALSE;
Next:
          min=atoi(in);

          while (isdigit(*in++))
            ;

          in--;

          if (*in=='.')
          {
            max=atoi(++in);

            while (isdigit(*in++))
              ;

            in--;
          }

          in--;
          goto Again;

        case 'x':
          sscanf(++in, "%2x", &hex_val);
          *out=(char)hex_val;
          *(out+1)='\0';

          if (in[1])
            in++;
          break;

        case '#':
          if (pmah)
            strcpy(out, PMAS(pmah, name));
          else
            strcpy(out, blank_str);
          break;

        case 'l':
        case 'p':
          if (pmah)
            strcpy(out, PMAS(pmah, path));
          else
            strcpy(out, blank_str);
          break;

        case 'n':
          if (pmah)
            strcpy(out, PMAS(pmah, descript));
          else
            strcpy(out, blank_str);
          break;

        case 'd':     /* Current division name */
          if (pmah)
          {
            strcpy(out, PMAS(pmah, name));
            if ((s=strrchr(out, '.'))!=NULL)
              *s='\0';
            break;
          }
          if (div)
            strcpy(out, div);
          else
            strcpy(out, blank_str);
          break;

        case 'a':     /* Name within division */
          if (pmah)
          {
            strcpy(out, PMAS(pmah, name));
            if (div && *div && (s=strrchr(out, '.'))!=NULL)
              strocpy(out, ++s);
            break;
          }
          strcpy(out, blank_str);
          break;

        case 't':
          if (pmah)
            strcpy(out, PMAS(pmah, echo_tag));
          else
            strcpy(out, blank_str);
          break;

        case 'f':
          if (pmah)
          {
            save_mah=mah;
            mah=*pmah;
          }

          /* Copy the filename out of the prompt string */
            
          for (s=temp; *++in > 32 && *in < 127 && s < temp+sizeof(temp)-1;)
            *s++=*in;
          
          *out=*s='\0';
          in--;

          Display_File(0, NULL, temp);

          if (pmah)
            mah=save_mah;
          break;

        case '*':
          if (ch=='@')        /* This area is tagged */
            strcpy(out, "@");
          else if (ch==' ')   /* This area is NOT tagged */
            strcpy(out, " ");
          else if (pmah && (pmah->ma.attribs & (MA_DIVBEGIN | MA_DIVEND)))
            strcpy(out, " ");
          else if (pmah)      /* Check if area has new mail */
          {
            if (pmah->ma.type & MSGTYPE_SDM)
            {
              sprintf(temp,
                      usr.lastread_ptr ? ps_lastread : ps_lastread_single,
                      PMAS(pmah, path));
            
              size=sizeof(word);
            }
            else
            {
              sprintf(temp, sq_lastread, PMAS(pmah, path));
              size=sizeof(UMSGID);
            }

            if ((lrfile=shopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT))==-1)
            {
              tempint=0;
              uid=0L;
            }
            else
            {
              lseek(lrfile, (long)usr.lastread_ptr*(long)size, SEEK_SET);
              
              /* Check to see if we can read the LR.  If not, set it        *
               * to zero by default.                                        */

              if (read(lrfile, (pmah->ma.type & MSGTYPE_SDM)
                         ? (char *)&tempint : (char *)&uid,
                       size) != (signed)size)
              { 
                tempint=0;
                uid=0;
              }

              close(lrfile);
            }

            if (pmah->ma.type & MSGTYPE_SDM)
              uid=(UMSGID)tempint;

            cvtit=TRUE;

            /* Get current bit from .area */

            if (mah.heap && eqstri(PMAS(pmah, name), MAS(mah, name)))
            {
              lrptr=last_msg;
              cvtit=FALSE;    /* don't attempt to convert UID to msgn */
            }

            highmsg=0L;

            if (pmah->ma.type & MSGTYPE_SDM)
            {
              lrptr=uid;

              sprintf(temp,"%s%ld.msg", PMAS(pmah, path), (long)(lrptr+1));

              if (fexist(temp))
                highmsg=(long)lrptr+1L;
            }
            else /* MSGTYPE_SQUISH */
            {
              if (mah.heap && eqstr(PMAS(pmah, name), MAS(mah, name)))
                highmsg=sq ? MsgHighMsg(sq) : 0L;
              else
              {
                lrptr=0L;

                if ((ta=MaxOpenArea(pmah)) != NULL)
                {
                  highmsg=MsgHighMsg(ta);

                  if (cvtit)
                    lrptr=MsgUidToMsgn(ta, uid, UID_PREV);

                  MsgCloseArea(ta);
                }
              }
            }

            if (lrptr < highmsg)
              strcpy(out, "*");
            else strcpy(out, " ");
          }
          break;
        
        
        case 'c':
          if ((sword)++cnt < min)
          {
            if (max==-1)
              to_skip=1;
            else to_skip=(word)max;
          }
          else
            cnt=0;
          
          max=min=0;
          break;

        default:
          logit(inv_ccmd, *in);
          break;
      }
      
      if (*in != 'c')
      {
        if (to_skip)
        {
          *out='\0';
          to_skip--;
        }
      }

      if (max != -1)
      {
        if ((sword)strlen(out) > max)
          out[max]='\0';
      }

      if (!lalign)
      {
        p=out;

        while ((sword)strlen(out) < min)
        {
          strocpy(p+1, p);

          *p++=' ';
        }
      }
      else if (min != -1)
      {
        while ((sword)strlen(out) < min)
          strcat(out," ");
      }

      out += strlen(out);
    }
    else *out++=*in;
  }

  *out='\0';

  return (strlen(outparm));
}


/* This function parses the "Format MsgHeader xxx"-type junk into          *
 * something that is displayable.                                          */

int ParseCustomFileAreaList(PFAH pfah, char *div, char *parm, char *outparm, int first)
{
  FAH save_fah;
  
  char temp[PATHLEN];
  char *in, *out, *p, *s;
       
  static word cnt;

  word hex_val;
  word lalign;
  word to_skip;
  sword max, min;
    
  if (first)
    cnt=0;

  for (in=parm,out=outparm,to_skip=0;*in;in++)
  {
    if (*in=='%')
    {
      max=min=-1;
      lalign=TRUE;

Again:

      switch (*++in)
      {
        case '-':
          lalign=TRUE;

          if (isdigit(*(in+1)))
          {
            in++;
            goto Next;
          }
          else goto Again;

        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '0':

          lalign=FALSE;
Next:
          min=atoi(in);

          while (isdigit(*in++))
            ;

          in--;

          if (*in=='.')
          {
            max=atoi(++in);

            while (isdigit(*in++))
              ;

            in--;
          }

          in--;
          goto Again;

        case 'x':
          sscanf(++in, "%2x", &hex_val);
          *out=(char)hex_val;
          *(out+1)='\0';

          if (in[1])
            in++;
          break;

        case '#':
          if (pfah)
            strcpy(out, PFAS(pfah, name));
          else
            strcpy(out, blank_str);
          break;

        case 'l':
        case 'p':
          if (pfah)
            strcpy(out, PFAS(pfah, downpath));
          else
            strcpy(out, blank_str);
          break;

        case 'n':     /* Area description */
          if (pfah)
            strcpy(out, PFAS(pfah, descript));
          else
            strcpy(out, blank_str);
          break;

        case 'd':     /* Current division name */
          if (pfah)
          {
            strcpy(out, PFAS(pfah, name));
            if ((s=strrchr(out, '.'))!=NULL)
              *s='\0';
            break;
          }
          if (div)
            strcpy(out, div);
          else
            strcpy(out, blank_str);
          break;

        case 'a':     /* Name within division */
          if (pfah)
          {
            strcpy(out, PFAS(pfah, name));
            if (div && *div && (s=strrchr(out, '.'))!=NULL)
              strocpy(out, ++s);
            break;
          }
          strcpy(out, blank_str);
          break;

        case 'f':
          if (pfah)
          {
            save_fah=fah;
            fah=*pfah;
          }

          /* Copy the filename out of the prompt string */
            
          for (s=temp; *++in > 32 && *in < 127 && s < temp+sizeof(temp)-1;)
            *s++=*in;
          
          *out=*s='\0';
          in--;

          Display_File(0, NULL, temp);

          if (pfah)
            fah=save_fah;
          break;

        case 'c':
          if ((sword)++cnt < min)
          {
            if (max==-1)
              to_skip=1;
            else to_skip=(word)max;
          }
          else
            cnt=0;
          
          max=min=0;
          break;

        default:
          logit(inv_ccmd, *in);
          break;
      }
      
      if (*in != 'c')
      {
        if (to_skip)
        {
          *out='\0';
          to_skip--;
        }
      }

      if (max != -1)
      {
        if ((sword)strlen(out) > max)
          out[max]='\0';
      }

      if (!lalign)
      {
        p=out;

        while ((sword)strlen(out) < min)
        {
          strocpy(p+1, p);

          *p++=' ';
        }
      }
      else if (min != -1)
      {
        while ((sword)strlen(out) < min)
          strcat(out, " ");
      }

      out += strlen(out);
    }
    else *out++=*in;
  }

  *out='\0';

  return (strlen(outparm));
}

#endif

