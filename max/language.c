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
static char rcs_id[]="$Id: language.c,v 1.1 2002/10/01 17:51:14 sdudley Exp $";
#pragma on(unreferenced)

#define MAX_LANG_max_chat
#define MAX_LANG_max_chng
#define MAX_LANG_end

#ifdef INTERNAL_LANGUAGES

  #define INITIALIZE_LANGUAGE

  #include "prog.h"
  #include "mm.h"

  void Chg_Language(void) {}

#else /* external languages */


#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <mem.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "alc.h"
#include "prog.h"
#include "mm.h"
#include "language.h"

static struct _lang glh, syh, ush;
static int langfile=-1;
static union stamp_combo prmdate;
static char psps_ltf[]="%s%s.ltf";
#ifdef ORACLE
static char dotltf[]=".ltf";
#endif
static int using_alternate=0;


/* Update global variables containing often-used strings so that we don't
 * have to go through the language file to get them.
 */

static void near UpdateStaticStrings(void)
{
  cYes=*Yes;
  cNo=*No;
  cNonStop=*M_nonstop;

  if (szHeyDude)
    free(szHeyDude);

  if (szPageMsg)
    free(szPageMsg);

  szHeyDude = strdup(ch_hey_dude_msg);
  szPageMsg = strdup(ch_page_msg);
}

 /* Sets/unset indicator for use of alternate strings in the current heap
  * This current scheme allows for only 1 alternate string set for any
  * heap, but can be expanded later if needed, in which case 'usealt'
  * becomes and index rather than 1 (alternate) or 0 (standard) as it is now
  */

void Set_Lang_Alternate(int usealt)
{
  using_alternate=(usealt && !local) ? TRUE : FALSE;
}


/* Read the specified heap number from the given heap file */

static int ReadHeap(int hfile,struct _lang *l)
{
  word ptrsize;
  word max_ptr;
  word max_len;

  /* Calculate the size of the pointer table (which is the number     *
   * of strings in this heap, multiplied by the size of each pointer) */

  ptrsize= (l->ch->ndefs + l->ch->adefs) * sizeof(HOFS);

  /* Read the pointers into the right spot... */

  if (lseek(hfile, l->ch->start_ofs, SEEK_SET)==-1L)
    return FALSE;

  max_ptr=min(ptrsize,l->max_ptrs);

  if (read(hfile, (char *)l->ptrs, max_ptr) != (signed)max_ptr)
    return FALSE;

  /* And now read the strings themselves */

  max_len=min(l->ch->hlen, l->max_heap);

  if (read(hfile, l->heap, max_len) != (signed)max_len)
    return FALSE;
  else return TRUE;
}


/* Zero out a language heap */

static void Blank_Lang(struct _lang *l)
{
  memset(l->ptrs, '\0', l->max_ptrs);
  memset(l->heap, '\0', l->max_heap);
  memset(l->hdat, '\0', sizeof(struct _heapdata)*MAX_HEAP);

  l->ch=NULL;
}


/* Initialize a language heap, and allocate appropriate amount of mem */

static int Init_Lang_Heap(struct _lang *l)
{
  if ((l->ptrs=(HOFS *)malloc(l->max_ptrs))==NULL ||
      (l->heap=(char *)malloc(l->max_heap))==NULL ||
      (l->heapnames=malloc(MAX_HEAPNAMES))==NULL)
  {
    if (l->ptrs)
      free(l->ptrs);

    logit(log_mem_nolheap);
    return FALSE;
  }

  Blank_Lang(l);

  return TRUE;
}



/* Free one language heap */
#ifndef ORACLE
static void near Deinit_Lang_Heap(struct _lang *l)
{
  free(l->heapnames);
  free(l->heap);
  free(l->ptrs);

  l->ptrs=NULL;
  l->heap=NULL;
  l->ch=NULL;
}
#endif

/* Deinitialize all of the language heaps and free their memory */

#ifndef ORACLE
  void LanguageCleanup(void)
  {
    Deinit_Lang_Heap(&ush);
    Deinit_Lang_Heap(&glh);
    Deinit_Lang_Heap(&syh);
  }
#endif

/* Load a certain language heap into memory */

static int Load_Language(char *name, struct _lang *l)
{
  char temp[PATHLEN];
  int lf;
  int x;

#ifdef ORACLE
  /* The language path may be relative, so we have to resolve it
   * relative to the system path.  On the flip side, Max is always
   * guaranteed to be in the \Max directory, so it doesn't have
   * this problem.
   */

  strcpy(temp, PRM(lang_path));

  if (temp[1] != ':' && temp[0] != '\\' && temp[0] != '/')
  {
    strcpy(temp, PRM(sys_path));
    strcat(temp, PRM(lang_path));
  }

  strcat(temp, name);
  strcat(temp, dotltf);

#else
  sprintf(temp, psps_ltf, PRM(lang_path), name);
#endif

  /* Open with deny-write since writing to the .LTF file while Max is       *
   * active is disastrous.                                                  */

  if ((lf=sopen(temp, O_RDONLY | O_BINARY | O_NOINHERIT,
                SH_DENYWR, S_IREAD | S_IWRITE))==-1)
  {
    return -1;
  }

  /* Read in the global file information */

  if (read(lf, (char *)&l->inf, sizeof(struct _gheapinf)) !=
          sizeof(struct _gheapinf))
  {
    close(lf);
    return -1;
  }

  /* Allocate the heap names array */

  if (read(lf, l->heapnames, l->inf.hn_len) != l->inf.hn_len)
    return -1;

  /* Now read in the booty on the hunks contained within */

  x=min(MAX_HEAP, l->inf.n_heap) * sizeof(struct _heapdata);

  if (read(lf, (char *)l->hdat, x) != x)
  {
    close(lf);
    return -1;
  }

  return lf;
}


