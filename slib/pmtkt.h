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

/****************************************************************************
 * PMTKT.H                                                                  *
 *                                                                          *
 *  This header file contains the declarations of some of the undocumented  *
 *  toolkit functions located in the PMTKT.DLL dynamic link library.        *
 ****************************************************************************/

#if defined(__MSC__)
    #pragma comment( lib,"pmtkt.lib" )
#elif defined(__WATCOMC__)
    #pragma library ("pmtkt.lib")
#endif

/****************************************************************************
 *  Misc constants                                                          *
 ****************************************************************************/
    #define MAX_FNAME_LEN     80 /* maximum file name length */
    #define MAXMESSAGELENGTH 128 /* maximum size of a msg */
/****************************************************************************/


/****************************************************************************
 *  String IDs                                                              *
 ****************************************************************************/
    #define IDS_MERGE1      0      /* merge string (%%) */
    #define IDS_IFN         1      /* %% is not a valid filename. */
    #define IDS_FNF         2      /* %% not found - Create new file? */
    #define IDS_REF         3      /* Replace existing %%? */
    #define IDS_SCC         4      /* %% has changed. Save current changes? */
    #define IDS_EOF         5      /* Error opening %% */
    #define IDS_ECF         6      /* Error creating %% */
/****************************************************************************/


/****************************************************************************
 * Return values from WtiDlgFile                                            *
 ****************************************************************************/
    #define TDF_ERRMEM   0
    #define TDF_INVALID  1
    #define TDF_NOOPEN   2
    #define TDF_NEWOPEN  3
    #define TDF_OLDOPEN  4
    #define TDF_NOSAVE   5
    #define TDF_NEWSAVE  6
    #define TDF_OLDSAVE  7
/****************************************************************************/


/****************************************************************************
 * Flags for WtiOpenFile                                                    *
 ****************************************************************************/
    #define OF_REOPEN    0x8000
    #define OF_EXIST     0x4000
    #define OF_CREATE    0x1000
    #define OF_PARSE     0x0100
    #define OF_READ      0x0080
    #define OF_WRITE     0x0040
    #define OF_READWRITE 0x0020
/****************************************************************************/


/****************************************************************************
 *  Definition of DLF data type used to pass data to WtiDlgFile             *
 ****************************************************************************/
    typedef struct _DLF {
        USHORT   rgbAction;             /* action usType:  eg. DLG_OPEN   */
        USHORT   rgbFlags;              /* open file flAttributes         */
        PHFILE   phFile;                /* file handle                    */
        PSZ      pszExt;                /* default file extension "\\.ext"*/
        PSZ      pszAppName;            /* application name               */
        PSZ      pszTitle;              /* panel title or NULL            */
        PSZ      pszInstructions;       /* panel instructions or NULL     */
        CHAR szFileName[MAX_FNAME_LEN]; /* relative file name             */
        CHAR szOpenFile[MAX_FNAME_LEN]; /* full path name of current file */
        CHAR szLastWild[MAX_FNAME_LEN]; /* last relative wild card name   */
        CHAR szLastFile[MAX_FNAME_LEN]; /* last relative file name    */
        } DLF;
    typedef DLF FAR *PDLF;

/* Action value for WtiDlgFile, the values may be ORed, except DLG_OPENDLG */
/* and DLG_SAVEDLG. */
    #define DLG_OPENDLG 0x00        /* Use the Open dialog box. */
    #define DLG_SAVEDLG 0x01        /* Use the Save (As) dialog box */
    #define DLG_NOOPEN  0x02        /* Don't open the file we selected */
    #define DLG_HELP    0x04        /* The dialog box supports Help */

