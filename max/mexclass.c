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

#include "mexall.h"

#ifdef MEX


  /* Return class information */

  word EXPENTRY intrin_class_info(void)
  {
    MA ma;
    word priv;
    int iClassInfoType;

    MexArgBegin(&ma);

    priv=(word)MexArgGetWord(&ma);
    iClassInfoType=(int)MexArgGetWord(&ma);
    if (iClassInfoType==65535)  /* 16->32 conversion */
      iClassInfoType=-1;
    if (iClassInfoType != -1)
    {
      if (iClassInfoType & 0x8000)
        iClassInfoType &= ~0x8000;
      else  /* Default: convert priv to class index */
        priv=ClassLevelIndex(priv);
    }
    regs_4[0]=ClassGetInfo(priv, iClassInfoType);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_class_abbrev(void)
  {
    MA ma;
    word priv;

    MexArgBegin(&ma);
    priv=(word)MexArgGetWord(&ma);
    MexReturnString(ClassAbbrev(ClassLevelIndex(priv)));
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_class_name(void)
  {
    MA ma;
    word priv;

    MexArgBegin(&ma);
    priv=(word)MexArgGetWord(&ma);
    MexReturnString(ClassDesc(ClassLevelIndex(priv)));
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_class_loginfile(void)
  {
    MA ma;
    word priv;

    MexArgBegin(&ma);
    priv=(word)MexArgGetWord(&ma);
    MexReturnString(ClassFile(ClassLevelIndex(priv)));
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_class_to_priv(void)
  {
    MA ma;
    char *s;

    MexArgBegin(&ma);
    s=MexArgGetString(&ma, FALSE);
    regs_2[0]=0;
    if (s)
    {
      regs_2[0]=ClassAbbrevLevel(s);
      free(s);
    }
    return MexArgEnd(&ma);
  }

#endif /* MEX */

