#include "xterm.h"
#include "status_bar.h"
#include "history.h"
#include "crayonizations.h"
#include "text_substitutions.h"

static void OutputLineWithAttributes(const char *Line, int *Attribs, int Len);

static int HandleComparison(const char *Op, const char *Item, const char *Comparator)
{
char *ptr;
int result=FALSE;

if (! StrValid(Op)) return(FALSE);
if (! StrValid(Item)) return(FALSE);
if (! StrValid(Comparator)) return(FALSE);
switch (*Op)
{
			case '<':
				if (*(Op+1) == '=') result=(atof(Item) <= atof(Comparator));
				else result=(atof(Item) < atof(Comparator));
			break;

			case '>':
				if (*(Op+1) == '=') result=(atof(Item) >= atof(Comparator));
				else result=(atof(Item) > atof(Comparator));
			break;

			case '=':
				result=pmatch(Comparator, Item, StrLen(Item), NULL, 0);
				if (result > 0) result=TRUE;
				else result=FALSE;
			break;

			case '!':
				result=pmatch(Comparator, Item, StrLen(Item), NULL, 0);
				if (result > 0) result=FALSE;
				else result=TRUE;
			break;

			case '%':
				if (atoi(Item) % atoi(Comparator) ==0) result=TRUE;
			break;
}


return(result);
}


static int IsInStringList(char *Item, char *List, char *Separator) 
{
char *Token=NULL;
const char *ptr;
int result=FALSE;

ptr=GetToken(List,Separator,&Token,GETTOKEN_QUOTES);
while (ptr)
{
	StripLeadingWhitespace(Token);
	StripTrailingWhitespace(Token);
	if (pmatch(Token, Item, StrLen(Item), NULL, 0) > 0)
	{
		result=TRUE;
		break;
	}
ptr=GetToken(ptr,Separator,&Token,GETTOKEN_QUOTES);
}

Destroy(Token);

return(result);
}



static int HandleCrayonIf(TCrayon *Crayon)
{
char *Expr=NULL, *Tempstr=NULL, *Token=NULL, *PrevToken=NULL, *Value=NULL;
const char *ptr, *tptr, *aptr;
int result=FALSE, val, i;

	Expr=SubstituteVarsInString(Expr,Crayon->Match,Vars,0);


	ptr=GetToken(Expr,"\\S",&Token,GETTOKEN_QUOTES|GETTOKEN_MULTI_SEPARATORS|GETTOKEN_INCLUDE_SEPARATORS);
	while (ptr)
	{
		switch (*Token)
		{
			case 'a':
				result=FALSE;
				if (strncmp(Token,"arg(",4)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+4);
					StrRTruncChar(Tempstr, ')');
					val=StrLen(Tempstr);
					for (i=1; i < cmdline_argc; i++)
					{
						aptr=cmdline_argv[i];
						if (*aptr=='-') aptr++;
						for ( ; *aptr != '\0'; aptr++)
						{
							for (tptr=Tempstr; *tptr != '\0'; tptr++)
							{
							if (*aptr	 == *tptr) result=TRUE;
							}
						}
					if (result) break;
					}
				}
			break;

			case 'f':
				result=FALSE;
				if (strncmp(Token,"focus",5)==0) result=GlobalFlags & FLAG_FOCUSED;
			break;

			case 'l':
				result=FALSE;
				if (strncmp(Token,"larg(",5)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+5);
					StrRTruncChar(Tempstr, ')');
					for (i=1; i < cmdline_argc; i++)
					{
						tptr=cmdline_argv[i];
						while (*tptr=='-') tptr++;

						if (strcmp(tptr,Tempstr)==0) 
						{
							result=TRUE;
							break;
						}
					}
				}
			break;

			case 'e':
				if (strncmp(Token,"exists(",7)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+7);
					StrRTruncChar(Tempstr, ')');

					if (access(Tempstr,F_OK)==0) result=TRUE;
					else result=FALSE;
				}
			break;

			case 'i':
				if (strcmp(Token,"in")==0)
				{
					ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
					if (IsInStringList(PrevToken,Token,",")) result=TRUE;
				}
				else if (strncmp(Token,"isatty(",7)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+7);
					StrRTruncChar(Tempstr, ')');

					if (strcmp(Tempstr,"stdin")==0) val=0;
					else val=1;
					result=isatty(val);
				}
			break;

			case 'n':
				if (strncmp(Token,"notatty(",8)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+8);
					StrRTruncChar(Tempstr, ')');
					if (strcmp(Tempstr,"stdin")==0) val=0;
					else val=1;
					result= !isatty(val);
				}
			break;

			case '<':
			case '>':
			case '=':
			case '!':
			case '%':
				//we don't want to get a blank for the next value, so forward past any spaces
				while(isspace(*ptr)) ptr++;
				ptr=GetToken(ptr,"\\S",&Value,GETTOKEN_QUOTES);
				result=HandleComparison(Token, PrevToken, Value);
			break;
			
			case ' ':
				StripTrailingWhitespace(Token);
			break;
		}

		if (StrValid(Token)) PrevToken=CopyStr(PrevToken,Token);

		ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES|GETTOKEN_MULTI_SEPARATORS|GETTOKEN_INCLUDE_SEPARATORS);
	}


