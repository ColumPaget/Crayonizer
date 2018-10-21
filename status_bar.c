#include "status_bar.h"
#include "xterm.h"
#include "text_substitutions.h"
#include "config_file.h"
#include "crayonizations.h"

ListNode *g_TopSB=NULL, *g_BottomSB=NULL;
int ScrollAreaEnd=0;
ListNode *Selections=NULL;
TCrayon *Active=NULL;

//Prototype for recursive call 
void DrawStatusBar(TStatusBar *SB);


TCrayon *StatusBarGetActive()
{
return(Active);
}



void StatusBarMargins(int *TopMargin, int *BottomMargin)
{
	*TopMargin=0;
	*BottomMargin=0;
	if (ListSize(g_TopSB)) *TopMargin=1;
	if (ListSize(g_BottomSB)) *BottomMargin=1;
}


void StatusBarAdjustCursor(int *x, int *y)
{
int TopMargin, BottomMargin;

StatusBarMargins(&TopMargin, &BottomMargin);	
if (*y <= TopMargin) *y=TopMargin+1;
if (*y >= ScrollAreaEnd) *y=ScrollAreaEnd;
}


int SetScrollRegion(int TopMargin, int BottomMargin)
{
int cursx=0, cursy=0;
char *ANSI=NULL;

	ScrollAreaEnd=ScreenRows;
	ScrollAreaEnd-=BottomMargin;

	XTermReadCursorPos(&cursx, &cursy);
	StatusBarAdjustCursor(&cursx, &cursy);

  ANSI=FormatStr(ANSI,"\x1b[%d;%dr\x1b[%d;%dH",TopMargin+1,ScrollAreaEnd-1,cursy,cursx);
  write(1,ANSI,StrLen(ANSI));

	Destroy(ANSI);

return(ScrollAreaEnd);
}





//This parses 'keypress' entries in the config file
void StatusBarParseSelection(STREAM *S, const char *Name)
{
char *Tempstr=NULL, *Token=NULL;
const char *ptr;
TCrayon *Item, *SubItem;
ListNode *Curr;

if (! Selections) Selections=ListCreate();
Item=(TCrayon *) calloc(1, sizeof(TCrayon));
ListAddNamedItem(Selections, Name, Item);

Tempstr=STREAMReadLine(Tempstr,S);
StripLeadingWhitespace(Tempstr);
StripTrailingWhitespace(Tempstr);

Tempstr=STREAMReadLine(Tempstr,S);
while (Tempstr)
{
  StripTrailingWhitespace(Tempstr);
  StripLeadingWhitespace(Tempstr);

  ptr=GetToken(Tempstr,"\\S",&Token,0);
  if (strcmp(Token,"}")==0) break;
  //else if (strcmp(Token,"{")==0) ParseCrayonList(S, SubItem);
	else
	{
	SubItem=NewCrayonAction(Item, 0);
	SubItem->Type=CRAYON_STRING;
	SubItem->Match=CopyStr(SubItem->Match, Token);
	ParseCrayonAction(SubItem, ptr);
	}

Tempstr=STREAMReadLine(Tempstr,S);
}

Destroy(Tempstr);
Destroy(Token);
}


//Pop doesn't remove the status bar. Destroy does that
TStatusBar *StatusBarPopStack(ListNode *Stack)
{
ListNode *Curr;

Curr=ListGetLast(Stack);

if (! Curr) return(NULL);
if (Curr==Stack) return(NULL);
return((TStatusBar *) Curr->Item);
}


void UpdateStatusBars(int ForceUpdate)
{
TStatusBar *SB;

SB=(TStatusBar *) StatusBarPopStack(g_TopSB);
if (SB && (ForceUpdate || (Now > SB->NextRefresh))) 
{
	DrawStatusBar(SB);
	SB->NextRefresh=Now + (int) SB->RefreshInterval;
}

SB=(TStatusBar *) StatusBarPopStack(g_BottomSB);
if (SB && (ForceUpdate || (Now > SB->NextRefresh))) 
{
	DrawStatusBar(SB);
	SB->NextRefresh=Now + (int) SB->RefreshInterval;
}
}


