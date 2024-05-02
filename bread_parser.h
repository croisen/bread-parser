#ifndef __CROI_BREAD_PARSER_H__
#define __CROI_BREAD_PARSER_H__

/*
 * After creating an app for my shs with dart
 * I may have gotten addicted to camelCase...
 * May I use it correctly...
 */

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BParserMaxLineWidth 80
#define BParserNoShortOpt   -127

enum BParserArgType {
    I32BP,
    I64BP,
    U32BP,
    U64BP,
    STRBP,
    ANYBP,
};

void bParserSetProgramName(char *name);
void bParserSetProgramVersion(char *version);
void bParserSetAuthorName(char *name);
void bParserSetAuthorEmail(char *email);

/*
 * Sets an opt like -v and --verbose, the group specifies in which order it
 * is shown in the automatically generated help command
 */
void bParserAddOpts(const char shortOpt, const char *longOpt, int64_t group);

// Sets the description written in the help command
void bParserAddDesc(
    const char shortOpt, const char *longOpt, const char *description
);

/*
 * Sets the arguments to be accepted by the option specified by add opt
 * Set argCount to -1 and only specify one argType to make an opt accept an
 * indefinite number of one argType
 */
void bParserAddArgs(
    const char shortOpt, const char *longOpt, int64_t argCount,
    enum BParserArgType argType
);

/*
 * Start parsing the opts (must be used after defining the opts)
 */
void bParserParse(int argc, char **argv);
void bParserPrintHelp(void);
bool bParserWasOptUsed(const char shortOpt, const char *longOpt);
void **bParserGetArgs(const char shortOpt, const char *longOpt);
void *bParserGetArg(const char shortOpt, const char *longOpt, uint64_t index);
int64_t bParserGetNumOfArgs(const char shortOpt, const char *longOpt);

#define BPINFO "[INFO]     "
#define BPWARN "[WARN]     "
#define BPFAIL "[ERROR]    " // This don't seem correct but I'mma go with it
#define BPCRIT "[CRITICAL] "