Destroy(PrevToken);
Destroy(Tempstr);
Destroy(Token);
Destroy(Expr);
return(result);
}




static int CrayonMatches(TCrayon *Crayon, const char *sptr, const char *eptr)
{
char *ptr;
int i, result=FALSE, pos1, pos2;
float val;


switch (Crayon->Type)
{
//These always 'match', as they are not checks on the value of anything

case CRAYON_ACTION:
case CRAYON_ARGS:
case CRAYON_LINE:
case CRAYON_STRING:
case CRAYON_SECTION:
case CRAYON_MAPTO:
case CRAYON_LINEMAPTO:
case CRAYON_APPEND: 
case CRAYON_PREPEND: 
case CRAYON_ONSTART: 
case CRAYON_ONEXIT: 
case CRAYON_PASSINPUT: 
case CRAYON_CMDLINE: 
case CRAYON_VALUE: 
	return(TRUE);
break;


case CRAYON_COMPARATOR:
	if (! sptr) return(FALSE);
	result=HandleComparison(Crayon->Op, sptr, Crayon->String);
break;

case CRAYON_LINENO:
	ptr=Crayon->Match;

	switch (*ptr)
	{
		case '*':
		result=TRUE;
		break;

		case '>':
		ptr++;
		pos1=strtol(ptr,NULL,10);
		if (LineNo > pos1) result=TRUE;
		break;

		case '<':
		ptr++;
		pos1=strtol(ptr,NULL,10);
		if (LineNo < pos1) result=TRUE;
		break;

		case '%':
		ptr++;
		pos1=strtol(ptr,NULL,10);
		if ((LineNo % pos1) ==0) result=TRUE;
		break;


		default:
			pos1=strtol(ptr,&ptr,10);
			if (*ptr=='-') 
			{
				ptr++;
				pos2=strtol(ptr,NULL,10);
				if ((LineNo >= pos1) && (LineNo <=pos2)) result=TRUE;
			}
			else if (pos1==LineNo) result=TRUE;
		break;
	}

break;


case CRAYON_IF:
	result=HandleCrayonIf(Crayon);
break;

}

return(result);
}


