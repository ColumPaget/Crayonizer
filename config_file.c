#include "config_file.h"
#include "keypress.h"
#include "status_bar.h"
#include "timers.h"

const char *Colors[]={"none","black","red","green","yellow","blue","magenta","cyan","white","none","none","darkgrey","lightred","lightgreen","lightyellow","lightblue","lightmagenta","lightcyan","lightgrey",NULL};
const char *CrayonTypes[]={"action","args","line","string","section","lineno","value","mapto","linemapto","append","prepend","onstart","onexit","cmdline","passinput","include","keypress","if","statusbar","timer","edit",NULL};


char *ParseAttribs(char *Verbs, TCrayon *Crayon, TCrayon **Action);


void MMapSetup()
{
char *FName=NULL, *Tempstr=NULL;
struct stat Stat;
int MMapSize=4096, result;
STREAM *S;

if (! CrayonizerMMap)
{

FName=MCopyStr(FName, GetCurrUserHomeDir(), "/.crayonizer.mmap", NULL);

result=stat(FName, &Stat);
if ((result==-1) || (Stat.st_size < MMapSize))
{
S=STREAMOpenFile(FName, SF_WRONLY | SF_CREAT);
if (S)
{
Tempstr=SetStrLen(Tempstr, MMapSize);
memset(Tempstr, 0, MMapSize);
STREAMWriteBytes(S, Tempstr, MMapSize);
STREAMClose(S);
}

}


S=STREAMOpenFile(FName, SF_RDWR | SF_MMAP);
if (S) CrayonizerMMap=STREAMGetItem(S, "MMap");
}

//Don't close!
//STREAMClose(S);
DestroyString(Tempstr);
DestroyString(FName);
}


//only used by 'IsAttrib'
#define FLAG_COLOR 1


int IsFlag(const char *String)
{
const char *AttribStrings[]={"top","bottom","persist","transient","refresh",NULL};
int AttribFlags[]={FLAG_TOP, FLAG_BOTTOM, FLAG_PERSIST, FLAG_TRANSIENT, FLAG_REFRESH};
int val;

val=MatchTokenFromList(String,AttribStrings,0);
if (val > -1) return(AttribFlags[val]);

return(0);
}


int IsAttrib(const char *String)
{
const char *AttribStrings[]={"caps","bold","hide","blink","uppercase","underline","lowercase","inverse",NULL};
int AttribFlags[]={FLAG_CAPS, FLAG_BOLD, FLAG_HIDE, FLAG_BLINK, FLAG_CAPS, FLAG_UNDER, FLAG_LOWERCASE, FLAG_INVERSE};
int val;

val=MatchTokenFromList(String,AttribStrings,0);
if (val > -1) return(AttribFlags[val]);

return(ParseColor(String));
}



int CrayonType(char *String)
{
int val;

	val=MatchTokenFromList(String,CrayonTypes,0);
	if (val==-1) val=0;
	return(val);
}


int ParseColor(char *ColorString)
{
char *ptr;
int val=0, Attrib=0;

	ptr=strchr(ColorString,'/');
	if (ptr)
	{
		*ptr='\0';
		val=MatchTokenFromList(ptr+1,Colors,0);
		if (val > 0) Attrib=val << 8;
		else *ptr='/';
	}
	val=MatchTokenFromList(ColorString,Colors,0);
	if (val > -1) Attrib |= val;

return(Attrib);
}



TCrayon *NewCrayonAction(TCrayon *Crayon, int Type)
{
TCrayon *Act;

if ((Crayon->ActionCount % 10)==0) 
{
	Crayon->Actions=(TCrayon *) realloc((void *) Crayon->Actions, (Crayon->ActionCount+10) * sizeof(TCrayon));
}

Act=Crayon->Actions + Crayon->ActionCount;
memset(Act,0,sizeof(TCrayon));

Act->Type=CRAYON_ACTION;
Act->ActType=Type;
Crayon->ActionCount++;

return(Act);
}


