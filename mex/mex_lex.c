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

#ifndef __GNUC__
#pragma off(unreferenced)
static char rcs_id[]="$Id: mex_lex.c,v 1.4 2004/01/27 20:57:25 paltas Exp $";
#pragma on(unreferenced)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "prog.h"
#include "mex.h"
#include "mex_lexp.h"

extern YYSTYPE yylval;
static MACRO macro={0};
static MACDEF macList=NULL;
static FILESTACK fstk[MAX_INCLUDE];
static int iStack=-1;
static XLTABLE xlt[]=
{
  {'n', '\n'},
  {'r', '\r'},
  {'a', '\a'},
  {'b', '\b'},
  {'f', '\f'},
  {'v', '\v'},
  {'t', '\t'},
  {0,   0}
};

static byte ifstatus[MAX_IFBLKS];
static byte ifdone[MAX_IFBLKS];
static int iBlock=-1;

/* Push a file onto the current file stack */

int push_fstk(char *fname)
{
  int i;

  if (iStack >= MAX_INCLUDE-1)
  {
    error(MEXERR_TOOMANYNESTEDINCL);
    return FALSE;
  }

  i=iStack+1;

  if ((fstk[i].fp=fopen(fname, "r"))==NULL)
  {
    error(MEXERR_CANTOPENREAD, fname);
    return FALSE;
  }

  strcpy(filename, fname);
  fstk[++iStack].name=strdup(fname);
  fstk[iStack].save_linenum=linenum;
  linenum=1;

  return TRUE;
}



/* Close off the file on the top of the stack */

int pop_fstk(void)
{
  if (iStack < 0)
    return FALSE;

  fclose(fstk[iStack].fp);

  if (fstk[iStack].name)
  {
    free(fstk[iStack].name);
    fstk[iStack].name=NULL;
  }

  /* Go back to the old line number */

  linenum=fstk[iStack].save_linenum;

  /* Change back to our original filename as well */

  if (--iStack >= 0)
    strcpy(filename, fstk[iStack].name);

  return TRUE;
}



/* Retrieve a character from the current file source -- whether that be     *
 * the main file, an include file, or a macro definition.                   */

int pull_character(void)
{
  int ch = EOF;

  if (macro.fInMacro)
  {
    if (macro.iPos >= macro.md->iLength-1)
      macro.fInMacro=FALSE;

#ifdef DEBUGLEX
    printf("%c*", macro.md->szText[macro.iPos]);
#endif
    return macro.md->szText[macro.iPos++];
  }

  /* Get characters from the current stream */

  while (iStack >= 0 && (ch=fgetc(fstk[iStack].fp))==EOF)
    pop_fstk();

#ifdef DEBUGLEX
  printf("%c%d", ch, iStack);
#endif
  fflush(stdout);

  if (iStack >= 0)
    return ch;
  else
    return EOF;
}




/* Peek at a character from the current file source */

int peek_character(void)
{
  int ch, i;

  if (macro.fInMacro)
    return macro.md->szText[macro.iPos];

  i=iStack;

  /* Don't return an EOF unless we are at the end of ALL our include files */

  while (i >= 0 && (ch=fgetc(fstk[i].fp))==EOF)
    ungetc(EOF, fstk[i--].fp);

  if (i < 0)
    return EOF;

  ungetc(ch, fstk[i].fp);
  return ch;
}

/* Processing after we just read in a ":" symbol */

static int near process_colon(void)
{
  int c;

  if ((c=peek_character())=='=')
  {
    pull_character();
    return T_ASSIGN;
  }

  return T_COLON;
}


/* Processing after we just read in a ">" symbol: */

static int near process_morethan(void)
{
  int c;

  if ((c=peek_character())=='=')
  {
    pull_character();
    return T_GE;
  }

  return T_GT;
}


/* Processing after we just read in a "<" symbol */

static int near process_lessthan(void)
{
  int c;

  if ((c=peek_character())=='=')
  {
    pull_character();
    return T_LE;
  }

  if (c=='>')
  {
    pull_character();
    return T_NOTEQUAL;
  }

  return T_LT;
}


