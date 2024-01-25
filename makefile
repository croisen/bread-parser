CC				= gcc
CFLAGS			= -Wall -Wextra -Wpedantic -Werror
OPTS			= -O3 -g --std=gnu99

EXE				= bread_parser_test
MAIN			= bread_parser_test.c

VALGRIND		:= $(shell command -v valgrind 2> /dev/null)
VALGRIND_ARGS	= --track-origins=yes --leak-check=full -s --show-leak-kinds=all

default:
	${CC} ${CFLAGS} ${OPTS} -o ${EXE} ${MAIN}

test:
	${CC} ${CFLAGS} ${OPTS} -o ${EXE} ${MAIN}
ifdef VALGRIND
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -o
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -v
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -p "Hello, World" "some num supposedly"
else
	./${EXE} -o
	./${EXE} -v
	./${EXE} -p "Hello, World" "some num supposedly"
endif
