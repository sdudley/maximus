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
 *  Revision 1.1  2003/06/11 14:44:51  wesgarland
 *  Initial Revision
 *
 */

static char rcs_id[]="$Id: xpunix.c,v 1.1 2003/06/11 14:44:51 wesgarland Exp $";

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

