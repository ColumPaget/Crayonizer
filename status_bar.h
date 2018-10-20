#ifndef CRAYONIZER_STATUS_BARS
#define CRAYONIZER_STATUS_BARS

#include "common.h"


#define SB_EDIT_PREV 1
#define SB_EDIT_NEXT 2
#define SB_EDIT_LEFT 3
#define SB_EDIT_RIGHT 4
#define SB_EDIT_DELETE 5
#define SB_EDIT_BACKSPACE 6
#define SB_EDIT_CLEAR 7
#define SB_EDIT_ENTER 9
#define SB_EDIT_OFF 10
#define SB_EDIT_ON 11
#define SB_EDIT_TOGGLE 12
#define SB_EDIT_CLOSE 13

typedef struct
{
int Type;
int Flags;
int pos;
int len;
char *Text;
int Attribs;
char *EditText;
int EditLen;
int EditCursor;
int RefreshInterval;
time_t NextRefresh;
char *FuncName;
ListNode *Items;
ListNode *Curr;
TCrayon *Action;
} TStatusBar;


TCrayon *StatusBarGetActive();
void StatusBarCloseActive();
int StatusBarHandleInput(STREAM *Out, const char *Input, const char *KeySym);
void StatusBarParseSelection(STREAM *S, const char *Name);
//TStatusBar *StatusBarCreate(int Type, int Attribs, const char *Text);
void StatusBarDestroy(TStatusBar *);
//TStatusBar *ParseStatusBar(const char *Data);
int SetupStatusBars(TStatusBar *Top, TStatusBar *Bottom);
void UpdateStatusBars(int ForceUpdate);

int InfoBar(TCrayon *Setup);
int QueryBar(TCrayon *Setup);
int SelectionBar(TCrayon *Setup);
int HistoryBar(ListNode *Items, TCrayon *Action);

#endif