#define bParserFprintf(columnWidth, stream, fmt, ...)                          \
    do {                                                                       \
        int printLen = columnWidth;                                            \
        uint64_t neededSize =                                                  \
            snprintf(NULL, 0, fmt __VA_OPT__(, ) __VA_ARGS__) + 1;             \
        char *_CL_printBuf = (char *)malloc(neededSize * sizeof(char));        \
        if (_CL_printBuf == NULL) {                                            \
            fprintf(stream, "Cannot initialize columned fprintf\n");           \
            fprintf(stream, fmt __VA_OPT__(, ) __VA_ARGS__);                   \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
                                                                               \
        sprintf(_CL_printBuf, fmt __VA_OPT__(, ) __VA_ARGS__);                 \
        char *_CL_copy = _CL_printBuf;                                         \
        while ((_CL_printBuf + neededSize - 1) >= _CL_copy) {                  \
            int wordLen = strcspn(_CL_copy, " ");                              \
            if (wordLen > printLen) {                                          \
                printf("\n");                                                  \
                printLen = columnWidth;                                        \
            }                                                                  \
                                                                               \
            if (_CL_copy[wordLen - 1] == '\n') {                               \
                printf("%.*s", wordLen, _CL_copy);                             \
                printLen  = columnWidth;                                       \
                _CL_copy += wordLen;                                           \
                _CL_copy += strcspn(_CL_copy, " ") + 1;                        \
            } else {                                                           \
                printf("%.*s ", wordLen, _CL_copy);                            \
                _CL_copy += wordLen;                                           \
                _CL_copy += strcspn(_CL_copy, " ") + 1;                        \
                printLen -= wordLen + 1;                                       \
            }                                                                  \
        }                                                                      \
        free(_CL_printBuf);                                                    \
    } while (false)

#define bParserLog(msgPrefix, fmtMessage, ...)                                 \
    bParserFprintf(80, stdout, msgPrefix fmtMessage __VA_OPT__(, ) __VA_ARGS__)

#define bParserPanic(exitCode, msgPrefix, fmtMessage, ...)                     \
    do {                                                                       \
        bParserFprintf(                                                        \
            80, stdout, msgPrefix fmtMessage __VA_OPT__(, ) __VA_ARGS__        \
        );                                                                     \
        exit(exitCode);                                                        \
    } while (false)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CROI_BREAD_PARSER_H__

#ifdef __CROI_BREAD_PARSER_IMPL__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <signal.h>
#include <stdlib.h>
#include <string.h>

typedef struct BParserMemTrackerArr {
    bool init;
    uint64_t size;
    uint64_t used;
    void **ptrs;
} BParserMemTrackerArr;

typedef struct BParserOpt {
    char shortOpt;
    char *longOpt;
    char *description;

    bool isUsed;
    bool isIndefinite;

    int64_t argCount;
    enum BParserArgType argType;
    uint64_t argsUsed;
    uint64_t argsCap;
    void **args;

    int64_t group;
} BParserOpt;

typedef struct BParserOptDynArr {
    bool init;
    uint64_t size;
    uint64_t used;

    BParserOpt *opts;
} BParserOptDynArr;

static BParserMemTrackerArr bParserMemTracker = {
    false,
    0,
    0,
    NULL,
};

static BParserOptDynArr bParserOptDynArr = {
    false,
    0,
    0,
    NULL,
};

static char *programName    = NULL;
static char *programVersion = NULL;
static char *authorName     = NULL;
static char *authorEmail    = NULL;

static void bParserMemTrackerInit(void);
static void bParserMemTrackerFree(void);
static void bParserMemTrackerFreeSignal(int sig);
static void bParserMemTrackerExpand(void);

static int64_t bParserFindPtrInTracker(void *ptr);
static void bParserOptDynArrInit(void);
static void bParserOptDynArrExpand(void);

// Unused within the scope of bread parser (Well I'll just rely on the atexit
// handlers to free the memory allocated by these)
/*static void bParserFree(void *ptr);*/
static void *bParserMalloc(uint64_t size);
static void *bParserCalloc(uint64_t nmemb, uint64_t size);
static void *bParserRealloc(void *ptr, uint64_t size);

static uint64_t bParserFindLongestOptLen(void);
static BParserOpt *
bParserFindOpt(const char shortOpt, const char *longOpt, bool noWarn);
static int bParserCompareOpts(const void *a, const void *b);
static void bParserPrintOpts(void);
static bool bParserParseArgs(BParserOpt *opt, char *supposedArg);
static bool bParserParseMultShortOpt(char *opt, char *supposedArg);
static bool bParserParseShortOpt(char *opt, char *supposedArg);
static bool bParserParseLongOpt(char *opt, char *supposedArg);

static uint64_t countChar(char *s, char c);

static uint64_t countChar(char *s, char c)
{
    uint64_t count = 0;
    for (uint64_t i = 0; s[i] != '\0'; i += 1)
        count += (s[i] == c) ? 1 : 0;

    return count;
}

static void bParserMemTrackerInit(void)
{
    bParserMemTracker.init = true;
    bParserMemTracker.size = 8;
    bParserMemTracker.used = 0;
    bParserMemTracker.ptrs =
        (void **)calloc(bParserMemTracker.size, sizeof(void *));
    if (bParserMemTracker.ptrs == NULL)
        bParserPanic(
            1, BPCRIT, "Failed to initialize memtracker for bread parser :(\n"
        );

    atexit(bParserMemTrackerFree);
    signal(SIGINT, bParserMemTrackerFreeSignal);
    signal(SIGSEGV, bParserMemTrackerFreeSignal);
}

static void bParserMemTrackerFreeSignal(int sig)
{
    exit(sig);
}

static void bParserMemTrackerFree(void)
{
    for (uint64_t i = 0; i < bParserMemTracker.used; i += 1)
        free(bParserMemTracker.ptrs[i]);

    bParserMemTracker.size = 0;
    bParserMemTracker.used = 0;
    bParserMemTracker.init = 0;
    free(bParserMemTracker.ptrs);
}

static void bParserMemTrackerExpand(void)
{
    if (bParserMemTracker.size == (bParserMemTracker.used + 1)) {
        void **newTracker = (void **)realloc(
            bParserMemTracker.ptrs, bParserMemTracker.size * sizeof(void *) * 2
        );
        if (newTracker == NULL)
            bParserPanic(
                EXIT_FAILURE, BPCRIT,
                "Failed to expand list of pointers for the mem "
                "tracker of bread parser...\n"
            );

        for (uint64_t i                         = bParserMemTracker.size;
             i < bParserMemTracker.size * 2; i += 1) {
            newTracker[i] = NULL;
        }

        bParserMemTracker.size *= 2;
        bParserMemTracker.ptrs  = newTracker;
    }
}

static int64_t bParserFindPtrInTracker(void *ptr)
{
    for (uint64_t i = 0; i < bParserMemTracker.used; i += 1) {
        if (bParserMemTracker.ptrs[i] == ptr)
            return i;
    }

    return -1;
}

/*static void bParserFree(void *ptr)*/
/*{*/
/*int64_t ptrIndex = -1;*/
/*for (uint64_t i = 0; i < bParserMemTracker.used; i += 1) {*/
/*if (bParserMemTracker.ptrs[i] == ptr) {*/
/*ptrIndex = i;*/
/*break;*/
/*}*/
/*}*/

/*if (ptrIndex != -1) {*/
/*
 * Did I use memmove correctly to do this?
 * [0, 1, 2] pop(1)
 * [0, gone, 2]
 * [0, 2]
 */
/*memmove(*/
/*bParserMemTracker.ptrs + ptrIndex,*/
/*bParserMemTracker.ptrs + ptrIndex + 1,*/
/*(bParserMemTracker.used - 1 - ptrIndex) * sizeof(void *)*/
/*);*/
/*bParserMemTracker.used -= 1;*/
/*}*/

/*// Holy crap I forgot to call free from this free(ptr);*/
/*free(ptr);*/
/*}*/

static void *bParserMalloc(uint64_t size)
{
    if (!bParserMemTracker.init)
        bParserMemTrackerInit();

    void *res = malloc((size_t)size);
    if (res == NULL)
        bParserPanic(
            EXIT_FAILURE, BPCRIT,
            "Malloc returned null when trying to allocate memory for "
            "the bread parser\n"
        );

    bParserMemTrackerExpand();

    bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
    bParserMemTracker.used                         += 1;
    return res;
}

static void *bParserCalloc(uint64_t nmemb, uint64_t size)
{
    if (!bParserMemTracker.init)
        bParserMemTrackerInit();

    void *res = calloc((size_t)nmemb, (size_t)size);
    if (res == NULL)
        bParserPanic(
            EXIT_FAILURE, BPCRIT,
            "Calloc returned null when trying to allocate memory for "
            "the bread parser\n"
        );

    bParserMemTrackerExpand();
    bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
    bParserMemTracker.used                         += 1;
    return res;
}

static void *bParserRealloc(void *ptr, uint64_t size)
{
    if (!bParserMemTracker.init)
        bParserMemTrackerInit();

    int64_t ptrIndex = bParserFindPtrInTracker(ptr);
    void *res        = realloc(ptr, (size_t)size);
    if (res == NULL)
        bParserPanic(
            EXIT_FAILURE, BPCRIT,
            "Realloc returned null when trying to allocate memory for "
            "the bread parser\n"
        );

    /*
     * Might as well just put the new ptr by realloc there as it
     * automatically does the freeing if it's on a new location
     */
    bParserMemTrackerExpand();
    if (ptrIndex != -1)
        bParserMemTracker.ptrs[ptrIndex] = res;
    else {
        bParserMemTracker.ptrs[bParserMemTracker.used]  = res;
        bParserMemTracker.used                         += 1;
    }

    return res;
}

static void bParserOptDynArrInit(void)
{
    bParserOptDynArr.init = true;
    bParserOptDynArr.size = 8;
    bParserOptDynArr.used = 0;
    bParserOptDynArr.opts =
        (BParserOpt *)bParserCalloc(bParserOptDynArr.size, sizeof(BParserOpt));
}

static void bParserOptDynArrExpand(void)
{
    if (bParserOptDynArr.size == (bParserOptDynArr.used + 1)) {
        BParserOpt *newTracker = (BParserOpt *)bParserRealloc(
            bParserOptDynArr.opts,
            bParserOptDynArr.size * sizeof(BParserOpt) * 2
        );

        bParserOptDynArr.size *= 2;
        bParserOptDynArr.opts  = newTracker;
    }
}

static uint64_t bParserFindLongestOptLen(void)
{
    /*
     * strlen of help (automatically added when bParserParse is called)
     * this is also a helper to add whitespace from the long opt to the
     * description of the option
     */
    uint64_t currentLen = 4;
    for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1) {
        BParserOpt opt  = bParserOptDynArr.opts[i];
        uint64_t newLen = (opt.longOpt != NULL) ? strlen(opt.longOpt) : 0;
        if (newLen > currentLen)
            currentLen = newLen;
    }

    return currentLen;
}

