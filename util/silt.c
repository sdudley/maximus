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
static char rcs_id[]="$Id: silt.c,v 1.1.1.1 2002/10/01 17:57:37 sdudley Exp $";
#pragma on(unreferenced)

/*# name=SILT, main module
*/

#define SILT
#define SILT_INIT
#define MAX_INCL_VER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "prog.h"
#include "max.h"
#include "silt.h"
#include "opusprm.h"

#ifdef __WATCOMC__
  #include <malloc.h>
#endif

struct _maxcol col=
{
#if 0
  /* Obsolete */
  '\x0e',       /* yellow */                  /*  menu_name;    */
  '\x0e',       /* yellow */                  /*  menu_high;    */
  '\x07',       /* gray */                    /*  menu_text;    */
  '\x0e',       /* yellow */                  /*  file_name;    */
  '\x05',       /* magenta */                 /*  file_size;    */
  '\x02',       /* green */                   /*  file_date;    */
  '\x03',       /* cyan */                    /*  file_desc;    */
  '\x0e',       /* yellow */                  /*  file_find;    */
  '\x04',       /* red */                     /*  file_off;     */
  '\x82',       /* blinking green */          /*  file_new;     */
  '\x03',       /* cyan */                    /*  msg_from;     */
  '\x03',       /* cyan */                    /*  msg_to;       */
  '\x03',       /* cyan */                    /*  msg_subj;     */
  '\x0e',       /* yellow */                  /*  msg_from_txt; */
  '\x0e',       /* yellow */                  /*  msg_to_txt;   */
  '\x0e',       /* yellow */                  /*  msg_subj_txt; */
  '\x0a',       /* lightgreen */              /*  msg_date;     */
  '\x0a',       /* lightgreen */              /*  msg_attr;     */
  '\x03',       /* cyan */                    /*  addr_type;    */
  '\x02',       /* green */                   /*  addr_locus;   */
  '\x03',       /* gray */                    /*  msg_text;     */
  '\x07',       /* cyan */                    /*  msg_quote;    */
  '\x0d',       /* lightmagenta */            /*  msg_kludge;   */
  '\x70',       /* black on white */          /*  hot_opt;      */
  '\x7c',       /* lightred on white */       /*  hot_more;     */
  '\x77',       /* white on white */          /*  hot_clr;      */
  '\x1c',       /* lightred on blue */        /*  fsr_msgn;     */
  '\x1e',       /* yellow on blue */          /*  fsr_msglink;  */
  '\x1e',       /* yellow on blue */          /*  fsr_attr;     */
  '\x1e',       /* yellow on blue */          /*  fsr_msginfo;  */
  '\x1f',       /* white on blue */           /*  fsr_date;     */
  '\x1e',       /* yellow on blue */          /*  fsr_addr;     */
  '\x1f',       /* white on blue */           /*  fsr_static;   */
  '\x1b',       /* lightcyan on blue */       /*  fsr_border;   */
#endif

  '\x70',       /* black on white */          /*  status_bar;   */
  '\xf0',       /* blinking black on white */ /*  status_cht;   */
  '\x70',       /* black on white */          /*  status_key;   */

  '\x1f',       /* white on blue */           /*  pop_text;     */
  '\x1e',       /* yellow on blue */          /*  pop_border;   */
  '\x1e',       /* yellow on blue */          /*  pop_high;     */
  '\x70',       /* black on grey */           /*  pop_list;     */
  '\x47',       /* grey on red */             /*  pop_lselect;  */

  '\x1f',       /* white on blue */           /*  wfc_stat;     */
  '\x1e',       /* yellow on blue */          /*  wfc_stat_bor; */
  '\x17',       /* gray on blue */            /*  wfc_modem;    */
  '\x1a',       /* lightgreen on blue */      /*  wfc_modem_bor;*/
  '\x1e',       /* yellow on blue */          /*  wfc_keys;     */
  '\x1f',       /* white on blue */           /*  wfc_keys_bor; */
  '\x1f',       /* white on blue */           /*  wfc_activ;    */
  '\x1b',       /* lightcyan on blue */       /*  wfc_activ_bor;*/
  '\x0e',       /* yellow on black */         /*  wfc_name;     */
  '\x0f'        /* white on black */          /*  wfc_line;     */
};