void StatusBarHide(TStatusBar *SB)
{
int x=0, y=0;
char *ANSI=NULL;

	XTermReadCursorPos(&x, &y);

	ANSI=FormatStr(ANSI,"\x1b[%d;0H%s\x1b[K",SB->pos,NORM);
  write(1,ANSI,StrLen(ANSI));
	ANSI=FormatStr(ANSI,"\x1b[%d;%dH",y,x);
  write(1,ANSI,StrLen(ANSI));

Destroy(ANSI);
}


void StatusBarDestroy(TStatusBar *SB)
{
ListNode *Curr;

if (SB->Flags & FLAG_PERSIST) return;
Curr=ListFindItem(g_TopSB, SB);
if (Curr) 
{
	ListDeleteNode(Curr);
	g_TopSB=NULL;
}

Curr=ListFindItem(g_BottomSB, SB);
if (Curr) 
{
	ListDeleteNode(Curr);
	g_BottomSB=NULL;
}

Destroy(SB->Text);
ListDestroy(SB->Items,Destroy);
free(SB);

UpdateStatusBars(FALSE);
}

void StatusBarClose(TStatusBar *SB)
{
int ScrollTop=0, ScrollBottom=0;

	StatusBarHide(SB);
	StatusBarDestroy(SB);

	StatusBarMargins(&ScrollTop, &ScrollBottom);
	SetScrollRegion(ScrollTop, ScrollBottom);
	Active=NULL;
}



//Close any temporary statusbars created on a keypress
void StatusBarCloseTemps(ListNode *List)
{
ListNode *Curr, *Next;
TStatusBar *SB;

Curr=ListGetNext(List);
while (Curr)
{
SB=(TStatusBar *) Curr->Item;
Next=ListGetNext(Curr);
if (SB->Flags & STATUSBAR_CLOSE_ON_NEW)
{
	StatusBarClose(SB);
	ListDeleteNode(Curr);
}
Curr=Next;
}
}


void StatusBarPushStack(TStatusBar *Top, TStatusBar *Bottom)
{
ListNode *Curr;
if (! g_TopSB) g_TopSB=ListCreate();
if (! g_BottomSB) g_BottomSB=ListCreate();

//Close any temporary statusbars created on a keypress
if (Top) StatusBarCloseTemps(g_TopSB);
if (Bottom) StatusBarCloseTemps(g_BottomSB);


if (Top) ListAddItem(g_TopSB, Top);
if (Bottom) ListAddItem(g_BottomSB, Bottom);
}



void StatusBarCloseActive()
{
TStatusBar *SB;

SB=StatusBarPopStack(g_BottomSB);
StatusBarClose(SB);
}

void StatusBarAddHistory(TStatusBar *SB, const char *Text)
{
if (! SB->Items) SB->Items=ListCreate();
ListAddItem(SB->Items,CopyStr(NULL,Text));
SB->Curr=NULL;
}


char *FormatStatusEditText(char *RetStr, TStatusBar *SB)
{
char *ptr, *Tempstr=NULL, *DefaultStr=NULL;

	if (! (SB->Flags & STATUSBAR_EDIT)) RetStr=CatStr(RetStr," [editing off]");
	else
	{
		//Can't use ANSICode twice in one MCopy, because it has an internal buffer
		DefaultStr=MCopyStr(DefaultStr, " ", ANSICode(SB->Attribs & 0xFF, (SB->Attribs & 0xFF00) >>8, SB->Attribs & 0xFF0000), NULL);

		if (SB->EditLen > SB->EditCursor)
		{
			ptr=SB->EditText + SB->EditCursor;
			RetStr=CatStrLen(RetStr, SB->EditText, ptr-SB->EditText);
			Tempstr=FormatStr(Tempstr,"%s%c%s%s",ANSICode(ANSI_RED,0,ANSI_INVERSE),*ptr,NORM,DefaultStr);
			RetStr=MCatStr(RetStr,Tempstr,ptr+1,NULL);
		}
		else RetStr=MCatStr(RetStr,SB->EditText,ANSICode(ANSI_RED,0,ANSI_INVERSE)," ",NORM,DefaultStr, NULL);
	}

	Destroy(DefaultStr);
	Destroy(Tempstr);

return(RetStr);
}




