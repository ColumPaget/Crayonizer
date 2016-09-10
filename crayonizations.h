#ifndef CRAYONIZATIONS_H
#define CRAYONIZATIONS_H

#include "common.h"

void FunctionCall(STREAM *Pipe, char *FuncName, char *Data, int DataLen);
int ApplyActions(STREAM *Pipe, int *AttribLine, char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon);
void ColorSubstring(STREAM *Pipe, int *AttribLine, char *Line, int Len, TCrayon *Crayon);
void OutputLineWithAttributes(char *Line, int *Attribs, int Len);
void Crayonize(STREAM *Pipe, char *Line, int Len, int IsFunc, ListNode *CrayonList);
int ProcessCrayonization(STREAM *Pipe, char *Line, int Len, int *Attribs, TCrayon *Crayon);

#endif
