#ifndef CRAYONIZER_HISTORY_H
#define CRAYONIZER_HISTORY_H

#include "common.h"

void TypingHistoryActivate(int NoOfLines);
void TypingHistoryAddKeypress(const char *Chars, int Len);
void TypingHistoryPopup(TCrayon *Crayon);

#endif
