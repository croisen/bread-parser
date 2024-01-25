#ifndef CROI_LIB_BREAD_PARSER_H
#define CROI_LIB_BREAD_PARSER_H

#include <assert.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

struct __arg
{
    size_t arg_count;
    size_t group_num;

    char short_opt;
    char *long_opt;
    char *descr;

    void **args;
};

struct __darr_for_ptr
{
    bool init;
    size_t size;
    size_t used;
    void **ptrs;
};

typedef enum __option
{
    NONE,
    COOL,
    ARGS,
} Bread_Option_T;

typedef struct __arg Arg[1];
typedef struct __arg *ArgPtr;

typedef struct __darr_for_ptr DA[1];

#define bread_panic(message, ...)            \
    fprintf(stderr, message, ##__VA_ARGS__); \
    exit(EXIT_FAILURE)

#define is_used(short_opt) \
    ((__is_used(short_opt) == COOL) || (__is_used(short_opt) == ARGS))
#define get_arg(short_opt) __get_args(short_opt)

void __memtracker_init();
void __memtracker_free();

void *bread_calloc(size_t nmemb, size_t size);
void *bread_malloc(size_t size);
void *bread_realloc(void *ptr, size_t size);

void bread_parse(int argc, char **argv);
void bread_parser_add_option(char short_opt, char *long_opt, int64_t arg_count,
                             int64_t group);
void bread_parser_add_descr(char short_opt, char *description);
void bread_print_args(void);

Bread_Option_T __is_used(char short_opt);
char **__get_args(char short_opt);

#endif

#ifdef CROI_LIB_BREAD_PARSER_IMPL_H

DA some_args    = {0};
DA alloced_ptrs = {0};

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
    for (size_t i = 0; i < some_args->used; i++)
    {
        ArgPtr x = ((ArgPtr)some_args->ptrs[i]);
        printf("-%c \t --%s \t %s\n", x->short_opt, x->long_opt, x->descr);
    }
}

void bread_parse(int argc, char **argv)
{
    (void)argc, (void)argv; // Not used for now
    bread_panic("Not implemented yet, bread_parse\n");
}

void bread_parser_add_option(char short_opt, char *long_opt, int64_t arg_count,
                             int64_t group)
{
    if (!some_args->init)
    {
        some_args->init = true;
        some_args->size = 4;
        some_args->used = 0;

        some_args->ptrs = bread_calloc(some_args->size, sizeof(ArgPtr));
        if (some_args->ptrs == NULL)
        {
            bread_panic(
                "Cannot initialize memory for storing the arguments, calloc "
                "returned NULL\n");
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

    some_args->ptrs[some_args->used] = bread_malloc(sizeof(struct __arg));
    if (((ArgPtr)some_args->ptrs[some_args->used]) == NULL)
    {
        bread_panic(
            "Cannot initialize memory for storing the arguments, malloc "
            "returned NULL\n");
    }

    ((ArgPtr)some_args->ptrs[some_args->used])->short_opt = short_opt;
    ((ArgPtr)some_args->ptrs[some_args->used])->arg_count = arg_count;
    ((ArgPtr)some_args->ptrs[some_args->used])->group_num = group;

    ((ArgPtr)some_args->ptrs[some_args->used])->long_opt =
        bread_malloc(sizeof(char) * (strlen(long_opt) + 1));

    if (((ArgPtr)some_args->ptrs[some_args->used])->long_opt == NULL)
    {
        bread_panic(
            "Cannot initialize memory for storing the arguments, malloc "
            "returned NULL\n");
    }

    strcpy(((ArgPtr)some_args->ptrs[some_args->used])->long_opt, long_opt);
    some_args->used += 1;
}

void bread_parser_add_descr(char short_opt, char *description)
{
    for (size_t i = 0; i < some_args->used; i += 1)
    {
        if (short_opt == ((ArgPtr)some_args->ptrs[i])->short_opt)
        {
            ((ArgPtr)some_args->ptrs[i])->descr =
                bread_malloc(sizeof(char) * (strlen(description) + 1));

            if (((ArgPtr)some_args->ptrs[i])->descr == NULL)
            {
                bread_panic("Cannot initialize memory for storing description "
                            "for argument '%c', malloc "
                            "returned NULL\n",
                            short_opt);
            }

            strcpy(((ArgPtr)some_args->ptrs[i])->descr, description);
        }
    }
}

Bread_Option_T __is_used(char short_opt);
char **__get_args(char short_opt);

#endif
