#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

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
