#include "keypress.h"
#include "status_bar.h"
#include "config_file.h"
#include "crayonizations.h"
#include "history.h"

TCrayon *KeyPresses=NULL;
int NoOfKeyPresses=0;
int KeypressFlags=0;






//This parses a function-key string sent from keyboard
static void ParseFunctionKey(char *KeySym, int MaxLen, char *ModName, char K1, char K2)
{
switch (K1)
{
case '1':
	switch (K2)
	{
	case '1': snprintf(KeySym,MaxLen,"%s%s",ModName,"F1"); break;
	case '2': snprintf(KeySym,MaxLen,"%s%s",ModName,"F2"); break;
	case '3': snprintf(KeySym,MaxLen,"%s%s",ModName,"F3"); break;
	case '4': snprintf(KeySym,MaxLen,"%s%s",ModName,"F4"); break;
	case '5': snprintf(KeySym,MaxLen,"%s%s",ModName,"F5"); break;
	case '7': snprintf(KeySym,MaxLen,"%s%s",ModName,"F6"); break;
	case '8': snprintf(KeySym,MaxLen,"%s%s",ModName,"F7"); break;
	case '9': snprintf(KeySym,MaxLen,"%s%s",ModName,"F8"); break;
	}
break;

case '2':
	switch (K2)
	{
	case '0': snprintf(KeySym,MaxLen,"%s%s",ModName,"F9"); break;
	case '1': snprintf(KeySym,MaxLen,"%s%s",ModName,"F10"); break;
	case '3': snprintf(KeySym,MaxLen,"%s%s",ModName,"F11"); break;
	case '4': snprintf(KeySym,MaxLen,"%s%s",ModName,"F12"); break;
	case '5': snprintf(KeySym,MaxLen,"%s%s",ModName,"F13"); break;
	case '6': snprintf(KeySym,MaxLen,"%s%s",ModName,"F14"); break;
	case '8': snprintf(KeySym,MaxLen,"%s%s",ModName,"F15"); break;
	case '9': snprintf(KeySym,MaxLen,"%s%s",ModName,"menu"); break;
	}
break;

case '3':
	switch (K2)
	{
	case '1': snprintf(KeySym,MaxLen,"%s%s",ModName,"F17"); break;
	case '2': snprintf(KeySym,MaxLen,"%s%s",ModName,"F18"); break;
	case '3': snprintf(KeySym,MaxLen,"%s%s",ModName,"F19"); break;
	case '4': snprintf(KeySym,MaxLen,"%s%s",ModName,"F20"); break;
	}
break;
}

}



static int ParseModifiedKey(char *KeySym, int MaxLen, char *ModName, char Key)
{

	switch (Key)
	{
			case '1':
				snprintf(KeySym,MaxLen,"%s%s",ModName,"home"); 
			break;

			case '2':
				snprintf(KeySym,MaxLen,"%s%s",ModName,"insert"); 
			break;

			case '3':
				snprintf(KeySym,MaxLen,"%s%s",ModName,"delete"); 
			break;

			case '4':
				snprintf(KeySym,MaxLen,"%s%s",ModName,"end"); 
			break;

			case '5':
				snprintf(KeySym,MaxLen,"%s%s",ModName,"pgup"); 
			break;

			case '6': 
				snprintf(KeySym,MaxLen,"%s%s",ModName,"pgdn"); 
			break;
	}

}




