#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "dossem.h"
#include "share.h"

/* hack by wes -- just a guess as to how Dos*Sem is supposed to
 * work, based on source-code context. It appears that these
 * are some kind of system-global named mutexes. 
 *
 * I can't really see a difference between HSEM and HMTX, besides
 * the name of the functions creating/destroying the mutexes, so
 * I will implement them the same. (Later note: HSEM = OS/2 1.x, HMTX = Warp)
 *
 * Basically, they will be implemented as pthread_mutexes on top of
 * file system lock files. That should get use the thread and global
 * semantic we want. Later, we can look at SYSV semaphores or something
 * if this is too slow.  Oh, and I won't use the lock files if we're
 * not using shared/named MutexSems.
 *
 */

#if defined(LINUX) && !defined(PTHREAD_MUTEX_ERRORCHECK)
/* RedHat 5.2 ships with a not-quite p pthreads interface.. don't know about later versions */
# define PTHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_ERRORCHECK_NP
# define pthread_mutexattr_settype(attr,type) pthread_mutexattr_setkind_np(attr,type)
#endif

#define UNIX_SEMDIR "/tmp/.dossem" /* RAM on Solaris */

#if defined(LINUX)
/*
 * Note: linux is retarded, separate LWPs are actually implemented
 * as different PIDs. Grr. So how the hell are we supposed to tell
 * the difference between a forked process, and another LWP?
 */
# define processID() (int)getpid() /* getppid, maybe? */
#else
# define processID() (int)getpid()
#endif

typedef enum { dmt_sem = 1, dmt_mutexsem } dos_mutex_t;

struct _hsem
{
  int             fd;
  dos_mutex_t     type; 
  int             owned;
  pthread_mutex_t mutex;
} _hsem;

static void mkdir_recursive(const char *path)
{
  char *parent;
  char *s;
  int  i;

  if (!path[0] || !path[1])
    return;
  
  i = mkdir(path, 0777);
  if (!i || errno == EEXIST)
    return;

  parent = strdup(path);
  s = strrchr(parent, '/');
  if (s)
  {
    *s = (char)0;
    mkdir_recursive(parent);
  }

  free(parent);
  return;
}

static void mkdir_recursive_for(const char *path)
{
  char *parent;
  char *s;

  parent = strdup(path);
  s = strrchr(parent, '/');
  if (s)
  {
    *s = (char)0;
    mkdir_recursive(parent);
  }

  free(parent);
  return;
}

int DosCreateSem(int NoExclusive, PHSEM SemHandle, const char *SemName)
{
  /* Create a system semaphore */
  char *SemPath, SemPathBuf[FILENAME_MAX + 1];
  char buf[32];
  HSEM sem;

  if (!SemHandle)
    return ERROR_INVALID_PARAMETER;

  if (SemName && (strcasecmp(SemName, "\\SEM\\") != 0))
    return ERROR_INVALID_NAME;

  if (SemName) 
  {
    snprintf(SemPathBuf, sizeof(SemPathBuf), UNIX_SEMDIR "/%s", SemName);
    fixPath(SemPathBuf);
    SemPath = SemPathBuf;
    mkdir_recursive_for(SemPath);
  }
  else
  {
    mkdir_recursive(UNIX_SEMDIR);
    SemPath = tempnam(UNIX_SEMDIR, "sem_");
  }

  sem = calloc(sizeof(*sem), 1);
  if (!sem)
    return ERROR_NOT_ENOUGH_MEMORY;
  else
    *SemHandle = sem;

  sem->fd = open(SemPathBuf, O_CREAT | O_EXCL | O_RDWR, 0666);
  sem->type = dmt_sem;

  if (sem->fd < 0)
  {
    free(sem);
    *SemHandle = NULL;

    switch(errno)
    {
      case ENOENT:
      case EPERM:
        return ERROR_INVALID_NAME;
      case EEXIST:
        return ERROR_DUPLICATE_NAME;
      default:
        return ERROR_INVALID_PARAMETER;         
    }
  }

  if (flock(sem->fd, LOCK_EX | LOCK_NB))
  {
    /* Somebody else open after we create/before we lock?? */
    close(sem->fd);
    free(sem);
    *SemHandle = NULL;

    return ERROR_DUPLICATE_NAME;
  }

  lseek(sem->fd, (off_t)0, SEEK_SET);
  sprintf(buf, "%i\n", processID());
  write(sem->fd, buf, strlen(buf));
  fsync(sem->fd);

  sem->owned = 1;
  return NO_ERROR;
}

