#ifndef CRAYONIZER_CONFIG_H
#define CRAYONIZER_CONFIG_H

#include "common.h"

TCrayon *NewCrayonAction(TCrayon *Crayon, int Type);
char *ParseCrayonAction(TCrayon *Item, char *Args);
char *ParseActionToken(char *Operations, TCrayon *Crayon);
void ParseCrayonEntry(TCrayon *Crayon, char *Token, char *Args);
void ParseCrayonList(STREAM *S, TCrayon *Crayon);
int ConfigReadFile(const char *Path, const char *CmdLine, char **CrayonizerDir, ListNode *CrayonList);
int EntryMatchesCommand(char *EntryList, char *EntryArgs, char *ProgPath, char *ProgArgs);
int ConfigLoad(const char *CmdLine, char **CrayonizerDir, ListNode *CrayonList);

#endif