//Sequences that consist of <esc>[<digit> can have a modifier character at 
//the end
static char ReadCSI_Num_Mod(STREAM *StdIn, char Digit, char *KeySym, int MaxLen)
{
int result;
char inchar=0, *Num=NULL, *Modifier=NULL;


Num=AddCharToStr(Num,Digit);
result=STREAMReadBytes(StdIn, &inchar,1);

switch (inchar)
{
	case '~':
		//no modifier
		ParseModifiedKey(KeySym, MaxLen, "", Digit);
	break;

	case '$':
		//shift
		ParseModifiedKey(KeySym, MaxLen, "shift-", Digit);
	break;

	case '^':
		//ctrl
		ParseModifiedKey(KeySym, MaxLen, "ctrl-", Digit);
	break;

	//'fake shift' sequences, I think
	case ';':
		Num=SetStrLen(Num,2);
		result=STREAMReadBytes(StdIn, Num,2);
		if (result==2)
		{
		if (strcmp(Num,"2A")==0) strcpy(KeySym,"shift-up");
		else if (strcmp(Num,"2B")==0) strcpy(KeySym,"shift-down");
		else if (strcmp(Num,"2C")==0) strcpy(KeySym,"shift-right");
		else if (strcmp(Num,"2D")==0) strcpy(KeySym,"shift-left");
		else if (strcmp(Num,"5A")==0) strcpy(KeySym,"ctrl-up");
		else if (strcmp(Num,"5B")==0) strcpy(KeySym,"ctrl-down");
		else if (strcmp(Num,"5C")==0) strcpy(KeySym,"ctrl-right");
		else if (strcmp(Num,"5D")==0) strcpy(KeySym,"ctrl-left");
		}
	break;

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
		Num=AddCharToStr(Num,inchar);
		result=STREAMReadBytes(StdIn, &inchar,1);
		switch (inchar)
		{
		case '~':
		ParseFunctionKey(KeySym, MaxLen, "", Num[0], Num[1]);
		break;

		case '$':
		ParseFunctionKey(KeySym, MaxLen, "shift-", Num[0], Num[1]);
		break;

		case '^':
		ParseFunctionKey(KeySym, MaxLen, "ctrl-", Num[0], Num[1]);
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
		Num=AddCharToStr(Num,inchar);
		result=STREAMReadBytes(StdIn, &inchar,1);
		switch (inchar)
		{
			case '~':
				if (strcmp(Num, "200")==0) KeySym=CopyStr(KeySym, "paste") ;
				if (strcmp(Num, "201")==0) KeySym=CopyStr(KeySym, "paste-end") ;
			break;
		}
		break;
		}
	break;
}

Destroy(Num);
return(inchar);
}





static int ReadEscapeSequence(STREAM *StdIn, char *Sequence, char *KeySym, int MaxLen)
{
int result;
char *ptr, *end;

ptr=Sequence;
*ptr='\x1b';
ptr++;
end=ptr;
strncpy(KeySym,"escape",MaxLen);

result=STREAMReadBytes(StdIn, ptr,1);
if (result > 0)
{
end=ptr+1;
switch (*ptr)
{
	case '\0':
	case '\x1b':
		strncpy(KeySym,"escape",MaxLen);
		return(end-Sequence);
	break;

	case 'O': 
		result=STREAMReadBytes(StdIn, end, 1);
		switch (*end)
		{
		case 'A': strncpy(KeySym,"ctrl-up",MaxLen); break;
		case 'B': strncpy(KeySym,"ctrl-down",MaxLen); break; 
		case 'c': strncpy(KeySym,"ctrl-left",MaxLen); break;
		case 'd': strncpy(KeySym,"ctrl-right",MaxLen); break;
		case 'P': strncpy(KeySym,"F1",MaxLen); break;
		case 'Q': strncpy(KeySym,"F2",MaxLen); break;
		case 'R': strncpy(KeySym,"F3",MaxLen); break;
		case 'S': strncpy(KeySym,"F4",MaxLen); break;
		}
		
		end++;
		return(end-Sequence); 
	break;

	case '[':
		ptr++;
		result=STREAMReadBytes(StdIn, ptr,1);
		if (result > 0)
		{
			end=ptr+1;
			switch (*ptr)
			{
				case 'a': strncpy(KeySym,"shift-up",MaxLen); return(end-Sequence); break;
				case 'b': strncpy(KeySym,"shift-down",MaxLen); return(end-Sequence); break;
				case 'c': strncpy(KeySym,"shift-right",MaxLen); return(end-Sequence); break;
				case 'd': strncpy(KeySym,"shift-left",MaxLen); return(end-Sequence); break;
				case 'A': strncpy(KeySym,"up",MaxLen); return(end-Sequence); break;
				case 'B': strncpy(KeySym,"down",MaxLen); return(end-Sequence); break;
				case 'C': strncpy(KeySym,"right",MaxLen); return(end-Sequence); break;
				case 'D': strncpy(KeySym,"left",MaxLen); return(end-Sequence); break;
				case 'H': strncpy(KeySym,"home",MaxLen); return(end-Sequence); break;
				case 'P': strncpy(KeySym,"pause",MaxLen); return(end-Sequence); break;
				case 'F': strncpy(KeySym,"end",MaxLen); return(end-Sequence); break;
				case 'Z': strncpy(KeySym,"shift-tab",MaxLen); return(end-Sequence); break;

				//window focus/unfocus events
				case 'I': 
					strncpy(KeySym,"focus",MaxLen); 
					GlobalFlags |= FLAG_FOCUSED;
					return(end-Sequence); 
				break;

				case 'O': 
					strncpy(KeySym,"unfocus",MaxLen); 
					GlobalFlags &= ~FLAG_FOCUSED;
					return(end-Sequence); 
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
				*end=ReadCSI_Num_Mod(StdIn, *ptr, KeySym, MaxLen);
				end++;
				break;
			}
		}
	break;

	default:
		//escape <key> is sent by alt-<key> in some terminals
		if (*ptr != '\0') 
		{
			snprintf(KeySym,MaxLen,"alt-%c",*ptr); 
			end=ptr+1;
			return(end-Sequence);	
		}
	break;
}

}

return(end-Sequence);
}




