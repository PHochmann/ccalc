GREEN='\033[1;32m'
NC='\033[0m'
mkdir -p bin
echo -e "${GREEN}COMPILING${NC}\n"
BASEDIR=$(dirname $0)
gcc -std=c99 -Wall -Werror -pedantic ${BASEDIR}/src/main.c ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/arithmetic/*.c -o ${BASEDIR}/bin/calculator.out -lm