static int _stdc stcomp(const void *arg1, const void *arg2)
{
  return(strcmp(((struct _st *)arg1)->token, ((struct _st *)arg2)->token));
}



static void near Silt_Format(void)
{
  printf("\nCommand-line format:\n\n"

         "  SILT <ctl_file> [[-x]...]\n\n");

  printf("Valid switches are:\n\n"

         "  -p      Generates Maximus .PRM file, and Opus .PRM files (if requested).\n"
         "  -a      Generates msg/file data files. (Implies -am and -af.)\n"
         "  -am     Generates message area data.\n"
         "  -af     Generates file area data.\n"
         "  -m      Generates *.MNU menu data files.\n");
  printf("  -x      Generates all of the above.\n"
         "  -2a     Generates optional Max 2.0-compatible AREA.* files (normal)\n"
         "  -2u     As above, but use underscores instead of dots in area name\n"
         "  -2s     As above, but use short area names (no divisions)\n"
         "  -u      Unattended mode -- tells SILT never to prompt for user input.\n\n");

  printf("If no switches are specified, then SILT defaults to generating everything\n"
         "except the Max 2.x-compatible area files.\n");

  exit(1);
}




static void near Parse_Silt_Args(int argc,char *argv[])
{
  int x;
  char *p;

  if (argc < 2)
    Silt_Format();

  for (x=2; x < argc; x++)
  {
    if (*argv[x]=='-' || *argv[x]=='/')
    {
      p=argv[x]+1;

      while (*p)
      {
        switch(tolower(*p))
        {
          case '2':
            do_marea = do_farea = TRUE;

            switch (tolower(p[1]))
            {
              case 'a': do_2areas=TRUE; break;
              case 'u': do_2areas=TRUE; do_ul2areas=TRUE; break;
              case 's': do_2areas=TRUE; do_short2areas=TRUE; break;
              default:  Silt_Format();
            }

            p++;
            break;

          case 'p':
            do_prm=TRUE;
            break;

          case 'a':
            switch (tolower(p[1]))
            {
              case 'm': do_marea=TRUE;  p++; break;
              case 'f': do_farea=TRUE;  p++; break;
              case 0:   do_marea=do_farea=TRUE; break;
              default:  Silt_Format();
            }
            break;

          case 's':
            printf("Warning!  The '-s103' and '-s110' options are no longer supported.\n");
            p += 3;
            break;

          case 'm':
            do_menus=TRUE;
            break;

          case 'o':  /* the old "sort" switch */
            break;

          case 'u':
            do_unattended=TRUE;
            break;

          case 'x':
            do_prm=do_marea=do_farea=do_menus=TRUE;
            break;

          case 'y':
            printf("Warning!  The '-y' parameter is obsolete!\n");
            break;

          default:
           printf("\aInvalid command-line parameter:  `%s'!\n",argv[x]);
           Silt_Format();
        }

        p++;
      }
    }
    else
    {
      printf("\aInvalid command-line parameter:  `%s'!\n",argv[x]);
      Silt_Format();
    }
  }

  /* If no args specified, do everything except for SYSBBS/SYSDAT! */

  if (!do_prm && !do_marea && !do_farea && !do_menus  && !do_2areas)
    do_prm=do_marea=do_farea=do_menus=TRUE;
}