static void PassToProgram(STREAM *Pipe, const char *ActivateLine, const char *Program)
{
STREAM *S, *CmdS, *CmdErr;
char *Tempstr=NULL;
int result, wrote;
ListNode *Streams;

Streams=ListCreate();
CmdS=STREAMCreate();
CmdErr=STREAMCreate();
PipeSpawn( &(CmdS->out_fd), &(CmdS->in_fd), &(CmdErr->in_fd), Program, "");

ListAddItem(Streams,Pipe);
ListAddItem(Streams,CmdS);
ListAddItem(Streams,CmdErr);
Tempstr=SetStrLen(Tempstr,4096);
STREAMFlush(Pipe);

if (StrLen(ActivateLine)) write(CmdS->out_fd,ActivateLine,StrLen(ActivateLine));
//write(1,"\n",1);

while (1)
{
	S=STREAMSelect(Streams,NULL);
	
	if (S)
	{
		if (S==CmdS) 
		{
			result=STREAMReadBytes(CmdS,Tempstr,4096);
			if (result < 1) break;
			write(Pipe->out_fd,Tempstr,result);
		}

		if (S==CmdErr) 
		{
			result=STREAMReadBytes(CmdErr,Tempstr,4096);
			if (result < 1) break;
			fwrite(Tempstr,1, result, stdout);
		}

		if (S==Pipe)
		{
			result=STREAMReadBytes(Pipe,Tempstr,4096);
			wrote=write(CmdS->out_fd,Tempstr,result);
			if (wrote < result) break;
		}
	}
}

STREAMClose(S);
Destroy(Tempstr);
}




