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
#include "modem.h"

#ifdef MEX

  /* Get string input from the user, using Max's internal input functions */

  word EXPENTRY intrin_input_str(void)
  {
    MA ma;
    word wLen;          /* Length of string currently in ia */
    IADDR ia;           /* Where to store string typed by user */
    word wType;         /* Options */
    word wCh;           /* Character */
    word wMax;          /* Max length of string */
    char *pszPrompt;    /* Optional prompt */
    word wRetVal;       /* Return value for this function */
    char *s;            /* String entered by user */

    wRetVal=(word)-1;

    MexArgBegin(&ma);

    /* Get the arguments */

    MexArgGetRefString(&ma, &ia, &wLen);
    wType=MexArgGetWord(&ma);
    wCh=MexArgGetByte(&ma);
    wMax=MexArgGetWord(&ma);
    pszPrompt=MexArgGetString(&ma, FALSE);

    if (wMax==0)  /* We need something here */
      wMax=256;

    /* Allocate memory for the string to be entered by user */

    if ((s=malloc(wMax+1)) != NULL)
    {
      Input(s, wType, wCh, wMax, *pszPrompt ? pszPrompt : NULL);
      wRetVal=strlen(s);

      /* Store the retrieved string in the spot asked for by the
       * user.
       */

      MexStoreByteStringAt(MexIaddrToVM(&ia), s, wRetVal);
      free(s);
    }

    regs_2[0]=wRetVal;

    if (pszPrompt)
      free(pszPrompt);

    return MexArgEnd(&ma);
  }


  /* Get one character of input from the user, allowing for the user
   * of hotkeys, prompts, and other Maximus-standard options.
   */

  word EXPENTRY intrin_input_ch(void)
  {
    MA ma;
    word wType;
    char *pszOptions;

    MexArgBegin(&ma);

    /* Get arguments */

    wType=MexArgGetWord(&ma);
    pszOptions=MexArgGetString(&ma, FALSE);

    /* Call input_char and store return value */

    regs_2[0] = Input_Char(wType, *pszOptions ? pszOptions : NULL);

    free(pszOptions);

    return MexArgEnd(&ma);
  }



  /* Get input from the user, based on a list of single-character
   * selections.
   */

  word EXPENTRY intrin_input_list(void)
  {
    MA ma;
    char *pszList;
    word wType;
    char *pszHelpFile;
    char *pszInvalidResponse;
    char *pszPrompt;

    MexArgBegin(&ma);

    /* Get arguments */

    pszList = MexArgGetString(&ma, FALSE);
    wType = MexArgGetWord(&ma);
    pszHelpFile = MexArgGetString(&ma, FALSE);
    pszInvalidResponse = MexArgGetString(&ma, FALSE);
    pszPrompt = MexArgGetString(&ma, FALSE);

    /* Call the internal GetListAnswer function */

    regs_2[0] = GetListAnswer(pszList,
                              pszHelpFile,
                              pszInvalidResponse,
                              wType,
                              percent_s,
                              pszPrompt);

    free(pszPrompt);
    free(pszInvalidResponse);
    free(pszHelpFile);
    free(pszList);

    return MexArgEnd(&ma);
  }


  /* Return TRUE if the user has pressed a key */

  word EXPENTRY intrin_kbhit(void)
  {
    regs_1[0]=(byte)Mdm_keyp();
    return 0;
  }



  /* Get a character from the user */

  word EXPENTRY intrin_getch(void)
  {
    vbuf_flush();
    regs_1[0]=(byte)Mdm_getcw();
    return 0;
  }


  /* Determine who hit the last key - sysop or remote
   */

  word EXPENTRY intrin_localkey(void)
  {
    regs_1[0]=(byte)Mdm_WasLocalKey();
    return 0;
  }


  word EXPENTRY intrin_keyboard(void)
  {
    MA ma;

    MexArgBegin(&ma);
    regs_2[0]=SetLocalKeystate(MexArgGetWord(&ma));
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_iskeyboard(void)
  {
    regs_2[0]=LocalKeystate();
    return 0;
  }

  word EXPENTRY intrin_time_check(void)
  {
    MA ma;

    MexArgBegin(&ma);
    regs_2[0]=!!do_timecheck;
    do_timecheck=!!MexArgGetWord(&ma);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_dcd_check(void)
  {
    MA ma;

    MexArgBegin(&ma);
    regs_2[0]=!(no_dcd_check);
    no_dcd_check=!MexArgGetWord(&ma);
    return MexArgEnd(&ma);
  }

  word EXPENTRY intrin_carrier(void)
  {
    regs_2[0]=local || real_carrier();
    return 0;
  }

  /* Reset the internal "more" prompt so that we appear to be at the top
   * of the page.
   */

  word EXPENTRY intrin_reset_more(void)
  {
    MA ma;
    byte *pbNonStop;

    MexArgBegin(&ma);
    pbNonStop=MexArgGetRef(&ma);

    display_line = display_col = 1;
    *pbNonStop = FALSE;

    return MexArgEnd(&ma);
  }

  /* Optionally do a more prompt, if we have reached the bottom of the
   * page.  Returns TRUE if we are to continue; FALSE otherwise.
   */

  word EXPENTRY intrin_do_more(void)
  {
    MA ma;
    byte *pbNonStop;
    char *colour;

    MexArgBegin(&ma);

    pbNonStop=MexArgGetRef(&ma);
    colour=MexArgGetString(&ma, FALSE);

    /* If the user wants a more prompt to be forced, do so here. */

    if (*pbNonStop==(byte)-1)
    {
      *pbNonStop=0;
      display_line=TermLength()+1;
    }

    regs_2[0] = !MoreYnBreak(pbNonStop, colour);

    free(colour);

    return MexArgEnd(&ma);
  }


#endif /* MEX */

