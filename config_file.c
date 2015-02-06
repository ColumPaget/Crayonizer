#include "config_file.h"

char *Colors[]={"none","black","red","green","yellow","blue","magenta","cyan","white","none","none","darkgrey","lightred","lightgreen","lightyellow","lightblue","lightmagenta","lightcyan",NULL};
char *CrayonTypes[]={"action","args","line","string","section","lineno","value","mapto","linemapto","append","prepend","exec","cmdline","passinput","include","keypress","if","statusbar",NULL};


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
		ptr++;
		val=MatchTokenFromList(ptr,Colors,0);
		if (val > 0) Attrib=val << 8;
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

Act->ActType=Type;
Crayon->ActionCount++;

return(Act);
}


//This parses those actions that take a string
int ParseStringAction(char *Line, int Type, TCrayon *Crayon)
{
	TCrayon *Action;
	char *Token=NULL, *ptr;

		ptr=GetToken(Line," ",&Token,GETTOKEN_QUOTES);

		Action=NewCrayonAction(Crayon,Type);
		Action->String=CopyStr(Action->String,Token);


		DestroyString(Token);

return(ptr);
}



char *ParseAttribs(char *Verbs, TCrayon *Crayon, TCrayon **Action)
{
char *Token=NULL, *ptr;
int val=0;
 
//Do this before anything else!

	ptr=GetToken(Verbs," ",&Token,0);

	if (strcasecmp(Token,"caps")==0) (*Action)->Attribs |= FLAG_CAPS;
	else if (strcasecmp(Token,"bold")==0) (*Action)->Attribs |= FLAG_BOLD;
	else if (strcasecmp(Token,"hide")==0) (*Action)->Attribs |= FLAG_HIDE;
	else if (strcasecmp(Token,"blink")==0) (*Action)->Attribs |= FLAG_BLINK;
	else if (strcasecmp(Token,"uppercase")==0) (*Action)->Attribs |= FLAG_CAPS;
	else if (strcasecmp(Token,"underline")==0) (*Action)->Attribs |= FLAG_UNDER;
	else if (strcasecmp(Token,"lowercase")==0) (*Action)->Attribs |= FLAG_LOWERCASE;
	else if (strcasecmp(Token,"inverse")==0) (*Action)->Attribs |= FLAG_INVERSE;
	else if (strcasecmp(Token,"clrtoeol")==0) (*Action)->Attribs |= FLAG_CLR2EOL;
	else if (strcasecmp(Token,"basename")==0) *Action=NewCrayonAction(Crayon, ACTION_BASENAME);
	else if (strcasecmp(Token,"setxtitle")==0) *Action=NewCrayonAction(Crayon, ACTION_SET_XTITLE);
	else if (strcasecmp(Token,"cls")==0) *Action=NewCrayonAction(Crayon, ACTION_CLEARSCREEN);
	else if (strcasecmp(Token,"clearscreen")==0) *Action=NewCrayonAction(Crayon, ACTION_CLEARSCREEN);
	else if (strcasecmp(Token,"restorextitle")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_RESTORE_XTITLE);
		GlobalFlags |= FLAG_RESTORE_XTITLE;
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
	else if (strcasecmp(Token,"dontcrayon")==0) 
	{
		*Action=NewCrayonAction(Crayon, ACTION_DONTCRAYON);
	}

	else (*Action)->Attribs |= ParseColor(Token);



DestroyString(Token);
return(ptr);
}



