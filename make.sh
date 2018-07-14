RED='\033[0;31m'
NC='\033[0m' # No Color
echo -e "${RED}COMPILING${NC}\n"
gcc -std=c99 -Wall -g source/main.c source/engine/*.c source/arithmetic/*.c -o bin/calculator.out -lm