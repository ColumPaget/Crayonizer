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
#include "help.h"
#include <sys/ioctl.h>

#define NORM "\x1b[0m"
#define CLRSCR "\x1b[2J\x1b[;H"
#define CTRLO 15

char *Version="0.0.8";
int GlobalFlags=0;
TCrayon *KeyPresses=NULL;
int NoOfKeyPresses=0;
int StartTime=0;


void HandleSigwinch(STREAM *Pipe)
{
    struct winsize w;
		char *Tempstr=NULL;

    ioctl(0, TIOCGWINSZ, &w);

		ScreenRows=w.ws_row;
		if (GlobalFlags & HAS_STATUSBAR) w.ws_row--;	
		w.ws_row--;	
    ioctl(Pipe->out_fd, TIOCSWINSZ, &w);
		

		DestroyString(Tempstr);
}


void HandleSignal(int sig)
{
int wid, len;

if (sig==SIGWINCH) GlobalFlags |= GOT_SIGWINCH;
if (sig==SIGTERM) GlobalFlags |= GOT_SIGTERM;
if (sig==SIGINT) GlobalFlags |= GOT_SIGINT;
}

void PropogateSignals(STREAM *Pipe)
{
int PeerPID;

PeerPID=atoi(STREAMGetValue(Pipe,"PeerPID"));
if (GlobalFlags & GOT_SIGWINCH) 
{
	HandleSigwinch(Pipe);
	//kill(PeerPID,SIGWINCH);
}

if (GlobalFlags & GOT_SIGTERM) kill(PeerPID,SIGTERM);
if (GlobalFlags & GOT_SIGINT) kill(PeerPID,SIGINT);
GlobalFlags &= ~(GOT_SIGWINCH | GOT_SIGTERM | GOT_SIGINT);
}


int HandleStdin(STREAM *StdIn, STREAM *Out)
{
int result, i;
char *Tempstr=NULL;

Tempstr=SetStrLen(Tempstr,4096);
result=STREAMReadBytes(StdIn,Tempstr,4096);
if (result > 0) 
{
	for (i=0; i < NoOfKeyPresses; i++)
	{
		if (strcmp(KeyPresses[i].Match,Tempstr)==0) 
		{
			ApplyActions(Out,NULL, NULL, 0, NULL, NULL, KeyPresses+i); 
			break;
		}
	}


	if (i==NoOfKeyPresses)
	{
	STREAMWriteBytes(Out,Tempstr,result);
	STREAMFlush(Out);
	}
}

DestroyString(Tempstr);
return(result);
}


char *HandleANSI(char *text, char *end)
{
char *ptr;

ptr=text;
ptr++;
if (*ptr=='[')
{
	ptr++;

	//Horrible switch function to handle all the possibilities. 
	//Hopefully it's faster than just doing strcmps
	switch (*ptr)
	{
		case 'H':
		GlobalFlags |= FLAG_CURSOR_HOME;
		return(ptr);	
		break;


		case 'K':
		return(text);	
		break;
	
		case ';':
		ptr++;
		if (*ptr=='H')
		{
			GlobalFlags |= FLAG_CURSOR_HOME;
			return(ptr);
		}
		break;

		case '0':
		if (strncmp(ptr,"0;0H",4)==0) 
		{
			GlobalFlags |= FLAG_CURSOR_HOME;
			return(ptr+3);
		}
		break;

		case '2':
		if (strncmp(ptr,"2J",2)==0) 
		{
			GlobalFlags |= FLAG_CURSOR_HOME;
			return(ptr+1);
		}
		break;
	}
}


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
if (ptr==end) return(ptr);

ptr++;
return(ptr);
}


