/**
 * @file 	wincomm.c	WinNT-style Comm functions for UNIX
 * @version	$Id: wincomm.c,v 1.6 2004/06/07 17:41:37 paltas Exp $
 * @author 	Wes Garland
 * @date	May 13 2003
 * @description	Fudge routines/hooks for the comm library and asyncnt.c.
 * 		Designed to replace WinNT functions of the same names.
 *  
 * $Log: wincomm.c,v $
 * Revision 1.6  2004/06/07 17:41:37  paltas
 * Fixed dynamic port speed
 *
 * Revision 1.5  2003/07/05 01:03:43  wesgarland
 * Robustification: Do not core if we are passed NULL handles
 *
 * Revision 1.4  2003/06/30 05:25:33  wesgarland
 * Minor silly errors corrected
 *
 * Revision 1.3  2003/06/29 20:48:59  wesgarland
 * Changes made to allow pluggable communications module. Code in not currently
 * pluggable, but "guts" will be identical to pluggable version of telnet
 * and raw IP plugins.
 *
 * Changed representation of COMMHANDLE (and deprecated OSCOMMHANDLE) in wincomm
 * code (pseudo Win32 communications API), to allow the potential for multiple
 * comm handles, better support UNIX comm plug-ins, etc.
 *
 * Added functions to "Windows" communications API which are normally handled by
 * the assignment operator -- those are possible under Win32 because the
 * underlying representation by which all comm data is accessed (e.g. word
 * length, parity, buffer sizes) is an integer (not unlike a UNIX file
 * descriptor), but under UNIX a COMMHANDLE is an incomplete struct pointer.
 * Using these routines instead of "inside" knowledge should allow new code
 * written for UNIX to be backported to Windows (and maybe other OSes) easily.
 *
 * This check-in is "barely tested" and expected to have bugs.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "prog.h"
#include "wincomm.h"

/** UNIX approximation for information referenced in Windows by
 *  an hfComm/COMMHANDLE. In Windows, this is basically like a
 *  file descriptor (HFILE) with the extra settings being magically
 *  returned by some internal table in the kernel. We abstract this
 *  away from a plain file descriptor for two reasons:
 *
 *  1. To contain the required information without keeping track
 *     of some magic internal table
 *
 *  2. To insure that we use the correct functions to coerce one
 *     type from another, rather than casting. This forces us
 *     (at compile time, hopefully) to use the correct routines
 *     for opening/closing a comm handle.
 *
 *  The extra information in Windows is somewhat like like ioctl
 *  or the POSIX termios details the tty device driver holds;
 *  by they are not quite the same. So getting them from the
 *  structures, and setting them to the structures + termios
 *  gives us a good, portable implementation -- across unices
 *  -and- other platforms.
 */
struct _COMMHANDLE
{
  int 			fd;		/**< UNIX file descriptor */
  struct _DCB		DCB;		/**< Windows DCB struct (data control block?) */
  struct _COMMTIMEOUTS	CT;		/**< Windows communications timeout structure */
  size_t		txBufSize;	/**< Transmit Buffer Size */
  size_t		rxBufSize;	/**< Receive Buffer Size */
};

BOOL SetCommState(hfComm hFile, LPDCB lpDCB)
{
  if (!hFile)
    return FALSE;

  lpDCB->DCBlength 	= sizeof(*lpDCB);
  lpDCB->fNull 		= FALSE;
  lpDCB->fAbortOnError 	= FALSE;

  hFile->DCB = *lpDCB;

  if(lpDCB->fDtrControl == DTR_CONTROL_ENABLED)
    ModemRaiseDTR(hFile->fd, lpDCB->BaudRate, lpDCB->fRtsControl ? RTS_CONTROL_ENABLE : 0);
  else
    ModemLowerDTR(hFile->fd);


  return TRUE;
}

BOOL GetCommState(hfComm hFile, LPDCB lpDCB)
{
  if (!hFile)
    return FALSE;

  *lpDCB = hFile->DCB;
  return TRUE;
}

BOOL SetCommMask(hfComm hFile, DWORD dwEvtMask)
{
  if (!hFile)
    return FALSE;

  return TRUE;
}

