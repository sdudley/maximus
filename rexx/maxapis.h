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

void MakeRxVal(PRXSTRING prxs, char *txt, int len);
void SetRexxVar(char *name, char *val);
void GetRexxVar(char *name, char *val, int sizeofvalue);
void SetPrefixBinary(char *prefix, char *name, char *value, int length);
void SetPrefixName(char *prefix, char *name, char *value);
void SetPrefixLong(char *prefix, char *name, long value);
void GetPrefixInt(char *prefix, char *name, void *pvalue, int sizeofvalue);
void GetPrefixName(char *prefix, char *name, char *value, int sizeofvalue);
void GetPrefixBinary(char *prefix, char *name, char *value, int sizeofvalue);
char * _fast stristr(char *string,char *search);
void ExportUser(char *p, struct _usr *pusr);
void ImportUser(char *p, struct _usr *pusr);
char * _fast sc_time(union stamp_combo *sc,char *string);
char * Keys(long key);
void SetPrefixDate(char *prefix, char *name, union stamp_combo sc);
char * Help_Level(int help);
char *Video_Mode(int video);

