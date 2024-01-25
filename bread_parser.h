#ifndef CROI_LIB_BREAD_PARSER_H
#define CROI_LIB_BREAD_PARSER_H

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

enum __arg_type
{
    BREAD_I64,
    BREAD_U64,
    BREAD_CHAR,
};

struct __arg
{
    bool used;

    size_t arg_count;
    size_t group_num;

    char short_opt;
    char *long_opt;
    char *descr;

    enum __arg_type *arg_type_list;
    void **args;
};

struct __darr_for_ptr
{
    bool init;
    size_t size;
    size_t used;
    void **ptrs;
};

typedef struct __arg *ArgPtr;

typedef struct __darr_for_ptr DA[1];

void bread_print_args(void);
void bread_parse(int argc, char **argv);

bool bread_parser_is_opt_used(char short_opt);
void **bread_parser_get_all_args(char short_opt);
void *bread_parser_get_arg(char short_opt, size_t index);

void bread_parser_add_option(char short_opt, char *long_opt, size_t group);
void bread_parser_add_descrp(char short_opt, char *description);
void bread_parser_opt_argmts(char short_opt, size_t arg_count, ...);

void __memtracker_init();
void __memtracker_free();

void __bread_panic(const char *message, ...);
void *__bread_calloc(size_t nmemb, size_t size);
void *__bread_malloc(size_t size);
void *__bread_realloc(void *ptr, size_t size);

#endif

#ifdef CROI_LIB_BREAD_PARSER_IMPL_H

DA some_args    = {0};
DA alloced_ptrs = {0};

