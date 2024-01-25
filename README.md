# Bread Parser

Tried to make my own command line argument parser for c, but you only need the
header. Very wip

# Usage

```c
#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(int argc, char **argv)
{
    // Add option and description
    bread_parser_add_option('o', "option", 0, 0);
    bread_parser_add_descr('o', "Some option");

    // Debug purpose or an additional use for the help function in the future
    bread_print_args();
    return 0;
}

```

# Compiling

Well it's a header only library so no other c files needed to be compiled for
this library to link to
