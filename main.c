//Crayonizer. A coloration/formatting app for command-line output
//Written by Colum Paget.
//Copyright 2013 Colum Paget.

/****  Gnu Public Licence ****/
/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "config_file.h"
#include "keypress.h"
#include "signals.h"
#include "help.h"

#define NORM "\x1b[0m"
#define CLRSCR "\x1b[2J\x1b[;H"
#define CTRLO 15

char *Version="1.0";
int GlobalFlags=0;
time_t StartTime=0;


//This handles ANSI sequences that start ESC[, these are 'Control Sequence Introducer' (CSI) codes 
//Some of these we need to detect, becuase the clear the screen or such
char *HandleCSI(char *ptr, char *end)
{
int val, AtEnd=FALSE;

	//special case, some xterm commands
	if (*ptr=='?')
	{
		ptr++;
		if (strncmp(ptr, "47h", 3)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			return(ptr+3);
		}
		else if (strncmp(ptr, "47l", 3)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			return(ptr+3);
		}
		else if (strncmp(ptr, "1049h", 5)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			return(ptr+5);
		}
		else if (strncmp(ptr, "1049l", 5)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			return(ptr+5);
		}
		else if (strncmp(ptr, "1047h", 5)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			return(ptr+5);
		}
		else if (strncmp(ptr, "1047l", 5)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			return(ptr+5);
		}
	}

	while (ptr && (ptr < end))
	{
	switch (*ptr)
	{
		case 'H':
		GlobalFlags |= FLAG_CURSOR_HOME | FLAG_REDRAW;
		AtEnd=TRUE;
		break;

		case 'J':
		if (val=='0') GlobalFlags |= FLAG_REDRAW; //^[0J Clear from cursor to end of screen
		if (val=='1') GlobalFlags |= FLAG_REDRAW; //^[1J Clear from cursor to start of screen
		if (val=='2') GlobalFlags |= FLAG_CURSOR_HOME | FLAG_REDRAW; //^[2J Clear whole Screen 
		AtEnd=TRUE;
		break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		val=*ptr;
		break;

		case ';':
		break;

		default:
		AtEnd=TRUE;
		break;
	}

	ptr++;
	if (*ptr==';') AtEnd=FALSE;
	if (AtEnd) break;
	}

return(ptr);
}


//Handle ANSI coming from the crayonized program 
char *HandleANSI(char *text, char *end)
{
char *ptr;

ptr=text;
if ((GlobalFlags & FLAG_STRIP_ANSI) && (*ptr==CTRLO)) return(ptr+1);

while (*ptr=='\x1b') 
{
ptr++;
if (*ptr=='[') ptr=HandleCSI(ptr+1, end);
else 
{
	while (ptr < end)
	{
		if (
					(isalpha(*ptr) || (*ptr=='@'))
					&&
					(*(ptr+1) != ';') 
				) break;
			ptr++;
	}
	
	//ESC[<val>@ means 'make room for characters to be inserted'
	if (*ptr=='@') return(text);
	if (*ptr=='H') return(text);
	if (ptr!=end) ptr++;
}
}

if (GlobalFlags & FLAG_STRIP_ANSI) return(ptr);
return(text);
}


