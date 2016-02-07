#include "timers.h"

ListNode *Timers=NULL;



void UpdateTimer(TCrayon *Timer)
{
char *ptr=NULL;

Timer->Value=(float) Now;
Timer->Value+=strtof(Timer->Match,&ptr);
if (ptr)
{
	switch (*ptr)
	{
		case 'm': Timer->Value *= 60.0; break;
		case 'h': Timer->Value *= 3600.0; break;
	}
}

}

void ParseTimer(const char *Config)
{
TCrayon *Crayon;

	if (! Timers) Timers=ListCreate();
	Crayon=(TCrayon *) calloc(1,sizeof(TCrayon));
	ParseCrayonEntry(Crayon,"timer", Config);
	UpdateTimer(Crayon);
	ListAddNamedItem(Timers, Crayon->Match, Crayon);
}


void ProcessTimers(STREAM *Pipe)
{
ListNode *Curr;
TCrayon *Timer;

Curr=ListGetNext(Timers);
while (Curr)
{
	Timer=(TCrayon *) Curr->Item;
	if (Timer->Value < (float) Now) 
	{
		UpdateTimer(Timer);
		ProcessCrayonization(Pipe, NULL, 0, NULL, Timer);
	}
	Curr=ListGetNext(Curr);
}

}
