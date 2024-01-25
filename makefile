CC			= cc
CFLAGS		= -Wall -Wextra -Wpedantic -Werror
OPTS		= -O3 -g

EXE			= bread_parser_test
MAIN		= bread_parser_test.c

test:
	${CC} ${CFLAGS} ${OPTS} -o ${EXE} ${MAIN}
	./${EXE}
