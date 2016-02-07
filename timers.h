
#ifndef CRAYONIZER_TIMERS_H
#define CRAYONIZER_TIMERS_H

#include "common.h"
#include "crayonizations.h"

void ParseTimer(const char *Config);
void UpdateTimer(TCrayon *Timer);
void ProcessTimers();

#endif
