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


struct _maxcol
{

#if 0

  /*
   *  Obsolete:
   *  These are now set via strings in the language file
   */

  byte menu_name;         /* yellow */
  byte menu_high;         /* yellow */
  byte menu_text;         /* gray */
  byte file_name;         /* yellow */
  byte file_size;         /* magenta */
  byte file_date;         /* green */
  byte file_desc;         /* cyan */
  byte file_find;         /* yellow */
  byte file_off;          /* red */
  byte file_new;          /* blinking green */
  byte msg_from;          /* cyan */
  byte msg_to;            /* cyan */
  byte msg_subj;          /* cyan */
  byte msg_from_txt;      /* yellow */
  byte msg_to_txt;        /* yellow */
  byte msg_subj_txt;      /* yellow */
  byte msg_date;          /* lightgreen */
  byte msg_attr;          /* lightgreen */
  byte addr_type;         /* cyan */
  byte addr_locus;        /* green */
  byte msg_text;          /* gray */
  byte msg_quote;         /* cyan */
  byte msg_kludge;        /* lightmagenta */
  byte hot_opt;           /* black on white */
  byte hot_more;          /* lightred on white */
  byte hot_clr;           /* white on white */
  byte fsr_msgn;          /* lightred on blue */
  byte fsr_msglink;       /* yellow on blue */
  byte fsr_attr;          /* yellow on blue */
  byte fsr_msginfo;       /* yellow on blue */
  byte fsr_date;          /* white on blue */
  byte fsr_addr;          /* yellow on blue */
  byte fsr_static;        /* white on blue */
  byte fsr_border;        /* lightcyan on blue */

#endif

  byte status_bar;        /* black on white */
  byte status_cht;        /* blinking black on white */
  byte status_key;        /* blinking black on white */

  byte pop_text;          /* white on blue */
  byte pop_border;        /* yellow on blue */
  byte pop_high;          /* yellow on blue */
  byte pop_list;          /* black on grey */
  byte pop_lselect;       /* grey on red */

  byte wfc_stat;          /* white on blue */
  byte wfc_stat_bor;      /* yellow on blue */
  byte wfc_modem;         /* gray on blue */
  byte wfc_modem_bor;     /* lgreen on blue */
  byte wfc_keys;          /* yellow on blue */
  byte wfc_keys_bor;      /* white on blue */
  byte wfc_activ;         /* white on blue */
  byte wfc_activ_bor;     /* lcyan on blue */
  byte wfc_name;          /* yellow on black */
  byte wfc_line;          /* white on black */
};


