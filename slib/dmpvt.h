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

#undef malloc
#undef realloc
#undef calloc
#undef free
#undef strdup

#define DBEXTRA (sizeof(int)*4)
#define DB_SIZE (sizeof(int)*2)

#ifdef __FLAT__
  #define MAX_PTR 8192
  #define KEY1 0x53440101
  #define KEY2 0x21210202

  #define PTR_TEXT "%08x"
  #define POINTER(x) (long)(x)
  #define PTR_SIZE "%08lx"
  #define SPC_TEXT "%08lx"
#else
  #define MAX_PTR 1024
  #define KEY1 0x5344
  #define KEY2 0x2121

  #define PTR_TEXT "%04x:%04x"
  #define POINTER(x) FP_SEG(x), FP_OFF(x)
  #define PTR_SIZE "%04lx"
  #define SPC_TEXT "%6ld"
#endif

struct _ptab
{
  void *p;

  #if defined(__NEARCODE__) || defined(__FLAT__)
  void near *aat;
  #else
  void far *aat;
  #endif

  int delta;
  int size;
};

#ifdef __NEARCODE__
  #define MODTYPE near
#else
  #define MODTYPE far
#endif

#ifdef __WATCOMC__
  /* wc passes args in registers, so we need to use some funky code         *
   * to get at the stack.                                                   */

  #ifdef __386__
    unsigned _get_sp(void);

    #pragma aux _get_sp = 0x8b 0xc4   /* mov eax, esp */  \
                          value [eax];

    #define MAGICSTACK(p) (((void MODTYPE **)_get_sp())[-4])
  #else
    unsigned _get_sp(void);
    unsigned _get_bp(void);
  
    #pragma aux _get_sp = 0x89 0xe0   /* mov ax, sp */  \
                          value [ax];

    #pragma aux _get_bp = 0x89 0xe8   /* mov ax, bp */  \
                          value [ax];

#ifdef __NEARCODE__
    #define MAGICSTACK(p) (((void MODTYPE **)_get_sp())[-6])
#else
    #define MAGICSTACK(p) (*(void MODTYPE **)((char *)_get_bp()+6))
#endif
  #endif
#else
  #ifdef __NEARCODE__
    #define MAGICSTACK(p) (((void MODTYPE **)&(p))[-1])
  #else
    #define MAGICSTACK(p) (void MODTYPE *)*(long MODTYPE *)((char MODTYPE *)&(p)-4)
  #endif
#endif



int _stdc d_add_table(void *p, void MODTYPE * magic, int size);