static int KeyPressRead(STREAM *StdIn, char **Data, char **KeySym, int MaxLen)
{
int result, i, j;
TCrayon *KP;

*Data=SetStrLen(*Data,MaxLen);
*KeySym=SetStrLen(*KeySym,MaxLen);

result=STREAMReadBytes(StdIn, *Data ,1);
StrTrunc(*Data, 1); 

if (result > 0)
{
	switch (**Data)
	{
	case EOF: return(result); break;

	case '\x1b': result=ReadEscapeSequence(StdIn, *Data, *KeySym, MaxLen); break;

  case 0x8: //shift-backspace
		strncpy(*KeySym,"shift-backspace",MaxLen);
		strncpy(*KeySym,"backspace",MaxLen);
  break;

  case 0x7F: //backspace
		strncpy(*KeySym,"backspace",MaxLen);
	break;

	case '\n':
		strncpy(*KeySym,"newline",MaxLen);
	break;

	case '\r':
		strncpy(*KeySym,"return",MaxLen);
	break;
	}
}

if (! StrValid(*KeySym)) *KeySym=CopyStr(*KeySym, *Data);

return(result);
}


static int FindKeypressAction(STREAM *Out, const char *KeySym)
{
int i;

//KeyPresses here are the actions booked against different keypress events
for (i=0; i < NoOfKeyPresses; i++)
{
	if (strcmp(KeyPresses[i].Match,KeySym)==0)
	{
		ProcessCrayonization(Out, NULL, 0, NULL, &(KeyPresses[i]));
		{
			return(TRUE);
		}
	}
}

	return(FALSE);
}

//This parses 'keypress' entries in the config file
TCrayon *KeypressParse(const char *Data)
{
TCrayon *KP;
const char *ptr;


if ((NoOfKeyPresses % 10)==0) 
{
	KeyPresses=(TCrayon *) realloc((void *) KeyPresses, (NoOfKeyPresses+10) * sizeof(TCrayon));
}

KP=KeyPresses + NoOfKeyPresses;
memset(KP,0,sizeof(TCrayon));
KP->Type=CRAYON_KEYPRESS;
ptr=GetToken(Data,"\\S",&KP->Match,0);
ParseActionToken(ptr,KP);

NoOfKeyPresses++;

return(KP);
}



int KeypressProcess(STREAM *StdIn, STREAM *Out)
{
int result, i, bytes_read;
char *Tempstr=NULL, *KeySym=NULL;
int MaxLen=20;

bytes_read=KeyPressRead(StdIn, &Tempstr, &KeySym, MaxLen);
if (bytes_read > 0)
{
    if (StatusBarHandleInput(Out, Tempstr, KeySym)) bytes_read=0;
		else if (FindKeypressAction(Out, KeySym)) bytes_read=0;
		else
		{
			TypingHistoryAddKeypress(Tempstr, bytes_read);
  		STREAMWriteBytes(Out,Tempstr,bytes_read);
  		STREAMFlush(Out);
		}
}

Destroy(Tempstr);
Destroy(KeySym);

return(bytes_read);
}


