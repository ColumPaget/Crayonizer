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
#include "escape_sequences.h"
#include "crayonizations.h"
#include "config_file.h"
#include "command_line.h"
#include "status_bar.h"
#include "timers.h"
#include "xterm.h"
#include "history.h"
#include "help.h"
#include <wait.h>

#define NORM "\x1b[0m"
#define CLRSCR "\x1b[2J\x1b[;H"

char *Version="2.2";
char *ConfigPaths=NULL;
int GlobalFlags=0;




int ColorProgramOutput(STREAM *Pipe, ListNode *CrayonList)
{
  static char *Buffer=NULL;
  static int BuffFill=0;
  char *line_start, *start, *end, *ptr;
  int result, len=0;
	char *Tempstr=NULL;

  if (! Buffer) Buffer=(char *) calloc(1,4097);
  result=STREAMReadBytes(Pipe,Buffer+BuffFill,4096-BuffFill);

  //Although we only read 'result' bytes, we already had 'BuffFill' bytes carried over
  //from last time (we were halfway through an ascii sequence, and expect to have the
  //rest of it in this read)
  result+=BuffFill;


  //if we fail to consume all the string, we will reset BuffFill to the remainder, so here we zero it
  BuffFill=0;

  if (result < 1) return(STREAM_CLOSED);

  StrTrunc(Buffer, result);
	//fprintf(stderr, "READ: [%s]\n", Buffer);

  end=Buffer+result;
  start=Buffer;
  for (ptr=start; ptr < end; )
  {
		switch (*ptr)
		{
			case '\x1b':
			case CTRLO:
				if (ptr > start) 
				{
								Crayonize(Pipe, start, ptr-start, FALSE, CrayonList);
				Tempstr=CopyStrLen(Tempstr, start, ptr - start);
				//fprintf(stderr, "RN: [%s]\n", Tempstr);
  
				}
				start=ptr;
        switch (EscapeSequenceHandle(&ptr,end))
        {
          case ES_STRIP:
          break;

          case ES_PART:
            //We have part of an ANSI sequence, with a bit missing. We are at the end of the sequence, so we move it to
						//the start of the Buffer and set BuffFill to be the length of this part.
            BuffFill=end-start;
            memmove(Buffer,start,BuffFill);
            return(BuffFill);
          break;

          case ES_OKAY:
						Crayonize(Pipe, start, ptr-start, FALSE, CrayonList);
						Tempstr=CopyStrLen(Tempstr, start, ptr-start);
						//fprintf(stderr, "ESCOK: [%s]\n", Tempstr);
						start=ptr;
          break;
        }
			break;

			case '\n':
        ptr++;
        Crayonize(Pipe, start, ptr - start, FALSE, CrayonList);
				Tempstr=CopyStrLen(Tempstr, start, ptr - start);
				//fprintf(stderr, "LN: [%s]\n", Tempstr);
        LineNo++;
				start=ptr;
			break;

			default:
				ptr++;
			break;
		}
	}


result=ptr - start;
if (result > 0)
{
	if (GlobalFlags & FLAG_EXPECT_LINES)
	{
 	  memmove(Buffer, start, result);
		BuffFill=result;
 	  return(BuffFill);
	}
	else 
	{
		Crayonize(Pipe, start, result, FALSE, CrayonList);
		//fprintf(stderr, "EN: [%s]\n", Tempstr);
	}
}

//By now, whatever happens, we'll have drawn our line, so if we need to 
//refresh status bars because we cleared the screen, then we do so here
if (GlobalFlags & FLAG_REDRAW)
{
	UpdateStatusBars(TRUE);
	GlobalFlags &= ~FLAG_REDRAW;	
}

//Don't do this, is static
//Destroy(Buffer);
return(BuffFill);
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

//if (GlobalFlags & HAS_STATUSBAR) SetupStatusBars();

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





STREAM *LaunchCommands(int argc, char *argv[], ListNode *Matches, const char *CrayonizerDir)
{
char *Tempstr=NULL;
ListNode *Curr;
STREAM *Pipe=NULL;
TCrayon *Crayon;
int i;

Curr=ListGetNext(Matches);
while (Curr)
{
Crayon=(TCrayon *) Curr->Item;

/*
if (Crayon->Type==CRAYON_EXEC) 
{
	if (StrValid(Tempstr)) Tempstr=MCatStr(Tempstr,";",Crayon->Match,NULL);
	else Tempstr=CopyStr(Tempstr,Crayon->Match);
}
*/

Curr=ListGetNext(Curr);
}

//if child crayonizations are not allowed (only one crayonizer in this
//session) then set the CRAYONIZER environment variable so sub-processes
//know not to run crayonizer
if (! (GlobalFlags & FLAG_CHILDCRAYON)) 
{
	Tempstr=FormatStr(Tempstr, "%d", getpid());
	setenv("CRAYONIZER", Tempstr, TRUE);
}

Tempstr=RebuildCommandLine(Tempstr,argc, argv, CrayonizerDir);

if (GlobalFlags & FLAG_DONTCRAYON)
{
	//Exit after running the command
	TTYReset(0);
	exit(system(Tempstr));
}

Pipe=STREAMSpawnCommand(Tempstr, "pty echo canon icrlf trust");
signal(SIGTERM, HandleSignal);
signal(SIGINT, HandleSignal);
signal(SIGWINCH, HandleSignal);

//Set initial window size, as though we'd received a SIGWINCH
HandleSigwinch(Pipe);

Destroy(Tempstr);
return(Pipe);
}



void LoadEnvironment()
{
char *Token=NULL;
const char *ptr;
int i;

for (i=0; environ[i] !=NULL; i++)
{
ptr=GetToken(environ[i],"=",&Token,0);
SetVar(Vars,Token,ptr);
}

Destroy(Token);
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

		fflush(stdout);
    if (result == STREAM_CLOSED) break;
  }
  else UpdateStatusBars(FALSE);
  PropagateSignals(CommandPipe);
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
for (i=0; i < argc; i++) 
{
	if (strchr(argv[i],' ')) CmdLine=MCatStr(CmdLine,"'",argv[i],"' ",NULL);
	else CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
}
StripTrailingWhitespace(CmdLine);

if (ConfigLoad(CmdLine, ConfigPaths, &CrayonizerDir, CrayonList))
{
	if (! StrValid(CrayonizerDir))
	{
		printf("ERROR! You must specify 'CrayonizerDir' in Crayonizer Config File\n");
		printf("Failure to do so can result in crayonizer calling itself resulting in\n");
		printf("an infinite loop of crayonizer processes (forkbomb)\n");
		printf("AND YOU DON'T WANT THAT!\n");
		exit(1);
	}
}

LoadEnvironment();

if (! ListSize(CrayonList)) GlobalFlags |=FLAG_DONTCRAYON;

if (isatty(0))
{
	if (! (GlobalFlags & FLAG_DONTCRAYON)) TTYConfig(0, 0,TTYFLAG_OUT_CRLF | TTYFLAG_IGNSIG | TTYFLAG_SAVE);
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

CrayonizerProcessInputs(Streams);
wait(&val);

SetDurationVariable();

ProcessAppends(CommandPipe,CrayonList,CRAYON_APPEND);
ProcessAppends(CommandPipe,CrayonList,CRAYON_ONEXIT);

STREAMClose(CommandPipe);
STREAMDestroy(StdIn);
ListDestroy(Streams,NULL);
ListDestroy(CrayonList,free);
Destroy(Tempstr);
Destroy(CmdLine);
Destroy(CrayonizerDir);
}



void CrayonizeSTDIN(int argc, char *argv[])
{
char *Tempstr=NULL, *CrayonizerDir=NULL, *CmdLine=NULL;
ListNode *CrayonList;
int val, i;

CrayonList=ListCreate();
for (i=0; i < argc; i++) CmdLine=MCatStr(CmdLine,argv[i]," ",NULL);
ConfigLoad(CmdLine, ConfigPaths, &CrayonizerDir, CrayonList);

StdIn=STREAMFromFD(0);

Tempstr=STREAMReadLine(Tempstr,StdIn);
while (Tempstr)
{
Crayonize(StdIn, Tempstr,StrLen(Tempstr),FALSE,CrayonList);
LineNo++;
Tempstr=STREAMReadLine(Tempstr,StdIn);
}

STREAMDestroy(StdIn);
ListDestroy(CrayonList,free);
Destroy(CrayonizerDir);
Destroy(CmdLine);
Destroy(Tempstr);
}



void CalledAsSelf(int argc, char *argv[])
{
const char *Args[]={"-v","-version","--version","-?","-h","-help","--help","-config-help","-pmatch-help","-stdin","-c",NULL};
typedef enum {ARG_V1, ARG_V2, ARG_V3, ARG_H1, ARG_H2, ARG_H3, ARG_H4, ARG_CONFIG_HELP, ARG_PMATCH_HELP, ARG_STDIN, ARG_CONFIG_PATH} TCmdArgs;
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

		case ARG_CONFIG_PATH:
			ConfigPaths=CopyStr(ConfigPaths, argv[++i]);
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
if (GlobalFlags & FLAG_RESTORE_XTITLE) 
{
	Tempstr=XTermReadValue(Tempstr, "\x1b[21;t", "\x1b]l");
  if (StrValid(Tempstr)) SetVar(Vars,"crayon_old_xtitle",Tempstr);
}

ptr=getenv("CRAYONIZER");
if (ptr && (atoi(ptr) > 0)) GlobalFlags |= FLAG_DONTCRAYON;


Tempstr=CopyStr(Tempstr,getenv("TERM"));
if (StrValid(Tempstr)) SetVar(Vars,"TERM",Tempstr);

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

if (StrValid(Tempstr)) 
{
			ptr=strchr(Tempstr,';');
			if (ptr) 
			{
				StrTrunc(Tempstr, ptr-Tempstr);
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

Destroy(Tempstr);
}



main(int argc, char *argv[])
{
char *ptr;

cmdline_argc=argc;
cmdline_argv=argv;

Streams=ListCreate();
Vars=ListCreate();
TypingHistoryActivate(10);

time(&StartTime);
CrayonizerGetEnvironment();
GlobalFlags |= FLAG_FOCUSED;
ConfigPaths=CopyStr(ConfigPaths, "'$(UserCrayonizerDir)/$(Command).conf','$(UserDir)/.crayonizer.conf','$(SystemCrayonizerDir)/$(Command).conf','$(SystemConfigDir)/crayonizer.conf','$(SystemCrayonizerDir)/crayonizer.conf'");

if (strcmp(basename(argv[0]),"crayonizer")==0) CalledAsSelf(argc, argv);
else 
{
	ptr=basename(argv[0]);
	argv[0]=ptr;
	CrayonizeCommand(argc,argv);
}
TTYReset(0);
}
