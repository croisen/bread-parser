#ifndef __CROI_BREAD_PARSER_H__
#define __CROI_BREAD_PARSER_H__

/*
 * After creating an app for my shs with dart
 * I may have gotten addicted to camelCase...
 * May I use it correctly...
 */

#include <stdbool.h>
#include <stdint.h>

#define BParserNoShortOpt -127

enum ArgType
{
    I32BP,
    I64BP,
    U32BP,
    U64BP,
    STRBP,
    ANYBP,
};

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

    void bParserAddOpts(const char shortOpt, const char *longOpt);
    void bParserAddDesc(const char shortOpt, const char *longOpt,
                        const char *description);
    void bParserAddArgs(const char shortOpt, const char *longOpt,
                        uint64_t argCount, ...);
    void bParserAddIndefArgs(const char shortOpt, const char *longOpt,
                             enum ArgType type);

    void bParserParse(int argc, char **argv);
    bool bParserWasOptUsed(const char shortOpt);
    void **bParserGetArgs(const char shortOpt);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CROI_BREAD_PARSER_H__

// #define __CROI_BREAD_PARSER_IMPL__
#ifdef __CROI_BREAD_PARSER_IMPL__

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BPINFO "[INFO]     "
#define BPWARN "[WARN]     "
#define BPFAIl "[ERROR]    " // This don't seem correct but I'mma go with it
#define BPCRIT "[CRITICAL] "

typedef struct BParserMemTrackerArr
{
    bool init;
    uint64_t size;
    uint64_t used;
    void **ptrs;
} BParserMemTrackerArr;

typedef struct BParserOpt
{
    char shortOpt;
    char *longOpt;
    char *description;

    bool isUsed;

    int64_t argCount;
    enum ArgType *argTypes;
    void **args;
} BParserOpt;

