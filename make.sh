GREEN='\033[1;32m'
NC='\033[0m'
echo -e "${GREEN}Compiling Calculator...${NC}"
BASEDIR=$(dirname $0)
mkdir -p ${BASEDIR}/bin
#gcc -O3 -s -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
gcc -g -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/*.c -o ${BASEDIR}/bin/calculator.out -lm
