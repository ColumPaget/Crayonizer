#include "text_substitutions.h"
#include <sys/mman.h>


#define SYS_STAT_PATH "/proc/stat"
#define MEMINFO_PATH "/proc/meminfo"
#define CPUINFO_PATH "/proc/cpuinfo"
#define UPTIME_FILENAME "/proc/uptime"



int GetLoad()
{
STREAM *S;
char *Tempstr=NULL, *Token=NULL;
char *ptr;
int Total=0, Idle, result=0;
static int LastTotal=0, LastIdle=0;

S=STREAMOpenFile(SYS_STAT_PATH, SF_RDONLY);
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	if (Tempstr)
	{
	StripTrailingWhitespace(Tempstr);
	ptr=GetToken(Tempstr," ",&Token,0);
	while (isspace(*ptr)) ptr++;
	ptr=GetToken(ptr," ",&Token,0);
	Total+=atoi(Token);
	ptr=GetToken(ptr," ",&Token,0);
	Total+=atoi(Token);
	ptr=GetToken(ptr," ",&Token,0);
	Total+=atoi(Token);
	ptr=GetToken(ptr," ",&Token,0);
	Idle=atoi(Token);
	
	Total+=Idle;
	if ((Total-LastTotal) > 0)
	{
	result=((Idle-LastIdle) * 100) / (Total-LastTotal);
	LastIdle=Idle;
	LastTotal=Total;
	}
	
	}
	STREAMClose(S);
}

DestroyString(Tempstr);
DestroyString(Token);

result=100-result;
return(result);
}



double GetMemUsage()
{
double total=0, val=0;
char *Tempstr=NULL;
STREAM *S=NULL;
float ftotal=0;

S=STREAMOpenFile(MEMINFO_PATH, SF_RDONLY);
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		if (strncmp(Tempstr, "MemTotal:", 9)==0) total=ParseHumanReadableDataQty(Tempstr+9, 0);
		if (strncmp(Tempstr, "MemFree:", 8)==0) val+=ParseHumanReadableDataQty(Tempstr+8, 0);
		if (strncmp(Tempstr, "Buffers:", 8)==0) val+=ParseHumanReadableDataQty(Tempstr+8, 0);
		if (strncmp(Tempstr, "Cached:", 7)==0) val+=ParseHumanReadableDataQty(Tempstr+7, 0);
	Tempstr=STREAMReadLine(Tempstr, S);
	}

  val=(100.0 - ((val / total) * 100.0));
	STREAMClose(S);
}

DestroyString(Tempstr);

return(val);
}



char *GetCPUSpeed()
{
double MHz=0;
char *Tempstr=NULL, *ptr;
STREAM *S=NULL;

S=STREAMOpenFile(CPUINFO_PATH, SF_RDONLY);
if (S)
{
	Tempstr=STREAMReadLine(Tempstr, S);
	while (Tempstr)
	{
		if (strncmp(Tempstr, "cpu MHz", 7)==0) 
		{
			ptr=strchr(Tempstr, ':');
			if (ptr)
			{
				ptr++;
				while (isspace(*ptr)) ptr++;
				MHz=atof(ptr) * 1024 * 1024;
				break;
			}
		}
	Tempstr=STREAMReadLine(Tempstr, S);
	}

	STREAMClose(S);
}

DestroyString(Tempstr);

return(GetHumanReadableDataQty(MHz,0));
}


int GetValueFromMMap(char *ValName, char **RetVal)
{
char *Name=NULL, *ptr;
int len=0;

		if (! CrayonizerMMap) return(0);
		ptr=GetNameValuePair(CrayonizerMMap, "\n","=",&Name, RetVal);
		while (ptr)
		{
		if (StrLen(Name) && (strcmp(ValName, Name)==0)) len=StrLen(*RetVal);
		ptr=GetNameValuePair(ptr, "\n","=",&Name, RetVal);
		}

DestroyString(Name);
return(len);
}


#define ENVTYPE_SCRIPT 1
#define ENVTYPE_FILE 2
#define ENVTYPE_MMAP 3


