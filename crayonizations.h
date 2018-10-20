#ifndef CRAYONIZATIONS_H
#define CRAYONIZATIONS_H

#include "common.h"

void FunctionCall(STREAM *Pipe, const char *FuncName, const char *Data, int DataLen);
int ApplyActions(STREAM *Pipe, int *AttribLine, const char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon);
void ColorSubstring(STREAM *Pipe, int *AttribLine, const char *Line, int Len, TCrayon *Crayon);
void Crayonize(STREAM *Pipe, const char *Line, int Len, int IsFunc, ListNode *CrayonList);
int ProcessCrayonization(STREAM *Pipe, const char *Line, int Len, int *Attribs, TCrayon *Crayon);

#endif
