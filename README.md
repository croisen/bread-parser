# Bread Parser

A naive implementation of a command line argument parser for c cli stuff.
I tried to make my own command line argument parser for c, but you only need the
header. Very wip

## Can maybe parse the following?

Try to open an issue if there's something bad happening...

```bash
# If an opt -c takes multiple args it can be written as
# Only comma separated args for an opt or just spam said opt with some args
./bread_parser_test -c 1,2,4
./bread_parser_test -c 1 -c 2 -c 4

# If an opt --verbose just doesn't have args
./bread_parser_test --verbose

# If an opt --problem-num has some args
./bread_parser_test --problem-num=1,2,4,5
./bread_parser_test --problem-num=1 --problem-num=2

# If you wish to chain multiple short opts together it's possible
# Though only the last opt in the chain(?) can take the argument
./bread_parser_test -ac 1

```

## Usage

In a c file you want to add this, define \_\_CROI_LIB_BREAD_PARSER_IMPL\_\_
only once and then include the bread_parser.h file, and hack away with your lsp
and with the example below to guide you

(sample)

```c
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

        /*
         * These printfs bout to cause a crash if the args passed to -c is less
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
```

## Compiling

Well it's a header only library so no other c files needed to be compiled for
this library to link to (maybe libc?). Or if you want to avoid the multiple
redefinition accidents by defining \_\_CROI_BREAD_PARSER_IMPL\_\_ multiple
times then use the bread_parser_pair.\* instead and compile the .c version and
link it to your main file

## Todo

-   [x] Be able to create command line opts
-   [x] Be able to have a long version of said opts
-   [x] Be able to add descriptions to said opts
-   [x] Parse argc and argv (incorrectly)
-   [x] Parse argc and argv (correctly) (I think I did it correctly now)
-   [x] Get pointers to the args of opts that have them
-   [x] After parsing, create a help opt using the current settings
-   [ ] Be able to parse how the arguments are shown in the help section like
        the stuff shown below (seems like a bad idea so no)

```bash
./bread_parser_test --some-opt='{1, "some str", -2}'
./bread_parser_test --some-opt=\{1, "some str", -2\}
./bread_parser_test --some-opt='{ 1, "some str", -2 }'
./bread_parser_test --some-opt=\{ 1, "some str", -2 \}
```

-   [ ] Add a list of args that was not received by the opts defined?
-   [x] Be able to parse the following

```bash
./bread_parser_test --some-opt=string1,"string 2",anyArg3
./bread_parser_test --some-opt string1,"string 2",anyArg3

# Suppose it's -c that takes an arg but it's not the last one used in the
# multi-opt short opts so it errors out? (Just trying to imitate the behavior of
# tar -xvf where it's -f that takes the arg)
./bread_parser_test -ca 1 # Errors
./bread_parser_test -ac 1 # Acts normally

# If -c takes multiple args or an indefinite number of args one can do
./bread_parser_test.c -c 1 -c 2 -c 3
```

-   [x] When specifying a definite amount of arguments for an option, do not
        pre-allocate the array for the receiver of the args
-   [x] Add a way to get the number of arguments passed into an option
-   [x] Make it accept only one type of arg even if it is not indefinite
