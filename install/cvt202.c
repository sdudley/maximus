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

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#include "install.h"
#include "prog.h"
#include "cvt202.h"
#include "ffind.h"
#include "tui.h"
#include "tagapi.h"

#ifdef OS_2
#define INCL_DOS
#include "pos2.h"
#endif


/* Options from the main dialog box */

VWIN *wWorking;

static char szNewPrm[PATHLEN];
static char szEventName[PATHLEN];
static char szUserName[PATHLEN];
static char szTagName[PATHLEN];
static char szConfigName[PATHLEN];



/* Stuff for performing the conversion itself */

static char szCtlDelim[]=" \t\n";     /* Delimiters for all .ctl files */
CVTSTATE csState;                     /* Current converter state */
FILESTK afsStack[MAX_FILE_STACK];     /* Stack of files being processed */
FILESTK *pfsCur;                      /* Pointer to current file */
AREADATA ad;                          /* Info about this file/msg area */
SESSIONDATA sd;                       /* Info about the session section */
MENUDATA md;                          /* Info about the current menu */



/* Master table linking all of the translate and xlat tables to
 * their associated states.
 */

static TRANSLATESTATE ptsTables[MAX_TRANSLATE_TABLES];

/*****************************************************************************
                                  Event
 *****************************************************************************/

CVTSTATE HandleEventEvent(CVTPARM *pcp)
{
  char szOut[PATHLEN];
  int iWord;

  sprintf(szOut, "%s\t%s\t%s ",
          pcp->szWords[0],
          pcp->szWords[1],
          pcp->szWords[2]);

  for (iWord=3; pcp->szWords[iWord]; iWord++)
  {
    switch (toupper(*pcp->szWords[iWord]))
    {
      case 'E':
        sprintf(szOut + strlen(szOut),
               "exit=%d ",
               atoi(pcp->szWords[iWord]+1));
        break;

      case 'Y':
      {
        char *szSlash;

        int iBells = 1;
        int iMax = 1;
        int iTune = -1;
        char szTune[PATHLEN];

        iBells = atoi(pcp->szWords[iWord]+1);

        if ((szSlash = strchr(pcp->szWords[iWord], '/')) != NULL)
        {
          iMax = atoi(szSlash + 1);

          if ((szSlash = strchr(szSlash+1, '/')) != NULL)
            iTune = atoi(szSlash + 1);
        }

        if (iTune==-1)
          strcpy(szTune, "Random");
        else
          sprintf(szTune, "Yell%d", iTune);

        sprintf(szOut + strlen(szOut),
                "bells=%d maxyell=%d tune=%s",
                iBells,
                iMax,
                szTune);
        break;
      }

      default:
        strcat(szOut, pcp->szWords[iWord]);
        strcat(szOut, " ");
    }
  }

  strcat(szOut, "\n");
  strcpy(pcp->szLine, szOut);

  return csState;
}


static TRANSLATOR ttEvent[] =
{
  {"event",       HandleEventEvent},
  {NULL,          0}
};

static XLATTABLE xtEvent[]=
{
  {NULL,              NULL,                 0}
};


/*****************************************************************************
                                  System
 *****************************************************************************/

CVTSTATE HandleSystemEnd(CVTPARM *pcp)
{
  /* Add some new keywords */
  fprintf(pfsCur->fpOut,
          "\tSwap\n"
          "\tFile Access\tAccess\n"
          "\tFile Callers\tCallers\n"
          "\tMCP Pipe\t\\pipe\\maximus\\mcp\n"
          "\tMCP Sessions\t16\n");

  fprintf(pfsCur->fpOut,
          "End System Section\n\n");

  /* Now add a line outside of the section */

  strcpy(pcp->szLine, "Include Access.Ctl");

  (void)pcp;
  return CS_TOP;
}

CVTSTATE HandleSystemFile(CVTPARM *pcp)
{
  /* Change "file password user.bbs" to just "file password user" */

  if (eqstri(pcp->szWords[1], "password"))
  {
    char *psz = stristr(pcp->szLine, ".bbs");

    if (psz)
    {
      *psz='\n';
      psz[1]=0;
    }
  }

  return csState;
}

static TRANSLATOR ttSystem[] =
{
  {"file",            HandleSystemFile},
  {"end",             HandleSystemEnd},
  {NULL,          0}
};

static XLATTABLE xtSystem[]=
{
  {NULL,              NULL,                 0}
};


/*****************************************************************************
                                  Matrix
 *****************************************************************************/


CVTSTATE HandleMatrixEnd(CVTPARM *pcp)
{
  (void)pcp;
  fprintf(pfsCur->fpOut,
          "\tMessage Edit Ask\tLocalAttach\tNormal\n");

  return CS_TOP;
}

static TRANSLATOR ttMatrix[] =
{
  {"end",         HandleMatrixEnd},
  {NULL,          0}
};

static XLATTABLE xtMatrix[]=
{
  {NULL,              NULL,                 0}
};


/*****************************************************************************
                                    Menu
 *****************************************************************************/

CVTSTATE HandleMenuHeader(CVTPARM *pcp)
{
  if (eqstri(pcp->szWords[1], "message"))
    strcpy(pcp->szLine, "\tHeaderFile\t:M\\HeadMsg\n");
  else if (eqstri(pcp->szWords[1], "file"))
    strcpy(pcp->szLine, "\tHeaderFile\t:M\\HeadFile\n");
  else if (eqstri(pcp->szWords[1], "chat"))
    strcpy(pcp->szLine, "\tHeaderFile\t:M\\HeadChat\n");
  else if (eqstri(pcp->szWords[1], "change"))
    strcpy(pcp->szLine, "\tHeaderFile\t:M\\HeadChg\n");
  else if (eqstri(pcp->szWords[1], "none"))
    *pcp->szLine = 0;
  else
    Unknown("MenuHeader", pcp->szWords[1]);

  return csState;
}

CVTSTATE HandleHeaderFile(CVTPARM *pcp)
{
  if (!eqstri(md.szName, "reader"))
  {
    /* Comment out all existing HeaderFile lines, so that we can use our
     * custom menu MEX HeaderFiles. */

    strocpy(pcp->szLine+1, pcp->szLine);
    *pcp->szLine = '%';
  }

  return csState;
}

