#include "xterm.h"
#include "status_bar.h"
#include "crayonizations.h"


int ProcessCrayonization(STREAM *Pipe, char *Line, int Len, int *Attribs, TCrayon *Crayon);



int IsInStringList(char *Item, char *List, char *Separator) 
{
char *Token=NULL, *ptr;
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

DestroyString(Token);

return(result);
}



int HandleCrayonIf(TCrayon *Crayon)
{
char *Expr=NULL, *Tempstr=NULL, *Token=NULL, *PrevToken=NULL, *ptr, *tptr, *aptr;
int result=FALSE, val, i;

	Expr=SubstituteVarsInString(Expr,Crayon->Match,Vars,0);


	ptr=GetToken(Expr,"\\S",&Token,GETTOKEN_QUOTES);
	while (ptr)
	{
		switch (*Token)
		{
			case 'a':
				result=FALSE;
				if (strncmp(Token,"arg(",4)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+4);
					tptr=strrchr(Tempstr,')');
					if (tptr) *tptr='\0';
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



			case 'l':
				result=FALSE;
				if (strncmp(Token,"larg(",5)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+5);
					tptr=strrchr(Tempstr,')');
					if (tptr) *tptr='\0';
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
					tptr=strrchr(Tempstr,')');
					if (tptr) *tptr='\0';

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
					tptr=strrchr(Tempstr,')');
					if (tptr) *tptr='\0';

					if (strcmp(Tempstr,"stdin")==0) val=0;
					else val=1;
					result=isatty(val);
				}
			break;

			case 'n':
				if (strncmp(Token,"notatty(",8)==0)
				{
					Tempstr=CopyStr(Tempstr,Token+8);
					tptr=strrchr(Tempstr,')');
					if (tptr) *tptr='\0';
					if (strcmp(Tempstr,"stdin")==0) val=0;
					else val=1;
					result= !isatty(val);
				}
			break;


			case '<':
				if (*(ptr+1) == '=')
				{
					ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
					result=(atoi(PrevToken) <= atoi(Token));
				}
				else
				{
				ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
				result=(atoi(PrevToken) < atoi(Token));
				}
			break;

			case '>':
				if (*(ptr+1) == '=')
				{
					ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
					result=(atoi(PrevToken) >= atoi(Token));
				}
				else
				{
					ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
					result=(atoi(PrevToken) > atoi(Token));
				}
			break;


			case '=':
				ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
				result=pmatch(Token, PrevToken, StrLen(PrevToken), NULL, 0);
				if (result > 0) result=TRUE;
				else result=FALSE;
			break;

			case '!':
				ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
				result=pmatch(Token, PrevToken, StrLen(PrevToken), NULL, 0);
				if (result > 0) result=FALSE;
				else result=TRUE;
			break;
		}
		PrevToken=CopyStr(PrevToken,Token);
		ptr=GetToken(ptr,"\\S",&Token,GETTOKEN_QUOTES);
	}


DestroyString(PrevToken);
DestroyString(Tempstr);
DestroyString(Token);
DestroyString(Expr);
return(result);
}




int CrayonMatches(TCrayon *Crayon, char *sptr, char *eptr)
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
case CRAYON_PASSINPUT: 
case CRAYON_EXEC: 
case CRAYON_CMDLINE: 
case CRAYON_VALUE: 
	return(TRUE);
break;


case CRAYON_COMPARATOR:
	val=atof(sptr);
//For a 'value' item, it's actions can contain multiple 'value' checks
	switch (Crayon->Op)
	{
	case '<':
	if (val < Crayon->Value) result=TRUE;
	break;
	
	case '>':
	if (val > Crayon->Value) result=TRUE;
	
	break;
	
	case '=':
	if (val == Crayon->Value) result=TRUE;
	break;
	
	case '!':
	if (val != Crayon->Value) result=TRUE;
	break;
	}
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