/* Process "/" or a double-slash comment */

static int near process_slash(void)
{
  int c;

  c=peek_character();

  if (c != '/')   /* if it's not a second '/', it must be divison */
    return TRUE;

  /* else it then must be a comment, so skip to EOL or EOF. */

  while ((c=peek_character()) != '\n' && c != EOF)
    pull_character();

  return FALSE;
}


/* Function to process numeric constants */

static int near process_digit(int c)
{
  word type=TYPE_DEC;
  int fLong = FALSE;
  int fUnsigned = FALSE;
  char *scan;
  char *p, *e;


  /* Set up pointers to beginning of identifier buffer */

  p=yytext;
  e=yytext+MAX_ID_LEN-1;


  /* Copy in the character we just found */

  *p++=(char)c;


  /* As long as we have space left in the buffer, get a character *
   * from the input file, and ensure that it's a digit.           */

  while (p < e && (c=peek_character()) != EOF &&
         (isxdigit(c) || tolower(c)=='x'))
  {
    pull_character();
    *p++=(char)c;
  }

  *p='\0';

  /* If we reached the end of the buffer, the constant was        *
   * too long                                                     */

  if (p==e)
    warn(MEXERR_WARN_CONSTANTTRUNCATED, yytext);

  /* Now check for trailing "u", "l", or "ul" specifiers to modify
   * the type of this constant.
   */

  do
  {
    c = peek_character();
    c = tolower(c);

    if (c=='l')
    {
      fLong = TRUE;
      pull_character();
    }

    /* No signed/unsigned support yet, but this paves the way for the
     * future.
     */

    if (c=='u')
    {
      fUnsigned = TRUE;
      pull_character();
    }
  }
  while (c=='l' || c=='u');

  scan=yytext;

  /* Numbers beginning with "0x" should be treated as hex */

  if (yytext[0]=='0' && tolower(yytext[1])=='x')
  {
    type=TYPE_HEX;
    scan += 2;
  }

  if (type==TYPE_DEC)
    yylval.constant.dwval=atol(scan);
  else
    sscanf(scan, "%" UINT32_XFORMAT, &yylval.constant.dwval);

  /* If the number will fit in a single integer, return it
   * as a word (assuming that we did not explicitly ask otherwise).
   * Otherwise, return it as a dword.
   */

  if (yylval.constant.dwval < 65536L && !fLong)
  {
    yylval.constant.val=(word)yylval.constant.dwval;
    return T_CONSTWORD;
  }

  return T_CONSTDWORD;
}


/* Process a single character constant */

static int near process_character_constant(void)
{
  int c;

  /* a character constant */

  if ((c=pull_character())==EOF || c=='\n' || c=='\r')
  {
    error(MEXERR_UNTERMCHARCONST);
    if (c=='\n')
      linenum++;
    return FALSE;
  }

  /* Check for escape characters */

  if (c=='\\')
  {
    struct _xlt *x;

    if ((c=pull_character())==EOF)
      return FALSE;

    for (x=xlt; x->from; x++)
      if (c==x->from)
      {
        c=x->to;
        break;
      }
  }

  yylval.constant.val=(word)c;

  /* Make sure that it ends with the appropriate punctuation */

  if (pull_character() != '\'')
  {
    error(MEXERR_INVALCHARCONST);
    return FALSE;
  }

  return TRUE;
}




/* Process a constant string */

