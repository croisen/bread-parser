#define __CROI_BREAD_PARSER_IMPL__
#include "bread_parser.h"

int main(void)
{
    bParserAddOpts('a', "alpha", 0);
    bParserAddDesc('a', "alpha", "Alpha description or something");

    bParserAddOpts(BParserNoShortOpt, "beta", 0);
    bParserAddDesc(BParserNoShortOpt, "beta", "Beta description or something");

    bParserAddOpts('c', NULL, 0);
    bParserAddDesc('c', NULL, "Charlie description or something");
    bParserAddArgs('c', NULL, 3, U64BP, U32BP, ANYBP);

    bParserAddOpts('d', NULL, 0);
    bParserAddDesc(
        'd', NULL,
        "Long description so Lorem ipsum dolor sit amet, consectetur "
        "adipisicing elit, sed do eiusmod tempor incididunt ut labore et "
        "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
        "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum "
        "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
        "non proident, sunt in culpa qui officia deserunt mollit anim id est "
        "laborum.");
    bParserAddArgs('d', NULL, 3, U64BP, U32BP, ANYBP);

    bParserAddOpts('e', "echo", 0);
    bParserAddArgs('e', NULL, 3, ANYBP, ANYBP, ANYBP);

    // Should trigger a warning?
    bParserAddDesc(BParserNoShortOpt, NULL, "I trigger an error bud");

    bParserPrintOpts();
    return 0;
}