static void near Show_Options(void)
{
  printf("Processing: ");

  if (do_prm)
  {
    printf("PRM file");

    if (do_marea || do_farea || do_menus || do_2areas)
      printf(", ");
  }

  if (do_marea)
  {
    printf("Message Areas");

    if (do_farea || do_menus  || do_2areas)
      printf(", ");
  }

  if (do_2areas)
  {
    printf("Max 2.x Areas");

    if (do_menus   || do_farea)
      printf(", ");
  }

  if (do_farea)
  {
    printf("File Areas");

    if (do_menus)
      printf(", ");
  }

  if (do_menus)
    printf("Menus");

  putchar('\n');
}



/* Make post-processing adjustments to the .prm file */

static void Adjust_Prm(void)
{
  char temp[PATHLEN];

  if (! prm.achg_keys)
    Make_String(prm.achg_keys,"[]?");
  
  if (! prm.high_msgarea)
    Make_String(prm.high_msgarea,"ZZZZZZZZZZ");
  
  if (! prm.high_filearea)
    Make_String(prm.high_filearea,"ZZZZZZZZZZ");
  
  if (! prm.begin_msgarea)
    Make_String(prm.begin_msgarea, "1");
  
  if (! prm.begin_filearea)
    Make_String(prm.begin_filearea, "1");

  if (! prm.adat_name)
  {
    strcpy(temp, strings+prm.sys_path);
    strcat(temp, "area.dat");

    Make_String(prm.adat_name, temp);
  }

  if (! prm.aidx_name)
  {
    strcpy(temp, strings+prm.sys_path);
    strcat(temp, "area.idx");

    Make_String(prm.aidx_name, temp);
  }

  if (! prm.access)
    Make_String(prm.access, "access");
}


static void near Write_Max_Prm(char *name)
{
  char temp[PATHLEN];
  int prmfile;

  printf("\nWriting Maximus Version %d PRM file...",CTL_VER);

  if (! (done_sys && done_equip && done_session && done_language && 
         done_colours))
  {
    printf("\n\aWarning!  PRM creation specified, but all sections are "
           "not present!");
  }
  else
  {
    sprintf(temp, "%s.prm", name);

    if ((prmfile=open(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                      S_IREAD | S_IWRITE))==-1)
    {
      printf("\n\aError opening `%s' for write!\n",temp);
      exit(1);
    }

    write(prmfile, (char *)&prm, sizeof(struct m_pointers));
    write(prmfile, strings, offset);
    close(prmfile);
  }
}



static void near Write_Colours(void)
{
  char temp[PATHLEN];
  int fd;

  if (!do_prm)
    return;

  sprintf(temp, "%scolours.dat", strings+prm.sys_path);
  
  if ((fd=sopen(temp, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                SH_DENYNO, S_IREAD | S_IWRITE))==-1)
  {
    printf("Can't open colour data file (`%s')!  Aborting...\n", temp);
    exit(1);
  }
  
  if (write(fd, (char *)&col, sizeof(col)) != sizeof(col))
    printf("Can't write to %s\n", temp);

  close(fd);
}


int _stdc c_main(int argc,char *argv[])
{
  char temp[PATHLEN];

  Hello("SILT", "Maximus Control File Compiler", VERSION, "1989, " THIS_YEAR);

  /*dmalloc_on(TRUE);*/

  #ifdef __WATCOMC__
  #ifndef __FLAT__
  _heapgrow();
  _amblksiz=512;
  #endif
  setbuf(stdout, NULL);
  #endif

  Parse_Silt_Args(argc,argv);
  Show_Options();

  /* now do the .ctl file */

  sprintf(temp, "%s.ctl", argv[1]);

  Initialize_Prm();
  qsort(silt_table, silt_table_size, sizeof(silt_table[0]), stcomp);
  Parse_Ctlfile(temp);

  MsgAreaClose();
  FileAreaClose();

  if (do_prm || do_2areas)
    putchar('\n');

  Adjust_Prm();

  if (do_prm)
  {
    Write_Max_Prm(argv[1]);
    Write_Access();
    Write_Colours();
  }

  if (do_2areas)
    Generate20Areas();

  printf("\n\nDone!\n");

  return 0;
}


