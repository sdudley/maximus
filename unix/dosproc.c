#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "dosproc.h"
#include "compat.h"

unsigned long DosExit(unsigned long ulAction, unsigned long ulResult)
{
  switch(ulAction)
  {
    case EXIT_THREAD:
     pthread_exit((void *)ulResult);
     break;
    case EXIT_PROCESS:
     exit(ulResult);
    default:
      return 1;
  }
}

unsigned short dosProc_exitCode;

typedef unsigned long (*exitHandler_t)(unsigned short);
static exitHandler_t *exitList = NULL;
static size_t exitListLength = 0;

void dosProc_exitHandler()
{
  exitHandler_t	*handler;

  if (!exitList)
    return;

  for (handler = exitList; handler; handler++)
  {
    if (*handler != (exitHandler_t)dosProc_exitHandler)
      (*handler)(dosProc_exitCode);
  }

  return;
}

unsigned long _DosExitList(unsigned long ulAction, unsigned long (*function)(unsigned short))
{
  /* Known bugs: does not check for dupe on add. Does not process "in order". "in order"
   * in this case is looking at (ulAction & ~0xff), and processing them in that order.
   * I don't think that will kill max. Also, there is no way to shrink the list.
   */

  exitHandler_t *exitList_new;
  size_t	i;
  static int    registered = 0;

  if (!registered)
    atexit(dosProc_exitHandler);

  switch(ulAction & 0xff)
  {
    case EXLST_EXIT:
      break;
    case EXLST_ADD:
      if (!function)
        return 1;

      exitList_new = realloc(exitList, sizeof(exitList[0]) * (exitListLength + 2));
      if (!exitList_new)
        return ERROR_NOT_ENOUGH_MEMORY;
      exitList = exitList_new;
      exitList[exitListLength] = function;
      exitList[exitListLength + 1] = NULL;
      exitListLength++;
      break;
    case EXLST_REMOVE:
      if (!function)
        return 1;

      for (i = 0; i < exitListLength; i++)
        if (exitList[i] == function)
        {
          exitList[i] = (exitHandler_t)dosProc_exitHandler;
	  return NO_ERROR;
        }
    default:
      return 1;
  }

  return NO_ERROR;
}

unsigned long DosScanEnv(const char *pszName, char **ppszValue)
{
  *ppszValue = getenv(pszName);
  if (*ppszValue)
    return NO_ERROR;

  return 1;
}

inline void DosSleep (unsigned long usec)
{
  usleep(usec);
}