#define bread_panic(message, ...)            \
    fprintf(stderr, message, ##__VA_ARGS__); \
    exit(EXIT_FAILURE)

int bread_args_compare(const void *a, const void *b)
{
    ArgPtr c = *(ArgPtr *)a;
    ArgPtr d = *(ArgPtr *)b;

    return c->group_num - d->group_num;
}

void __memtracker_init()
{
    atexit(__memtracker_free);
    alloced_ptrs->init = true;
    alloced_ptrs->size = 4;
    alloced_ptrs->used = 0;

    alloced_ptrs->ptrs = calloc(alloced_ptrs->size, sizeof(void *));
    if (alloced_ptrs->ptrs == NULL)
    {
        bread_panic(
            "Cannot initialize memory tracker for the bread parser, calloc "
            "returned NULL\n");
    }
}

void __memtracker_free()
{
    for (size_t i = 0; i < alloced_ptrs->used; i += 1)
    {
        free(alloced_ptrs->ptrs[i]);
    }

    free(alloced_ptrs->ptrs);
}

void *bread_calloc(size_t nmemb, size_t size)
{
    if (!alloced_ptrs->init)
    {
        __memtracker_init();
    }

    void *res = calloc(nmemb, size);
    if (alloced_ptrs->size <= (alloced_ptrs->used + 1))
    {
        void **new_arr = realloc(alloced_ptrs->ptrs,
                                 sizeof(void *) * (alloced_ptrs->size * 2));
        if (new_arr == NULL)
        {
            free(res);
            bread_panic("Cannot track new memory given by bread_calloc, "
                        "realloc return NULL\n");
        }

        for (size_t i  = (alloced_ptrs->size - 1); i < (alloced_ptrs->size * 2);
             i        += 1)
        {
            new_arr[i] = NULL;
        }

        alloced_ptrs->ptrs  = new_arr;
        alloced_ptrs->size *= 2;
    }

    if (res != NULL)
    {
        alloced_ptrs->ptrs[alloced_ptrs->used]  = res;
        alloced_ptrs->used                     += 1;
    }

    return res;
}

void *bread_malloc(size_t size)
{
    if (!alloced_ptrs->init)
    {
        __memtracker_init();
    }

    void *res = malloc(size);
    if (alloced_ptrs->size <= (alloced_ptrs->used + 1))
    {
        void **new_arr = realloc(alloced_ptrs->ptrs,
                                 sizeof(void *) * (alloced_ptrs->size * 2));
        if (new_arr == NULL)
        {
            free(res);
            bread_panic("Cannot track new memory given by bread_malloc, "
                        "realloc return NULL\n");
        }

        for (size_t i  = (alloced_ptrs->size - 1); i < (alloced_ptrs->size * 2);
             i        += 1)
        {
            new_arr[i] = NULL;
        }

        alloced_ptrs->ptrs  = new_arr;
        alloced_ptrs->size *= 2;
    }

    if (res != NULL)
    {
        alloced_ptrs->ptrs[alloced_ptrs->used]  = res;
        alloced_ptrs->used                     += 1;
    }

    return res;
}

void *bread_realloc(void *ptr, size_t size)
{
    if (!alloced_ptrs->init)
    {
        __memtracker_init();
    }

    void *res = realloc(ptr, size);
    if (alloced_ptrs->size <= (alloced_ptrs->used + 1))
    {
        void **new_arr = realloc(alloced_ptrs->ptrs,
                                 sizeof(void *) * (alloced_ptrs->size * 2));
        if (new_arr == NULL)
        {
            free(res);
            bread_panic("Cannot track new memory given by bread_malloc, "
                        "realloc return NULL\n");
        }

        for (size_t i  = (alloced_ptrs->size - 1); i < (alloced_ptrs->size * 2);
             i        += 1)
        {
            new_arr[i] = NULL;
        }

        alloced_ptrs->ptrs  = new_arr;
        alloced_ptrs->size *= 2;
    }

    for (size_t i = 0; i < alloced_ptrs->used; i += 1)
    {
        if (res != NULL && alloced_ptrs->ptrs[i] == ptr)
        {
            alloced_ptrs->ptrs[i] = res;
            goto bread_realloc_ret;
        }
    }

    alloced_ptrs->ptrs[alloced_ptrs->used]  = res;
    alloced_ptrs->used                     += 1;

bread_realloc_ret:
    return res;
}

void bread_print_args()
{
    qsort((ArgPtr *)some_args->ptrs, some_args->used, sizeof(ArgPtr),
          bread_args_compare);

    size_t opt_len = 0;
    for (size_t i = 0; i < some_args->used; i++)
    {
        ArgPtr x = ((ArgPtr)some_args->ptrs[i]);
        size_t y = strlen(x->long_opt);
        if (x->long_opt != NULL)
        {
            opt_len = (y > opt_len) ? y : opt_len;
        }
    }

    for (size_t i = 0; i < some_args->used; i++)
    {
        ArgPtr x = ((ArgPtr)some_args->ptrs[i]);
        printf("\t-%c \t --%s", x->short_opt, x->long_opt);

        if (x->arg_type_list != NULL)
        {
            if (x->long_opt != NULL)
            {
                for (size_t ii  = 0; ii < (opt_len - strlen(x->long_opt));
                     ii        += 1)
                {
                    printf(" ");
                }
            }

            if (x->descr != NULL)
            {
                printf("\t %s\n\t\t\t\t={ ", x->descr);
            }
            else
            {
                printf("\n\t\t\t\t={ ");
            }

            for (size_t iii = 0; iii < x->arg_count; iii += 1)
            {
                enum __arg_type z = x->arg_type_list[iii];
                switch (z)
                {
                case BREAD_I64:
                    printf("INT");
                    break;
                case BREAD_U64:
                    printf("UINT");
                    break;
                case BREAD_CHAR:
                    printf("STR");
                    break;
                }

                if ((iii + 1) != x->arg_count)
                {
                    printf(", ");
                }
            }

            printf(" }");
        }
        else if (x->descr != NULL)
        {
            printf("\t %s", x->descr);
        }

        printf("\n");
    }
}

void bread_parser_add_option(char short_opt, char *long_opt, size_t group)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            bread_panic("Cannot initialize memory for storing the "
                        "arguments, calloc returned NULL\n");
        }
    }

    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            fprintf(stderr, "Option -%c is already defined\n", short_opt);
            return;
        }
    }

    if (some_args->size <= (some_args->used + 1))
    {
        void **new_arr = bread_realloc(some_args->ptrs,
                                       sizeof(ArgPtr) * (some_args->size * 2));
        if (new_arr == NULL)
        {
            bread_panic("Cannot track new memory given by bread_malloc, "
                        "realloc return NULL\n");
        }

        for (size_t i  = (some_args->size - 1); i < (some_args->size * 2);
             i        += 1)
        {
            new_arr[i] = NULL;
        }

        some_args->ptrs  = new_arr;
        some_args->size *= 2;
    }

    some_args->ptrs[some_args->used] = bread_calloc(1, sizeof(struct __arg));
    if (((ArgPtr)some_args->ptrs[some_args->used]) == NULL)
    {
        bread_panic(
            "Cannot initialize memory for storing the arguments, calloc "
            "returned NULL\n");
    }

    ((ArgPtr)some_args->ptrs[some_args->used])->short_opt     = short_opt;
    ((ArgPtr)some_args->ptrs[some_args->used])->group_num     = group;
    ((ArgPtr)some_args->ptrs[some_args->used])->descr         = NULL;
    ((ArgPtr)some_args->ptrs[some_args->used])->arg_type_list = NULL;
    ((ArgPtr)some_args->ptrs[some_args->used])->args          = NULL;
    ((ArgPtr)some_args->ptrs[some_args->used])->arg_count     = 0;
    ((ArgPtr)some_args->ptrs[some_args->used])->used          = false;

    if (long_opt != NULL)
    {
        ((ArgPtr)some_args->ptrs[some_args->used])->long_opt =
            bread_malloc(sizeof(char) * (strlen(long_opt) + 1));

        if (((ArgPtr)some_args->ptrs[some_args->used])->long_opt == NULL)
        {
            bread_panic(
                "Cannot initialize memory for storing the arguments, malloc "
                "returned NULL\n");
        }

        strcpy(((ArgPtr)some_args->ptrs[some_args->used])->long_opt, long_opt);
    }
    else
    {
        ((ArgPtr)some_args->ptrs[some_args->used])->long_opt = NULL;
    }

    some_args->used += 1;
}

