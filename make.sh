BASEDIR=$(dirname $0)
mkdir -p ${BASEDIR}/bin

if [ "$1" = "-debug" ] || [ "$1" = "-d" ]; then
    printf "Compiling Calculator and performing tests..."
    # Compiling calculator (retain symbols etc.)
    gcc -DDEBUG -g -std=c99 -Wall -Wextra -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/commands/*.c ${BASEDIR}/src/main.c -o ${BASEDIR}/bin/calculator.out -lm -lreadline
    # Run tests only when compilation succeeded
    if [ "$?" = 0 ]; then
        # Compiling tests.c
        gcc -DDEBUG -g -std=c99 -Wall -Wextra -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/commands/arith_context.c ${BASEDIR}/tests/parser_test.c -o ${BASEDIR}/bin/parser_test.out -lm
        if [ "$?" = 0 ]; then
            # Executing tests
            ${BASEDIR}/bin/parser_test.out
        fi
    fi
else
    echo -e "Compiling Calculator (Release)"
    gcc -std=c99 -Wall -Wextra -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/commands/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm -lreadline
fi