void DrawStatusBar(TStatusBar *SB)
{
int x=0, y=0;
char *ANSI=NULL;

	if (! SB) return;
	if (GlobalFlags & FLAG_ALTERNATE_SCREEN) return;
	XTermReadCursorPos(&x, &y);
	StatusBarAdjustCursor(&x, &y);

	ANSI=FormatStr(ANSI,"\x1b[%d;0H",SB->pos);
	ANSI=MCatStr(ANSI, ANSICode(SB->Attribs & 0xFF, (SB->Attribs & 0xFF00) >> 8, SB->Attribs & 0xFF0000), NULL);
	fputs(ANSI, stdout);

	if ( (SB->Type==ACTION_QUERYBAR) || (SB->Type==ACTION_HISTORYBAR) ) ANSI=FormatStatusEditText(ANSI, SB);
	else ANSI=SubstituteTextValues(ANSI, SB->Text, ScreenCols);

	//Clear to end of line
	ANSI=MCatStr(ANSI, "\x1b[K", NORM, NULL);
	fputs(ANSI, stdout);

	ANSI=FormatStr(ANSI,"\x1b[%d;%dH",y,x);
	fputs(ANSI, stdout);

	Destroy(ANSI);
}



			

static int StatusBarParseKeySym(const char *KeySym)
{
int len;
char *Tempstr=NULL, *ptr;

switch (*KeySym)
{
	case 'b': 
		if (strcmp(KeySym,"backspace")==0) return(SB_EDIT_BACKSPACE); 
	break;


	case 'd': 
		if (strcmp(KeySym,"down")==0) return(SB_EDIT_PREV); 
		if (strcmp(KeySym,"delete")==0) return(SB_EDIT_DELETE); 
	break;

	case 'e':
		if (strcmp(KeySym,"escape")==0) return(SB_EDIT_CLOSE); 
	break;

	case 'u': 
		if (strcmp(KeySym,"up")==0) return(SB_EDIT_NEXT); 
	break;

	case 'l': 
		if (strcmp(KeySym,"left")==0) return(SB_EDIT_LEFT); 
	break;

	case 'r': 
		if (strcmp(KeySym,"right")==0) return(SB_EDIT_LEFT); 
		if (strcmp(KeySym,"return")==0) return(SB_EDIT_ENTER); 
	break;


	case 's': 
		if (strcmp(KeySym,"shift-backspace")==0) return(SB_EDIT_CLEAR); 
	break;
}

return(0);
}


void EditBarSetText(TStatusBar *SB, const char *Text)
{
		SB->EditText=CopyStr(SB->EditText, Text);
		SB->EditLen=StrLen(SB->EditText);
		SB->EditCursor=SB->EditLen;
}


static void EditBarHandleInput(TStatusBar *SB, STREAM *Out, const char *Input, int Action)
{
int len;
char *Tempstr=NULL, *ptr;
ListNode *Node;

if (SB->EditCursor > SB->EditLen) SB->EditCursor=SB->EditLen;
switch (Action)
{
	case SB_EDIT_PREV:
		if (! SB->Curr) Node=ListGetLast(SB->Items);
		else Node=ListGetPrev(SB->Curr);

		if (Node) SB->Curr=Node;
		if (SB->Curr) EditBarSetText(SB, (const char *) SB->Curr->Item);
	break;
	
	case SB_EDIT_NEXT:
		Node=ListGetNext(SB->Curr);

		if (Node) SB->Curr=Node;
		if (SB->Curr) EditBarSetText(SB, (const char *) SB->Curr->Item);
	break;

	case SB_EDIT_LEFT:
		if (SB->EditCursor > 0) SB->EditCursor--;
	break;

	case SB_EDIT_RIGHT:
		if (SB->EditCursor < StrLen(SB->EditText)) SB->EditCursor++;
	break;

	case SB_EDIT_DELETE:
		if ((SB->EditLen > 0) && (SB->EditCursor < SB->EditLen))
		{
			SB->EditLen--;
			ptr=SB->EditText+SB->EditCursor;
			if (SB->EditCursor < SB->EditLen) memmove(ptr,ptr+1,StrLen(ptr+1)+1);
			else StrTrunc(SB->EditText, ptr - SB->EditText);
		}
	break;

	case SB_EDIT_CLEAR:
		EditBarSetText(SB, "");
	break;

	case SB_EDIT_BACKSPACE:
		if (SB->EditLen > 0) 
		{
			SB->EditLen--;
			SB->EditCursor--;
			ptr=SB->EditText+SB->EditCursor;
			if (SB->EditCursor < SB->EditLen) memmove(ptr,ptr+1,StrLen(ptr+1)+1);
			else StrTrunc(SB->EditText, ptr - SB->EditText);

		}
	break;

	case SB_EDIT_ENTER:
		StatusBarHide(SB);
		StatusBarAddHistory(SB, SB->EditText);
		FunctionCall(Out, SB->FuncName, SB->EditText, SB->EditLen);
		EditBarSetText(SB, "");
	break;

	default:
		if (StrLen(Input))
		{
			if (SB->EditCursor < SB->EditLen)
			{
			ptr=SB->EditText+SB->EditCursor;
			Tempstr=CopyStrLen(Tempstr,SB->EditText,ptr-SB->EditText);
			Tempstr=MCatStr(Tempstr,Input,ptr,NULL);
			SB->EditText=CopyStr(SB->EditText,Tempstr);
			}
			else SB->EditText=AddCharToStr(SB->EditText, Input[0]);
			SB->EditCursor++;
			SB->EditLen++;
		}
	break;
}

DrawStatusBar(SB);

Destroy(Tempstr);
}





