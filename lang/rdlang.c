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


#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include "prog.h"
#define MAX_INCL_LANGUAGE
#include "language.h"

#define DETAILED 0

struct _gheapinf ginf;

void
doRead(int fd, void * buf, int len, char * what)
{
  int r = read(fd, buf, len);
  if (r == len)
    return;
  fprintf(stderr, "%s: read error, wanted %d, got %d\n", what, len, r);
  exit(1);
}

void
doSeek(int fd, long ofs, int mode, char * what)
{
  if (lseek(fd, ofs, mode) != -1)
    return;
  fprintf(stderr, "%s: seek error offset=%ld, mode=%d\n", what, ofs, mode);
  exit(1);
}

void *
zalloc(unsigned sz, char * what)
{
  void * p = malloc(sz);
  if (!p)
  {
    fprintf(stderr, "%s: memory allocation error, %u bytes\n", what, sz);
    exit(1);
  }
  return p;
}

char *
escape(char * s)
{
  int i = 0;
  static char buf[1024];

  while (*s)
  {
    buf[i]='\\';
    if (*s=='\r')
      buf[++i] = 'r';
    else if (*s == '\n')
      buf[++i]='n';
    else if (*s == '\7')
      buf[++i]='a';
    else if (*s == '\f')
      buf[++i]='f';
    else if (*s == '\t')
      buf[++i]='t';
    else if (*s < ' ')
      i += sprintf(buf + i + 1, "x%02x", *s);
    else buf[i] = *s;
    ++i;
    ++s;
  }
  buf[i]=0;
  return buf;
}


int
main(int argc, char *argv[])
{
  char * lang_name = (argc < 2) ? "english.ltf" : argv[1];
  int fd = open(lang_name, O_RDONLY|O_BINARY);
  if (fd==-1)
    fprintf(stderr,"open failed '%s' errno=%d\n",lang_name,errno);
  else
  {
    int i;
    char * nameheap;

    doRead(fd, &ginf, sizeof(struct _gheapinf), "Global heap data");
    printf("                     Language name:  %s\n"
           "                   Number of heaps:  %d\n"
           "Largest dynamic offsets array size:  %u\n"
           "       Largest dynamic heap length:  %u\n"
           "       Total non-alternate strings:  %u\n\n",
           ginf.language,ginf.n_heap,ginf.max_gptrs_len,ginf.max_gheap_len,ginf.file_ptrs);

    nameheap=malloc(ginf.hn_len);
    doRead(fd, nameheap, ginf.hn_len, "names heap");

    printf("Heap# StartOfs StartSym Strings Alterns HeapLen Name\n"
           "----- -------- -------- ------- ------- ------- ------------\n");

    for (i = 0; i < ginf.n_heap; ++i)
    {
      int j;
      struct _heapdata hd;
      HOFS * ptrs;
      char * heap;

      doSeek(fd, sizeof ginf + ginf.hn_len + (i * sizeof hd), SEEK_SET, "heap data");
      doRead(fd, &hd, sizeof hd, "heap data");
      printf(" %4u %8lu %8u %7u %7u %7u %s\n",
             i,hd.start_ofs,hd.start_num,hd.ndefs,hd.adefs,hd.hlen,nameheap+hd.heapname);
#if DETAILED
      if (hd.ndefs)
      {
        doSeek(fd, hd.start_ofs, SEEK_SET, "heapptrs");
        ptrs = zalloc((hd.ndefs+hd.adefs)*sizeof(HOFS), "heap ptrs");
        doRead(fd, ptrs, (hd.ndefs+hd.adefs)*sizeof(HOFS), "heap ptrs");
        heap = zalloc(hd.hlen, "heap strings");
        doRead(fd, heap, hd.hlen, "heap strings");
        for (j=0; j < hd.ndefs; ++j)
        {
          printf("    %-4u: '%s'\n", hd.start_num+j, escape(heap + ptrs[j]));
          if (j < hd.adefs && ptrs[j] != ptrs[j+hd.ndefs])
            printf("   *%-4u: '%s'\n", hd.start_num+j, escape(heap + ptrs[j+hd.ndefs]));
        }
      }
#endif
    }
    close(fd);
  }
  return 0;
}