CVTSTATE HandleMenuDefault(CVTPARM *pcp)
{
  char *szName = NULL;
  int iWord;

  if (stristr(pcp->szStrippedLine, "area_change"))
  {
    int fGotArea = FALSE;

    for (iWord=0; pcp->szWords[iWord]; iWord++)
    {
      if (fGotArea)
      {
        if (eqstri(pcp->szWords[iWord], "file"))
        {
          szName = "File_Area";
          break;
        }
        else if (eqstri(pcp->szWords[iWord], "message"))
        {
          szName = "Msg_Area";
          break;
        }
      }
      else if (eqstri(pcp->szWords[iWord], "area_change"))
        fGotArea = TRUE;
    }
  }

  /* Now write the converted option to the output file */

  if (szName)
  {
    int i;

    if (iWord==1)
      fprintf(pfsCur->fpOut, "\t");

    for (i=0; i < iWord-1; i++)
      fprintf(pfsCur->fpOut, "%s ", pcp->szWords[i]);

    if (iWord != 1)
      fprintf(pfsCur->fpOut, "\t");

     fprintf(pfsCur->fpOut, "%s ", szName);

    for (i += 2; pcp->szWords[i]; i++)
      fprintf(pfsCur->fpOut, "%s ", pcp->szWords[i]);

    fprintf(pfsCur->fpOut, "\n");

    *pcp->szLine = 0;
  }

  return csState;
}

CVTSTATE HandleMenuEnd(CVTPARM *pcp)
{
  (void)pcp;

  /* Append this set of options to the message menu */

  if (eqstri(md.szName, "message"))
  {
    fprintf(pfsCur->fpOut,
            "NoDsp   Msg_Change                             Disgrace \"`46\"   ; Alt-C\n"
            "NoDsp   Read_Previous                          Disgrace \"`75\"   ; Left\n"
            "NoDsp   Read_Original                          Disgrace \"`115\"  ; Ctrl-Left\n"
            "NoDsp   Read_Next                              Disgrace \"`77\"   ; Right\n"
            "NoDsp   Read_Reply                             Disgrace \"`116\"  ; Ctrl-Right\n"
            "NoDsp   Msg_Reply                              Disgrace \"`16\"   ; Alt-Q\n"
            "NoDsp   Msg_Reply                              Disgrace \"`19\"   ; Alt-R\n"
            "NoDsp   Msg_Kill                               Disgrace \"`37\" \"=\"; Alt-K\n");

    fprintf(pfsCur->fpOut,
            "%%%%      Msg_Download_Attach                    Disgrace \"^Download Attaches\"\n"
            "%%%%      Msg_Reply_Area         .               Disgrace \"$Reply Elsewhere\"\n");
  }
  else if (eqstri(md.szName, "reader"))
  {
    fprintf(pfsCur->fpOut,
            "%%       Msg_Restrict                           Disgrace \"Restrict Date\"\n");
  }

  if (md.szName)
    free(md.szName);

  return CS_TOP;
}

static TRANSLATOR ttMenu[] =
{
  {"headerfile",  HandleHeaderFile},
  {"menuheader",  HandleMenuHeader},
  {"end",         HandleMenuEnd},
  {"*",           HandleMenuDefault},
  {NULL,          0}
};

static XLATTABLE xtMenu[]=
{
  {"contents",      "File_Contents",    0},
  {"download",      "File_Download",    0},
  {"locate",        "File_Locate",      0},
  {"newfiles",      "File_NewFiles",    0},
  {"upload",        "File_Upload",      0},
  {"type",          "File_View",        0},
  {"raw",           "File_Raw",         0},
  {"override_path", "File_Override",    0},
  {"enter_message", "Msg_Enter",        0},
  {"forward",       "Msg_Forward",      0},
  {"xport",         "Msg_Xport",        0},
  {"statistics",    "MEX\t\tM\\Stats",    0},
  {NULL,            NULL,               0}
};



/*****************************************************************************
                                  Colour
 *****************************************************************************/


CVTSTATE HandleColourStrip(CVTPARM *pcp)
{
  /* Removed the indicated colour specification */

  *pcp->szLine = 0;
  return csState;
}


CVTSTATE HandleColourEnd(CVTPARM *pcp)
{
  (void)pcp;
  return CS_TOP;
}

static TRANSLATOR ttColour[] =
{
  {"menu",        HandleColourStrip},
  {"hotflash",    HandleColourStrip},
  {"file",        HandleColourStrip},
  {"message",     HandleColourStrip},
  {"fsr",         HandleColourStrip},
  {"end",         HandleColourEnd},
  {NULL,          0}
};

static XLATTABLE xtColour[]=
{
  {NULL,              NULL,                 0}
};


/*****************************************************************************
                                  Language
 *****************************************************************************/

CVTSTATE HandleLanguageStrip(CVTPARM *pcp)
{
  /* Comment out all languages */

  *pcp->szLine = '%';
  return csState;
}


CVTSTATE HandleLanguageEnd(CVTPARM *pcp)
{
  (void)pcp;

  /* Set the default language to english */

  fprintf(pfsCur->fpOut, "        Language English\n");
  return CS_TOP;
}

static TRANSLATOR ttLanguage[] =
{
  {"language",    HandleLanguageStrip},
  {"end",         HandleLanguageEnd},
  {NULL,          0}
};

static XLATTABLE xtLanguage[]=
{
  {NULL,              NULL,                 0}
};



/*****************************************************************************
                                  Session
 *****************************************************************************/


CVTSTATE HandleSessionUpload(CVTPARM *pcp)
{
  /* Strip out "upload reward" or "upload .bbs" */

  if (eqstri(pcp->szWords[1], "reward") ||
      eqstri(pcp->szWords[1], ".bbs"))
  {
    *pcp->szLine = 0;
  }

  return csState;
}

CVTSTATE HandleSessionNo(CVTPARM *pcp)
{
  if (eqstri(pcp->szWords[1], "filesbbs"))
    *pcp->szLine = 0;

  return csState;
}

CVTSTATE HandleSessionStrip(CVTPARM *pcp)
{
  *pcp->szLine = 0;
  return csState;
}

CVTSTATE HandleSessionFormat(CVTPARM *pcp)
{
  /* Change "Format Msg/FileFormat ...%-9.9" to "...%-20" */

  if (eqstri(pcp->szWords[1], "MsgFormat") ||
      eqstri(pcp->szWords[1], "FileFormat"))
  {
    char *psz;
    static char szReplace[] = "%-20";

    while ((psz = strstr(pcp->szLine, "%-9.9")) != NULL)
    {
      strocpy(psz+4, psz+5);
      memmove(psz, szReplace, strlen(szReplace));
    }
  }

  return csState;
}



