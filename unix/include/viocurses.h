#ifndef _VIOCURSES_H
#define _VIOCURSES_H

int VioSetCurPos(int row, int column, void *handle);
int VioWrtTTY(const char *string, size_t length, void *handle);
/* chtype cursesAttribute(unsigned char dosAttribute); */

#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008

#define BACKGROUND_BLUE      0x0010
#define BACKGROUND_GREEN     0x0020
#define BACKGROUND_RED       0x0040
#define BACKGROUND_INTENSITY 0x0080

#endif
