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
static char rcs_id[]="$Id: events.c,v 1.1.1.1 2002/10/01 17:50:55 sdudley Exp $";
#pragma on(unreferenced)

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "prog.h"
#include "mm.h"
#include "events.h"

static struct _event *elist;
static unsigned num_event;

static char event_name[]="%sevents%02x%s";
static union stamp_combo done_date;


/* ENTRYPOINT: Convert an event-style date into a DOS-format date */

void Event_To_Dos_Date(struct _etime *et, union stamp_combo *sc, union stamp_combo *today)
{
  sc->msg_st.date=today->msg_st.date;
  sc->msg_st.time.hh=et->hour;
  sc->msg_st.time.mm=et->min;
  sc->msg_st.time.ss=0;
}


/* Free all of the elements in the event list */

void EventListFree(void)
{
  struct _event *e, *enext;

  num_event=0;

  for (e=elist; e; enext=e->next, free(e), e=enext)
    ;

  elist=NULL;
}



/* Add the specified event (in disk format) to our memory linked list */

static void near EventAddList(struct _diskevent *de)
{
  struct _event *e, *l;

  if ((e=malloc(sizeof(struct _event)))==NULL)
    return;

  e->next=NULL;
  e->day=de->day;
  e->fill1=de->fill1;
  e->flags=de->flags;
  e->erl=de->erl;
  e->data1=de->data1;
  e->data2=de->data2;
  e->data3=de->data3;
  e->start=de->start;
  e->end=de->end;
  e->eventnum=num_event++;
  strcpy(e->tune, de->tune);

  /* Now insert at end of list */

  for (l=elist; l && l->next; l=l->next)
    ;

  /* Now link at the appropriate spot */

  if (l)
    l->next=e;
  else elist=e;
}


/* Convert a memory event structure to a disk structure */

static void near EventMemToDisk(struct _event *pe, struct _diskevent *pde)
{
  memset(pde, '\0', sizeof(struct _diskevent));

  pde->day=pe->day;
  pde->fill1=pe->fill1;
  pde->flags=pe->flags;
  pde->erl=pe->erl;
  pde->data1=pe->data1;
  pde->data2=pe->data2;
  pde->data3=pe->data3;
  pde->start=pe->start;
  pde->end=pe->end;
  pde->eventnum=pe->eventnum;

  strcpy(pde->tune, pe->tune);
}



/* Write the entire event file to disk */

static void near Write_Events(int e_num, int dateit)
{
  struct _event *pe;
  struct _diskevent de;
  char ebbsn[PATHLEN];
  char edatn[PATHLEN];
  SCOMBO scBbs, scDat;
  int fd;

  /* If there are no events, there is nothing to do. */

  if (!elist)
    return;

  sprintf(edatn, event_name, PRM(sys_path), e_num, dotdat);

  if ((fd=sopen(edatn,
                O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                SH_DENYNONE,
                S_IREAD | S_IWRITE))==-1)
  {
    cant_open(edatn);
    return;
  }

  if (dateit)
    Get_Dos_Date(&done_date);

  write(fd, (char *)&done_date, sizeof(union stamp_combo));

  /* Now loop through the list and write it to disk */

  for (pe=elist; pe; pe=pe->next)
  {
    EventMemToDisk(pe, &de);

    if (write(fd, (char *)&de, sizeof de) != sizeof de)
    {
      logit(cantwrite, edatn);
      break;
    }
  }

  close(fd);

  /* Now check the date on the .bbs file.  If the .dat file is
   * not newer than the .bbs file (which is should be, since we
   * just wrote it!), then there is a potential infinite loop.
   *
   * To correct the problem, reset the date on the .bbs file so
   * that it contains today's timestamp, and then touch the .dat
   * file two seconds later.
   */

  sprintf(ebbsn, event_name, PRM(sys_path), e_num, dotbbs);

  if (FileDate(ebbsn, &scBbs)==0 &&
      FileDate(edatn, &scDat)==0)
  {
    if (! GEdate(&scDat, &scBbs))
    {
      SCOMBO now;

      Get_Dos_Date(&now);

      SetFileDate(ebbsn, &now);
      Delay(200);
      SetFileDate(edatn, &now);
    }
  }

}



