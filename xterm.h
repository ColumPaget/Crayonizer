
#ifndef CRAYON_ANSI_H
#define CRAYON_ANSI_H

#include "common.h"

#define NORM "\x1b[0m"
#define CLRSCR "\x1b[2J\x1b[;H"


char *VtCode(int Color, int BgColor, int Flags);

void XTermReadCursorPos(int *x, int *y);

char *XTermReadValue(char *RetStr, const char *Query, const char *ReplyStart);

#endif
