#ifndef _WES_IO_H
#define _WES_IO_H
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>

#ifndef O_BINARY
# define O_BINARY 0
# define O_TEXT 0
#endif

#ifdef LINUX
# define O_NOINHERIT 01000000
#endif

#ifdef SOLARIS
# define O_NOINHERIT 0x10000
#endif

#ifdef __FreeBSD__
# define O_NOINHERIT 0x20000
#endif

#ifndef O_NOINHERIT
# error You must choose a value for O_NOINHERIT which does not conflict with other vendor open flags!
#endif
 
int sopen(const char *filename, int openMode, int shacc, ...);
char *fixPathDup(const char *filename);
void fixPathDupFree(const char *filename, char *filename_dup);
char *fixPath(char *filename);
void fixPathMove(char *filename);
int sopen(const char *filename, int openMode, int shacc, ...);
long tell(int fd);
int fputchar(int c);
#endif
