
#ifndef CRAYONIZER_SIGNALS_H
#define CRAYONIZER_SIGNALS_H

#include "common.h"

void HandleSigwinch(STREAM *Pipe);
void HandleSignal(int sig);
void PropagateSignals(STREAM *Pipe);

#endif