int QueryBarHandleInput(TStatusBar *QB, STREAM *Out, const char *Input, const char *KeySym)
{
char *Tempstr=NULL, *FuncName=NULL;
int len, Action, result=FALSE;

Action=StatusBarParseKeySym(KeySym);

switch (Action)
{
	case SB_EDIT_CLOSE:
		StatusBarClose(QB);
		result=TRUE;
	break;

	case SB_EDIT_ENTER:
		FuncName=CopyStr(FuncName, QB->FuncName);		
		Tempstr=CopyStr(Tempstr, QB->EditText);	
		StatusBarClose(QB);
		FunctionCall(Out, FuncName, Tempstr, StrLen(Tempstr));
		UpdateStatusBars(TRUE);
		result=TRUE;
	break;

	default:
		EditBarHandleInput(QB, Out, Input, Action);
		DrawStatusBar(QB);
		result=TRUE;
	break;
}

return(result);
}



char *SelectionBarBuildText(char *RetStr, ListNode *List, ListNode *Selected, int WinWidth)
{
ListNode *Curr;
char *ptr;
int len;

RetStr=CopyStr(RetStr,"");
Curr=ListGetNext(List);
while (Curr)
{
if (Curr==Selected) 
{
	RetStr=MCatStr(RetStr, " [",Curr->Item, "] ",NULL);
	len=StrLen(RetStr);
	if (len > WinWidth)
	{
		ptr=RetStr+(len-WinWidth);
		memmove(RetStr,ptr,StrLen(ptr)+1);
	}
}
else RetStr=MCatStr(RetStr, "  ",Curr->Item, "  ",NULL);

Curr=ListGetNext(Curr);
}

if (StrLen(RetStr) > WinWidth) StrTrunc(RetStr, WinWidth);

return(RetStr);
}




int SelectBarHandleInput(TStatusBar *SB, STREAM *Out, const char *Input, const char *KeySym)
{
ListNode *Curr, *Node;
int len, result=FALSE, i;
char *Tempstr=NULL, *ptr;
TCrayon *SelectionList, *SelectionItem;

if (strcmp(KeySym, "left")==0) 
{
	Curr=ListGetPrev(SB->Curr);
	if (Curr && (Curr != SB->Items)) SB->Curr=Curr;
	SB->Text=SelectionBarBuildText(SB->Text, SB->Items, SB->Curr, ScreenCols);
	DrawStatusBar(SB);
	result=TRUE;
}
else if (strcmp(KeySym, "right")==0) 
{
	Curr=ListGetNext(SB->Curr);
	if (Curr) SB->Curr=Curr;
	SB->Text=SelectionBarBuildText(SB->Text, SB->Items, SB->Curr, ScreenCols);
	DrawStatusBar(SB);
	result=TRUE;
}
else if (strcmp(KeySym, "escape")==0) 
{
	StatusBarClose(SB);
	result=TRUE;
}
else if (strcmp(KeySym, "return")==0) 
{
	Curr=ListFindNamedItem(Selections, SB->EditText);
	Tempstr=CopyStr(Tempstr, SB->Curr->Item);
	len=StrLen(Tempstr);
	StatusBarClose(SB);

	SelectionList=(TCrayon *) Curr->Item;
	for (i=0; i < SelectionList->ActionCount; i++)
  {
		SelectionItem=&SelectionList->Actions[i];
		if (strcmp(SelectionItem->Match, Tempstr)==0)
    {
    	ApplyActions(Out, NULL, Tempstr, len, Tempstr, Tempstr+len, SelectionItem);
			break;
		}
  }

	result=TRUE;
}

Destroy(Tempstr);

return(result);
}



