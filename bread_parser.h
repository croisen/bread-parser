// O hail no #pragmas
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#elif defined __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvariadic-macros"
#endif

#ifndef CROI_LIB_BREAD_PARSER_H
#define CROI_LIB_BREAD_PARSER_H

#include <inttypes.h>
#include <math.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <signal.h>
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

    uint64_t arg_count;
    uint64_t group_num;

    char short_opt;
    char *long_opt;
    char *descr;

    enum __arg_type *arg_type_list;
    void **args;
};

struct __darr_for_ptr
{
    bool init;
    uint64_t size;
    uint64_t used;
    void **ptrs;
};

typedef struct __arg *ArgPtr;

typedef struct __darr_for_ptr DA[1];

extern void bread_parser_set_program_name(const char *name);
extern void bread_parser_set_author_name(const char *name);
extern void bread_parser_set_author_email(const char *email);

extern void bread_print_args(void);
extern void bread_print_help(int exit_code, char **argv);
extern void bread_parse(int argc, char **argv);

extern bool bread_parser_is_opt_used(char short_opt);
extern void **bread_parser_get_all_args(char short_opt);
extern void *bread_parser_get_arg(char short_opt, uint64_t index);

extern void bread_parser_add_option(char short_opt, char *long_opt,
                                    uint64_t group);
extern void bread_parser_add_descrp(char short_opt, char *description);
extern void bread_parser_opt_argmts(char short_opt, uint64_t arg_count, ...);

extern void __bread_free(void *ptr);
extern void *__bread_calloc(uint64_t nmemb, uint64_t size);
extern void *__bread_malloc(uint64_t size);
extern void *__bread_realloc(void *ptr, uint64_t size);

#define __bread_panic(message, ...)          \
    fprintf(stderr, message, ##__VA_ARGS__); \
    exit(EXIT_FAILURE)

#endif

#ifdef CROI_LIB_BREAD_PARSER_IMPL_H

char *author_name  = NULL;
char *author_email = NULL;
char *program_name = NULL;

DA some_args       = {0};
DA alloced_ptrs    = {0};

extern int __memtracker_sort(const void *a, const void *b);
extern void __memtracker_init(void);
extern void __memtracker_free_void(void);
extern void __memtracker_free_sig(int dummy);

int __bread_args_compare(const void *a, const void *b)
{
    ArgPtr c = *(ArgPtr *)a;
    ArgPtr d = *(ArgPtr *)b;

    if (c->group_num > d->group_num)
    {
        return 1;
    }
    else if (c->group_num < d->group_num)
    {
        return -1;
    }

    if (c->short_opt > d->short_opt)
    {
        return 1;
    }
    else if (c->short_opt < d->short_opt)
    {
        return -1;
    }

    return 0;
}

int __memtracker_sort(const void *a, const void *b)
{
    void *c = *(void **)a;
    void *d = *(void **)b;

    // Sorted from highest to lowest
    return (uint64_t)d - (uint64_t)c;
}

void __memtracker_init(void)
{
    atexit(__memtracker_free_void);
    signal(SIGINT, __memtracker_free_sig);
    signal(SIGSEGV, __memtracker_free_sig);

    alloced_ptrs->init = true;
    alloced_ptrs->size = 4;
    alloced_ptrs->used = 0;

    alloced_ptrs->ptrs = calloc(alloced_ptrs->size, sizeof(void *));
    if (alloced_ptrs->ptrs == NULL)
    {
        __bread_panic(
            "Cannot initialize memory tracker for the bread parser, calloc "
            "returned NULL\n");
    }
}

void __memtracker_free_void(void)
{
    for (uint64_t i = 0; i < alloced_ptrs->size; i += 1)
    {
#ifdef __BCROI_PARSER_DEBUG
        printf("Freeing alloced_ptrs: %p\n", alloced_ptrs->ptrs[i]);
#endif
        free(alloced_ptrs->ptrs[i]);
    }

#ifdef __BCROI_PARSER_DEBUG
    printf("Freeing alloced_ptrs itself: %p\n", (void *)alloced_ptrs->ptrs);
#endif
    free(alloced_ptrs->ptrs);
}

void __memtracker_free_sig(int dummy)
{
    __bread_panic("Signal %s caught, now exiting...\n", strsignal(dummy));
}

void bread_parser_set_program_name(const char *name)
{
    program_name = __bread_calloc(strlen(name) + 1, sizeof(char));
    if (program_name == NULL)
    {
        __bread_panic("Unable to set program name to %s\n", name);
    }

    strcpy(program_name, name);
}

void bread_parser_set_author_name(const char *name)
{
    author_name = __bread_calloc(strlen(name) + 1, sizeof(char));
    if (author_name == NULL)
    {
        __bread_panic("Unable to set author name to %s\n", name);
    }

    strcpy(author_name, name);
}

void bread_parser_set_author_email(const char *email)
{
    author_email = __bread_calloc(strlen(email) + 1, sizeof(char));
    if (author_email == NULL)
    {
        __bread_panic("Unable to set author name to %s\n", email);
    }

    strcpy(author_email, email);
}

void *__bread_calloc(uint64_t nmemb, uint64_t size)
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
            __bread_panic("Cannot track new memory given by bread_calloc, "
                          "realloc return NULL\n");
        }

        for (uint64_t i                       = (alloced_ptrs->size - 1);
             i < (alloced_ptrs->size * 2); i += 1)
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

