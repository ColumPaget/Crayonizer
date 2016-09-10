#ifndef CRAYONIZER_ESCAPE_SEQUENCES_H
#define CRAYONIZER_ESCAPE_SEQUENCES_H

#include "common.h"

#define BEL 7
#define CTRLO 15

#define ES_OKAY 0
#define ES_STRIP 1
#define ES_PART 2

//Handle ANSI coming from the crayonized program 
int EscapeSequenceHandle(char **text, char *end);

#endif