/* Make sure that the PRM is newer than the language file */

static void Verify_Language_Date(int fd,char *name)
{
  union stamp_combo ld;
  
  get_fdt(fd, &ld);

  if (GEdate(&ld, &prmdate))
  {
    logit("!Old language %s: recompile .PRM file with SILT!", name);
    quit(ERROR_CRITICAL);
  }
}



/* Make sure that the number of pointers in this language file is correct */

static void CheckLangPtrs(struct _lang *ph, char *name)
{
  if (ph->inf.file_ptrs != n_lang_ptrs)
  {
    logit("!Language '%s': string count mismatch", name);
    maximus_exit(1);
  }
}

/* Initialize all of the language files and static strings */

void Initialize_Languages(void)
{
  int sysfile;

  ush.max_ptrs=prm.max_ptrs;
  glh.max_ptrs=prm.max_glh_ptrs;
  syh.max_ptrs=prm.max_syh_ptrs;

  ush.max_heap=prm.max_heap;
  glh.max_heap=prm.max_glh_len;
  syh.max_heap=prm.max_syh_len;

  Init_Lang_Heap(&ush);
  Init_Lang_Heap(&glh);
  Init_Lang_Heap(&syh);

  if (usr.lang > prm.max_lang)
    usr.lang=0;

  if ((langfile=Load_Language(PRM(lang_file[usr.lang]), &ush))==-1)
    if ((langfile=Load_Language(PRM(lang_file[usr.lang=0]), &ush))==-1)
      logit(log_err_lang, PRM(lang_file[0]));

  if ((sysfile=Load_Language(PRM(lang_file[0]), &syh))==-1)
    logit(log_err_lang, PRM(lang_file[0]));

  if (langfile==-1 || sysfile==-1)
    quit(ERROR_CRITICAL);

  /* Make sure that the language files have the right number of pointers */

  CheckLangPtrs(&ush, PRM(lang_file[usr.lang]));
  CheckLangPtrs(&syh, PRM(lang_file[0]));

  FileDate(prmname, &prmdate);
  
  Verify_Language_Date(langfile, PRM(lang_file[usr.lang]));
  Verify_Language_Date(sysfile, PRM(lang_file[0]));

  /* Read the GLOBAL (ie. static, always-in-memory) strings out of the      *
   * user language file.                                                    */

  glh.ch=&ush.hdat[0];
  ReadHeap(langfile, &glh);

  UpdateStaticStrings();

  /* Read the sysop strings (which are always static) out of the sysop      *
   * language file.                                                         */

  memmove(&syh.hdat[1], &ush.hdat[1], sizeof(struct _heapdata));
  syh.ch=&syh.hdat[1];
  ReadHeap(langfile, &syh);

  /* We only need to read the sysop stuff once, so close this file.         */

  close(sysfile);
}

/* Return a pointer to the specified string based on the string# only       */

static char *HeapPtr(struct _lang *l,int sn)
{
  /* It's in the sysop heap, so return the pointer to the string in     *
   * that hunk of data.                                                 */

  if (l->heap && l->ptrs)
  {
    if (sn < l->ch->ndefs)
    {

      /* If we're using alternates, and the heap being queried has
       * a set of alternates and the index is less than the number of
       * alternate offsets, then we'll skip past the standard the ptrs
       * and use the alternate set by incrementing the index to address
       * the alternate set which have been appended to the ptrs array
       */

      if (using_alternate && sn < l->ch->adefs)
        sn += l->ch->ndefs;

      /* Now, retrieve the string */

      return l->heap + l->ptrs[sn];
    }
  }
  return "";
}


/* Return a pointer to the specifed string based on the string# and         *
 * heap# given                                                              */

static char *HeapOfs(struct _lang *l,int sn)
{
  return HeapPtr(l, sn - l->ch->start_num);
}

static char *heapnm(struct _lang *l, struct _heapdata *h)
{
  return l->heapnames + h->heapname;
}

/* Retrieve a given string number from a named heap */

