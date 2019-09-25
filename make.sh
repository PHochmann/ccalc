DIR=$(dirname $0)
CFLAGS="-std=c99 -Wall -Wextra -Werror -pedantic"
LFLAGS="-lm -lreadline"
FILES="${DIR}/src/*/*.c ${DIR}/src/*.c"
BIN="${DIR}/bin/calculator.out"

mkdir -p ${DIR}/bin

if [ "$1" = "-debug" ] || [ "$1" = "-d" ]; then
    echo "Compiling Calculator (Debug)"
    # Compiling calculator (retain symbols etc.)
    gcc -DDEBUG -g ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}
    # Run tests only when compilation succeeded
    if [ "$?" = 0 ]; then
        # Compiling tests.c
        gcc ${CFLAGS} ${DIR}/src/parsing/*.c ${DIR}/src/arithmetics/arith_context.c ${DIR}/src/string_util.c ${DIR}/tests/*.c -o ${DIR}/bin/tests.out -lm
        if [ "$?" = 0 ]; then
            # Executing tests
            ${DIR}/bin/tests.out
        fi
    fi
else
    echo "Compiling Calculator (Release)"
    gcc -O3 ${CFLAGS} ${FILES} -o ${BIN} ${LFLAGS}
fi
