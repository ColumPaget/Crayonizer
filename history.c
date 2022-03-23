#include "history.h"
#include "status_bar.h"

static ListNode *History=NULL;
static int HistorySize=0;

void TypingHistoryActivate(int NoOfLines)
{
    if (! History) History=ListCreate();
    HistorySize=NoOfLines;
}


void TypingHistoryAddKeypress(const char *Chars, int Len)
{
    static char *Line=NULL;
    int i;

    if (! History) return;
    for (i=0; i < Len; i++)
    {
        if ((Chars[i]=='\n') || (Chars[i]=='\r'))
        {
            if (StrValid(Line)) ListAddNamedItem(History, Line, NULL);
            Line=CopyStr(Line, "");
        }
        else Line=AddCharToStr(Line, Chars[i]);
    }

//Don't do this, 'Line' is a static!
//Destroy(Line);
}


void TypingHistoryPopup(TCrayon *Crayon)
{
    if (History) HistoryBar(History, Crayon);
}
