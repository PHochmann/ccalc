BASEDIR=$(dirname $0)
GREEN='\033[1;32m'
NC='\033[0m'

mkdir -p ${BASEDIR}/bin

if [ "$1" = "-debug" ]; then

	echo -e "${GREEN}Compiling Calculator and performing tests${NC}"
	# Compiling tests.c
	gcc -g -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/arith.c ${BASEDIR}/tests/tests.c -o ${BASEDIR}/bin/tests.out -lm
	# Executing tests
	${BASEDIR}/bin/tests.out
	
	# Compiling calculator (retain symbols etc.)
	gcc -g -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
	
else

	echo -e "${GREEN}Compiling Calculator (Release)${NC}"
	# Compiling calculator (stripped and optimised)
	gcc -O3 -s -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
	
fi
