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

    // Should trigger a warning?
    bParserAddDesc(BParserNoShortOpt, NULL, "I trigger an error bud");

    bParserPrintOpts();
    return 0;
}
