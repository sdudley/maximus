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
static char rcs_id[]="$Id: m_updsf.c,v 1.1.1.1 2002/10/01 17:52:51 sdudley Exp $";
#pragma on(unreferenced)

#error This file is no longer used by Maximus

/*# name=Module to update SCANFILE.DAT after reading a message
*/

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include "prog.h"
#include "max_msg.h"
#include "old_msg.h"

void Update_Scanfile(int mode,int msgnum,int scanfile)
{
  struct _omsg msg2;
  long scan_pos;
  int sf;

  /* Now update the bit in the SCANFILE.DAT in each area! */

  if (scanfile==-1)   /* If the scanfile isn't open already */
  {
    if ((sf=shopen(scanfile_name, O_RDWR | O_BINARY))==-1)
      return;
  }
  else                /* Otherwise preserve the current state */
  {
    sf=scanfile;
    scan_pos=tell(sf);
  }


  /* Skip over first message header */

  lseek(sf, (long)sizeof(struct _omsg), SEEK_SET);


  /* Now read through until we find the right message number */

  while (read(sf, (char *)&msg2, sizeof(struct _omsg))==sizeof(struct _omsg))
  {
    if (msg2.cost==msgnum)
    {
      /* Back up one, and write the message */

      if (mode==0)            /* Set Rec'd flag if mode==0 */
        msg2.attr |= MSGREAD;
      else if (mode==1)       /* Erase msg header if mode==1 */
      {
        *msg2.to='\0';
        *msg2.from='\0';
        *msg2.subj='\0';
      }

      lseek(sf, -(long)sizeof(struct _omsg), SEEK_CUR);
      write(sf, (char *)&msg2, sizeof(struct _omsg));
      break;
    }
  }

  if (scanfile==-1)     /* Now restore state -- Either close or seek. */
    close(sf);
  else lseek(sf, scan_pos, SEEK_SET);
}
