CC				= cc
CFLAGS			= -Wall -Wextra -Wpedantic -Werror
OPTS			= -O3 -g --std=gnu99 -fPIE
DEBUG_FLAGS		= -D'__BCROI_PARSER_DEBUG'

EXE				= bread_parser_test
MAIN			= bread_parser_test.c

VALGRIND		:= ${shell command -v valgrind 2> /dev/null}
VALGRIND_ARGS	= --track-origins=yes --leak-check=full -s --show-leak-kinds=all


.PHONY: all test debug

all: clean ${EXE}

${EXE}: ${MAIN}
	${CC} ${CFLAGS} ${OPTS} -o $@ $?

debug: ${MAIN}
	${CC} ${CFLAGS} ${OPTS} -o ${EXE} $? ${DEBUG_FLAGS}

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
