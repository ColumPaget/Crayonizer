#include "command_line.h"
#include "crayonizations.h"

static ListNode *CmdLineSubs=NULL;


static char *RebuildPath(char *RetStr, char *Path, const char *CrayonizerDir)
{
char *NewPath=NULL, *Token=NULL;
const char *ptr;

NewPath=CopyStr(RetStr,"");
ptr=GetToken(Path,":",&Token,0);
while (ptr)
{
  if (strcmp(Token,CrayonizerDir) !=0) NewPath=MCatStr(NewPath,Token,":",NULL);
  ptr=GetToken(ptr,":",&Token,0);
}

Destroy(Token);
return(NewPath);
}


static char *CommandLineSubstitute(char *RetStr, const char *Pattern, const char *Substitute, const char *Input)
{
ListNode *Matches, *Curr;
TPMatch *Match;

Matches=ListCreate();
if (pmatch(Pattern, Input, StrLen(Input), Matches, PMATCH_SUBSTR ))
{
	Curr=ListGetNext(Matches);
	Match=(TPMatch *) Curr->Item;
	RetStr=CopyStrLen(RetStr,Input, Match->Start - Input);
	RetStr=CatStr(RetStr, Substitute);
	RetStr=CatStr(RetStr, Match->End);
}
else RetStr=CopyStr(RetStr, Input);

ListDestroy(Matches, Destroy);
return(RetStr);
}


static char *CommandLineProcessSubstitutions(char *CommandLine, const char *Input)
{
ListNode *Curr;
char *Tempstr=NULL;

CommandLine=CopyStr(CommandLine, Input);
Curr=ListGetNext(CmdLineSubs);
while (Curr)
{
if ((! StrValid(Curr->Tag)) || (strcmp(Curr->Tag, "$")==0)) Tempstr=MCopyStr(Tempstr, CommandLine, Curr->Item, NULL);
else Tempstr=CommandLineSubstitute(Tempstr, Curr->Tag, (const char *) Curr->Item, CommandLine);
CommandLine=CopyStr(CommandLine, Tempstr);
Curr=ListGetNext(Curr);
}

Destroy(Tempstr);

return(CommandLine);
}



void ProcessCmdLine(char *CmdLine, ListNode *Crayons)
{
TCrayon *Item;
ListNode *Curr;

Curr=ListGetNext(Crayons);
while (Curr)
{
	Item=(TCrayon *) Curr->Item;
	if (Item->Type==CRAYON_CMDLINE)
	{
		 ColorSubstring(NULL, NULL, CmdLine, StrLen(CmdLine), Item);
	}
	Curr=ListGetNext(Curr);
}

}	


void CommandLineAddSubstitution(const char *Match, const char *Substitution)
{
if (! CmdLineSubs) CmdLineSubs=ListCreate();
ListAddNamedItem(CmdLineSubs, Match, CopyStr(NULL, Substitution));
}


char *RebuildCommandLine(char *CommandLine, int argc, char *argv[], const char *CrayonizerDir)
{
char *Tempstr=NULL, *Path=NULL;
const char *ptr;
int i;


ptr=GetVar(Vars, "ReplaceCommand");
if (StrValid(ptr)) return(CopyStr(CommandLine, ptr));

Path=RebuildPath(Path, getenv("PATH"), CrayonizerDir);
Tempstr=FindFileInPath(Tempstr, argv[0], Path);

//CommandLine=MCatStr(CommandLine, " ",GetVar(Vars,"ExtraCmdLineOptions")," ",NULL);


Tempstr=CatStr(Tempstr, " ");
for (i=1; i < argc; i++)
{
	if (strchr(argv[i], ' ')==0) Tempstr=MCatStr(Tempstr,argv[i]," ",NULL);
	else Tempstr=MCatStr(Tempstr,"'",argv[i],"' ",NULL);
}

CommandLine=CommandLineProcessSubstitutions(CommandLine, Tempstr);
StripTrailingWhitespace(CommandLine);

Destroy(Tempstr);
return(CommandLine);
}

