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
static char rcs_id[]="$Id: s_colour.c,v 1.1.1.1 2002/10/01 17:57:40 sdudley Exp $";
#pragma on(unreferenced)

/*# name=SILT: 'Section Matrix and EchoMail' processing logic
*/

#define SILT
#define NOVARS
#define NOINIT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prog.h"
#include "max.h"
#include "silt.h"

static struct _cols
{
  char *name;
  byte col;
} cols[]=
{
  {"black",       0},
  {"blue",        1},
  {"green",       2},
  {"cyan",        3},
  {"red",         4},
  {"magenta",     5},
  {"brown",       6},
  {"grey",        7},
  {"gray",        7},
  {"darkgrey",    8|0},
  {"darkgray",    8|0},
  {"lightblue",   8|1},
  {"lightgreen",  8|2},
  {"lightcyan",   8|3},
  {"lightred",    8|4},
  {"lightmagenta",8|5},
  {"yellow",      8|6},
  {"white",       8|7},
  {NULL,          0}
};

static byte near translate_colour(char *name)
{
  struct _cols *c;

  for (c=cols; c->name; c++)
    if (eqstri(name, c->name))
      return c->col;
    
  printf("\n\aInvalid colour `%s'\n", name);
  Compiling(-1, NULL, NULL);
  
  return 0;
}




static struct _coltab
{
  char *opt;
  byte *col_ptr;
} coltab[]=
{
  /* Obsolete */
  {"menu name",       NULL},
  {"menu highlight",  NULL},
  {"menu text",       NULL},
  {"hotflash bar",    NULL},
  {"hotflash more",   NULL},
  {"hotflash clear",  NULL},
  {"file name",       NULL},
  {"file size",       NULL},
  {"file date",       NULL},
  {"file desc",       NULL},
  {"file find",       NULL},
  {"file offline",    NULL},
  {"file new",        NULL},
  {"message from",    NULL},
  {"message fromtxt", NULL},
  {"message to",      NULL},
  {"message totxt",   NULL},
  {"message subj",    NULL},
  {"message subjtxt", NULL},
  {"message attrib",  NULL},
  {"message date",    NULL},
  {"message address", NULL},
  {"message locus",   NULL},
  {"message body",    NULL},
  {"message quote",   NULL},
  {"message kludge",  NULL},
  {"fsr msgnum",      NULL},
  {"fsr msglinks",    NULL},
  {"fsr attribute",   NULL},
  {"fsr msginfo",     NULL},
  {"fsr date",        NULL},
  {"fsr address",     NULL},
  {"fsr static",      NULL},
  {"fsr border",      NULL},
  /* End Obsolete */
  {"status bar",      &col.status_bar},
  {"status chat",     &col.status_cht},
  {"status key",      &col.status_key},
  {"popup text",      &col.pop_text},
  {"popup border",    &col.pop_border},
  {"popup highlight", &col.pop_high},
  {"popup list",      &col.pop_list},
  {"popup lselect",   &col.pop_lselect},
  {"wfc status",      &col.wfc_stat},
  {"wfc statusbor",   &col.wfc_stat_bor},
  {"wfc modem",       &col.wfc_modem},
  {"wfc modembor",    &col.wfc_modem_bor},
  {"wfc keys",        &col.wfc_keys},
  {"wfc keysbor",     &col.wfc_keys_bor},
  {"wfc activity",    &col.wfc_activ},
  {"wfc activitybor", &col.wfc_activ_bor},
  {"wfc name",        &col.wfc_name},
  {"wfc line",        &col.wfc_line},
  {NULL,              0}
};

static void near handle_colour(struct _coltab *ct, char *col)
{
  char *s;
  byte colour=0;
  
  s=strtok(col, ctl_delim);
  
  while (s)
  {
    if (eqstri(s, "blink") || eqstri(s, "blinking"))
      colour |= 128;
    else if (eqstri(s, "on"))
    {
      /* handle background colours */
      
      s=strtok(NULL, ctl_delim);
      
      if (s)
      {
        /* Strip off the background bits */
        
        colour &= ~0x70;
        
        colour |= (translate_colour(s) & 0x07) << 4;
      }
    }
    else
    {
      colour &= ~0x0f;
      colour |= translate_colour(s);
    }
    
    s=strtok(NULL, ctl_delim);
  }
  
  *ct->col_ptr=colour;
}

int Parse_Colours(FILE *ctlfile)
{
  char line[MAX_LINE];
  char option[MAX_LINE];
  char colour[MAX_LINE];
  char keyw[MAX_LINE];
  
  struct _coltab *ct;
  
  linenum++;
  
  while (fgets(line, MAX_LINE, ctlfile) != NULL)
  {
    Strip_Comment(line);
    
    getword(line, keyw, ctl_delim, 1);

    if (*keyw != '\0')
    {
      if (eqstri(keyw, "end"))
        break;

      strcpy(option, keyw);
      strcat(option, " ");
      getword(line, option+strlen(option), ctl_delim, 2);

      strcpy(colour, fchar(line, ctl_delim, 3));

      for (ct=coltab; ct->opt; ct++)
        if (eqstri(option, ct->opt))
        {
          if (ct->col_ptr)
            handle_colour(ct, colour);
          else
          {
            printf("\n\aWarning!  Colour definition statement '%s' is\n"
                   "obsolete in this version of Maximus (line %d of '%s').\n",
                   option, linenum, ctl_name);
            Compiling(-1, NULL, NULL);
          }

          break;
        }

      if (ct->opt==NULL)
        Unknown_Ctl(linenum, option);
    }

    linenum++;
  }

  linenum++;

  return 0;
}