static int near process_string(void)
{
  char *str, *sptr, *olds;
  int slen, c;

  /* Allocate memory on the heap to hold the new string */

  str=sptr=smalloc(slen=STR_BLOCK);

  /* Now scan 'till the end of the string is reached */

  while ((c=pull_character()) != EOF)
  {
    /* If string is larger than buffer (minus one for the NUL),   *
     * make the buffer a bit larger.                              */

    if (sptr-str >= slen-1)
    {
      olds=str;
      str=realloc(str, slen += STR_BLOCK);

      if (str==NULL)
        NoMem();

      sptr = str+(sptr-olds);
    }

    /* Check for the end-of-string quote */

    if (c=='"')
    {
      /* Skip over all whitespace and returns */

      while ((c=peek_character())==' ' || c=='\t' || c=='\n' || c=='\r')
      {
        if (pull_character()=='\n')
          linenum++;
      }

      /* If the next non-whitespace char is NOT a quote, then it's*
       * the end of the string.  If that's the case, exit loop.   */

      if (c=='"')
        pull_character();
      else
        break;
    }
    else if (c=='\\')  /* a backslash "escape character" */
    {
      struct _xlt *x;

      /* Now compare this character to those listed, and make     *
       * the appropriate translations.                            */

      if ((c=pull_character())==EOF)
        break;

      for (x=xlt; x->from; x++)
        if (c==x->from)
        {
          *sptr++=x->to;
          break;
        }

      /* If it wasn't found, just pass the second character thru  *
       * as a normal character.                                   */

      if (! x->from)
      {
        /* Handle "x34" hex-type codes */

        if (c != 'x')
          *sptr++=(char)c;
        else
        {
          char str[3];
          int hex;

          str[0]=pull_character();
          str[1]=pull_character();
          str[2]=0;

          if (sscanf(str, "%x", &hex) != 1)
            error(MEXERR_INVALIDHEX, str);
          else
            *sptr++=hex;
        }
      }
    }
    else /* 'c' is a normal character */
    {
      *sptr++=(char)c;
    }
  }

  if (c==EOF)
    error(MEXERR_UNTERMSTRINGCONST);

  *sptr='\0';
  yylval.constant.lit=str;

  return T_CONSTSTRING;
}


/* Process an "#include" direective */

static void near process_include(char *name)
{
  static char semicolon[]=";";
  char *mex_include=getenv("MEX_INCLUDE");
  char fname[PATHLEN];
  char *tok, *p;

  if (name==NULL)
  {
    error(MEXERR_INVALIDINCLUDEDIR);
    return;
  }

  tok=strtok(name, "<>\"");

  if (tok==NULL)
  {
    error(MEXERR_INVALIDINCLUDEFILE);
    return;
  }

  if (!fexist(tok))
  {
    p=strtok(mex_include, semicolon);

    while (p)
    {
      strcpy(fname, mex_include);
      Add_Trailing(fname, PATH_DELIM);
      strcat(fname, tok);

      if (fexist(fname))
      {
        tok=fname;
        break;
      }

      p=strtok(NULL, semicolon);
    }
  }

  push_fstk(tok);
}


static MACDEF near find_macro(char *szName)
{
  register char ch=*szName;
  MACDEF md;

  for (md=macList; md; md=md->next)
    if (ch==*md->szName && eqstr(md->szName,szName))
      return md;
  return NULL;
}

static void near process_undef(char *line)
{
  MACDEF md;

  md=find_macro(line);

  /* It was there, so remove it */

  if (md!=NULL)
  {
    if (md==macList)
      macList=md->next;
    else
    {
      MACDEF dd;

      for (dd=macList;dd->next!=md; dd=dd->next)
        ;
      dd->next=md->next;
    }
    free(md->szName);
    free(md->szText);
    free(md);
  }
}

/* Handle a macro definition */

static void near process_define(char *line)
{
  MACDEF md;
  char *m, *p;

  if ((m=strtok(line, " \t\n"))==NULL)
  {
    error(MEXERR_INVALIDDEFINE);
    return;
  }

  /* Use everything else on the line as the replacement */

  p = m + strlen(m)+1;
  strtok(p, "\n");

  if ((md=find_macro(m))!=NULL)
  {
    if (!eqstr(p,md->szText))  /* #defines are identical */
      error(MEXERR_MACROALREADYDEFINED, m);

    return;
  }

  md=smalloc(sizeof *md);

  /* Grab the name of the macro */

  md->szName=strdup(m);
  md->szText=strdup(p);
  md->iLength=strlen(md->szText);

  /* Append to the linked list of macros */

  md->next=macList;
  macList=md;
}

/* Test for a macro definition - include/exclude block on result */

