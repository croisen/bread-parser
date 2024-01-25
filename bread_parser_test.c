#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(int argc, char **argv)
{
    bread_parser_add_option('o', "option", 0);
    bread_parser_add_descrp('o', "Some option");

    bread_parser_add_option('v', "verbose", 0);
    bread_parser_add_descrp('v', "Make verbose something something");

    bread_parser_add_option('p', "problem-num", 0);
    bread_parser_add_descrp('p', "Some problem num");
    bread_parser_opt_argmts('p', 3, BREAD_I64, BREAD_CHAR, BREAD_U64);

    bread_print_args();
    bread_parse(argc, argv);
    return 0;
}
