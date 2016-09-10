#include "escape_sequences.h"

//This handles ANSI sequences that start ESC[, these are 'Control Sequence Introducer' (CSI) codes 
//Some of these we need to detect, because they clear the screen or such
int EscapeSequenceCSI(char **text, char *end)
{
int val, AtEnd=FALSE;

//first character will be '['
(*text)++;

	//special case, some xterm commands
	if (**text=='?')
	{
		(*text)++;
		if (strncmp(*text, "47h", 3)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			*(text)+=3;
			return(ES_OKAY);
		}
		else if (strncmp(*text, "47l", 3)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			*(text)+=3;
			return(ES_OKAY);
		}
		else if (strncmp(*text, "1049h", 5)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			*(text)+=5;
			return(ES_OKAY);
		}
		else if (strncmp(*text, "1049l", 5)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			*(text)+=5;
			return(ES_OKAY);
		}
		else if (strncmp(*text, "1047h", 5)==0)
		{
			GlobalFlags |= FLAG_ALTERNATE_SCREEN;
			*(text)+=5;
			return(ES_OKAY);
		}
		else if (strncmp(*text, "1047l", 5)==0)
		{
			GlobalFlags &= ~FLAG_ALTERNATE_SCREEN;
			GlobalFlags |= FLAG_REDRAW;
			*(text)+=5;
			return(ES_OKAY);
		}
	}

	while (*text < end)
	{
	switch (**text)
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
		case ';':
		break;

		//if we hit escape or CTRLO we don't want to
		//or normal AtEnd processiong, just return 
		case '\x1b':
		case CTRLO:
			return(ES_OKAY);
		break;

		default:
		AtEnd=TRUE;
		break;
	}

	(*text)++;
	if (**text==';') AtEnd=FALSE;
	if (AtEnd) 
	{
		if (GlobalFlags & FLAG_STRIP_ANSI) return(ES_STRIP);
		return(ES_OKAY);
	}
	}

return(ES_PART);
}

int EscapeSequenceOSC(char **text, char *end)
{
char *text_start=NULL, *text_end=NULL;
char *Str=NULL;

//first character will be ']'
(*text)++;
switch (**text)
{
//set title bar
case '0':
case '1':
case '2':
	(*text)++;
	if (**text==';') text_start=(*text)+1;
	while ((*text < end) && (**text != '\007'))  (*text)++;
	if (*text==end) return(ES_PART);
	text_end=*text;
	if (**text =='\007') (*text)++;
	if (GlobalFlags & FLAG_STRIP_XTITLE) 
	{
		if (text_start)
		{
		Str=CopyStrLen(Str, text_start, text_end-text_start);
		SetVar(Vars,"crayon_xtitle",Str);
		DestroyString(Str);	
		}
		return(ES_STRIP);
	}
	if (GlobalFlags & FLAG_STRIP_ANSI) return(ES_STRIP);
	return(ES_OKAY);
break;

default:
	while ((*text < end) && (**text != '\007') && (**text != '\x1b'))  (*text)++;
	if (*text==end) return(ES_PART);
	if (**text =='\007') (*text)++;
	if (GlobalFlags & FLAG_STRIP_ANSI) return(ES_STRIP);
	return(ES_OKAY);
break;
}

}

//Handle ANSI coming from the crayonized program 
int EscapeSequenceHandle(char **text, char *end)
{
char *ptr;

if (**text==CTRLO)
{
	(*text)++;
	if (GlobalFlags & FLAG_STRIP_ANSI) return(ES_STRIP);
	return(ES_OKAY);
}

//otherwise it must be escape
(*text)++;
if (**text=='[') return(EscapeSequenceCSI(text, end));
if (**text==']') return(EscapeSequenceOSC(text, end));

for (ptr=*text; ptr < end; ptr++)
{
	switch (*ptr)
	{
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
	case ';':
		//numbers are part of the escape code. Codes end with a numeric or certain punctuation
	break;

	case 'H':
	case '@':
	  //we never strip these control codes, which are about homing the cursor or
		//ESC[<val>@ means 'make room for characters to be inserted'
		if (*(ptr+1) != ';')
		{
			*text=ptr+1;
			return(ES_OKAY);
		}
	break;

	default:
			*text=ptr+1;
			if (GlobalFlags & FLAG_STRIP_ANSI) return(ES_STRIP);
			return(ES_OKAY);
	break;
	}
}

//if we get to here we must have a partial escape code	
return(ES_PART);
}