static BParserOpt *
bParserFindOpt(const char shortOpt, const char *longOpt, bool noWarn)
{
    /*
     * Might as well make a signal that an option only has a longOpt?
     * Or should I make this a macro (yes I did make it a macro)
     */
    if (shortOpt == BParserNoShortOpt) {
        if (longOpt == NULL && !noWarn) {
            bParserLog(
                BPWARN, "Undefined option (shortOpt is BParserNoShortOpt and "
                        "longOpt is null at the same time) cannot be found\n"
            );
            return NULL;
        }

        for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1) {
            if (bParserOptDynArr.opts[i].longOpt == NULL)
                continue;

            if (strcmp(bParserOptDynArr.opts[i].longOpt, longOpt) == 0)
                return bParserOptDynArr.opts + i;
        }
    } else {
        for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1) {
            if (bParserOptDynArr.opts[i].shortOpt == shortOpt)
                return bParserOptDynArr.opts + i;
        }
    }

    if (!noWarn) {
        if (shortOpt == BParserNoShortOpt)
            bParserLog(BPWARN, "Option --%s cannot be found\n", longOpt);
        else
            bParserLog(BPWARN, "Option -%c cannot be found\n", shortOpt);
    }

    return NULL;
}

static int bParserCompareOpts(const void *a, const void *b)
{
    BParserOpt *f    = (BParserOpt *)a;
    BParserOpt *s    = (BParserOpt *)b;

    // The higher the group, it gets displayed first
    int compareGroup = (f->group < s->group) - (f->group > s->group);
    if (compareGroup != 0)
        return compareGroup;

    // Makes the no short opt become last?
    if ((f->shortOpt == BParserNoShortOpt) !=
        (s->shortOpt == BParserNoShortOpt)) {
        if (f->shortOpt == BParserNoShortOpt)
            return 1;
        else
            return -1;
    }

    // Compares alphabetically
    int compareShortOpt =
        (f->shortOpt > s->shortOpt) - (f->shortOpt < s->shortOpt);
    if (compareShortOpt != 0)
        return compareShortOpt;

    int compareFirstLetterLongOpt = 0;

    // Well the long opt can be null soooo guard rails
    if (f->longOpt == NULL)
        return -1;
    if (s->longOpt == NULL)
        return 1;

    // To compare alphabetically, the higher one becomes last
    compareFirstLetterLongOpt =
        (f->longOpt[0] > s->longOpt[0]) - (f->longOpt[0] < s->longOpt[0]);
    if (compareFirstLetterLongOpt != 0)
        return compareFirstLetterLongOpt;

    return 0;
}

