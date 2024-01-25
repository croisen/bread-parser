#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(void)
{
    bread_parser_add_option('o', "option", 0, 0);
    bread_parser_add_option('v', "verbose", 0, 0);

    bread_parser_add_descr('o', "Some option");
    bread_parser_add_descr('v', "Make verbose something something");

    bread_print_args();
    return 0;
}