void *__bread_malloc(uint64_t size)
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
            __bread_panic("Cannot track new memory given by bread_malloc, "
                          "realloc return NULL\n");
        }

        for (uint64_t i                       = (alloced_ptrs->size - 1);
             i < (alloced_ptrs->size * 2); i += 1)
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

void *__bread_realloc(void *ptr, uint64_t size)
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
            __bread_panic("Cannot track new memory given by bread_malloc, "
                          "realloc return NULL\n");
        }

        for (uint64_t i                       = (alloced_ptrs->size - 1);
             i < (alloced_ptrs->size * 2); i += 1)
        {
            new_arr[i] = NULL;
        }

        alloced_ptrs->ptrs  = new_arr;
        alloced_ptrs->size *= 2;
    }

    for (uint64_t i = 0; i < alloced_ptrs->used; i += 1)
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

void __bread_free(void *ptr)
{
    qsort(alloced_ptrs->ptrs, alloced_ptrs->used, sizeof(void *),
          __memtracker_sort);
    uint64_t key = (uint64_t)ptr;
    void **item  = bsearch(&key, alloced_ptrs->ptrs, alloced_ptrs->used,
                           sizeof(void *), __memtracker_sort);
    if (item != NULL)
    {
        *item = NULL;
        qsort(alloced_ptrs->ptrs, alloced_ptrs->used, sizeof(void *),
              __memtracker_sort);
    }

    free(ptr);
    alloced_ptrs->used -= 1;
}

// Prints the information about an argument via it's argptr
// internal function only
void __bread_print_some_arg(ArgPtr x)
{
    if (x == NULL)
    {
        return;
    }

    printf("\t-%c \t --%s={ ", x->short_opt, x->long_opt);

    for (uint64_t i = 0; i < x->arg_count; i += 1)
    {
        enum __arg_type z = x->arg_type_list[i];
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

        if ((i + 1) != x->arg_count)
        {
            printf(", ");
        }
    }

    if (x->descr != NULL)
    {
        printf(" }\t %s\n", x->descr);
    }
    else
    {
        printf(" }\n");
    }
}

