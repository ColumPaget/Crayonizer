#include "config_file.h"
#include "keypress.h"
#include "status_bar.h"
#include "timers.h"

const char *Colors[]={"none","black","red","green","yellow","blue","magenta","cyan","white","none","none","darkgrey","lightred","lightgreen","lightyellow","lightblue","lightmagenta","lightcyan","lightgrey",NULL};
const char *CrayonTypes[]={"action","args","line","string","section","lineno","value","mapto","linemapto","append","prepend","onstart","onexit","cmdline","passinput","include","keypress","if","statusbar","timer","edit",NULL};

//only used by 'IsAttrib'
#define FLAG_COLOR 1



char *ParseAttribs(char *Verbs, TCrayon *Crayon, TCrayon **Action);



//colors can be either a color name or a forecolor/backcolor pair
int ParseColor(const char *ColorString)
{
const char *ptr;
int val=0, Attrib=0;
char *Token=NULL;

	ptr=strchr(ColorString,'/');
	if (ptr)
	{
		Token=CopyStrLen(Token,ColorString,ptr-ColorString);
		val=MatchTokenFromList(ptr+1,Colors,0);
		if (val > 0) Attrib=val << 8;
	}
	else Token=CopyStr(Token, ColorString);

	val=MatchTokenFromList(Token,Colors,0);
	if (val > -1) Attrib |= val;

	DestroyString(Token);
return(Attrib);
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




int IsFlag(const char *String)
{
const char *AttribStrings[]={"top","bottom","persist","transient","refresh",NULL};
int AttribFlags[]={FLAG_TOP, FLAG_BOTTOM, FLAG_PERSIST, FLAG_TRANSIENT, FLAG_REFRESH};
int val;

val=MatchTokenFromList(String,AttribStrings,0);
if (val > -1) return(AttribFlags[val]);

return(0);
}


int MatchActionType(const char *Str)
{
const char *ActStrings[]={"", "args", "echo", "replace","setstr","setenv","send","bell","playsound","exec","passto","passinput","basename","setxtitle","restorextitle","setxlabel","xselection","setxiconname","raise","lower","iconify","deiconify","maximize","demaximize","wide","high","font","fontup","fontdown","cls","clearscreen","cleartoeol","altscreen","normscreen","xtermbgcolor","xtermfgcolor","rxvtbgcolor","rxvtfgcolor","bgcolor","fgcolor","dontcrayon","allowchildcrayon","redraw","infobar","querybar","selectbar","call",NULL};

return(MatchTokenFromList(Str,ActStrings,0));
}




int CrayonType(char *String)
{
int val;

	if (! StrValid(String)) return(-1);
	if (*String=='#') return(-1);
	val=MatchTokenFromList(String,CrayonTypes,0);
	if (val==-1) val=0;
	return(val);
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
	else
	{
	val=MatchActionType(Token);
	switch (val)
	{
		case ACTION_CLRTOEOL: (*Action)->Attribs |= FLAG_CLR2EOL; break;
		case ACTION_RESTORE_XTITLE: 
		*Action=NewCrayonAction(Crayon, ACTION_RESTORE_XTITLE);
		GlobalFlags |= FLAG_RESTORE_XTITLE;
		break;

		case ACTION_REPLACE:
		case ACTION_SETSTR:
		case ACTION_SETENV:
		case ACTION_SEND:
		case ACTION_PLAYSOUND:
		case ACTION_EXEC:
		case ACTION_PASSTO:
		case ACTION_ECHO:
		case ACTION_FONT:
		case ACTION_ARGS:
		case ACTION_FUNCCALL:
		case ACTION_XTERM_BGCOLOR:
		case ACTION_XTERM_FGCOLOR:
		case ACTION_RXVT_BGCOLOR:
		case ACTION_RXVT_FGCOLOR:
			ptr=ParseStringAction(ptr,val,Crayon);
		break;

		case ACTION_BGCOLOR:
		if (strcmp(getenv("TERM"),"rxvt")==0) ptr=ParseStringAction(ptr,ACTION_RXVT_BGCOLOR,Crayon);
		else ptr=ParseStringAction(ptr,ACTION_XTERM_BGCOLOR,Crayon);
		break;

		case ACTION_FGCOLOR:
		if (strcmp(getenv("TERM"),"rxvt")==0) ptr=ParseStringAction(ptr,ACTION_RXVT_FGCOLOR,Crayon);
		else ptr=ParseStringAction(ptr,ACTION_XTERM_FGCOLOR,Crayon);
		break;

	case ACTION_INFOBAR:
	case ACTION_QUERYBAR:
	case ACTION_SELECTBAR:
		ptr=ParseStatusBar(val, Crayon, ptr);
	break;

	default: (*Action)=NewCrayonAction(Crayon, val); break;
	}
	}

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
				if (StrValid(Action->Op)) Action=NewCrayonAction(Crayon,0);
				Action->Op=CopyStrLen(Action->Op, Token, 1);
				Action->Value=atof(Token+1);
				Action->String=CopyStr(Action->String, Token+1);
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
		Item->Start=strtol(Item->Match,&ptr2,10);
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

	default:
		if (MatchActionType(Token) >0) 
		{
			Item->Type=CRAYON_ACTION;
			ParseCrayonAction(Item, Args);
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
	
	if (Type > -1)
	{
	Crayon->Type=Type;
	switch (Type)
	{
	case CRAYON_ARGS: 
		Crayon->String=CopyStr(Crayon->String,Args); 
	break;
	
	case CRAYON_ACTION:
		ParseCrayonAction(Crayon, Args);
	break;

	default:
		if (MatchActionType(Token) >0) 
		{
			Crayon->Type=CRAYON_ACTION;
			ParseCrayonAction(Crayon, Args);
		}
		else
		{
		ptr=GetToken(Args,"\\S",&Crayon->Match,GETTOKEN_QUOTES);
		ParseCrayonAction(Crayon, ptr);
		}
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
TCrayon *ParseCrayonization(const char *Type, const char *Config, ListNode *CrayonList)
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

	return(Crayon);
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
			else if (strcasecmp(Token,"allowchildcrayon")==0) GlobalFlags |= FLAG_CHILDCRAYON;
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
			else if (strcasecmp(Token,"pty")==0) GlobalFlags |= FLAG_USE_PTY;
			else if (strcasecmp(Token,"stripansi")==0) GlobalFlags |= FLAG_STRIP_ANSI;
			else if (strcasecmp(Token,"stripxtitle")==0) GlobalFlags |= FLAG_STRIP_XTITLE;
			else if (strcasecmp(Token,"command")==0) SetVar(Vars,"ReplaceCommand",ptr);
			else if(strcmp(Token,"{")==0) 
			{
				if (Crayon) ParseCrayonList(S,Crayon);
			}
			//Otherwise it's a 'standard' crayonization that can occur 
			//in sublists/functions etc
			else Crayon=ParseCrayonization(Token, ptr, CrayonList);
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
	
	if (MatchActionType(Token) > -1)
	{
		 ParseCrayonization("action", Tempstr, Select);
	}
	else ParseCrayonization(Token, ptr, Select);
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

Args=GetToken(CommandLine," ",&ProgName,GETTOKEN_QUOTES);
Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr)
{
	StripTrailingWhitespace(Tempstr);
	StripLeadingWhitespace(Tempstr);
	ptr=GetToken(Tempstr,"\\S",&Token,0);
	if (strcasecmp(Token,"CrayonizerDir")==0) *CrayonizerDir=CopyStr(*CrayonizerDir,ptr);
	else if (strcasecmp(Token,"allowchildcrayon")==0) GlobalFlags |= FLAG_CHILDCRAYON;
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
SetVar(Vars,"SystemCrayonizerDir","/etc/crayonizer.d");
GetToken(CmdLine," ",&Tempstr,0);
SetVar(Vars,"Command",Tempstr);
UserDir=CopyStr(UserDir,GetCurrUserHomeDir());
SetVar(Vars,"UserDir",UserDir);
Tempstr=MCopyStr(Tempstr,UserDir,"/.crayonizer.d",NULL);
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

