#ifndef _WES_SHARE_H
#define _WES_SHARE_H

#include <io.h>

#ifdef LINUX
#include <sys/file.h>
#endif

#ifdef SOLARIS
#include <flock.h>
#endif

#define SH_DENYNONE	0
#define SH_DENYRD	1 << 0
#define SH_DENYWR	1 << 1
#define SH_DENYRW	SH_DENYRD | SH_DENYWR
#define SH_COMPAT	SH_DENYWR
#define SH_DENYNO	SH_DENYNONE

#endif