static void near process_ifdef(char *line, byte inblock,byte which)
{
  char *p;

  if ((p=strtok(line, " \t\n"))==NULL)
  {
    error(MEXERR_INVALIDIFDEF);
    return;
  }

  if (++iBlock >= MAX_IFBLKS)
  {
    error(MEXERR_TOOMANYNESTEDIFDEF);
    iBlock--;
  }

  if (!inblock)
    ifstatus[iBlock]=0;
  else
    ifstatus[iBlock]=find_macro(p) ? which : !which;

  ifdone[iBlock]=ifstatus[iBlock];
}

static void near process_endif()
{
  if (iBlock-- == -1)
  {
    error(MEXERR_UNMATCHEDENDIF);
    iBlock = 0;
  }
}

static void near process_else()
{
  if (iBlock == -1)
    error(MEXERR_UNMATCHEDELSE);
  else
  {
    /* Reverse previous state of this block */

    ifstatus[iBlock]=!ifstatus[iBlock];


    /* If we've already done this block via a
     * previous ifdef or elifdef, don't do it again
     */

    if (ifdone[iBlock])
      ifstatus[iBlock]=0;

    /* Now, if we've just switched this block on, check all previously
     * nested blocks to make sure we're ok on all levels.
     *
     */

    if (ifstatus[iBlock])
    {
      int i;

      for (i=iBlock; i-- > 0; )
        if (!ifstatus[i])     /* Still prevented by at least one level */
          ifstatus[iBlock]=0;

      ifdone[iBlock]=ifstatus[iBlock];
    }
  }
}


static void near process_elseifdef(char *line)
{
  if (iBlock == -1)
    error(MEXERR_UNMATCHEDELIFDEF);
  else
  {
    process_else();
    if (ifstatus[iBlock])
    {
      --iBlock;   /* Step back one level and assert new define */
      process_ifdef(line,TRUE,TRUE);
    }
  }
}



/* Process a preprocessor directive */

static int near process_preprocessor(byte inblock)
{
  char *ppbuf, *s, *tok;

  ppbuf=smalloc(PPLEN);

  /* Read till the end of line */

  for (s=ppbuf;
         s < ppbuf+PPLEN-1 && (*s=(char)peek_character()) != EOF &&
         *s != '\n';
       s++)
  {
    pull_character();
  }

  /* Since we read an EOL, increment the line counter too */

  if (*s=='\n')
  {
    pull_character();
    linenum++;
  }

  /* Cap the string */

  *++s='\0';

  /* Now parse the preprocessor directive from the line */

  tok=strtok(ppbuf, " \t\n");

  if ((s=strtok(NULL, "\r\n"))==NULL)
    s="";
  else
  {
    char *p;

    /* Chop off any '//' comments */

    if ((p=strstr(s, "//")) != NULL)
      *p=0;

    /* And remove trailing whitespace */

    p=s+strlen(s);
    while ( --p >= s && (*p == ' ' || *p == '\t'))
      *p = '\0';
  }

  if (eqstr(tok, "endif"))
    process_endif();
  else if (eqstr(tok, "else"))
    process_else();
  else if (eqstr(tok, "elifdef"))
    process_elseifdef(s);
  else if (eqstr(tok, "elseifdef"))
    process_elseifdef(s);
  else if (eqstr(tok, "ifdef"))
    process_ifdef(s,inblock,TRUE);
  else if (eqstr(tok, "ifndef"))
    process_ifdef(s,inblock,FALSE);
  else if (inblock)
  {
    if (eqstr(tok, "error"))
      error(MEXERR_ERRORDIRECTIVE, s);
    else if (eqstr(tok, "include"))
    {
      tok=strtok(s, " \t\n");
      process_include(tok);
    }
    else if (eqstr(tok, "define"))
      process_define(s);
    else if (eqstr(tok, "undef"))
      process_undef(s);
    else
    {
      error(MEXERR_UNKNOWNDIRECTIVE, tok);
    }
  }

  free(ppbuf);
  return 0;
}




