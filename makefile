CC			= cc
CFLAGS		= -Wall -Wextra -Wpedantic -Werror
OPTS		= -O3 -g

EXE			= bread_parser_test
MAIN		= bread_parser_test.c

VALGRIND	:= $(shell command -v valgrind 2> /dev/null)

test:
	${CC} ${CFLAGS} ${OPTS} -o ${EXE} ${MAIN}
ifdef VALGRIND
	valgrind --track-origins=yes ./${EXE}
else
	./${EXE}
endif