int ColorProgramOutput(STREAM *Pipe, ListNode *CrayonList)
{
	static char *Buffer=NULL;
	static int BuffFill=0;
	char *Tempstr=NULL, *ptr, *ansi_start, *line_start, *end;
	int result, len=0;
	

	if (! Buffer) Buffer=(char *) calloc(1,4097);
	Tempstr=(char *) calloc(1,4097);
	result=STREAMReadBytes(Pipe,Buffer+BuffFill,4096-BuffFill);

	//Although we only read 'result' bytes, we already had 'BuffFill' bytes carried over
	//from last time (we were halfway through an ascii sequence, and expect to have the
	//rest of it in this read)
	result+=BuffFill;

	//Clear out the 'BuffFill' variable, we don't want it to hold it's value
	//for the next call of this function
	BuffFill=0;

	Buffer[result]='\0';

	if (result > 0)
	{
		ptr=Buffer;
		end=Buffer+result;
		while (ptr < end)
		{
			line_start=ptr;
			while (ptr < end) 
			{
				if ((*ptr=='\x1b')  || (*ptr==CTRLO))
				{

					ansi_start=ptr;
					ptr=HandleANSI(ptr,end);

					//if ptr != ansi_start then strip ansi is active
					if (ptr != ansi_start) continue;

			/*
					if (GlobalFlags & FLAG_CURSOR_HOME)
					{
						//Flush what we have
						if (len >0) Crayonize(Pipe, Tempstr,len,CrayonList);
						//result=(ptr-ansi_start) +1;
						//write(1,ansi_start,result);
						len=0;
						GlobalFlags &= ~FLAG_CURSOR_HOME;
						LineNo=-1;
						break;
					}
		*/
				
					if (ptr==end) 
					{
						//We have part of an ANSI sequence, with a bit missing
						//So save this in 'buffer' and recall
						BuffFill=end-ansi_start;
						memmove(Buffer,ansi_start,BuffFill);
						break;
					}

					//if we stripped something in HandleANSI, then consider the new piece of text
					if (ptr != ansi_start) continue;
				}


				//if we got through all the above, then we add the character to the
				//Line. If the character is a \n, then we break out of the inner loop
				Tempstr[len]=*ptr;
				len++;
				if (*ptr == '\n') break;

				ptr++; 
			}

			if ((ptr==end) && (*ptr != '\n') && (GlobalFlags & FLAG_EXPECT_LINES))
			{
					BuffFill=ptr-line_start;
					memmove(Buffer,line_start,BuffFill);
					continue;
			}

//fprintf(stderr,"CL: %d [%s]\n",GlobalFlags & FLAG_STRIP_ANSI, Tempstr); fflush(NULL);
			if (len >0) Crayonize(Pipe, Tempstr,len,CrayonList);
			if (GlobalFlags & FLAG_CURSOR_HOME)
			{
						GlobalFlags &= ~FLAG_CURSOR_HOME;
						LineNo=-1;
			}
			LineNo++;
			ptr++;
			len=0;
		}

		//By now, whatever happens, we'll have drawn our line, so if we need to 
		//refresh status bars because we cleared the screen, then we do so here
		if (GlobalFlags & FLAG_REDRAW)
		{
			UpdateStatusBars(TRUE);
			GlobalFlags &= ~FLAG_REDRAW;	
		}
	}

	DestroyString(Tempstr);

	//Don't do this, is static
	//DestroyString(Buffer);
	return(result);
}



void ProcessAppends(STREAM *Pipe, ListNode *Crayons, int Type)
{
TCrayon *Item;
ListNode *Curr;
int i, Len;
int *Attribs=NULL;

if (Type==CRAYON_PREPEND) GlobalFlags |= FLAG_DOING_PREPENDS;
if (Type==CRAYON_ONSTART) GlobalFlags |= FLAG_DOING_PREPENDS;
if (Type==CRAYON_APPEND) GlobalFlags |= FLAG_DOING_APPENDS;
if (Type==CRAYON_ONEXIT) GlobalFlags |= FLAG_DOING_APPENDS;

if (GlobalFlags & HAS_STATUSBAR) SetupStatusBars();

Curr=ListGetNext(Crayons);
while (Curr)
{
	Item=(TCrayon *) Curr->Item;

	if (
			(Item->Type==Type) || 
			(Item->Type==CRAYON_IF) || 
			(Item->Type==CRAYON_ARGS)
		)
	{
		Len=StrLen(Item->Match);
		
		Attribs=(int *) realloc(Attribs,Len*sizeof(int));
		ApplyActions(Pipe, Attribs, Item->Match, Len, Item->Match, Item->Match+Len,Item); 

		if (GlobalFlags & FLAG_DONTCRAYON) break;

	}
Curr=ListGetNext(Curr);
}

GlobalFlags &= ~(FLAG_DOING_APPENDS | FLAG_DOING_PREPENDS);

free(Attribs);
}	


char *RebuildPath(char *RetStr, char *Path, const char *CrayonizerDir)
{
char *NewPath=NULL, *Token=NULL, *ptr;

NewPath=CopyStr(RetStr,"");
ptr=GetToken(Path,":",&Token,0);
while (ptr)
{
  if (strcmp(Token,CrayonizerDir) !=0) NewPath=MCatStr(NewPath,Token,":",NULL);
  ptr=GetToken(ptr,":",&Token,0);
}

DestroyString(Token);
return(NewPath);
}