int DosOpenSem(PHSEM SemHandle, const char *SemName)
{
  /* Request access to an existing system semaphore */

  char *SemPath, SemPathBuf[FILENAME_MAX + 1];
  HSEM sem;

  if (!SemHandle)
    return ERROR_INVALID_PARAMETER;

  if (SemName && (strcasecmp(SemName, "\\SEM\\") != 0))
    return ERROR_INVALID_NAME;

  if (SemName) 
  {
    snprintf(SemPathBuf, sizeof(SemPathBuf), UNIX_SEMDIR "/%s", SemName);
    fixPath(SemPathBuf);
    SemPath = SemPathBuf;
    mkdir_recursive_for(SemPath);
  }
  else
  {
    mkdir_recursive(UNIX_SEMDIR);
    SemPath = tempnam(UNIX_SEMDIR, "sem_");
  }

  sem = calloc(sizeof(*sem), 1);
  if (!sem)
    return ERROR_NOT_ENOUGH_MEMORY;
  else
    *SemHandle = sem;

  sem->fd = open(SemPathBuf, O_CREAT | O_RDWR, 0666);
  sem->type = dmt_sem;

  if (sem->fd < 0)
  {
    free(sem);
    *SemHandle = NULL;

    switch(errno)
    {
      case ENOENT:
      case EPERM:
        return ERROR_INVALID_NAME;
      default:
        return ERROR_INVALID_PARAMETER;         
    }
  }

  return NO_ERROR;
}

int DosSemRequest(HSEM SemHandle, int how)
{
  int rc = NO_ERROR;
  HSEM sem = SemHandle; /* handy */
  int lockFlags = LOCK_EX;
  int i;
  char buf[32];

  /* Request ownership of a semaphore */

  if (SemHandle->owned)
    return NO_ERROR;

  if (how != SEM_INDEFINITE_WAIT)
    lockFlags |= LOCK_NB;

  if (flock(sem->fd, lockFlags))
    return ERROR_ALREADY_EXISTS;

  i = read(sem->fd, buf, sizeof(buf));
  buf[i] = (char)0;
  if (buf[0] && (atoi(buf) != processID()))
    rc = ERROR_SEM_OWNER_DIED;

  sprintf(buf, "%i\n", processID());
  write(sem->fd, buf, strlen(buf));
  fsync(sem->fd);

  return rc;
}

int DosSemClear(HSEM SemHandle)
{
  /* Relinquish semaphore ownership */

  HSEM sem = SemHandle; /* handy */

  if (!SemHandle)
    return ERROR_INVALID_PARAMETER;

  if (!sem->owned)
    return ERROR_INVALID_HANDLE;

  ftruncate(sem->fd, 0);
  flock(sem->fd, LOCK_UN);
  sem->owned = 0;

  return NO_ERROR;
}

int DosCloseSem(HSEM SemHandle)
{
  /* Close a system semaphore */

  HSEM sem = SemHandle; /* handy */

  if (!SemHandle)
    return ERROR_INVALID_PARAMETER;

  DosSemClear(SemHandle);
  close(sem->fd);
  free(sem);

  return NO_ERROR;
}

int DosSemWait(HSEM SemHandle, int how)
{
  /* Wait on a semaphore to be cleared */
  int rc;

  rc = DosSemRequest(SemHandle, how);
  if (rc)
    return rc;

  return DosSemClear(SemHandle);
}