char *SubstituteTextValues(char *RetStr, const char *Text, int MaxLen)
{
const char *ptr, *tptr;
char *Tempstr=NULL, *Name=NULL;
STREAM *S;
int len=0, val;
int Type=0;

RetStr=CopyStr(RetStr,"");
for (ptr=Text; *ptr != '\0'; ptr++)
{
	switch (*ptr)
	{
	case '%':
	ptr++;
	switch (*ptr)
	{
		case '%': 
			RetStr=AddCharToBuffer(RetStr,len++,'%'); 
		break;

		case 'c': 
			tptr=GetCPUSpeed();
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 't': 
			tptr=GetDateStr("%H:%M",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'T': 
			tptr=GetDateStr("%H:%M:%S",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'D': 
			tptr=GetDateStr("%y/%m/%d",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'H': 
			tptr=GetDateStr("%H",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'M': 
			tptr=GetDateStr("%M",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'S': 
			tptr=GetDateStr("%S",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'a': 
			tptr=GetDateStr("%a",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'b': 
			tptr=GetDateStr("%b",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'd': 
			tptr=GetDateStr("%d",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		case 'm': 
			tptr=GetDateStr("%m",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		// 2 digit year
		case 'y': 
			tptr=GetDateStr("%y",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		// 4 digit year
		case 'Y': 
			tptr=GetDateStr("%Y",NULL);
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

		//system load
		case 'L': 
			Tempstr=FormatStr(Tempstr, "%d", GetLoad());
			len+=StrLen(Tempstr);
			RetStr=CatStr(RetStr, Tempstr);
		break;

		//hostname 
		case 'h':
		Tempstr=SetStrLen(Tempstr, 255);
		//don't depend on hostname to set the terminating byte!
		memset(Tempstr,0,255);
		if (gethostname(Tempstr, 255)==0)
		{
			len+=StrLen(Tempstr);
			RetStr=CatStr(RetStr, Tempstr);
		}
		break;

		// memory usage
		case 'u': 
			Tempstr=FormatStr(Tempstr,"%0.1f",GetMemUsage());
			tptr=Tempstr;
			len+=StrLen(tptr);
			RetStr=CatStr(RetStr,tptr);
		break;

	}
	break;

	case '$':
	ptr++;
	switch (*ptr)
	{
		case '$':
		Type=ENVTYPE_SCRIPT;
		ptr++;
		break;

		case 'f':
		case 'F':
		Type=ENVTYPE_FILE;
		ptr++;
		break;

		case 'm':
		Type=ENVTYPE_MMAP;
		ptr++;
		break;
	}

	if (*ptr=='(') ptr++;
	for (tptr=ptr; (*tptr !='\0') && (*tptr != ')'); tptr++);
	Name=CopyStrLen(Name, ptr,tptr-ptr);
	ptr=tptr;

	//default blank value if we can't read from script/file/whatever
	tptr="";
	switch (Type)
	{
		case ENVTYPE_SCRIPT:
		S=STREAMSpawnCommand(Name, COMMS_BY_PIPE | SPAWN_TRUST_COMMAND);
		if (S)
		{
		Tempstr=STREAMReadLine(Tempstr, S);
		StripTrailingWhitespace(Tempstr);
		tptr=Tempstr;
		STREAMClose(S);
		}
		waitpid(-1,NULL,WNOHANG);
		break;

		case ENVTYPE_FILE:
		S=STREAMOpenFile(Name, SF_RDONLY);
		if (S)
		{
		Tempstr=STREAMReadLine(Tempstr, S);
		StripTrailingWhitespace(Tempstr);
		tptr=Tempstr;
		STREAMClose(S);
		}
		break;

		case ENVTYPE_MMAP:
			if (GetValueFromMMap(Name, &Tempstr)) tptr=Tempstr;
		break;

		default:
		tptr=getenv(Name);
		break;
	}

	val=StrLen(tptr);
	if (val > 0)
	{
	RetStr=CatStr(RetStr, tptr);
	len+=val;
	}
	break;

	default: RetStr=AddCharToBuffer(RetStr,len++,*ptr); break;
	}

}

if (len > MaxLen) RetStr[MaxLen]='\0';

DestroyString(Tempstr);
DestroyString(Name);

return(RetStr);
}