static void ApplySingleAction(STREAM *Pipe, int *AttribLine, const char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Action)
{
int Attribs;
int start, end, i;
char *Tempstr=NULL, *EnvName=NULL, *Value=NULL;
char *ptr;
TCrayon *StatusBar=NULL;

	if (GlobalFlags & FLAG_DONTCRAYON) return;

	StatusBar=StatusBarGetActive();

	Attribs=Action->Attribs;
	start=MatchStart-Line;
	end=MatchEnd-Line;
	if (end > Len) end=Len;

		//These have to be done when we have the particular match in hand, they 
		//can't be done when we output the line at the end


		switch (Action->ActType)
		{
		case ACTION_BASENAME:
			ptr=strrchr(MatchStart,'/');
			if (ptr && (ptr < MatchEnd))
			{
				MatchStart=ptr+1;
				start=MatchStart-Line;
			}
		break;


		case ACTION_REPLACE:
			strncpy(MatchStart,Action->String,MatchEnd-MatchStart);
		break;

		case ACTION_SETENV:
		case ACTION_SETSTR:
			ptr=strchr(Action->String,'=');
			if (ptr) 
			{
				EnvName=CopyStrLen(EnvName,Action->String,ptr-Action->String);
				Tempstr=CopyStr(Tempstr,ptr+1);
			}
			else
			{
				EnvName=CopyStr(EnvName,Action->String);
				//end points to last character in match, not character after it
				//so we must +1 to len
				Tempstr=CopyStrLen(Tempstr,MatchStart,(end-start)+1);
			}
			StripTrailingWhitespace(Tempstr);

			Value=SubstituteTextValues(Value, Tempstr, StrLen(Tempstr));
			setenv(EnvName,Value,TRUE);
			SetVar(Vars,EnvName,Value);
		break;

		case ACTION_ARGS:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			SetVar(Vars,"ExtraCmdLineOptions",Tempstr);
		break;

		/*
		case ACTION_PLAYSOUND:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			SoundPlayFile("/dev/dsp",Tempstr,VOLUME_LEAVEALONE, PLAYSOUND_NONBLOCK);
		break;
		*/

		case ACTION_EXEC:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			PassToProgram(StdIn, NULL, Tempstr);
		break;

		case ACTION_PASSTO:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			PassToProgram(Pipe, Line, Tempstr);
		break;

		case ACTION_SEND:
			if (StrValid(Action->String)) Tempstr=SubstituteVarsInString(Tempstr, Action->String, Vars, 0);
			else Tempstr=SubstituteVarsInString(Tempstr, Line, Vars, 0);
			STREAMWriteLine(Tempstr,Pipe); STREAMFlush(Pipe);
			usleep(25000);
			//fprintf(stderr,"SEND '%s'\n",Tempstr);
		break;

		case ACTION_ECHO:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			fwrite(Tempstr,1, StrLen(Tempstr), stdout);
		break;

		case ACTION_ALTSCREEN:
			fputs("\x1b[?47h",stdout); 
		break;

		case ACTION_NORMSCREEN:
			fputs("\x1b[?47l",stdout); 
		break;
		
		case ACTION_BELL:
			fputs("\x07",stdout);
		break;

		case ACTION_XTERM_FGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]10;",EnvName,"\007",NULL);
			fputs(Tempstr, stdout);
		break;

		case ACTION_XTERM_BGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]11;",EnvName,"\007",NULL);
			fputs(Tempstr, stdout);
		break;

		case ACTION_RXVT_FGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]39;",EnvName,"\007",NULL);
			fputs(Tempstr, stdout);
		break;

		case ACTION_RXVT_BGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]49;",EnvName,"\007",NULL);
			fputs(Tempstr, stdout);
		break;

		case ACTION_SET_XTITLE: 
			EnvName=SubstituteTextValues(EnvName, MatchStart, end-start);
			StripTrailingWhitespace(EnvName);
			if (! StrValid(EnvName))
			{
			ptr=GetVar(Vars,"crayon_default_xtitle");
			if (StrValid(ptr)) EnvName=SubstituteTextValues(EnvName, ptr, StrLen(ptr));
			}
			Tempstr=MCopyStr(Tempstr,"\x1b]2;",EnvName,"\x07", NULL);
			fputs(Tempstr, stdout);
		break;


		case ACTION_RESTORE_XTITLE:
			ptr=GetVar(Vars,"crayon_old_xtitle");
			if (! StrValid(ptr)) ptr=GetVar(Vars,"crayon_default_xtitle");
			Tempstr=MCopyStr(Tempstr,"\x1b]2;", ptr, "\x07", NULL);
			fputs(Tempstr, stdout);
		break;


		case ACTION_SET_XICONNAME: 
			Tempstr=CopyStr(Tempstr,"\x1b]1;");
			Tempstr=CatStrLen(Tempstr,MatchStart,end-start);
			StripTrailingWhitespace(Tempstr);
			Tempstr=CatStr(Tempstr,"\x07");
			fputs(Tempstr, stdout);
		break;

		case ACTION_XSELECTION: 
			Tempstr=CopyStr(Tempstr,"\x1b]52;cp;");
			EnvName=EncodeBytes(EnvName,MatchStart,end-start,ENCODE_BASE64);
			Tempstr=CatStr(Tempstr,EnvName);
			StripTrailingWhitespace(Tempstr);
			Tempstr=CatStr(Tempstr,"\x07");
			fputs(Tempstr, stdout);
		break;


		case ACTION_DEICONIFY: 
			fputs("\x1b[1t",stdout);
		break;

		case ACTION_ICONIFY: 
			fputs("\x1b[2t", stdout);
		break;

		case ACTION_DEMAXIMIZE: 
			fputs("\x1b[9;0t", stdout);
		break;

		case ACTION_MAXIMIZE: 
			fputs("\x1b[9;1t", stdout);
		break;

		case ACTION_HIGH: 
			fputs("\x1b[9;2t", stdout);
		break;

		case ACTION_WIDE: 
			fputs("\x1b[9;3t", stdout);
		break;

		case ACTION_XRAISE: 
			fputs("\x1b[5t", stdout);
		break;

		case ACTION_XLOWER: 
			fputs("\x1b[6t", stdout);
		break;

		case ACTION_FONT_UP:
			fputs("\x1b[?35h\x1b]50;#+1\x07", stdout);
		break;

		case ACTION_FONT_DOWN:
			fputs("\x1b[?35h\x1b]50;#-1\x07", stdout);
		break;

		case ACTION_FONT:
			Tempstr=MCopyStr(Tempstr,"\x1b[?35h\x1b]50;",Action->String,"\x07", NULL);
			fputs(Tempstr, stdout);
		break;

		case ACTION_CLEARSCREEN:
			fputs(CLRSCR, stderr);
		break;

		case ACTION_FUNCCALL:
			FunctionCall(Pipe, Action->String, Line, Len);
		break;

	case ACTION_INFOBAR:
		if (! StatusBar) InfoBar(Action);
		else if (StatusBar==Action) StatusBarCloseActive();
	break;
	
	case ACTION_QUERYBAR:
		if (! StatusBar) QueryBar(Action);
		else if (StatusBar==Action) StatusBarCloseActive();
	break;
	
	case ACTION_SELECTBAR:
		if (! StatusBar) SelectionBar(Action);
		else if (StatusBar==Action) StatusBarCloseActive();
	break;

	case ACTION_HISTORYBAR:
		if (! StatusBar) TypingHistoryPopup(Action);
		else if (StatusBar==Action) StatusBarCloseActive();
	break;

	case ACTION_EDIT:
		StatusBarHandleInput(Pipe, NULL, Action->String);
	break;

		case ACTION_DONTCRAYON:
			//use '=' not '|=' here because we don't want it to honor
			//any other flags
			GlobalFlags = FLAG_DONTCRAYON;
		break;

		}


	if (AttribLine) 
	{
		for (i=start; i < end; i++) 
		{
			//if there is a foreground color,then mask out any previous foreground values set, 
			//othewise they will combine to produce an unexpected color
			if (Attribs & 0x00FF) AttribLine[i] &= 0xFFFFFF00;

			//if there is a background color,then mask out any previous background values set, 
			//othewise they will combine to produce an unexpected color
			if (Attribs & 0x00FF00) AttribLine[i] &= 0xFFFF00FF;
			AttribLine[i] |= Attribs;
		}
	}

	Destroy(Tempstr);
	Destroy(EnvName);
	Destroy(Value);
}


