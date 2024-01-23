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


void CommandLineAddSubstitution(int Type, const char *Match, const char *Substitution)
{
    if (! CmdLineSubs) CmdLineSubs=ListCreate();
    ListAddTypedItem(CmdLineSubs, Type, Match, CopyStr(NULL, Substitution));
}



static char *CommandLineSubstituteItem(char *RetStr, int SubsType, const char *Input, const char *Substitute, const char *MatchStart, const char *MatchEnd)
{
    char *Tempstr=NULL;
    const char *ptr;

    //Tempstr now holds the text to substitute into the command-line
    Tempstr=SubstituteVarsInString(Tempstr, Substitute, Vars, 0);

		//only process if we found a match
		if (MatchStart)
		{
    switch (SubsType)
    {
    //replace matching text with the substitution
    case CMDLINE_SUB:
        //copy up to match
        RetStr=CopyStrLen(RetStr, Input, MatchStart - Input);
        //append substition
        RetStr=CatStr(RetStr, Tempstr);
        //copy from end of match
        RetStr=CatStr(RetStr, MatchEnd);
        break;

    //insert substitution between ProgName and the rest of the command line
    case CMDLINE_INSERT:
        //extract program name/path
        ptr=GetToken(Input, "\\S", &RetStr, GETTOKEN_QUOTES);

        //insert substitution after program name
        RetStr=MCatStr(RetStr, " ", Tempstr, NULL);

        //copy up to substituted string
        if (MatchStart) RetStr=CatStrLen(RetStr,ptr, MatchStart - ptr);
        else RetStr=CatStr(RetStr, ptr);

        //clip out substituted string by jumping over it
        if (MatchEnd) RetStr=MCatStr(RetStr, " ", MatchEnd, NULL);
        break;

    //append substitution to end of command line
    case CMDLINE_APPEND:
        //copy up to match
        if (MatchStart) RetStr=CopyStrLen(RetStr,Input, MatchStart - Input);
        else RetStr=CatStr(RetStr, Input);

        //clip out substituted string by jumping over it
        if (MatchEnd) RetStr=MCatStr(RetStr, " ", MatchEnd, NULL);
        else RetStr=CatStr(RetStr, " ");

        //append substition
        RetStr=CatStr(RetStr, Tempstr);
        break;
    }
		}

    Destroy(Tempstr);
    return(RetStr);
}


static char *CommandLineSubstitute(char *RetStr, int SubsType, const char *Pattern, const char *Substitute, const char *Input)
{
    ListNode *Matches, *Curr;
    TPMatch *Match;
    char *Tempstr=NULL;
    const char *ptr;

    Matches=ListCreate();
    if (pmatch(Pattern, Input, StrLen(Input), Matches, PMATCH_SUBSTR ))
    {
        Curr=ListGetNext(Matches);
        Match=(TPMatch *) Curr->Item;

        Tempstr=CopyStrLen(Tempstr, Match->Start, Match->End - Match->Start);
        SetVar(Vars, "match", Tempstr);

        RetStr=CommandLineSubstituteItem(RetStr, SubsType, Input, Substitute, Match->Start, Match->End);
    }
    else RetStr=CopyStr(RetStr, Input);

    ListDestroy(Matches, Destroy);
    Destroy(Tempstr);
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
        if (! StrValid(Curr->Tag)) Tempstr=CommandLineSubstituteItem(Tempstr, Curr->ItemType, CommandLine, (const char *) Curr->Item, NULL, NULL);
        else Tempstr=CommandLineSubstitute(Tempstr, Curr->ItemType, Curr->Tag, (const char *) Curr->Item, CommandLine);

        //Must use Tempstr as a buffer so that we don't wind up copying from CommandLine into itself (snake eating its own tail)
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



char *RebuildCommandLine(char *CommandLine, int argc, char *argv[], const char *CrayonizerDir)
{
    char *Tempstr=NULL, *Path=NULL;
    const char *ptr;
    int i;


    ptr=GetVar(Vars, "ReplaceCommand");

    if (StrValid(ptr)) return(CopyStr(CommandLine, ptr));

    Path=RebuildPath(Path, getenv("PATH"), CrayonizerDir);
    Tempstr=FindFileInPath(Tempstr, argv[0], Path);

    if (! StrValid(Tempstr))
    {
        fprintf(stderr, "ERROR: Cannot locate program '%s'\n", argv[0]);
        exit(1);
    }



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