void ProcessCmdLine(char *CmdLine, ListNode *Crayons)
{
TCrayon *Item;
ListNode *Curr;

Curr=ListGetNext(Crayons);
while (Curr)
{
	Item=(TCrayon *) Curr->Item;
	if (Item->Type==CRAYON_CMDLINE)
	{
		 ColorSubstring(NULL, NULL, CmdLine, StrLen(CmdLine), Item);
	}
	Curr=ListGetNext(Curr);
}

}	


char *RebuildCommandLine(char *RetStr, int argc, char *argv[], const char *CrayonizerDir)
{
char *CommandLine=NULL, *Tempstr=NULL, *ptr;
int i;


ptr=GetVar(Vars, "ReplaceCommand");
if (StrLen(ptr)) return(CopyStr(RetStr,ptr));

Tempstr=RebuildPath(Tempstr, getenv("PATH"), CrayonizerDir);
CommandLine=RetStr;
CommandLine=FindFileInPath(CommandLine, argv[0], Tempstr);
CommandLine=MCatStr(CommandLine, " ",GetVar(Vars,"ExtraCmdLineOptions")," ",NULL);
for (i=1; i < argc; i++)
{
	Tempstr=QuoteCharsInStr(Tempstr,argv[i]," 	()");
	CommandLine=MCatStr(CommandLine,Tempstr," ",NULL);
}

DestroyString(Tempstr);
return(CommandLine);
}





STREAM *LaunchCommands(int argc, char *argv[], ListNode *Matches, const char *CrayonizerDir)
{
char *Tempstr=NULL;
ListNode *Curr;
STREAM *Pipe;
TCrayon *Crayon;
int i;

Curr=ListGetNext(Matches);
while (Curr)
{
Crayon=(TCrayon *) Curr->Item;

/*
if (Crayon->Type==CRAYON_EXEC) 
{
	if (StrLen(Tempstr)) Tempstr=MCatStr(Tempstr,";",Crayon->Match,NULL);
	else Tempstr=CopyStr(Tempstr,Crayon->Match);
}
*/

Curr=ListGetNext(Curr);
}


Tempstr=RebuildCommandLine(Tempstr,argc, argv, CrayonizerDir);

if (GlobalFlags & FLAG_DONTCRAYON)
{
	//Exit after running the command
	exit(system(Tempstr));
}

Pipe=STREAMSpawnCommand(Tempstr,COMMS_BY_PTY|SPAWN_TRUST_COMMAND|TTYFLAG_ECHO|TTYFLAG_CANON|TTYFLAG_CRLF|COMMS_COMBINE_STDERR);
signal(SIGTERM, HandleSignal);
signal(SIGINT, HandleSignal);
signal(SIGWINCH, HandleSignal);

//Set initial window size, as though we'd received a SIGWINCH
HandleSigwinch(Pipe);

DestroyString(Tempstr);
return(Pipe);
}



void LoadEnvironment()
{
char *Token=NULL, *ptr;
int i;

for (i=0; environ[i] !=NULL; i++)
{
ptr=GetToken(environ[i],"=",&Token,0);
SetVar(Vars,Token,ptr);
}

DestroyString(Token);
}



void CrayonizerProcessInputs()
{
struct timeval tv;
STREAM *S;
int result;

time(&Now);
tv.tv_sec=1;
tv.tv_usec=0;

while (1)
{
  S=STREAMSelect(Streams,&tv);

	if ((tv.tv_sec==0) && (tv.tv_usec==0))
	{
		tv.tv_sec=1;
		tv.tv_usec=0;
		Now++;
	}

  if (S)
  {
    if (S==StdIn) result=KeypressProcess(StdIn,CommandPipe);
    else result=ColorProgramOutput(CommandPipe, CrayonList);

    if (result == STREAM_CLOSED) break;
  }
  else UpdateStatusBars(FALSE);
  PropogateSignals(CommandPipe);
	ProcessTimers(CommandPipe);
}
}



