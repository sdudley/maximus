#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <pty.h>
#include <utmp.h>
#include "process.h"
#include "io.h"
#include "prog.h"
#include "ntcomm.h"

#define unixfd(hc)      FileHandle_fromCommHandle(ComGetHandle(hc))

/* Record locking code */

int lock(int fd, long offset, long len)
{
  struct flock  lck;

  lck.l_type    = F_WRLCK;                      /* setting a write lock */
  lck.l_whence  = SEEK_SET;                     /* offset l_start from beginning of file */
  lck.l_start   = (off_t)offset;
  lck.l_len     = len;

  return fcntl(fd, F_SETLK, &lck);
}

int unlock(int fd, long offset, long len)
{
  struct flock  lck;

  lck.l_type    = F_UNLCK;                      /* setting not locked */
  lck.l_whence  = SEEK_SET;                     /* offset l_start from beginning of file */
  lck.l_start   = (off_t)offset;
  lck.l_len     = len;

  return fcntl(fd, F_SETLK, &lck);
}

void NoMem() __attribute__((weak));
void NoMem()
{
  fprintf(stderr, "Out of memory!\n");
  _exit(1);
}
