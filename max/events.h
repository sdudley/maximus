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

#ifndef __EVENTS_H_DEFINED
#define __EVENTS_H_DEFINED

  #define EFLAG_DONE  0x01
  #define EFLAG_YELL  0x02
  #define EFLAG_ERLVL 0x04

  #define DAY_SUN     0x01
  #define DAY_MON     0x02
  #define DAY_TUE     0x04
  #define DAY_WED     0x08
  #define DAY_THU     0x10
  #define DAY_FRI     0x20
  #define DAY_SAT     0x40

  struct _etime
  {
    word hour;
    word min;
  };

  /* Structure of an event, as it is in memory */

  struct _event
  {
    struct _event *next;    /* Next event in linked list                    */

    byte day;               /* Day of week -- see the DAY_XXX define        */
    byte fill1;             /* Reserved by Maximus                          */
    
    word flags;             /* Event flags -- see EFLAGS_XXX, above.        */

    word erl;               /* Errorlevel to use for this event             */
    
    word data1;             /* Optional data -- currently used by Yell evts */
    word data2;
    word data3;

    struct _etime start;    /* Start time of event                          */
    struct _etime end;      /* End time of event                            */

    word eventnum;          /* Number of this event                         */
    char tune[32];          /* Tune to play during this event               */
  };

  /* Structure of an event, as it is on disk */

  struct _diskevent
  {
    byte day;               /* Day of week -- see the DAY_XXX define        */
    byte fill1;             /* Reserved by Maximus                          */
    
    word flags;             /* Event flags -- see EFLAGS_XXX, above.        */

    word erl;               /* Errorlevel to use for this event             */
    
    word data1;             /* Optional data -- currently used by Yell evts */
    word data2;
    word data3;

    struct _etime start;    /* Start time of event                          */
    struct _etime end;      /* End time of event                            */

    word eventnum;          /* Number of this event                         */
    char tune[32];          /* Tune to play during this event               */
  };

void Read_Events(void);
int GetEvent(word eflag, int not_eflag, struct _event *out, int now);
void Write_One_Event(struct _event *e);
void Event_To_Dos_Date(struct _etime *et, union stamp_combo *sc, union stamp_combo *today);
void Read_Event_File(int e_num);


#endif /* __EVENTS_H_DEFINED */


