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

/*
 * User class API
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <malloc.h>
#include "prog.h"
#include "mm.h"
#include "max_oldu.h"

  /* Read class information file and return a classinfo handle
   */

int ClassReadFile(char *pszName)
{
  int fd, sz, rc;
  CLSHDR chdr;

  rc=FALSE;
  fd=shopen(pszName, O_RDONLY|O_BINARY|O_NOINHERIT);

  if (fd!=-1)
  {

    if (read(fd, &chdr, sizeof chdr) == sizeof chdr &&
        chdr.ulclhid == CLS_ID && chdr.ussize != 0 &&
        chdr.usclfirst != 0 && chdr.usn != 0)
    {

      sz=sizeof(*pclh) + (chdr.usn*chdr.ussize) + chdr.usstr;
      pclh=malloc(sz);

      if (pclh!=NULL)
      {

        /* Set up control information */

        pclh->usn=chdr.usn;
        pclh->ussize=chdr.ussize;
        pclh->pcInfo=(CLSREC *)(((char *)pclh)+sizeof(*pclh));
        pclh->pHeap=(char *)(pclh->pcInfo+pclh->usn);

        sz=(sizeof(CLSREC)*pclh->usn) + chdr.usstr;

        lseek(fd, chdr.usclfirst, SEEK_SET);

        /* Read in records & strings buffer in one hit */

        if (read(fd, pclh->pcInfo, sz) == sz)
          rc=TRUE;
        else
        {
          free(pclh);
          pclh=NULL;
        }
      }

    }

    close(fd);
  }
  return rc;
}


  /* Dispose of a class info handle
   */

void ClassDispose()
{
  if (pclh)
    free(pclh);
}



  /* Return ptr to a class record
   */

CLSREC *ClassRec(int idx)
{
  if (ValidClassIndex(idx)) /* Do this manually in case struct has grown */
    return (CLSREC *)(((char *)pclh->pcInfo)+(idx*pclh->ussize));
  return pclh->pcInfo;            /* Return first record */
}

int ClassKeyIndex(word key)
{
  int   i;

  for (i=0; i<pclh->usn; ++i)
  {
    CLSREC *pc=ClassRec(i);
    if (pc->usKey==key)
      return i;
  }

  return -1;
}

  /* Convert a privilege 'key' to a privilege level
   */

word ClassKeyLevel(word key)
{
  int i=ClassKeyIndex(key);
  return (i==-1) ? (word)-1 : ClassRec(i)->usLevel;
}


  /* Convert priv symbol to a a class index
   */

int ClassAbbrevIndex(char *pszAbbrev)
{
  int   i, n;

  if (isdigit(*pszAbbrev) || *pszAbbrev=='-')
  {
    word usn = (word)atol(pszAbbrev);

    /* Assume it is a numeric privilege */

    return ClassLevelIndex(usn);
  }

  n=strcspn(pszAbbrev, " \t/,-=+<>()&|");
  for (i=0; i<pclh->usn; ++i)
  {
    char *cName = ClassAbbrev(i);
    char *cAlias = ClassAlias(i);

    if ((strnicmp(cName, pszAbbrev, n)==0 && cName[n]=='\0') ||
        (strnicmp(cAlias, pszAbbrev, n)==0 && cAlias[n]=='\0'))
    {
      return i;
    }
  }

  logit(log_invalid_acs, pszAbbrev);
  return -1;
}


word ClassLevel(char *pszAbbrev)
{
  if (isdigit(*pszAbbrev) || *pszAbbrev=='-')
    return (word)atol(pszAbbrev);

  return ClassAbbrevLevel(pszAbbrev);
}


  /* Return priv level for a abbrev symbol
   */

word ClassAbbrevLevel(char *pszAbbrev)
{
  int idx=ClassAbbrevIndex(pszAbbrev);

  return (idx==-1) ? (word)-1 : ClassRec(idx)->usLevel;
}


  /* Convert level to index
   * Note: this match is actually <= so an exact level is not required
   */

int ClassLevelIndex(word usLevel)
{
  int     i, j=-1;

  for (i=0; i<pclh->usn; i++)
  {
    CLSREC *pc=ClassRec(i);
    if (pc->usLevel > usLevel)
      break;
    j=i;
    if (pc->usLevel == usLevel)
      break;
  }

  return j;
}


  /* Return class information
   */