CVTSTATE HandleSessionArea(CVTPARM *pcp)
{
  if (eqstri(pcp->szWords[1], "data"))
  {
    sd.fWroteMarea = TRUE;
    strcpy(pcp->szLine, "\tMessageData\tMarea\n");
  }
  else if (eqstri(pcp->szWords[1], "index"))
  {
    sd.fWroteFarea = TRUE;
    strcpy(pcp->szLine, "\tFileData\tFarea\n");
  }
  else if (!eqstri(pcp->szWords[1], "change"))
    Unknown("area type", pcp->szWords[1]);

  return csState;
}

CVTSTATE HandleSessionBegin(CVTPARM *pcp)
{
  (void)pcp;

  fputs(pcp->szLine, pfsCur->fpOut);

  /* Insert new keywords into session section */

  fprintf(pfsCur->fpOut,
          "\tStage Path\tStage\n"
          "\tCheck\tRIP\n"
          "%%\tCheck\tANSI\n"
          "\tMaxMsgSize\t8192\n"
          "\tMin RIP Baud\t65535\n"
          "\tRIP Path\tRIP\n"
          "\tTrack Base\tTrk\n"
          "\tTrack View\tSysop\n"
          "\tTrack Modify\tSysop\n"
          "%%\tTrack Exclude\tTrkexcl.Bbs\n"
          "\tUses EntryHelp\tMisc\\MsgEntry\n"
          "\tUses HeaderHelp\tMisc\\HdrEntry\n"
          "%%\tUses Configure\tMisc\\Configure\n"
          "\tAttach Base\tAtt\n"
          "\tAttach Path\tAttaches\n"
          "\tAttach Archiver\tZIP\n"
          "\tKill Attach\tAsk Normal\n");
  *pcp->szLine=0;

  return CS_SESSION;
}

CVTSTATE HandleSessionEnd(CVTPARM *pcp)
{
  (void)pcp;

  /* Ensure that the msg/file data statements get written, even if prior
   * version was relying on defaults.
   */

  if (!sd.fWroteMarea)
    fprintf(pfsCur->fpOut, "\tMessageData\tMarea\n");

  if (!sd.fWroteFarea)
    fprintf(pfsCur->fpOut, "\tFileData\tFarea\n");

  fprintf(pfsCur->fpOut, "\n");
  return CS_TOP;
}

static TRANSLATOR ttSession[] =
{
  {"no",              HandleSessionNo},
  {"upload",          HandleSessionUpload},
  {"ratio",           HandleSessionStrip},
  {"userlist",        HandleSessionStrip},
  {"define",          HandleSessionStrip},
  {"format",          HandleSessionFormat},
  {"area",            HandleSessionArea},
  {"end",             HandleSessionEnd},
  {NULL,              0}
};

static XLATTABLE xtSession[]=
{
  {NULL,              NULL,                 0}
};



/*****************************************************************************
                                  Area
 *****************************************************************************/

CVTSTATE HandleAreaEnd(CVTPARM *pcp)
{
  (void)pcp;

  if (ad.fMsg)
  {
    fprintf(pfsCur->fpOut, "MsgArea %s\n", ad.szName);
    fprintf(pfsCur->fpOut, "\tACS\t\t%s\n", ad.szMsgAccess ? ad.szMsgAccess : "Hidden");

    if (ad.szMsgInfo)
      fprintf(pfsCur->fpOut, "\tDesc\t\t%s\n", ad.szMsgInfo);

    if (ad.szMsgMenuName)
      fprintf(pfsCur->fpOut, "\tMenuName\tMESSAGE %s\n", ad.szMsgMenuName);

    if (ad.szPath)
      fprintf(pfsCur->fpOut, "\tPath\t\t%s\n", ad.szPath);

    if (ad.szMsgOverride)
    {
      fprintf(pfsCur->fpOut, "\t%%Override\t\t<menu_name> <option_type> <acs> [<key>]\n");
      fprintf(pfsCur->fpOut, "\t%% CONVERT WARNING!  The above line must be converted manually,\n");
      fprintf(pfsCur->fpOut, "\t%% Original: 'MsgOverride %s'\n\n", ad.szMsgOverride);
    }

    if (ad.szOrigin)
    {
      fprintf(pfsCur->fpOut, "\tOrigin\t\t. . %s\n", ad.szOrigin);
      fprintf(pfsCur->fpOut, "\t%% CONVERT WARNING!  The above line may have to be manually\n"
                             "\t%% modified to use the correct origination address.\n\n");
    }

    if (ad.szMsgBarricade)
      fprintf(pfsCur->fpOut, "\tBarricade\tMESSAGE %s\n", ad.szMsgBarricade);

    if (ad.szMsgName)
      fprintf(pfsCur->fpOut, "\tTag\t\t%s\n", ad.szMsgName);

    if (ad.iRenumDays)
      fprintf(pfsCur->fpOut, "\tRenum Days\t%d\n", ad.iRenumDays);

    if (ad.iRenumMax)
      fprintf(pfsCur->fpOut, "\tRenum Max\t%d\n", ad.iRenumMax);

    /* Ensure that the area defaults to *.MSG format */

    if (!stristr(ad.szMsgStyle, "squish") && !stristr(ad.szMsgStyle, "*.msg"))
      strcat(ad.szMsgStyle, "*.MSG ");

    if (!stristr(ad.szMsgStyle, "pub") && !stristr(ad.szMsgStyle, "pvt"))
      strcat(ad.szMsgStyle, "Pub ");

    fprintf(pfsCur->fpOut, "\tStyle\t\t%s\n", ad.szMsgStyle);
    fprintf(pfsCur->fpOut, "End MsgArea\n\n");
  }

  if (ad.fFile)
  {
    fprintf(pfsCur->fpOut, "FileArea %s\n", ad.szName);
    fprintf(pfsCur->fpOut, "\tACS\t\t%s\n", ad.szFileAccess ? ad.szFileAccess : "Hidden");

    if (ad.szFileInfo)
      fprintf(pfsCur->fpOut, "\tDesc\t\t%s\n", ad.szFileInfo);

    if (ad.szDownload)
      fprintf(pfsCur->fpOut, "\tDownload\t%s\n", ad.szDownload);

    if (ad.szUpload)
      fprintf(pfsCur->fpOut, "\tUpload\t\t%s\n", ad.szUpload);

    if (ad.szFileList)
      fprintf(pfsCur->fpOut, "\tFileList\t%s\n", ad.szFileList);

    if (ad.szFileMenuName)
      fprintf(pfsCur->fpOut, "\tMenuName\tFILE %s\n", ad.szFileMenuName);

    if (ad.szFileOverride)
    {
      fprintf(pfsCur->fpOut, "\t%%Override\t\t<menu_name> <option_type> <acs> [<key>]\n");
      fprintf(pfsCur->fpOut, "\t%% CONVERT WARNING!  The above line must be converted manually,\n");
      fprintf(pfsCur->fpOut, "\t%% Original: 'FileOverride %s'\n\n", ad.szFileOverride);
    }

    if (ad.szFileBarricade)
      fprintf(pfsCur->fpOut, "\tBarricade\tFILE %s\n", ad.szFileBarricade);

    fprintf(pfsCur->fpOut, "End FileArea\n\n");
  }

  if (ad.szName)         free(ad.szName);
  if (ad.szMsgAccess)    free(ad.szMsgAccess);
  if (ad.szMsgMenuName)  free(ad.szMsgMenuName);
  if (ad.szMsgName)      free(ad.szMsgName);
  if (ad.szPath)         free(ad.szPath);
  if (ad.szOrigin)       free(ad.szOrigin);
  if (ad.szMsgOverride)  free(ad.szMsgOverride);
  if (ad.szMsgBarricade) free(ad.szMsgBarricade);

  if (ad.szFileInfo)     free(ad.szFileInfo);
  if (ad.szDownload)     free(ad.szDownload);
  if (ad.szUpload)       free(ad.szUpload);
  if (ad.szFileList)     free(ad.szFileList);
  if (ad.szFileAccess)   free(ad.szFileAccess);
  if (ad.szFileMenuName) free(ad.szFileMenuName);
  if (ad.szFileOverride) free(ad.szFileOverride);
  if (ad.szFileBarricade)free(ad.szFileBarricade);

  *pcp->szLine = 0;
  return CS_TOP;
}