static void bParserPrintOpts(void)
{
    /*
     * I want the output to be like
     *     -v    --verbose    Makes this program more verbose
     *                        =possible,args
     * 4 (starting) + 2 (dash and letter) + 4 (space) + [2 + longestOptLen -
     * currentOptLen] (get to the end of --longOpt) + 4 (space) description
     */
    int longestOptLen = (int)bParserFindLongestOptLen();
    int lengthToDescr = 16 + longestOptLen;
    qsort(
        bParserOptDynArr.opts, bParserOptDynArr.used, sizeof(BParserOpt),
        bParserCompareOpts
    );

    for (uint64_t i = 0; i < bParserOptDynArr.used; i += 1) {
        BParserOpt opt    = bParserOptDynArr.opts[i];
        int currentOptLen = (opt.longOpt != NULL) ? strlen(opt.longOpt) : 0;

        if (opt.shortOpt != BParserNoShortOpt)
            printf("    -%c    ", opt.shortOpt);
        else
            printf("          ");

        if (opt.longOpt != NULL)
            printf("--%s", opt.longOpt);
        else
            // -- alternative for nothing
            printf("%*s", 2, "");

        /*
         * Done: Try to keep track of the 80 line limit in the output?
         * Nah just let it be for now
         * I couldn't just let it be...
         * And dem was my previous solution bad, there were 2-3 loops inside
         * this if block back then...
         */
        if (opt.description != NULL) {
            // Some space after the long opt
            printf("%*s", longestOptLen - currentOptLen + 4, "");

            uint64_t descriptionLen = strlen(opt.description);
            int printLen            = BParserMaxLineWidth - lengthToDescr;
            char *descrPtr2         = opt.description;
            while ((uint64_t)opt.description + descriptionLen >=
                   (uint64_t)descrPtr2) {
                int wordLen = (int)strcspn(descrPtr2, " ");
                if (wordLen > printLen) {
                    printf("\n%*s", lengthToDescr, " ");
                    printLen = BParserMaxLineWidth - lengthToDescr;
                }

                // printf("printLen: %4d wordLen: %4d\n", printLen,
                // wordLen);
                printf("%.*s ", wordLen, descrPtr2);
                descrPtr2 += wordLen;

                // +1 to compensate after the space?
                printLen  -= wordLen + 1;
                descrPtr2 += strcspn(descrPtr2, " ") + 1;
            }

            printf("\n");
        }

        if (opt.argCount > 0 && !opt.isIndefinite) {
            if (opt.description != NULL)
                printf("%*s", lengthToDescr, "");

            printf("=");
            for (uint64_t ii = 0; ii < (uint64_t)opt.argCount; ii += 1) {
                switch (opt.argType) {
                case I32BP:
                    printf("I32");
                    break;
                case I64BP:
                    printf("I64");
                    break;
                case U32BP:
                    printf("U32");
                    break;
                case U64BP:
                    printf("U64");
                    break;
                case STRBP:
                    printf("STR");
                    break;
                case ANYBP:
                    printf("ANY");
                    break;
                }

                if (ii < (uint64_t)lengthToDescr &&
                    (ii + 1) != (uint64_t)opt.argCount)
                    printf(",");
                else
                    printf("\n");
            }

            /*
             * Extra newline so that the args can feel separated from
             * the next opt?
             */
            if (opt.description != NULL)
                printf("\n");
        } else if (opt.isIndefinite) {
            if (opt.description != NULL)
                printf("%*s", lengthToDescr, "");

            printf("=");
            switch (opt.argType) {
            case I32BP:
                printf("I32");
                break;
            case I64BP:
                printf("I64");
                break;
            case U32BP:
                printf("U32");
                break;
            case U64BP:
                printf("U64");
                break;
            case STRBP:
                printf("STR");
                break;
            case ANYBP:
                printf("ANY");
                break;
            }

            printf("...\n");

            /*
             * Extra newline so that the args can feel separated from
             * the next opt?
             */
            if (opt.description != NULL)
                printf("\n");
        } else if (opt.description == NULL)
            /*
             * Well if there's no description and no args well it doesn't
             * really print a newline anywhere except here
             */
            printf("\n");
    }
}

