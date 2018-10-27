#ifndef CRAYONIZER_CMDLINE_H
#define CRAYONIZER_CMDLINE_H

#include "common.h"

#define CMDLINE_SUB 0
#define CMDLINE_INSERT 1
#define CMDLINE_APPEND 2

void ProcessCmdLine(char *CmdLine, ListNode *Crayons);
char *RebuildCommandLine(char *RetStr, int argc, char *argv[], const char *CrayonizerDir);
void CommandLineAddSubstitution(int Type, const char *Match, const char *Substitution);

#endif
