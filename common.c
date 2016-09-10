#include "common.h"

ListNode *Vars=NULL;
int ScreenRows=0, ScreenCols=0;
int LineNo=0;
int cmdline_argc;
char **cmdline_argv;
char *CrayonizerMMap=NULL;
STREAM *StdIn=NULL, *CommandPipe=NULL;
ListNode *CrayonList=NULL, *Functions=NULL, *Streams=NULL;
time_t Now=0;