char *s_reth(char *hname, word strn)
{
  word heap;

  if (ush.ch && eqstri(heapnm(&ush,ush.ch),hname))
  {
    return HeapPtr(&ush, strn);
  }
  else if (glh.ch && eqstri(heapnm(&glh,glh.ch),hname))
  {
    return HeapPtr(&glh, strn);
  }
  else if (syh.ch && eqstri(heapnm(&syh,syh.ch),hname))
  {
    return HeapPtr(&syh, strn);
  }
  else
  {
    /* It wasn't in any of the above, so it must have been in one of the    *
     * non-current dynamic user heaps which are still on disk...            */

    ush.ch=NULL;

    /* Now find out which heap it's in... */

    for (heap=0; heap < ush.inf.n_heap; heap++)
    {
      if (eqstri(heapnm(&ush,&ush.hdat[heap]),hname))
      {
        /* Ah-hah; got it!  Now fish out the heap file from disk */

        ush.ch=&ush.hdat[heap];
        ReadHeap(langfile, &ush);

        #ifdef TEST_VER
        if (debug_ovl)
          logit(">@Lang: %02x", heap);
        #endif
        return HeapPtr(&ush, strn);
      }
    }

    return "";
  }
}


/* Retrieve a given string number from one of the three main heaps */

char *s_ret(word strn)
{
  word heap;

  /* First check to see if it's in either the current dynamic user heap     *
   * (ush), the static global heap (glh), or the static sysop heap (syh).   */

  if (ush.ch && InHeap(strn, ush.ch))
  {
    return HeapOfs(&ush, strn);
  }
  else if (glh.ch && InHeap(strn, glh.ch))
  {
    return HeapOfs(&glh, strn);
  }
  else if (syh.ch && InHeap(strn, syh.ch))
  {
    return HeapOfs(&syh, strn);
  }
  else
  {
    /* It wasn't in any of the above, so it must have been in one of the    *
     * non-current dynamic user heaps which are still on disk...            */

    ush.ch=NULL;

    /* Now find out which heap it's in... */

    for (heap=0; heap < ush.inf.n_heap; heap++)
    {
      if (InHeap(strn, &ush.hdat[heap]))
      {
        /* Ah-hah; got it!  Now fish out the heap file from disk */

        ush.ch=&ush.hdat[heap];
        ReadHeap(langfile, &ush);

        #ifdef TEST_VER
        if (debug_ovl)
          logit(">@Lang: %02x", heap);
        #endif
        return HeapOfs(&ush, strn);
      }
    }

    return "";
  }
}

/* Change the user's language to a new one, and reload lang heaps */

void Switch_To_Language(void)
{
  Blank_Lang(&ush);
  Blank_Lang(&glh);

  close(langfile);

  if ((langfile=Load_Language(PRM(lang_file[usr.lang]), &ush))==-1)
    if ((langfile=Load_Language(PRM(lang_file[usr.lang=0]), &ush))==-1)
    {
      logit(log_err_lang, PRM(lang_file[usr.lang]));
      quit(ERROR_CRITICAL);
    }

  Verify_Language_Date(langfile, PRM(lang_file[usr.lang]));

  /* Read the GLOBAL (ie. static, always-in-memory) strings out of the      *
   * user language file.                                                    */

  glh.ch=&ush.hdat[0];
  ReadHeap(langfile, &glh);
  
  UpdateStaticStrings();
}


/* Prompt the user to change to a new language */

int Get_Language(void)
{
  char temp[PATHLEN];
  byte lng;

  do
  {
    if (! *linebuf)
    {
      Puts(select_lang);

      for (lng=0; lng < MAX_LANG; lng++)
      {
        if (*PRM(lang_file[lng]))
        {
          int fd;

          sprintf(temp, psps_ltf, PRM(lang_path), PRM(lang_file[lng]));

          if ((fd=shopen(temp, O_RDONLY | O_BINARY)) != -1)
          {
            struct _gheapinf gh;

            if (read(fd, (char *)&gh, sizeof gh) == sizeof gh)
              Printf(list_option, lng+1, gh.language);

            close(fd);
          }
        }
      }
    }

    WhiteN();

    InputGets(temp, select_p);

    lng=(byte)atoi(temp);

    if (!lng)
      return -1;

    lng--;
  }
  while (lng >= MAX_LANG || *PRM(lang_file[lng])=='\0');

  return lng;
}


/* Set the user's default language */

void Chg_Language(void)
{
  int lang;

  if ((lang=Get_Language())==-1)
    return;

  usr.lang=lang;

  Switch_To_Language();

  if (*language_change)
  {
    Puts(language_change);
    Press_ENTER();
  }
}



#ifndef ORACLE
  /* Return the number of the current heap, so that we can save this variable *
   * and reload this heap later.  (This is so that we can read a string from  *
   * another heap, and then put things back the way they were.)               */

  int Language_Save_Heap(void)
  {
    word x;

    if (ush.ch==0)
      return -1;

    for (x=0; x < MAX_HEAP && x < ush.inf.n_heap; x++)
    {
      if (ush.ch==&ush.hdat[x])
        return x;
    }

    return -1;
  }


  /* Reload the specified heap number */

  void Language_Restore_Heap(int h)
  {
    if (h==-1)
      return;

    ush.ch=&ush.hdat[h];
    ReadHeap(langfile, &ush);
  }
#endif

#endif  /* !INTERNAL_LANGUAGES */


