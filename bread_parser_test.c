#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define __CROI_BREAD_PARSER_IMPL__
#include "bread_parser.h"

void initOpts(void);

int main(int argc, char **argv)
{
    initOpts();
    bParserParse(argc, argv);

    if (bParserWasOptUsed('c', NULL)) {
        printf("-c was used!\n");
        void **x = bParserGetArgs('c', NULL);

        /* These printfs bout to cause a crash if the args passed to -c is less
         * than 3
         */
        printf("%" PRIu64 "\n", *((uint64_t *)x[0]));
        printf("%" PRIu64 "\n", *((uint64_t *)x[1]));
        printf("%s\n", (char *)x[2]);
    }

    return 0;
}

void initOpts(void)
{
    bParserSetProgramName("bread_parser");
    bParserSetProgramVersion("2.0.1");
    bParserSetAuthorName("croisen");
    bParserSetAuthorEmail("andrewjames.git.gan@gmail.com");

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
        "I need a long description so; lorem ipsum dolor sit amet, consectetur "
        "adipisicing elit, sed do eiusmod tempor incididunt ut labore et "
        "dolore magna aliqua. Ut enim ad minim veniam, quis nostrud "
        "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat. "
        "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum "
        "dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat "
        "non proident, sunt in culpa qui officia deserunt mollit anim id est "
        "laborum."
    );
    bParserAddArgs('d', NULL, 3, U64BP, U32BP, ANYBP);

    bParserAddOpts('e', "echo", 0);
    bParserAddArgs('e', NULL, 3, ANYBP, ANYBP, ANYBP);

    // Should trigger a warning?
    bParserAddDesc(BParserNoShortOpt, NULL, "I trigger an error bud");
}
