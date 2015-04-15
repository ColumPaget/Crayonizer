#include "common.h"

ListNode *Vars=NULL;
int ScreenRows=0;
int LineNo=0;
int cmdline_argc;
char **cmdline_argv;
STREAM *StdIn=NULL, *CommandPipe=NULL;
ListNode *ColorMatches=NULL, *Streams=NULL;