// Parses the arguments after --opt or -o when using bread_parse
// and initializes the necessary struct **fields** with said args
// though the whole program would exit if the input doesn't match
// the signature made with bread_parser_opt_argmts (Also an internal function)
uint64_t __bread_parse_opt_args(ArgPtr x, uint64_t offset, uint64_t argc,
                                char **argv)
{
    uint64_t args_parsed = 0;
    if (x == NULL)
    {
        __bread_panic("Unknown argument %s was passed\n", argv[offset]);
    }

    x->used = true;

    if (x->arg_count == 0)
    {
        return args_parsed;
    }

    x->args = __bread_calloc(x->arg_count, sizeof(void *));
    if (x->args == NULL)
    {
        __bread_panic("Cannot allocate memory for the arguments "
                      "of opt %s\n",
                      argv[offset]);
    }

    for (uint64_t i = 1; i <= x->arg_count; i += 1)
    {
        if ((i + offset) >= argc)
        {
            fprintf(stderr, "Not enough arguments for opt %s\n", argv[offset]);
            fprintf(stderr,
                    "Expected arguments of opt %s in the order seen "
                    "below\n",
                    argv[offset]);
            __bread_print_some_arg(x);
            break;
        }

        // Parse args after the --opt or -o, and if it doesn't match
        // the signature(?) made by bread_parser_opt_argmts, exit immediately
        switch (x->arg_type_list[i - 1])
        {
        case BREAD_I64:
        {
            char *idk;
            long res = strtol(argv[offset + i], &idk, 10);
            if ((res == 0) && (strcmp(argv[offset + i], "0") != 0))
            {
                fprintf(stderr,
                        "Expecting a number argument for opt arg #%" PRIu64
                        " of %s, got \"%s\"\n",
                        i, argv[offset], argv[offset + i]);
                fprintf(
                    stderr,
                    "Expected arguments of opt %s in the order seen below\n",
                    argv[offset]);
                __bread_print_some_arg(x);
                printf("\n\n");
                bread_print_help(1, argv);
            }

            x->args[i - 1] = __bread_malloc(sizeof(res));
            if (x->args[i - 1] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              argv[offset], argv[offset + i]);
                continue;
            }

            *((int64_t *)x->args[i - 1])  = res;
            args_parsed                  += 1;
            break;
        }
        case BREAD_U64:
        {
            char *idk;
            unsigned long res = strtoul(argv[offset + i], &idk, 10);
            if ((res == 0) && (strcmp(argv[offset + i], "0") != 0))
            {
                fprintf(stderr,
                        "Expecting an unsigned number argument for "
                        "opt arg #%" PRIu64 " of %s, got \"%s\"\n",
                        i, argv[offset], argv[offset + i]);
                fprintf(stderr,
                        "Expected arguments of opt %s in the order seen "
                        "below\n",
                        argv[offset]);
                __bread_print_some_arg(x);
                printf("\n\n");
                bread_print_help(1, argv);
            }

            x->args[i - 1] = __bread_malloc(sizeof(res));
            if (x->args[i - 1] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              argv[offset], argv[offset + i]);
            }

            *((uint64_t *)x->args[i - 1])  = res;
            args_parsed                   += 1;
            break;
        }
        case BREAD_CHAR:
        {
            x->args[i - 1] =
                __bread_malloc((strlen(argv[offset + i]) + 1) * sizeof(char));
            if (x->args[i - 1] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              argv[offset], argv[offset + i]);
            }

            strcpy(x->args[i - 1], argv[offset + i]);
            args_parsed += 1;
            break;
        }
        }
    }

    return args_parsed;
}