//This parses those actions that take a string
char *ParseStringAction(char *Line, int Type, TCrayon *Crayon)
{
	TCrayon *Action;
	char *Token=NULL, *ptr, *sptr;

		ptr=GetToken(Line," ",&Token,GETTOKEN_QUOTES);

		Action=NewCrayonAction(Crayon,Type);
		
		for (sptr=Token; *sptr != '\0'; sptr++)
		{
			if (*sptr=='\\')
			{
				sptr++;
				switch (*sptr)
				{
					case 'a': Action->String=AddCharToStr(Action->String,0x07); break;
					case 'b': Action->String=AddCharToStr(Action->String,0x08); break;
					case 'c': 
						sptr++;
						if ((*sptr > 64) && (*sptr < 91)) Action->String=AddCharToStr(Action->String,*sptr-64);
						if ((*sptr > 96) && (*sptr < 123)) Action->String=AddCharToStr(Action->String,*sptr-96);
					break;
					case 'n': Action->String=AddCharToStr(Action->String,'\n'); break;
					case 'r': Action->String=AddCharToStr(Action->String,'\r'); break;
					case 't': Action->String=AddCharToStr(Action->String,'	'); break;
					default: Action->String=AddCharToStr(Action->String, *sptr); break;
				}
			}
			else Action->String=AddCharToStr(Action->String, *sptr);
		}


		DestroyString(Token);

return(ptr);
}



char *ParseStatusBar(int Type, TCrayon *Crayon, char *Config)
{
char *Token=NULL, *ptr;
TCrayon *StatusBar, *Action;
int val;

		StatusBar=NewCrayonAction(Crayon, Type);
		StatusBar->Value=10;
		//parse attribs
		ptr=GetToken(Config,"\\S",&Token,GETTOKEN_QUOTES);
		while (ptr)
		{
		  val=IsAttrib(Token);
			if (val) StatusBar->Attribs |= val;
			else
			{
				val=IsFlag(Token);
				if (val==FLAG_REFRESH)
				{
					ptr=GetToken(ptr," ",&Token,GETTOKEN_QUOTES);
					StatusBar->Value=atof(Token);
				}

				StatusBar->Flags |= val;
			}

			if (! val) break;
			ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
		}

		//parse selection list and action
		StatusBar->Match=CopyStr(StatusBar->Match, Token);
		ptr=GetToken(ptr, "\\S", &StatusBar->String, GETTOKEN_QUOTES);

		if (strstr(StatusBar->Match, "$m")) MMapSetup();
		Action=NewCrayonAction(StatusBar, ACTION_NONE);
		ptr=ParseAttribs(ptr, StatusBar, &Action);

		DestroyString(Token);

		return(ptr);
}