void PassToProgram(STREAM *Pipe, char *ActivateLine, char *Program)
{
STREAM *S, *CmdS, *CmdErr;
char *Tempstr=NULL;
int result, wrote;
ListNode *Streams;


Streams=ListCreate();
CmdS=STREAMCreate();
CmdErr=STREAMCreate();
PipeSpawn( &(CmdS->out_fd), &(CmdS->in_fd), &(CmdErr->in_fd), Program);

ListAddItem(Streams,Pipe);
ListAddItem(Streams,CmdS);
ListAddItem(Streams,CmdErr);
Tempstr=SetStrLen(Tempstr,4096);
STREAMFlush(Pipe);

write(CmdS->out_fd,ActivateLine,StrLen(ActivateLine));
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
			write(1,Tempstr,result);
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
DestroyString(Tempstr);
}



void ApplySingleAction(STREAM *Pipe, int *AttribLine, char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Action)
{
int Attribs;
int start, end, i;
char *Tempstr=NULL, *EnvName=NULL, *ptr;

if (GlobalFlags & FLAG_DONTCRAYON) return;

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

		case ACTION_SET_XTITLE: 
			Tempstr=CopyStr(Tempstr,"\x1b]0;");
			Tempstr=CatStrLen(Tempstr,MatchStart,end-start);
			StripTrailingWhitespace(Tempstr);
			Tempstr=CatStr(Tempstr,"\x07");
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_RESTORE_XTITLE:
			Tempstr=CopyStr(Tempstr,"\x1b]0;");
			Tempstr=CatStr(Tempstr,GetVar(Vars,"OldXtermTitle"));
			StripTrailingWhitespace(Tempstr);
			Tempstr=CatStr(Tempstr,"\x07");
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_FONT_UP:
			Tempstr=CopyStr(Tempstr,"\x1b[?35h\x1b]50;#+1\x07");
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_FONT_DOWN:
			Tempstr=CopyStr(Tempstr,"\x1b[?35h\x1b]50;#-1\x07");
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_REPLACE:
		strcpy(MatchStart,Action->String);
		break;

		case ACTION_SETENV:
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
			setenv(EnvName,Tempstr,TRUE);
			SetVar(Vars,EnvName,Tempstr);
		break;

		case ACTION_ARGS:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			SetVar(Vars,"ExtraCmdLineOptions",Tempstr);
		break;

		case ACTION_PLAYSOUND:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			SoundPlayFile("/dev/dsp",Action->String,VOLUME_LEAVEALONE, PLAYSOUND_NONBLOCK);
		break;

		case ACTION_EXEC:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
		break;

		case ACTION_PASSTO:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			PassToProgram(Pipe, Line, Tempstr);
		break;

		case ACTION_SEND:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			STREAMWriteLine(Tempstr,Pipe); STREAMFlush(Pipe);
			usleep(250000);
		break;

		case ACTION_ECHO:
			Tempstr=SubstituteVarsInString(Tempstr,Action->String,Vars,0);
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_ALTSCREEN:
			write(1,"\x1b[?47h",6); 
		break;

		case ACTION_NORMSCREEN:
			write(1,"\x1b[?47l",6); 
		break;
		
		case ACTION_BELL:
			write(1,"\x07",1);
		break;

		case ACTION_XTERM_FGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]10;",EnvName,"\007",NULL);
			write(1,Tempstr,StrLen(Tempstr));
		break;


		case ACTION_XTERM_BGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]11;",EnvName,"\007",NULL);
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_RXVT_FGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]39;",EnvName,"\007",NULL);
			write(1,Tempstr,StrLen(Tempstr));
		break;

		case ACTION_RXVT_BGCOLOR:
			EnvName=SubstituteVarsInString(EnvName,Action->String,Vars,0);
			Tempstr=MCopyStr(Tempstr,"\x1b]49;",EnvName,"\007",NULL);
			write(1,Tempstr,StrLen(Tempstr));
		break;


		case ACTION_CLEARSCREEN:
			write(1,CLRSCR,StrLen(CLRSCR)); 
		break;

		case ACTION_EDIT:
			StatusBarHandleInput(Pipe, NULL, Action->Attribs);
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

	DestroyString(Tempstr);
	DestroyString(EnvName);
}


int IsMatchType(TCrayon *Crayon)
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


