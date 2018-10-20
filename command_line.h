#ifndef CRAYONIZER_CMDLINE_H
#define CRAYONIZER_CMDLINE_H

#include "common.h"

void ProcessCmdLine(char *CmdLine, ListNode *Crayons);
char *RebuildCommandLine(char *RetStr, int argc, char *argv[], const char *CrayonizerDir);
void CommandLineAddSubstitution(const char *Match, const char *Substitution);

#endif