/** Set the communications timeout values. These values are
 *  actually used by the UNIX mode comm drivers... but I
 *  don't think anything else does.
 *
 *  Recall that some timeout values in here are 16 ticks = 1ms.
 *  Will document more later (check msdn.com in the interm)
 */
BOOL SetCommTimeouts(COMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  if (!hFile)
    return FALSE;

  hFile->CT = *lpCommTimeouts;    
  return TRUE;
}

/** Get the communications timeout values set by SetCommTimeouts() */
BOOL GetCommTimeouts(COMMHANDLE hFile, LPCOMMTIMEOUTS lpCommTimeouts)
{
  if (!hFile)
    return FALSE;

  *lpCommTimeouts = hFile->CT;
  return TRUE;
}

/** COMMHANDLE initializer */
BOOL SetupComm(COMMHANDLE hFile, DWORD dwInQueue, DWORD dwOutQueue)
{
  if (!hFile)
    return FALSE;

  memset(hFile, 0, sizeof(*hFile));

  if (hFile->txBufSize)
    hFile->txBufSize 	= dwOutQueue;
  else
    if (!hFile->txBufSize)
      hFile->txBufSize	= 1024;

  if (hFile->rxBufSize)
    hFile->rxBufSize 	= dwInQueue;
  else
    if (!hFile->rxBufSize)
      hFile->rxBufSize = 1024;

  hFile->fd 		= -1;
  hFile->DCB.isTerminal	= TRUE;
  return TRUE;
}

BOOL SetCommBreak(COMMHANDLE hFile)
{
  if (!hFile)
    return FALSE;

  return TRUE;
}

BOOL ClearCommBreak(COMMHANDLE hFile)
{
  if (!hFile)
    return FALSE;

  return TRUE;
}

/** Translates an HFILE into a COMMHANDLE.
 *
 *  Under NT, they seem to be equivalent types, much like
 *  UNIX file descriptors. However, we maintain more
 *  information with COMMHANDLEs than just the file descriptor
 *  under UNIX, because we want to be able to retrieve things
 *  like the DCB which are controlled via the COMMHANDLE, and
 *  presumably handled by file descriptor in the NT kernel.
 *
 *  An alternate implementation might be to statically store
 *  the "related" information into an array indexed by file 
 *  descriptor. That seems really inelegant to me, although
 *  it *would* allow passing around a raw file descriptor
 *  if that was needed. Either way, using the libcomm routines
 *  to translate between the two data types (even if that
 *  translation routine boils down to the assignment operator)
 *  allows either implementation without breaking the API -- as
 *  long as the comm libraries and compat libraries are updated
 *  simulataneously.
 *
 *  @see 	max_asyncnt.c  
 *
 *  @param	hf	file handle (file descriptor) to convert
 *  @param	ch	Pointer to an allocated communications handle,
 *                      which is populated and initialized by
 *                      this function.
 *  @returns	ch, which now contains the file descriptor, etc.
 */
COMMHANDLE CommHandle_fromFileHandle(COMMHANDLE ch, HFILE hf)
{
  if (ch == NULL)
    ch = calloc(sizeof(*ch), 1);

  SetupComm(ch, 0, 0);
  ch->fd = (int)hf;
  return ch;
}

/** Update the file descriptor within a COMMHANDLE.
 *  @see _CommHandle_fromFileHandle()
 */
void CommHandle_setFileHandle(COMMHANDLE ch, HFILE hf)
{
  if (ch)
    ch->fd = (int)hf;
}

/** Translates a COMMHANDLE into an HFILE.
 *  @see 	CommHandle_fromFileHandle()
 *
 *  @param	ch	"Windows" communications handle. NOT a Maximus communications handle!
 *  @returns	A UNIX File Descriptor
 *
 *  @note	Under NT, this could probably implemented as a
 *              a macro: #define CommHandle_fromFileHandle(fh) (fh)
 */
HFILE FileHandle_fromCommHandle(COMMHANDLE ch)
{
  if (ch) 
    return ch->fd;
  else
    return -1;
}




