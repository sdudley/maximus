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

#ifndef PRMAPI_H_INCLUDED__
#define PRMAPI_H_INCLUDED__

typedef struct _hprm
{
  struct m_pointers mp;
  char *pszHeap;
} *HPRM;

HPRM PrmFileOpen(char *pszFile, int iAbortOnError);
char * PrmRelativeString(HPRM hp, char *pszIn, char *pszOut);
void PrmFileClose(HPRM hp);

#define PrmFileString(hp, field)  ((hp)->pszHeap + (hp)->mp.field)
#define PrmFileValue(hp, field)   ((hp)->mp.field)

#endif /* PRMAPI_H_INCLUDED__ */

