#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include "process.h"
#include "io.h"

static void noop(int sig)
{
  ;
}

int spawnvp(int mode, const char *Cfile, char *const argv[])
{
  pid_t		pid;
  struct stat	sb;
  char		*file;

 signal(SIGCHLD, noop);

  file = fixPathDup(Cfile);

  if (!Cfile || stat(file, &sb))	/* Provide errno e.g. EPERM, ENOENT to caller */
    return -1;

  if (mode != P_OVERLAY)
    pid = fork();
  else
    pid = 0; /* fake being a child */

   if (pid) /* Parent */
  {
    if ((mode == P_NOWAIT) || (mode == P_NOWAITO))
    {
      fixPathDupFree(Cfile, file);
      return 0;
    }

    if (mode == P_WAIT)
    {
      int status;
      pid_t dead_kid;

      sleep(0);
      fixPathDupFree(Cfile, file);

      do
      {     
        errno = 0;
        dead_kid = waitpid(pid, &status, 0);
	if (dead_kid == pid)
          break;
      } while(errno != EINTR);

      if (dead_kid != pid)
        return -1;

      if (WIFEXITED(status))
        return 0; /* normal child exit */

      if (WIFSIGNALED(status))
        fprintf(stderr, __FUNCTION__ ": Child (%s) exited due to signal %i!\n", Cfile, WSTOPSIG(status));

      return -1;
    }
  }

  if (mode == P_NOWAITO)
  {
    /* Parent will not reap -- use double fork trick to avoid zombies */

    pid = getpid();
    signal(SIGCHLD, SIG_IGN);
    (void)setpgid(pid, pid); 
    if (fork())
      _exit(0);
  }

  execvp(file, argv);
  fprintf(stderr, __FUNCTION__ ": could not spawn %s! (%s)\n", file, strerror(errno));
  _exit(1);
  
}








