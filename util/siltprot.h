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

#include "areadat.h"

int Make_Strng(char *value,int type);
int Make_Pth(char *value);
void Strip_Comment(char *l);
int Parse_Ctlfile(char *ctlname);
void Initialize_Prm(void);
int Parse_System(FILE *ctlfile);
int Parse_Equipment(FILE *ctlfile);
int Parse_Matrix(FILE *ctlfile);
int Parse_Session(FILE *ctlfile);
int Parse_Reader(FILE *ctlfile);
int Parse_Area(FILE *ctlfile,char *name);
int Deduce_Priv(char *p);
int Deduce_Attribute(char *a);
int Deduce_Class(int priv);
void Unknown_Ctl(int linenum,char *p);
int Compiling(char type,char *string,char *name);
void Blank_Sys(struct _sys *sys,int mode);
int Add_Backslash(char *s);
int Remove_Backslash(char *s);
int Parse_Weekday(char *s);
int Parse_Menu(FILE *ctlfile,char *name);
dword Deduce_Lock(char *p);
void Attrib_Or(int clnum,int attr,struct _area *area);
void Attrib_And(int clnum,int attr,struct _area *area);
void Add_Path(char *s,int warn);
void Add_Filename(char *s);
char *fchar(char *str,char *delim,int wordno);
int Add_To_Heap(char *s,int fancy);
void Blank_Area(struct _area *area);
/* byte MaxPrivToOpus(int maxpriv); */
void Add_Specific_Path(char *frompath,char *topath,char *add_path);
int makedir(char *d);
int Parse_Language(FILE *ctlfile);
void NoMem(void);
int Parse_Colours(FILE *ctlfile);
void Initialize_Colours(void);
int Parse_Protocol(FILE *ctlfile, char *name);
int ParseMsgArea(FILE *ctlfile, char *name);
int ParseFileArea(FILE *ctlfile, char *name);
int tsearch(char *key,struct _st base[],unsigned int num);
void FileAreaClose(void);
void MsgAreaClose(void);
void ErrWrite(void);
int VerbParse(void *pfi, struct _vbtab *verbs, char *line);
void near FiltPath(void *v, char *words[], char *line);
void near FiltOverride(void *v, char *words[], char *line);
void near FiltMenuname(void *v, char *words[], char *line);
int ParseFileArea(FILE *ctlfile, char *name);
void ParseMsgDivisionBegin(char *name, char *acs, char *displayfile, char *descript);
void ParseMsgDivisionEnd(void);
void BadDivisionName(void);
void ParseFileDivisionBegin(char *name, char *acs, char *displayfile, char *descript);
void ParseFileDivisionEnd(void);
void Generate20Areas(void);
void assert_dir(char *path);
void Write_Access();
int ParseAccess(FILE *ctlfile,char *name);
int max2priv(word usLevel);