static void initOptArgs(BParserOpt *opt)
{
    if (opt->argsCap == 0) {
        opt->argsCap = (opt->isIndefinite) ? 4 : opt->argCount + 1;
        opt->args    = (void **)bParserCalloc(opt->argsCap, sizeof(void *));
    }
}

static void expandOptArgsCap(BParserOpt *opt)
{
    if ((opt->argsUsed + 1) >= opt->argsCap) {
        void **newArgs = (void **)bParserRealloc(
            opt->args, opt->argsCap * 2 * sizeof(void *)
        );
        for (uint64_t i = opt->argsCap; i < opt->argsCap * 2; i += 1)
            opt->args[i] = NULL;

        opt->argsCap *= 2;
        opt->args     = newArgs;
    }
}

static bool bParserParseArgs(BParserOpt *opt, char *supposedArg)
{
    if (opt->argCount == 0 && !opt->isIndefinite)
        return false;

    initOptArgs(opt);
    uint64_t argCount = countChar(supposedArg, ',') + 1;
    char *copy        = supposedArg;
    for (uint64_t i = opt->argsUsed; i < argCount; i += 1) {
        if (!opt->isIndefinite && i >= (uint64_t)opt->argCount) {
            if (opt->shortOpt != BParserNoShortOpt)
                bParserPanic(
                    EXIT_FAILURE, BPCRIT,
                    "Opt -%c only accepts %" PRId64
                    " arguments see help for more info by passing -h\n",
                    opt->shortOpt, opt->argCount
                );
            else
                bParserPanic(
                    EXIT_FAILURE, BPCRIT,
                    "Opt --%s only accepts %" PRId64
                    " arguments see help for more info by passing -h\n",
                    opt->longOpt, opt->argCount
                );
        }

        uint64_t argLen = strcspn(copy, ",");
        int scanfRes    = -2;
        expandOptArgsCap(opt);
        switch (opt->argType) {
        case I32BP: // FALLTHROUGH
        case U32BP:
            opt->args[i] = bParserMalloc(sizeof(uint32_t));
            scanfRes     = scanf("%" SCNu32, (uint32_t *)(opt->args[i]));
            if (scanfRes == EOF || scanfRes == 0) {
                if (opt->shortOpt != BParserNoShortOpt)
                    bParserPanic(
                        EXIT_FAILURE, BPCRIT,
                        "Opt -%c Arg #%" PRIu64
                        " is wrong, it is expecting a 32 bit integer see help "
                        "for more info by passing -h\n",
                        opt->shortOpt, i + 1
                    );
                else
                    bParserPanic(
                        EXIT_FAILURE, BPCRIT,
                        "Opt --%s Arg #%" PRIu64
                        " is wrong, it is expecting a 32 bit integer see help "
                        "for more info by passing -h\n",
                        opt->longOpt, i + 1
                    );
            }
            break;
        case I64BP: // FALLTHROUGH
        case U64BP:
            opt->args[i] = bParserMalloc(sizeof(uint64_t));
            scanfRes     = sscanf(copy, "%" SCNu64, (uint64_t *)(opt->args[i]));
            if (scanfRes == EOF || scanfRes == 0) {
                if (opt->shortOpt != BParserNoShortOpt)
                    bParserPanic(
                        EXIT_FAILURE, BPCRIT,
                        "Opt -%c Arg #%" PRIu64
                        " is wrong, it is expecting a 64 bit integer see help "
                        "for more info by passing -h\n",
                        opt->shortOpt, i + 1
                    );
                else
                    bParserPanic(
                        EXIT_FAILURE, BPCRIT,
                        "Opt --%s Arg #%" PRIu64
                        " is wrong, it is expecting a 64 bit integer see help "
                        "for more info by passing -h\n",
                        opt->longOpt, i + 1
                    );
            }
            break;
        case STRBP: // FALLTHROUGH
        case ANYBP:
            opt->args[i] = bParserCalloc(argLen + 1, sizeof(char));
            memcpy(opt->args[i], copy, argLen);
            break;
        default:
            opt->args[i] = bParserCalloc(argLen + 1, sizeof(char));
            memcpy(opt->args[i], copy, argLen);
            break;
        }

        copy          += argLen + 1;
        opt->argsUsed += 1;
    }
    return true;
}

