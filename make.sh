# - - - - - - - Build configuration - - - - - - -
use_readline=true
test_tables=false
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

if [ "$1" = "-debug" ] || [ "$1" = "-d" ]; then

    gcc ${DEFINES} -Og -g2 ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}

    if [ "$?" = 0 ]; then
        gcc ${DEFINES} -Og -g2 ${CFLAGS} ${DIR}/src/parsing/*.c ${DIR}/src/arithmetics/arith_context.c ${DIR}/src/util/string_util.c ${DIR}/src/util/table.c ${DIR}/tests/*.c -o ${DIR}/bin/tests.out -lm
        if [ "$?" = 0 ]; then
            ${DIR}/bin/tests.out
        fi
    fi
else
    gcc ${DEFINES} -O3 ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}
fi
