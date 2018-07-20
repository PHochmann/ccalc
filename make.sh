RED='\033[1;32m'
NC='\033[0m'
mkdir -p bin
echo -e "${RED}COMPILING${NC}\n"
BASEDIR=$(dirname $0)
gcc -std=c99 -Wall -g ${BASEDIR}/src/main.c ${BASEDIR}/src/engine/*.c ${BASEDIR}/src/arithmetic/*.c -o ${BASEDIR}/bin/calculator.out -lm