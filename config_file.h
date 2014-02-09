#ifndef CRAYONIZER_CONFIG_H
#define CRAYONIZER_CONFIG_H

#include "common.h"

int EntryMatchesCommand(char *EntryList, char *EntryArgs, char *ProgPath, char *ProgArgs);
int ConfigReadFile(char *Path, char *CommandLine, char **CrayonizerDir, ListNode *ColorMatches);

#endif