static bool bParserParseMultShortOpt(char *opt, char *supposedArg)
{
    (void)supposedArg;
    bool usedSupposedArg = false;
    char *copy           = opt;
    while (*copy != '\0') {
        BParserOpt *foundOpt = bParserFindOpt(*copy, NULL, false);
        if (foundOpt == NULL)
            return false;

        if ((foundOpt->argCount > 0 || foundOpt->isIndefinite) &&
            (*(copy + 1) != '\0')) {
            bParserLog(
                BPWARN,
                "Using an opt that takes an argument such as -%c at the "
                "start "
                "or the middle of a multi short opt commandline argument "
                "is "
                "annoying to parse, so the argument '%s' will not be used "
                "for "
                "-%c and will only used for -%c in this multi short opt "
                "commandline argument: -%s\n",
                foundOpt->shortOpt, supposedArg, foundOpt->shortOpt,
                opt[strlen(opt) - 1], opt
            );
            copy += 1;
            continue;
        }

        if ((foundOpt->argCount == 0 || !foundOpt->isIndefinite) &&
            foundOpt->isUsed)
            bParserLog(
                BPWARN,
                "The opt -%c that is not supposed to take multiple "
                "arguments has already been used\n",
                foundOpt->shortOpt
            );
        else
            foundOpt->isUsed = true;

        if ((foundOpt->argCount > 0 || foundOpt->isIndefinite) &&
            *(copy + 1) == '\0')
            usedSupposedArg = bParserParseArgs(foundOpt, supposedArg);

        copy += 1;
    }

    return usedSupposedArg;
}

