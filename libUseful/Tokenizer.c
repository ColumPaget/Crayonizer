#include "includes.h"
#include "Tokenizer.h"
#include "String.h"

#define TOK_SPACE 1
#define TOK_CODE  2


//Does the current position match against Pattern
int GetTokenSepMatch(const char *Pattern,const char **start, const char **end, int Flags)
{
    const char *pptr, *eptr;
    int MatchType=0;

//if start and end pointers are same, then we've no string
    if (*start==*end) return(FALSE);

    pptr=Pattern;
    eptr=*start;


    while (1)
    {
//check the current 'pattern' char
        switch (*pptr)
        {
        //if we run out of pattern, then we got a match
        case '\0':
            *end=eptr;
            return(TRUE);
            break;


        //Quoted char
        case '\\':
            pptr++;
            if (*pptr=='S') MatchType=TOK_SPACE;
            if (*pptr=='X') MatchType=TOK_CODE;
            break;

        }

        switch (*eptr)
        {
        //if we run out of string, then we got a part match, but its not
        //a full match, so we return fail
        case '\0':
            *start=eptr;
            *end=eptr;
            return(FALSE);
            break;

        case '\\':
            //if we got a quoted character we can't have found
            //the separator, so return false
            if (Flags & GETTOKEN_BACKSLASH)
            {
                if (*eptr != *pptr) return(FALSE);
            }
            else
            {
                eptr++;
                *start=eptr;
                return(FALSE);
            }
            break;

        case '"':
        case '\'':
            if (Flags & GETTOKEN_HONOR_QUOTES)
            {
                eptr=traverse_quoted(eptr);
                if (*eptr == '\0') eptr--;	//because there's a ++ going to happen when we return
                *start=eptr;
                return(FALSE);
            }
            else if (*eptr != *pptr) return(FALSE);
            break;

        case ' ':
        case '	':
        case '\n':
        case '\r':
            if ((MatchType==TOK_SPACE) || (MatchType==TOK_CODE))
            {
                while (isspace(*eptr)) eptr++;
                eptr--;
                MatchType=0;
            }
            else if (*eptr != *pptr) return(FALSE);
            break;

        case '(':
        case ')':
        case '=':
        case '!':
        case '<':
        case '>':
            if (MatchType==TOK_CODE) MatchType=0;
            else if (*eptr != *pptr) return(FALSE);
            break;

        default:
            if (MatchType != 0) return(FALSE);
            if (*eptr != *pptr) return(FALSE);
            break;
        }

        pptr++;
        eptr++;
    }

    return(FALSE);
}





//Searches through 'String' for a match of a Pattern
int GetTokenFindSeparator(const char *Pattern, const char *String, const char **SepStart, const char **SepEnd, int Flags)
{
    const char *start_ptr=NULL, *end_ptr=NULL, *ptr;

    start_ptr=String;
    while (*start_ptr != '\0')
    {
        if ((*start_ptr=='\\') && (! (Flags & GETTOKEN_BACKSLASH)))
        {
            start_ptr++;
            start_ptr++;
            continue;
        }

        if (GetTokenSepMatch(Pattern,&start_ptr, &end_ptr, Flags))
        {
            *SepStart=start_ptr;
            *SepEnd=end_ptr;
            return(TRUE);
        }
        if ((*start_ptr) !='\0') start_ptr++;
    }

//We found nothing, set sep start to equal end of string
    *SepStart=start_ptr;
    *SepEnd=start_ptr;

    return(FALSE);
}

char **BuildMultiSeparators(const char *Pattern)
{
    const char *ptr, *next;
    int count=0;
    char **separators;

    ptr=strchr(Pattern, '|');
    while (ptr)
    {
        count++;
        ptr++;
        ptr=strchr(ptr,'|');
    }

//count + 2 because last item will lack a '|' and  we want a NULL at the end
//of the separator array
    separators=(char **) calloc(count+2,sizeof(char *));
    ptr=Pattern;
    count=0;
    while (ptr && (*ptr !='\0'))
    {
        while (*ptr=='|') ptr++;
        if (*ptr!='\0')
        {
            next=strchr(ptr,'|');
            if (next) separators[count]=CopyStrLen(NULL, ptr, next-ptr);
            else separators[count]=CopyStr(NULL, ptr);
            count++;
            ptr=next;
        }
    }

    return(separators);
}



int GetTokenMultiSepMatch(char **Separators, const char **start_ptr, const char **end_ptr, int Flags)
{
    char **ptr;
    const char *sptr, *eptr;

    ptr=Separators;
    while (*ptr !=NULL)
    {
        //must do this as GetTokenSepMatch moves these pointers on, and that'll cause problems
        //if one of our separators fails to match part way through
        sptr=*start_ptr;
        eptr=*end_ptr;

				if (GetTokenFindSeparator(*ptr, *start_ptr, &sptr, &eptr, Flags))
        {
            *start_ptr=sptr;
            *end_ptr=eptr;
            return(TRUE);
        }
        ptr++;
    }

    *start_ptr=*end_ptr;

    return(FALSE);
}




