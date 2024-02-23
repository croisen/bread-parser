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

In a c file you want to add this, define CROI_LIB_BREAD_PARSER_IMPL_H only once
and then include the bread_parser.h file, and hack away with your lsp and with
the example below to guide you

```c
#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(int argc, char **argv)
{
    // Add options (the -s or --something), descriptions
    bread_parser_add_option('o', "option", 0);
    bread_parser_add_descrp('o', "Some option");

    bread_parser_add_option('v', "verbose", 0);
    bread_parser_add_descrp('v', "Make verbose something something");

    bread_parser_add_option('p', "problem-num", 0);
    bread_parser_add_descrp('p', "Some problem num");

    // Define if the opt has arguments, and what types it has
    // Must be in order and if the user of your program supplies
    // less than the number of arg count specified, or supplies
    // argument types that are not in the left to right order
    // defined here. Well the bread_parser will exit
    bread_parser_opt_argmts('p', 3, BREAD_I64, BREAD_CHAR, BREAD_U64);

    // Debug print all opts, descriptions, and their args
    bread_print_args();

    // Start parsing
    bread_parse(argc, argv);

    // Check if an opt is used
    if (bread_parser_is_opt_used('o'))
    {
        printf("-o is used\n");
    }
    else
    {
        printf("-o is not used\n");
    }

    // Get args of said option if any, returns null if option has no arg count
    void **problem_num_args = bread_parser_get_all_args('p');
    return 0;
}
```

## Compiling

Well it's a header only library so no other c files needed to be compiled for
this library to link to
