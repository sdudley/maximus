#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

int stricmp(const char *a, const char *b)
{
  return strcasecmp(a, b);
}

int strnicmp(const char *a, const char *b, size_t n)
{
  return strncasecmp(a, b, n);
}

char *strupr(char *sOrig)
{
  char *s = sOrig;

  if (s && *s)
  {
    do
    {
      *s = toupper(*s);
    }
    while (*s++);
  }

  return sOrig;
}

char *strlwr(char *sOrig)
{
  char *s = sOrig;

  if (s && *s)
  {
    do
    {
      *s = tolower(*s);
    }
    while (*s++);
  }

  return sOrig;
}

char *itoa(int value, char *buffer, int radix)
{
  /* known bugs: only supports 8, 10, 16 radix. That should be enough for max */

  char *fmt;

  if (!buffer)
    return NULL;

  switch(radix)
  {
    case 8:
      fmt = "%o";
      break;
    case 10:
      fmt = "%d";
      break;
    case 16:
      fmt = "%x";
      break;
    default:
      return NULL;
  }

  sprintf(buffer, fmt, value);
  return buffer;
}

int memicmp(const void *p, const void *q, size_t length)
{
  const unsigned char *P;
  const unsigned char *Q;

  for (P=p, Q=q; ((void *)P - p) < length; P++, Q++)
  {
    if (tolower((int)*P) == tolower((int)*Q))
      continue;

    if (*P < *Q)
      return -1;

    return 1;
  }

  return 0;
}

