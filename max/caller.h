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

/*
**  caller.h (FD Mailer)
**
**  Structures for CALLER.nnn
**
**  Copyright 1991 Joaquim H. Homrighausen. All rights reserved.
**
**  Last revised: 91-12-14                       FrontDoor 2.03+
**
**  THIS FILE MAY NOT BE DISTRIBUTED OR USED WITHOUT PRIOR WRITTEN
**  PERMISSION FROM JOAQUIM H. HOMRIGHAUSEN. THIS FILE CONTAINS AND
**  DEFINES UNPUBLISHED INFORMATION. IT WILL BE PUBLISHED WITH THE
**  RELEASE OF THE NEXT VERSION OF FRONTDOOR.
*/

#define CREC_VER 1

struct _crec
    {
    unsigned int    iversion;                         /*Interface version==1*/
    unsigned long   timestamp,                    /*UNIX style date and time*/
                    portrate,                    /*DTE speed, 300-115200 BPS*/
                    callrate;                    /*DCE speed, 300-115200 BPS*/
    unsigned int    comport,                      /*0-65535, 0=Local, 1=COM1*/
                    timeleft;                       /*Number of minutes left*/
    char            resultstr[255],            /*Modem result string, ASCIIZ*/
                    callerID[255];                       /*Caller ID, ASCIIZ*/
    char            __rsrvd[512];                               /*Future use*/
    };


/* end of file "caller.h" */