//Spawns a command, reads text from it and 'Crayonizes' it
void CrayonizeCommand(int argc, char *argv[])
{
STREAM *S;
char *Tempstr=NULL, *CrayonizerDir=NULL, *CmdLine=NULL;
int val, i, result;

CrayonList=ListCreate();
for (i=0; i < argc; i++) CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
StripTrailingWhitespace(CmdLine);

ConfigLoad(CmdLine, &CrayonizerDir, CrayonList);

if (! StrLen(CrayonizerDir))
{
		printf("ERROR! You must specify 'CrayonizerDir' in Crayonizer Config File\n");
		printf("Failure to do so can result in crayonizer calling itself resulting in\n");
		printf("an infinite loop of crayonizer processes (forkbomb)\n");
		printf("AND YOU DON'T WANT THAT!\n");
		exit(1);
}

LoadEnvironment();

//if not outputing to a tty then run
//if (! isatty(1)) GlobalFlags |=FLAG_DONTCRAYON;
if (! ListSize(CrayonList)) GlobalFlags |=FLAG_DONTCRAYON;


//Okay, we're committed to running the command!
Streams=ListCreate();




//if (KeypressFlags & KEYPRESS_PASSINPUT)
if (isatty(0))
{
	InitTTY(0, 0,TTYFLAG_LFCR|TTYFLAG_IGNSIG);
	StdIn=STREAMFromFD(0);
	STREAMSetTimeout(StdIn,10);
	ListAddItem(Streams,StdIn);
}
HandleSigwinch(NULL);

ProcessCmdLine(CmdLine, CrayonList);

if (GlobalFlags & HAS_FOCUS) 
{
	Tempstr=CopyStr(Tempstr,"\x1b[?1004;1043h");
	write(1, Tempstr, StrLen(Tempstr));
}

//We do non-text 'onstart' items before text-output prepends
ProcessAppends(NULL, CrayonList, CRAYON_ONSTART);
ProcessAppends(NULL, CrayonList, CRAYON_PREPEND);


CommandPipe=LaunchCommands(argc, argv, CrayonList, CrayonizerDir);


ListAddItem(Streams,CommandPipe);
CrayonizerProcessInputs();
wait(&val);

Tempstr=FormatStr(Tempstr,"%d",time(NULL) - StartTime);
SetVar(Vars,"duration",Tempstr);
ProcessAppends(CommandPipe,CrayonList,CRAYON_APPEND);
ProcessAppends(CommandPipe,CrayonList,CRAYON_ONEXIT);

STREAMClose(CommandPipe);
STREAMDisassociateFromFD(StdIn);
ListDestroy(Streams,NULL);
ListDestroy(CrayonList,free);
DestroyString(Tempstr);
DestroyString(CmdLine);
DestroyString(CrayonizerDir);
}



void CrayonizeSTDIN(int argc, char *argv[])
{
char *Tempstr=NULL, *CrayonizerDir=NULL, *CmdLine=NULL;
ListNode *CrayonList;
int val, i;

CrayonList=ListCreate();
for (i=0; i < argc; i++) CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
ConfigLoad(CmdLine, &CrayonizerDir, CrayonList);

StdIn=STREAMFromFD(0);

Tempstr=STREAMReadLine(Tempstr,StdIn);
while (Tempstr)
{
Crayonize(StdIn, Tempstr,StrLen(Tempstr),CrayonList);
LineNo++;
Tempstr=STREAMReadLine(Tempstr,StdIn);
}

STREAMDisassociateFromFD(StdIn);
ListDestroy(CrayonList,free);
DestroyString(CrayonizerDir);
DestroyString(CmdLine);
DestroyString(Tempstr);
}



void CalledAsSelf(int argc, char *argv[])
{
const char *Args[]={"-v","-version","--version","-?","-h","-help","--help","-config-help","-pmatch-help","-stdin",NULL};
typedef enum {ARG_V1, ARG_V2, ARG_V3, ARG_H1, ARG_H2, ARG_H3, ARG_H4, ARG_CONFIG_HELP, ARG_PMATCH_HELP, ARG_STDIN};
int i, val, new_argc=0;
char **new_argv=NULL;


for (i=1; i < argc; i++)
{
	val=MatchTokenFromList(argv[i],Args,0);
	
	switch(val)
	{
		case ARG_V1:
		case ARG_V2:
		case ARG_V3:
			printf("crayonizer (Crayonizer) %s\n",Version);
		break;
		
		case ARG_H1:
		case ARG_H2:
		case ARG_H3:
		case ARG_H4:
			PrintUsage();
		break;

		case ARG_CONFIG_HELP:
			PrintConfigFileHelp();
		break;

		case ARG_PMATCH_HELP:
			PrintPMatchHelp();
		break;

		case ARG_STDIN:
			CrayonizeSTDIN(argc, argv);
		break;

		default:
			new_argc++;
			new_argv=(char **) realloc(new_argv,sizeof(char *) * new_argc);
			new_argv[new_argc-1]=argv[i];
		break;
	}
	
}

if (new_argc > 0) CrayonizeCommand(new_argc,new_argv);
}