static int IsMatchType(TCrayon *Crayon)
{

switch (Crayon->Type)
{
	case CRAYON_LINE:
	case CRAYON_LINEMAPTO:
	case CRAYON_LINENO:
	case CRAYON_VALUE:
	case CRAYON_STRING:
	case CRAYON_MAPTO:
	case CRAYON_SECTION:
	case CRAYON_IF:
		return(TRUE);	
	break;
}

return(FALSE);
}



static int ProcessSubactions(STREAM *Pipe, int *AttribLine, const char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon)
{
int i;

for (i=0; i < Crayon->ActionCount; i++)
{
	if (IsMatchType(&Crayon->Actions[i]))
	{
		ProcessCrayonization(Pipe, Line, Len, AttribLine, &Crayon->Actions[i]);
	}
	else ApplyActions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, &Crayon->Actions[i]);
}
}



static int ProcessActionAndSubactions(STREAM *Pipe, int *AttribLine, const char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon)
{
int result=FALSE;

		if (CrayonMatches(Crayon, MatchStart,MatchEnd)) 
		{
			ApplySingleAction(Pipe,AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
			ProcessSubactions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
			result=TRUE;
		}
	return(result);
}


//This function handles action types that involve adding lines of text to the output,
//usually appends and prepends
static void AppendTextAction(STREAM *Pipe, TCrayon *Crayon)
{
int Len, *Attribs=NULL;
char *Tempstr=NULL;

    Tempstr=SubstituteVarsInString(Tempstr,Crayon->Match,Vars,0);
    Tempstr=CatStr(Tempstr,"\n");
    Len=StrLen(Tempstr);
    Attribs=(int *) calloc(Len, sizeof(int));
    memset(Attribs,0,Len*sizeof(int));
    ProcessActionAndSubactions(Pipe, Attribs, Tempstr, Len, Tempstr, Tempstr+Len, Crayon);

		switch (Crayon->Type)
		{
			case CRAYON_APPEND:
			case CRAYON_PREPEND:
			OutputLineWithAttributes(Tempstr, Attribs, Len);
			break;

			case CRAYON_ONSTART:
			case CRAYON_ONEXIT:
			break;
		}

		Destroy(Tempstr);
		free(Attribs);
}

static void OutputLineWithAttributes(const char *Line, int *Attribs, int Len)
{
int i, LastAttrib=0, BgAttr;
char *Tempstr=NULL;

if (! Attribs) return;
if (Attribs[0] & FLAG_HIDE) return;

for (i=0; i < Len; i++)
{
	if (Attribs[i] != LastAttrib) 
	{
		Tempstr=CatStr(Tempstr,NORM);

		//Bg colors are set into the higher byte of 'attribs', so that we 
		//can hold both fg and bg in the same int, thus we must shift them down
		BgAttr=(Attribs[i] & 0xFF00) >> 8;

		if (Attribs[i] > 0) Tempstr=CatStr(Tempstr,ANSICode(Attribs[i] & 0x00FF, BgAttr, Attribs[i] & 0xFF0000));
		LastAttrib=Attribs[i];
	}
	if (LastAttrib & FLAG_CAPS) Tempstr=AddCharToStr(Tempstr,toupper(Line[i]));
	else if (LastAttrib & FLAG_LOWERCASE) Tempstr=AddCharToStr(Tempstr,tolower(Line[i]));
	else if ((LastAttrib & FLAG_CLR2EOL) && (Line[i]=='\n')) Tempstr=CatStr(Tempstr, "\x1b[K\n");
	//Translate 'del' to 'erase'
	else if (Line[i]=='\177') Tempstr=CatStr(Tempstr,"\010 \010");
	else Tempstr=AddCharToStr(Tempstr,Line[i]);
}

if (LastAttrib > 0) Tempstr=CatStr(Tempstr,NORM);
fwrite(Tempstr,1, StrLen(Tempstr), stdout);
Destroy(Tempstr);
}

void FunctionCall(STREAM *Pipe, const char *FuncName, const char *Data, int DataLen)
{
ListNode *Curr, *Node;

Curr=ListFindNamedItem(Functions, FuncName);
if (Curr) Crayonize(Pipe, Data, DataLen, TRUE, (ListNode *) Curr->Item);
}



int ApplyActions(STREAM *Pipe, int *AttribLine, const char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon)
{
int i,sum=0;
int result=0;
const char *ptr;

if (GlobalFlags & FLAG_DONTCRAYON) return(FALSE);


	//for map to we must calculate which attribute to use
	switch (Crayon->Type)
	{
	case CRAYON_MAPTO:
	case CRAYON_LINEMAPTO:
		if (MatchStart && MatchEnd)
		{
		for (ptr=MatchStart; ptr < MatchEnd; ptr++) sum+=*ptr;
		ApplySingleAction(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, &Crayon->Actions[sum % Crayon->ActionCount]);
		result=TRUE;
		}
	break;

	case CRAYON_PREPEND:
		if (GlobalFlags & FLAG_DOING_PREPENDS) 
		{
			AppendTextAction(Pipe, Crayon);
			result=TRUE;
		}
	break;

	case CRAYON_APPEND:
		if (GlobalFlags & FLAG_DOING_APPENDS) 
		{
			AppendTextAction(Pipe, Crayon);
			result=TRUE;
		}
	break;

	case CRAYON_ONSTART:
		if (GlobalFlags & FLAG_DOING_PREPENDS) 
		{
			AppendTextAction(Pipe, Crayon);
			result=TRUE;
		}
	break;

	case CRAYON_ONEXIT:
		if (GlobalFlags & FLAG_DOING_APPENDS) 
		{
			AppendTextAction(Pipe, Crayon);
			result=TRUE;
		}
	break;

	case CRAYON_TIMER:
	//Key presses will already have been matched
	case CRAYON_KEYPRESS:
		ProcessSubactions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
		result=TRUE;
	break;

	default:
    result=ProcessActionAndSubactions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
	break;

	}

return(result);
}


//AttribLine is the string of attribute values for each char in Line
void ColorSubstring(STREAM *Pipe, int *AttribLine, const char *Line, int Len, TCrayon *Crayon)
{
int result;
ListNode *Matches, *Curr;
TPMatch *Match;


	Matches=ListCreate();
	result=pmatch(Crayon->Match, Line, Len, Matches, PMATCH_SUBSTR);

	if (result > 0)
	{
		Curr=ListGetNext(Matches);
		while (Curr)
		{
			Match=(TPMatch *) Curr->Item;
			if (Match->End) ApplyActions(Pipe, AttribLine, Line, Len, Match->Start, Match->End, Crayon);
		Curr=ListGetNext(Curr);
		}
	}
	
	ListDestroy(Matches,Destroy);

}




int ProcessCrayonization(STREAM *Pipe, const char *Line, int Len, int *Attribs, TCrayon *Crayon)
{
 const char *p_SectionStart, *p_SectionEnd, *ptr;
 char *WorkingSpace=NULL;
 int *WorkingAttribs=NULL;
 int result=FALSE;

	if (Len==0)
	{
		// Working space is provided so that we can 'replace' text 
		WorkingSpace=SetStrLen(WorkingSpace,255);
		Line=WorkingSpace;
		WorkingAttribs=(int *) calloc(255,sizeof(int));
		Attribs=WorkingAttribs;
		Len=255;
	}

	switch (Crayon->Type)
	{
	case CRAYON_LINE:
	case CRAYON_LINEMAPTO:
	if (pmatch(Crayon->Match,Line,Len,NULL,PMATCH_SUBSTR) > 0) 
	{
		result=ApplyActions(Pipe, Attribs, Line, Len, Line, Line+Len, Crayon);
	}
	break;

	case CRAYON_TIMER:
	case CRAYON_ACTION:
	case CRAYON_IF:
	case CRAYON_LINENO:
	case CRAYON_KEYPRESS:
	result=ApplyActions(Pipe, Attribs, Line, Len, Line, Line+Len, Crayon);
	break;

	case CRAYON_SECTION:
	ptr=Line+Len;
	if (Line+Crayon->Start >= ptr) 
	{
		Destroy(WorkingSpace);
		Destroy(WorkingAttribs);
		return(FALSE); 
	}
	p_SectionStart=Line+Crayon->Start;
	if ((p_SectionStart+Crayon->Len) > ptr) p_SectionEnd=ptr;
	else p_SectionEnd=p_SectionStart+Crayon->Len;

	//if the section command has a match string, then call 'ColorSubstring' to get that looked at
	//else call 'ApplyActions' to apply any action list to this section of text
	if (StrLen(Crayon->Match)) ColorSubstring(Pipe, Attribs+Crayon->Start, p_SectionStart, p_SectionEnd-p_SectionStart, Crayon);
	else result=ApplyActions(Pipe, Attribs+Crayon->Start, p_SectionStart, p_SectionEnd-p_SectionStart, p_SectionStart, p_SectionEnd, Crayon);
	break;


	case CRAYON_VALUE:
	case CRAYON_STRING:
	case CRAYON_MAPTO:
		ColorSubstring(Pipe, Attribs, Line, Len, Crayon);
	break;
	}

	Destroy(WorkingSpace);
	Destroy(WorkingAttribs);

return(result);
}


void Crayonize(STREAM *Pipe, const char *Line, int Len, int IsFuncCall, ListNode *CrayonList)
{
int *Attribs=NULL;
ListNode *Curr;

if (LineNo < 0) LineNo=0;
if (Len > 0) 
{
	Attribs=(int *) calloc(Len,sizeof(int));
	if (IsFuncCall) Attribs[0] |= FLAG_HIDE;
}
Curr=ListGetNext(CrayonList);
while (Curr)
{
	ProcessCrayonization(Pipe, Line, Len, Attribs, (TCrayon *) Curr->Item);
	Curr=ListGetNext(Curr);
}
OutputLineWithAttributes(Line, Attribs, Len);
if (GlobalFlags & FLAG_CURSOR_HOME)
{
	GlobalFlags &= ~FLAG_CURSOR_HOME;
	LineNo=-1;
}
if (Attribs) free(Attribs);
}

