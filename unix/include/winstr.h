#ifndef _WES_WINSTR_H
#define _WES_WINSTR_H

#ifdef  __cplusplus
extern "C" {
#endif

inline int stricmp(const char *a, const char *b);
inline int strnicmp(const char *a, const char *b, size_t n);
char *strupr(char *s);
char *strlwr(char *s);
char *itoa(int value, char *buffer, int radix);
int memicmp(const void *p, const void *q, size_t length);

#ifdef  __cplusplus
}
#endif

#endif