CVTSTATE HandleAreaMsgInfo(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  ad.szMsgInfo = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}


CVTSTATE HandleAreaMsgAccess(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  ad.szMsgAccess = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaType(CVTPARM *pcp)
{
  ad.fMsg = TRUE;

  if (eqstri(pcp->szWords[1], "*.msg"))
    strcat(ad.szMsgStyle, "*.MSG ");
  else if (eqstri(pcp->szWords[1], "squish"))
    strcat(ad.szMsgStyle, "Squish ");
  else
    Unknown("area type", pcp->szWords[1]);

  return csState;
}

CVTSTATE HandleAreaRenum(CVTPARM *pcp)
{
  ad.fMsg = TRUE;

  if (eqstri(pcp->szWords[1], "days"))
    ad.iRenumDays = atoi(pcp->szWords[2] ? pcp->szWords[2] : "");
  else if (eqstri(pcp->szWords[1], "max"))
    ad.iRenumMax = atoi(pcp->szWords[2] ? pcp->szWords[2] : "");
  else
    Unknown("renum type", pcp->szWords[1]);

  return csState;
}

CVTSTATE HandleAreaMsgMenuName(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  ad.szMsgMenuName = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaMsgName(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  ad.szMsgName = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaMatrix(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Net ");
  ad.szPath = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaEcho(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Echo ");
  ad.szPath = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaConf(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Conf ");
  ad.szPath = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaLocal(CVTPARM *pcp)
{
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Local ");
  ad.szPath = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaPublic(CVTPARM *pcp)
{
  ad.fMsg = TRUE;

  if (eqstri(pcp->szWords[1], "and"))
    strcat(ad.szMsgStyle, "Pub Pvt ");
  else
    strcat(ad.szMsgStyle, "Pub ");

  return csState;
}

CVTSTATE HandleAreaPrivate(CVTPARM *pcp)
{
  (void)pcp;

  ad.fMsg = TRUE;

  if (eqstri(pcp->szWords[1], "and"))
    strcat(ad.szMsgStyle, "Pub Pvt ");
  else
    strcat(ad.szMsgStyle, "Pvt ");

  return csState;
}

CVTSTATE HandleAreaAlias(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Alias ");
  return csState;
}

CVTSTATE HandleAreaReadOnly(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "ReadOnly ");
  return csState;
}

CVTSTATE HandleAreaAnonymous(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "Anon ");
  return csState;
}

CVTSTATE HandleAreaHigh(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "HiBit ");
  return csState;
}

CVTSTATE HandleAreaNoNameKludge(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "NoNameKludge ");
  return csState;
}

CVTSTATE HandleAreaUseRealname(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  strcat(ad.szMsgStyle, "RealName ");
  return csState;
}

CVTSTATE HandleAreaOrigin(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  ad.szOrigin = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 3));
  return csState;
}

CVTSTATE HandleAreaMsgOverride(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  ad.szMsgOverride = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaMsgBarricade(CVTPARM *pcp)
{
  (void)pcp;
  ad.fMsg = TRUE;
  ad.szMsgBarricade = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaDownload(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szDownload = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaUpload(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szUpload = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileInfo(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileInfo = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileAccess(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileAccess = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileMenuName(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileMenuName = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileList(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileList = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileOverride(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileOverride = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}

CVTSTATE HandleAreaFileBarricade(CVTPARM *pcp)
{
  (void)pcp;
  ad.fFile = TRUE;
  ad.szFileBarricade = sstrdup(fchar(pcp->szStrippedLine, szCtlDelim, 2));
  return csState;
}


CVTSTATE HandleAreaObsolete(CVTPARM *pcp)
{
  *pcp->szLine = '%';
  fputs(pcp->szLine, pfsCur->fpOut);

  fprintf(pfsCur->fpOut,
          "%% Warning!  The above line contains a obsolete statement\n"
          "%% from Maximus version 1.x.  It must be converted manually.\n\n");

  return csState;
}

CVTSTATE HandleAreaApp(CVTPARM *pcp)
{
  /* Copy 'app' lines from input to output */

  fprintf(pfsCur->fpOut, pcp->szLine);
  return csState;
}

CVTSTATE HandleAreaPrivRelated(CVTPARM *pcp)
{
  *pcp->szLine = '%';
  fputs(pcp->szLine, pfsCur->fpOut);

  fprintf(pfsCur->fpOut,
          "%% Warning!  The above line contains a keyword which is not\n"
          "%% directly supported by Maximus version 3.  Instead, use the\n"
          "%% expanded power of the new Override statement to achieve the\n"
          "%% same functionality.\n");

  return csState;
}


static TRANSLATOR ttArea[]=
{
  {"msginfo",       HandleAreaMsgInfo},
  {"msgaccess",     HandleAreaMsgAccess},
  {"type",          HandleAreaType},
  {"renum",         HandleAreaRenum},
  {"msgmenuname",   HandleAreaMsgMenuName},
  {"msgname",       HandleAreaMsgName},
  {"matrix",        HandleAreaMatrix},
  {"netmail",       HandleAreaMatrix},
  {"echomail",      HandleAreaEcho},
  {"echo",          HandleAreaEcho},
  {"conf",          HandleAreaConf},
  {"conference",    HandleAreaConf},
  {"local",         HandleAreaLocal},
  {"public",        HandleAreaPublic},
  {"private",       HandleAreaPrivate},
  {"alias",         HandleAreaAlias},
  {"read-only",     HandleAreaReadOnly},
  {"anonymous",     HandleAreaAnonymous},
  {"high",          HandleAreaHigh},
  {"no",            HandleAreaNoNameKludge},
  {"use",           HandleAreaUseRealname},
  {"origin",        HandleAreaOrigin},
  {"msgoverride",   HandleAreaMsgOverride},
  {"msgbarricade",  HandleAreaMsgBarricade},

  {"download",      HandleAreaDownload},
  {"upload",        HandleAreaUpload},

  {"filelist",      HandleAreaFileList},
  {"fileinfo",      HandleAreaFileInfo},
  {"fileaccess",    HandleAreaFileAccess},
  {"filemenuname",  HandleAreaFileMenuName},
  {"fileoverride",  HandleAreaFileOverride},
  {"filebarricade", HandleAreaFileBarricade},

  {"override",      HandleAreaObsolete},
  {"access",        HandleAreaObsolete},
  {"menupath",      HandleAreaObsolete},

  {"app",           HandleAreaApp},
  {"application",   HandleAreaApp},

  {"twit",          HandleAreaPrivRelated},
  {"disgrace",      HandleAreaPrivRelated},
  {"limited",       HandleAreaPrivRelated},
  {"normal",        HandleAreaPrivRelated},
  {"worthy",        HandleAreaPrivRelated},
  {"privil",        HandleAreaPrivRelated},
  {"privel",        HandleAreaPrivRelated},
  {"favored",       HandleAreaPrivRelated},
  {"extra",         HandleAreaPrivRelated},
  {"clerk",         HandleAreaPrivRelated},
  {"asstsysop",     HandleAreaPrivRelated},
  {"sysop",         HandleAreaPrivRelated},

  {"end",           HandleAreaEnd},
  {NULL,          0}
};


static XLATTABLE xtArea[]=
{
  {NULL,              NULL,                 0}
};

/*****************************************************************************
                                  Top
 *****************************************************************************/

CVTSTATE HandleTopSystem(CVTPARM *pcp)
{
  (void)pcp;
  return CS_SYSTEM;
}

CVTSTATE HandleTopMatrix(CVTPARM *pcp)
{
  (void)pcp;
  return CS_MATRIX;
}

CVTSTATE HandleTopColour(CVTPARM *pcp)
{
  (void)pcp;
  return CS_COLOUR;
}

CVTSTATE HandleTopLanguage(CVTPARM *pcp)
{
  (void)pcp;
  return CS_LANGUAGE;
}

CVTSTATE HandleTopSession(CVTPARM *pcp)
{
  (void)pcp;
  return HandleSessionBegin(pcp);
}

/* HandleTopMenu
 *
 * Process a "Menu <name>" statement.
 */

CVTSTATE HandleTopMenu(CVTPARM *pcp)
{
  memset(&md, 0, sizeof md);
  md.szName = sstrdup(pcp->szWords[1]);
  return CS_MENU;
}


/* HandleTopArea
 *
 * Process an "Area <name>" statement.
 */

CVTSTATE HandleTopArea(CVTPARM *pcp)
{
  memset(&ad, 0, sizeof ad);
  ad.szName = sstrdup(pcp->szWords[1]);
  *pcp->szLine = 0;
  return CS_AREA;
}

/* HandleTopInclude
 *
 * This function handles one file being included from inside
 * another.
 */

CVTSTATE HandleTopInclude(CVTPARM *pcp)
{
  char *szFile;

//  printf("Handling include: 0=%s, 1=%s\n", pcp->szWords[0], pcp->szWords[1]);

  if (pfsCur+1 == afsStack + MAX_FILE_STACK)
  {
    printf("Error!  Too many nested includes.  (Tried to include '%s'.)\n",
           pcp->szWords[1]);
    return csState;
  }

  /* Increment the state, parse the file, and then return */

  pfsCur++;

  szFile = pcp->szWords[1];

  if (!fexist(szFile))
  {
    char szBase[PATHLEN];

    strcpy(szBase, szDirBase);
    strcat(szBase, szFile);

    if (fexist(szBase))
      szFile = szBase;
  }

  CvtFile(szFile);
  pfsCur--;

  return csState;
}


static TRANSLATOR ttTop[]=
{
  {"include",     HandleTopInclude},
  {"system",      HandleTopSystem},
  {"matrix",      HandleTopMatrix},
  {"menu",        HandleTopMenu},
  {"area",        HandleTopArea},
  {"colours",     HandleTopColour},
  {"session",     HandleTopSession},
  {"language",    HandleTopLanguage},
  {NULL,          0}
};


/* Global translate table for all states */

static XLATTABLE xtTop[]=
{
  {"Enter_Msg",       "Msg_Enter",          0},
  {NULL,              NULL,                 0}
};


/*****************************************************************************
                               Main Program
 *****************************************************************************/

dword SqHash(byte *f)
{
  dword hash=0, g;
  char *p;

  for (p=f; *p; p++)
  {
    hash=(hash << 4) + (dword)tolower(*p);

    if ((g=(hash & 0xf0000000L)) != 0L)
    {
      hash |= g >> 24;
      hash |= g;
    }
  }
  

  /* Strip off high bit */

  return (hash & 0x7fffffffLu);
}


/* set_status
 *
 * Display a message in the status window indicating what file we are
 * processing.
 */

static void set_status(char *pszName)
{
  char szMsg[PATHLEN];

  if (wWorking)
    WinClose(wWorking);

  sprintf(szMsg, "Converting file %s...", pszName);
  wWorking = WinStatus(szMsg);
}

void Unknown(char *szWhat, char *szValue)
{
  char msg[256];

  sprintf(msg, "Unknown %s on line %d of %s:\n'%s'",
         szWhat, pfsCur->iLine, pfsCur->szInName, szValue);

  WinInfo(msg);
}

char *fchar(char *str,char *delim,int wordno)
{
  char *s;

  if ((s=firstchar(str, delim, wordno))==NULL)
    return str + strlen(str);

  return s;
}

/* CvtInit
 *
 * Initializes the conversion program and sets up variables.
 */

static void CvtInit(void)
{
  /* Zero out the file stack and the translatestate stack */

  memset(afsStack, 0, sizeof afsStack);
  pfsCur = afsStack;

  csState = CS_TOP;

  /* Now set up the global table of translate tables */

  memset(ptsTables, 0, sizeof ptsTables);
  ptsTables[CS_TOP].ptt = ttTop;
  ptsTables[CS_TOP].pxt = xtTop;
  ptsTables[CS_TOP].fHaltOutput = FALSE;
  ptsTables[CS_SYSTEM].ptt = ttSystem;
  ptsTables[CS_SYSTEM].pxt = xtSystem;
  ptsTables[CS_SYSTEM].fHaltOutput = FALSE;
  ptsTables[CS_MATRIX].ptt = ttMatrix;
  ptsTables[CS_MATRIX].pxt = xtMatrix;
  ptsTables[CS_MATRIX].fHaltOutput = FALSE;
  ptsTables[CS_MENU].ptt = ttMenu;
  ptsTables[CS_MENU].pxt = xtMenu;
  ptsTables[CS_MENU].fHaltOutput = FALSE;
  ptsTables[CS_AREA].ptt = ttArea;
  ptsTables[CS_AREA].pxt = xtArea;
  ptsTables[CS_AREA].fHaltOutput = TRUE;
  ptsTables[CS_COLOUR].ptt = ttColour;
  ptsTables[CS_COLOUR].pxt = xtColour;
  ptsTables[CS_COLOUR].fHaltOutput = FALSE;
  ptsTables[CS_SESSION].ptt = ttSession;
  ptsTables[CS_SESSION].pxt = xtSession;
  ptsTables[CS_SESSION].fHaltOutput = FALSE;
  ptsTables[CS_EVENT].ptt = ttEvent;
  ptsTables[CS_EVENT].pxt = xtEvent;
  ptsTables[CS_EVENT].fHaltOutput = FALSE;
  ptsTables[CS_LANGUAGE].ptt = ttLanguage;
  ptsTables[CS_LANGUAGE].pxt = xtLanguage;
  ptsTables[CS_LANGUAGE].fHaltOutput = FALSE;
}

/* set_extension
 *
 * Add extension szNewExt to szName.
 */

static void near set_extension(char *szName, char *szNewExt)
{
  char *szLastDot;
  char *szLastPath;

  szLastDot = strrchr(szName, '.');
  szLastPath = strrstr(szName, "\\/:");

  if (szLastDot && (!szLastPath || (szLastPath && szLastPath < szLastDot)))
    szLastDot[1] = '\0';

  if (!szLastDot)
    strcat(szName, ".");

  strcat(szName, szNewExt);
}

/* Ensure that the user wants to overwrite a given file */

static int query_overwrite(char *szName)
{
  int rc;

  rc = WinGetYN(" WARNING!  The following file:\r\n\r\n"
                "   %s\r\n\r\n"
                " already exists.  Do you wish to\r\n"
                " overwrite it?  (Answering 'no' will\r\n"
                " abort the conversion.)",
                szName);

  return rc;
}

/* fatal_file_err
 *
 * Display a fatal error message because we could not access
 * a certain file.
 */

static void fatal_file_err(char *szErrFmt, char *szName)
{
  char szMsg[PATHLEN];
  strcpy(szMsg, " Fatal error ");
  sprintf(szMsg + strlen(szMsg), szErrFmt, szName);
  strcat(szMsg, ".");

  WinInfo(szMsg);
  WinExit(1);
}

/* StripComment
 *
 * This function strips the comment from a .ctl file line.  A comment
 * is anything that starts with a punctuation character before any
 * other alpha characters.
 */

void StripComment(char *szLine)
{
  char *p;

  p=strchr(szLine, '\n');

  if (p)
    *p=0;

  while (*szLine)
  {
    if (isalpha(*szLine))
      break;
    else if (ispunct(*szLine))
    {
      *szLine='\0';
      break;
    }

    szLine++;
  }
}


/* do_xlat
 *
 * Perform exact keyword substitutions based on the data in the given
 * translate table.
 */

static void do_xlat(CVTPARM *pcp, XLATTABLE *pxt)
{
  /* For each entry in the translate table... */

  while (pxt->szFrom)
  {
    char *psz = pcp->szLine - 1;

    while ((psz = stristr(psz + 1, pxt->szFrom)) != NULL)
    {
      int iLen = strlen(pxt->szFrom);

      /* Ensure that it is bounded by word delimiters */

      if ((psz==pcp->szLine || strchr(szCtlDelim, psz[-1]) || !psz[-1]) &&
          (strchr(szCtlDelim, psz[iLen]) || !psz[iLen]))
      {
        int iDiff = strlen(pxt->szTo) - strlen(pxt->szFrom);

        /* Copy it in-place with the replacement */

        if (iDiff > 0)
          strocpy(psz + iDiff, psz);
        else
          strocpy(psz + strlen(pxt->szTo), psz + strlen(pxt->szTo) + (-iDiff));

        memcpy(psz, pxt->szTo, strlen(pxt->szTo));

/*        printf("Xlat '%s' to '%s'\n", pxt->szFrom, pxt->szTo);*/
      }
    }

    pxt++;
  }
}




/* handle_keyword
 *
 * This function compares a keyword from the file against keywords in
 * the current translation table.  When it finds a match, it calls
 * the handler function for that keyword.
 */

static void handle_keyword(CVTPARM *pcp, TRANSLATOR *pttTable)
{
  TRANSLATOR *ptt;

  for (ptt=pttTable; ptt->szKeyword; ptt++)
  {
    if (eqstri(pcp->szKeyword, ptt->szKeyword) || *ptt->szKeyword=='*')
    {
/*      printf("Calling handler for '%s'\n", pcp->szKeyword);*/
      csState = (*ptt->kwh)(pcp);
      break;
    }
  }
}


/* CvtParse
 *
 * This function is responsible for parsing an entire include file and
 * processing each line correctly depending on the parser's state.
 */

static void CvtParse(void)
{
  char szTemp[MAX_CTL_LINE];
  char szLine[MAX_CTL_LINE];
  char szStrippedLine[MAX_CTL_LINE];
  char szKeyword[MAX_CTL_LINE];
  int iWord;
  CVTPARM cp;
  CVTSTATE csOld;

  while (fgets(szLine, MAX_CTL_LINE, pfsCur->fpIn))
  {
    pfsCur->iLine++;

    strcpy(szStrippedLine, szLine);
    StripComment(szStrippedLine);

/*    printf("%02d L%3d: '%-0.63s'\n", csState, pfsCur->iLine, szStrippedLine);*/

    getword(szStrippedLine, szKeyword, szCtlDelim, 1);

    memset(cp.szWords, 0, sizeof cp.szWords);
    iWord = 0;

    do
    {
      getword(szStrippedLine, szTemp, szCtlDelim, iWord+1);

      if (*szTemp)
        cp.szWords[iWord++] = sstrdup(szTemp);
      else
        cp.szWords[iWord++] = NULL;
    }
    while (*szTemp);

    /* Copy pointers to local vars into the cvtparm structure because other
     * called functions need to access/modify these arguments.
     */

    cp.szLine = szLine;
    cp.szStrippedLine = szStrippedLine;
    cp.szKeyword = szKeyword;

    /* Perform any straight keyword substitutions, first from the global
     * table, and then from the local table if necessary.
     */

    do_xlat(&cp, xtTop);

    if (ptsTables[csState].pxt)
      do_xlat(&cp, ptsTables[csState].pxt);

    /* Perform any custom keyword handling */

    csOld = csState;
    handle_keyword(&cp, ptsTables[csState].ptt);

    /* Free memory allocated to word array */

    for (iWord=0; cp.szWords[iWord]; iWord++)
      free(cp.szWords[iWord]);

    /* Write to output file, including any translations made from above */

    if (!ptsTables[csOld].fHaltOutput || csState != csOld)
      fprintf(pfsCur->fpOut, "%s", szLine);
  }
}


/* CvtFile
 *
 * This function is responsible for converting one file.  It may be
 * called recursively due to 'Include' statements.
 */

static void CvtFile(char *pszName)
{
  char szOldName[PATHLEN];
  set_status(pszName);

  strcpy(pfsCur->szInName, pszName);
  strcpy(pfsCur->szOutName, pszName);
  set_extension(pfsCur->szOutName, "$3$");

  if (fexist(pfsCur->szOutName) && !query_overwrite(pfsCur->szOutName))
  {
    WinInfo(" Conversion aborted! ");
    WinExit(1);
  }

  pfsCur->fpIn = fopen(pfsCur->szInName, "r");
  pfsCur->fpOut = fopen(pfsCur->szOutName, "w");
  pfsCur->iLine = 0;

  if (!pfsCur->fpIn)
    fatal_file_err("opening %s for read", pfsCur->szInName);

  if (!pfsCur->fpOut)
    fatal_file_err("opening %s for write", pfsCur->szOutName);

  CvtParse();

  fclose(pfsCur->fpOut);
  fclose(pfsCur->fpIn);

  /* Now rename the original to .v2 and make the new one have the same
   * name as the original.
   */

  strcpy(szOldName, pfsCur->szInName);
  set_extension(szOldName, "v2");
  unlink(szOldName);
  rename(pfsCur->szInName, szOldName);
  rename(pfsCur->szOutName, pfsCur->szInName);
}

void CvtConfig(char *szCfg, char *szPrm)
{
  FILE *fp;

  set_status(szCfg);

  if ((fp=fopen(szCfg, "a"))==NULL)
    fatal_file_err("opening %s for append", szCfg);

  fprintf(fp, "\nSET MAXIMUS=%s\n", szPrm);
#if 0 /* Maximus does this automatically */
  {
    char szBase[PATHLEN];
    char *p;

    strcpy(szBase, szDirBase);
    strcat(szBase, "MCP.EXE");

    fprintf(fp, "RUN=%s . \\PIPE\\MAXIMUS\\MCP 16 SERVER\n", szBase);
  }
#endif
  fclose(fp);

  if (wWorking)
  {
    WinClose(wWorking);
    wWorking = 0;
  }
}


/* CvtTag
 *
 * Convert a Maximus 2.x-format tag file to Maximus 3.0
 */

static void CvtTag(char *szTag)
{
  struct _mtagidx mti;
  struct _tagdata td;
  char szOutIdx[PATHLEN];
  char szOutDat[PATHLEN];
  int fdBbs, fdIdx, fdDat;

  set_status(szTag);

  strcpy(szOutIdx, szTag);
  strcpy(szOutDat, szTag);

  set_extension(szOutIdx, "idx");
  set_extension(szOutDat, "dat");

  /* Just skip tag conversion if the file doesn't exist */

  if ((fdBbs=shopen(szTag, O_RDONLY | O_BINARY))==-1)
    return;

  if ((fdIdx=sopen(szOutIdx, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                  SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    fatal_file_err("opening %s for write", szOutIdx);
  }

  if ((fdDat=sopen(szOutDat, O_CREAT | O_TRUNC | O_WRONLY | O_BINARY,
                  SH_DENYNONE, S_IREAD | S_IWRITE))==-1)
  {
    fatal_file_err("opening %s for write", szOutIdx);
  }

  while (read(fdBbs, (char *)&td, sizeof td)==sizeof td)
  {
    static char buf[512];

    /* Convert the old tag data into the new format */

    memset(buf, 0, sizeof buf);
    strcpy(buf, td.areas);

    if (*buf != 1)
    {
      memmove(buf+1, buf, strlen(buf)+1);
      *buf=1;
    }

    if (buf[strlen(buf)-1] != 1)
      strcat(buf, "\x01");

    mti.dwUserHash=SqHash(td.name);

    if (strlen(buf)==1)
      memset(&mti, 0, sizeof mti);
    else
    {
      mti.dwOffset=tell(fdDat);
      mti.dwUsed=strlen(buf);

      /* Round up to the nearest 128-byte multiple */

      mti.dwLen=(mti.dwUsed + MTAG_PAD_SIZE) / MTAG_PAD_SIZE * MTAG_PAD_SIZE;
      write(fdDat, buf, mti.dwLen);
    }

    write(fdIdx, (char *)&mti, sizeof mti);
  }

  close(fdDat);
  close(fdIdx);
  close(fdBbs);
}

/* CvtEvents
 *
 * This program converts all of the event files in the current directory.
 */

static void CvtEvents(char *szFilespec)
{
  struct _evtlist
  {
    struct _evtlist *next;
    char *szName;
  };

  struct _evtlist *pevList;
  struct _evtlist *pevNext;
  struct _evtlist *pev;
  FFIND *ff;
  char *pszSlash;
  char szName[PATHLEN];

  ff = FindOpen(szFilespec, 0);

  pszSlash = strrstr(szFilespec, ":\\/");

  if (pszSlash)
    pszSlash[1] = 0;

  csState = CS_EVENT;

  pevList = NULL;

  /* Create a linked list of all event files to be converted */

  if (ff)
  {
    do
    {
      strcpy(szName, pszSlash ? szFilespec : "");
      strcat(szName, ff->szName);

      if ((pev=malloc(sizeof(struct _evtlist))) != NULL)
      {
        pev->szName = strdup(szName);
        pev->next = pevList;
        pevList = pev;
      }

    }
    while (FindNext(ff)==0);

    FindClose(ff);
  }

  /* Now convert all of the files and free the associated memory */

  for (pev = pevList;
       pev;
       pevNext = pev->next, free(pev), pev=pevNext)
  {
    if (pev->szName)
    {
      CvtFile(pev->szName);
      free(pev->szName);
    }
  }
}



/* set_files_base
 *
 * Set up the default location for the config files
 */

void set_files_base(char *szBase)
{
  sprintf(szEventName,  "%sEvent*.Bbs", szBase);
  sprintf(szTagName,    "%sMtag.Bbs", szBase);
  sprintf(szUserName,   "%sUser.Bbs", szBase);
#ifdef OS_2
  sprintf(szConfigName, "%c:\\Config.Sys", (char)cBootDrive);
#else
  sprintf(szConfigName, "%c:\\Autoexec.Bat", (char)cBootDrive);
#endif
}



/* ValFile
 *
 * Tidy up the name of a non-base file
 */

MenuFunction(ValFile)
{
  NW(opt);

  fancy_str(opt->data);
  _TuiMenuOptNormal(opt);
  
  return 0;
}




/*
  123456789012345678901234567890123456789012345678901234567890
 1ÚÄ| Maximus 2.0 to Maximus 3.0 Conversion |ÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
 2³                                                        ³
 3³ Please select the operations that you wish to perform: ³
 4³                                                        ³
 5³ [X] Convert control files  [d:\max\max.ctl           ] ³
 6³ [X] Convert event files    [d:\max\event*.bbs        ] ³
 7³ [X] Convert user file      [d:\max\user.bbs          ] ³
 8³ [X] Convert tag file       [d:\max\mtag.bbs          ] ³
 9³ [X] Add to CONFIG.SYS      [c:\config.sys            ] ³
10³                                                        ³
 1³           ÚÄÄÄÄÄÄÄÄ¿ ÚÄÄÄÄÄÄÄÄ¿                        ³
 2³           ³   OK   ³ ³ Cancel ³                        ³
 3³           ÀÄÄÄÄÄÄÄÄÙ ÀÄÄÄÄÄÄÄÄÙ                        ³
 4³                                                        ³
 5ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
*/

TIT_DIALOG(dlgCvt, -1, -1, BORDER_SINGLE, 0, 58, 17, "´ Maximus 2.0 to Maximus 3.0 Conversion Ã")
  DLG_INF("Please select the operations that you wish to perform:", 0, 1)

  DLG_CHK("1; Extract " VERSION " ~files",    1, 3, &fExtractFiles)
  DLG_CHK("1; Convert c~ontrol files",        1, 4, &fCvtCtl)
  DLG_CHK("1; Convert ~event files",          1, 5, &fCvtEvt)
  DLG_CHK("1; Convert ~tag file",             1, 6, &fCvtTag)
#ifdef OS_2
  DLG_CHK("1; Add to CONFIG.~SYS",            1, 7, &fCvtConfig)
#else
  DLG_CHK("1; Add to AUTOEXEC.~BAT",          1, 7, &fCvtConfig)
#endif
  DLG_CHK("1; ~Recompile CTL/MEC/MEX/MAD",    1, 8, &fRecompile)

  DLG_STV("24:60;",                          31, 4, szCtlName,    ValFile)
  DLG_STV("24:60;",                          31, 5, szEventName,  ValFile)
  DLG_STV("24:60;",                          31, 6, szTagName,    ValFile)
  DLG_STV("24:60;",                          31, 7, szConfigName, ValFile)

  DLG_OK (";  ~OK  ",                        13,11)
  DLG_CAN(";~Cancel",                        26,11)
END_MENU




/* PerformConversion
 *
 * This function starts the conversion of all of the requested
 * files.
 */

void PerformConversion(void)
{
  static char szTemp[512];
  char *p;

  CvtInit();

  strcpy(szNewPrm, szCtlName);

  p = stristr(szNewPrm, ".ctl");

  if (p)
    strcpy(p, ".prm");
  else
    strcat(p, ".prm");

  if (fExtractFiles)
  {
    if (!DoUpgradeExtract())
      WinExit(1);
  }

  if (fCvtConfig)
    CvtConfig(szConfigName, szNewPrm);
  else
  {
    sprintf(szTemp,
            " WARNING!  You have selected to continue\r\n"
#ifdef OS_2
            " without updating your CONFIG.SYS for the\r\n"
#else
            " without updating your AUTOEXEC.BAT for the\r\n"
#endif
            " new version of Maximus.  You must add the\r\n"
            " following line manually:\r\n\r\n"

            "     SET MAXIMUS=%s",
            szNewPrm);

    WinInfo(szTemp);
  }

  if (fCvtCtl)
    CvtFile(szCtlName);

  if (fCvtEvt)
    CvtEvents(szEventName);

  if (fCvtTag)
    CvtTag(szTagName);

  if (fRecompile)
    recompile_system();

  if (wWorking)
  {
    WinClose(wWorking);
    wWorking = 0;
  }


  if (fCvtConfig)
  {
    WinInfo(" Conversion complete!  You must now\r\n"
            " reboot your system before running\r\n"
            " Maximus.\r\n\r\n"
#ifdef OS_2
            " You must also run \"CVTUSRP -P\" to convert\r\n"
#else
            " You must also run \"CVTUSR -P\" to convert\r\n"
#endif
            " your Maximus user file.\r\n\r\n"

            " Please read the README file for other\r\n"
            " important information on completing\r\n"
            " the conversion!");
  }
  else
  {
    WinInfo(" Conversion complete!");
  }
}


