#ifndef _WES_DOSPROC_H
# include <stdlib.h>
/* Including stdlib so that exit() is guaranteed to by defined AFTER
 * the first inclusion of stdlib.h; stdlib.h itself will prevent
 * its multiple inclusion.
 */
# define _WES_DOSPROC_H
# define EXIT_THREAD     0
# define EXIT_PROCESS    1
 
# define EXLST_EXIT      1       /* Done processing, system should call the next cleanup function */
# define EXLST_ADD       2       /* Add a function to cleanups */
# define EXLST_REMOVE    3       /* Remove a function from cleanups */
# define DosExitList(a,f) do { if ((a & 0xFF) == EXLST_EXIT) return; else _DosExitList(a,f); } while(0)
# define exit(a) do { dosProc_exitCode = a; exit(a); } while(0)
extern unsigned short dosProc_exitCode;
unsigned long DosExit(unsigned long ulAction, unsigned long ulResult);
unsigned long _DosExitList(unsigned long ulAction, unsigned long (*function)(unsigned short));

typedef unsigned long (*PFN)(unsigned short);

unsigned long DosScanEnv(const char *pszName, char **ppszValue);
void DosSleep (unsigned long usec);
#endif
