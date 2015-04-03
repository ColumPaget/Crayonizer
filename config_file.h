#ifndef CRAYONIZER_CONFIG_H
#define CRAYONIZER_CONFIG_H

#include "common.h"

int EntryMatchesCommand(char *EntryList, char *EntryArgs, char *ProgPath, char *ProgArgs);
int ConfigReadFile(const char *Path, const char *CmdLine, char **CrayonizerDir, ListNode *ColorMatches);

#endif