// Why bool because it only needs to tell to increase the argv index by 1 or
// 2
static bool bParserParseShortOpt(char *opt, char *supposedArg)
{
    if (strlen(opt) != 1)
        return bParserParseMultShortOpt(opt, supposedArg);

    BParserOpt *foundOpt = bParserFindOpt(*opt, NULL, false);
    if (foundOpt == NULL)
        return false;

    if ((foundOpt->argCount == 0 || !foundOpt->isIndefinite) &&
        foundOpt->isUsed)
        bParserLog(
            BPWARN,
            "The opt -%c that is not supposed to take multiple "
            "arguments has already been used\n",
            foundOpt->shortOpt
        );
    else
        foundOpt->isUsed = true;

    if (foundOpt->argCount > 0 || foundOpt->isIndefinite) {
        return bParserParseArgs(foundOpt, supposedArg);
    }

    return false;
}

// Why bool because it only needs to tell to increase the argv index by 1 or
// 2
static bool bParserParseLongOpt(char *opt, char *supposedArg)
{
    uint64_t optLen  = (uint64_t)strlen(opt);
    uint64_t toEqual = (uint64_t)strcspn(opt, "=");
    char *copy       = (char *)bParserMalloc((optLen + 1) * sizeof(char));
    strcpy(copy, opt);

    BParserOpt *foundOpt = bParserFindOpt(BParserNoShortOpt, copy, false);
    if (foundOpt == NULL)
        return false;

    if (optLen == toEqual) {
        if ((foundOpt->argCount == 0 || !foundOpt->isIndefinite) &&
            foundOpt->isUsed)
            bParserLog(
                BPWARN,
                "The opt --%s that is not supposed to take multiple "
                "arguments has already been used\n",
                foundOpt->longOpt
            );
        else
            foundOpt->isUsed = true;

        return bParserParseArgs(foundOpt, supposedArg);
    } else {
        copy[toEqual]        = '\0';
        BParserOpt *foundOpt = bParserFindOpt(BParserNoShortOpt, copy, false);
        if ((foundOpt->argCount == 0 || !foundOpt->isIndefinite) &&
            foundOpt->isUsed)
            bParserLog(
                BPWARN,
                "The opt --%s that is not supposed to take multiple "
                "arguments has already been used\n",
                foundOpt->longOpt
            );
        else
            foundOpt->isUsed = true;

        copy[toEqual] = '=';
        bParserParseArgs(foundOpt, &(copy[toEqual + 1]));
        return false;
    }
}

void bParserPrintHelp(void)
{
    printf(
        "Program Name: %s\tVersion: %s\n",
        (programName == NULL) ? "N/A" : programName,
        (programVersion == NULL) ? "N/A" : programVersion
    );
    bParserPrintOpts();
    printf("\nAuthor: %s\n", (authorName == NULL) ? "N/A" : authorName);
    printf("Email: %s\n", (authorEmail == NULL) ? "N/A" : authorEmail);
}

void bParserAddOpts(const char shortOpt, const char *longOpt, int64_t group)
{
    if (!bParserOptDynArr.init)
        bParserOptDynArrInit();

    BParserOpt *newOpt   = (BParserOpt *)bParserMalloc(sizeof(BParserOpt));

    newOpt->shortOpt     = shortOpt;
    newOpt->longOpt      = NULL;
    newOpt->description  = NULL;
    newOpt->isUsed       = false;
    newOpt->isIndefinite = false;
    newOpt->argCount     = 0;
    newOpt->argType      = ANYBP;
    newOpt->args         = NULL;
    newOpt->group        = group;
    newOpt->argsCap      = 0;
    newOpt->argsUsed     = 0;

    if (longOpt != NULL) {
        uint64_t longOptLen = strlen(longOpt);
        newOpt->longOpt = (char *)bParserCalloc(longOptLen + 1, sizeof(char));
        if (newOpt->longOpt != NULL) {
            strcpy(newOpt->longOpt, longOpt);
        } else {
            bParserLog(
                BPCRIT,
                "Failed to allocate memory for the long opt version of "
                "opt -%c\n",
                shortOpt
            );
        }
    }

    bParserOptDynArrExpand();
    memcpy(
        bParserOptDynArr.opts + bParserOptDynArr.used, newOpt,
        sizeof(BParserOpt)
    );
    bParserOptDynArr.used += 1;
}