/* Handle structure operator, range operator, and ellipsis operator */

static int near process_dot(void)
{
  int c;

  /* Handle '.' (structure operator), '..' (range operator), and    *
   * '...' (ellipsis).                                              */

  if ((c=peek_character())=='.')
  {
    pull_character();

    if ((c=peek_character())=='.')
    {
      pull_character();
      return T_ELLIPSIS;
    }

    return T_RANGE;
  }

  /* Otherwise it's just a single dot */

  return T_DOT;
}



/* Process a normal identifier */

static int near process_id(int c, int *prc)
{
  MACDEF md;
  struct _id *i;
  char *p, *e;

  /* Probably a reserved word or identifier */

  if (! isidchar(c))
  {
    error(MEXERR_INVALIDCHAR, (byte)c < ' ' ? ' ' : c, c);
    return FALSE;
  }

  /* Set up pointers to identifier buffer */

  p=yytext;
  e=yytext+MAX_ID_LEN-1;

  /* Now copy the characters into the ID buffer 'till we find       *
   * end of file, or a non-identifier character.                    */

  for (*p++=(char)c;
       (*p=(char)peek_character()) != EOF && isidchar(*p) && p < e;
       p++)
    pull_character();

  /* Now cap the string */

  *p='\0';

  if (p==e)
    warn(MEXERR_WARN_IDENTTRUNCATED, yytext);

  /* Scan table of reserved words, and if found, return appropriate *
   * value.                                                         */

  for (i=idlist;i->name;i++)
    if (eqstri(yytext,i->name))
    {
      *prc=i->value;
      return TRUE;
    }


  /* It wasn't found, so make a copy of the name, for insertion in  *
   * the symbol table.                                              */

  yylval.id=strdup(yytext);

  if (!macro.fInMacro && (md=find_macro(yytext))!=NULL)
  {
    macro.fInMacro=TRUE;
    macro.iPos=0;
    macro.md=md;

    return FALSE;
  }

  *prc=T_ID;
  return TRUE;
}



/* yylex() - the lexical analysis function.  */

int yylex(void)
{
  int c, rc;

  /* Get a character until we hit end-of-file */

  while ((c=pull_character()) != EOF)
  {

    /* Non-active #ifdef/#ifndef block handler */

    if (iBlock >= 0 && !ifstatus[iBlock])
    {
      switch (c)
      {
        case '\n':  linenum++;  break;
        case ' ':
        case '\t':
        case '\r':  break;
        case '#':   process_preprocessor(FALSE); break;
        default:    while ((c=peek_character()) != '\n' && c != EOF)
                      pull_character(); /* Waste the rest */
                    break;
      }
    }
    else
    {
      switch (c)
      {
        case '\n':      linenum++;  break;
        case ' ':
        case '\t':
        case '\r':      break;
        case '{':       return T_BEGIN;
        case '}':       return T_END;
        case ':':       return process_colon();
        case '=':       return T_EQUAL;
        case '>':       return process_morethan();
        case '<':       return process_lessthan();
        case '*':       return T_BMULTIPLY;
        case '&':       return T_BAND;
        case '|':       return T_BOR;
        case '/':       if (process_slash()) return T_BDIVIDE; break;
        case '%':       return T_BMODULUS;
        case '+':       return T_BPLUS;
        case '-':       return T_MINUS;
        case '(':       return T_LPAREN;
        case ')':       return T_RPAREN;
        case '[':       return T_LBRACKET;
        case ']':       return T_RBRACKET;
        case ',':       return T_COMMA;
        case ';':       return T_SEMICOLON;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':       return process_digit(c);
        case '\'':      if (process_character_constant()) return T_CONSTBYTE; break;
        case '\"':      return process_string();
        case '#':       process_preprocessor(TRUE); break;
        case '.':       return process_dot(); break;
        default:        if (process_id(c, &rc)) return rc; break;
      } /* switch */
    }
  } /* while */

  if (iBlock != -1)
    error(MEXERR_UNMATCHEDIFDEF);

  /* Return a negative value to indicate EOF */

  return -1;
}