int ColorProgramOutput(STREAM *Pipe, ListNode *ColorMatches)
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
				if (
							((*ptr=='\x1b') || (*ptr==CTRLO)) &&
							(GlobalFlags & FLAG_STRIP_ANSI)
					)
				{
					if (*ptr==CTRLO) 
					{
						ptr++;
						continue;
					}

					ansi_start=ptr;
					ptr=HandleANSI(ptr,end);

					if (GlobalFlags & FLAG_CURSOR_HOME)
					{
						//Flush what we have
						if (len >0) ColorLine(Pipe, Tempstr,len,ColorMatches);
						result=(ptr-ansi_start) +1;
						write(1,ansi_start,result);
						len=0;
						GlobalFlags &= ~FLAG_CURSOR_HOME;
						LineNo=-1;
						break;
					}
				
					if (ptr==end) 
					{
						//We have part of an ASCI sequence, with a bit missing
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

	//fprintf(stderr,"ColorLine: [%s]\n",Tempstr);
			if (len >0) ColorLine(Pipe, Tempstr,len,ColorMatches);
			LineNo++;
			ptr++;
			len=0;
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
if (Type==CRAYON_APPEND) GlobalFlags |= FLAG_DOING_APPENDS;

Curr=ListGetNext(Crayons);
while (Curr)
{
	Item=(TCrayon *) Curr->Item;
	if (Item->Type==CRAYON_STATUSBAR) DrawStatusBar(Item->String);

	if (
			(Item->Type==Type) || (Item->Type==CRAYON_IF) || (Item->Type==CRAYON_ARGS) 
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


char *RebuildCommandLine(char *RetStr, int argc, char *argv[])
{
char *CommandLine=NULL, *Quoted=NULL;
int i;


CommandLine=MCopyStr(RetStr, argv[0]," ",GetVar(Vars,"ExtraCmdLineOptions")," ",NULL);
for (i=1; i < argc; i++)
{
	Quoted=QuoteCharsInStr(Quoted,argv[i]," 	&|");
	CommandLine=MCatStr(CommandLine,Quoted," ",NULL);
}

return(CommandLine);
return(Quoted);
}





STREAM *LaunchCommands(int argc, char *argv[], ListNode *Matches)
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

if (Crayon->Type==CRAYON_EXEC) 
{
	if (StrLen(Tempstr)) Tempstr=MCatStr(Tempstr,";",Crayon->Match,NULL);
	else Tempstr=CopyStr(Tempstr,Crayon->Match);
}

Curr=ListGetNext(Curr);
}


Tempstr=RebuildCommandLine(Tempstr,argc, argv);

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


char *LoadConfig(char *CmdLine, char **CrayonizerDir, ListNode *ColorMatches)
{
int i;
char *Tempstr=NULL;

Tempstr=MCopyStr(Tempstr,GetCurrUserHomeDir(),"/",USER_CONFIG_FILE,NULL);
if (! ConfigReadFile(Tempstr, &CmdLine, CrayonizerDir, ColorMatches))
{
	if (! ConfigReadFile(GLOBAL_CONFIG_FILE, &CmdLine, CrayonizerDir, ColorMatches))
	{
		printf("ERROR! Crayonizer can't find config file. Tried %s and %s\n",Tempstr, GLOBAL_CONFIG_FILE);
		exit(1);
	}
}

return(CmdLine);
}


char *RebuildPath(char *RetStr, char *Path, char *CurrDir)
{
char *NewPath=NULL, *Token=NULL, *ptr;

NewPath=CopyStr(RetStr,"");
ptr=GetToken(Path,":",&Token,0);
while (ptr)
{
  if (strcmp(Token,CurrDir) !=0) NewPath=MCatStr(NewPath,Token,":",NULL);
  ptr=GetToken(ptr,":",&Token,0);
}

DestroyString(Token);
return(NewPath);
}


void LoadEnvironment()
{
char *Token=NULL, *ptr;
int i;

Vars=ListCreate();
for (i=0; environ[i] !=NULL; i++)
{
ptr=GetToken(environ[i],"=",&Token,0);
SetVar(Vars,Token,ptr);
}

DestroyString(Token);
}


//Spawns a command, reads text from it and 'Crayonizes' it
void CrayonizeCommand(int argc, char *argv[])
{
STREAM *StdIn=NULL, *Pipe=NULL, *S;
char *Tempstr=NULL, *CrayonizerDir=NULL, *CmdLine=NULL;
ListNode *ColorMatches, *Streams;
int val, i, result;

ColorMatches=ListCreate();
for (i=0; i < argc; i++) CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
StripTrailingWhitespace(CmdLine);

CmdLine=LoadConfig(CmdLine, &CrayonizerDir, ColorMatches);

if (! StrLen(CrayonizerDir))
{
		printf("ERROR! You must specify 'CrayonizerDir' in Crayonizer Config File\n");
		printf("Failure to do so can result in crayonizer calling itself resulting in\n");
		printf("an infinite loop of crayonizer processes (forkbomb)\n");
		printf("AND YOU DON'T WANT THAT!\n");
		exit(1);
}

Tempstr=RebuildPath(Tempstr,getenv("PATH"),CrayonizerDir);
setenv("PATH",Tempstr,TRUE);
LoadEnvironment();

//if not outputing to a tty then run
//if (! isatty(1)) GlobalFlags |=FLAG_DONTCRAYON;
if (! ListSize(ColorMatches)) GlobalFlags |=FLAG_DONTCRAYON;


//Okay, we're committed to running the command!
Streams=ListCreate();


//if we are going to restore the title at the end, then we need to set it here
//if (GlobalFlags & FLAG_RESTORE_XTITLE) XTermReadValue();


ProcessCmdLine(CmdLine, ColorMatches);
ProcessAppends(Pipe, ColorMatches,CRAYON_PREPEND);


Pipe=LaunchCommands(argc, argv, ColorMatches);


if (GlobalFlags & FLAG_PASSINPUT)
{
	InitTTY(0, 0,TTYFLAG_LFCR|TTYFLAG_IGNSIG);
	StdIn=STREAMFromFD(0);
	ListAddItem(Streams,StdIn);
}


ListAddItem(Streams,Pipe);

while (1)
{
	S=STREAMSelect(Streams,NULL);

	if (S)
	{
		if (S==StdIn) result=HandleStdin(StdIn,Pipe);
		else result=ColorProgramOutput(Pipe, ColorMatches);

		if (result < 0) break;
	}
	PropogateSignals(Pipe);
}
wait(&val);

Tempstr=FormatStr(Tempstr,"%d",time(NULL) - StartTime);
SetVar(Vars,"duration",Tempstr);
ProcessAppends(Pipe,ColorMatches,CRAYON_APPEND);

STREAMClose(Pipe);
STREAMDisassociateFromFD(StdIn);
ResetTTY(0);
ListDestroy(Streams,NULL);
ListDestroy(ColorMatches,free);
DestroyString(Tempstr);
DestroyString(CmdLine);
DestroyString(CrayonizerDir);
}



void CrayonizeSTDIN(int argc, char *argv[])
{
STREAM *StdIn=NULL;
char *Tempstr=NULL, *CrayonizerDir=NULL, *CmdLine=NULL;
ListNode *ColorMatches;
int val, i;

ColorMatches=ListCreate();
for (i=0; i < argc; i++) CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
LoadConfig(CmdLine, &CrayonizerDir, ColorMatches);

StdIn=STREAMFromFD(0);

Tempstr=STREAMReadLine(Tempstr,StdIn);
while (Tempstr)
{
ColorLine(StdIn, Tempstr,StrLen(Tempstr),ColorMatches);
LineNo++;
Tempstr=STREAMReadLine(Tempstr,StdIn);
}

STREAMDisassociateFromFD(StdIn);
ListDestroy(ColorMatches,free);
DestroyString(CrayonizerDir);
DestroyString(CmdLine);
DestroyString(Tempstr);
}



void CalledAsSelf(int argc, char *argv[])
{
char *Args[]={"-v","-version","--version","-?","-h","-help","--help","-config-help","-pmatch-help","-stdin",NULL};
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



main(int argc, char *argv[])
{
char *ptr;

cmdline_argc=argc;
cmdline_argv=argv;

time(&StartTime);

if (strcmp(basename(argv[0]),"crayonizer")==0) CalledAsSelf(argc, argv);
else 
{
	ptr=basename(argv[0]);
	argv[0]=ptr;
	CrayonizeCommand(argc,argv);
}
}