dword ClassGetInfo(int idx, int itype)
{
  dword rc;
  if (itype==CIT_NUMCLASSES)
    rc=pclh->usn;
  else
  {
    CLSREC *pcr=ClassRec(idx);

    switch (itype)
    {
      case CIT_DAY_TIME:      rc=pcr->usTimeDay;          break;
      case CIT_CALL_TIME:     rc=pcr->usTimeCall;         break;
      case CIT_DL_LIMIT:      rc=pcr->ulFileLimit;        break;
      case CIT_RATIO:         rc=pcr->usFileRatio;        break;
      case CIT_MIN_BAUD:      rc=(100L*pcr->usMinBaud);   break;
      case CIT_MIN_XFER_BAUD: rc=(100L*pcr->usFileBaud);  break;
      case CIT_MAX_CALLS:     rc=pcr->usCallsDay;         break;
      case CIT_FREE_RATIO:    rc=pcr->usFreeRatio;        break;
      case CIT_UPLOAD_REWARD: rc=pcr->usUploadReward;     break;
      case CIT_ACCESSFLAGS:   rc=pcr->ulAccFlags;         break;
      case CIT_MAILFLAGS:     rc=pcr->ulMailFlags;        break;
      case CIT_USERFLAGS:     rc=pcr->ulUsrFlags;         break;
      case CIT_LEVEL:         rc=pcr->usLevel;            break;
      case CIT_KEY:           rc=pcr->usKey;              break;
      case CIT_INDEX:         rc=idx;                     break;
      case CIT_OLDPRIV:       rc=pcr->usOldPriv;          break;
      default:                rc=0;/*logit(class_err);?*/ break;
    }
  }
  return rc;
}

/* Convert an ASCIIZ keys string to a bitmask */

static void SZKeyMask(char *pszKeys, dword *maskon, dword *maskoff)
{
  char *p;

  *maskon=0L;
  *maskoff=0L;

  /* Now test to see if the user needs to have certain keys on or off */

  if ((p=strchr(pszKeys, '/')) != NULL)
  {

    /* Scan through each key in turn */

    while (*++p)
    {
      unsigned char ch;
      dword *maskptr=maskon;

      if (*p=='!')  /* Not key */
      {
        maskptr=maskoff;
        ++p;
      }

      ch=(unsigned char)toupper(*p);

      if (ch >= '1' && ch <= '8')
        *maskptr |= (1L << (ch-'1'));
      else if (ch >= 'A' && ch <= 'X')
        *maskptr |= (1L << ((ch-'A')+8));
      else break;
    }
  }
}

static int _PrivOK(char *acstest, unsigned use_real_priv)
{
  int rc;
  int privop;
  char *equals;

  /* Test for "item=value" comparisons */

  equals = strchr(acstest, '=');

  if (equals)
  {

     /* See if it is actually a comparison operator */

    if (equals==acstest || equals[ 1] == '=' || equals[-1] == '<' ||
                           equals[-1] == '>' || equals[-1]=='!')
      ;
    else
    {
      int len = equals - acstest;

      Strip_Underscore(equals+1);

      if (strnicmp(acstest, "name", len)==0)
      {
        return eqstri(equals+1, usr.name);
      }
      else if (strnicmp(acstest, "alias", len)==0)
      {
        return eqstri(equals+1, usr.alias);
      }
      else
      {
        logit(log_invalid_acs, acstest);
        return FALSE;
      }
    }
  }

  privop=privGE;    /* Default to >= comparison */

  if (*acstest=='>')
  {
    privop=privGT;
    if (*(++acstest)=='=')
    {
      privop=privGE;
      ++acstest;
    }
  }
  else if (*acstest=='<')
  {
    privop=privLT;
    if (*(++acstest)=='=')
    {
      privop=privLE;
      ++acstest;
    }
    else if (*acstest=='>')
    {
      privop=privNE;
      ++acstest;
    }
  }
  else if (*acstest=='!')
  {
    privop=privNE;
    if (*(++acstest)=='=')  /* Superfluous */
      ++acstest;
  }
  else if (*acstest=='=')
  {
    privop=privEQ;
    if (*(++acstest)=='=')  /* Superfluous */
      ++acstest;
  }

  if (*acstest=='@')
  {
    use_real_priv=TRUE; /* Force use of real priv */
    ++acstest;
  }

  rc=TRUE;
  if (*acstest && *acstest != '/')    /* Blank priv, assume ok */
  {
    word compareto_priv=use_real_priv ? realpriv() : usr.priv;
    word compare_priv=ClassLevel(acstest);

    switch (privop)
    {
      default:
      case privGE:  rc=( compareto_priv >= compare_priv ); break;
      case privLE:  rc=( compareto_priv <= compare_priv ); break;
      case privGT:  rc=( compareto_priv >  compare_priv ); break;
      case privLT:  rc=( compareto_priv <  compare_priv ); break;
      case privEQ:  rc=( compareto_priv == compare_priv ); break;
      case privNE:  rc=( compareto_priv != compare_priv ); break;
    }
  }

    /* Finally, make sure that the keys match okay */

  if (rc)
  {
    dword maskon, maskoff;

    SZKeyMask(acstest,&maskon,&maskoff);
    rc=((usr.xkeys & maskon) == maskon) && ((usr.xkeys & maskoff) == 0);
  }

  return rc;
}