void CrayonizerGetEnvironment()
{
char *Tempstr=NULL, *ptr;
int val;

//if we are going to restore the title at the end, then we need to set it here
/*
if (GlobalFlags & FLAG_RESTORE_XTITLE) 
{
	Tempstr=XTermReadValue(Tempstr, "\x1b[21;t", "\x1b]l");
  if (StrLen(Tempstr)) SetVar(Vars,"OldXtermTitle",Tempstr);
}
*/

Tempstr=CopyStr(Tempstr,getenv("TERM"));
if (StrLen(Tempstr)) SetVar(Vars,"TERM",Tempstr);

if (strcmp(Tempstr,"linux")==0)
{
					SetVar(Vars,"BACKCOLOR","black");
					SetVar(Vars,"BACKTONE","dark");
}
else if (strncmp(Tempstr,"xterm",5)==0)
{
					SetVar(Vars,"BACKCOLOR","black");
					SetVar(Vars,"BACKTONE","dark");
}
else if (strncmp(Tempstr,"rxvt",4)==0)
{
					SetVar(Vars,"BACKCOLOR","white");
					SetVar(Vars,"BACKTONE","light");
}

Tempstr=CopyStr(Tempstr,getenv("COLORFGBG"));

if (StrLen(Tempstr)) 
{
			ptr=strchr(Tempstr,';');
			if (ptr) 
			{
				*ptr='\0';
				ptr++;
				SetVar(Vars,"COLORFG",Tempstr);

/*
	ANSI	 terminfo equivalent	 Description
[ 4 0 m	setab 0	 Set background to color #0 - black
[ 4 1 m	setab 1	 Set background to color #1 - red
[ 4 2 m	setab 2	 Set background to color #2 - green
[ 4 3 m	setab 3	 Set background to color #3 - yellow
[ 4 4 m	setab 4	 Set background to color #4 - blue
[ 4 5 m	setab 5	 Set background to color #5 - magenta
[ 4 6 m	setab 6	 Set background to color #6 - cyan
[ 4 7 m	setab 7	 Set background to color #7 - white
*/

				switch (atoi(ptr))
				{
					case 0:
					case 8:
					SetVar(Vars,"BACKCOLOR","black");
					SetVar(Vars,"BACKTONE","dark");
					break;

					case 1:
					case 9:
					SetVar(Vars,"BACKCOLOR","red");
					SetVar(Vars,"BACKTONE","dark");
					break;

					case 2:
					case 10:
					SetVar(Vars,"BACKCOLOR","green");
					SetVar(Vars,"BACKTONE","dark");
					break;

					case 3:
					case 11:
					SetVar(Vars,"BACKCOLOR","yellow");
					SetVar(Vars,"BACKTONE","light");
					break;

					case 4:
					case 12:
					SetVar(Vars,"BACKCOLOR","blue");
					SetVar(Vars,"BACKTONE","dark");
					break;

					case 5:
					case 13:
					SetVar(Vars,"BACKCOLOR","magenta");
					SetVar(Vars,"BACKTONE","light");
					break;

					case 6:
					case 14:
					SetVar(Vars,"BACKCOLOR","cyan");
					SetVar(Vars,"BACKTONE","light");
					break;

					case 7:
					case 15:
					SetVar(Vars,"BACKCOLOR","white");
					SetVar(Vars,"BACKTONE","light");
					break;
				}
			}
}


/*
printf("TERM: %s\n",GetVar(Vars,"TERM"));
printf("BGC:: %s\n",GetVar(Vars,"BACKCOLOR"));
printf("BGTT: %s\n",GetVar(Vars,"BACKTONE"));
*/

DestroyString(Tempstr);
}



main(int argc, char *argv[])
{
char *ptr;

cmdline_argc=argc;
cmdline_argv=argv;

Vars=ListCreate();
time(&StartTime);
CrayonizerGetEnvironment();
GlobalFlags=FLAG_FOCUSED;

if (strcmp(basename(argv[0]),"crayonizer")==0) CalledAsSelf(argc, argv);
else 
{
	ptr=basename(argv[0]);
	argv[0]=ptr;
	CrayonizeCommand(argc,argv);
}
if (isatty(0)) ResetTTY(0);
}
