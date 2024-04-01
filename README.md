# Bread Parser

A bloated command line argument parser for c.
I tried to make my own command line argument parser for c, but you only need the
header. Very wip

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

## Usage

In a c file you want to add this, define \_\_CROI_LIB_BREAD_PARSER_IMPL\_\_ only
once and then include the bread_parser.h file, and hack away with your lsp and
with the example below to guide you

```c
#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(void)
{
    printf("Rework in progress...\n");
    return 0;
}
```

## Compiling

Well it's a header only library so no other c files needed to be compiled for
this library to link to (maybe libc)