int DosCreateMutexSem(const char *SemName, PHMTX hmtx_p, int AttributeFlags, int InitialState)
{
  /* Create a system mutex semaphore. The semaphore may be either
   * private or shared and is accessible by all threads of the 
   * calling process.
   */

  char *SemPath, SemPathBuf[FILENAME_MAX + 1];
  char buf[32];
  HMTX hmtx;
  pthread_mutexattr_t attr;

  if (!hmtx_p)
    return ERROR_INVALID_PARAMETER;

  if (SemName && (strcasecmp(SemName, "\\SEM32\\") != 0))
    return ERROR_INVALID_NAME;

  if (SemName) 
  {
    AttributeFlags |= DC_SEM_SHARED;
    snprintf(SemPathBuf, sizeof(SemPathBuf), UNIX_SEMDIR "/%s", SemName);
    fixPath(SemPathBuf);
    SemPath = SemPathBuf;
    mkdir_recursive_for(SemPath);
  }

  if (AttributeFlags & DC_SEM_SHARED)
  {
    mkdir_recursive(UNIX_SEMDIR);
    SemPath = tempnam(UNIX_SEMDIR, "sem32");
  }
  
  hmtx = calloc(sizeof(**hmtx_p), 1);
  if (!hmtx)
    return ERROR_NOT_ENOUGH_MEMORY;

  *hmtx_p = hmtx;
  hmtx->type = dmt_mutexsem;

  hmtx->fd = open(SemPathBuf, O_CREAT | O_EXCL | O_RDWR, 0666);
  if (hmtx->fd < 0)
  {
    free(hmtx);
    *hmtx_p = NULL;

    switch(errno)
    {
      case ENOENT:
      case EPERM:
        return ERROR_INVALID_NAME;
      case EEXIST:
        return ERROR_DUPLICATE_NAME;
      default:
        return ERROR_INVALID_PARAMETER;         
    }
  }

  if (InitialState == 0)
    return NO_ERROR;

  if (flock(hmtx->fd, LOCK_EX | LOCK_NB))
  {
    /* Somebody else open after we create/before we lock?? */
    close(hmtx->fd);
    free(hmtx);
    *hmtx_p = NULL;

    return ERROR_DUPLICATE_NAME;
  }

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
  pthread_mutex_init(&hmtx->mutex, &attr);
  pthread_mutexattr_destroy(&attr);
  pthread_mutex_lock(&hmtx->mutex);

  sprintf(buf, "%i\n", processID());
  write(hmtx->fd, buf, strlen(buf));
  fsync(hmtx->fd);

  return NO_ERROR;
}

int DosRequestMutexSem(HMTX hmtx, int how)
{
  int lockMode = LOCK_EX;
  char buf[32];
  int i;
  int rc = NO_ERROR;

  if (!hmtx)
    return ERROR_INVALID_HANDLE;

  if (how != SEM_INDEFINITE_WAIT)
    lockMode |= LOCK_NB;

  /* First, try and lock the file.. */
  if (flock(hmtx->fd, lockMode))
  {
    switch(errno)
    {
      case EINTR:	return ERROR_INTERRUPT;
      case EINVAL:	return ERROR_INVALID_HANDLE;
      case EWOULDBLOCK:	return ERROR_TIMEOUT; /* surely a better error code exists */
      default: return ERROR_INVALID_HANDLE;
    }
  }

  if (lseek(hmtx->fd, (off_t)0, SEEK_SET))
    return ERROR_INVALID_HANDLE;

  i = read(hmtx->fd, buf, sizeof(buf));
  buf[i] = (char)0;
  if (buf[0] && (atoi(buf) != processID()))
    rc = ERROR_SEM_OWNER_DIED;

  if (how == SEM_INDEFINITE_WAIT)
    pthread_mutex_lock(&hmtx->mutex);
  else
  {
    if (pthread_mutex_trylock(&hmtx->mutex))
    {
      close(hmtx->fd);
      free(hmtx);
      return ERROR_DUPLICATE_NAME;
    }
  }

  lseek(hmtx->fd, (off_t)0, SEEK_SET);
  sprintf(buf, "%i\n", processID());
  write(hmtx->fd, buf, strlen(buf));
  fsync(hmtx->fd);
  hmtx->owned = 1;

  return rc;
}




