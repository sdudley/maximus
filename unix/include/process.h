#ifndef _PROCESS_H
#define _PROCESS_H

#define P_WAIT          1
#define P_NOWAIT        2
#define P_NOWAITO       3
#define P_OVERLAY       4

int spawnvp(int mode, const char *file, char *const argv[]);

#endif
