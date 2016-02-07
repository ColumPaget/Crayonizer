#include "common.h"

ListNode *Vars=NULL;
int ScreenRows=0, ScreenCols=0;
int LineNo=0;
int cmdline_argc;
char **cmdline_argv;
char *CrayonizerMMap=NULL;
STREAM *StdIn=NULL, *CommandPipe=NULL;
ListNode *ColorMatches=NULL, *Streams=NULL, *Functions=NULL;
time_t Now=0;

