#include <stdio.h>
#define CROI_LIB_BREAD_PARSER_IMPL_H
#include "bread_parser.h"

int main(int argc, char **argv)
{
    bread_parser_set_program_name("bread_parser_test");
    bread_parser_set_author_name("croisen");
    bread_parser_set_author_email("andrewjames.git.gan@gmail.com");

    bread_parser_add_option('o', "option", 0);
    bread_parser_add_descrp('o', "Some option");

    bread_parser_add_option('v', "verbose", 0);
    bread_parser_add_descrp('v', "Make verbose something something");

    bread_parser_add_option('p', "problem-num", 0);
    bread_parser_add_descrp('p', "Some problem num");
    bread_parser_opt_argmts('p', 3, BREAD_I64, BREAD_CHAR, BREAD_U64);

    bread_parse(argc, argv);

    if (bread_parser_is_opt_used('o'))
    {
        printf("-o is used\n");
    }
    else
    {
        printf("-o is not used\n");
    }

    void **problem_num_args = bread_parser_get_all_args('p');
    if (problem_num_args != NULL)
    {
        printf("Problem args #%lu: %ld\n", 1L, *(long *)problem_num_args[0]);
        printf("Problem args #%lu: %s\n", 2L, (char *)problem_num_args[1]);
        printf("Problem args #%lu: %lu\n", 3L,
               *(unsigned long *)problem_num_args[2]);
    }

    return 0;
}