void __bread_parser_opt_args_equals_one_arg(ArgPtr x, char *long_opt_with_arg)
{
    if (x->arg_count > 0)
    {
        x->args = __bread_calloc(x->arg_count, sizeof(void *));
        if (x->args == NULL)
        {
            __bread_panic(
                "Could not allocate memory for the arguments of opt --%s\n",
                x->long_opt);
        }
    }
    else
    {
        return;
    }

    uint64_t total_arg_len = strlen(long_opt_with_arg);
    char *copy             = __bread_calloc(total_arg_len + 1, sizeof(char));
    strcpy(copy, long_opt_with_arg);

    char *where_first_brace = strtok(copy, "{");
    where_first_brace       = strtok(NULL, ",");
    if (*where_first_brace == ' ')
    {
        where_first_brace += 1;
    }

    char *current_arg = where_first_brace;
    uint64_t arg_len  = 0;
    x->used           = true;
    for (uint64_t i = 0; i < x->arg_count; i += 1)
    {
        if ((i + 1) >= x->arg_count)
        {
            copy[total_arg_len - 1]     = '\0';
            char *last_bits             = &copy[total_arg_len - 2];
            char *partway_into_last_arg = strtok(NULL, ",");
            current_arg                 = partway_into_last_arg -
                          ((*partway_into_last_arg == ' ') ? 2 : 1);
            arg_len = (uint64_t)last_bits - (uint64_t)partway_into_last_arg -
                      ((*(partway_into_last_arg - 1) == ' ') ? 2 : 1) -
                      (uint64_t)current_arg;
        }
        else
        {
            char *partway_into_next_arg = strtok(NULL, ",");
            if (partway_into_next_arg == NULL)
            {
                fprintf(stderr,
                        "Insufficient amount of args passed into opt --%s\n",
                        x->long_opt);
                fprintf(
                    stderr,
                    "Expected arguments of opt --%s in the order seen below\n",
                    x->long_opt);
                __bread_print_some_arg(x);
                exit(EXIT_FAILURE);
            }

            current_arg = partway_into_next_arg -
                          ((*partway_into_next_arg == ' ') ? 2 : 1);
            arg_len = (uint64_t)partway_into_next_arg -
                      ((*partway_into_next_arg == ' ') ? 2 : 1) -
                      (uint64_t)current_arg;
        }

        char *alloced_current_arg = __bread_calloc(arg_len + 1, sizeof(char));
        strncpy(alloced_current_arg, current_arg, arg_len);

#ifdef __BCROI_PARSER_DEBUG
        printf("alloced current: %s, len: %" PRIu64 "\n", alloced_current_arg,
               arg_len);
#endif

        switch (x->arg_type_list[i])
        {
        case BREAD_I64:
        {
            char *idk;
            unsigned long res = strtol(alloced_current_arg, &idk, 10);
            if ((res == 0) && (strcmp(alloced_current_arg, "0") != 0))
            {
                fprintf(stderr,
                        "Expecting a number argument for opt arg "
                        "#%" PRIu64 " of --%s, got \"%s\"\n",
                        i + 1, x->long_opt, alloced_current_arg);
                fprintf(
                    stderr,
                    "Expected arguments of opt --%s in the order seen below\n",
                    x->long_opt);
                __bread_print_some_arg(x);
                exit(EXIT_FAILURE);
            }

            x->args[i] = __bread_malloc(sizeof(res));
            if (x->args[i] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              x->long_opt, alloced_current_arg);
            }

            *((int64_t *)x->args[i]) = res;
            break;
        }
        case BREAD_U64:
        {
            char *idk;
            unsigned long res = strtoul(alloced_current_arg, &idk, 10);
            if ((res == 0) && (strcmp(alloced_current_arg, "0") != 0))
            {
                fprintf(stderr,
                        "Expecting an unsigned number argument for opt arg "
                        "#%" PRIu64 " of --%s, got \"%s\"\n",
                        i + 1, x->long_opt, alloced_current_arg);
                fprintf(
                    stderr,
                    "Expected arguments of opt --%s in the order seen below\n",
                    x->long_opt);
                __bread_print_some_arg(x);
                exit(EXIT_FAILURE);
            }

            x->args[i] = __bread_malloc(sizeof(res));
            if (x->args[i] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              x->long_opt, alloced_current_arg);
            }

            *((uint64_t *)x->args[i]) = res;
            break;
        }
        case BREAD_CHAR:
        {
            x->args[i] = __bread_malloc((arg_len + 1) * sizeof(char));
            if (x->args[i] == NULL)
            {
                __bread_panic("Cannot allocate memory for the argument "
                              "of opt %s which is %s\n",
                              x->long_opt, alloced_current_arg);
            }

            strcpy(x->args[i], alloced_current_arg);
            break;
        }
        }

        __bread_free(alloced_current_arg);
    }

    __bread_free(copy);
}

// void __bread_parser_opt_args_equals_mul_arg(ArgPtr x, uint64_t offset, int
// argc, char **argv)
//{
// uint64_t arg_len = strlen(long_opt_witharg);
// char *copy       = __bread_calloc(arg_len + 1, sizeof(char));
// strcpy(copy, long_opt_arg);

// char *where_equals = strtok(copy, "=");
// char *where_brace  = strtok(copy, "{");

//// TODO: Be able to do --some-opt='{1, "some str", -1}'
//// --some-opt='{ 1, "some str", -79 }'
//// and even absurdly --some-opt=\{ 1, "some_opt", -2 \}
// for (uint64_t i = 0; i < x->arg_count; i += 1)
//{
// switch (x->arg_type_list[i])
//{
// case BREAD_I64:
//{
// break;
//}
// case BREAD_U64:
//{
// break;
//}
// case BREAD_CHAR:
//{
// break;
//}
//}

