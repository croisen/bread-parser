CC				= cc
CFLAGS			= -Wall -Wextra -Wpedantic -Werror
OPTS			= --std=gnu99
RELEASE_FLAGS	= -O3 -s
DEBUG_FLAGS		= -Og -g

EXE				= bread_parser_test
MAIN			= bread_parser_test.c

VALGRIND		:= ${shell command -v valgrind 2> /dev/null}
VALGRIND_ARGS	= --track-origins=yes --leak-check=full -s --show-leak-kinds=all


.PHONY: all test debug

all: clean ${EXE}

${EXE}: ${MAIN}
	${CC} ${CFLAGS} ${RELEASE_FLAGS} ${OPTS} -o $@ $?

debug: ${MAIN}
	${CC} ${CFLAGS} ${DEBUG_FLAGS} ${OPTS} -o ${EXE} $?

clean:
	rm -f ${EXE}

test: debug
ifdef VALGRIND
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -o
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -v
	${VALGRIND} ${VALGRIND_ARGS} ./${EXE} -p "Hello, World" "some num supposedly"
else
	./${EXE} -o
	./${EXE} -v
	./${EXE} -p "Hello, World" "some num supposedly"
endif
