/*
 * SqaFix 0.99b8
 * Copyright 1992, 2003 by Pete Kvitek.  All rights reserved.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
/*
 * SQAFIX: Squish Area Fix Utility For Squish Echo Mail Processor
 *
 * Safe string and memory manipulations routines header module
 *
 * Created: 06/Jan/92
 * Updated: 19/Apr/97
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

#ifndef SQAF_STR_DEFS
#define SQAF_STR_DEFS

/***************************************************************************
* String/memory function prototypes
*/

#ifdef SQAF_STR_STDCALLS

  // Map all the safe string/memory macros to the standard functions

#define xstrlen strlen
#define xstrcpy strcpy
#define xstrlen strlen
#define xstrcpy strcpy
#define xstrncpy strncpy
#define xstrcat strcat
#define xstrncat strncat
#define xstrcmp strcmp
#define xstricmp stricmp
#define xstrlwr strlwr
#define xstrupr strupr
#define xstrchr strchr
#define xstrrchr strrchr
#define xstrstr strstr
#define xstrtok strtok

#define xmemchr memchr
#define xmemset memset
#define xmemcmp memcmp
#define xmemicmp memicmp
#define xmemcpy memcpy
#define xmemmove memmove

#else   /* SQAF_STR_STDCALLS */

  // Map all the safe string/memory macros to the safe functions

#define xstrlen(s) x_strlen(__FILE__, __LINE__, (s))
#define xstrcpy(s1, s2) x_strcpy(__FILE__, __LINE__, (s1), (s2))
#define xstrncpy(s1, s2, n) x_strncpy(__FILE__, __LINE__, (s1), (s2), (n))
#define xstrcat(s1, s2) x_strcat(__FILE__, __LINE__, (s1), (s2))
#define xstrncat(s1, s2, n) x_strncat(__FILE__, __LINE__, (s1), (s2), (n))
#define xstrcmp(s1, s2) x_strcmp(__FILE__, __LINE__, (s1), (s2))
#define xstricmp(s1, s2) x_stricmp(__FILE__, __LINE__, (s1), (s2))
#define xstrlwr(s) x_strlwr(__FILE__, __LINE__, (s))
#define xstrupr(s) x_strupr(__FILE__, __LINE__, (s))
#define xstrchr(s, ch) x_strchr(__FILE__, __LINE__, (s), (ch))
#define xstrrchr(s, ch) x_strrchr(__FILE__, __LINE__, (s), (ch))
#define xstrstr(s1, s2) x_strstr(__FILE__, __LINE__, (s1), (s2))
#define xstrtok(s1, s2) x_strtok(__FILE__, __LINE__, (s1), (s2))

#define xmemchr(p, c, n) x_memchr(__FILE__, __LINE__, (p),(c),(n))
#define xmemset(p, c, n) x_memset(__FILE__, __LINE__, (p),(c),(n))
#define xmemcmp(p1, p2, n) x_memcmp(__FILE__, __LINE__, (p1),(p2),(n))
#define xmemicmp(p1, p2, n) x_memicmp(__FILE__, __LINE__, (p1),(p2),(n))
#define xmemcpy(p1, p2, n) x_memcpy(__FILE__, __LINE__, (p1),(p2),(n))
#define xmemmove(p1, p2, n) x_memmove(__FILE__, __LINE__, (p1),(p2),(n))

  // Declare safe string/memory functions

int x_strlen(char * pszFile, long iLine, char * psz);
char * x_strcpy(char * pszFile, long iLine, char * psz1, char * psz2);
char * x_strncpy(char * pszFile, long iLine, char * psz1, char * psz2, int cch);
char * x_strcat(char * pszFile, long iLine, char * psz1, char * psz2);
char * x_strncat(char * pszFile, long iLine, char * psz1, char * psz2, int cch);
int x_strcmp(char * pszFile, long iLine, char * psz1, char * psz2);
int x_stricmp(char * pszFile, long iLine, char * psz1, char * psz2);
char * x_strlwr(char * pszFile, long iLine, char * psz);
char * x_strupr(char * pszFile, long iLine, char * psz);
char * x_strchr(char * pszFile, long iLine, char * psz, int ch);
char * x_strrchr(char * pszFile, long iLine, char * psz, int ch);
char * x_strstr(char * pszFile, long iLine, char * psz1, char * psz2);
char * x_strtok(char * pszFile, long iLine, char * psz1, char * psz2);

void * x_memchr(char * pszFile, long iLine, void * p, int ch, int n);
void * x_memset(char * pszFile, long iLine, void * p, int ch, int n);
int x_memcmp(char * pszFile, long iLine, void * p1, void * p2, int n);
int x_memicmp(char * pszFile, long iLine, void * p1, void * p2, int n);
void * x_memcpy(char * pszFile, long iLine, void * p1, void * p2, int n);
void * x_memmove(char * pszFile, long iLine, void * p1, void * p2, int n);

#endif  /* SQAF_STR_STDCALLS */

#endif  /* SQAF_STR_DEFS */

/***************************************************************************
* End of SQASTR.H                                                          *
****************************************************************************/
