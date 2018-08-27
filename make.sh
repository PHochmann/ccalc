BASEDIR=$(dirname $0)
GREEN='\033[1;32m'
NC='\033[0m'

mkdir -p ${BASEDIR}/bin

if [ "$1" = "-debug" ]; then

	printf "${GREEN}Compiling Calculator and performing tests...${NC}"
	
	# Compiling calculator (retain symbols etc.)
	gcc -DDEBUG -g -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
	
	# Run tests only when compilation succeeded
	if [ "$?" = 0 ]; then
		# Compiling tests.c
		gcc -g -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/arith.c ${BASEDIR}/tests/tests.c -o ${BASEDIR}/bin/tests.out -lm
		
		# Executing tests
		${BASEDIR}/bin/tests.out
		
		if [ "$?" = 0 ]; then
			echo -e "${GREEN}passed${NC}"
		fi
	fi
	
else

	echo -e "${GREEN}Compiling Calculator (Release)${NC}"
	# Compiling calculator (stripped and optimised)
	gcc -O3 -s -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
	
fi