// if ((i + 1) >= x->arg_count)
//{
// continue;
//}
// else
//{
//}
//}
//}

void bread_print_args(void)
{
    qsort((ArgPtr *)some_args->ptrs, some_args->used, sizeof(ArgPtr),
          __bread_args_compare);

    uint64_t opt_len = 0;
    for (uint64_t i = 0; i < some_args->used; i++)
    {
        ArgPtr x = ((ArgPtr)some_args->ptrs[i]);
        if (x->long_opt != NULL)
        {
            uint64_t y = strlen(x->long_opt);
            opt_len    = (y > opt_len) ? y : opt_len;
        }
    }

    for (uint64_t i = 0; i < some_args->used; i++)
    {
        ArgPtr x = ((ArgPtr)some_args->ptrs[i]);
        printf("\t-%c \t --%s", x->short_opt, x->long_opt);
        if (x->long_opt != NULL)
        {
            for (uint64_t ii = 0; ii < (opt_len - strlen(x->long_opt)); ii += 1)
            {
                printf(" ");
            }
        }

        if (x->arg_type_list != NULL)
        {
            if (x->descr != NULL)
            {
                printf("\t %s\n", x->descr);
                for (uint64_t ii  = 0; ii < (opt_len - strlen(x->long_opt));
                     ii          += 1)
                {
                    printf(" ");
                }

                printf("\t\t\t={ ");
            }
            else
            {
                printf("={ ");
            }

            for (uint64_t iii = 0; iii < x->arg_count; iii += 1)
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

        if (x->short_opt == 'h')
        {
            printf("\n");
        }

        printf("\n");
    }
}

void bread_parser_add_option(char short_opt, char *long_opt, uint64_t group)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = __bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            __bread_panic("Cannot initialize memory for storing the "
                          "arguments, calloc returned NULL\n");
        }
    }

    for (uint64_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            fprintf(stderr, "Option -%c is already defined\n", short_opt);
            return;
        }
    }

    if (some_args->size <= (some_args->used + 1))
    {
        void **new_arr = __bread_realloc(
            some_args->ptrs, sizeof(ArgPtr) * (some_args->size * 2));
        if (new_arr == NULL)
        {
            __bread_panic("Cannot track new memory given by bread_malloc, "
                          "realloc return NULL\n");
        }

        for (uint64_t i  = (some_args->size - 1); i < (some_args->size * 2);
             i          += 1)
        {
            new_arr[i] = NULL;
        }

        some_args->ptrs  = new_arr;
        some_args->size *= 2;
    }

    some_args->ptrs[some_args->used] = __bread_calloc(1, sizeof(struct __arg));
    if (((ArgPtr)some_args->ptrs[some_args->used]) == NULL)
    {
        __bread_panic(
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
            __bread_malloc(sizeof(char) * (strlen(long_opt) + 1));

        if (((ArgPtr)some_args->ptrs[some_args->used])->long_opt == NULL)
        {
            __bread_panic(
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

        some_args->ptrs = __bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            __bread_panic("Cannot initialize memory for storing the "
                          "arguments, calloc returned NULL\n");
        }
    }

    for (uint64_t i = 0; i < some_args->used; i += 1)
    {
        ArgPtr x = (ArgPtr)some_args->ptrs[i];
        if (short_opt == x->short_opt && x->descr == NULL)
        {
            ((ArgPtr)some_args->ptrs[i])->descr =
                __bread_calloc((strlen(description) + 1), sizeof(char));

            if (((ArgPtr)some_args->ptrs[i])->descr == NULL)
            {
                __bread_panic(
                    "Cannot initialize memory for storing description "
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

// Adds arguments to opt, arg types is denoted by BREAD_U64, BREAD_I64,
// and BREAD_CHAR
void bread_parser_opt_argmts(char short_opt, uint64_t arg_count, ...)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = __bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            __bread_panic("Cannot initialize memory for storing the "
                          "arguments, calloc returned NULL\n");
        }
    }

    ArgPtr x = NULL;
    for (uint64_t i = 0; i < some_args->used; i += 1)
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
    x->arg_type_list = __bread_malloc(sizeof(enum __arg_type) * arg_count);
    if (x->arg_type_list == NULL)
    {
        __bread_panic("Cannot allocate memory for the arg types of option %c\n",
                      short_opt);
    }

    for (uint64_t i = 0; i < arg_count; i += 1)
    {
        x->arg_type_list[i] = va_arg(args, enum __arg_type);
    }

    va_end(args);
}

// Prints all the arguments made with bread_parser_add_option
// Though the help option would not exist yet as that is only
// finalized after using bread_parse
void bread_print_help(int exit_code, char **argv)
{
    printf("Program name: %s\n",
           (program_name != NULL) ? program_name : argv[0]);
    bread_print_args();

    printf("Made by: %s\n", (author_name != NULL)
                                ? author_name
                                : "Forgot to set my name in this "
                                  "(argparser is made by croisen)");
    printf("If faced with any problems contact <<%s>>\n",
           (author_email != NULL)
               ? author_email
               : "croisen the one who made the argparser (jk)");

    exit(exit_code);
}

// The one that mainly parses the arguments passed into an application
// (not really), and it adds the -h option to the whole app
// (**should be called only once**)
void bread_parse(int argc, char **argv)
{
    bread_parser_add_option('h', "help", 0);
    bread_parser_add_descrp('h', "Prints this help option");

    if (argc > 1 && (argv[1][1] == 'h' || argv[1][1] == '?' ||
                     strcmp(&argv[1][2], "help") == 0))
    {
        bread_print_help(0, argv);
    }

    for (int i = 1; i < argc; i += 1)
    {
        ArgPtr x             = NULL;
        char *equals_used    = strtok(argv[i], "=");
        uint64_t args_parsed = 0;

        if (argv[i][0] != '-')
        {
            continue;
        }

        equals_used = strtok(NULL, "=");
        for (uint64_t ii = 0; ii < some_args->used; ii += 1)
        {
            ArgPtr current = (ArgPtr)some_args->ptrs[ii];
            if (strcmp(current->long_opt, &argv[i][2]) == 0 ||
                current->short_opt == argv[i][1])
            {
                x = (ArgPtr)some_args->ptrs[ii];
                break;
            }
        }

        if (x == NULL)
        {
            fprintf(stderr, "Unknown argument %s passed into %s\n\n", argv[i],
                    (program_name != NULL) ? program_name : argv[0]);
            bread_print_help(1, argv);
        }

        if (x->used == true)
        {
            continue;
        }

        if (equals_used != NULL)
        {
            uint64_t ptr_diff = (uint64_t)equals_used - (uint64_t)argv[i] - 1;
            argv[i][ptr_diff] = '=';
            if (argv[i][strlen(argv[i]) - 1] == '}')
            {
                __bread_parser_opt_args_equals_one_arg(x, argv[i]);
            }
            else
            {
                // args_parsed =
                //__bread_parser_opt_args_equals_mul_arg(x, i, argc, argv);
            }
        }
        else
        {

            args_parsed = __bread_parse_opt_args(x, i, argc, argv);
        }

        i += args_parsed;
    }
}

// Returns true if the opt was used after bread_parse
// (the first safe guard against null ptr dereferencing on opts and args that
// wasn't passed)
bool bread_parser_is_opt_used(char short_opt)
{
    ArgPtr x = NULL;
    for (uint64_t i = 0; i < some_args->used; i += 1)
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

// Returns a list of ptrs to the args of an opt after bread_parse
// returns null if opt has an arg count of zero
// (a.k.a. bread_parser_opt_argmts was not used on this opt)
void **bread_parser_get_all_args(char short_opt)
{
    ArgPtr x = NULL;
    for (uint64_t i = 0; i < some_args->used; i += 1)
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

// Used after bread_parse, returns a pointer to a specific argument in an opt
// and indexing starts at 0, and this function would fail if you use
// it on an opt that has an arg count of 0
void *bread_parser_get_arg(char short_opt, uint64_t index)
{
    ArgPtr x = NULL;
    for (uint64_t i = 0; i < some_args->used; i += 1)
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
                "Index %" PRIu64
                " is higher than the arg count of short opt %c\n",
                index, short_opt);
        return NULL;
    }

    return x->args[index];
}

#endif
