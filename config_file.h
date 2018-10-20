#ifndef CRAYONIZER_CONFIG_H
#define CRAYONIZER_CONFIG_H

#include "common.h"

TCrayon *NewCrayonAction(TCrayon *Crayon, int Type);
const char *ParseCrayonAction(TCrayon *Item, const char *Args);
const char *ParseActionToken(const char *Operations, TCrayon *Crayon);
void ParseCrayonEntry(TCrayon *Crayon, const char *Token, const char *Args);
//void ParseCrayonList(STREAM *S, TCrayon *Crayon);
int ConfigReadFile(const char *Path, const char *CmdLine, char **CrayonizerDir, ListNode *CrayonList);
int ConfigLoad(const char *CmdLine, const char *ConfigPaths, char **CrayonizerDir, ListNode *CrayonList);

#endif