/* PrivOK - function used to check an ACS string to see if the current
 * user has enough privileges to be given access to the resource.
 */

int PrivOK(char *acstest, unsigned use_real_priv)
{
  char *acsdup;
  char *next;
  char *s;
  int  rel=0;

  int rc = TRUE;

  /* If not enough memory to duplicate the string, get out */

  if ((acsdup=alloca(strlen(acstest)+1))==NULL)
  {
    logit(log_badnm);
    return FALSE;
  }

  strcpy(acsdup,acstest);

  /* Find all of the qualifiers in the string */

  for (next = acsdup; next; )
  {
    char *p;

    s = next;

    if ((p=strpbrk(s, ",|&"))==NULL)  /* AND (,&) + OR (|) */
      next = NULL;
    else
    {
      rel=(*p=='|');  /* For OR relation */
      *p=0;
      next = p+1;
    }

    if (!_PrivOK(s, use_real_priv))
    {
      rc = FALSE; /* Switch default to FALSE */
      if (!rel)   /* If AND, then fail immediately */
        break;
    }
    else
    {
      rc = TRUE;  /* Switch default to TRUE */
      if (rel)    /* If OR, then return success immediately */
        break;
    }
  }

  return rc;
}


void ClassFlag(int idx, int which, dword fSet, dword fReset)
{
  if (ValidClassIndex(idx))
  {
    CLSREC * pcr=ClassRec(idx);
    switch (which)
    {
      case CIT_ACCESSFLAGS:
        pcr->ulAccFlags |= fSet;
        pcr->ulAccFlags &= ~fReset;
        break;
      case CIT_MAILFLAGS:
        pcr->ulMailFlags |= fSet;
        pcr->ulMailFlags &= ~fReset;
        break;
      case CIT_USERFLAGS:
        pcr->ulUsrFlags |= fSet;
        pcr->ulUsrFlags &= ~fReset;
        break;
    }
  }
}

char *privstr(word priv, char *buf)
{
  int ucls;
  char *prv=buf;

  ucls=ClassLevelIndex(priv);
  if (ClassGetInfo(ucls,CIT_LEVEL)==priv)
    prv=ClassAbbrev(ucls);
  else
    sprintf(buf,"%u",priv);

  return prv;
}

/* Convert an old priv level to a new */

word max3priv(int iPriv)
{
  int i;
  word j=ClassRec(0)->usLevel;           /* Assume lowest possible level */

  if (iPriv==HIDDEN)                             /* Automatic assumption */
    return (word)-1;

  for (i=0; i<pclh->usn; i++)
  {
    CLSREC *pc=ClassRec(i);
    int lvl=(short)pc->usOldPriv;

    if (lvl==iPriv)                                /* Got an exact match */
      return pc->usLevel;

    if (iPriv > lvl && pc->usLevel > j)     /* Find a close match anyway */
      j=pc->usLevel;
  }

  return j;
}


int ValidClassIndex(int idx)
{
  return ((int)idx >= 0 && idx < (int)(pclh)->usn);
}

char *ClassAbbrev(int idx)
{
  return ((pclh)->pHeap + (ClassRec(idx)->zAbbrev));
}


char *ClassDesc(int idx)
{
  return ((pclh)->pHeap + (ClassRec(idx)->zDesc));
}

char *ClassFile(int idx)
{
  return ((pclh)->pHeap + (ClassRec(idx)->zLoginFile));
}

char *ClassAlias(int idx)
{
  return ((pclh)->pHeap + (ClassRec(idx)->zAlias));
}