//Once we've found our token we need to do various cleanups and post processing on it
const char *GetTokenPostProcess(const char *SearchStr, const char *SepStart, const char *SepEnd, char **Token, int Flags)
{
    const char *sptr, *eptr;

		if (! SepStart)
		{
			*Token=CopyStr(*Token, SearchStr);
			return(SearchStr+StrLen(SearchStr));
		}

    sptr=SearchStr;
    if (Flags & GETTOKEN_INCLUDE_SEP)
    {
        if (SepStart==SearchStr) eptr=SepEnd;
        else
        {
            eptr=SepStart;
            SepEnd=SepStart;
        }
    }
    else if (Flags & GETTOKEN_APPEND_SEP) eptr=SepEnd;
    else if (SepStart) eptr=SepStart;
    else eptr=SepEnd;

    if (Flags & GETTOKEN_STRIP_QUOTES)
    {
        if ((*sptr=='"') || (*sptr=='\''))
        {
            //is character before the sep a quote? If so, we copy one less char, and also start one character later
            //else we copy the characters as well
            eptr--;
            if (*sptr==*eptr) sptr++;
            else eptr++;
        }
    }

    if (eptr >= sptr) *Token=CopyStrLen(*Token, sptr, eptr-sptr);
    else *Token=CopyStr(*Token, sptr);

    if (Flags & GETTOKEN_STRIP_SPACE)
    {
        StripTrailingWhitespace(*Token);
        StripLeadingWhitespace(*Token);
    }

//return empty string, but not null
    if ((! SepEnd) || (*SepEnd=='\0'))
    {
        SepEnd=SearchStr+StrLen((char *) SearchStr);
    }

    return(SepEnd);
}



const char *GetTokenSeparators(const char *SearchStr, char **Separators, char **Token, int Flags)
{
    const char *SepStart=NULL, *SepEnd=NULL;

    /* this is a safety measure so that there is always something in Token*/
    if (Token) *Token=CopyStr(*Token,"");
    if ((! Token) || StrEnd(SearchStr)) return(NULL);
		SepStart=SearchStr;
		GetTokenMultiSepMatch(Separators, &SepStart, &SepEnd, Flags);
    return(GetTokenPostProcess(SearchStr, SepStart, SepEnd, Token, Flags));
}


const char *GetToken(const char *SearchStr, const char *Separator, char **Token, int Flags)
{
    const char *SepStart=NULL, *SepEnd=NULL;
    char **separators;
    int len;

    if (! Token) return(NULL);
    if (StrEnd(SearchStr))
    {
        *Token=CopyStr(*Token,"");
        return(NULL);
    }

    if (! StrValid(Separator))
    {
        *Token=CopyStr(*Token,SearchStr);
        return(NULL);
    }

//if we've only got one character than just keep it simple
    if (
        (! (Flags & GETTOKEN_HONOR_QUOTES)) &&
        (*(Separator+1)=='\0')
    )
    {
        SepStart=strchr(SearchStr,*Separator);
        if (! SepStart) SepStart=SearchStr+StrLen(SearchStr);
        if (*SepStart=='\0') SepEnd=SepStart;
        else SepEnd=SepStart+1;
        if (Flags) return(GetTokenPostProcess(SearchStr, SepStart, SepEnd, Token, Flags));
        *Token=CopyStrLen(*Token, SearchStr, SepStart-SearchStr);
        return(SepEnd);
    }
    else
    {
//right, now we're committed to doing a complex search
        *Token=CopyStr(*Token,"");


        if (Flags & GETTOKEN_MULTI_SEPARATORS)
        {
            separators=BuildMultiSeparators(Separator);
						SepStart=SearchStr;
						GetTokenMultiSepMatch(separators, &SepStart, &SepEnd, Flags);
        }
        else GetTokenFindSeparator(Separator, SearchStr, &SepStart, &SepEnd, Flags);
    }

    return(GetTokenPostProcess(SearchStr, SepStart, SepEnd, Token, Flags));
}








int GetTokenParseConfig(const char *Config)
{
    const char *ptr;
    int Flags=0;

    for (ptr=Config; *ptr != '\0'; ptr++)
    {
        switch (*ptr)
        {
        case 'm':
            Flags |= GETTOKEN_MULTI_SEPARATORS;
            break;
        case 'q':
            Flags |= GETTOKEN_HONOR_QUOTES;
            break;
        case 'Q':
            Flags |= GETTOKEN_QUOTES;
            break;
        case 's':
            Flags |= GETTOKEN_INCLUDE_SEPARATORS;
            break;
        case '+':
            Flags |= GETTOKEN_APPEND_SEPARATORS;
            break;
        }
    }

    return(Flags);
}


const char *GetNameValuePair(const char *Input, const char *PairDelim, const char *NameValueDelim, char **Name, char **Value)
{
    const char *ptr, *ptr2;
    char *Token=NULL;

    *Name=CopyStr(*Name,"");
    *Value=CopyStr(*Value,"");
    ptr=GetToken(Input,PairDelim,&Token,GETTOKEN_HONOR_QUOTES);
    if (StrValid(Token))
    {
        if ((Token[0]=='"') || (Token[0]=='\''))
        {
           // StripQuotes(Token);
        }
        ptr2=GetToken(Token,NameValueDelim,Name,GETTOKEN_QUOTES);
//ptr2=GetToken(ptr2,PairDelim,Value,GETTOKEN_HONOR_QUOTES);
        *Value=CopyStr(*Value,ptr2);

        StripQuotes(*Name);
        StripQuotes(*Value);
    }

    DestroyString(Token);
    return(ptr);
}