void bread_parser_add_descrp(char short_opt, char *description)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            bread_panic("Cannot initialize memory for storing the "
                        "arguments, calloc returned NULL\n");
        }
    }

    for (size_t i = 0; i < some_args->used; i += 1)
    {
        ArgPtr x = (ArgPtr)some_args->ptrs[i];
        if (short_opt == x->short_opt && x->descr == NULL)
        {
            ((ArgPtr)some_args->ptrs[i])->descr =
                bread_calloc((strlen(description) + 1), sizeof(char));

            if (((ArgPtr)some_args->ptrs[i])->descr == NULL)
            {
                bread_panic("Cannot initialize memory for storing description "
                            "for argument '%c', malloc "
                            "returned NULL\n",
                            short_opt);
            }

            strcpy(((ArgPtr)some_args->ptrs[i])->descr, description);
            return;
        }
    }

    fprintf(stderr, "Argument short opt -%c not found\n", short_opt);
}

void bread_parser_opt_argmts(char short_opt, size_t arg_count, ...)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            bread_panic("Cannot initialize memory for storing the "
                        "arguments, calloc returned NULL\n");
        }
    }

    ArgPtr x = NULL;
    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            x = some_args->ptrs[i];
        }
    }

    if (x == NULL)
    {
        fprintf(stderr, "Short opt -%c not found\n", short_opt);
        return;
    }

    va_list args;
    va_start(args, arg_count);

    x->arg_count     = arg_count;
    x->arg_type_list = bread_malloc(sizeof(enum __arg_type) * arg_count);
    if (x->arg_type_list == NULL)
    {
        bread_panic("Cannot allocate memory for the arg types of option %c\n",
                    short_opt);
    }

    for (size_t i = 0; i < arg_count; i += 1)
    {
        x->arg_type_list[i] = va_arg(args, enum __arg_type);
    }

    va_end(args);
}

