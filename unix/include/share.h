#ifndef _WES_SHARE_H
#define _WES_SHARE_H

#include <io.h>
#include <sys/file.h>

#if defined(SOLARIS) || defined(NEED_FLOCK)
int flock(int fd, int lockmode);
# define FLOCK_IS_FCNTL
#endif

#if !defined(LOCK_EX)
# define LOCK_SH	0x01		/**< shared file lock */
# define LOCK_EX	0x02		/**< exclusive file lock */
# define LOCK_NB	0x04		/**< don't block when locking */
# define LOCK_UN	0x08		/**< unlock file */
# ifndef FLOCK_IS_FCNTL
#  error flock header missing!
# endif
#endif

#define SH_DENYNONE	0
#define SH_DENYRD	1 << 0
#define SH_DENYWR	1 << 1
#define SH_DENYRW	SH_DENYRD | SH_DENYWR
#define SH_COMPAT	SH_DENYWR
#define SH_DENYNO	SH_DENYNONE

#endif
