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
#include "max_file.h"

#ifdef MEX

  /* Message area find functions */

  static void near fileareaexport(struct mex_farea *pfa, PFAH fas)
  {
    MexKillStructString(mex_farea, pfa, name);
    MexKillStructString(mex_farea, pfa, descript);
    MexKillStructString(mex_farea, pfa, downpath);
    MexKillStructString(mex_farea, pfa, uppath);
    MexKillStructString(mex_farea, pfa, filesbbs);
    MexKillStructString(mex_farea, pfa, barricade);
    StoreString(MexPtrToVM(pfa), struct mex_farea, name,      PFAS(fas,name));
    StoreString(MexPtrToVM(pfa), struct mex_farea, descript,  PFAS(fas,descript));
    StoreString(MexPtrToVM(pfa), struct mex_farea, downpath,  PFAS(fas,downpath));
    StoreString(MexPtrToVM(pfa), struct mex_farea, uppath,    PFAS(fas,uppath));
    StoreString(MexPtrToVM(pfa), struct mex_farea, filesbbs,  PFAS(fas,filesbbs));
    StoreString(MexPtrToVM(pfa), struct mex_farea, barricade, PFAS(fas,barricade));
    pfa->division=(fas->fa.attribs & FA_DIVBEGIN)
                  ? 1 : (fas->fa.attribs & FA_DIVEND)
                  ? 2 : 0;
    pfa->attribs=fas->fa.attribs;
  }

  static word near findfilearea(struct mex_farea * pfa, int direction)
  {
    FAH fa;

    memset(&fa, 0, sizeof fa);
    fa.fa.cbPrior = pmisThis->cbPriorFile;

    while ((direction ? AreaFileFindNext : AreaFileFindPrior )(pmisThis->hafFile,&fa,FALSE)==0)
    {
      pmisThis->cbPriorFile = fa.fa.cbPrior;

      if (!(fa.fa.attribs & FA_HIDDN) && PrivOK(FAS(fa, acs), TRUE))
      {
        fileareaexport(pfa, &fa);
        DisposeFah(&fa);
        return TRUE;
      }
    }
    AreaFileFindClose(pmisThis->hafFile);
    pmisThis->hafFile=0;
    DisposeFah(&fa);
    return FALSE;
  }

  word EXPENTRY intrin_fileareafindfirst(void)
  {
    MA ma;
    struct mex_farea * pfa;
    char * psz;
    int flags;

    MexArgBegin(&ma);
    if (pmisThis->hafFile)
    {
      AreaFileFindClose(pmisThis->hafFile);
      pmisThis->hafFile=0;
    }

    pmisThis->cbPriorFile = 0;
    pfa=MexArgGetRef(&ma);
    psz=MexArgGetString(&ma,FALSE);
    flags=MexArgGetWord(&ma);
    pmisThis->hafFile=AreaFileFindOpen(haf, (psz && *psz) ? psz : NULL, flags);
    if (pmisThis->hafFile==NULL)
      regs_2[0]=FALSE;
    else
    {
      if ((regs_2[0]=findfilearea(pfa,TRUE))!=0)
      {
        /* Found the area, and it it accessible & visible,
         * so we reset our find parameters to allow us to
         * find the next or prev sequential
         */
        AreaFileFindChange(pmisThis->hafFile,NULL,flags);
      }
    }
    if (psz)
      free(psz);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_fileareafindnext(void)
  {
    MA ma;
    struct mex_farea * pfa;

    MexArgBegin(&ma);
    pfa=MexArgGetRef(&ma);
    if (!pmisThis->hafFile)
      regs_2[0]=FALSE;
    else
      regs_2[0]=findfilearea(pfa,TRUE);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_fileareafindprev(void)
  {
    MA ma;
    struct mex_farea * pfa;

    MexArgBegin(&ma);
    pfa=MexArgGetRef(&ma);
    if (!pmisThis->hafFile)
      regs_2[0]=FALSE;
    else
      regs_2[0]=findfilearea(pfa,FALSE);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_fileareafindclose(void)
  {
    if (pmisThis->hafFile)
    {
      AreaFileFindClose(pmisThis->hafFile);
      pmisThis->hafFile=0;
    }
    return regs_2[0]=0;
  }  

  word EXPENTRY intrin_file_area(void)
  {
    File_Area();
    if (fah.heap)
    {
      fileareaexport(MexDSEG(pmisThis->vmaFarea), &fah);
      SetAreaName(usr.files, FAS(fah, name));
      MexKillStructString(mex_usr, pmisThis->pmu, files);
      StoreString(MexPtrToVM(pmisThis->pmu), struct mex_usr, files, FAS(fah,name));
    }
    return 0;
  }

  word EXPENTRY intrin_fileareaselect(void)
  {
    MA ma;
    FAH myfah;
    char * psz;
    BARINFO bi;

    memset(&myfah, 0, sizeof myfah);

    if (pmisThis->hafFile)
    {
      AreaFileFindClose(pmisThis->hafFile);
      pmisThis->hafFile=0;
    }
    MexArgBegin(&ma);
    psz=MexArgGetString(&ma,FALSE);
    regs_2[0]=FALSE;
    if (psz && *psz)
    {
      char temp[MAX_ALEN];

      strcpy(temp, psz);
      pmisThis->hafFile=AreaFileFindOpen(haf, temp, 0);
      if (pmisThis->hafFile==NULL)
        regs_2[0]=FALSE;
      else
      {
        int rc=AreaFileFindNext(pmisThis->hafFile,&myfah,FALSE);
        if (rc!=0)
        {
          char *p;

          strcpy(temp,usr.files);
          p=strrchr(temp,'.');
          if (p)
          {
            strcpy(p+1,psz);
            AreaFileFindReset(pmisThis->hafFile);
            AreaFileFindChange(pmisThis->hafFile, temp, 0);
            rc=AreaFileFindNext(pmisThis->hafFile,&myfah,FALSE);
          }
        }
        if (rc==0 &&
            !(myfah.fa.attribs & FA_HIDDN) &&
            ValidFileArea(NULL, &myfah, VA_VAL | VA_PWD | VA_EXTONLY, &bi) &&
            PopPushFileAreaSt(&myfah,&bi))
        {
          SetAreaName(usr.files, FAS(myfah, name));
          fileareaexport(MexDSEG(pmisThis->vmaFarea), &myfah);
          strcpy(usr.files, FAS(myfah,name));
          regs_2[0]=TRUE;
          MexKillStructString(mex_usr, pmisThis->pmu, files);
          StoreString(MexPtrToVM(pmisThis->pmu), struct mex_usr, files, FAS(myfah,name));
        }
        AreaFileFindClose(pmisThis->hafFile);
        pmisThis->hafFile=0;
        DisposeFah(&myfah);
      }
    }
    if (psz)
      free(psz);
    return MexArgEnd(&ma);
  }

#endif /* MEX */

