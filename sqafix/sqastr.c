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
 * Safe string and memory manipulations routines module
 *
 * Created: 06/Jan/92
 * Updated: 19/Apr/97
 *
 * Written by Pete Kvitek of JV Dialogue 1st BBS (2:5020/6)
 * Copyright (c) 1992-2000 by JV DIALOGUE. All rights reserved.
 *
 */

#ifndef SQAF_STR_STDCALLS

 #include "sqafix.h"

/*
 * Local declarations
 */

 static CHAR achNull[] = "\n%s(%lu): %s == NULL"                \
                         " in v" VERSION " " __DATE__ " " __TIME__"\a\n";

 #define ASSERT_NOTNULL(p, psz)                                 \
         if (p == NULL) {                                       \
           WriteLog(achNull, pszFile, iLine, psz);              \
           DoAbort();                                           \
         }                                                      \

/*
 * Local subroutines
 */

 static VOID DoAbort(VOID)
 {
   WriteLog("\nFatal error, please send the line above to"
            "\nPete Kvitek@2:5020/6 or <kvitekp@dialoc.com>"
            "\n");
   exit(EXIT_FAILURE);
 }

/*
 * String manipulation functions
 */

 int x_strlen(char * pszFile, long iLine, char * psz)
 {
   ASSERT_NOTNULL(psz, "x_strlen: psz");
   return strlen(psz);
 }

 char * x_strcpy(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_strcpy: psz1");
   ASSERT_NOTNULL(psz2, "x_strcpy: psz2");
   return strcpy(psz1, psz2);
 }

 char * x_strncpy(char * pszFile, long iLine, char * psz1, char * psz2, int cch)
 {
   ASSERT_NOTNULL(psz1, "x_strncpy: psz1");
   ASSERT_NOTNULL(psz2, "x_strncpy: psz2");
   return strncpy(psz1, psz2, cch);
 }

 char * x_strcat(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_strcat: psz1");
   ASSERT_NOTNULL(psz2, "x_strcat: psz2");
   return strcat(psz1, psz2);
 }

 char * x_strncat(char * pszFile, long iLine, char * psz1, char * psz2, int cch)
 {
   ASSERT_NOTNULL(psz1, "x_strncat: psz1");
   ASSERT_NOTNULL(psz2, "x_strncat: psz2");
   return strncat(psz1, psz2, cch);
 }

 int x_strcmp(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_strcmp: psz1");
   ASSERT_NOTNULL(psz2, "x_strcmp: psz2");
   return strcmp(psz1, psz2);
 }

 int x_stricmp(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_stricmp: psz1");
   ASSERT_NOTNULL(psz2, "x_stricmp: psz2");
   return stricmp(psz1, psz2);
 }

 char * x_strlwr(char * pszFile, long iLine, char * psz)
 {
   ASSERT_NOTNULL(psz, "x_strlwr: psz");
   return strlwr(psz);
 }

 char * x_strupr(char * pszFile, long iLine, char * psz)
 {
   ASSERT_NOTNULL(psz, "x_strupr: psz");
   return strupr(psz);
 }

 char * x_strchr(char * pszFile, long iLine, char * psz, int ch)
 {
   ASSERT_NOTNULL(psz, "x_strchr: psz");
   return strchr(psz, ch);
 }

 char * x_strrchr(char * pszFile, long iLine, char * psz, int ch)
 {
   ASSERT_NOTNULL(psz, "x_strrchr: psz");
   return strrchr(psz, ch);
 }

 char * x_strstr(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_strstr: psz1");
   ASSERT_NOTNULL(psz2, "x_strstr: psz2");
   return strstr(psz1, psz2);
 }

 char * x_strtok(char * pszFile, long iLine, char * psz1, char * psz2)
 {
   ASSERT_NOTNULL(psz1, "x_strtok: psz1");
   ASSERT_NOTNULL(psz2, "x_strtok: psz2");
   return strtok(psz1, psz2);
 }

/*
 * Memory manipulation functions
 */

 void * x_memchr(char * pszFile, long iLine, void * p, int ch, int n)
 {
   ASSERT_NOTNULL(p, "x_memchr: p");
   return memchr(p, ch, n);
 }

 void * x_memset(char * pszFile, long iLine, void * p, int ch, int n)
 {
   ASSERT_NOTNULL(p, "x_memset: p");
   return memset(p, ch, n);
 }

 int x_memcmp(char * pszFile, long iLine, void * p1, void * p2, int n)
 {
   ASSERT_NOTNULL(p1, "x_memcmp: p1");
   ASSERT_NOTNULL(p2, "x_memcmp: p2");
   return memcmp(p1, p2, n);
 }

 int x_memicmp(char * pszFile, long iLine, void * p1, void * p2, int n)
 {
   ASSERT_NOTNULL(p1, "x_memicmp: p1");
   ASSERT_NOTNULL(p2, "x_memicmp: p2");
   return memicmp(p1, p2, n);
 }

 void * x_memcpy(char * pszFile, long iLine, void * p1, void * p2, int n)
 {
   ASSERT_NOTNULL(p1, "x_memcpy: p1");
   ASSERT_NOTNULL(p2, "x_memcpy: p2");
   return memcpy(p1, p2, n);
 }

 void * x_memmove(char * pszFile, long iLine, void * p1, void * p2, int n)
 {
   ASSERT_NOTNULL(p1, "x_memmove: p1");
   ASSERT_NOTNULL(p2, "x_memmove: p2");
   return memmove(p1, p2, n);
 }

#endif  /* SQAF_STR_STDCALLS */

/***************************************************************************
* End of SQASTR.C                                                          *
****************************************************************************/
