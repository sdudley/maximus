/** @file 		wincomm.c
 *  @author 		Wes Garland
 *  @date		May 13 2003
 *  @description	Fudge routines/hooks for the comm library and asyncnt.c.
 *                      Designed to replace WinNT functions of the same name.
 *  @note		These routines only understand one comm port per
 *  			process. If we want more, we'll have to get fancier. But
 *  			somehow, I don't think Max will care..
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "prog.h"
#include "wincomm.h"

#ifdef _REENTRANT
# error not yet: needs more work (statics)
#endif

/* Static data which represents the first file descriptor 
 * we set anything for.  This way, Get/Set will return
 * reasonable values.
 */
static struct _DCB		static_DCB;
static int 			static_hFile = -1;
static struct _COMMTIMEOUTS	static_CT;

BOOL SetCommState(int hFile, LPDCB lpDCB)
{
  lpDCB->DCBlength = sizeof(*lpDCB);
  lpDCB->fNull = FALSE;
  lpDCB->fAbortOnError = FALSE;

  if ((static_hFile == hFile) || (static_hFile == -1))
  {
    static_hFile = hFile;
    static_DCB = *lpDCB;
    return TRUE;
  }

  return FALSE;
}

BOOL GetCommState(int hFile, LPDCB lpDCB)
{
  if (static_hFile == hFile)
  {
    *lpDCB = static_DCB;
    return TRUE;
  }

  return FALSE;
}

BOOL SetCommMask(int hFile, DWORD dwEvtMask)
{
  return TRUE;
}

/** Set the communications timeout values. These values are
 *  actually used by the UNIX mode comm drivers... but I
 *  don't think anything else does.
 *
 *  Recall that timeout values in here are 16 ticks = 1ms.
 */
BOOL SetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  if ((static_hFile == hFile) || (static_hFile == -1))
  { 
    static_hFile = hFile;
    static_CT = *lpCommTimeouts;
    return TRUE;
  }

  return FALSE;
}

/** Get the communications timeout values set by SetCommTimeouts() */
BOOL GetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  if (static_hFile == hFile)
  {
    *lpCommTimeouts = static_CT;
    return TRUE;
  }

  return FALSE;
}

BOOL SetupComm(OSCOMMHANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue)
{
  if (static_hFile == -1)
  {
    static_hFile = hFile;
    return TRUE;
  }

  return FALSE;
}

BOOL SetCommBreak(OSCOMMHANDLE hFile)
{
  return TRUE;
}

BOOL ClearCommBreak(OSCOMMHANDLE hFile)
{
  return TRUE;
}




