#include "xterm.h"

#define NORM "\x1b[0m"
#define CLRSCR "\x1b[2J\x1b[;H"



void XTermReadCursorPos(int *x, int *y)
{
char *Tempstr=NULL, *ptr;
STREAM *StdIn;
int result;

StdIn=STREAMFromFD(0);
STREAMSetTimeout(StdIn,100);
STREAMFlush(StdIn);
Tempstr=CopyStr(Tempstr,"\x1b[6n");
write(1,Tempstr,StrLen(Tempstr));
Tempstr=STREAMReadToTerminator(Tempstr,StdIn,'R');
result=StrLen(Tempstr);
//Tempstr=SetStrLen(Tempstr,100);
//result=read(0,Tempstr,100);
if (result > 0)
{
	StrTrunc(Tempstr, result);
	ptr=Tempstr;
	if (strncmp(ptr,"\x1b[",2)==0)
	{
		*y=(int) strtol(ptr+2,&ptr,10);
		if (*ptr==';')
		{
			ptr++;
			*x=(int) strtol(ptr,&ptr,10);
		}
	}
}

STREAMDestroy(StdIn);
Destroy(Tempstr);
}



char *XTermReadValue(char *RetStr, const char *Query, const char *ReplyStart)
{
char *Tempstr=NULL, *ptr;
STREAM *StdIn;
int len;

RetStr=CopyStr(RetStr, "");
StdIn=STREAMFromFD(0);
STREAMSetTimeout(StdIn,1);
write(1,Query,StrLen(Query));
Tempstr=STREAMReadToTerminator(Tempstr,StdIn,'\\');
len=StrLen(Tempstr);
if (len > 0)
{
	ptr=Tempstr+len-2;
	if (strcmp(ptr,"\x1b\\")==0) StrTrunc(Tempstr, ptr-Tempstr);;

	ptr=Tempstr;
	len=StrLen(ReplyStart);
	if (strncmp(ptr,ReplyStart,len)==0) ptr+=len;
	if (StrValid(ptr)) RetStr=CopyStr(RetStr,ptr);
}

STREAMDestroy(StdIn);
Destroy(Tempstr);

return(RetStr);
}