char *ParseAttribs(char *Verbs, TCrayon *Crayon, TCrayon **Action)
{
char *Token=NULL, *ptr;
int val=0;
 
//Do this before anything else!
	ptr=GetToken(Verbs," ",&Token,0);

	val=IsAttrib(Token);
	//Is Attrib actually returns attribs, so if it's >0 we can use it to set
	//attribs
	if (val) (*Action)->Attribs |= val;
	else if (strcasecmp(Token,"clrtoeol")==0) (*Action)->Attribs |= FLAG_CLR2EOL;
	else if (strcasecmp(Token,"basename")==0) *Action=NewCrayonAction(Crayon, ACTION_BASENAME);
	else if (strcasecmp(Token,"setxtitle")==0) *Action=NewCrayonAction(Crayon, ACTION_SET_XTITLE);
	else if (strcasecmp(Token,"setxlabel")==0) *Action=NewCrayonAction(Crayon, ACTION_SET_XICONNAME);
	else if (strcasecmp(Token,"raise")==0) *Action=NewCrayonAction(Crayon, ACTION_XRAISE);
	else if (strcasecmp(Token,"lower")==0) *Action=NewCrayonAction(Crayon, ACTION_XLOWER);
	else if (strcasecmp(Token,"iconify")==0) *Action=NewCrayonAction(Crayon, ACTION_ICONIFY);
	else if (strcasecmp(Token,"deiconify")==0) *Action=NewCrayonAction(Crayon, ACTION_DEICONIFY);
	else if (strcasecmp(Token,"maximize")==0) *Action=NewCrayonAction(Crayon, ACTION_MAXIMIZE);
	else if (strcasecmp(Token,"demaximize")==0) *Action=NewCrayonAction(Crayon, ACTION_DEMAXIMIZE);
	else if (strcasecmp(Token,"wide")==0) *Action=NewCrayonAction(Crayon, ACTION_WIDE);
	else if (strcasecmp(Token,"high")==0) *Action=NewCrayonAction(Crayon, ACTION_HIGH);
	else if (strcasecmp(Token,"font")==0) ptr=ParseStringAction(ptr,ACTION_FONT,Crayon);
	else if (strcasecmp(Token,"fontup")==0) *Action=NewCrayonAction(Crayon, ACTION_FONT_UP);
	else if (strcasecmp(Token,"fontdown")==0) *Action=NewCrayonAction(Crayon, ACTION_FONT_DOWN);
	else if (strcasecmp(Token,"cls")==0) *Action=NewCrayonAction(Crayon, ACTION_CLEARSCREEN);
	else if (strcasecmp(Token,"clearscreen")==0) *Action=NewCrayonAction(Crayon, ACTION_CLEARSCREEN);

	else if (strcasecmp(Token,"restorextitle")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_RESTORE_XTITLE);
		GlobalFlags |= FLAG_RESTORE_XTITLE;
	}
	else if (strcasecmp(Token,"replace")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_REPLACE,Crayon);
	}
	else if (strcasecmp(Token,"setstr")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_SETENV,Crayon);
	}
	else if (strcasecmp(Token,"setenv")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_SETENV,Crayon);
	}
	else if (strcasecmp(Token,"send")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_SEND,Crayon);
	}
	else if (strcasecmp(Token,"bell")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_BELL,Crayon);
	}
	else if (strcasecmp(Token,"playsound")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_PLAYSOUND,Crayon);
	}
	else if (strcasecmp(Token,"exec")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_EXEC,Crayon);
	}
	else if (strcasecmp(Token,"passto")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_PASSTO,Crayon);
	}
	else if (strcasecmp(Token,"echo")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_ECHO,Crayon);
	}
	else if (strcasecmp(Token,"altscreen")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_ALTSCREEN);
	}
	else if (strcasecmp(Token,"normscreen")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_NORMSCREEN);
	}
	else if (strcasecmp(Token,"xtermbgcolor")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_XTERM_BGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"xtermfgcolor")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_XTERM_FGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"rxvtbgcolor")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_RXVT_BGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"rxvtfgcolor")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_RXVT_FGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"bgcolor")==0) 
	{
		if (strcmp(getenv("TERM"),"rxvt")==0) ptr=ParseStringAction(ptr,ACTION_RXVT_BGCOLOR,Crayon);
		else ptr=ParseStringAction(ptr,ACTION_XTERM_BGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"fgcolor")==0) 
	{
		if (strcmp(getenv("TERM"),"rxvt")==0) ptr=ParseStringAction(ptr,ACTION_RXVT_FGCOLOR,Crayon);
		else ptr=ParseStringAction(ptr,ACTION_XTERM_FGCOLOR,Crayon);
	}
	else if (strcasecmp(Token,"args")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_ARGS,Crayon);
	}
	else if (strcasecmp(Token,"edit")==0) 
	{
		ptr=ParseStringAction(ptr,ACTION_EDIT,Crayon);
	}
	else if (strcasecmp(Token,"dontcrayon")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_DONTCRAYON);
	}
	else if (strcasecmp(Token,"infobar")==0) ptr=ParseStatusBar(ACTION_INFOBAR, Crayon, ptr);
	else if (strcasecmp(Token,"querybar")==0) ptr=ParseStatusBar(ACTION_QUERYBAR, Crayon, ptr);
	else if (strcasecmp(Token,"selectbar")==0) ptr=ParseStatusBar(ACTION_SELECTBAR, Crayon, ptr);
	else if (strcasecmp(Token,"call")==0) ptr=ParseStringAction(ptr,ACTION_FUNCCALL,Crayon);


