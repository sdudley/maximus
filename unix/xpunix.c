#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include "share.h"

/** @file	xpunix.c - Cross-Platform UNIX helpers
 *  @author	Wes Garland
 *  @date	June 5th, 2003
 *
 *  $Log: xpunix.c,v $
 *  Revision 1.4  2004/01/28 06:38:11  paltas
 *  Fixed compiler warnings, still Comdll missing, but I need to do some
 *  rewrite stuff there, so it will be fixed later.
 *
 *  Revision 1.3  2004/01/27 21:03:54  paltas
 *  Fixed localmode
 *
 *  Revision 1.1  2003/06/11 14:44:51  wesgarland
 *  Initial Revision
 *
 */

#ifndef __GNUC__
static char rcs_id[]="$Id: xpunix.c,v 1.4 2004/01/28 06:38:11 paltas Exp $";
#endif

#if defined(FLOCK_IS_FCNTL)
/** Implement flock with fcntl.
 *
 *  @bug errno codes match fcntl(), not flock(). They're
 *  not portable anyhow, so more information probably
 *  won't hurt. Also, errno will probably never be set to 
 *  EAWOULDBLOCK.
 *
 *  @param	fd		File Descriptor
 *  @param	lockmode	Mode to lock in (bitmask)
 *  - LOCK_EX	Exclusive Lock
 *  - LOCK_SH   Shared Lock
 *  - LOCK_UN   Remove all ocks
 *  - LOCK_NB   Non-Blocking lock
 *  @return	0 on success
 */
 
int flock(int fd, int lockmode)
{
  int          cmd,
               result;
  struct flock fl;
 
  if (lockmode & LOCK_NB)
    cmd=F_SETLK;
  else
    cmd=F_SETLKW;
 
  memset(&fl, 0, sizeof(fl));
 
  switch(lockmode & ~LOCK_NB)
  {
    case LOCK_SH:  
      fl.l_type = F_RDLCK;
      break;
    case LOCK_EX:
      fl.l_type = F_WRLCK;
      break;
    case LOCK_UN:
      fl.l_type = F_UNLCK;
      break;
 }
   
  fl.l_whence=SEEK_SET;
 
  result=fcntl(fd, cmd, &fl);
  return result;
}
#endif /* SOLARIS || NEED_FLOCK */

