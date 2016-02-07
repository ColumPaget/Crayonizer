
#ifndef CRAYONIZER_KEYPRESS_H
#define CRAYONIZER_KEYPRESS_H

#include "common.h"

#define KEYPRESS_PASSINPUT 1
#define KEYPRESS_LINEDIT 2


extern int KeypressFlags;


TCrayon *KeypressParse(char *Name);
int KeypressProcess(STREAM *StdIn, STREAM *Out);


#endif