DestroyString(Token);
return(ptr);
}



// Token will either be a test (with an operation like '<' '=' etc) or a list of
// actions (colors, uppercase) to apply
char *ParseActionToken(char *Operations, TCrayon *Crayon)
{
char *Token=NULL, *ptr, *nptr;
TCrayon *Action=NULL;
int val=0;
 
//Do this before anything else!
ptr=Operations;

Action=NewCrayonAction(Crayon,0);
while (StrLen(ptr))
{		
	nptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);

		switch (*Token)
		{
			case '=':
			case '!':
			case '<':
			case '>':
				if (Action->Op != '\0') Action=NewCrayonAction(Crayon,0);
				Action->Op=*Token;
				Action->Value=atof(Token+1);
				Action->Type=CRAYON_COMPARATOR;
			break;

			default:
				nptr=ParseAttribs(ptr,Crayon,&Action);
			break;
		}
	ptr=nptr;
}

DestroyString(Token);

return(ptr);
}





char *ParseCrayonAction(TCrayon *Item, char *Args)
{
char *Token=NULL, *ptr, *ptr2;


switch (Item->Type)
{
	case CRAYON_ACTION:
	case CRAYON_LINENO:
	case CRAYON_LINE:
	case CRAYON_STRING:
	case CRAYON_APPEND:
	case CRAYON_PREPEND:
	case CRAYON_ONSTART:
	case CRAYON_ONEXIT:
	case CRAYON_VALUE:
	case CRAYON_CMDLINE:
	case CRAYON_IF:
	case CRAYON_KEYPRESS:
	case CRAYON_TIMER:
		ptr=ParseActionToken(Args, Item);
	break;

	case CRAYON_SECTION:
		Item->Start=strtol(Item->Match,&ptr2,30);
		ptr2++;
		Item->Len=strtol(ptr2,NULL,10) - Item->Start;

		ptr=GetToken(Args,"\\S",&Item->Match,GETTOKEN_QUOTES);
		ptr=ParseActionToken(ptr, Item);
	break;


	case CRAYON_MAPTO:
	case CRAYON_LINEMAPTO:
	ptr=GetToken(Args,"\\S",&Token,GETTOKEN_QUOTES);
	while (ptr)
	{
		ParseActionToken(Token, Item);
		ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
	}
	break;
}

DestroyString(Token);

return(ptr);
}



void ParseCrayonEntry(TCrayon *Crayon, char *Token, char *Args)
{
	char *ptr;
	int Type=0;
	TCrayon *CLS;

	Type=CrayonType(Token);
		
	if (Type > 0)
	{
	Crayon->Type=Type;
	switch (Type)
	{
	case CRAYON_ARGS: 
		Crayon->String=CopyStr(Crayon->String,Args); 
	break;
	
	case CRAYON_ACTION:
		ParseCrayonAction(Crayon, ptr);
	break;

	default:
		ptr=GetToken(Args,"\\S",&Crayon->Match,GETTOKEN_QUOTES);
		ParseCrayonAction(Crayon, ptr);
	break;
	}
	}

}



void ParseCrayonList(STREAM *S, TCrayon *Crayon)
{
char *Tempstr=NULL, *Token=NULL, *ptr;
ListNode *Curr;
TCrayon *SubItem;

	if (! Crayon) return;
	Tempstr=STREAMReadLine(Tempstr,S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);

		ptr=GetToken(Tempstr,"\\S",&Token,0);
		if (StrLen(Token) && (*Token != '#'))
		{
			if (strcmp(Token,"}")==0) break;
		
			if (strcmp(Token,"{")==0) ParseCrayonList(S, SubItem);
			else if (CrayonType(Token) == CRAYON_ACTION) ParseCrayonAction(Crayon, Tempstr);
			else
			{
			SubItem=NewCrayonAction(Crayon, 0);
			ParseCrayonEntry(SubItem, Token,ptr);
			}
		}

	Tempstr=STREAMReadLine(Tempstr,S);
	}

DestroyString(Tempstr);
DestroyString(Token);
}



