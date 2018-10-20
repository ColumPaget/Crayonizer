#include "libUseful.h"

pmatch_test(const char *Pattern, const char *Str)
{
int result;
ListNode *Matches, *Curr;
char *Tempstr=NULL;
TPMatch *Match;

  Matches=ListCreate();
  result=pmatch(Pattern, Str, StrLen(Str), Matches, PMATCH_SUBSTR);

  if (result > 0)
  {
    Curr=ListGetNext(Matches);
    while (Curr)
    {
      Match=(TPMatch *) Curr->Item;
			Tempstr=CopyStrLen(Tempstr, Match->Start, Match->End-Match->Start);
			printf("M: [%s] [%s]\n",Pattern,Tempstr);
    Curr=ListGetNext(Curr);
    }
  }
	printf("\n");
 ListDestroy(Matches,Destroy);
 Destroy(Tempstr);
}


main()
{
 pmatch_test("\\D*.\\D*.\\D*.\\D*[ \\t\\n]", "     inet addr:127.0.0.1  Bcast:0.0.0.0  Mask:255.0.0.0");
 pmatch_test("\\-O\\D*.\\D*.\\D*.\\D*[ \\t\\n\\0]", "     inet addr:127.0.0.1  Bcast:0.0.0.0  Mask:255.0.0.0");
}
