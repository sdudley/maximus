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

  typedef struct _statdata
  {
    char *pszName;                      /* Name of static data object */
    void *pvData;                       /* Pointer to the data area */
    dword dwSize;                       /* Size of the object */
    struct _statdata *next;             /* Pointer to next object in list */
  } STATDATA, *PSTATDATA;

  static PSTATDATA sdList = NULL;
  static PSTATDATA sdStringList = NULL;


  /* Create a piece of static data */

  word EXPENTRY intrin_create_static_data(void)
  {
    MA ma;
    PSTATDATA psd;
    char *pszName;
    dword size;
    word rc;

    MexArgBegin(&ma);
    pszName = MexArgGetString(&ma, FALSE);
    size = MexArgGetDword(&ma);
    rc = MexArgEnd(&ma);

    /* Invalid args: return -2 */

    if (!pszName || !size)
      regs_2[0] = -2;
    else
    {
      /* If data already exists, return -3 */

      for (psd=sdList; psd; psd=psd->next)
        if (strcmp(pszName, psd->pszName)==0)
          break;

      if (psd)
        regs_2[0] = -3;
      else
      {
        /* If not enough memory for request, return -1 */

        if ((size_t)size != size || (psd=malloc(sizeof *psd))==NULL)
          regs_2[0]=-1;
        else
        {
          psd->pszName = strdup(pszName);
          psd->pvData = malloc((size_t)size);
          psd->dwSize = (size_t)size;

          if (!psd->pvData || !psd->pszName)
          {
            if (psd->pvData)
              free(psd->pvData);

            if (psd->pszName)
              free(psd->pszName);

            free(psd);

            regs_2[0] = -1;
          }
          else
          {
            memset(psd->pvData, 0, (size_t)size);

            /* Append to beginning of list */

            psd->next = sdList;
            sdList = psd;

            /* Successful return of 0 */

            regs_2[0] = 0;
          }
        }
      }
    }

    if (pszName)
      free(pszName);

    return rc;
  }


  /* Destroy a previously-created instance of static data */

  word EXPENTRY intrin_destroy_static_data(void)
  {
    PSTATDATA psd, psdLast;
    MA ma;
    char *pszName;
    word rc;

    MexArgBegin(&ma);
    pszName = MexArgGetString(&ma, FALSE);
    rc = MexArgEnd(&ma);

    if (!pszName)
      regs_2[0] = -1;
    else
    {
      for (psd=sdList, psdLast=NULL;
           psd;
           psdLast=psd, psd=psd->next)
      {
        if (strcmp(psd->pszName, pszName)==0)
          break;
      }

      if (!psd)
        regs_2[0]=-1;
      else
      {
        /* Link the last node over this one */

        if (psdLast)
          psdLast->next = psd->next;
        else
          sdList = psd->next;

        /* Free the memory associated with this node */

        free(psd->pvData);
        free(psd->pszName);
        free(psd);

        regs_2[0] = 0;
      }

      free(pszName);
    }

    return rc;
  }


  /* Get a piece of static data */

  word EXPENTRY intrin_get_static_data(void)
  {
    MA ma;
    PSTATDATA psd;
    char *pszFindName;
    void *pvData;
    word rc;

    /* Get pointer to the data to be returned */

    MexArgBegin(&ma);
    pszFindName = MexArgGetString(&ma, FALSE);
    pvData = MexArgGetRef(&ma);
    rc = MexArgEnd(&ma);

    if (!pszFindName)
      psd = NULL;
    else
    {
      /* Try to find the requested piece of static data */

      for (psd = sdList; psd; psd=psd->next)
        if (strcmp(psd->pszName, pszFindName)==0)
          break;
    }

    free(pszFindName);

    /* If not found, return -1 */

    if (!psd)
      regs_2[0] = -1;
    else
    {
      memcpy(pvData, psd->pvData, (size_t)psd->dwSize);
      regs_2[0] = 0;
    }

    return rc;
  }


  word EXPENTRY intrin_set_static_data(void)
  {
    MA ma;
    PSTATDATA psd;
    char *pszFindName;
    void *pvData;
    word rc;

    /* Get pointer to the data to be returned */

    MexArgBegin(&ma);
    pszFindName = MexArgGetString(&ma, FALSE);
    pvData = MexArgGetRef(&ma);
    rc = MexArgEnd(&ma);

    if (!pszFindName)
    {
      regs_2[0] = -1;
      psd = NULL;
    }
    else
    {
      /* Try to find the requested piece of static data */

      for (psd = sdList; psd; psd=psd->next)
        if (strcmp(psd->pszName, pszFindName)==0)
          break;

      if (!psd)
        regs_2[0] = -1;
      else
      {
        memcpy(psd->pvData, pvData, (size_t)psd->dwSize);
        regs_2[0] = 0;
      }
    }

    free(pszFindName);
    return rc;
  }



  /* Create a static string object */

  word EXPENTRY intrin_create_static_string(void)
  {
    MA ma;
    PSTATDATA psd;
    char *pszName;
    word rc;

    MexArgBegin(&ma);
    pszName = MexArgGetString(&ma, FALSE);
    rc = MexArgEnd(&ma);

    /* Invalid args: return -2 */

    if (!pszName)
      regs_2[0] = -2;
    else
    {
      /* If data already exists, return -3 */

      for (psd=sdStringList; psd; psd=psd->next)
        if (strcmp(pszName, psd->pszName)==0)
          break;

      if (psd)
        regs_2[0] = -3;
      else
      {
        /* If not enough memory for request, return -1 */

        if ((psd=malloc(sizeof *psd))==NULL)
          regs_2[0]=-1;
        else
        {
          psd->pszName = strdup(pszName);
          psd->pvData = NULL;
          psd->dwSize = 0;

          if (!psd->pszName)
          {
            free(psd);
            regs_2[0] = -1;
          }
          else
          {
            /* Append to beginning of list */

            psd->next = sdStringList;
            sdStringList = psd;

            /* Successful return of 0 */

            regs_2[0] = 0;
          }
        }
      }
    }

    if (pszName)
      free(pszName);

    return rc;
  }


  /* Destroy a previously-created instance of static data */

  word EXPENTRY intrin_destroy_static_string(void)
  {
    PSTATDATA psd, psdLast;
    MA ma;
    char *pszName;
    word rc;

    MexArgBegin(&ma);
    pszName = MexArgGetString(&ma, FALSE);
    rc = MexArgEnd(&ma);

    if (!pszName)
      regs_2[0] = -1;
    else
    {
      for (psd=sdStringList, psdLast=NULL;
           psd;
           psdLast=psd, psd=psd->next)
      {
        if (strcmp(psd->pszName, pszName)==0)
          break;
      }

      if (!psd)
        regs_2[0]=-1;
      else
      {
        /* Link the last node over this one */

        if (psdLast)
          psdLast->next = psd->next;
        else
          sdStringList = psd->next;

        /* Free the memory associated with this node */

        if (psd->pvData)
          free(psd->pvData);

        free(psd->pszName);
        free(psd);

        regs_2[0] = 0;
      }

      free(pszName);
    }

    return rc;
  }


  /* Get a piece of static data */

  word EXPENTRY intrin_get_static_string(void)
  {
    char *pszFindName;
    PSTATDATA psd;
    IADDR where;
    word wLen;
    word rc;
    MA ma;

    /* Get pointer to the data to be returned */

    MexArgBegin(&ma);
    pszFindName = MexArgGetString(&ma, FALSE);
    MexArgGetRefString(&ma, &where, &wLen);
    rc = MexArgEnd(&ma);

    if (!pszFindName)
      psd = NULL;
    else
    {
      /* Try to find the requested piece of static data */

      for (psd = sdStringList; psd; psd=psd->next)
        if (strcmp(psd->pszName, pszFindName)==0)
          break;
    }

    free(pszFindName);

    /* If not found, return -1 */

    if (!psd)
      regs_2[0] = -1;
    else
    {
      /* Store the string in the requested location */

      MexStoreByteStringAt(MexIaddrToVM(&where),
                           psd->pvData,
                           (size_t)psd->dwSize);
      regs_2[0] = 0;
    }

    return rc;
  }


  word EXPENTRY intrin_set_static_string(void)
  {
    MA ma;
    PSTATDATA psd;
    char *pszFindName;
    void *pvData;
    IADDR where;
    word wLen;
    word rc;

    /* Get pointer to the data to be returned */

    MexArgBegin(&ma);
    pszFindName = MexArgGetString(&ma, FALSE);
    pvData = MexArgGetNonRefString(&ma, &where, &wLen);
    rc = MexArgEnd(&ma);

    if (!pszFindName)
    {
      regs_2[0] = -1;
      psd = NULL;
    }
    else
    {
      /* Try to find the requested piece of static data */

      for (psd = sdStringList; psd; psd=psd->next)
        if (strcmp(psd->pszName, pszFindName)==0)
          break;

      if (!psd)
        regs_2[0] = -1;
      else
      {
        /* Now reallocate enough memory to hold the string */

        if (psd->pvData)
          free(psd->pvData);

        psd->dwSize = 0;

        if ((psd->pvData = malloc(wLen+1))==NULL)
          regs_2[0] = -2;
        else
        {
          memcpy(psd->pvData, pvData, wLen);
          psd->dwSize = wLen;
          regs_2[0] = 0;
        }
      }
    }

    free(pszFindName);
    return rc;
  }

#endif /* MEX */