/* ENTRYPOINT: Read the binary event file from disk */

void Write_One_Event(struct _event *e)
{
  struct _event *pe, *penext;

  for (pe=elist; pe; pe=pe->next)
    if (pe->eventnum==e->eventnum)
    {
      penext=pe->next;  /* just in case it has been changed */
      *pe=*e;           /* copy structure */
      pe->next=penext;  /* restore next pointer */
      break;
    }

  Write_Events(event_num, FALSE);
}



/* Syntax error parsing the event file */

static void near Event_Syntax_Err(char *err)
{
  logit(log_event_err, err);
}


/* Parse a flag out of events.bbs */

static void near Parse_Flag(struct _diskevent *e, char *word)
{
  char flag[PATHLEN];
  char value[PATHLEN];

  
  getword(word, flag, "=", 1);
  getword(word, value, "=", 2);

  if (eqstri(flag, "exit"))
  {
    e->flags |= EFLAG_ERLVL;
    e->erl=atoi(value);
  }
  else if (eqstri(flag, "bells"))
  {
    e->flags |= EFLAG_YELL;
    e->data1=atoi(value);

    if (!e->data2)
      e->data2=3;

    if (! *e->tune)
      strcpy(e->tune, "Yell");
  }
  else if (eqstr(flag, "maxyell"))
    e->data2=atoi(value);
  else if (eqstr(flag, "tune"))
    strnncpy(e->tune, value, sizeof(e->tune)-1);
  else Event_Syntax_Err(flag);
}


/* Parse the current day bitmask out of an ascii string */

static void near Parse_Day(struct _diskevent *e, char *word)
{
  char text[PATHLEN];
  unsigned wnum, day;

  wnum=1;

  for (;;)
  {
    getword(word, text, "|", wnum++);

    if (! *text)
      break;

    if (eqstri(text, "all"))
    {
      e->day |= DAY_SUN | DAY_MON | DAY_TUE | DAY_WED |
                DAY_THU | DAY_FRI | DAY_SAT;
    }
    else if (eqstri(text, "wkday"))
      e->day |= (DAY_MON | DAY_TUE | DAY_WED | DAY_THU | DAY_FRI);
    else if (eqstri(text, "wkend"))
      e->day |= (DAY_SAT | DAY_SUN);
    else
    {
      for (day=0; day < 7; day++)
        if (eqstri(text, weekday_ab[day]))
        {
          e->day |= (1 << day);
          break;
        }

      if (day==7)
        printf("Error!  Unknown day in event file: `%s'\n", word);
    }
  }
}


/* Parse the current time out of an ASCII string */

static void near Parse_Time(struct _etime *t, char *word)
{
  char hour[20];
  char min[20];

  getword(word, hour, ":", 1);
  getword(word, min, ":", 2);

  t->hour=atoi(hour);
  t->min=atoi(min);
  
  if (t->hour > 23 || t->min > 59)
    logit(log_bad_evt_time, word);
}


/* Process one line in the event file */

static int near Process_Event_Line(char *line)
{
  struct _diskevent de;
  int wn=1;
  char *word;
  char *p;

  memset(&de, '\0', sizeof de);


  /* Strip comments */

  if ((p=strchr(line, ';')) != NULL)
    *p='\0';
  
  word=strtok(line, ctl_delim);

  while (word && *word)
  {
    switch (wn++)
    {
      case 1:
        if (!eqstri(word, "event"))
        {
          Event_Syntax_Err(word);
          return FALSE;
        }
        break;

      case 2:
        Parse_Day(&de, word);
        break;

      case 3:
        Parse_Time(&de.start, word);
        break;

      default:
        if (isdigit(*word))
          Parse_Time(&de.end, word);
        else Parse_Flag(&de, word);
    }

    word=strtok(NULL, ctl_delim);
  }


  /* If the line was successfully parsed, add it to the event list */

  if (wn >= 2)
  {
    EventAddList(&de);
    return TRUE;
  }

  return FALSE;
}


/* Parse the entire event file */