//This parses a 'standard crayonization', which means those things that
//can occur anywhere in the config, not just at the highest level
void ParseCrayonization(char *Type, char *Config, ListNode *CrayonList)
{
	TCrayon *Crayon=NULL, *CLS=NULL, *Action=NULL;

	Crayon=(TCrayon *) calloc(1,sizeof(TCrayon));
	ParseCrayonEntry(Crayon, Type, Config);
	ListAddNamedItem(CrayonList,Crayon->Match,Crayon);

	if (Crayon->Type==CRAYON_STATUSBAR)
	{
		//Add a CLS entry
		CLS=(TCrayon *) calloc(1,sizeof(TCrayon));
		ParseCrayonEntry(CLS, "string", "[2J");
		//ParseCrayonEntry(CLS, "string", "\x1b[;H");
		Action=NewCrayonAction(Crayon,0);
		Action->ActType=ACTION_REDRAW;
		ListAddNamedItem(CrayonList,CLS->Match,CLS);
	}

}



void ConfigReadEntry(STREAM *S, ListNode *CrayonList)
{
char *Tempstr=NULL, *Token=NULL, *ptr;
TCrayon *Crayon=NULL;

	Tempstr=STREAMReadLine(Tempstr,S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);
		ptr=GetToken(Tempstr,"\\S",&Token,0);

		if (*Token=='#')  /*do nothing*/;
		else if (strcmp(Token,"}")==0) break;
		else if (CrayonList)
		{
			if (strcasecmp(Token,"passinput")==0) KeypressFlags |=  KEYPRESS_PASSINPUT;
			else if (strcasecmp(Token,"lineedit")==0) KeypressFlags |= KEYPRESS_LINEDIT;
			else if (strcasecmp(Token,"expectlines")==0) GlobalFlags |= FLAG_EXPECT_LINES;
			else if (strcasecmp(Token,"keypress")==0) Crayon=KeypressParse(ptr);
			else if (strcasecmp(Token,"selection")==0) StatusBarParseSelection(S, ptr);
			//these commands are effectively keypress without 'keypress' so pass
			//all of Tempstr
			else if (strcasecmp(Token,"focus")==0)
			{
				GlobalFlags |= HAS_FOCUS;
				Crayon=KeypressParse(Tempstr);
			}
			else if (strcasecmp(Token,"unfocus")==0) 
			{
				GlobalFlags |= HAS_FOCUS;
				Crayon=KeypressParse(Tempstr);
			}
			else if (strcasecmp(Token,"timer")==0) ParseTimer(ptr);
			else if (strcasecmp(Token,"stripansi")==0) GlobalFlags |= FLAG_STRIP_ANSI;
			else if (strcasecmp(Token,"command")==0) SetVar(Vars,"ReplaceCommand",ptr);
			else if(strcmp(Token,"{")==0) 
			{
				if (Crayon) ParseCrayonList(S,Crayon);
			}
			//Otherwise it's a 'standard' crayonization that can occur 
			//in sublists/functions etc
			else ParseCrayonization(Token, ptr, CrayonList);
		}
	Tempstr=STREAMReadLine(Tempstr,S);
	}

DestroyString(Tempstr);
DestroyString(Token);

}


int EntryMatchesCommand(char *EntryList, char *EntryArgs, char *ProgPath, char *ProgArgs)
{
char *Entry=NULL, *ptr, *p_ProgName;
int result=FALSE;

p_ProgName=strrchr(ProgPath,'/');
if (p_ProgName) p_ProgName++;
else p_ProgName=ProgPath;
ptr=GetToken(EntryList,"|",&Entry,0);
while (ptr)
{
	if (strcmp(Entry,p_ProgName)==0)	
	{
		if (StrLen(EntryArgs)==0) 
		{
			result=TRUE;
			break;
		}

		if (pmatch(EntryArgs,ProgArgs,StrLen(ProgArgs),NULL,0) > 0) 
		{
			result=TRUE;
			break;
		}
	}
	ptr=GetToken(ptr,"|",&Entry,0);
}

DestroyString(Entry);
return(result);
}