typedef struct BParserOptDynArr
{
    bool init;
    uint64_t size;
    uint64_t used;

    BParserOpt *opts;
} BParserOptDynArr;

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define bParserLog(msgPrefix, fmtMessage, ...)                                 \
    fprintf(stdout, "(%s Line: %3" PRIu64 ") " msgPrefix fmtMessage, __FILE__, \
            (uint64_t)__LINE__, ##__VA_ARGS__)

#define bParserPanic(exitCode, msgPrefix, fmtMessage, ...)                     \
    fprintf(stderr, "(%s Line: %3" PRIu64 ") " msgPrefix fmtMessage, __FILE__, \
            (uint64_t)__LINE__, ##__VA_ARGS__),                                \
        exit(EXIT_FAILURE)

    BParserMemTrackerArr bParserMemTracker = {
        false,
        0,
        0,
        NULL,
    };

    BParserOptDynArr bParserOptDynArr = {
        false,
        0,
        0,
        NULL,
    };

    void bParserMemTrackerInit(void);
    void bParserMemTrackerFree(void);
    void bParserMemTrackerExpand(void);

    void bParserMemTrackerInit(void)
    {
        bParserMemTracker.init = true;
        bParserMemTracker.size = 8;
        bParserMemTracker.used = 0;
        bParserMemTracker.ptrs =
            (void **)calloc(bParserMemTracker.size, sizeof(void *));
        if (bParserMemTracker.ptrs == NULL)
            bParserPanic(
                1, BPCRIT,
                "Failed to initialize memtracker for bread parser :(\n");

        atexit(bParserMemTrackerFree);
    }

    void bParserMemTrackerFree(void)
    {
        for (uint64_t i = 0; i < bParserMemTracker.used; i += 1)
        {
            free(bParserMemTracker.ptrs[i]);
        }
        bParserMemTracker.size = 0;
        bParserMemTracker.used = 0;
        bParserMemTracker.init = 0;
        free(bParserMemTracker.ptrs);
    }

    void bParserMemTrackerExpand(void)
    {
        if (bParserMemTracker.size == (bParserMemTracker.used + 1))
        {
            void **newTracker =
                realloc(bParserMemTracker.ptrs,
                        bParserMemTracker.size * sizeof(void *) * 2);
            if (newTracker == NULL)
                bParserPanic(EXIT_FAILURE, BPCRIT,
                             "Failed to expand list of pointers for the mem "
                             "tracker of bread parser...\n");

            for (uint64_t i                         = bParserMemTracker.size;
                 i < bParserMemTracker.size * 2; i += 1)
            {
                newTracker[i] = NULL;
            }

            bParserMemTracker.size *= 2;
            bParserMemTracker.ptrs  = newTracker;
        }
    }

    bool bParserIsPtrInTracker(void *ptr)
    {
        for (uint64_t i = 0; i < bParserMemTracker.used; i += 1)
        {
            if (bParserMemTracker.ptrs[i] == ptr)
                return true;
        }

        return false;
    }

    void bParserFree(void *ptr)
    {
        int64_t ptrIndex = -1;
        for (uint64_t i = 0; i < bParserMemTracker.used; i += 1)
        {
            if (bParserMemTracker.ptrs[i] == ptr)
            {
                ptrIndex = i;
                break;
            }
        }

        if (ptrIndex != -1)
        {
            // Did I use memmove correctly to do this?
            // [0, 1, 2] pop(1)
            // [0, gone, 2]
            // [0, 2]
            memmove(bParserMemTracker.ptrs + ptrIndex,
                    bParserMemTracker.ptrs + ptrIndex + 1,
                    (bParserMemTracker.used - 1 - ptrIndex) * sizeof(void *));
            bParserMemTracker.used -= 1;
        }
    }

    void *bParserMalloc(uint64_t size)
    {
        void *res = malloc((size_t)size);
        if (res == NULL)
            bParserPanic(
                EXIT_FAILURE, BPCRIT,
                "Malloc returned null when trying to allocate memory for "
                "the bread parser\n");

        bParserMemTrackerExpand();
        bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
        bParserMemTracker.used                         += 1;
        return res;
    }

    void *bParserCalloc(uint64_t nmemb, uint64_t size)
    {
        void *res = calloc((size_t)nmemb, (size_t)size);
        if (res == NULL)
            bParserPanic(
                EXIT_FAILURE, BPCRIT,
                "Calloc returned null when trying to allocate memory for "
                "the bread parser\n");

        bParserMemTrackerExpand();
        bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
        bParserMemTracker.used                         += 1;
        return res;
    }

    void *bParserRealloc(void *ptr, uint64_t size)
    {
        void *res = realloc(ptr, (size_t)size);
        if (res == NULL)
            bParserPanic(
                EXIT_FAILURE, BPCRIT,
                "Realloc returned null when trying to allocate memory for "
                "the bread parser\n");

        if (!bParserIsPtrInTracker(res))
        {
            bParserMemTrackerExpand();
            bParserFree(ptr);
            bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
            bParserMemTracker.used                         += 1;
        }

        return res;
    }

    void bParserOptDynArrInit()
    {
        bParserOptDynArr.init = true;
        bParserOptDynArr.size = 8;
        bParserOptDynArr.used = 0;
        bParserOptDynArr.opts =
            bParserCalloc(bParserOptDynArr.size, sizeof(BParserOpt));
    }

    void bParserOptDynArrExpand()
    {
        if (bParserOptDynArr.size == (bParserOptDynArr.used + 1))
        {
            BParserOpt *newTracker =
                realloc(bParserOptDynArr.opts,
                        bParserOptDynArr.size * sizeof(BParserOpt) * 2);
            if (newTracker == NULL)
                bParserPanic(EXIT_FAILURE, BPCRIT,
                             "Failed to expand list of pointers for the mem "
                             "tracker of bread parser\n");

            bParserOptDynArr.size *= 2;
            bParserOptDynArr.opts  = newTracker;
        }
    }

    uint64_t bParserFindLongestOptLen(void)
    {
        // strlen of help (automatically added when bParserParse is called)
        // this is also a helper to add whitespace from the long opt to the
        // description of the option
        uint64_t currentLen = 4;
        for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1)
        {
            uint64_t newLen = strlen(bParserOptDynArr.opts[i].longOpt);
            if (newLen > currentLen)
                currentLen = newLen;
        }

        return currentLen;
    }

    BParserOpt *bParserFindOpt(const char shortOpt, const char *longOpt)
    {
        // Might as well make a signal that an option only has a longOpt?
        // Or should I make this a macro (yes I did make it a macro)
        if (shortOpt == BParserNoShortOpt)
        {
            for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1)
            {
                if (strcmp(bParserOptDynArr.opts[i].longOpt, longOpt) == 0)
                    return bParserOptDynArr.opts + i;
            }
        }
        else
        {
            if (longOpt == NULL)
                return NULL;

            for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1)
            {
                if (bParserOptDynArr.opts[i].shortOpt == shortOpt)
                    return bParserOptDynArr.opts + i;
            }
        }

        if (shortOpt == BParserNoShortOpt)
            bParserLog(BPWARN, "Option --%s cannot be found\n", longOpt);
        else
            bParserLog(BPWARN, "Option -%c cannot be found\n", shortOpt);

        return NULL;
    }

    void bParserAddOpts(const char shortOpt, const char *longOpt)
    {
        BParserOpt *newOpt = bParserMalloc(sizeof(BParserOpt));
        if (newOpt == NULL)
        {
            bParserPanic(
                EXIT_FAILURE, BPCRIT,
                "Cannot add new opt -%c --%s due to malloc returning null\n",
                shortOpt, longOpt);
        }

        newOpt->shortOpt = shortOpt;

        if (longOpt != NULL)
        {
            uint64_t longOptLen = strlen(longOpt);
            newOpt->longOpt     = bParserCalloc(longOptLen + 1, sizeof(char));
            if (newOpt->longOpt != NULL)
            {
                strcpy(newOpt->longOpt, longOpt);
            }
            else
            {
                bParserLog(
                    BPCRIT,
                    "Failed to allocate memory for the long opt version of "
                    "opt -%c\n",
                    shortOpt);
            }
        }

        bParserOptDynArrExpand();
        memcpy(bParserOptDynArr.opts + bParserOptDynArr.used, newOpt,
               sizeof(BParserOpt));
        bParserOptDynArr.used += 1;
    }

    void bParserAddDesc(const char shortOpt, const char *longOpt,
                        const char *description)
    {
        BParserOpt *opt = bParserFindOpt(shortOpt, longOpt);
        if (opt == NULL)
            return;

        if (description != NULL)
        {
            uint64_t descriptionLen = strlen(description);
            opt->description = bParserCalloc(descriptionLen + 1, sizeof(char));
            if (opt->description != NULL)
            {
                strcpy(opt->description, description);
            }
            else
            {
                bParserLog(
                    BPCRIT,
                    "Failed to allocate memory for the description of opt "
                    "-%c\n",
                    shortOpt);
            }

            return;
        }
    }

    void bParserAddArgs(const char shortOpt, const char *longOpt,
                        uint64_t argCount, ...)
    {
        BParserOpt *opt = bParserFindOpt(shortOpt, longOpt);
        if (opt == NULL || argCount <= 0)
            return;

        va_list ap = {
            {0, 0, 0, 0}
        };
        va_start(ap, argCount);

        opt->argCount = argCount;
        opt->argTypes = bParserMalloc(argCount * sizeof(enum ArgType));
        if (opt->argTypes == NULL)
        {
            va_end(ap);

            if (shortOpt == BParserNoShortOpt)
            {
                bParserPanic(1, BPCRIT,
                             "Cannot add argument types to opt -%c\n",
                             shortOpt);
            }
            else
            {
                bParserPanic(1, BPCRIT,
                             "Cannot add argument types to opt --%s\n",
                             longOpt);
            }
        }

        for (uint64_t i = 0; i < argCount; i += 1)
        {
            opt->argTypes[i] = va_arg(ap, enum ArgType);
        }

        va_end(ap);
    }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CROI_BREAD_PARSER_IMPL__