static int near Parse_Event_File(char *ebbsname)
{
  char temp[PATHLEN];
  FILE *ef;

  EventListFree();

  if ((ef=fopen(ebbsname, fopen_read))==NULL)
    return FALSE;

  while (fgets(temp, PATHLEN, ef) != NULL)
    Process_Event_Line(temp);

  fclose(ef);
  return TRUE;
}


/* Returns TRUE if we are currently in the specified event */

static int near InEvent(struct _event *e, int for_now)
{
  union stamp_combo then_st, then_end;
  union stamp_combo now;
  time_t gmtime;
  struct tm *ts;

  gmtime=time(NULL);
  ts=localtime(&gmtime);
  TmDate_to_DosDate(ts,&now);
  
  /* Shift a one over 'ts->tm_wday' positions, which corresponds to the     *
   * bitmask used for the 'day' field in the event-reading function.  If    *
   * it doesn't match, then we're certainly not in this event.              */
     
  if ( ( e->day & (1 << ts->tm_wday) )==0)
    return FALSE;

  if (!for_now)
    return TRUE;

  Event_To_Dos_Date(&e->start, &then_st,  &now);
  Event_To_Dos_Date(&e->end,   &then_end, &now);

  /* If right now is between the start and end time, then return TRUE... */
  
  return (GEdate(&now, &then_st) && !GEdate(&now, &then_end));
}



/* Find and return the event we're in, with the qualifier that it must      *
 * have the 'eflag' bit(s) turned on.  Returns TRUE if found (and copies    *
 * the event into 'out' (if non-NULL) and returns FALSE if not found.       */

int GetEvent(word eflag, int not_eflag, struct _event *out, int now)
{
  struct _event *e;

  
  for (e=elist; e; e=e->next)
    if ((e->flags & eflag)==eflag &&
        (e->flags & not_eflag)==0 &&
        InEvent(e, now))
    {
      if (out)
        *out=*e;
      
      return TRUE;
    }
    
  return FALSE;
}





/* ENTRYPOINT: Read the binary event file */

void Read_Event_File(int e_num)
{
  struct _event *pe;
  struct _diskevent de;
  char edatname[PATHLEN];
  union stamp_combo now;
  int fd;


  EventListFree();

  sprintf(edatname, event_name, PRM(sys_path), e_num, dotdat);

  if ((fd=shopen(edatname, O_RDONLY | O_BINARY))==-1)
  {
    cant_open(edatname);
    return;
  }

  read(fd, (char *)&done_date, sizeof(union stamp_combo));

  while (read(fd, (char *)&de, sizeof de)==sizeof de)
    EventAddList(&de);

  close(fd);

  Get_Dos_Date(&now);

  /* If we've started a new day, reset all of the 'done' flags on the       *
   * events                                                                 */

  if (now.dos_st.date != done_date.dos_st.date)
  {
    for (pe=elist; pe; pe=pe->next)
      if (pe->flags & EFLAG_DONE)
        pe->flags &= ~EFLAG_DONE;

    Write_Events(e_num, TRUE);
  }
}



static int near TryReadEither(int e_num, int write_anyway)
{
  union stamp_combo edat_st, ebbs_st;

  char edatn[PATHLEN];
  char ebbsn[PATHLEN];

  sprintf(edatn, event_name, PRM(sys_path), e_num, dotdat);
  sprintf(ebbsn, event_name, PRM(sys_path), e_num, dotbbs);

  /* Get dates of eventsxx.dat and eventsxx.bbs */

  FileDate(ebbsn, &ebbs_st);
  FileDate(edatn, &edat_st);

  if (!fexist(ebbsn) || GTdate(&ebbs_st, &edat_st))
  {
    /* Read ASCII control file */

    if (!Parse_Event_File(ebbsn))
      if (!write_anyway)
        return FALSE;

    Write_Events(e_num, TRUE);               /* write out in binary format */
    SetFileDate(edatn, &ebbs_st);
  }
  else Read_Event_File(e_num);  /* read binary format directly */

  return TRUE;
}


/* ENTRYPOINT:  Read the current event status from disk and either          *
 * parse the binary or ascii event file.                                    */

void Read_Events(void)
{
  if (!TryReadEither(event_num, FALSE))
  {
    if (TryReadEither(0, FALSE))
      event_num=0;
    else TryReadEither(event_num, TRUE);
  }
}


