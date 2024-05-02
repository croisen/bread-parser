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
