#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "prog.h"
#include "wincomm.h"

/* not yet */
/* These are "fudge routines" for the fd version of the comm driver. real code to come later. */

#ifdef _REENTRANT
# error write me first!
#endif

BOOL SetCommState(int hFile, LPDCB lpDCB)
{
  lpDCB->DCBlength = sizeof(*lpDCB);
  return TRUE;
}

BOOL GetCommState(int hFile, LPDCB lpDCB)
{
  LPDCB s = lpDCB;

  s->DCBlength = sizeof(*s);
  s->fNull = FALSE;
  s->fAbortOnError = FALSE;
  return TRUE;
}

BOOL SetCommMask(int hFile, DWORD dwEvtMask)
{
  return TRUE;
}

BOOL SetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  return TRUE;
}

BOOL GetCommTimeouts(OSCOMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  return TRUE;
}

BOOL SetupComm(OSCOMMHANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue)
{
  return TRUE;
}

BOOL SetCommBreak(OSCOMMHANDLE hFile)
{
  return TRUE;
}

BOOL ClearCommBreak(OSCOMMHANDLE hFile)
{
  return TRUE;
}

