DIR=$(dirname $0)
FLAGS="-std=c99 -Wall -Wextra -Werror -pedantic"
mkdir -p ${BASEDIR}/bin

if [ "$1" = "-debug" ] || [ "$1" = "-d" ]; then
    # Compiling calculator (retain symbols etc.)
    gcc -DDEBUG -g ${FLAGS} ${DIR}/src/engine/*.c ${DIR}/src/commands/*.c ${DIR}/src/main.c -o ${DIR}/bin/calculator.out -lm -lreadline
    # Run tests only when compilation succeeded
    if [ "$?" = 0 ]; then
        # Compiling tests.c
        gcc -DDEBUG -g ${FLAGS} ${DIR}/src/engine/*.c ${DIR}/src/commands/arith_context.c ${DIR}/tests/*.c -o ${DIR}/bin/tests.out -lm
        if [ "$?" = 0 ]; then
            # Executing tests
            ${DIR}/bin/tests.out
        fi
    fi
else
    echo -e "Compiling Calculator (Release)"
    gcc ${FLAGS} ${DIR}/src/engine/*.c ${DIR}/src/commands/*.c ${DIR}/src/*.c -o ${DIR}/bin/calculator.out -lm -lreadline
fi