/* flAttributes used to filter entries provided in directory and file */
/* list boxes.                                                        */
    #define ATTRDIRLIST  0x4010 /* list avail. drives & dirs in dir list box */
    #define ATTRDRIVE    0x4000 /* list avail. drives in dir list box        */
    #define ATTRDIR      0x0010 /* list avail. dirs in dir list box          */
    #define ATTRNORMAL   0x0000 /* list normal files in file list box        */
    #define ATTRREADONLY 0x0001 /* list read-only files in file list box     */
    #define ATTRHIDDEN   0x0002 /* list hidden files in file list box        */
    #define ATTRSYSTEM   0x0004 /* list system files in file list box        */
    #define ATTRARCHIVED 0x0020 /* list archived files in file list box      */
/****************************************************************************/


/****************************************************************************/
 extern USHORT APIENTRY WtiAlertBox( HWND hwnd, USHORT idMes,
                                     PSZ  pszText1, PSZ pszText2,
                                     USHORT idHelp, USHORT flStyle );
    /* Purpose          This function merges sz[idMes] and szText1 and
     *                  displays a message box using wStyle.
     *
     * Parameters       hwnd contains the handle of the owner window.
     *
     *                  idMes contains contains one of the following
     *                  string IDs:
     *
     *                  IDS_MERGE1 ... merge string (%%)
     *                  IDS_IFN    ... %% is not a valid filename.
     *                  IDS_FNF    ... % not found - Create new file?
     *                  IDS_REF    ... Replace existing %%?
     *                  IDS_SCC    ... %% has changed. Save current changes?
     *                  IDS_EOF    ... Error opening %%
     *                  IDS_ECF    ... Error creating %%
     *
     *                  pszText1 is a pointer to the null terminated string
     *                  that is to replace the %% characters in the message
     *                  identified by the idMes parameter.
     *
     *                  pszText2 is a pointer to the null terminated string
     *                  that contains the message box caption.
     *
     *                  idHelp contains the message box id. This id is passed
     *                  to the HK_HELP hook if a WM_HELP message is received
     *                  by the message box window. Set this value to 0 if
     *                  you don't want the message box to have a help button.
     *
     *                  flStyle contains the flags used to specify the type
     *                  of message box to be created. See the documentation
     *                  for the WinMessageBox function for a description
     *                  of the message box style flags.
     *
     * Return Value     The return value indicates the user's response to
     *                  the message box. See the documentation for the
     *                  WinMessageBox function for a list of return codes.
     */

 extern BOOL APIENTRY WtiMergeStrings( PSZ pszSrc,PSZ pszMerge,PSZ pszDst );
    /* Purpose          This function scans szSrc for "%%". If found, it
     *                  then inserts the string szMerge at that point. The
     *                  rest of szSrc is then appended.
     *
     * Parameters       pszSrc is a pointer to the source string.
     *
     *                  pszMerge is a pointer to the string that is to
     *                  replace "%%" in the source string.
     *
     *                  pszDst is a pointer to the location where the
     *                  merged string is to be stored. Note that pszDst
     *                  must point to a memory buffer that is at least
     *                  MAXMESSAGELENGTH bytes long.
     *
     * Return Value     The function returns a value of TRUE if it does
     *                  a merge, false otherwise.
     */

 extern USHORT APIENTRY WtiDlgFile( HWND hwnd,PDLF pdlf );
    /* Purpose          This function invokes either an Open or Save
     *                  dialog box.
     *
     * Parameters       hwnd contains the handle of the owner window.
     *
     *                  pdlf is a pointer to a structure of type DLF
     *                  that is used to pass initialization data to
     *                  the dialog box.
     *
     * Return Value     The function returns one of the following
     *                  exit codes:
     *
     *                      TDF_INVALID - Library error (internal error),
     *                      TDF_ERRMEM  - Out of memory error
     *                      TDF_NOOPEN  - User hits cancel
     *
     *                  specific to DLG_OPENDLG:
     *                      TDF_NEWOPEN - Created new file
     *                      TDF_OLDOPEN - Opened existing file
     *                      In both of the above cases, the file is opened
     *                      in read-only mode.
     *
     *                  specific to DLG_SAVEDLG:
     *                      TDF_NEWSAVE - user wants to save to a new file
     *                      TDF_OLDSAVE - user wants to save over existing file
     *                      In the above cases, the file is opened in
     *                      write-only mode.
     *
     *                  specific to DLG_NOOPEN:
     *                      TDF_NEWSAVE - user wants to save to a new file
     *                      TDF_OLDSAVE - user wants to save over existing file
     *                      In the above cases, the file is not opened.
     */

 extern VOID APIENTRY WtiLFillStruct( PVOID pSrc,USHORT cb,BYTE fillByte );
    /* Purpose          This function fills the specified structure with
     *                  the byte value passed in the fillByte parameter.
     *
     * Parameters       pSrc is a pointer to the structure to be filled.
     *
     *                  cb is the size of the structure in bytes.
     *
     *                  fillByte contains the fill value.
     */

 extern VOID APIENTRY WtiLCopyStruct( PVOID pSrc,PVOID pDst,USHORT cb );
    /* Purpose          This function copies the contents of the
     *                  source structure into the destination structure.
     *
     * Parameters       pSrc is a pointer to the source structure.
     *
     *                  pDst is a pointer to the destination structure.
     *
     *                  cb is the structure size in bytes.
     */

 extern int APIENTRY WtiLStrLen( PSZ pszStr );
    /* Purpose          This function operates the same as strlen
     *                  except with long ptrs.
     */

 extern VOID APIENTRY WtiLStrCat( PSZ pszDst,PSZ pszSrc );
    /* Purpose          This function operates the same as strcat
     *                  except with long ptrs.
     */

 extern int APIENTRY WtiLStrCmp( PSZ pszStr1,PSZ pszStr2 );
    /* Purpose          This function operates the same as strcmp
     *                  except with long ptrs.
     */

 extern VOID APIENTRY WtiLStrCpy( PSZ pszDst,PSZ pszSrc );
    /* Purpose          This function operates the same as strcpy
     *                  except with long ptrs.
     */

 extern VOID APIENTRY WtiAddExt( PSZ pszDst,PSZ pszExt );
    /* Purpose          This function adds the extension to a
     *                  file name if it is missing.
     *
     * Parameters       pszDst points to the input file name.
     *
     *                  pszExt points to the extension to be added.
     */

 extern PSZ APIENTRY WtiFileInPath( PSZ pszPath );
    /* Purpose          This function returns a pointer to the
     *                  filename part of the given path string.
     */

 extern BOOL APIENTRY WtiOpenFile( PSZ pszFile,PHFILE phFile,
                                   PSZ pszOpenFile,USHORT wMode );
    /* Purpose          This function parses the input filename into
     *                  a fully expanded file name. Then, depending on
     *                  the setting of the wMode parameter, the function
     *                  will attempt to open the file.
     *
     * Parameters       pszFile is a pointer to a string containing
     *                  the name of the file to be opened.
     *
     *                  phFile is a pointer to the location where the
     *                  file handle for the newly opened file is to
     *                  be stored.
     *
     *                  pszOpenFile is a pointer to the memory location
     *                  where the fully expanded file name is to be
     *                  stored.
     *
     *                  wMode is one of the following constants:
     *
     *                      OF_READ:      open file for reading only
     *                      OF_WRITE:     open file for writing only
     *                      OF_READWRITE: open file for reading and writing
     *                      OF_CREATE:    create the file if it does not exist
     *                      OF_REOPEN:    open file using info in reopen buffer
     *                      OF_EXIST:     test file existence
     *                      OF_PARSE:     parse file name, with no other action
     *
     * Return Value     The function returns TRUE if the operation is
     *                  successful, FALSE otherwise.
     */

 extern ULONG APIENTRY WtiGetTextExtent( HPS hps, PCH pchStr, USHORT cch );
    /* Purpose          This function calculates the dimensions of the
     *                  rectangle that would be occupied by the input
     *                  text string.
     *
     * Parameters       hps is a handle to a presentation space.
     *
     *                  pchStr is a pointer to the input text string.
     *
     *                  cch contains the length of the input string.
     *
     * Return Value     This function returns the x extent in the
     *                  low order word and the y extent in the
     *                  high order word.
     */