int ProcessActionAndSubactions(STREAM *Pipe, int *AttribLine, char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon)
{
int i, result=FALSE;

		if (CrayonMatches(Crayon, MatchStart,MatchEnd)) 
		{
			ApplySingleAction(Pipe,AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
			result=TRUE;
			for (i=0; i < Crayon->ActionCount; i++)
			{
				if (IsMatchType(&Crayon->Actions[i]))
				{
					 ProcessCrayonization(Pipe, Line, Len, AttribLine, &Crayon->Actions[i]);
				}
				else ApplyActions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, &Crayon->Actions[i]);
			}
		}
	return(result);
}

//This function handles action types that involve adding lines of text to the output,
//usually appends and prepends
void AppendTextAction(STREAM *Pipe, TCrayon *Crayon)
{
int Len, *Attribs=NULL;
char *Tempstr=NULL;

    Tempstr=SubstituteVarsInString(Tempstr,Crayon->Match,Vars,0);
    Tempstr=CatStr(Tempstr,"\n");
    Len=StrLen(Tempstr);
    Attribs=(int *) calloc(Len, sizeof(int));
    memset(Attribs,0,Len*sizeof(int));
    ProcessActionAndSubactions(Pipe, Attribs, Tempstr, Len, Tempstr, Tempstr+Len, Crayon);

		OutputLineWithAttributes(Tempstr, Attribs, Len);

		DestroyString(Tempstr);
		free(Attribs);
}


int ApplyActions(STREAM *Pipe, int *AttribLine, char *Line, int Len, char *MatchStart, char *MatchEnd, TCrayon *Crayon)
{
int i,sum=0;
char *ptr;
int result=0;

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

	case CRAYON_APPEND:
		if (GlobalFlags & FLAG_DOING_APPENDS) 
		{
			AppendTextAction(Pipe, Crayon);
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

	//Key presses will already have been matched
	case CRAYON_KEYPRESS:
	for (i=0; i < Crayon->ActionCount; i++)
	{
		ApplySingleAction(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, &Crayon->Actions[i]);
		result=TRUE;
	}
	break;

	default:
    result=ProcessActionAndSubactions(Pipe, AttribLine, Line, Len, MatchStart, MatchEnd, Crayon);
	break;

	}

return(result);
}


//AttribLine is the string of attribute values for each char in Line
void ColorSubstring(STREAM *Pipe, int *AttribLine, char *Line, int Len, TCrayon *Crayon)
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
			ApplyActions(Pipe, AttribLine, Line, Len, Match->Start, Match->End, Crayon);
		Curr=ListGetNext(Curr);
		}
	}
	
	ListDestroy(Matches,DestroyString);

}


void OutputLineWithAttributes(char *Line, int *Attribs, int Len)
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
	else Tempstr=AddCharToStr(Tempstr,Line[i]);
}

Tempstr=CatStr(Tempstr,NORM);
write(1,Tempstr,StrLen(Tempstr));
DestroyString(Tempstr);
}


int ProcessCrayonization(STREAM *Pipe, char *Line, int Len, int *Attribs, TCrayon *Crayon)
{
 char *p_SectionStart, *p_SectionEnd, *ptr;
 int result=FALSE;

	switch (Crayon->Type)
	{
	case CRAYON_LINE:
	case CRAYON_LINEMAPTO:
	if (pmatch(Crayon->Match,Line,Len,NULL,PMATCH_SUBSTR) > 0) 
	{
		result=ApplyActions(Pipe, Attribs, Line, Len, Line, Line+Len, Crayon);
	}
	break;


	case CRAYON_IF:
	case CRAYON_LINENO:
	result=ApplyActions(Pipe, Attribs, Line, Len, Line, Line+Len, Crayon);
	break;

	case CRAYON_SECTION:
	ptr=Line+Len;
	if (Line+Crayon->Start >= ptr) return; 
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

return(result);
}


void ColorLine(STREAM *Pipe, char *Line, int Len, ListNode *ColorMatches)
{
int *Attribs=NULL;
ListNode *Curr;

Attribs=(int *) calloc(Len,sizeof(int));

Curr=ListGetNext(ColorMatches);
while (Curr)
{
	ProcessCrayonization(Pipe, Line, Len, Attribs, (TCrayon *) Curr->Item);
	Curr=ListGetNext(Curr);
}
OutputLineWithAttributes(Line, Attribs, Len);
free(Attribs);
}