int StatusBarHandleInput(STREAM *Out, const char *Input, const char *KeySym)
{
TStatusBar *SB;
int result=FALSE;

if (GlobalFlags & FLAG_ALTERNATE_SCREEN) return(FALSE);
SB=StatusBarPopStack(g_BottomSB);
if (SB)
{
	switch (SB->Type)
	{
		case ACTION_SELECTBAR:
		result=SelectBarHandleInput(SB, Out, Input, KeySym);
		break;

		case ACTION_QUERYBAR:
		case ACTION_HISTORYBAR:
		result=QueryBarHandleInput(SB, Out, Input, KeySym);
		break;
	}
}

return(result);
}


int SetupStatusBars(TStatusBar *Top, TStatusBar *Bottom)
{
int TopMargin=0, BottomMargin=0, ScrollAreaEnd=0;

	StatusBarPushStack(Top, Bottom);
	StatusBarMargins(&TopMargin, &BottomMargin);
	ScrollAreaEnd=SetScrollRegion(TopMargin, BottomMargin);
	if (Bottom) Bottom->pos=ScrollAreaEnd+1;

	UpdateStatusBars(TRUE);

	return(ScrollAreaEnd);
}





TStatusBar *StatusBarCreate(int Type, TCrayon *Setup, const char *Text)
{
TStatusBar *SB;

SB=(TStatusBar *) calloc(1,sizeof(TStatusBar));
SB->len=1;

SB->Type=Type;
SB->Attribs=Setup->Attribs;
SB->Flags |= STATUSBAR_ACTIVE | STATUSBAR_EDIT | Setup->Flags;
if (Setup->Type==CRAYON_KEYPRESS) SB->Flags |= STATUSBAR_CLOSE_ON_NEW;
SB->Action=Setup;
SB->Text=CopyStr(SB->Text, Text);
SB->FuncName=CopyStr(SB->FuncName, Setup->String);
SB->RefreshInterval=Setup->Value;

Active=Setup;

return(SB);
}



int InfoBar(TCrayon *Action)
{
TStatusBar *QB;

QB=StatusBarCreate(ACTION_INFOBAR, Action, Action->Match);

if (Action->Flags & FLAG_TOP) SetupStatusBars(QB, NULL);
else SetupStatusBars(NULL, QB);
}



int QueryBar(TCrayon *Action)
{
TStatusBar *QB;
char *String=NULL;

String=MCopyStr(String, Action->Match, " %E", NULL);
QB=StatusBarCreate(ACTION_QUERYBAR, Action, String);
SetupStatusBars(NULL, QB);

Destroy(String);
return(TRUE);
}



int SelectionBar(TCrayon *Setup)
{
TStatusBar *SB;
char *Tempstr=NULL, *KeySym=NULL;
const char *ptr;
int WinWidth;

SB=StatusBarCreate(ACTION_SELECTBAR, Setup, "");
EditBarSetText(SB, Setup->String);

SB->Items=ListCreate();
ptr=GetToken(Setup->Match," ",&Tempstr,0);
while (ptr)
{
	ListAddItem(SB->Items,CopyStr(NULL,Tempstr));
	ptr=GetToken(ptr," ",&Tempstr,0);
}

SB->Curr=ListGetNext(SB->Items);
SB->Text=SelectionBarBuildText(SB->Text, SB->Items, SB->Curr, ScreenCols);
SetupStatusBars(NULL, SB);

Destroy(Tempstr);
Destroy(KeySym);

return(TRUE);
}



int HistoryBar(ListNode *Items, TCrayon *Action)
{
TStatusBar *HB;
ListNode *Curr;
char *String=NULL;

Curr=ListGetLast(Items);
if (Curr)
{
HB=StatusBarCreate(ACTION_HISTORYBAR, Action, "%E ");
EditBarSetText(HB, (const char *) Curr->Tag);
if (! HB->Items) HB->Items=ListCreate();

while (Curr)
{
	ListAddItem(HB->Items, CopyStr(NULL, Curr->Tag));
	Curr=ListGetPrev(Curr);
}

HB->Curr=ListGetNext(HB->Items);

SetupStatusBars(NULL, HB);
}

Destroy(String);
return(TRUE);
}