void ParseFunction(STREAM *S, const char *Config)
{
char *Token=NULL, *Tempstr=NULL, *ptr;
ListNode *Select;
TCrayon *Crayon;

if (! Functions) Functions=ListCreate();

ptr=GetToken(Config, "\\S", &Token, GETTOKEN_QUOTES);
Select=ListCreate();

ListAddNamedItem(Functions, Token, Select);

Tempstr=STREAMReadLine(Tempstr, S);
while (Tempstr)
{
	StripTrailingWhitespace(Tempstr);
	StripLeadingWhitespace(Tempstr);
	ptr=GetToken(Tempstr, "\\S", &Token, GETTOKEN_QUOTES);
	if (strcmp(Token,"}")==0) break;
	
	ParseCrayonization(Token, ptr, Select);
	Tempstr=STREAMReadLine(Tempstr, S);
}

DestroyString(Tempstr);
DestroyString(Token);
}




int ConfigReadFile(const char *Path, const char *CommandLine, char **CrayonizerDir, ListNode *CrayonList)
{
STREAM *S;
char *Tempstr=NULL, *Token=NULL, *ptr;
char *ProgName=NULL, *Args;


S=STREAMOpenFile(Path,SF_RDONLY);
if (! S) return(FALSE);

Args=GetToken(CommandLine," ",&ProgName,0);
Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr)
{
	StripTrailingWhitespace(Tempstr);
	StripLeadingWhitespace(Tempstr);
	ptr=GetToken(Tempstr,"\\S",&Token,0);
	if (strcasecmp(Token,"CrayonizerDir")==0) *CrayonizerDir=CopyStr(*CrayonizerDir,ptr);
	else if (strcasecmp(Token,"entry")==0)
	{
		ptr=GetToken(ptr,"\\S",&Token,0);
		if (EntryMatchesCommand(Token,ptr,ProgName, Args))
		{
			ConfigReadEntry(S, CrayonList);
		}
		else ConfigReadEntry(S, NULL);
	}
	else if (strcasecmp(Token,"function")==0) ParseFunction(S, ptr);
	else if (strcasecmp(Token,"include")==0) ConfigReadFile(ptr, CommandLine, CrayonizerDir, CrayonList);
	else if (strcasecmp(Token,"selection")==0) StatusBarParseSelection(S, ptr);

	Tempstr=STREAMReadLine(Tempstr,S);
}

DestroyString(ProgName);
DestroyString(Tempstr);
DestroyString(Token);

return(TRUE);
}


int ConfigLoad(const char *CmdLine, char **CrayonizerDir, ListNode *CrayonList)
{
char *Paths[]={"$(UserCrayonizerDir)/$(Command).conf","$(UserDir)/.crayonizer.conf","$(SystemCrayonizerDir)/$(Command).conf","$(SystemConfigDir)/crayonizer.conf",NULL};
int RetVal=FALSE;

char *Tempstr=NULL, *UserDir=NULL;
ListNode *Vars;
int i;


Vars=ListCreate();
SetVar(Vars,"SystemConfigDir","/etc");
SetVar(Vars,"SystemCrayonizerDir","/etc/crayonizer");
GetToken(CmdLine," ",&Tempstr,0);
SetVar(Vars,"Command",Tempstr);
UserDir=CopyStr(UserDir,GetCurrUserHomeDir());
SetVar(Vars,"UserDir",UserDir);
Tempstr=MCopyStr(Tempstr,UserDir,"/.crayonizer",NULL);
SetVar(Vars,"UserCrayonizerDir",Tempstr);


for (i=0; Paths[i] !=NULL; i++)
{
	Tempstr=SubstituteVarsInString(Tempstr,Paths[i],Vars,0);
	if (ConfigReadFile(Tempstr, CmdLine, CrayonizerDir, CrayonList))
	{
		RetVal=TRUE;
		break;
	}
}


if (! RetVal)
{
	printf("ERROR! Crayonizer can't find config file.\n");
	exit(1);
}

DestroyString(Tempstr);
DestroyString(UserDir);
return(RetVal);
}

