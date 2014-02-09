#ifndef CRAYONIZATIONS_H
#define CRAYONIZATIONS_H

#include "common.h"

void DrawStatusBar(const char *Text);
void ApplyActions(STREAM *Pipe, int *AttribLine, char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon);
void ColorSubstring(STREAM *Pipe, int *AttribLine, char *Line, int Len, TCrayon *Crayon);
void OutputLineWithAttributes(char *Line, int *Attribs, int Len);
void ColorLine(STREAM *Pipe, char *Line, int Len, ListNode *ColorMatches);

#endif
