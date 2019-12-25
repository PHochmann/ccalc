# - - - - - - - Build configuration - - - - - - -
use_readline=true
test_tables=true
# - - - - - - - - - - - - - - - - - - - - - - - -

LFLAGS="-lm"
DEFINES=""
DIR=$(dirname $0)
CFLAGS="-std=c99 -Wall -Wextra -Werror -pedantic"
FILES="${DIR}/src/*/*.c ${DIR}/src/*.c"
BIN="${DIR}/bin/calculator.out"

if [ "$use_readline" = true ]; then
    LFLAGS="${LFLAGS} -lreadline"
    DEFINES="${DEFINES} -DUSE_READLINE"
fi

if [ "$test_tables" = true ]; then
    DEFINES="${DEFINES} -DTEST_TABLES"
fi

echo "Compiling Calculator (use_readline = ${use_readline})"
mkdir -p ${DIR}/bin

if [ "$1" = "-t" ] || [ "$1" = "-d" ]; then
    gcc ${DEFINES} -Og -g2 ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}
    if [ "$?" = 0 ] && [ "$1" = "-t" ]; then
        gcc ${DEFINES} -Og -g2 ${CFLAGS} \
            ${DIR}/src/tree/*.c \
            ${DIR}/src/arithmetics/arith_context.c \
            ${DIR}/src/string_util.c \
            ${DIR}/src/table/*.c \
            ${DIR}/tests/*.c \
            -o ${DIR}/bin/tests.out -lm
        if [ "$?" = 0 ]; then
            ${DIR}/bin/tests.out
        fi
    fi
else
    gcc ${DEFINES} -O3 ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}
fi