void bParserAddDesc(
    const char shortOpt, const char *longOpt, const char *description
)
{
    if (!bParserOptDynArr.init)
        bParserOptDynArrInit();

    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return;

    if (description != NULL) {
        uint64_t descriptionLen = strlen(description);
        opt->description =
            (char *)bParserCalloc(descriptionLen + 1, sizeof(char));
        strcpy(opt->description, description);

        return;
    }
}

void bParserAddArgs(
    const char shortOpt, const char *longOpt, int64_t argCount,
    enum BParserArgType type
)
{
    if (!bParserOptDynArr.init)
        bParserOptDynArrInit();

    if (argCount == 0)
        return;

    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return;

    if (argCount == -1)
        opt->isIndefinite = true;
    else
        opt->argCount = argCount;

    opt->argType = type;
}

void bParserSetProgramName(char *name)
{
    programName = name;
}

void bParserSetProgramVersion(char *version)
{
    programVersion = version;
}

void bParserSetAuthorName(char *name)
{
    authorName = name;
}

void bParserSetAuthorEmail(char *email)
{
    authorEmail = email;
}

void bParserParse(int argc, char **argv)
{
    if (bParserFindOpt(BParserNoShortOpt, "help", true) == NULL) {
        bParserAddOpts('h', "help", -1);
        bParserAddOpts('?', "help", -1);
        bParserAddDesc('h', NULL, "Prints this help statement");
        bParserAddDesc('?', NULL, "Prints this help statement");
    }

    for (uint64_t i = 1; i < (uint64_t)argc; i += 1) {
        bool isLongOpt    = (argv[i][0] == '-' && argv[i][1] == '-');
        bool isShortOpt   = (argv[i][0] == '-');
        char *supposedArg = ((uint64_t)argc <= (i + 1)) ? NULL : argv[i + 1];
        uint64_t parsedSupposedArg = 0;

        /*
         * Maybe it's in the C standard that true is 1 and false is 0 but
         * dunno if I can't really rely on what I don't know, maybe true is
         * another value
         */
        if (isLongOpt) {
            if (strcmp(&(argv[i][2]), "help") == 0) {
                bParserPrintHelp();
                exit(EXIT_SUCCESS);
            }
            parsedSupposedArg =
                (bParserParseLongOpt(argv[i] + 2, supposedArg)) ? 1 : 0;
        } else if (isShortOpt) {
            if (argv[i][1] == 'h' || argv[i][1] == '?') {
                bParserPrintHelp();
                exit(EXIT_SUCCESS);
            }
            parsedSupposedArg =
                (bParserParseShortOpt(argv[i] + 1, supposedArg)) ? 1 : 0;
        }

        i += parsedSupposedArg;
    }
}

bool bParserWasOptUsed(const char shortOpt, const char *longOpt)
{
    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return false;

    if (opt->argsUsed == 0)
        return false;

    return opt->isUsed;
}

void **bParserGetArgs(const char shortOpt, const char *longOpt)
{
    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return NULL;

    return opt->args;
}

void *bParserGetArg(const char shortOpt, const char *longOpt, uint64_t index)
{
    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return NULL;

    void **args = opt->args;
    if (args == NULL)
        return NULL;

    // Out of bounds?
    if (opt->argsUsed <= index)
        return NULL;

    return args[index];
}

int64_t bParserGetNumOfArgs(const char shortOpt, const char *longOpt)
{
    BParserOpt *opt = bParserFindOpt(shortOpt, longOpt, false);
    if (opt == NULL)
        return 0;

    if (opt->argCount == 0 && !opt->isIndefinite)
        return 0;

    return opt->argsUsed;
}

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __CROI_BREAD_PARSER_IMPL__
