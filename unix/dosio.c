#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <prog.h>
#include <uni.h>

#ifdef sopen
# undef sopen
#endif

/* TODO Add some calls to adaptcase somewhere */

char *fixPathDup(const char *filename)
{
  /* Remember to free return if it's not filename,
  ** or filename + 2!
  */

  char *fnDup;
  char *slash;

  if (!filename)
    return NULL;

  if (filename[0] && (filename[1] == ':'))
    filename = filename + 2;

  if (!strchr(filename, '\\'))
    return (char *)filename;

  fnDup = strdup(filename);
  if (!fnDup)
    return (char *)filename; /* ack! */

  for (slash = strchr(fnDup, '\\'); slash; slash = strchr(slash, '\\'))
    *slash = '/';

  return fnDup;
}

void fixPathDupFree(const char *filename, char *filename_dup)
{
  if (filename == filename_dup)
    return;

  if (filename == filename_dup + 2)
    return;

  if (filename_dup)
    free(filename_dup);
}

char *fixPath(char *filename)
{
  char *slash;

  if (!filename)
    return NULL;

  if (filename[0] && (filename[1] == ':'))
    filename = filename + 2;

  if (!strchr(filename, '\\'))
    return filename;

  for (slash = strchr(filename, '\\'); slash; slash = strchr(slash, '\\'))
    *slash = '/';

  return filename;
}

void fixPathMove(char *filename)
{
  char *new = fixPath(filename);

  if (new == filename + 2)
    memmove(filename, filename + 2, strlen(filename) + 1);
}

int sopen(const char *filename, int openMode, int shacc, ...)
{
  /* Wes' implementation of sopen -- not 100% compatible
  ** with watcom version, hope it's close enough.
  **
  ** Biggest incompatibility is that there is no easy way to 
  ** reads to a file from another arbitrary unix open. Later,
  ** we can re-implement DOS semantics with SYSV shmem if
  ** this gives us grief.
  **
  ** Note: Assumes DENYWR is true if DENYRW is true.
  */

  va_list       ap;
  int           fd;
  int           perms;
  int           lockMode = LOCK_NB;
  char		*filename_dup;
  int		no_inherit = 0;

  va_start(ap, shacc);
  perms=va_arg(ap, int);
  va_end(ap);

  if (openMode & O_NOINHERIT)
  {
    openMode &= ~O_NOINHERIT;
    no_inherit = 1;    
  }

/* QNX docs say:
	SH_COMPAT Set compatibility mode. 
	SH_DENYRW Prevent read or write access to the file. 
	SH_DENYWR Prevent write access to the file. 
	SH_DENYRD Prevent read access to the file. 
	SH_DENYNO Permit both read and write access to the file. 
*/

  if (shacc & SH_DENYWR)	/* Prevent Write access to the file */
    lockMode |= LOCK_SH;	/* Deny writes means allow many readers, right? */
  else if (shacc & SH_DENYRD)
    lockMode |= LOCK_EX;	/* Deny read.. well, let's deny read and write. */

#if defined(FLOCK_IS_FCNTL)	/* can't get locks in wrong mode on some platforms */
  if ((openMode & O_RDWR) != O_RDWR)
  {
    if (
	((lockMode & LOCK_SH) && !(openMode & O_RDONLY)) ||
	((lockMode & LOCK_EX) && !(openMode & O_WRONLY))
	)
    {
      openMode &= ~(O_RDONLY | O_WRONLY);	
      openMode |= O_RDWR;
    }
  }
#endif /* FLOCK_IS_FCNTL */

  filename_dup = fixPathDup(filename);
  fd = open(filename_dup, openMode, perms);
  fixPathDupFree(filename, filename_dup);

  if (fd < 0)
    return fd;

  if (shacc != SH_DENYNO)
  {
    if (flock(fd, lockMode))
    {
      close(fd);
      return -1;
    }
  }

  if (no_inherit)
  {
    int flags;

    flags = fcntl(fd, F_GETFD);
    flags |= FD_CLOEXEC;

    fcntl(fd, F_SETFD);
  }

  return fd;
}

long tell(int fd)
{
  return lseek(fd, 0L, SEEK_CUR);
}