// Token will either be a test (with an operation like '<' '=' etc) or a list of
// actions (colors, uppercase) to apply
void ParseActionToken(char *Operations, TCrayon *Crayon)
{
char *Token=NULL, *ptr, *nptr;
TCrayon *Action=NULL;
int val=0;
 
//Do this before anything else!

ptr=Operations;
Action=NewCrayonAction(Crayon,0);
while (StrLen(ptr))
{		
	nptr=GetToken(ptr," ",&Token,0);
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
}





void ParseCrayonAction(TCrayon *Item, char *Args)
{
char *Token=NULL, *ptr, *ptr2;


switch (Item->Type)
{
	case CRAYON_LINENO:
	case CRAYON_LINE:
	case CRAYON_STRING:
	case CRAYON_APPEND:
	case CRAYON_PREPEND:
	case CRAYON_VALUE:
	case CRAYON_EXEC:
	case CRAYON_CMDLINE:
	case CRAYON_IF:
		ParseActionToken(Args, Item);
	break;

	case CRAYON_SECTION:
		Item->Start=strtol(Item->Match,&ptr2,10);
		ptr2++;
		Item->Len=strtol(ptr2,NULL,10) - Item->Start;

		ptr=GetToken(Args,"\\S",&Item->Match,GETTOKEN_QUOTES);
		ParseActionToken(ptr, Item);
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

return(Item);
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
	case CRAYON_STATUSBAR:
		GlobalFlags |= HAS_STATUSBAR;
		Crayon->String=CopyStr(Crayon->String,Args); 
	break;

	case CRAYON_ARGS: 
		Crayon->String=CopyStr(Crayon->String,Args); 
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

	if (! Crayon) return(NULL);
	Tempstr=STREAMReadLine(Tempstr,S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);
		ptr=GetToken(Tempstr,"\\S",&Token,0);
		if (strcmp(Token,"}")==0) break;
		
		if (strcmp(Token,"{")==0) ParseCrayonList(S, SubItem);
		else SubItem=NewCrayonAction(Crayon, 0);
		ParseCrayonEntry(SubItem, Token,ptr);
		
	Tempstr=STREAMReadLine(Tempstr,S);
	}

DestroyString(Tempstr);
DestroyString(Token);
}


void ParseKeypress(char *Name)
{
TCrayon *KP;
char *ptr;


if ((NoOfKeyPresses % 10)==0) 
{
	KeyPresses=(TCrayon *) realloc((void *) KeyPresses, (NoOfKeyPresses+10) * sizeof(TCrayon));
}

KP=KeyPresses + NoOfKeyPresses;
memset(KP,0,sizeof(TCrayon));
KP->Match=SetStrLen(NULL,2);
ptr=Name;

if 
	(
		(strncasecmp(ptr,"ctrl",4)==0)
)
{
	ptr+=4;
	if (ispunct(*ptr)) ptr++;
	KP->Match[0]=toupper(*ptr)-64;
}
else KP->Match[0]=*ptr;

ptr++;

ParseActionToken(ptr,KP);

NoOfKeyPresses++;
}



void ConfigReadEntry(STREAM *S, char **CommandLine, ListNode *ColorMatches)
{
char *Tempstr=NULL, *Token=NULL, *ptr;
TCrayon *Crayon=NULL, *CLS, *Action;
ListNode *Curr;

	Tempstr=STREAMReadLine(Tempstr,S);
	while (Tempstr)
	{
		StripTrailingWhitespace(Tempstr);
		StripLeadingWhitespace(Tempstr);
		ptr=GetToken(Tempstr,"\\S",&Token,0);

		if (strcmp(Token,"}")==0) break;
		else if (ColorMatches)
		{
			if (strcasecmp(Token,"passinput")==0) GlobalFlags |= FLAG_PASSINPUT;
			else if (strcasecmp(Token,"stripansi")==0) GlobalFlags |= FLAG_STRIP_ANSI;
			else if (strcasecmp(Token,"expectlines")==0) GlobalFlags |= FLAG_EXPECT_LINES;
			else if (strcasecmp(Token,"keypress")==0) ParseKeypress(ptr);
			else if (strcasecmp(Token,"command")==0) *CommandLine=CopyStr(*CommandLine,ptr);
			else if (Crayon && (strcmp(Token,"{")==0)) ParseCrayonList(S,Crayon);
			else
			{
		 		Crayon=(TCrayon *) calloc(1,sizeof(TCrayon));
				ParseCrayonEntry(Crayon,Token,ptr);
				ListAddNamedItem(ColorMatches,Crayon->Match,Crayon);

				if (Crayon->Type==CRAYON_STATUSBAR)
				{
					//Add a CLS entry
					CLS=(TCrayon *) calloc(1,sizeof(TCrayon));
					ParseCrayonEntry(CLS, "string", "[2J");
					//ParseCrayonEntry(CLS, "string", "\x1b[;H");
					Action=NewCrayonAction(Crayon,0);
					Action->ActType=ACTION_REDRAW;
					ListAddNamedItem(ColorMatches,CLS->Match,CLS);
				}

			}
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



int ConfigReadFile(char *Path, char **CommandLine, char **CrayonizerDir, ListNode *ColorMatches)
{
STREAM *S;
char *Tempstr=NULL, *Token=NULL, *ptr;
char *ProgName=NULL, *Args;


S=STREAMOpenFile(Path,O_RDONLY);
if (! S) return(FALSE);

Args=GetToken(*CommandLine," ",&ProgName,0);
Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr)
{
	StripTrailingWhitespace(Tempstr);
	StripLeadingWhitespace(Tempstr);
	ptr=GetToken(Tempstr,"\\S",&Token,0);
	if (strcasecmp(Token,"CrayonizerDir")==0) *CrayonizerDir=CopyStr(*CrayonizerDir,ptr);
	if (strcasecmp(Token,"entry")==0)
	{
		ptr=GetToken(ptr,"\\S",&Token,0);
		if (EntryMatchesCommand(Token,ptr,ProgName, Args))
		{
			ConfigReadEntry(S, CommandLine,ColorMatches);
		}
		else ConfigReadEntry(S,NULL,NULL);

	}
	if (strcasecmp(Token,"include")==0) ConfigReadFile(ptr, CommandLine, CrayonizerDir, ColorMatches);

	Tempstr=STREAMReadLine(Tempstr,S);
}

DestroyString(ProgName);
DestroyString(Tempstr);
DestroyString(Token);

return(TRUE);
}