void bread_parse(int argc, char **argv)
{
    for (int i = 1; i < argc; i += 1)
    {
        ArgPtr x = NULL;
        if (argv[i][0] == '-')
        {
            if (argv[i][1] == '-')
            {
                for (size_t ii = 0; ii < some_args->used; ii += 1)
                {
                    if (strcmp(((ArgPtr)some_args->ptrs[ii])->long_opt,
                               &argv[i][2]) == 0)
                    {
                        x = (ArgPtr)some_args->ptrs[ii];
                    }
                }

                if (x->used == true)
                {
                    continue;
                }

                if (x == NULL)
                {
                    bread_panic("Unknown argument %s was passed\n", argv[i]);
                }

                x->used = true;
                if (x->arg_count > 0)
                {
                    x->args = bread_malloc(sizeof(void *) * x->arg_count);
                    if (x->args == NULL)
                    {
                        bread_panic("Cannot allocate memory for the arguments "
                                    "of opt %s\n",
                                    argv[i]);
                    }

                    for (size_t ii = 1; ii <= x->arg_count; ii += 1)
                    {
                        switch (x->arg_type_list[ii - 1])
                        {
                        case BREAD_I64:
                        {
                            char *idk;
                            long res = strtol(argv[i + ii], &idk, 10);
                            if ((res == 0) && (strcmp(argv[i + ii], "0") != 0))
                            {
                                bread_panic("Expecting a number argument for "
                                            "opt arg #%lu "
                                            "of %s, got %s\n",
                                            ii, argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = bread_malloc(sizeof(res));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = &res;
                            break;
                        }
                        case BREAD_U64:
                        {
                            char *idk;
                            unsigned long res = strtoul(argv[i + ii], &idk, 10);
                            if ((res == 0) && (strcmp(argv[i + ii], "0") != 0))
                            {
                                bread_panic(
                                    "Expecting an unsigned number argument "
                                    "for opt arg #%lu of %s, got %s\n",
                                    ii, argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = bread_malloc(sizeof(res));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = &res;
                            break;
                        }
                        case BREAD_CHAR:
                        {
                            x->args[ii - 1] = bread_malloc(
                                (strlen(argv[i + ii]) + 1) * sizeof(char));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            strcpy(x->args[ii - 1], argv[i + ii]);
                            break;
                        }
                        }
                    }
                }
            }
            else
            {
                for (size_t ii = 0; ii < some_args->used; ii += 1)
                {
                    if (((ArgPtr)some_args->ptrs[ii])->short_opt == argv[i][1])
                    {
                        x = (ArgPtr)some_args->ptrs[ii];
                    }
                }

                if (x == NULL)
                {
                    bread_panic("Unknown argument %s was passed\n", argv[i]);
                }

                x->used = true;
                if (x->arg_count > 0)
                {
                    x->args = bread_malloc(sizeof(void *) * x->arg_count);
                    if (x->args == NULL)
                    {
                        bread_panic("Cannot allocate memory for the arguments "
                                    "of opt %s\n",
                                    argv[i]);
                    }

                    for (size_t ii = 1; ii <= x->arg_count; ii += 1)
                    {
                        switch (x->arg_type_list[ii - 1])
                        {
                        case BREAD_I64:
                        {
                            char *idk;
                            long res = strtol(argv[i + ii], &idk, 10);
                            if ((res == 0) && (strcmp(argv[i + ii], "0") != 0))
                            {
                                bread_panic("Expecting a number argument for "
                                            "opt arg #%lu "
                                            "of %s, got %s\n",
                                            ii, argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = bread_malloc(sizeof(res));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = &res;
                            break;
                        }
                        case BREAD_U64:
                        {
                            char *idk;
                            unsigned long res = strtoul(argv[i + ii], &idk, 10);
                            if ((res == 0) && (strcmp(argv[i + ii], "0") != 0))
                            {
                                bread_panic(
                                    "Expecting an unsigned number argument "
                                    "for opt arg #%lu of %s, got %s\n",
                                    ii, argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = bread_malloc(sizeof(res));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            x->args[ii - 1] = &res;
                            break;
                        }
                        case BREAD_CHAR:
                        {
                            x->args[ii - 1] = bread_malloc(
                                (strlen(argv[i + ii]) + 1) * sizeof(char));
                            if (x->args[ii - 1] == NULL)
                            {
                                bread_panic(
                                    "Cannot allocate memory for the argument "
                                    "of opt %s which is %s\n",
                                    argv[i], argv[i + ii]);
                            }

                            strcpy(x->args[ii - 1], argv[i + ii]);
                            break;
                        }
                        }
                    }
                }
            }
        }

        if (x != NULL)
        {
            i += x->arg_count;
        }
    }
}

bool bread_parser_is_opt_used(char short_opt)
{
    ArgPtr x = NULL;
    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            x = (ArgPtr)some_args->ptrs[i];
        }
    }

    if (x == NULL)
    {
        fprintf(stderr, "Short opt -%c not found\n", short_opt);
        return NULL;
    }

    return x->used;
}

void **bread_parser_get_all_args(char short_opt)
{
    ArgPtr x = NULL;
    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            x = (ArgPtr)some_args->ptrs[i];
        }
    }

    if (x == NULL)
    {
        fprintf(stderr, "Short opt -%c not found\n", short_opt);
        return NULL;
    }

    return x->args;
}

void *bread_parser_get_arg(char short_opt, size_t index)
{
    ArgPtr x = NULL;
    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            x = (ArgPtr)some_args->ptrs[i];
        }
    }

    if (x == NULL)
    {
        fprintf(stderr, "Short opt -%c not found\n", short_opt);
        return NULL;
    }

    if (index >= x->arg_count)
    {
        fprintf(stderr,
                "Index %lu is higher than the arg count of short opt %c\n",
                index, short_opt);
        return NULL;
    }

    return x->args[index];
}

#endif
